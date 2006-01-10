/*
 * Copyright 2005 Vasil Dimov
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions 
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <err.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>

#include "execcmd.h"
#include "logmsg.h"
#include "mkdb.h"
#include "portdef.h"
#include "portsearch.h"
#include "store.h"
#include "vector.h"

static const char rcsid[] = "$Id: mkdb.c,v 1.5 2006/01/10 10:48:46 dd Exp $";

struct pc_arg_t {
	const struct options_t	*opts;
	struct store_t		*store;
	int			s_exists;
	unsigned		ports_cnt;
	char			*category;
};

/*
 * Process all categories, contained in `line' (space-separated)
 */
static void process_categories(char *line, void *arg);

/*
 * Process all ports, contained in `line' (space-separated)
 */
static void process_ports_in_cat(char *line, void *arg);

/*
 * Create the packing list for a given port using make generate-plist
 */
static void mkplist(struct port_t *port, const struct pc_arg_t *arg);

/*
 * Process each line from port's plist, arg points to a port_t structure
 */
static void process_plist(char *line, void *arg);

/*
 * Retrieve port's last modification time, fs_category and fs_port members
 * of `port' must be set
 */
static void get_port_mtime(struct port_t *port);

/***/

void
mkdb(const struct options_t *opts)
{
	struct pc_arg_t	arg;

	arg.opts = opts;
	arg.ports_cnt = 1;

	logmsg(L_NOTICE, opts->verbose, "Creating database\n");

	alloc_store(&arg.store);

	if ((arg.s_exists = s_exists(NULL)))
	{
		logmsg(L_INFO, opts->verbose,
		       "Using data from existent database\n");
		s_read_start(arg.store);
	}
	else
		logmsg(L_INFO, opts->verbose,
		       "Previous database does not exist, creating from scratch\n");

	s_upd_start(arg.store);

#if 0
	char		*cmd = "make";
	char *const	args[] = {cmd, "-C", PORTSDIR, "-V", "SUBDIR", NULL};

	execcmd(cmd, args, process_categories, &arg);
#else
	char	categories[] = "archivers";

	process_categories(categories, &arg);
#endif

	s_upd_end(arg.store);

	if (arg.s_exists)
		s_read_end(arg.store);

	free_store(arg.store);
}

static void
process_categories(char *line, void *arg_void)
{
	struct pc_arg_t	*arg = (struct pc_arg_t *)arg_void;

	char		path[PATH_MAX];

	char		*cmd = "make";
	char *const	args[] = {cmd, "-C", path, "-V", "SUBDIR", NULL};

	while ((arg->category = strsep((char **)&line, " ")) != NULL)
	{
		logmsg(L_NOTICE, arg->opts->verbose, "Processing category %s\n",
		       arg->category);

		snprintf(path, sizeof(path), "%s/%s", PORTSDIR, arg->category);
		execcmd(cmd, args, process_ports_in_cat, arg);
	}
}

static void
process_ports_in_cat(char *line, void *arg_void)
{
	struct pc_arg_t			*arg = (struct pc_arg_t *)arg_void;
	struct port_t			port;
	struct vector_iterator_t	vi;
	char				*file;

	port.fs_category = arg->category;

	while ((port.fs_port = strsep((char **)&line, " ")) != NULL)
	{
		logmsg(L_INFO, arg->opts->verbose, "Processing port %s/%s\n",
		       port.fs_category, port.fs_port);

		get_port_mtime(&port);

		v_start(&port.plist, 256);

		mkplist(&port, arg);

		port.id = arg->ports_cnt;

		vi_reset(&vi, &port.plist);

		while (vi_next(&vi, (void **)&file))
			s_add_pfile(arg->store, &port, file);

		v_destroy(&port.plist);

		s_add_port(arg->store, &port);

		arg->ports_cnt++;
	}
}

static void
mkplist(struct port_t *port, const struct pc_arg_t *arg)
{
	struct port_t	s_port;
	int		create_plist;
	char		portdir[PATH_MAX];
	char		arg_PORTDIR[PATH_MAX];
	char		arg_I[PATH_MAX];
	char		*cmd = "make";
	char *const	args[] = {cmd,
		"-C", "../Mk", arg_I, arg_PORTDIR, "show-plist", NULL};

	if (arg->s_exists)
	{
		s_port.fs_category = port->fs_category;
		s_port.fs_port = port->fs_port;

		if (s_load_port_by_path(arg->store, &s_port) != -1)
		{
			logmsg(L_DEBUG, arg->opts->verbose,
			       "%s/%s database mtime:   %s",
			       port->fs_category, port->fs_port,
			       ctime(&s_port.mtime));

			logmsg(L_DEBUG, arg->opts->verbose,
			       "%s/%s filesystem mtime: %s",
			       port->fs_category, port->fs_port,
			       ctime(&port->mtime));

			if (port->mtime > s_port.mtime)
			{
				logmsg(L_INFO, arg->opts->verbose,
				       "%s/%s database outdated, recreating plist\n",
				       port->fs_category, port->fs_port);
				create_plist = 1;
			}
			else
			{
				logmsg(L_INFO, arg->opts->verbose,
				       "%s/%s using stored data\n",
				       port->fs_category, port->fs_port);
				create_plist = 0;
			}
		}
		else
		{
			logmsg(L_INFO, arg->opts->verbose,
			       "%s/%s not found in database, recreating plist\n",
			       port->fs_category, port->fs_port);
			create_plist = 1;
		}

	}
	else
		create_plist = 1;

	if (create_plist)
	{
		snprintf(portdir, sizeof(portdir), "%s/%s/%s", PORTSDIR,
			 port->fs_category, port->fs_port);

		/* math/vecfem (and maybe others) does ``.include <Makefile.inc>''
		 * to include /usr/ports/math/vecfem/Makefile.inc and therefore
		 * needs -I */
		snprintf(arg_I, sizeof(arg_I), "-I%s", portdir);
		snprintf(arg_PORTDIR, sizeof(arg_PORTDIR), "PORTDIR=%s", portdir);

		execcmd(cmd, args, process_plist, port);
	}
	else
	{
		port->id = s_port.id;
		s_load_port_plist(arg->store, port);
	}
}

static void
process_plist(char *line, void *arg)
{
	struct port_t	*port = (struct port_t *)arg;

	if (line[0] != '@')
		v_add(&port->plist, line, strlen(line) + 1);
}

static void
get_port_mtime(struct port_t *port)
{
	char		path[PATH_MAX];
	struct stat	sb;

	snprintf(path, sizeof(path), "%s/%s/%s/Makefile", PORTSDIR,
		 port->fs_category, port->fs_port);

	if (stat(path, &sb) == -1)
		err(EX_OSERR, "stat(): %s", path);

	port->mtime = sb.st_mtime;
}

/* EOF */

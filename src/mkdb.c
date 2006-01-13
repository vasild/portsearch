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
#include "exhaust_fp.h"
#include "logmsg.h"
#include "mkdb.h"
#include "portdef.h"
#include "portsearch.h"
#include "store.h"
#include "vector.h"
#include "xlibc.h"

#define IDXFS	'|'

static const char rcsid[] = "$Id: mkdb.c,v 1.9 2006/01/13 10:11:28 dd Exp $";

/* process_indexline parameter */
struct pi_arg_t {
	const struct options_t	*opts;
	struct store_t		*store;
	int			s_exists;
	char			*category;
};

static char	portsindex[PATH_MAX];

/*
 * Set index's filename using make -V INDEXFILE
 */
static void set_portsindex(const char *portsdir);
static void _set_portsindex(char *line, void *arg);

/*
 * Process single line from ports' INDEX file
 */
static void process_indexline(char *line, void *arg_void);

/*
 * Initialize port->path using port->indexln_raw
 */
static void set_port_path(struct port_t *port);

/*
 * port->path must be initialized
 * Initialize:
 * port->id using internal counter
 * port->mtime from the filesystem
 * port->plist by either generating it or retrieving it from
 * the old database (if one exists and port has not been modified recently)
 */
static void set_port_data(struct port_t *port, const struct pi_arg_t *arg);

/*
 * Create the packing list for a given port using make generate-plist
 * port->path must be initialized
 */
static void mkplist(struct port_t *port, const struct pi_arg_t *arg);

/*
 * Add file to port's plist
 */
static void add_pfile(char *file, void *port_void);

/*
 * Retrieve port's last modification time
 * port->path must be initialized
 */
static void set_port_mtime(const char *portsdir, struct port_t *port);

/*
 * Return pointer inside portpath, that points after portsdir
 */
static const char *mk_port_short_path(const char *portsdir,
				      const char *portpath);

/***/

void
mkdb(const struct options_t *opts)
{
	struct pi_arg_t	arg;
	FILE		*portsindex_fp;

	arg.opts = opts;

	set_portsindex(opts->portsdir);

	logmsg(L_NOTICE, opts->verbose, "Creating database\n");

	alloc_store(&arg.store);

	if ((arg.s_exists = s_exists(NULL)))
	{
		logmsg(L_NOTICE, opts->verbose,
		       "Using data from existent database\n");
		s_read_start(arg.store);
	}
	else
		logmsg(L_NOTICE, opts->verbose,
		       "Previous database does not exist, creating from scratch\n");

	s_upd_start(arg.store);

	portsindex_fp = xfopen(portsindex, "r");

	exhaust_fp(portsindex_fp, process_indexline, &arg);

	xfclose(portsindex_fp, portsindex);

	s_upd_end(arg.store);

	if (arg.s_exists)
		s_read_end(arg.store);

	free_store(arg.store);
}

static void
set_portsindex(const char *portsdir)
{
	char		*cmd = "make";
	char *const	args[] = {cmd,
		"-C", (char *)portsdir, "-V", "INDEXFILE", NULL};

	execcmd(cmd, args, _set_portsindex, (void *)portsdir);
}

static void
_set_portsindex(char *line, void *arg)
{
	snprintf(portsindex, sizeof(portsindex), "%s/%s", (char *)arg, line);
}

static void
process_indexline(char *line, void *arg_void)
{
	struct pi_arg_t	*arg = (struct pi_arg_t *)arg_void;
	struct port_t	addport;

	addport.indexln_raw = xstrdup(line);

	v_start(&addport.plist, 256);

	set_port_path(&addport);

#define TEST	1

#if TEST
	if (strncmp("/usr/ports/archivers", addport.path, 20) == 0)
	{
#endif
	logmsg(L_INFO, arg->opts->verbose, "==> %s\n",
	       mk_port_short_path(arg->opts->portsdir, addport.path));

	set_port_data(&addport, arg);

	s_add_port(arg->store, &addport);
#if TEST
	}
#endif

	v_destroy(&addport.plist);

	xfree(addport.indexln_raw);
}

static void
set_port_path(struct port_t *port)
{
	char	*path_beg;
	char	*path_end;

	path_beg = xstrchr(port->indexln_raw, IDXFS) + 1;
	path_end = xstrchr(path_beg, IDXFS);

	path_end[0] = '\0';

	snprintf(port->path, sizeof(port->path), "%s", path_beg);

	path_end[0] = IDXFS;
}

static void
set_port_data(struct port_t *port, const struct pi_arg_t *arg)
{
	static unsigned	portid = 1;

	const char	*spath;

	struct port_t	s_port;
	int		gen_plist;

	spath = mk_port_short_path(arg->opts->portsdir, port->path);

	set_port_mtime(arg->opts->portsdir, port);

	logmsg(L_DEBUG, arg->opts->verbose, "===> %s filesystem mtime: %s",
	       spath, ctime(&port->mtime));

	if (arg->s_exists)
	{
		snprintf(s_port.path, sizeof(s_port.path), "%s", port->path);

		if (s_load_port_by_path(arg->store, &s_port) != -1)
		{
			logmsg(L_DEBUG, arg->opts->verbose,
			       "===> %s database mtime:   %s", spath,
			       ctime(&s_port.mtime));

			if (port->mtime > s_port.mtime)
			{
				logmsg(L_INFO, arg->opts->verbose,
				       "===> %s outdated, recreating data\n",
				       spath);
				gen_plist = 1;
			}
			else
			{
				logmsg(L_INFO, arg->opts->verbose,
				       "===> %s using stored data\n",
				       spath);
				gen_plist = 0;
			}
		}
		else
		{
			logmsg(L_INFO, arg->opts->verbose,
			       "===> %s not found in database, recreating data\n",
			       spath);
			gen_plist = 1;
		}

	}
	else  /* store does not exist */
		gen_plist = 1;

	if (gen_plist)
	{
		mkplist(port, arg);
	}
	else
	{
		port->id = s_port.id;  /* temporary set to the old id */
		s_load_port_plist(arg->store, port);
	}

	port->id = portid;

	portid++;
}

static void
mkplist(struct port_t *port, const struct pi_arg_t *arg)
{
	char		arg_PORTDIR[PATH_MAX];
	char		arg_I[PATH_MAX];
	char		*cmd = "make";
	char *const	args[] = {cmd,
		"-C", "../Mk", arg_I, arg_PORTDIR, "show-plist", NULL};

	/* math/vecfem (and maybe others) does ``.include <Makefile.inc>''
	 * to include /usr/ports/math/vecfem/Makefile.inc and therefore
	 * needs -I */
	snprintf(arg_I, sizeof(arg_I), "-I%s", port->path);
	snprintf(arg_PORTDIR, sizeof(arg_PORTDIR), "PORTDIR=%s",
		 port->path);

	execcmd(cmd, args, add_pfile, port);
}

static void
add_pfile(char *file, void *port_void)
{
	struct port_t	*port = (struct port_t *)port_void;

	if (file[0] != '@')
		v_add(&port->plist, file, strlen(file) + 1);
}

static void
set_port_mtime(const char *portsdir, struct port_t *port)
{
	char		path[PATH_MAX];
	struct stat	sb;

	snprintf(path, sizeof(path), "%s/Makefile", port->path);

	if (stat(path, &sb) == -1)
		err(EX_OSERR, "stat(): %s", path);

	port->mtime = sb.st_mtime;
}

static const char *
mk_port_short_path(const char *portsdir, const char *portpath)
{
	const char	*spath;

	spath = portpath + strlen(portsdir);

	if (spath[0] == '/')
		spath++;

	return spath;
}

/* EOF */

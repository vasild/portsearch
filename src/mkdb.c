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

#include <sys/param.h>

#include <stdio.h>
#include <string.h>

#include "execcmd.h"
#include "get_port_mtime.h"
#include "mkdb.h"
#include "portdef.h"
#include "portsearch.h"
#include "store.h"
#include "vector.h"

static const char rcsid[] = "$Id: mkdb.c,v 1.1 2005/12/14 07:32:16 dd Exp $";

struct arg_t {
	unsigned	ports_cnt;
	char		*category;
	struct store_t	store;
};

/*
 * Process all categories, contained in `line' (space-separated)
 */
static int process_categories(char *line, void *arg);

/*
 * Process all ports, contained in `line' (space-separated)
 */
static int process_ports_in_cat(char *line, void *arg);

/*
 * Create the packing list for a given port with make generate-plist
 */
static void mkplist(struct port_t *port);

/*
 * Process each line from port's plist, arg points to a port_t structure
 */
static int process_plist(char *line, void *arg);

/*
 * Cut string `s' at first occurence of any of chars in `stop'
 */
static void cut_to(char *s, const char *stop);

/**/

void
mkdb(const struct options_t *opts)
{
	struct arg_t	arg;

	arg.ports_cnt = 1;

	printf("Creating database...\n");
	fflush(stdout);

	s_start(&arg.store);

#if 1
	char		*cmd = "make";
	char *const	args[] = {cmd, "-C", PORTSDIR, "-V", "SUBDIR", NULL};

	execcmd(cmd, args, process_categories, &arg);
#else
	char	categories[] = "cad";

	process_categories(categories, &arg);
#endif

	s_end(&arg.store);
}

static int
process_categories(char *line, void *arg_void)
{
	struct arg_t	*arg = (struct arg_t *)arg_void;

	char		path[PATH_MAX];

	char		*cmd = "make";
	char *const	args[] = {cmd, "-C", path, "-V", "SUBDIR", NULL};

	cut_to(line, "\n");

	while ((arg->category = strsep((char **)&line, " ")) != NULL)
	{
		snprintf(path, sizeof(path), "%s/%s", PORTSDIR, arg->category);
		execcmd(cmd, args, process_ports_in_cat, arg);
	}

	return 0;
}

static int
process_ports_in_cat(char *line, void *arg_void)
{
	struct arg_t			*arg = (struct arg_t *)arg_void;
	struct port_t			port;
	struct vector_iterator_t	vi;
	char				*file;

	port.fs_category = arg->category;

	cut_to(line, "\n");

	while ((port.fs_port = strsep((char **)&line, " ")) != NULL)
	{
		port.id = arg->ports_cnt;

		v_start(&port.plist, 256);

		mkplist(&port);

		vi_reset(&vi, &port.plist);

		while (vi_next(&vi, (void **)&file))
			s_add_file(&arg->store, &port, file);

		v_destroy(&port.plist);

		s_add_port(&arg->store, &port);

		arg->ports_cnt++;
	}

	return 0;
}

static void
mkplist(struct port_t *port)
{
	char		portdir[PATH_MAX];
	char		arg_PORTDIR[PATH_MAX];
	char		arg_I[PATH_MAX];
	char		*cmd = "make";
	char *const	args[] = {cmd,
		"-C", "../Mk", arg_I, arg_PORTDIR, "show-plist", NULL};

	get_port_mtime(port);

	snprintf(portdir, sizeof(portdir), "%s/%s/%s", PORTSDIR,
		 port->fs_category, port->fs_port);

	/* math/vecfem (and maybe others) does ``.include <Makefile.inc>''
	 * to include /usr/ports/math/vecfem/Makefile.inc and therefore
	 * needs -I */
	snprintf(arg_I, sizeof(arg_I), "-I%s", portdir);
	snprintf(arg_PORTDIR, sizeof(arg_PORTDIR), "PORTDIR=%s", portdir);

	execcmd(cmd, args, process_plist, port);
}

static int
process_plist(char *line, void *arg)
{
	struct port_t	*port = (struct port_t *)arg;

	cut_to(line, "\n");

	if (line[0] != '@')
		v_add(&port->plist, line, strlen(line) + 1);

	return 0;
}

static void
cut_to(char *s, const char *stop)
{
	*(s + strcspn(s, stop)) = '\0';
}

/* EOF */

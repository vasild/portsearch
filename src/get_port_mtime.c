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

#include <glob.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <sysexits.h>

#include "get_port_mtime.h"

#define USE_GLOB	1

#if !USE_GLOB
#define STATFILES_SZ	3
static const char *statfiles[STATFILES_SZ] =
{"Makefile", "pkg-plist", "files"};
#endif

static const char rcsid[] = "$Id: get_port_mtime.c,v 1.1 2005/12/14 06:11:47 dd Exp $";

void
get_port_mtime(struct port_t *port)
{
#if USE_GLOB
	char		pref[PATH_MAX];
	char		patt[PATH_MAX];
	glob_t		g;
	int		globflags;
	int		i;
	struct stat	sb;

	snprintf(pref, sizeof(pref), "%s/%s/%s", PORTSDIR, port->fs_category,
		 port->fs_port);

	globflags = GLOB_ERR | GLOB_NOSORT;

	snprintf(patt, sizeof(patt), "%s/Makefile*", pref);
	glob(patt, globflags, NULL, &g);

	globflags |= GLOB_APPEND;

	snprintf(patt, sizeof(patt), "%s/pkg-plist*", pref);
	glob(patt, globflags, NULL, &g);

	snprintf(patt, sizeof(patt), "%s/files*", pref);
	glob(patt, globflags, NULL, &g);

	if (g.gl_pathc == 0)
		errx(EX_NOINPUT, "No crucial files found in %s/%s/%s",
		     PORTSDIR, port->fs_category, port->fs_port);

	port->mtime = 0;

	for (i = 0; i < g.gl_pathc; i++)
	{
		if (stat(g.gl_pathv[i], &sb) == -1)
			err(EX_OSERR, "stat(): %s", g.gl_pathv[i]);

		if (port->mtime < sb.st_mtime)
			port->mtime = sb.st_mtime;
	}

	globfree(&g);
#else
	char		path[PATH_MAX];
	struct stat	sb;
	int		files_found;

	unsigned	i;

	port->mtime = 0;
	files_found = 0;

	for (i = 0; i < STATFILES_SZ; i++)
	{
		snprintf(path, sizeof(path), "%s/%s/%s/%s", PORTSDIR,
			 port->fs_category, port->fs_port, statfiles[i]);

		if (stat(path, &sb) == -1)
		{
			if (errno != ENOENT)  /* ignore nonexisting files */
				err(EX_OSERR, "stat(): %s", path);
		}
		else
		{
			files_found = 1;
		}

		if (port->mtime < sb.st_mtime)
			port->mtime = sb.st_mtime;
	}

	if (files_found == 0)
	{
		errx(EX_NOINPUT, "No crucial files found in %s/%s/%s",
		     PORTSDIR, port->fs_category, port->fs_port);
	}
#endif
}

/* EOF */

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

#include <err.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>

#include "execcmd.h"
#include "get_port_mtime.h"
#include "portdef.h"

#define DTFMT	"%Y.%m.%d %H:%M:%S"

static const char rcsid[] = "$Id: get_port_mtime_main.c,v 1.1 2005/12/14 06:11:47 dd Exp $";

int
main(int argc, char **argv)
{
	struct port_t	port;
	unsigned	i;

	struct tm	*tstruct;
	char		tstring[64];

	if (argc < 2)
		errx(EX_USAGE, "Usage: %s port [port [port ...]]\n"
		     "port can be /usr/ports/category/port or category/port",
		     argv[0]);

	for (i = 1; i < argc; i++)
	{
		port.fs_category = strdup(basename(dirname(argv[i])));
		port.fs_port = basename(argv[i]);

		get_port_mtime(&port);

		tstruct = localtime(&port.mtime);

		strftime(tstring, sizeof(tstring), DTFMT, tstruct);

		printf("%s/%s:%s\n", port.fs_category, port.fs_port, tstring);

		free(port.fs_category);
	}

	return 0;
}

/* EOF */

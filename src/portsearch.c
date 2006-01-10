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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "execcmd.h"
#include "mkdb.h"
#include "portdef.h"
#include "portsearch.h"
#include "store.h"

static const char rcsid[] = "$Id: portsearch.c,v 1.5 2006/01/10 15:23:21 dd Exp $";

/*
 * Get PORTSDIR from /etc/make.conf
 */
static void set_portsdir(struct options_t *opts);
static void _set_portsdir(char *line, void *arg);

/*
 * Print usage information end exit
 */
static void usage();

/*
 * Parse command line options and store results in `opts',
 * calls usage() if incorrect options are given
 */
static void parse_opts(int argc, char **argv, struct options_t *opts);

/***/

int
main(int argc, char **argv)
{
	struct options_t	opts;

	memset(&opts, 0, sizeof(opts));

	set_portsdir(&opts);

	parse_opts(argc, argv, &opts);

	if (opts.update_db)
		mkdb(&opts);
	else if (opts.search_file)
		show_ports_by_pfile(&opts);

	return 0;
}

static void
set_portsdir(struct options_t *opts)
{
	char		*cmd = "make";
	char *const	args[] = {cmd, "-C", "/", "-V", "PORTSDIR", NULL};

	execcmd(cmd, args, _set_portsdir, opts);

	if (opts->portsdir[0] == '\0')
		opts->portsdir = "/usr/ports";
}

static void
_set_portsdir(char *line, void *arg)
{
	static char	portsdir[PATH_MAX];

	snprintf(portsdir, sizeof(portsdir), "%s", line);

	((struct options_t *)arg)->portsdir = portsdir;
}

static void
usage()
{
	const char	*prog;
	
	prog = getprogname();

	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "update/create database:\n");
	fprintf(stderr, "  %s -u [-p portsdir] [-vvv]\n", prog);
	fprintf(stderr, "show ports that install file\n");
	fprintf(stderr, "  %s -f fileregexp\n", prog);

	exit(EX_USAGE);
}

static void
parse_opts(int argc, char **argv, struct options_t *opts)
{
	int	ch;

	while ((ch = getopt(argc, argv, "f:p:uvh")) != -1)
		switch (ch)
		{
		case 'f':
			opts->search_file = optarg;
			break;
		case 'p':  /* or 'd'irectory or 'r'oot */
			opts->portsdir = optarg;
			break;
		case 'u':
			opts->update_db = 1;
			break;
		case 'v':
			opts->verbose++;
			break;
		case 'h':
		case '?':
		default:
			usage();
		}

	argc -= optind;
	argv += optind;

	if (argc > 0)
		usage();

	if (!opts->update_db && !opts->search_file)
		usage();

	if (opts->update_db && opts->search_file)
		usage();
}

/* EOF */

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

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "mkdb.h"
#include "portdef.h"
#include "portsearch.h"
#include "store.h"

static const char rcsid[] = "$Id: portsearch.c,v 1.3 2005/12/23 10:00:18 dd Exp $";

/*
 * Print usage information end exit
 */
static void usage();

/*
 * Parse command line options and store results in `opts',
 * calls usage() if incorrect options are given
 */
static void parse_opts(int argc, char **argv, struct options_t *opts);

/**/

int
main(int argc, char **argv)
{
	struct options_t	opts;

	parse_opts(argc, argv, &opts);

	if (opts.update_db)
		mkdb(&opts);
	else if (opts.search_file)
		show_ports_by_pfile(&opts);

	return 0;
}

static void
usage()
{
	const char	*prog;
	
	prog = getprogname();

	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "%s -u [-v]		(update/create database)\n", prog);
	fprintf(stderr, "%s -f fileregexp	(show ports that install file)\n", prog);

	exit(EX_USAGE);
}

static void
parse_opts(int argc, char **argv, struct options_t *opts)
{
	int	ch;

	memset(opts, 0, sizeof(struct options_t));

	while ((ch = getopt(argc, argv, "uvf:h")) != -1)
		switch (ch)
		{
		case 'u':
			opts->update_db = 1;
			break;
		case 'v':
			opts->verbose++;
			break;
		case 'f':
			opts->search_file = optarg;
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

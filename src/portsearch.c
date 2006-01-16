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

#include <sys/cdefs.h>
#include <sys/param.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "display.h"
#include "execcmd.h"
#include "mkdb.h"
#include "portdef.h"
#include "portsearch.h"
#include "store.h"

#define OPT_NAME	"name="
#define OPT_NAME_LEN	5

__RCSID("$Id: portsearch.c,v 1.10 2006/01/16 17:53:32 dd Exp $");

/*
 * Retrieve PORTSDIR using make -V PORTSDIR
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
	struct store_t		*store;

	memset(&opts, 0, sizeof(opts));

	set_portsdir(&opts);

	parse_opts(argc, argv, &opts);

	if (opts.update_db)
		mkdb(&opts);
	else if (opts.search_crit)
	{
		if (!s_exists(NULL))
			errx(EX_USAGE, "Database does not exist, please create it first using the -u option");

		alloc_store(&store);

		s_search_start(store);

		if (opts.search_crit & SEARCH_BY_PFILE)
			filter_ports_by_pfile(store, opts.search_file);

		if (opts.search_crit & SEARCH_BY_NAME)
			filter_ports_by_name(store, opts.search_name);

		display_ports(get_ports(store), opts.search_crit);

		s_search_end(store);

		free_store(store);
	}

	return 0;
}

static void
set_portsdir(struct options_t *opts)
{
	char		*cmd = "make";
	char *const	args[] = {cmd,  /* -f /dev/null suggested by "Matthew D. Fuller" <fullermd@over-yonder.net> */
		"-f", "/dev/null", "-V", "PORTSDIR", NULL};

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
	fprintf(stderr, "\n");
	fprintf(stderr, "update/create database:\n");
	fprintf(stderr, "  %s -u [-p portsdir] [-vvv]\n", prog);
	fprintf(stderr, "\n");
	fprintf(stderr, "search for ports that (based on extended regular expressions):\n");
	fprintf(stderr, "  -n name\tare named like `name' (%s can be used instead of -n)\n", OPT_NAME);
	fprintf(stderr, "  -f file\tinstall file `file'\n");

	exit(EX_USAGE);
}

static void
parse_opts(int argc, char **argv, struct options_t *opts)
{
	int	ch;

	while ((ch = getopt(argc, argv, "f:n:p:uvh")) != -1)
		switch (ch)
		{
		case 'f':
			opts->search_crit |= SEARCH_BY_PFILE;
			opts->search_file = optarg;
			break;
		case 'n':
			opts->search_crit |= SEARCH_BY_NAME;
			opts->search_name = optarg;
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

	for (; argc > 0; argc--)
		if (strncmp(OPT_NAME, argv[argc - 1], OPT_NAME_LEN) == 0)
		{
			opts->search_crit |= SEARCH_BY_NAME;
			opts->search_name = argv[argc - 1] + OPT_NAME_LEN;
		}
		else
			usage();

	if (!opts->update_db && !opts->search_crit)
		usage();

	if (opts->update_db && opts->search_crit)
		usage();
}

/* EOF */

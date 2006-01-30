/*
 * Copyright 2005-2006 Vasil Dimov
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
#define OPT_KEY		"key="
#define OPT_KEY_LEN	4

__RCSID("$Id: portsearch.c,v 1.13 2006/01/30 16:39:50 dd Exp $");

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
 * Print version information and exit
 */
static void print_version();

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

		filter_ports(store, &opts);

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
	fprintf(stderr, "  %s -u [-H portshome] [-vvv]\n", prog);
	fprintf(stderr, "\n");
	fprintf(stderr, "search for ports (based on extended regular expressions):\n");
	fprintf(stderr, "  -n name\tby name (%s can be used instead of -n)\n", OPT_NAME);
	fprintf(stderr, "  -k key\tby name, comment or dependencies\n");
	fprintf(stderr, "  -p path\tby path on the filesystem\n");
	fprintf(stderr, "  -i info\tby info (comment)\n");
	fprintf(stderr, "  -m maint\tby maintainer\n");
	fprintf(stderr, "  -c category\tby category\n");
	fprintf(stderr, "  -F fdep\tby fetch dependencies\n");
	fprintf(stderr, "  -E edep\tby extract dependencies\n");
	fprintf(stderr, "  -P pdep\tby patch dependencies\n");
	fprintf(stderr, "  -B bdep\tby build dependencies\n");
	fprintf(stderr, "  -R rdep\tby run dependencies\n");
	fprintf(stderr, "  -D dep\tby build or run dependencies\n");
	fprintf(stderr, "  -w www\tby www site\n");
	fprintf(stderr, "  -f file\tthat install file\n");
	fprintf(stderr, "  -I\t\tignore case\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  -V\t\tprint version information\n");

	exit(EX_USAGE);
}

static void
print_version()
{
	printf("portsearch %s\n", PORTSEARCH_VERSION);
	exit(EX_OK);
}

static void
parse_opts(int argc, char **argv, struct options_t *opts)
{
	int	ch;

	while ((ch = getopt(argc, argv,
			    "vuH:" "n:k:p:i:m:c:F:E:P:B:R:w:f:I" "Vh")) != -1)
		switch (ch)
		{
		case 'v':
			opts->verbose++;
			break;
		case 'u':
			opts->update_db = 1;
			break;
		case 'H':
			opts->portsdir = optarg;
			break;
		case 'n':
			opts->search_crit |= SEARCH_BY_NAME;
			opts->search_name = optarg;
			break;
		case 'k':
			opts->search_crit |= SEARCH_BY_KEY;
			opts->search_key = optarg;
			break;
		case 'p':
			opts->search_crit |= SEARCH_BY_PATH;
			opts->search_path = optarg;
			break;
		case 'i':
			opts->search_crit |= SEARCH_BY_INFO;
			opts->search_info = optarg;
			break;
		case 'm':
			opts->search_crit |= SEARCH_BY_MAINT;
			opts->search_maint = optarg;
			break;
		case 'c':
			opts->search_crit |= SEARCH_BY_CAT;
			opts->search_cat = optarg;
			break;
		case 'F':
			opts->search_crit |= SEARCH_BY_FDEP;
			opts->search_fdep = optarg;
			break;
		case 'E':
			opts->search_crit |= SEARCH_BY_EDEP;
			opts->search_edep = optarg;
			break;
		case 'P':
			opts->search_crit |= SEARCH_BY_PDEP;
			opts->search_pdep = optarg;
			break;
		case 'B':
			opts->search_crit |= SEARCH_BY_BDEP;
			opts->search_bdep = optarg;
			break;
		case 'R':
			opts->search_crit |= SEARCH_BY_RDEP;
			opts->search_rdep = optarg;
			break;
		case 'w':
			opts->search_crit |= SEARCH_BY_WWW;
			opts->search_www = optarg;
			break;
		case 'f':
			opts->search_crit |= SEARCH_BY_PFILE;
			opts->search_file = optarg;
			break;
		case 'I':
			opts->icase = 1;
			break;
		case 'V':
			print_version();
			/* NOT REACHED */
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
		else if (strncmp(OPT_KEY, argv[argc - 1], OPT_KEY_LEN) == 0)
		{
			opts->search_crit |= SEARCH_BY_KEY;
			opts->search_key = argv[argc - 1] + OPT_KEY_LEN;
		}
		else
			usage();

	if (!opts->update_db && !opts->search_crit)
		usage();

	if (opts->update_db && opts->search_crit)
		usage();
}

/* EOF */

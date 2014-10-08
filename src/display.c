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

#include <stdio.h>

#include "display.h"
#include "portdef.h"
#include "portsearch.h"
#include "vector.h"

/*
 * Return true if at least one of `outflds' is DISP_RAWFILES
 */
static int is_rawfiles_on(const int outflds[DISP_FLDS_CNT]);

/*
 * Return the number of matched ports. We need this in advance if rawfiles
 * is on because we want to prefix each filename with `portpath:' only if
 * more than one port is matched.
 */
static size_t matched_ports_cnt(const struct ports_t *ports, int search_crit);

/*
 * Returns true if portpath should be shown.
 */
static int should_show_portpath(int rawfiles_is_on,
				const struct ports_t *ports,
				const struct options_t *opts);

void
display_ports(const struct ports_t *ports, const struct options_t *opts)
{
	struct vector_iterator_t	vi;
	struct port_t			*port;
	char				*filename;
	int				rawfiles_is_on;
	int				show_portpath;
	size_t				ports_cnt;
	size_t				files_cnt;
	size_t				i, ii;

	rawfiles_is_on = is_rawfiles_on(opts->outflds_parsed);

	show_portpath = should_show_portpath(rawfiles_is_on, ports, opts);

	ports_cnt = 0;
	files_cnt = 0;
	for (i = 0; i < ports->sz; i++)
		if (ports->arr[i]->matched == opts->search_crit)
		{
			ports_cnt++;

			port = ports->arr[i];

			if (rawfiles_is_on)
			{
				vi_reset(&vi, &port->plist);
				while (vi_next(&vi, (void **)&filename))
				{
					if (show_portpath)
						printf("%s:", port->path);
					printf("%s\n", filename);
				}
				continue;
			}

			for (ii = 0; ii < DISP_FLDS_CNT; ii++)
				switch (opts->outflds_parsed[ii])
				{
				case DISP_NAME:
					printf("Port:\t%s\n", port->pkgname);
					break;
				case DISP_PATH:
					printf("Path:\t%s\n", port->path);
					break;
				case DISP_INFO:
					printf("Info:\t%s\n", port->comment);
					break;
				case DISP_MAINT:
					printf("Maint:\t%s\n", port->maint);
					break;
				case DISP_CAT:
					printf("Index:\t%s\n", port->categories);
					break;
				case DISP_FDEP:
					printf("F-deps:\t%s\n", port->fdep);
					break;
				case DISP_EDEP:
					printf("E-deps:\t%s\n", port->edep);
					break;
				case DISP_PDEP:
					printf("P-deps:\t%s\n", port->pdep);
					break;
				case DISP_BDEP:
					printf("B-deps:\t%s\n", port->bdep);
					break;
				case DISP_RDEP:
					printf("R-deps:\t%s\n", port->rdep);
					break;
				case DISP_WWW:
					printf("WWW:\t%s\n", port->www);
					break;
				}

			if (ISSET(SEARCH_BY_PFILE, opts->search_crit))
			{
				printf("Files:\t");
				vi_reset(&vi, &port->plist);

				vi_next(&vi, (void **)&filename);
				files_cnt++;
				printf("%s", filename);

				while (vi_next(&vi, (void **)&filename))
				{
					files_cnt++;
					printf(", %s", filename);
				}

				printf("\n");
			}

			printf("\n");
		}

	if (!rawfiles_is_on)
	{
		printf("%u ports", (unsigned)ports_cnt);
		if (ISSET(SEARCH_BY_PFILE, opts->search_crit))
			printf(", %u files", (unsigned)files_cnt);
		printf("\n");
	}
}

static int
is_rawfiles_on(const int outflds[DISP_FLDS_CNT])
{
	int	i;

	for (i = 0; i < DISP_FLDS_CNT; i++)
		if (outflds[i] == DISP_RAWFILES)
			return 1;

	return 0;
}

static size_t
matched_ports_cnt(const struct ports_t *ports, int search_crit)
{
	size_t	cnt;
	size_t	i;

	cnt = 0;
	for (i = 0; i < ports->sz; i++)
		if (ports->arr[i]->matched == search_crit)
			cnt++;

	return cnt;
}

static int
should_show_portpath(int rawfiles_is_on,
		     const struct ports_t *ports,
		     const struct options_t *opts)
{
	if (rawfiles_is_on &&
	    (opts->always_show_portpath ||
	     matched_ports_cnt(ports, opts->search_crit) > 1))
		return 1;

	return 0;
}

/* EOF */

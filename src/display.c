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

__RCSID("$Id: display.c,v 1.11 2006/11/02 16:38:50 dd Exp $");

void
display_ports(const struct ports_t *ports, int search_crit,
	      int outflds[DISP_FLDS_CNT])
{
	struct vector_iterator_t	vi;
	struct port_t			*port;
	char				*filename;
	int				rawfiles_is_on;
	size_t				ports_cnt;
	size_t				files_cnt;
	size_t				i, ii;

	rawfiles_is_on = 0;
	for (ii = 0; ii < DISP_FLDS_CNT; ii++)
		if (outflds[ii] == DISP_RAWFILES)
		{
			rawfiles_is_on = 1;
			break;
		}

	ports_cnt = 0;
	files_cnt = 0;
	for (i = 0; i < ports->sz; i++)
		if (ports->arr[i]->matched == search_crit)
		{
			ports_cnt++;

			port = ports->arr[i];

			if (rawfiles_is_on)
			{
				vi_reset(&vi, &port->plist);
				while (vi_next(&vi, (void **)&filename))
					printf("%s:%s\n", port->path, filename);
				continue;
			}

			for (ii = 0; ii < DISP_FLDS_CNT; ii++)
				switch (outflds[ii])
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

			if (ISSET(SEARCH_BY_PFILE, search_crit))
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
		if (ISSET(SEARCH_BY_PFILE, search_crit))
			printf(", %u files", (unsigned)files_cnt);
		printf("\n");
	}
}

/* EOF */

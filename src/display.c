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

#include <stdio.h>

#include "display.h"
#include "portdef.h"
#include "portsearch.h"
#include "vector.h"

#define ISSET(flag, flags)	(flags & flag)

static const char rcsid[] = "$Id: display.c,v 1.2 2006/01/13 07:47:53 dd Exp $";

void
display_ports(const struct ports_t *ports, const struct options_t *opts,
	      int flags)
{
	struct vector_iterator_t	vi;
	struct port_t			*port;
	char				*filename;
	size_t				ports_cnt;
	size_t				files_cnt;
	size_t				i;

	for (ports_cnt = files_cnt = i = 0; i < ports->sz; i++)
		if (ports->arr[i]->matched == 1 || ISSET(DISPLAY_ALL, flags))
		{
			ports_cnt++;

			port = ports->arr[i];

			printf("Port:\t%s\n", port->descr.pkgname);
			printf("Path:\t%s\n", port->descr.path);
			printf("Info:\t%s\n", port->descr.comment);
			printf("Maint:\t%s\n", port->descr.maint);
			printf("Mtime:\t%s", ctime(&port->mtime));
			//printf("E-deps:\t%s\n", port->descr.edeps);
			//printf("P-deps:\t%s\n", port->descr.pdeps);
			//printf("F-deps:\t%s\n", port->descr.fdeps);
			//printf("B-deps:\t%s\n", port->descr.bdeps);
			//printf("R-deps:\t%s\n", port->descr.rdeps);
			printf("WWW:\t%s\n", port->descr.www);

			if (ISSET(DISPLAY_PFILES, flags))
			{
				printf("Files:\t");
				vi_reset(&vi, &ports->arr[i]->plist);

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
	printf("%zu ports", ports_cnt);
	if (ISSET(DISPLAY_PFILES, flags))
		printf(", %zu files", files_cnt);
	printf("\n");
}

/* EOF */

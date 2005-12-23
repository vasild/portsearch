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

static const char rcsid[] = "$Id: display.c,v 1.1 2005/12/23 09:56:42 dd Exp $";

void
display_ports(const struct ports_t *ports, const struct options_t *opts,
	      int flags)
{
	struct vector_iterator_t vi;
	char	*filename;
	size_t	ports_cnt, files_cnt, i;

	for (ports_cnt = files_cnt = i = 0; i < ports->sz; i++)
		if (ports->arr[i]->matched == 1 || ISSET(DISPLAY_ALL, flags))
		{
			ports_cnt++;

			printf("Path:\t%s/%s\n", ports->arr[i]->fs_category,
			       ports->arr[i]->fs_port);

			printf("Mtime:\t%s", ctime(&ports->arr[i]->mtime));

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

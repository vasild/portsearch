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

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "exhaust_fp.h"

__RCSID("$Id: exhaust_fp.c,v 1.3 2006/01/30 12:44:16 dd Exp $");

void
exhaust_fp(FILE *fp, void (*process)(char *, void *), void *process_arg)
{
	char	*buf;
	size_t	bufsz = BUFSIZ;  /* start with some reasonable size */
	size_t	bufofft = 0;
	size_t	buflen;

        if ((buf = (char *)malloc(bufsz)) == NULL)
		err(EX_OSERR, "malloc(): %zu", bufsz);

        while (fgets(buf + bufofft, bufsz - bufofft, fp) != NULL)
	{
                buflen = strlen(buf);
                if (buf[buflen - 1] != '\n')
		{
                        bufsz *= 2;
                        if ((buf = realloc(buf, bufsz)) == NULL)
				err(EX_OSERR, "realloc(): %zu", bufsz);
                        bufofft = buflen;
                        continue;
                }

		/* remove trailing newline */
		buf[buflen - 1] = '\0';

		process(buf, process_arg);

                bufofft = 0;
        }

	if (ferror(fp))
		errx(EX_IOERR, "ferror()");

        free(buf);
}

/* EOF */

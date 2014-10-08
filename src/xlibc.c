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

#include <sys/types.h>
#include <sys/cdefs.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <regex.h>

#include "xlibc.h"

void *
xmalloc(size_t size)
{
	void	*ptr;

	if ((ptr = malloc(size)) == NULL)
		err(EX_OSERR, "malloc(): %u", (unsigned)size);

	return ptr;
}

void
xfree(void *ptr)
{
	free(ptr);
}

FILE *
xfopen(const char *path, const char *mode)
{
	FILE	*fp;

	if ((fp = fopen(path, mode)) == NULL)
		err(EX_NOINPUT, "fopen(): %s", path);

	return fp;
}

void
xfclose(FILE *stream, const char *filename)
{
	if (fclose(stream) == -1)
		err(EX_IOERR, "fclose()%s%s",
		    filename == NULL ? "" : ": ",
		    filename == NULL ? "" : filename);
}

char *
xstrchr(const char *s, int c)
{
	char	*ret;

	if ((ret = strchr(s, c)) == NULL)
		err(EX_DATAERR, "strchr(): cannot locate %c in %s", c, s);

	return ret;
}

char *
xstrdup(const char *str)
{
	char	*ret;

	if ((ret = strdup(str)) == NULL)
		err(EX_OSERR, "strdup(): %s", str);

	return ret;
}

void
xregcomp(regex_t *preg, const char *pattern, int cflags)
{
	int	comp_err;
	char	comp_errstr[BUFSIZ];  /* BUFSIZ should be quite enough */

	if ((comp_err = regcomp(preg, pattern, cflags)) != 0)
	{
		regerror(comp_err, preg, comp_errstr, sizeof(comp_errstr));
		errx(EX_DATAERR, "\"%s\": %s", pattern, comp_errstr);
	}
}

void
xregfree(regex_t *preg)
{
	regfree(preg);
}

/* EOF */

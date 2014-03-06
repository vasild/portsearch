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

#include "vector.h"

void
v_start(struct vector_t *v, size_t initial_sz)
{
	size_t	malloc_bytes;

	malloc_bytes = initial_sz * sizeof(void *);

	if ((v->base = malloc(malloc_bytes)) == NULL)
		err(EX_OSERR, "malloc(): %u", (unsigned)malloc_bytes);
	v->base_sz = initial_sz;
	v->nelems = 0;
}

void
v_add(struct vector_t *v, const void *data, size_t size)
{
	size_t	realloc_bytes;

	if (v->nelems >= v->base_sz)
	{
		v->base_sz *= 2;
		realloc_bytes = v->base_sz * sizeof(void *);
		if ((v->base = realloc(v->base, realloc_bytes)) == NULL)
			err(EX_OSERR, "realloc(): %u", (unsigned)realloc_bytes);
	}

	if ((v->base[v->nelems] = malloc(size)) == NULL)
		err(EX_OSERR, "malloc(): %u", (unsigned)size);

	memcpy(v->base[v->nelems], data, size);

	v->nelems++;
}

void
v_destroy(struct vector_t *v)
{
	size_t	i;

	for (i = 0; i < v->nelems; i++)
		free(v->base[i]);

	free(v->base);

	v->base = NULL;
	v->base_sz = v->nelems = 0;
}

void
vi_reset(struct vector_iterator_t *vi, const struct vector_t *v)
{
	vi->v = v;
	vi->current = 0;
}

int
vi_next(struct vector_iterator_t *vi, void **elem)
{
	if (vi->current >= vi->v->nelems)
		return 0;

	*elem = vi->v->base[vi->current];

	vi->current++;

	return 1;
}

/* EOF */

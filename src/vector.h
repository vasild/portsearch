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

#ifndef VECTOR_H
#define VECTOR_H

#include <stdio.h>

struct vector_t {
	void	**base;
	size_t	base_sz;
	size_t	nelems;
};

struct vector_iterator_t {
	const struct vector_t	*v;
	size_t			current;
};

/*
 * Create vector with the specified initial size,
 * which must be greater than zero
 */
void v_start(struct vector_t *v, size_t initial_sz);

/*
 * Add `data' to `v', data is copied
 */
void v_add(struct vector_t *v, const void *data, size_t size);

/*
 * Free resources, allocated by `v'
 */
void v_destroy(struct vector_t *v);

/*
 * Initialize `vi' with `v'
 */
void vi_reset(struct vector_iterator_t *vi, const struct vector_t *v);

/*
 * Iterate over `vi', saving the current element in `elem'
 * Returns 1 if the element was successfully saved in `elem',
 * 0 otherwise (no more elements)
 */
int vi_next(struct vector_iterator_t *vi, void **elem);

#endif  /* VECTOR_H */

/* EOF */

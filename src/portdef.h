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

#ifndef PORTDEF_H
#define PORTDEF_H

#include <time.h>

#include "vector.h"

struct port_t {
	unsigned	id;  /* port unique number */
	time_t		mtime;  /* last modification time */
	char		path[128];  /* full port's path, used to identify the port when id is not applicable */
	char		*indexln_raw;  /* line from INDEX file for this port */
	/* pointers inside indexln_raw */
	char		*pkgname;
	char		*prefix;
	char		*comment;
	char		*pkgdescr;
	char		*maint;
	char		*categories;
	char		*fdep;
	char		*edep;
	char		*pdep;
	char		*bdep;
	char		*rdep;
	char		*www;
	struct vector_t	plist;  /* plist files */
	int		matched;  /* matched by some search criteria */
};

struct ports_t {
	struct port_t	**arr;  /* ports' array, there may be NULL pointers in it */
	size_t		sz;  /* number of allocated elements in arr */
};

#endif  /* PORTDEF_H */

/* EOF */

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

#include <sys/param.h>
#include <sys/utsname.h>

#include <err.h>
#include <stdio.h>
#include <sysexits.h>

#include "portdef.h"
#include "store.h"

#define FS	"|"
#define RS	"\n"

static const char rcsid[] = "$Id: store_txt.c,v 1.1 2005/12/14 06:11:47 dd Exp $";

static char	index_fn[PATH_MAX];
static char	plist_fn[PATH_MAX];

static char	index_new_fn[PATH_MAX];
static char	plist_new_fn[PATH_MAX];

/*
 * Set index and plist filenames
 */
static void set_filenames();

void
s_start(struct store_t *store)
{
	set_filenames();

	if ((store->index = fopen(index_new_fn, "w")) == NULL)
		err(EX_CANTCREAT, "fopen(): %s", index_new_fn);

	if ((store->plist = fopen(plist_new_fn, "w")) == NULL)
		err(EX_CANTCREAT, "fopen(): %s", plist_new_fn);
}

void
s_add_port(struct store_t *store, const struct port_t *port)
{
	if (fprintf(store->index, "%u"FS"%s"FS"%s"FS"%u"RS, port->id,
		    port->fs_category, port->fs_port, (unsigned)port->mtime)
	    == -1)
		err(EX_IOERR, "fprintf(): %s", index_new_fn);
}

void
s_add_file(struct store_t *store, const struct port_t *port, const char *file)
{
	if (fprintf(store->plist, "%u"FS"%s"RS, port->id, file) == -1)
		err(EX_IOERR, "fprintf(): %s", plist_new_fn);
}

void
s_end(struct store_t *store)
{
	if (fclose(store->index) == -1)
		err(EX_IOERR, "fclose(): %s", index_new_fn);

	if (fclose(store->plist) == -1)
		err(EX_IOERR, "fclose(): %s", plist_new_fn);

	if (rename(index_new_fn, index_fn) == -1)
		err(EX_CANTCREAT, "rename(): %s to %s", index_new_fn, index_fn);

	if (rename(plist_new_fn, plist_fn) == -1)
		err(EX_CANTCREAT, "rename(): %s to %s", plist_new_fn, plist_fn);
}

static void
set_filenames()
{
	struct utsname	un;
	char		scheme[PATH_MAX];

	if (uname(&un) == -1)
		err(EX_OSERR, "uname()");

	snprintf(scheme, sizeof(scheme), "%c-%s", un.release[0], un.machine);

	snprintf(index_fn, sizeof(index_fn), "%s-index", scheme);
	snprintf(plist_fn, sizeof(plist_fn), "%s-plist", scheme);

	snprintf(index_new_fn, sizeof(index_new_fn), "%s-index.new", scheme);
	snprintf(plist_new_fn, sizeof(plist_new_fn), "%s-plist.new", scheme);
}

/* EOF */

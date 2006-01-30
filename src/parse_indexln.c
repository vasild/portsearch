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
#include <string.h>
#include <sysexits.h>

#include "parse_indexln.h"
#include "portdef.h"

__RCSID("$Id: parse_indexln.c,v 1.4 2006/01/30 12:44:16 dd Exp $");

void
parse_indexln(struct port_t *port)
{
	char	fs[2] = {IDXFS, '\0'};
	char	*fld, *raw_p;
	size_t	idx;

	raw_p = port->indexln_raw;

	for (idx = 0; ((fld = strsep(&raw_p, fs)) != NULL); idx++)
		switch (idx)
		{
		case 0:
			port->pkgname = fld;
			break;
		case 1:
			snprintf(port->path, sizeof(port->path), "%s", fld);
			break;
		case 2:
			port->prefix = fld;
			break;
		case 3:
			port->comment = fld;
			break;
		case 4:
			port->pkgdescr = fld;
			break;
		case 5:
			port->maint = fld;
			break;
		case 6:
			port->categories = fld;
			break;
		case 7:
			port->bdep = fld;
			break;
		case 8:
			port->rdep = fld;
			break;
		case 9:
			port->www = fld;
			break;
		case 10:
			port->edep = fld;
			break;
		case 11:
			port->pdep = fld;
			break;
		case 12:
			port->fdep = fld;
			break;
		default:
			errx(EX_DATAERR, "Cannot parse INDEX line for %s",
			     port->pkgname);
		}
}

/* EOF */

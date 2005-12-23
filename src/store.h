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

#ifndef STORE_H
#define STORE_H

#include <stdio.h>

#include "portdef.h"
#include "portsearch.h"

struct store_t {
	FILE	*index;
	FILE	*plist;
};

#if 0
struct all_ports_t {
	struct ports_t	ports;
	char		*rawdata;
};
#endif

/*
 * Initialize store for updating
 */
void s_upd_start(struct store_t *store);

/*
 * Add port to store
 */
void s_add_port(struct store_t *store, const struct port_t *port);

/*
 * Add plist file from port to store
 */
void s_add_pfile(struct store_t *store, const struct port_t *port,
		 const char *file);

/*
 * Close store from updating
 */
void s_upd_end(struct store_t *store);

/**/

void show_ports_by_pfile(const struct options_t *opts);

/*
 * Find ports that install file, which matches regular expression `file_re'.
 * Data is malloc'd and free_ports_by_plist() must be called when it is not
 * needed anymore. The plist member of each port, in the resulting array,
 * contains the matched files.
 */
void find_ports_by_plist(const char *file_re, struct ports_t *ports);

#if 0
/*
 * Load all ports from existing store
 */
void load_ports(struct ports_t *ports);

/*
 * Free `ports', initialized by previous call to load_ports
 */
void free_ports(struct ports_t *ports);
#endif

#endif  /* STORE_H */

/* EOF */

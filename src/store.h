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

#ifndef STORE_H
#define STORE_H

#include <stdio.h>

#include "portdef.h"
#include "portsearch.h"

struct store_t;

/*
 * *s = malloc(sizeof(struct store_t));
 */
void alloc_store(struct store_t **s);

/*
 * Free resources allocated by alloc_store()
 */
void free_store(struct store_t *s);

/*
 * Check if store exists. If `s' is a NULL pointer then check if default store
 * exists. Returns 1 for existence, 0 otherwise.
 */
int s_exists(struct store_t *s);

/*
 * Return internal `ports' structure
 */
struct ports_t *get_ports(struct store_t *s);

/* store manipulation procedures */

/*
 * Initialize store for updating, independent of s_read_start()
 */
void s_upd_start(struct store_t *s);

/*
 * Close store from updating
 */
void s_upd_end(struct store_t *s);

/*
 * Add port to store
 */
void s_add_port(struct store_t *s, const struct port_t *port);

/* store reading procedures */

/*
 * Initialize store for reading, independent of s_upd_start()
 * Either this or s_search_start must be called
 * Loads both index and plist files
 */
void s_read_start(struct store_t *s);

/*
 * Close store, freeing resources allocated by s_read_start()
 */
void s_read_end(struct store_t *s);

/*
 * Initialize store for searching
 * Either this or s_read_start must be called
 * Loads only the index file
 */
void s_search_start(struct store_t *s);

/*
 * Close store, freeing resources allocated by s_search_start()
 */
void s_search_end(struct store_t *s);

/*
 * Get pointer to port in store that has its path member equal to `path'
 * Store must have been s_read_start'ed
 * If port is not found, then -1 is returned
 */
int s_load_port_by_path(struct store_t *s, const char *path,
			struct port_t **port);

/*
 * Load port's plist
 * id member of `port' must be set
 */
void s_load_port_plist(struct store_t *s, struct port_t *port);

/* All searching is done based on extended regular expressions */

/*
 * Add SEARCH_BY_NAME to `matched' member of all ports named like
 * `search_name'
 */
void filter_ports_by_name(struct store_t *s, const char *search_name);

/*
 * Add SEARCH_BY_PFILE to `matched' member of all ports that have
 * `search_file' in their plist
 */
void filter_ports_by_pfile(struct store_t *s, const char *search_file);

#endif  /* STORE_H */

/* EOF */

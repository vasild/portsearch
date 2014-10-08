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

#define DBDIR	"/var/db/portsearch"

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
 * Check if the store exists.
 * Return 1 for existence, 0 otherwise.
 */
int s_exists();

/*
 * Return internal `ports' structure
 */
struct ports_t *get_ports(struct store_t *s);

/* store manipulation procedures */

/*
 * Initialize a temporary new store, do not touch the current one
 * Independent of s_read_start()
 */
void s_new_start(struct store_t *s);

/*
 * Close the temporary new store and replace the current one with it
 */
void s_new_end(struct store_t *s);

/*
 * Add a port to the temporary store created by s_new_start()
 */
void s_add_port(struct store_t *s, const struct port_t *port);

/* store reading procedures */

/*
 * Initialize store for reading, independent of s_new_start()
 * Either this or s_search_start must be called
 * s_load_port_by_path() and s_load_port_plist() can be used when this
 * function is called
 */
void s_read_start(struct store_t *s);

/*
 * Close store, freeing resources allocated by s_read_start()
 */
void s_read_end(struct store_t *s);

/*
 * Initialize store for searching
 * Either this or s_read_start can be called, but not both
 * Loads only the index database
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
 * Filter internal ports structure (that can be retrieved with get_ports()),
 * so that all non-NULL members get their `matched' member properly
 * initialized based on opts->search_crit
 */
void filter_ports(struct store_t *s, const struct options_t *opts);

#endif  /* STORE_H */

/* EOF */

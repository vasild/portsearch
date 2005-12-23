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

#include <sys/types.h>
#include <sys/limits.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/utsname.h>

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "display.h"
#include "exhaust_fp.h"
#include "portdef.h"
#include "store.h"
#include "vector.h"
#include "xmalloc.h"

#define DBDIR	"/var/db/portsearch"

#define RSi	'\n'  /* record separator for index file */
#define FSi	'|'  /* field separator for index file */

/* RSp must be '\n' because we use fgets */
#define RSp	'\n'  /* record separator for plist file */
#define FSp	'|'  /* field separator for plist file */

static const char rcsid[] = "$Id: store_txt.c,v 1.2 2005/12/23 10:00:18 dd Exp $";

struct f_arg_t {
	regex_t		re;
	struct ports_t	*ports;
};

struct aarg_t {
	regex_t		re;
	struct ports_t	ports;
};

static char	index_fn[PATH_MAX];
static char	plist_fn[PATH_MAX];

static char	index_new_fn[PATH_MAX];
static char	plist_new_fn[PATH_MAX];

/*
 * Set index and plist filenames
 */
static void set_filenames();

static void compile_pfile_re(const char *file_re, regex_t *re);
static void open_plist(FILE **fp);

static void f(char *line, void *arg);

#if 1

int pcmp(const void *p1v, const void *p2v);
void load_ports(struct ports_t *ports, char **raw);
void free_ports(struct ports_t *ports, char *raw);

/*
 * Load index from disk, ``raw'' will be exact image of the file's content
 */
void load_ports_raw(char **raw);

/*
 * Calculate ports' count
 */
size_t calc_ports_cnt(const char *raw);

/*
 * Free raw data allocated by `load_ports_raw'
 */
void free_ports_raw(char *raw);
#endif

void
s_upd_start(struct store_t *store)
{
	set_filenames();

	if (mkdir(DBDIR, 0755) == -1)
		if (errno != EEXIST)
			err(EX_CANTCREAT, "mkdir(): %s", DBDIR);

	if ((store->index = fopen(index_new_fn, "w")) == NULL)
		err(EX_CANTCREAT, "fopen(): %s", index_new_fn);

	if ((store->plist = fopen(plist_new_fn, "w")) == NULL)
		err(EX_CANTCREAT, "fopen(): %s", plist_new_fn);
}

void
s_add_port(struct store_t *store, const struct port_t *port)
{
	if (fprintf(store->index, "%u%c""%s%c""%s%c""%u%c",
		    port->id, FSi,
		    port->fs_category, FSi,
		    port->fs_port, FSi,
		    (unsigned)port->mtime, RSi)
	    == -1)
		err(EX_IOERR, "fprintf(): %s", index_new_fn);
}

void
s_add_pfile(struct store_t *store, const struct port_t *port, const char *file)
{
	if (fprintf(store->plist, "%u%c""%s%c", port->id, FSp, file, RSp) == -1)
		err(EX_IOERR, "fprintf(): %s", plist_new_fn);
}

void
s_upd_end(struct store_t *store)
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

	snprintf(scheme, sizeof(scheme), "%s/%c-%s",
		 DBDIR, un.release[0], un.machine);

	snprintf(index_fn, sizeof(index_fn), "%s-index", scheme);
	snprintf(plist_fn, sizeof(plist_fn), "%s-plist", scheme);

	snprintf(index_new_fn, sizeof(index_new_fn), "%s-index.new", scheme);
	snprintf(plist_new_fn, sizeof(plist_new_fn), "%s-plist.new", scheme);
}

/**/

void
_get_max_portid(char *line, void *maxid_void)
{
	unsigned	*maxid = (unsigned *)maxid_void;
	unsigned	current;

	current = (unsigned)strtoul(line, NULL, 10);

	if (current > *maxid)
		*maxid = current;
}

unsigned
get_max_portid()
{
	FILE		*fp;
	unsigned	maxid;

	if ((fp = fopen(index_fn, "r")) == NULL)
		err(EX_NOINPUT, "fopen(): %s", index_fn);

	exhaust_fp(fp, _get_max_portid, &maxid);

	fclose(fp);

	return maxid;
}

void
alloc_ports(struct ports_t *ports, unsigned max_portid)
{
	size_t		malloc_bytes;
	unsigned	i;

	/* we want to acces max_portid'th member without nasty expressions 
	 * like [portid-1] */
	ports->sz = max_portid + 1;

	malloc_bytes = ports->sz * sizeof(struct port_t *);

	if ((ports->arr = (struct port_t **)malloc(malloc_bytes)) == NULL)
		err(EX_OSERR, "malloc(): %zu", malloc_bytes);

	for (i = 0; i < ports->sz; i++)
		ports->arr[i] = NULL;
}

void
load_ports_data(struct ports_t *ports)
{

}

void
find_ports_by_plist(const char *file_re, struct ports_t *ports)
{
	FILE		*plist_fp;
	struct f_arg_t	f_arg;

	f_arg.ports = ports;

	set_filenames();

	alloc_ports(ports, get_max_portid());

	compile_pfile_re(file_re, &f_arg.re);

	open_plist(&plist_fp);

	exhaust_fp(plist_fp, f, &f_arg);

	fclose(plist_fp);

	regfree(&f_arg.re);

	load_ports_data(ports);

#if 1
	unsigned	i;
	for (i = 0; i < ports->sz; i++)
		if (ports->arr[i] != NULL)
		{
			struct vector_iterator_t	vi;
			char				*file;

			vi_reset(&vi, &ports->arr[i]->plist);
			while (vi_next(&vi, (void **)&file))
				printf("%u|%s\n", ports->arr[i]->id, file);
		}
#endif
}

static void
compile_pfile_re(const char *file_re, regex_t *re)
{
	int	comp_err;
	char	comp_errstr[BUFSIZ];  /* BUFSIZ should be quite enough */

	if ((comp_err = regcomp(re, file_re, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(comp_err, re, comp_errstr, sizeof(comp_errstr));
		errx(EX_USAGE, "\"%s\": %s", file_re, comp_errstr);
	}
}

void
open_index(FILE **fp)
{
	if ((*fp = fopen(index_fn, "r")) == NULL)
		err(EX_NOINPUT, "fopen(): %s", index_fn);
}

static void
open_plist(FILE **fp)
{
	if ((*fp = fopen(plist_fn, "r")) == NULL)
		err(EX_NOINPUT, "fopen(): %s", plist_fn);
}

static void
f(char *line, void *arg_void)
{
	struct f_arg_t	*arg = (struct f_arg_t *)arg_void;
	static unsigned	line_num = 0;
	char		*file;

	unsigned	portid;
	struct ports_t	*ports;
	struct port_t	*port;

	line_num++;

	ports = arg->ports;

	if ((file = strchr(line, FSp)) == NULL)
		errx(EX_DATAERR, "corrupted datafile: %s: "
		     "``%c'' not found on line %u", plist_fn, FSp, line_num);

	/* split line into portid and filename */
	file[0] = '\0';
	file++;

	if (regexec(&arg->re, file, 0, NULL, 0) != 0)
		/* no match */
		return;

	portid = (unsigned)strtoul(line, NULL, 10);

	if (ports->arr[portid] == NULL)
	{
		if ((port = (struct port_t *)malloc(sizeof(struct port_t)))
		    == NULL)
			err(EX_OSERR, "malloc(): %zu", sizeof(struct port_t));

		port->id = portid;

		v_start(&port->plist, 4);

		ports->arr[portid] = port;
	}

	v_add(&ports->arr[portid]->plist, file, strlen(file) + 1);
}


void
get_port_by_id(struct ports_t *ports, const char *portid, struct port_t **port)
{
	struct port_t	key;
	struct port_t	*key_p;
	struct port_t	**res;

	key.id = (unsigned)strtoul(portid, NULL, 10);

	key_p = &key;

	res = (struct port_t **)bsearch(&key_p, ports->arr, ports->sz,
					sizeof(struct port_t **), pcmp);

	if (res == NULL)
		errx(EX_DATAERR, "corrupted database: port with id %u exists in "
		     "plist file but not found in index file", key.id);

	*port = *res;
}

void
gather_pfiles(char *line, void *arg)
{
	struct aarg_t	*aarg = (struct aarg_t *)arg;

	static unsigned	line_num = 0;

	char		*portid;
	struct port_t	*port;
	char		*FSp_pos;
	char		*filename;

	line_num++;

	if ((FSp_pos = strchr(line, FSp)) == NULL)
		errx(EX_DATAERR, "corrupted datafile: %s: "
		     "``%c'' not found on line %u", plist_fn, FSp, line_num);

	if (regexec(&aarg->re, FSp_pos + 1, 0, NULL, 0) != 0)
		return;

	/* match */

	/* split line into portid and filename */
	filename = FSp_pos + 1;
	FSp_pos[0] = '\0';
	portid = line;

	get_port_by_id(&aarg->ports, portid, &port);

	if (port->matched == 0)
		v_start(&port->plist, 2);

	port->matched = 1;

	v_add(&port->plist, filename, strlen(filename) + 1);
}

void
show_ports_by_pfile(const struct options_t *opts)
{
	FILE		*plist_fp;
	struct aarg_t	aarg;
	char		*portsraw;

	set_filenames();
	
	load_ports(&aarg.ports, &portsraw);

	compile_pfile_re(opts->search_file, &aarg.re);

	open_plist(&plist_fp);

	exhaust_fp(plist_fp, gather_pfiles, &aarg);

	fclose(plist_fp);

	regfree(&aarg.re);

	display_ports(&aarg.ports, opts, DISPLAY_PFILES);

	free_ports(&aarg.ports, portsraw);
}


#if 1
int
pcmp(const void *p1v, const void *p2v)
{
	struct port_t	*p1;
	struct port_t	*p2;

	p1 = *(struct port_t **)p1v;
	p2 = *(struct port_t **)p2v;

	if (p1 == NULL && p2 == NULL)
		return 0;
	if (p1 == NULL)
		return -1;
	if (p2 == NULL)
		return 1;

	return p1->id - p2->id;
}

void
load_ports(struct ports_t *ports, char **raw)
{
	char		rs[2] = {RSi, '\0'};
	char		fs[2] = {FSi, '\0'};
	char		*raw_p, *rec, *fld;
	size_t		rec_idx, fld_idx;
	size_t		i;
	struct port_t	*cur_port;

	set_filenames();

	load_ports_raw(raw);

	ports->sz = calc_ports_cnt(*raw);

	ports->arr =
	    (struct port_t **)xmalloc(ports->sz * sizeof(struct port_t *));

	for (i = 0; i < ports->sz; i++)
		ports->arr[i] = (struct port_t *)xmalloc(sizeof(struct port_t));

	raw_p = *raw;

	for (rec_idx = 0; (rec = strsep(&raw_p, rs)) != NULL; rec_idx++)
	{
		if (rec[0] == '\0')
			continue;

		cur_port = ports->arr[rec_idx];

		cur_port->matched = 0;

		for (fld_idx = 0; (fld = strsep(&rec, fs)) != NULL; fld_idx++)
		{
			switch (fld_idx)
			{
			case 0:
				cur_port->id = (unsigned)strtoul(fld, NULL, 10);
				break;
			case 1:
				cur_port->fs_category = fld;
				break;
			case 2:
				cur_port->fs_port = fld;
				break;
			case 3:
				cur_port->mtime = (time_t)strtoul(fld, NULL,10);
				break;
			default:
				assert(0 && "The impossible happened, committing suicide");
			}
		}
	}

	/* normally ports are loaded ordered, but just to make sure */
	if (mergesort(ports->arr, ports->sz, sizeof(struct port_t **), pcmp)
	    == -1)
		err(EX_OSERR, "mergesort()");
}

void
free_ports(struct ports_t *ports, char *raw)
{
	size_t	i;

	for (i = 0; i < ports->sz; i++)
		if (ports->arr[i] != NULL)
			xfree(ports->arr[i]);

	xfree(ports->arr);

	free_ports_raw(raw);
}

void
load_ports_raw(char **raw)
{
	int		fd;
	struct stat	sb;
	size_t		sz;
	size_t		offt;
	ssize_t		rd_len;

	if ((fd = open(index_fn, O_RDONLY)) == -1)
		err(EX_NOINPUT, "open(): %s", index_fn);

	if (fstat(fd, &sb) == -1)
		err(EX_OSERR, "fstat(): %s", index_fn);

	sz = sb.st_size;

	if ((*raw = (char *)malloc(sz + 1)) == NULL)
		err(EX_OSERR, "malloc(): %zu", sz);

	offt = 0;

	while ((rd_len = read(fd, *raw, sz - offt)) <= sz - offt)
	{
		if (rd_len == -1)
			err(EX_IOERR, "read(): %s", index_fn);

		if (rd_len == 0)
			break;

		offt += rd_len;
	}

	if (sz != offt)
		errx(EX_PROTOCOL, "while reading %s: fstat returned %zu bytes "
		     "for file size but we got %zu from it",
		     index_fn, sz, offt);

	(*raw)[sz] = '\0';

	if (close(fd) == -1)
		err(EX_IOERR, "close(): %s", index_fn);
}

size_t
calc_ports_cnt(const char *content)
{
	const char	*ch;
	size_t		cnt;

	for (cnt = 0, ch = content; *ch != '\0'; ch++)
		if (*ch == RSi)
			cnt++;

	return cnt;
}

void
free_ports_raw(char *raw)
{
	xfree(raw);
}
#endif

/* EOF */

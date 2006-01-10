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

static const char rcsid[] = "$Id: store_txt.c,v 1.4 2006/01/10 09:23:47 dd Exp $";

struct pline_t {
	unsigned	portid;
	char		*pfile;
};

struct plist_t {
	char		*raw;
	size_t		plines_cnt;
	struct pline_t	*plines;
};

struct store_t {
	char		index_fn[PATH_MAX];
	char		plist_fn[PATH_MAX];
	char		index_new_fn[PATH_MAX];
	char		plist_new_fn[PATH_MAX];

	FILE		*index_fp;
	FILE		*plist_fp;
	FILE		*index_new_fp;
	FILE		*plist_new_fp;

	struct ports_t	ports;
	char		*ports_raw;
	struct plist_t	*plist;
};

/* gather_pfiles argument */
struct garg_t {
	regex_t		re;
	struct store_t	store;
};

/*
 * Set index and plist filenames
 */
static void set_filenames(struct store_t *store);

/*
 * Load file from disk, ``raw'' will be exact image of the file's content
 */
static void load_file(const char *filename, char **raw);

/*
 * Free data allocated by load_file()
 */
static void free_file(char *raw);

/*
 * Place plist files that match `arg->re' in the appropriate `plist'
 * members of the `arg->ports' structure
 */
static void gather_pfiles(char *line, void *arg);

/*
 */
static void get_port_by_id(struct ports_t *ports, const char *portid,
			   struct port_t **port);

/*
 * Calculate number of occurences of `sep' in `raw'
 */
static size_t records_cnt(const char *raw, char sep);

/*
 * Compare 2 ports accoring to their IDs
 */
static int ports_cmp(const void *p1v, const void *p2v);

/*
 * Load whole index file (all ports) from disk
 */
static void load_ports(struct store_t *s);

/*
 * Free data allocated by load_ports()
 */
static void free_ports(struct store_t *s);

/*
 * Load whole plist file from disk
 */
static void load_plist(struct store_t *s);

/*
 * Free data allocated by load_plist()
 */
static void free_plist(struct store_t *s);

/*
 * Compare 2 plist lines, according to their portids
 */
static int plines_cmp(const void *l1v, const void *l2v);

/*
 * Compile the regular expression, exit on error
 */
static void xregcomp(const char *restr, regex_t *re);

/*
 * Open file and exit on failure
 */
static void xfopen(FILE **fp, const char *filename, const char *mode);

/***/

void
alloc_store(struct store_t **s)
{
	*s = (struct store_t *)xmalloc(sizeof(struct store_t));
}

void
free_store(struct store_t *s)
{
	xfree(s);
}

int
s_exists(struct store_t *s)
{
	struct store_t	s_default;
	struct store_t	*p;

	if (s != NULL)
	{
		p = s;
	}
	else
	{
		set_filenames(&s_default);
		p = &s_default;
	}

	if (access(p->index_fn, F_OK) == -1 || access(p->plist_fn, F_OK) == -1)
		return 0;

	return 1;
}

void
s_upd_start(struct store_t *s)
{
	set_filenames(s);

	if (mkdir(DBDIR, 0755) == -1)
		if (errno != EEXIST)
			err(EX_CANTCREAT, "mkdir(): %s", DBDIR);

	if ((s->index_new_fp = fopen(s->index_new_fn, "w")) == NULL)
		err(EX_CANTCREAT, "fopen(): %s", s->index_new_fn);

	if ((s->plist_new_fp = fopen(s->plist_new_fn, "w")) == NULL)
		err(EX_CANTCREAT, "fopen(): %s", s->plist_new_fn);
}

void
s_add_port(struct store_t *s, const struct port_t *port)
{
	if (fprintf(s->index_new_fp,
		    "%u%c""%s%c""%s%c""%u%c",
		    port->id, FSi,
		    port->fs_category, FSi,
		    port->fs_port, FSi,
		    (unsigned)port->mtime, RSi) == -1)
		err(EX_IOERR, "fprintf(): %s", s->index_new_fn);
}

void
s_add_pfile(struct store_t *s, const struct port_t *port, const char *file)
{
	if (fprintf(s->plist_new_fp,
		    "%u%c""%s%c",
		    port->id, FSp,
		    file, RSp) == -1)
		err(EX_IOERR, "fprintf(): %s", s->plist_new_fn);
}

void
s_upd_end(struct store_t *s)
{
	if (fclose(s->index_new_fp) == -1)
		err(EX_IOERR, "fclose(): %s", s->index_new_fn);

	if (fclose(s->plist_new_fp) == -1)
		err(EX_IOERR, "fclose(): %s", s->plist_new_fn);

	if (rename(s->index_new_fn, s->index_fn) == -1)
		err(EX_CANTCREAT, "rename(): %s to %s",
		    s->index_new_fn, s->index_fn);

	/* XXX if interrupted here database is left in inconsistent state */

	if (rename(s->plist_new_fn, s->plist_fn) == -1)
		err(EX_CANTCREAT, "rename(): %s to %s",
		    s->plist_new_fn, s->plist_fn);
}

/***/

void
s_read_start(struct store_t *s)
{
	set_filenames(s);

	load_ports(s);
	load_plist(s);
}

void
s_read_end(struct store_t *s)
{
	free_ports(s);
	free_plist(s);
}

void
show_ports_by_pfile(const struct options_t *opts)
{
	struct garg_t	garg;

	set_filenames(&garg.store);
	
	load_ports(&garg.store);

	xregcomp(opts->search_file, &garg.re);

	xfopen(&garg.store.plist_fp, garg.store.plist_fn, "r");

	exhaust_fp(garg.store.plist_fp, gather_pfiles, &garg);

	fclose(garg.store.plist_fp);

	regfree(&garg.re);

	display_ports(&garg.store.ports, opts, DISPLAY_PFILES);

	free_ports(&garg.store);
}

/***/

static void
set_filenames(struct store_t *store)
{
	struct utsname	un;
	char		scheme[PATH_MAX];

	if (uname(&un) == -1)
		err(EX_OSERR, "uname()");

	snprintf(scheme, sizeof(scheme), "%s/%c-%s",
		 DBDIR, un.release[0], un.machine);

	snprintf(store->index_fn, sizeof(store->index_fn), "%s-index", scheme);
	snprintf(store->plist_fn, sizeof(store->plist_fn), "%s-plist", scheme);

	snprintf(store->index_new_fn, sizeof(store->index_new_fn), "%s-index.new", scheme);
	snprintf(store->plist_new_fn, sizeof(store->plist_new_fn), "%s-plist.new", scheme);
}

static void
load_ports(struct store_t *s)
{
	char		rs[2] = {RSi, '\0'};
	char		fs[2] = {FSi, '\0'};
	char		*raw_p, *rec, *fld;
	size_t		rec_idx, fld_idx;
	size_t		i;
	struct port_t	*cur_port;

	load_file(s->index_fn, &s->ports_raw);

	s->ports.sz = records_cnt(s->ports_raw, RSi);

	s->ports.arr =
	    (struct port_t **)xmalloc(s->ports.sz * sizeof(struct port_t *));

	for (i = 0; i < s->ports.sz; i++)
		s->ports.arr[i] =
		    (struct port_t *)xmalloc(sizeof(struct port_t));

	raw_p = s->ports_raw;

	for (rec_idx = 0; (rec = strsep(&raw_p, rs)) != NULL; rec_idx++)
	{
		if (rec[0] == '\0')
			continue;

		cur_port = s->ports.arr[rec_idx];

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
	if (mergesort(s->ports.arr,
		      s->ports.sz,
		      sizeof(struct port_t **),
		      ports_cmp) == -1)
		err(EX_OSERR, "mergesort()");
}

static void
free_ports(struct store_t *s)
{
	size_t	i;

	for (i = 0; i < s->ports.sz; i++)
		if (s->ports.arr[i] != NULL)
			xfree(s->ports.arr[i]);

	xfree(s->ports.arr);

	free_file(s->ports_raw);
}

static void
load_file(const char *filename, char **raw)
{
	int		fd;
	struct stat	sb;
	size_t		sz;
	size_t		offt;
	ssize_t		rd_len;

	if ((fd = open(filename, O_RDONLY)) == -1)
		err(EX_NOINPUT, "open(): %s", filename);

	if (fstat(fd, &sb) == -1)
		err(EX_OSERR, "fstat(): %s", filename);

	sz = sb.st_size;

	if ((*raw = (char *)malloc(sz + 1)) == NULL)
		err(EX_OSERR, "malloc(): %zu", sz);

	offt = 0;

	while ((rd_len = read(fd, *raw, sz - offt)) <= sz - offt)
	{
		if (rd_len == -1)
			err(EX_IOERR, "read(): %s", filename);

		if (rd_len == 0)
			break;

		offt += rd_len;
	}

	if (sz != offt)
		errx(EX_PROTOCOL, "while reading %s: fstat returned %zu bytes "
		     "for file size but we got %zu from it",
		     filename, sz, offt);

	(*raw)[sz] = '\0';

	close(fd);
}

static void
free_file(char *raw)
{
	xfree(raw);
}

static void
gather_pfiles(char *line, void *arg_void)
{
	struct garg_t	*arg = (struct garg_t *)arg_void;

	static unsigned	line_num = 0;

	char		*portid;
	struct port_t	*port;
	char		*FSp_pos;
	char		*filename;

	line_num++;

	if ((FSp_pos = strchr(line, FSp)) == NULL)
		errx(EX_DATAERR, "corrupted datafile: %s: "
		     "``%c'' not found on line %u",
		     arg->store.plist_fn, FSp, line_num);

	if (regexec(&arg->re, FSp_pos + 1, 0, NULL, 0) != 0)
		return;

	/* match */

	/* split line into portid and filename */
	filename = FSp_pos + 1;
	FSp_pos[0] = '\0';
	portid = line;

	get_port_by_id(&arg->store.ports, portid, &port);

	if (port->matched == 0)
		v_start(&port->plist, 2);

	port->matched = 1;

	v_add(&port->plist, filename, strlen(filename) + 1);
}

static void
get_port_by_id(struct ports_t *ports, const char *portid, struct port_t **port)
{
	struct port_t	key;
	struct port_t	*key_p;
	struct port_t	**res;

	key_p = &key;

	key.id = (unsigned)strtoul(portid, NULL, 10);

	res = (struct port_t **)bsearch(&key_p, ports->arr, ports->sz,
					sizeof(struct port_t **), ports_cmp);

	if (res == NULL)
		errx(EX_DATAERR, "corrupted database: port with id %u exists in "
		     "plist file but not found in index file", key.id);

	*port = *res;
}

static size_t
records_cnt(const char *content, char sep)
{
	const char	*ch;
	size_t		cnt;

	for (cnt = 0, ch = content; *ch != '\0'; ch++)
		if (*ch == sep)
			cnt++;

	return cnt;
}

static int
ports_cmp(const void *p1v, const void *p2v)
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

int
s_load_port_by_path(struct store_t *s, struct port_t *port)
{
	size_t	i;

	for (i = 0; i < s->ports.sz; i++)
	{
		if (s->ports.arr[i] == NULL)
			continue;

		if (strcmp(s->ports.arr[i]->fs_category,
			   port->fs_category) == 0 &&
		    strcmp(s->ports.arr[i]->fs_port,
			   port->fs_port) == 0)
		{
			port->id = s->ports.arr[i]->id;
			port->mtime = s->ports.arr[i]->mtime;
			return 0;
		}
	}

	return -1;
}

void
s_load_port_plist(struct store_t *s, struct port_t *port)
{
	struct pline_t	search_pline;
	struct pline_t	*found_pline;
	struct pline_t	*p;

	search_pline.portid = port->id;

	found_pline = (struct pline_t *)bsearch(&search_pline,
						s->plist->plines,
						s->plist->plines_cnt,
						sizeof(struct pline_t),
						plines_cmp);

	if (found_pline != NULL)
	{
		for (p = found_pline - 1; p->portid == port->id; p--)
			v_add(&port->plist, p->pfile, strlen(p->pfile) + 1);

		for (p = found_pline; p->portid == port->id; p++)
			v_add(&port->plist, p->pfile, strlen(p->pfile) + 1);
	}
}

static void
load_plist(struct store_t *s)
{
	char	rs[2] = {RSp, '\0'};
	char	fs[2] = {FSp, '\0'};
	char	*raw_p, *rec, *fld;
	size_t	rec_idx, fld_idx;

	s->plist = (struct plist_t *)xmalloc(sizeof(struct plist_t));

	load_file(s->plist_fn, &s->plist->raw);

	s->plist->plines_cnt = records_cnt(s->plist->raw, RSp);

	s->plist->plines = (struct pline_t *)xmalloc(s->plist->plines_cnt *
						     sizeof(struct pline_t));

	raw_p = s->plist->raw;

	for (rec_idx = 0; (rec = strsep(&raw_p, rs)) != NULL; rec_idx++)
	{
		if (rec[0] == '\0')
			continue;

		for (fld_idx = 0; (fld = strsep(&rec, fs)) != NULL; fld_idx++)
		{
			switch (fld_idx)
			{
			case 0:
				s->plist->plines[rec_idx].portid =
				    (unsigned)strtoull(fld, NULL, 10);
				break;
			case 1:
				s->plist->plines[rec_idx].pfile = fld;
				break;
			default:
				assert(0 && "The impossible happened, committing suicide");
			}
		}
	}

	/* normally plist is loaded ordered, but just to make sure */
	if (mergesort(s->plist->plines,
		      s->plist->plines_cnt,
		      sizeof(struct pline_t),
		      plines_cmp) == -1)
		err(EX_OSERR, "mergesort()");
}

static void
free_plist(struct store_t *s)
{
	xfree(s->plist->plines);

	free_file(s->plist->raw);

	xfree(s->plist);
}

static int
plines_cmp(const void *l1v, const void *l2v)
{
	struct pline_t	*l1;
	struct pline_t	*l2;

	l1 = (struct pline_t *)l1v;
	l2 = (struct pline_t *)l2v;

	return l1->portid - l2->portid;
}

static void
xregcomp(const char *restr, regex_t *re)
{
	int	comp_err;
	char	comp_errstr[BUFSIZ];  /* BUFSIZ should be quite enough */

	if ((comp_err = regcomp(re, restr, REG_EXTENDED | REG_NOSUB)) != 0)
	{
		regerror(comp_err, re, comp_errstr, sizeof(comp_errstr));
		errx(EX_USAGE, "\"%s\": %s", restr, comp_errstr);
	}
}

static void
xfopen(FILE **fp, const char *filename, const char *mode)
{
	if ((*fp = fopen(filename, mode)) == NULL)
		err(EX_NOINPUT, "fopen(): %s", filename);
}

/* EOF */

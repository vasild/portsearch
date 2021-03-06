/*
 * Copyright 2005-2014 Vasil Dimov
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
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <err.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>
#if __FreeBSD_version < 500000
/* we use chdir(2) on 4.x */
#include <unistd.h>
#endif

#include "execcmd.h"
#include "exhaust_fp.h"
#include "logmsg.h"
#include "mkdb.h"
#include "parse_indexln.h"
#include "portdef.h"
#include "portsearch.h"
#include "store.h"
#include "vector.h"
#include "xlibc.h"

/* process_indexline parameter */
struct pi_arg_t {
	const struct options_t	*opts;
	struct store_t		*store;
	int			s_exists;
	char			*category;
};

static char	portsindex[PATH_MAX];

/*
 * Set index's filename using make -V INDEXFILE
 */
static void set_portsindex(const char *portsdir);
static void _set_portsindex(char *line, void *arg);

/*
 * Process single line from ports' INDEX file
 */
static void process_indexline(char *line, void *arg_void);

/*
 * port->path must be initialized
 * Initialize:
 * port->id using internal counter
 * port->plist by either generating it or retrieving it from
 * the old store (if one exists and port has not been modified recently)
 */
static void set_port_data(struct port_t *port, const struct pi_arg_t *arg);

/*
 * Create the packing list for a given port using make generate-plist
 * port->path must be initialized
 */
static void mkplist(struct port_t *port, const struct pi_arg_t *arg);

/*
 * Add file to port's plist
 */
static void add_pfile(char *file, void *port_void);

/*
 * Return pointer inside portpath, that points after portsdir
 */
static const char *mk_port_short_path(const char *portsdir,
				      const char *portpath);

/*
 * Return pointer inside pkgname - after the last `-'
 */
static const char *mk_pkgversion(const char *pkgname);

/***/

void
mkdb(const struct options_t *opts)
{
	struct pi_arg_t	arg;
	FILE		*portsindex_fp;

	arg.opts = opts;

	set_portsindex(opts->portsdir);

	logmsg(L_NOTICE, opts->verbose, "Creating store\n");

	alloc_store(&arg.store);

	arg.s_exists = s_exists();
	if (arg.s_exists)
	{
		s_read_start(arg.store);
		logmsg(L_NOTICE, opts->verbose,
		       "Using data from existent store\n");
	}
	else
		logmsg(L_NOTICE, opts->verbose,
		       "Previous store does not exist, creating from scratch\n");

	s_new_start(arg.store);

	portsindex_fp = xfopen(portsindex, "r");

	exhaust_fp(portsindex_fp, process_indexline, &arg);

	xfclose(portsindex_fp, portsindex);

	s_new_end(arg.store);

	if (arg.s_exists)
		s_read_end(arg.store);

	free_store(arg.store);
}

static void
set_portsindex(const char *portsdir)
{
	char		*cmd = "make";
#if __FreeBSD_version >= 500000
	char *const	args[] = {cmd,
		"-C", (char *)portsdir, "-V", "INDEXFILE", NULL};
#else  /* 4.x handles -C differently */
	char		makefile[PATH_MAX];
	char *const	args[] = {cmd,
		"-f", makefile, "-V", "INDEXFILE", NULL};
	snprintf(makefile, sizeof(makefile), "%s/Makefile", portsdir);
#endif

	execcmd(cmd, args, _set_portsindex, (void *)portsdir);
}

static void
_set_portsindex(char *line, void *arg)
{
	snprintf(portsindex, sizeof(portsindex), "%s/%s", (char *)arg, line);
}

static void
process_indexline(char *line, void *arg_void)
{
	struct pi_arg_t	*arg = (struct pi_arg_t *)arg_void;
	struct port_t	addport;

	addport.indexln_raw = line;

	v_start(&addport.plist, 256);

	parse_indexln(&addport);

#define TEST	0

#if TEST
	if (strncmp("/usr/ports/archivers", addport.path, 20) == 0)
	{
#endif
	logmsg(L_INFO, arg->opts->verbose, "==> %s\n",
	       mk_port_short_path(arg->opts->portsdir, addport.path));

	set_port_data(&addport, arg);

	s_add_port(arg->store, &addport);
#if TEST
	}
#endif

	v_destroy(&addport.plist);
}

static void
set_port_data(struct port_t *port, const struct pi_arg_t *arg)
{
	static unsigned	portid = 1;

	const char	*spath;  /* short path: /usr/ports/a/b -> a/b */
	const char	*pkgver_index;
	const char	*pkgver_store;

	struct port_t	*store_port;
	int		gen_plist;

	spath = mk_port_short_path(arg->opts->portsdir, port->path);

	pkgver_index = mk_pkgversion(port->pkgname);

	logmsg(L_DEBUG, arg->opts->verbose, "===> %s INDEX version: %s\n",
	       spath, pkgver_index);

	if (arg->s_exists)
	{
		if (s_load_port_by_path(arg->store, port->path, &store_port)
		    != -1)
		{
			pkgver_store = mk_pkgversion(store_port->pkgname);

			logmsg(L_DEBUG, arg->opts->verbose,
			       "===> %s STORE version: %s\n", spath,
			       pkgver_store);

			if (strcmp(pkgver_store, pkgver_index) != 0)
			{
				logmsg(L_INFO, arg->opts->verbose,
				       "===> %s versions differ, recreating data\n",
				       spath);
				gen_plist = 1;
			}
			else
			{
				logmsg(L_INFO, arg->opts->verbose,
				       "===> %s using stored data\n",
				       spath);
				gen_plist = 0;
			}
		}
		else
		{
			logmsg(L_INFO, arg->opts->verbose,
			       "===> %s not found in store, recreating data\n",
			       spath);
			gen_plist = 1;
		}

	}
	else  /* store does not exist */
		gen_plist = 1;

	if (gen_plist)
		mkplist(port, arg);
	else
	{
		port->id = store_port->id;  /* temporary set to the old id */
		s_load_port_plist(arg->store, port);
	}

	port->id = portid;

	portid++;
}

static void
mkplist(struct port_t *port, const struct pi_arg_t *arg)
{
	char		our_makefile[PATH_MAX];
	char		port_makefile[PATH_MAX];
	char		*cmd = "make";
#if __FreeBSD_version >= 500000
	char *const	args[] = {cmd,
		"-C", port->path, "-f", port_makefile, "-f", our_makefile,
		"show-plist", NULL};
#else  /* 4.x handles -C differently */
	char		curdir_arg[PATH_MAX];
	char *const	args[] = {cmd,
		"-C", port->path, curdir_arg,
		"-f", port_makefile, "-f", our_makefile,
		"show-plist", NULL};
	snprintf(curdir_arg, sizeof(curdir_arg), ".CURDIR=%s", port->path);

	/* math/vecfem does .include <Makefile.inc>, no hope for this on 4.x */
	chdir(port->path);
#endif

	snprintf(our_makefile, sizeof(our_makefile), "%s/Makefile", DATADIR);
	snprintf(port_makefile, sizeof(port_makefile), "%s/Makefile", port->path);

	execcmd(cmd, args, add_pfile, port);
}

static void
add_pfile(char *file, void *port_void)
{
	struct port_t	*port = (struct port_t *)port_void;

	if (file[0] != '@')
		v_add(&port->plist, file, strlen(file) + 1);
}

static const char *
mk_port_short_path(const char *portsdir, const char *portpath)
{
	const char	*spath;

	spath = portpath + strlen(portsdir);

	if (spath[0] == '/')
		spath++;

	return spath;
}

static const char *
mk_pkgversion(const char *pkgname)
{
	const char	*ret;

	if ((ret = strrchr(pkgname, '-')) == NULL)
		/* should never happen */
		return pkgname;

	return ++ret;
}

/* EOF */

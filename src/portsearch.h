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

#ifndef PORTSEARCH_H
#define PORTSEARCH_H

#define PORTSEARCH_VERSION	"1.2.0"

#define SEARCH_BY_PFILE	000001
#define SEARCH_BY_NAME	000002
#define SEARCH_BY_KEY	000004
#define SEARCH_BY_PATH	000010
#define SEARCH_BY_INFO	000020
#define SEARCH_BY_MAINT	000040
#define SEARCH_BY_CAT	000100
#define SEARCH_BY_FDEP	000200
#define SEARCH_BY_EDEP	000400
#define SEARCH_BY_PDEP	001000
#define SEARCH_BY_BDEP	002000
#define SEARCH_BY_RDEP	004000
#define SEARCH_BY_DEP	010000
#define SEARCH_BY_WWW	020000

#define DFLT_OUTFLDS	"name,path,info,maint,bdep,rdep,www"

struct options_t {
	const char	*portsdir;
	int		update_db;
	int		verbose;
	int		search_crit;
	const char	*search_file;
	const char	*search_name;
	const char	*search_key;
	const char	*search_path;
	const char	*search_info;
	const char	*search_maint;
	const char	*search_cat;
	const char	*search_fdep;
	const char	*search_edep;
	const char	*search_pdep;
	const char	*search_bdep;
	const char	*search_rdep;
	const char	*search_dep;
	const char	*search_www;
	int		icase;
	const char	*outflds;
};

#endif  /* PORTSEARCH_H */

/* EOF */

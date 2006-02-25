/*
 * Copyright (c) 2005-2006, Dan Ponte
 *
 * fakedbfs.h - main header
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the author nor the names of his contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/**
 * @file fakedbfs.h
 * @brief Main header file.
 */
/* $Amigan: fakedbfs/include/fakedbfs/fakedbfs.h,v 1.74 2006/02/25 09:52:13 dcp1990 Exp $ */
#include <fakedbfs/types.h>

#include <fakedbfs/fdbfsconfig.h>
#ifndef _SQLITE3_H_
#include <sqlite3.h>
#endif
#include <fakedbfs/conf.h>
#ifndef HAVE_DBSPECDATA_H
#include <fakedbfs/dbspecdata.h>
#endif
#ifndef HAVE_QUERY_H
#include <fakedbfs/query.h>
#endif
#if defined(DMALLOC) && !defined(NODMALLOC)
#include "dmalloc.h"
#endif

#ifdef HAVE_FICL_H
#include <ficl.h>
#endif

#define ERR(act, fmt, ...) fdbfs_ferr(f, act, fmt, __VA_ARGS__)
#define SERR(act, fmt) fdbfs_ferr(f, act, fmt)
#define CERR(act, fmt, ...) fdbfs_cferr(f, act, fmt, __VA_ARGS__)
#define SCERR(act, fmt) fdbfs_cferr(f, act, fmt)
#define _unused       __attribute__((__unused__))

#define MAJOR_API_VERSION 2
#define MINOR_API_VERSION 0

#define FAKEDBFSVER "2.0.0" /* major changes with major incompat changes, minor with minor incompat, micro with additions, bugfixes, and security fixes */
#define VERNAME "Doctor Wu" /* this changes for each release */
#define FAKEDBFSMAJOR	2
#define FAKEDBFSMINOR	0
#define FAKEDBFSMICRO	0


#ifndef lint
#define RCSID(str) static const char _cvsid[] __unused = str;
#else
#define RCSID(str) ;
#endif

#define DELIMCHAR "|"
#define FDBFSDIR ".fdbfs"
#define FDBFSPLUGENV "FDBFSPLUGPATH"

#ifdef lint
/* LINTLIBRARY */
#endif

#if !defined(ISLEX) && defined(FREEDEBUG) && !defined(DMALLOC)
#undef free
#define free(x)		printf("fr %p (%s:%d)\n", x, __FILE__, __LINE__); free(x)
#endif



#define FIELDS_FLAG_MMAPED	0x1	/* set if we need to do an mmap (only for stuff that supports this! */
#define FIELDS_FLAG_LASTDEF	0x2	/* used in the indexer to keep track of what it can and cannot free */




struct PluginInfo {
	const char *extensions; /* a list of file extensions, not including dots, that
				this plugin handles. Each extension is separated
				by a slash character. (since it cannot occur in a filename)
				Note that this isn't required, but if a file's extension is in
				the list, the plugin will be given priority,
				thereby speeding up searches (check_file() is
				always called, however).
			  */
	const char *pluginname; /* the name of the plugin; self-explanatory */
	const char *version; /* the version of the plugin */
	const char *author; /* author (usually name and email address) */
	const char *website; /* website of plugin */
	const int majapi; /* major API version; this changes when incompatible changes
			are made to the API. This should always be set to the
			define MAJOR_API_VERSION.
		    */
	const int minapi; /* minor API version; this changes for backwards-compatible
			changes. This should be set to MINOR_API_VERSION.
		    */
}; 	/*
	note that this structure will stay fairly consistent, at least for the
	first elements listed here. Another words: plugin_inf.majapi should
	always refer to the same object, even across major API changes.
	*/


#define DEBUGFUNC_STDERR ((void(*)(char*, enum ErrorAction))0)
#define AFFPROTO (answer_t * /* buffer */, answer_t * /* default */, char * /*fieldname*/, char * /* unformatted name */, \
			char * /* filename */, enum DataType, struct EnumHead * /* if oenum */, struct EnumSubElem * /* if sub */)
#define ASKFUNC_STD ((answer_t*(*)AFFPROTO)0)

union _ansdata {
	char *string; /* this and vd will be free()d if they aren't NULL, no exceptions. Hence, make them dynamic. */
	int integer; /* applies to enums and subenums as well */
	long long linteger; /* long integer */
	FLOATTYPE fp;
	void *vd;
};

struct _answer {
	enum DataType dt;
	ansdata_t ad;
	size_t len;
};

struct _fdbfs {
	char *dbname;
	sqlite3 *db;
	error_t error;
	config_t conf;
	query_t *curq; /* current query; must be reset on each exec/step */
	struct Plugin *plugins;
	void (*debugfunc)(char*, enum ErrorAction);
	answer_t *(*askfieldfunc) AFFPROTO; /* returns status: 0 means no change, 1 means change, -1 means error */
	Heads heads;
	ficlSystem *fsys;
	ficlstate_t fst;
	confnode_t *rconf;
};

/* crawler stuff */
struct _file_id {
#if defined(UNIX)
	int filenum;
	int devnum;
#endif
	char *filename;
};

struct _crawl {
	crawlframe_t *topframe;
	crawlframe_t *curframe;
	fdbfs_t *f;
	int maxlevels;
	int mlbefdep; /* max levels before we just start doing a depth traversal rather than a breadth */
};

struct DirState {
#if defined(UNIX) && defined(HAVE_DIR_H)
	DIR *dir;
#endif
};

struct _crawlframe {
	crawlframe_t **stack; /* this is stack of POINTERS to crawlframes, not of crawlframes themselves! */
	crawlframe_t **sp; /* *sp is the object, sp is what we deal with */
	crawlframe_t **stop;
	int maxelements;
	int cindex;
	file_id_t oid;
	struct DirState ds;
	int level;
	crawlframe_t *parent;
	crawl_t *fajah;
};

struct _dspdata {
	fdbfs_t *f;
	char *yytext;
	int error;
	struct CatalogueHead *cat;
	fields_t *fhead;
	fields_t *lastf;
	fields_t *cf;
};

#define CRAWL_ERROR	0x0 /* error; curframe null? */
#define CRAWL_FILE	0x1 /* found a file; act upon it */
#define CRAWL_DIR	0x2 /* found a directory; rerun crawl_go() to find the next entry */
#define CRAWL_FINISHED	0x3 /* we've hit the bottom; give up */

/* flags */
#define CRAWL_DIE_ON_ERROR	0x1


/* end crawler */

#ifdef NO_CALLOC
void* allocz(size_t size);
#else
#define allocz(x)	calloc(1, x)
#endif

/**
 * @brief Set askfieldfunction.
 *
 * fdbfs_set_aff() sets the askfieldfunction used in indexing.
 * @param f The instance of fakedbfs on which to operate.
 * @param AFFPROTO The new askfieldfunc.
 */
void fdbfs_set_aff(fdbfs_t *f, answer_t *(*aff)AFFPROTO);

/**
 * @brief Create a new instance of fakedbfs.
 *
 * fdbfs_new() creates a new instance of fakedbfs. 
 * @param[in] dbfile The filename of the database.
 * @param[out] error On error, *error will be set. *error must be freed.
 * @param[in] debugf The function to use as the debug function.
 * @param[in] useplugins Whether to enable the plugin, configuration, and ficl subsystems.
 * @return A new fdbfs object.
 */
fdbfs_t *fdbfs_new(char *dbfile, char **error, void (*debugf)(char*, enum ErrorAction), int useplugins);

/**
 * @brief Destroy a fdbfs instance
 *
 * fdbfs_destroy() destroys a previously-created instance of fakedbfs.
 * @param f The instance to destroy.
 * @return Non-zero on success.
 */
int fdbfs_destroy(fdbfs_t *f);

/**
 * @brief Read dbspecs from DB.
 *
 * fdbfs_read_specs_from_db() reads all the dbspecs from the database and stores them in the instance of fakedbfs specified.
 * @param f The instance of fakedbfs on which to operate.
 * @return Non-zero on success, zero on error.
 */
int fdbfs_read_specs_from_db(fdbfs_t *f);

/* misc string functions */

/**
 * @brief Normalise string.
 *
 * @param s String to normalise.
 * @return Copy (from strdup()) of the string, normalised. Free when done.
 */
char* fdbfs_normalise(char *s);

/**
 * @brief Compute number of base10 digits for given unsigned integer.
 *
 * @param n Number to examine.
 * @return Number of digits.
 */
size_t fdbfs_number_size(unsigned int n);

/**
 * @brief Compute number of base10 digits for given signed integer.
 *
 * @param n Number to examine.
 * @return Number of digits.
 */
size_t fdbfs_signed_size(int n);

answer_t* fdbfs_askfunc_std AFFPROTO;

/**
 * @brief Duplicate string within quotes
 *
 * Returns a malloc()'d string containing whatever was inside the quotes of the operand.
 * @param s String to copy from.
 * @return Malloc'd string, NULL on error.
 */
char* fdbfs_strdupq(char *s);

/**
 * @brief Same thing as libc strdup().
 *
 * @param str String to duplicate.
 * @return Malloc'd string, NULL on error.
 */
char* fdbfs_fstrdup(const char *str);

/* crawl stuff -- will move to own file when we can*/
crawl_t* fdbfs_crawler_new(fdbfs_t *f, int mlevels, int mlbd);
void fdbfs_crawler_destroy(crawl_t *cr);
int fdbfs_crawl_dir(crawl_t *cr, char *dir); /* simply adds dir to the base frame */



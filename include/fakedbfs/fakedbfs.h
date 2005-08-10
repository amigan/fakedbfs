/*
 * Copyright (c) 2005, Dan Ponte
 *
 * fakedbfs.h - fakedbfs library header
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
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
/* $Amigan: fakedbfs/include/fakedbfs/fakedbfs.h,v 1.4 2005/08/10 03:28:00 dcp1990 Exp $ */
#ifndef _SQLITE3_H_
#include <sqlite3.h>
#endif
#ifndef HAVE_DBSPECDATA_H
#include <dbspecdata.h>
#endif
#define ERR(act, fmt, ...) ferr(f, act, fmt, __VA_ARGS__)
#define CERR(act, fmt, ...) cferr(f, act, fmt, __VA_ARGS__)
#define _unused       __attribute__((__unused__))

#define FAKEDBFSVER "1.0"

#ifndef lint
#define RCSID(str) static const char _cvsid[] __unused = str;
#else
#define RCSID(str) ;
#endif

#ifdef lint
/* LINTLIBRARY */
#endif

enum ErrorAction {
	die,
	error,
	warning,
	cont,
	harmless
};

typedef struct {
	char *emsg;
	unsigned short freeit;
	enum ErrorAction action;
} error_t;

typedef struct fdbfs_instance {
	char *dbname;
	sqlite3 *db;
	error_t error;
} fdbfs_t;
typedef enum coltype {
	text,
	real,
	integer,
	blob
} coltype_t;
typedef struct Field {
	char *fieldname;
	enum DataType type;
	void *val;
	size_t len;
	struct Field *next;
} fields_t;
typedef struct tok {
	char *str;
	int num;
	unsigned int unum;
	struct EnumElem *enumelem;
	struct CatElem *catelem;
	struct EnumSubElem *subelem;
	struct EnumHead *ehead;
} Toke;


void* allocz(size_t size);
char* normalise(char *s);
struct EnumElem* find_elem_by_name(struct EnumElem *h, char *name);
struct EnumHead* find_enumhead_by_name(struct EnumHead *h, char *name);
struct CatalogueHead* find_cathead_by_name(struct CatalogueHead *h, char *name);
struct EnumSubElem* copy_sub_list(
		struct EnumSubElem *from, 
		struct EnumSubElem *to,
		struct EnumElem *fajah,
		int *lastval
		);
struct EnumSubElem* free_enum_sub_elem(struct EnumSubElem *e, short int allsub); /* returns next */
void free_enum_sub_elem_list(struct EnumSubElem *head, short int allsub);
struct EnumElem* free_enum_elem(struct EnumElem *e);
void free_enum_elem_list(struct EnumElem *head);
struct EnumHead* free_enum_head(struct EnumHead *e);
void free_enum_head_list(struct EnumHead *head);
struct CatElem* free_cat_elem(struct CatElem *e);
void free_cat_elem_list(struct CatElem *head);
struct CatalogueHead* free_cat_head(struct CatalogueHead *e);
void free_cat_head_list(struct CatalogueHead *head);
void free_head_members(Heads *hd);
const char* gettype(enum DataType t);
int create_table(fdbfs_t *f, char *tname, char *tspec);
int add_to_enum_list_table(fdbfs_t *f, char *name, char *tname, char *specf);
int add_to_cat_list_table(fdbfs_t *f, char *name, char *alias, char *tablename,
		char *fieldtable, char *specf);
int add_to_field_desc(fdbfs_t *f, char *tablename, char *name, char *alias, enum DataType
		type, char *typen);
int add_enum_elem(fdbfs_t *f, char *tname, char *name, char *fmtname, unsigned int value,
		enum DataType dtype, char *subelements);
int ferr(fdbfs_t *f, enum ErrorAction severity, char *fmt, ...);
int cferr(fdbfs_t *f, enum ErrorAction severity, char *fmt, ...);
int bind_field(fdbfs_t *f, int *count, enum DataType type, void *value, size_t len, sqlite3_stmt *stmt);
int table_exists(fdbfs_t *f, char *tname);
int open_db(fdbfs_t *f);
int close_db(fdbfs_t *f);
struct EnumSubElem* subelements_from_field(fdbfs_t *f, struct EnumElem *fajah, char *subs);
struct EnumElem* enumelems_from_dbtab(fdbfs_t *f, char *table);
struct EnumHead* enums_from_db(fdbfs_t *f);
struct CatElem* catelems_from_dbtab(fdbfs_t *f, char *table, struct EnumHead *enumhead);
struct CatalogueHead* cats_from_db(fdbfs_t *f, struct EnumHead *enumhead);
int make_tables_from_spec(fdbfs_t *f, char *sfile, Heads *h);

/* application interfaces */
int parse_definition(fdbfs_t *f, char *filename);
int start_db(fdbfs_t *f);
fdbfs_t *new_fdbfs(char *dbfile, char **error);
int destroy_fdbfs(fdbfs_t *f);
void estr_free(error_t *e);

/*
 * Copyright (c) 2005-2006, Dan Ponte
 *
 * types.h - typedefs
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
 * @file types.h
 * @brief typedefs.
 */
#ifndef HAVE_FDBFSTYPES_H
#define HAVE_FDBFSTYPES_H

#ifndef _SQLITE3_H_
#include <sqlite3.h> /* for sqlite3 type */
#endif

#define FLOATTYPE double

typedef enum _coltype coltype_t;
typedef enum DataType datatype_t;
typedef struct _error error_t;
typedef struct _fields fields_t;
typedef struct _Toke Toke;
typedef struct _config config_t;
typedef struct _confnode confnode_t;
typedef struct _answer answer_t;
typedef struct _fdbfs fdbfs_t;
typedef struct _file_id file_id_t;
typedef struct _crawl crawl_t;
typedef struct _crawlframe crawlframe_t;
typedef struct _dspdata dspdata_t;
typedef struct _freg freg_t;
typedef struct _fregmatch fregmatch_t;
typedef struct _ficlstate ficlstate_t;
typedef struct _ficlplug ficlplug_t;
typedef struct _db db_t;
typedef union _ansdata ansdata_t;
typedef union Data data_t;

enum _coltype {
	text,
	real,
	integer,
	blob
};

enum DataType {
	number = 1, /* 0 might be used for some error condition later in the DB */
	boolean,
	string,
	fp,
	image,
	binary,
	oenum, /* DO NOT USE FOR "OTHER" IN AN ENUM */
	oenumsub, /* this either; this is for .sub references */
	usnumber, /* new type for unsigned numbers */
	datime, /* date and time; see lastupdate (this is really like number, but treated differently for display purposes */
	character, /* for use with conf only */
	bigint, /* 64 bits */
	usbigint
};

struct _db {
	sqlite3 *db;
};

struct _fields {
	char *fieldname;
	char *fmtname;
	enum DataType type;
	enum DataType othtype;
	void *val;
	void *otherval;
	struct EnumHead *ehead;
	struct EnumSubElem *subhead;
	size_t len;
	size_t othlen;
	int flags;
	struct _fields *subparent; /* if type == oenumsub, this points to another field_t that is where the subhead will be determined. */
	struct _fields *next;
};

struct _Toke {
	char *str;
	int num;
	unsigned int unum;
	FLOATTYPE *flt; /* hack because even though it could be passed by value, we don't have a FLOATTYPE operand (O4) in the VM */
	struct EnumElem *enumelem;
	struct CatElem *catelem;
	struct EnumSubElem *subelem;
	struct EnumHead *ehead;
};


union Data {
	int integer;
	long long linteger;
	unsigned long long uslinteger;
	unsigned int usint;
	char *string;
	char character;
	struct {
		void *ptr;
		size_t len;
	} pointer;
	FLOATTYPE fp;
};

enum ErrorAction {
	die,
	error,
	warning,
	cont,
	harmless
};

struct _error {
	char *emsg;
	unsigned short freeit;
	enum ErrorAction action;
};


struct _ficlstate {
	fdbfs_t *f;
	short do_outp;
};


#endif

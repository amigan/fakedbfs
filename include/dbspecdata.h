/*
 * Copyright (c) 2005, Dan Ponte
 *
 * dbspecdata.h - data structures used for db spec building
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
/* $Amigan: fakedbfs/include/dbspecdata.h,v 1.7 2005/11/27 02:51:26 dcp1990 Exp $ */
#ifndef HAVE_DBSPECDATA_H
#define HAVE_DBSPECDATA_H 1
#define MAXLINE 2048
#define ALLSUBSTART 6500 /* defining more than this number-1 of subelements leads to
			  * undefined behaviour. avoid doing so (not like you will anyway)
			  */
typedef struct {
	struct CatalogueHead *cathead;
	struct EnumHead *enumhead;
	struct CatalogueHead *lastcath;
	struct EnumHead *lastenumh;
	struct CatalogueHead *curcath;
	struct EnumHead *curenumh;
	/* from DB (for collision detection) */
	struct CatalogueHead *db_cath;
	struct EnumHead *db_enumh;
	/* elements */
	struct CatElem *catelemhead;
	struct EnumElem *enumelemhead;
	struct CatElem *lastcatel;
	struct EnumElem *lastenumel;
	struct CatElem *curcatel;
	struct EnumElem *curenumel;
	/* subelements */
	struct EnumSubElem *subelhead;
	struct EnumSubElem *lastsubel;
	/* allsubs */
	struct EnumSubElem *allsubelhead;
	struct EnumSubElem *lastallsubel;
	/* counters */
	unsigned int lastevalue;
	unsigned int lastsubval;
	unsigned int lastallsubval; /* should start at a high number, such as 6500 */
	int err;
	void *instance;
} Heads;
/*
 * note that any fmtname could also point to that structure's name;
 * one must check if the fc (First letter Capitalize) flag is set and
 * act accordingly. This is to save some memory. (only applies to stuff with
 * alias capability, not ENUM block names)
 * If alias == name with no fc set, this means that no alias was defined.
 */
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
	datime /* date and time; see lastupdate (this is really like number, but treated differently for display
	purposes */
};

/* Enum stuff */

#if 0
this isn't really necessary...
struct EnumSubHead {
	struct EnumElem *father;
	struct EnumSubElem *headsub;
	short int hasSelf;
};
#endif

/* NB: *ALL* name/alias-type fields (basically anything not a const char*) *MUST* be allocated dynamically (or at the very least located
 * in writeable memory, but this will fuck up free_() functions), or strange segfaults will result.
 */
#define SUBE_IS_SELF 0x1
#define SUBE_IS_SAMEAS 0x2
#define SUBE_IS_ALLSUB 0x4
struct EnumSubElem {
	char *name;
	/* Not necessary: char *fmtname; */
	unsigned int value;
	struct EnumElem *father;
	int flags; /* bit 1 is "is self" 2 means "don't free strings" */
	struct EnumSubElem *next;
};

struct EnumElem {
	char *name;
	char *fmtname;
	unsigned int value;
	short int other;
	enum DataType othertype; /* CANNOT BE another enum */
	struct EnumSubElem *subhead;
	struct EnumElem *next;
};
#define ENUMH_FROM_DB 0x1 /* if this flag is set, DO NOT write back out to DB; it is for collision detection (reading from DB.
			     For more info, see dbinit.c's enums_from_db(). */
struct EnumHead {
	char *name;
	int flags;
	struct EnumSubElem *allsubs;
	struct EnumElem *headelem;
	struct EnumElem *otherelem;
	struct EnumHead *next;
};

/* catalogue stuff */
#define CATE_USES_FC 0x1
struct CatElem {
	char *name;
	char *alias; /* analogous to fmtname... */
	enum DataType type;
	struct EnumHead *enumptr;
	struct CatElem *subcatel; /* only set if type == oenumsub */
	int flags; /* bit 1 is "uses <fc> param" */
	struct CatElem *next;
};
#define CATH_USES_FC 0x1
#define CATH_FROM_DB 0x2 /* if this flag is set, DO NOT write back out to DB; it is for collision detection (reading from DB.
			     For more info, see dbinit.c's cats_from_db(). */
struct CatalogueHead {
	char *name;
	char *fmtname;
	int flags; /* just like elem as of now */
	struct CatElem *headelem;
	struct CatalogueHead *next; /* God only knows if we will use this... */
};
#define CAT_TABLE_PREFIX "c_"
#define CAT_FIELD_TABLE_PREFIX "cft_"
#define ENUM_TABLE_PREFIX "endef_"
#define OTHER_ELEM_PREFIX "oth_" /* do not prefix anything else with this */
#endif

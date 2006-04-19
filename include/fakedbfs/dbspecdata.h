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
/* $Amigan: fakedbfs/include/fakedbfs/dbspecdata.h,v 1.11 2006/04/19 19:58:22 dcp1990 Exp $ */
#ifndef HAVE_DBSPECDATA_H
#define HAVE_DBSPECDATA_H 1
#include <fakedbfs/types.h>
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

/**
 * @brief Find enum element by name.
 *
 * @param h List head to search.
 * @param name Name to search for.
 * @return The element we found, or NULL on no match/error.
 */
struct EnumElem* fdbfs_find_elem_by_name(struct EnumElem *h, const char *name);

/**
 * @brief Find catalogue element by enumhead.
 *
 * @param h List head to search.
 * @param en Enumhead to search for.
 * @return The element we found, or NULL on no match/error.
 */
struct CatElem* fdbfs_find_catelem_enum(struct CatElem *h, struct EnumHead *en);

/**
 * @brief Search enum list and return name of element with specified value.
 *
 * @param h Head of list.
 * @param val Value to search for.
 * @param fmted 1 if formatted name is desired. 0 otherwise.
 * @return Name of enum element, or NULL if none found.
 */
char* fdbfs_get_enum_string_by_value(struct EnumElem *h, unsigned int val, short int fmted);

/**
 * @brief Get subelement head by value in enum.
 *
 * @param h List head to search.
 * @param val Value to search for.
 * @return Sub element list, or NULL if no match.
 */
struct EnumSubElem* fdbfs_get_subhead_by_enval(struct EnumElem *h, unsigned int val);

/**
 * @brief Get string in list of enum subelements by value.
 *
 * @param h Sub element list head to search.
 * @param val Value to search for.
 * @return Name of subelement with specified value, or NULL otherwise.
 */
char* fdbfs_get_enum_sub_string_by_value(struct EnumSubElem *h, unsigned int val);

/**
 * @brief Search list of catalogue heads for a certain name.
 *
 * @param h Head of list to search.
 * @param name Name to search for.
 * @return The element if found, NULL otherwise.
 */
struct CatalogueHead* fdbfs_find_cathead_by_name(struct CatalogueHead *h, const char *name);

/**
 * @brief Search for catalogue element by name.
 *
 * @param h Head of list to search.
 * @param name Name to search for.
 * @return The element if found, NULL otherwise.
 */
struct CatElem* fdbfs_find_catelem_by_name(struct CatElem *h, const char *name);

/**
 * @brief Search for enum head by name.
 *
 * @param h Head of list to search.
 * @param name Name to search for.
 * @return The element if found, NULL otherwise.
 */
struct EnumHead* fdbfs_find_enumhead_by_name(struct EnumHead *h, const char *name);

/**
 * @brief Copy subelement list.
 *
 * For use with stuff like the sameas directive. Copies list of subelements.
 * @param[in] from From list.
 * @param[out] to To list. This must point to an allocated struct EnumSubElem; subsequent items will be allocated.
 * @param[in] fajah Father enum element of new list.
 * @param[in,out] lastval Pointer to integer specifying the value to use. Will be increased to last value when done.
 * @return Tail element of copied list.
 */
struct EnumSubElem* fdbfs_copy_sub_list(
		struct EnumSubElem *from,
		struct EnumSubElem *to,
		struct EnumElem *fajah,
		int *lastval
		);

/**
 * @brief Construct EnumElem list from database enum description table.
 *
 * DOes so according to TABLE_FORMAT.
 * @param f Instance of fakedbfs.
 * @param table Table to read.
 * @param e Parent of all elements.
 * @returns List of EnumElems, or NULL on error.
 */
struct EnumElem* fdbfs_enumelems_from_dbtab(fdbfs_t *f, const char *table, struct EnumHead *e);

/**
 * @brief Construct EnumHead list from database.
 *
 * @param f Instance of fakedbfs.
 * @return EnumHead list, or NULL on error.
 */
struct EnumHead* fdbfs_enums_from_db(fdbfs_t *f);

/**
 * @brief Construct CatElem list from catalogue description table.
 *
 * @param f Instance of fakedbfs.
 * @param table Name of catalogue description table.
 * @param enumhead Head of enums to search when enum element is encountered.
 * @return CatElem list, or NULL on error.
 */
struct CatElem* fdbfs_catelems_from_dbtab(fdbfs_t *f, const char *table, struct EnumHead *enumhead);

/**
 * @brief Construct CatalogueHead list from database.
 *
 * @param f Instance of fakedbfs.
 * @param enumhead EnumHead list to search.
 * @return CatalogueHead list, or NULL on error.
 */
struct CatalogueHead* fdbfs_cats_from_db(fdbfs_t *f, struct EnumHead *enumhead);

/**
 * @brief Free an EnumSubElem object.
 *
 * @param e Object to free.
 * @param allsub 1 if intentionally freeing allsubs, 0 to ignore allsubs.
 * @return Next element in list, NULL on error.
 */
struct EnumSubElem* fdbfs_free_enum_sub_elem(struct EnumSubElem *e, short int allsub); /* returns next */

/**
 * @brief Free list of EnumSubElems.
 *
 * @param head Head of list.
 * @param allsub 1 if intentionally freeing allsubs, 0 to ignore allsubs.
 */
void fdbfs_free_enum_sub_elem_list(struct EnumSubElem *head, short int allsub);

/**
 * @brief Free EnumElem object.
 *
 * @param e Object to free.
 * @return Next element in list, NULL on error.
 */
struct EnumElem* fdbfs_free_enum_elem(struct EnumElem *e);

/**
 * @brief Free list of EnumElems.
 *
 * @param head List to traverse and free.
 */
void fdbfs_free_enum_elem_list(struct EnumElem *head);

/**
 * @brief Free EnumHead object.
 *
 * @param e Object to free.
 * @return Next element in list, NULL on error.
 */
struct EnumHead* fdbfs_free_enum_head(struct EnumHead *e);

/**
 * @brief Free list of EnumHeads.
 *
 * @param head List to traverse and free.
 */
void fdbfs_free_enum_head_list(struct EnumHead *head);

/**
 * @brief Free CatElem object.
 *
 * @param e Object to free.
 * @return Next element, NULL on error.
 */
struct CatElem* fdbfs_free_cat_elem(struct CatElem *e);

/**
 * @brief Free list of CatElems.
 *
 * @param head List to traverse and free.
 */
void fdbfs_free_cat_elem_list(struct CatElem *head);

/**
 * @brief Free CatHead object.
 *
 * @param e Object to free.
 * @return Next element, NULL on error.
 */
struct CatalogueHead* fdbfs_free_cat_head(struct CatalogueHead *e);

/**
 * @brief Free list of CatHeads.
 *
 * @param head List to traverse and free.
 */
void fdbfs_free_cat_head_list(struct CatalogueHead *head);

/**
 * @brief Free certain members of a Heads object.
 *
 * fdbfs_free_head_members() frees the lists specified by both hd->cathead and hd->enumhead.
 * @param hd Heads object to free from.
 */
void fdbfs_free_head_members(Heads *hd);

/**
 * @brief Parse dbspec.
 *
 * Parses the specified specfile and modifies the database accordingly.
 * Must close and reopen before using other parts of the library to see changes.
 * @param f Instance of fakedbfs.
 * @param filename Specfile filename to read.
 * @return 0 on error.
 */
int fdbfs_dbspec_parse(fdbfs_t *f, const char *filename);

/**
 * @brief Free list of actcat_t.
 *
 * @param h Head of list to free.
 */
void fdbfs_actcats_free(actcat_t *h);

/**
 * @brief Make actcat_t list from database.
 *
 * @param f Instance of fakedbfs.
 * @param cathead Catalogue head to search for definitions.
 * @return Pointer to list of actcat_t elements, or NULL on error.
 */
actcat_t* fdbfs_catalogues_from_db(fdbfs_t *f, struct CatalogueHead *cathead);

#define CAT_TABLE_PREFIX "c_"
#define CAT_FIELD_TABLE_PREFIX "cft_"
#define ENUM_TABLE_PREFIX "endef_"
#define OTHER_ELEM_PREFIX "oth_" /* do not prefix anything else with this */
#endif

/*
 * Copyright (c) 2005-2006, Dan Ponte
 *
 * memory.c - allocation and free functions
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
/* $Amigan: fakedbfs/libfakedbfs/memory.c,v 1.27 2006/01/31 17:26:26 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#ifdef UNIX
#include <dlfcn.h> /* for dlclose */
#include <sys/mman.h>
#endif
#include "dbspec.h"
/* us */
#include <fakedbfs/dbspecdata.h>
#include <fakedbfs/lexdefines.h>
#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/plugins.h>
#include <fakedbfs/fields.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/memory.c,v 1.27 2006/01/31 17:26:26 dcp1990 Exp $")


#ifdef NO_CALLOC
void* allocz(size)
	size_t size;
{
	void *p = malloc(size);
	memset(p, 0, size);
#ifdef AZ_DEBUG
	printf("p is %p\n", p);
#endif
	return p;
}
#endif

struct EnumSubElem* fdbfs_free_enum_sub_elem(e, allsub) /* returns next */
	struct EnumSubElem *e;
	short int allsub; /* are we intentionally freeing allsubs? */
{
	struct EnumSubElem *nx;
#ifdef FREEDEBUG
	printf("se %p %s a %s p %s\n", e, e->name, e->flags & SUBE_IS_SAMEAS ? "sameas" : "", e->flags & SUBE_IS_SELF ? "self" : "");
#endif
	if((e->flags & SUBE_IS_ALLSUB) && !allsub) {
#ifdef FREEDEBUG
		printf("is allsub, no can do\n");
#endif
		return NULL;
	}
#ifdef FREEDEBUG
	else if(allsub) {
		printf("yay, we can free it\n");
	}
#endif
	if(!(e->flags & SUBE_IS_SAMEAS) && e->name != NULL) {
		free(e->name);
		e->name = NULL; /* maybe we need this */
	}
	nx = e->next;
	free(e);
	return nx;
}

void fdbfs_free_enum_sub_elem_list(head, allsub)
	struct EnumSubElem *head;
	short int allsub;
{
	struct EnumSubElem *c = head;

	while (c != NULL) {
	    c = fdbfs_free_enum_sub_elem(c, allsub);
	}
}

struct EnumElem* fdbfs_free_enum_elem(e)
	struct EnumElem *e;
{
	struct EnumElem *nx;
	/* if(e->name == e->fmtname) {
		free(e->name);
	} else { */
		free(e->name);
		free(e->fmtname);
	/* } */
	nx = e->next;
	fdbfs_free_enum_sub_elem_list(e->subhead, 0);
	free(e);
	return nx;
}

void fdbfs_free_enum_elem_list(head)
	struct EnumElem *head;
{
	struct EnumElem *c = head;
	while (c != NULL) {
		c = fdbfs_free_enum_elem(c);
	}
}

struct EnumHead* fdbfs_free_enum_head(e)
	struct EnumHead *e;
{
	struct EnumHead *nx;
	free(e->name);
	fdbfs_free_enum_elem_list(e->headelem);
	fdbfs_free_enum_sub_elem_list(e->allsubs, 1);
	nx = e->next;
	free(e);
	return nx;
}

void fdbfs_free_enum_head_list(head)
	struct EnumHead *head;
{
	struct EnumHead *c = head;
	
	while (c != NULL) {
		c = fdbfs_free_enum_head(c);
	}
}

/* catalogues */

struct CatElem* fdbfs_free_cat_elem(e)
	struct CatElem *e;
{
	struct CatElem *nx;
	if(e->name == e->alias) {
		free(e->name);
	} else {
		free(e->name);
		free(e->alias);
	}
	nx = e->next;
	free(e);
	return nx;
}

void fdbfs_free_cat_elem_list(head)
	struct CatElem *head;
{
	struct CatElem *c = head;

	while (c != NULL) {
		c = fdbfs_free_cat_elem(c);
	}
}

struct CatalogueHead* fdbfs_free_cat_head(e)
	struct CatalogueHead *e;
{
	struct CatalogueHead *nx;
	if(e->name == e->fmtname) {
		free(e->name);
	} else {
		free(e->name);
		free(e->fmtname);
	}
	fdbfs_free_cat_elem_list(e->headelem);
	nx = e->next;
	free(e);
	return nx;
}

void fdbfs_free_cat_head_list(head)
	struct CatalogueHead *head;
{
	struct CatalogueHead *c = head;

	while (c != NULL) {
		c = fdbfs_free_cat_head(c);
	}
}

void fdbfs_free_head_members(hd) /* only the heads contained within, not the structure itself */
	Heads *hd;
{
	fdbfs_free_cat_head_list(hd->cathead);
	fdbfs_free_enum_head_list(hd->enumhead);
}

static struct Plugin* destroy_plugin(e)
	struct Plugin *e;
{
	struct Plugin *nx;

#ifdef UNIX
	dlclose(e->pl.libhandle);
#endif
	nx = e->next;
	free(e);
	
	return nx;
}

void fdbfs_free_plugin_list(h)
	struct Plugin *h;
{
	struct Plugin *c = h;
	while (c != NULL) {
		c = destroy_plugin(c);
	}
}

void fdbfs_estr_free(e)
	error_t *e;
{
	if(e->emsg == NULL) return;
	if(e->freeit) free(e->emsg);
	e->emsg = NULL;
}

fields_t* fdbfs_free_field(e)
	fields_t *e;
{
	fields_t *nx;
	
	if(e->fieldname != NULL)
		free(e->fieldname);
	if(e->fmtname != NULL)
		free(e->fmtname);
	if(e->val != NULL && !(e->flags & FIELDS_FLAG_MMAPED))
		free(e->val);
#ifdef HAVE_MMAP
	if(e->flags & FIELDS_FLAG_MMAPED)
		munmap(e->val, e->len);
#endif
	if(e->otherval != NULL)
		free(e->otherval);
	nx = e->next;
	free(e);
	return nx;
}

void fdbfs_free_field_list(h)
	fields_t *h;
{
	fields_t *c = h;

	while (c != NULL) {
		c = fdbfs_free_field(c);
	}
}

char* fdbfs_strdupq(s)
	char *s;
{
	/* I am indebted to FreeBSD's /usr/src/lib/libc/string/strdup.c */
        size_t len;
        char *copy;
	char *str = s;
	char *c;

	if(*str == '"')
		str++;

	c = strrchr(str, '"');
        len = strlen(str) + (c != NULL ? 0 : 1);
        if ((copy = malloc(len)) == NULL)
                return (NULL);
        memcpy(copy, str, len);
	*(copy + --len) = '\0'; /* null thingy since it's memcpy() */
        return (copy); 
}

void fdbfs_free_answer_t(e)
	answer_t *e;
{
	if(e->ad.string != NULL)
		free(e->ad.string);
	else if(e->ad.vd != NULL)
		free(e->ad.vd);
	free(e);
}

char *fdbfs_fstrdup(str)
	const char *str;
{
	return strdup(str);
}

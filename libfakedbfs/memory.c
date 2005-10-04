/*
 * Copyright (c) 2005, Dan Ponte
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
/* $Amigan: fakedbfs/libfakedbfs/memory.c,v 1.20 2005/10/04 17:04:03 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#ifdef UNIX
#include <dlfcn.h> /* for dlclose */
#endif
#include "dbspec.h"
/* us */
#include <dbspecdata.h>
#include <lexdefines.h>
#include <fakedbfs.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/memory.c,v 1.20 2005/10/04 17:04:03 dcp1990 Exp $")


#ifdef NO_CALLOC
void* allocz(size)
	size_t size;
{
	void *p = malloc(size);
	memset(p, 0, size);
	return p;
}
#endif

struct EnumSubElem* free_enum_sub_elem(e, allsub) /* returns next */
	struct EnumSubElem *e;
	short int allsub; /* are we intentionally freeing allsubs? */
{
	struct EnumSubElem *nx;
#ifdef FREEDEBUG
	printf("se %p %s a %s p %s\n", e, e->name, e->flags & SUBE_IS_SAMEAS ? "sameas" : "", e->flags & SUBE_IS_SELF ? "self" : "");
#endif
	if((e->flags & SUBE_IS_ALLSUB) && !allsub) {
		return NULL;
	}
	if(!(e->flags & SUBE_IS_SAMEAS) && e->name != NULL) {
		free(e->name);
		e->name = NULL; /* maybe we need this */
	}
	nx = e->next;
	free(e);
	return nx;
}

void free_enum_sub_elem_list(head, allsub)
	struct EnumSubElem *head;
	short int allsub;
{
	struct EnumSubElem *c = head;

	while (c != NULL) {
	    c = free_enum_sub_elem(c, allsub);
	}
}

struct EnumElem* free_enum_elem(e)
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
	free_enum_sub_elem_list(e->subhead, 0);
	free(e);
	return nx;
}

void free_enum_elem_list(head)
	struct EnumElem *head;
{
	struct EnumElem *c = head;
	while (c != NULL) {
		c = free_enum_elem(c);
	}
}

struct EnumHead* free_enum_head(e)
	struct EnumHead *e;
{
	struct EnumHead *nx;
	free(e->name);
	free_enum_elem_list(e->headelem);
	free_enum_sub_elem_list(e->allsubs, 1);
	nx = e->next;
	free(e);
	return nx;
}

void free_enum_head_list(head)
	struct EnumHead *head;
{
	struct EnumHead *c = head;
	
	while (c != NULL) {
		c = free_enum_head(c);
	}
}

/* catalogues */

struct CatElem* free_cat_elem(e)
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

void free_cat_elem_list(head)
	struct CatElem *head;
{
	struct CatElem *c = head;

	while (c != NULL) {
		c = free_cat_elem(c);
	}
}

struct CatalogueHead* free_cat_head(e)
	struct CatalogueHead *e;
{
	struct CatalogueHead *nx;
	if(e->name == e->fmtname) {
		free(e->name);
	} else {
		free(e->name);
		free(e->fmtname);
	}
	free_cat_elem_list(e->headelem);
	nx = e->next;
	free(e);
	return nx;
}

void free_cat_head_list(head)
	struct CatalogueHead *head;
{
	struct CatalogueHead *c = head;

	while (c != NULL) {
		c = free_cat_head(c);
	}
}

void free_head_members(hd) /* only the heads contained within, not the structure itself */
	Heads *hd;
{
	free_cat_head_list(hd->cathead);
	free_enum_head_list(hd->enumhead);
}

struct Plugin* destroy_plugin(e)
	struct Plugin *e;
{
	struct Plugin *nx;

#ifdef UNIX
	dlclose(e->libhandle);
#endif
	nx = e->next;
	free(e);
	
	return nx;
}

void destroy_plugin_list(h)
	struct Plugin *h;
{
	struct Plugin *c = h;
	while (c != NULL) {
		c = destroy_plugin(c);
	}
}

void estr_free(e)
	error_t *e;
{
	if(e->emsg == NULL) return;
	if(e->freeit) free(e->emsg);
	e->emsg = NULL;
}

fields_t* free_field(e)
	fields_t *e;
{
	fields_t *nx;
	
	if(e->fieldname != NULL)
		free(e->fieldname);
	if(e->fmtname != NULL)
		free(e->fmtname);
	if(e->val != NULL)
		free(e->val);
	if(e->otherval != NULL)
		free(e->otherval);
	nx = e->next;
	free(e);
	return nx;
}

void free_field_list(h)
	fields_t *h;
{
	fields_t *c = h;

	while (c != NULL) {
		c = free_field(c);
	}
}

char* strdupq(s)
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

void free_answer_t(e)
	answer_t *e;
{
	if(e->string != NULL)
		free(e->string);
	if(e->vd != NULL)
		free(e->vd);
	free(e);
}

char *fstrdup(str)
	const char *str;
{
	return strdup(str);
}

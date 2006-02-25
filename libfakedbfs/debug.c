/*
 * Copyright (c) 2005-2006, Dan Ponte
 *
 * debug.c - debugging stuff (list dumps, etc)
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
/* $Amigan: fakedbfs/libfakedbfs/debug.c,v 1.5 2006/02/25 07:12:31 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include "dbspec.h"
/* us */
#include <fakedbfs/dbspecdata.h>
#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/debug.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/debug.c,v 1.5 2006/02/25 07:12:31 dcp1990 Exp $")

struct EnumSubElem* fdbfs_debug_dump_enum_sub_elem(e, allsub) /* returns next */
	struct EnumSubElem *e;
	short int allsub; /* are we intentionally freeing allsubs? */
{
	struct EnumSubElem *nx;
	nx = e->next;
	printf("subelem %p '%s'. value == %d. father == %p. flags == %x.%s\n", e, e->name,
			e->value, e->father, e->flags, allsub ? " allsub." : "");
	return nx;
}

void fdbfs_debug_dump_enum_sub_elem_list(head, allsub)
	struct EnumSubElem *head;
	short int allsub;
{
	struct EnumSubElem *c, *next;
	for(c = head; c != NULL; c = next) {
		next = fdbfs_debug_dump_enum_sub_elem(c, allsub);
	}
}

struct EnumElem* fdbfs_debug_dump_enum_elem(e)
	struct EnumElem *e;
{
	struct EnumElem *nx;
	nx = e->next;
	printf("enumelem '%s'. fmtname '%s'. value == %d. other == %d. otype == %d.\n",
			e->name, e->fmtname, e->value, e->other, e->othertype);
	fdbfs_debug_dump_enum_sub_elem_list(e->subhead, 0);
	return nx;
}

void fdbfs_debug_dump_enum_elem_list(head)
	struct EnumElem *head;
{
	struct EnumElem *c, *next;
	for(c = head; c != NULL; c = next) {
		next = fdbfs_debug_dump_enum_elem(c);
	}
}

struct EnumHead* fdbfs_debug_dump_enum_head(e)
	struct EnumHead *e;
{
	struct EnumHead *nx;
	printf("enumhead '%s'. flags == %x. allsubs == %p.\n",
			e->name, e->flags, e->allsubs);
	fdbfs_debug_dump_enum_sub_elem_list(e->allsubs, 1);
	fdbfs_debug_dump_enum_elem_list(e->headelem);
	nx = e->next;
	return nx;
}

void fdbfs_debug_dump_enum_head_list(head)
	struct EnumHead *head;
{
	struct EnumHead *c, *next;
	for(c = head; c != NULL; c = next) {
		next = fdbfs_debug_dump_enum_head(c);
	}
}

/* catalogues */

struct CatElem* fdbfs_debug_dump_cat_elem(e)
	struct CatElem *e;
{
	struct CatElem *nx;
	printf("catelem '%s'. alias '%s'. type == %d. ehead == %p. flags = %d. n %c= al.\n",
			e->name, e->alias, e->type, e->enumptr, e->flags,
			e->name == e->alias ? '=' : '!');
	nx = e->next;
	return nx;
}

void fdbfs_debug_dump_cat_elem_list(head)
	struct CatElem *head;
{
	struct CatElem *c, *next;
	for(c = head; c != NULL; c = next) {
		next = fdbfs_debug_dump_cat_elem(c);
	}
}

struct CatalogueHead* fdbfs_debug_dump_cat_head(e)
	struct CatalogueHead *e;
{
	struct CatalogueHead *nx;
	printf("cathead '%s'. fmtname '%s'. flags == %x. headel == %p. n %c= fmtn.\n",
			e->name, e->fmtname, e->flags, e->headelem, e->name == e->fmtname ?
			'=' : '!');
	fdbfs_debug_dump_cat_elem_list(e->headelem);
	nx = e->next;
	return nx;
}

void fdbfs_debug_dump_cat_head_list(head)
	struct CatalogueHead *head;
{
	struct CatalogueHead *c, *next;
	for(c = head; c != NULL; c = next) {
		next = fdbfs_debug_dump_cat_head(c);
	}
}

void fdbfs_debug_dump_head_members(hd) /* only the heads contained within, not the structure itself */
	Heads *hd;
{
	fdbfs_debug_dump_cat_head_list(hd->cathead);
	fdbfs_debug_dump_enum_head_list(hd->enumhead);
}

void fdbfs_debug_dump_fields(h)
	fields_t *h;
{
	fields_t *c;
	for(c = h; c != NULL; c = c->next) {
		printf("%s (%s) = ", c->fmtname, c->fieldname);
		switch(c->type) {
			case string:
				printf("\"%s\"", (char*)c->val);
				break;
			case number:
				printf("%d (%x)", *(int*)c->val, *(int*)c->val);
				break;
			case boolean:
				printf("%s", *(int*)c->val ? "true" : "false");
				break;
			case fp:
				printf("%g", *(FLOATTYPE*)c->val);
				break;
			case oenum:
				printf("%s (%d)", fdbfs_get_enum_string_by_value(c->ehead->headelem, *(int*)c->val, 1), *(int*)c->val);
				break;
			/* TODO: handle oenumsub */
			default:
				break;
		}
		printf("\n");
	}

	printf("\n");
}

void fdbfs_debug_dump_confnode(c)
	confnode_t *c;
{
	for(; c != NULL; c = c->next) {
		printf("n %s data %p %s children:\n", c->tag, c->data.string, c->flags & CN_FLAG_LEAF ? "(leaf)"  : "");
		fdbfs_debug_dump_confnode(c->child);
		printf("--%s end--\n", c->tag);
	}
}



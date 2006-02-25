/*
 * Copyright (c) 2005-2006, Dan Ponte
 *
 * conf.c - fakedbfs configuration subsystem
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
/* $Amigan: fakedbfs/libfakedbfs/conf.c,v 1.16 2006/02/25 06:42:15 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <float.h>
#include <ctype.h>

/* other libraries */
#include <sqlite3.h>

#include <fakedbfs/fdbfsconfig.h>
#include <fakedbfs/conf.h>
#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/db.h>
#include <fakedbfs/debug.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/conf.c,v 1.16 2006/02/25 06:42:15 dcp1990 Exp $")

static void conf_node_link_parent(parent, n)
	confnode_t *parent, *n;
{
	if(parent->child == NULL) {
		parent->child = n;
		n->next = NULL;
	} else {
		/* this is a search routine, it's slow:
			for(c = parent->child; c->next != NULL; c = c->next) ;
		*/
		n->next = parent->child;
		parent->child = n;
	}
}

confnode_t* fdbfs_conf_node_create(tag, parent, leaf)
	char *tag;
	confnode_t *parent;
	int leaf;
{
	confnode_t *n;

	n = allocz(sizeof(*n));
	n->tag = strdup(tag);

	if(parent != NULL) {
		conf_node_link_parent(parent, n);
	}

	if(leaf)
		n->flags |= CN_FLAG_LEAF;

	return n;
}

/* add function to check if all the MIBs we need are in the DB and add them
 * if we must... */
static int conf_init_db(f)
	fdbfs_t *f;
{
	int rc;
	union Data dta;

	if(!fdbfs_db_table_exists(f, CONFTABLE)) {
		rc = fdbfs_db_create_table(f, CONFTABLE, CONFTABLESPEC);
		if(!rc)
			return 0;
	}

	dta.integer = 1;

	if(!fdbfs_db_mib_add(f, ROOT_NODE_TAG ".plugins.forthpri", boolean, dta))
		return 0;

	return 1;
}

static confnode_t* conf_create_branch(tag)
	char *tag;
{
	confnode_t *n;

	n = allocz(sizeof(*n));

	n->tag = strdup(tag);

	return n;
}

static confnode_t* conf_search_tag(parent, name)
	confnode_t *parent;
	char *name;
{
	confnode_t *c;

	for(c = parent->child; c != NULL; c = c->next) {
		if(strcmp(c->tag, name) == 0) {
			return c;
		}
	}

	return NULL;
}

static confnode_t* conf_search_mib(p, mib)
	confnode_t *p;
	char *mib; /* not const! */
{
	confnode_t *c = p;
	char *ln;
	char *oln;
	short lastelem = 0;

	mib = strdup(mib);
	ln = mib;

	while(!lastelem) {
		oln = ln;
		ln = strchr(ln, '.');

		if(!ln)
			lastelem = 1;
		else {
			*ln = '\0';
		}
		c = conf_search_tag(c, oln);
		
		if(c == NULL) {
			free(mib);
			return NULL;
		}

		if(!lastelem) {
			*ln = '.';
			ln++;
		}
	}

	free(mib);

	return c;
}

static confnode_t* conf_search_create_branch(parent, name)
	confnode_t *parent;
	char *name;
{
	confnode_t *c;

	for(c = parent->child; c != NULL; c = c->next) {
		if(strcmp(c->tag, name) == 0) {
			return c;
		}
	}

	c = conf_create_branch(name);
	conf_node_link_parent(parent, c);

	return c;
}

static int conf_connect_to_tree(t, mib, n)
	confnode_t *t; /* parent */
	char *mib;
	confnode_t *n;
{
	confnode_t *c = t;
	char *ln = mib;
	char *oln;

	while(1) {
		oln = ln;
		ln = strchr(ln, '.');

		if(!ln) {
			break;
		}

		if(oln[1] == '\0')
			break;

		*ln = '\0';
		c = conf_search_create_branch(c, oln);
		*ln = '.';
		ln++;
	}

	conf_node_link_parent(c, n);

	return 1;
}

static int conf_add_to_tree(f, mib, type, data, dynamic)
	fdbfs_t *f;
	char *mib;
	enum DataType type;
	union Data *data;
	short dynamic;
{
	confnode_t *n;
	char *lastnode;
	int rc;

	lastnode = strrchr(mib, '.');
	if(lastnode == NULL)
		return 0;

	if(strlen(++lastnode) < 1)
		return 0;

	n = allocz(sizeof(*n));
	n->tag = strdup(lastnode);
	n->flags |= CN_FLAG_LEAF;

	if(dynamic == 1)
		n->flags |= CN_DYNA_DATA;
	else if(dynamic == 2)
		n->flags |= CN_DYNA_STR;

	n->data = *data;
	n->type = type;

	/* TODO: walk the tree filling in any non-leaf nodes as needed, and connect this mib to its proper place. */
	*--lastnode = '\0'; /* just to help */
	rc = conf_connect_to_tree(f->rconf, mib, n);
	*lastnode = '.';
	return rc;
}

static void conf_destroy_data(c)
	confnode_t *c;
{
	if(c->flags & CN_FLAG_LEAF) {
		if(c->flags & CN_DYNA_DATA)
			free(c->data.pointer.ptr);
		else if(c->flags & CN_DYNA_STR)
			free(c->data.string);
	}
}

void fdbfs_conf_destroy_tree(t)
	confnode_t *t;
{
	confnode_t *c = t, *nx;

	while(c != NULL) {
		nx = c->next;
		fdbfs_conf_destroy_tree(c->child);
		if(c->tag != NULL)
			free(c->tag);

		conf_destroy_data(c);

		free(c);
		c = nx;
	}
}

/*
 * This routine is really slow, because in order to attach out new node to the tree,
 * the attachment routine needs to search the entire tree. Unless we use childlast, which we do.
 */
int fdbfs_conf_read_from_db(f)
	fdbfs_t *f;
{
	sqlite3_stmt *cst;
	const char *sql = "SELECT mib,type,value FROM " CONFTABLE;
	const char *tail;
	enum DataType type;
	char *mib;
	union Data data;
	short dynamic;
	int rc;

	if((rc = sqlite3_prepare(f->db, sql, strlen(sql), &cst, &tail)) != SQLITE_OK) {
		return ERR(die, "conf_read_from_db: sqlite %s", sqlite3_errmsg(f->db));
	}

	while((rc = sqlite3_step(cst)) == SQLITE_ROW) {
		dynamic = 0;
		mib = strdup(sqlite3_column_text(cst, 0));
		type = sqlite3_column_int(cst, 1);
		switch(type) {
			case number:
			case boolean:
			case usnumber:
			case character:
				data.integer = sqlite3_column_int(cst, 2);
				break;
			case datime:
			case bigint:
			case usbigint:
				data.linteger = sqlite3_column_int64(cst, 2);
				break;
			case string:
				data.string = strdup(sqlite3_column_text(cst, 2));
				dynamic = 2;
				break;
			case fp:
				data.fp = sqlite3_column_double(cst, 2);
				break;
			case binary:
				data.pointer.len = sqlite3_column_bytes(cst, 2);
				data.pointer.ptr = malloc(data.pointer.len);
				memcpy(data.pointer.ptr, sqlite3_column_blob(cst, 2), data.pointer.len);
				dynamic = 1;
				break;
			default:
				sqlite3_finalize(cst);
				return ERR(die, "conf_read_from_db: unsupported type %x", type);
		}

		conf_add_to_tree(f, mib, type, &data, dynamic);
		free(mib);
	}

	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		ERR(die, "conf_read_from_db: step error: %s", sqlite3_errmsg(f->db));
		sqlite3_finalize(cst);
		return 0;
	}

	sqlite3_finalize(cst);

	return 1;
}

static void dump_confnode(c)
	confnode_t *c;
{
	for(; c != NULL; c = c->next) {
		printf("n %s data %p %s children:\n", c->tag, c->data.string, c->flags & CN_FLAG_LEAF ? "(leaf)"  : "");
		dump_confnode(c->child);
		printf("--%s end--\n", c->tag);
	}
}

enum DataType fdbfs_conf_get(f, mib, data)
	fdbfs_t *f;
	char *mib;
	union Data *data; /* output */
{
	char *mibcpy = strdup(mib);
	confnode_t *tnode;

	tnode = conf_search_mib(f->rconf, mibcpy);
	free(mibcpy);

	if(tnode == NULL) {
		return -1;
	}

	/* otherwise we found it; copy the data elem (note that this is only valid until we destroy fdbfs_t */

	*data = tnode->data;

	return tnode->type;
}

int fdbfs_conf_set(f, mib, type, data) /* this routine is dedicated to Genesis' song "In The Beginning", to which it was written */
	fdbfs_t *f;
	char *mib;
	enum DataType type;
	union Data data; /* if applicable, this must persist for us to free according to our whims. */
{
	char *mibcpy = strdup(mib);
	confnode_t *tnode;
	short dynamic = 0;

	tnode = conf_search_mib(f->rconf, mibcpy);

	switch(type) {
		case string:
			dynamic = 2;
			break;
		case binary:
			dynamic = 1;
			break;
		default:
			dynamic = 0;
			break;
	}

	if(tnode == NULL) {
		conf_add_to_tree(f, mibcpy, type, &data, dynamic);
		if(!fdbfs_db_mib_add(f, mibcpy, type, data)) {
			free(mibcpy);
			return 0;
		}
	} else {
		conf_destroy_data(tnode);
		tnode->data = data;
		tnode->type = type;
		if(!fdbfs_db_mib_update(f, mibcpy, type, data)) {
			free(mibcpy);
			return 0;
		}
	}

	free(mibcpy);

	return 1;
}

int fdbfs_conf_init(f)
	fdbfs_t *f;
{
	f->rconf = allocz(sizeof(confnode_t));
	f->rconf->flags |= CN_FLAG_ROOT;

	if(!conf_init_db(f))
		return 0;

	if(!fdbfs_conf_read_from_db(f))
		return 0;

	return 1;
}

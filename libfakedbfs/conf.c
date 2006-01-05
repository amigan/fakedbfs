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
/* $Amigan: fakedbfs/libfakedbfs/conf.c,v 1.5 2006/01/05 13:40:43 dcp1990 Exp $ */
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

#include <fdbfsconfig.h>

#include <fakedbfs.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/conf.c,v 1.5 2006/01/05 13:40:43 dcp1990 Exp $")

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

confnode_t* conf_node_create(tag, parent, leaf)
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

int conf_init_db(f)
	fdbfs_t *f;
{
	int rc;
	union Data dta;

	rc = create_table(f, CONFTABLE, CONFTABLESPEC);

	if(!rc)
		return 0;

	dta.integer = 1;

	if(!db_mib_add(f, ROOT_NODE_TAG ".plugins.forthpri", boolean, dta))
		return 0;

	return 1;
}

int conf_add_to_tree(f, mib, type, data, dynamic)
	fdbfs_t *f;
	char *mib;
	enum DataType type;
	union Data *data;
	short dynamic;
{
	confnode_t *n;
	char *lastnode;

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

	/* TODO: walk the tree filling in any non-leaf nodes as needed, and connect this mib to its proper place. */

	return 1;
}

/*
 * This routine is really slow, because in order to attach out new node to the tree,
 * the attachment routine needs to search the entire tree. Unless we use childlast, which we do.
 */
int conf_read_from_db(f)
	fdbfs_t *f;
{
	sqlite3_stmt *cst;
	const char *sql = "SELECT mib,type,value FROM " CONFTABLE;
	const char *tail;
	enum DataType type;
	const char *mib;
	union Data data;
	short dynamic;
	int rc;

	if((rc = sqlite3_prepare(f->db, sql, strlen(sql), &cst, &tail)) != SQLITE_OK) {
		return ERR(die, "conf_read_from_db: sqlite %s", sqlite3_errmsg(f->db));
	}

	while((rc = sqlite3_step(cst)) == SQLITE_ROW) {
		dynamic = 0;
		mib = sqlite3_column_text(cst, 0);
		type = sqlite3_column_int(cst, 1);
		switch(type) {
			case number:
			case boolean:
			case usnumber:
			case character:
				data.integer = sqlite3_column_int(cst, 2);
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
	}

	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		ERR(die, "conf_read_from_db: step error: %s", sqlite3_errmsg(f->db));
		sqlite3_finalize(cst);
		return 0;
	}

	sqlite3_finalize(cst);

	return 1;
}

int conf_init(f)
	fdbfs_t *f;
{
	f->rconf = conf_node_create(ROOT_NODE_TAG, NULL, 0);

	conf_read_from_db(f);

	return 1;
}

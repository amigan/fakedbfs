/*
 * Copyright (c) 2005-2006, Dan Ponte
 *
 * query.c - query code (namely the virtual machine)
 * See doc/QUERY_OPCODES for how all of this works.
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
 * @file query.c
 * @brief Query VM and related stuff.
 *
 * @sa query.h
 */
/* $Amigan: fakedbfs/libfakedbfs/query.c,v 1.42 2006/02/25 06:42:15 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <float.h>

/* other libraries */
#include <sqlite3.h>
/* us */
#include <fakedbfs/query.h>

#include <fakedbfs/fdbfsconfig.h>
#include "queryparser.h"
#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/fdbfsregex.h>
#include <fakedbfs/debug.h>

#ifdef UNIX
#	include <sys/mman.h>
#	include <fcntl.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#endif

RCSID("$Amigan: fakedbfs/libfakedbfs/query.c,v 1.42 2006/02/25 06:42:15 dcp1990 Exp $")


#define ParseTOKENTYPE Toke
#define ParseARG_PDECL ,query_t *q

void QParse(void *yyp, int yymajor, ParseTOKENTYPE yyminor ParseARG_PDECL);
void QParseTrace(FILE *TraceFILE, char *zTracePrompt);
void *QParseAlloc(void *(*mallocProc)(size_t));
void QParseFree(void *p, void (*freeProc)(void*));

/**
 * @brief Initialise the query stack.
 *
 * @param f Query to operate on.
 * @param size Number of elements to allocate.
 * @retval 1 Normal.
 */
static int init_stack(f, size)
	query_t *f;
	size_t size;
{
	f->stacksize = size;
	f->items = 0;
	f->stackbase = malloc(sizeof(operands_t) * size);
	f->csp = f->stackbase;
	f->top = f->stackbase + ((sizeof(operands_t) * size) - sizeof(operands_t));
	return 1;
}

/**
 * @brief Destroy stack.
 *
 * @param f Query to operate on.
 * @retval 1 Normal.
 */
static int destroy_stack(f)
	query_t *f;
{
	free(f->stackbase);
	f->csp = f->stackbase = f->top = NULL;
	f->stacksize = 0;
	f->items = 0;
	return 1;
}

int fdbfs_query_spush(q, o1, o2, o3, used)
	query_t *q;
	int o1;
	unsigned int o2;
	void *o3;
	int used;
{
	if(q->items >= q->stacksize)
		return 0;
	q->csp += sizeof(*q->csp);
	q->items++;
	q->csp->o1 = o1;
	q->csp->o2 = o2;
	q->csp->o3 = o3;
	q->csp->used = used;
	return 1;
}

int fdbfs_query_spop(q, bf)
	query_t *q;
	operands_t *bf;
{
	if(q->items <= 0)
		return 0;
	bf->o1 = q->csp->o1;
	bf->o2 = q->csp->o2;
	bf->o3 = q->csp->o3;
	bf->used = q->csp->used;
	q->csp -= sizeof(*q->csp);
	q->items--;
	return 1;
}

int fdbfs_query_pop1(q, o1)
	query_t *q;
	int *o1;
{
	operands_t c;
	int rc;

	rc = fdbfs_query_spop(q, &c);
	if(!rc)
		return rc;

	*o1 = c.o1;
	return rc;
}

int fdbfs_query_pop2(q, o2)
	query_t *q;
	unsigned int *o2;
{
	operands_t c;
	int rc;

	rc = fdbfs_query_spop(q, &c);
	if(!rc)
		return rc;

	*o2 = c.o2;
	return rc;
}

int fdbfs_query_pop3(q, o3)
	query_t *q;
	void **o3;
{
	operands_t c;
	int rc;

	rc = fdbfs_query_spop(q, &c);
	if(!rc)
		return rc;

	*o3 = c.o3;
	if(c.used & US_DYNA)
		rc = US_DYNA; /* free it please */
	return rc;
}

int fdbfs_query_push1(q, o1)
	query_t *q;
	int o1;
{
	return fdbfs_query_spush(q, o1, 0x0, NULL, USED_O1);
}

int fdbfs_query_push2(q, o2)
	query_t *q;
	unsigned int o2;
{
	return fdbfs_query_spush(q, 0x0, o2, NULL, USED_O2);
}

int fdbfs_query_push3(q, o3)
	query_t *q;
	void *o3;
{
	return fdbfs_query_spush(q, 0x0, 0x0, o3, USED_O3);
}

query_t* fdbfs_query_new(f, stacksize)
	fdbfs_t *f;
	size_t stacksize;
{
	query_t *n;
	n = allocz(sizeof(*n));
	n->f = f;
	
	if((n->enumh = f->heads.db_enumh) == NULL) {
		SCERR(die, "No definition in the fdbfs_t structure. Have you called fdbfs_read_specs_from_db()? ");
		free(n);
		return NULL;
	}
	
	if((n->cath = f->heads.db_cath) == NULL) {
		SCERR(die, "No definition in the fdbfs_t structure. Have you called fdbfs_read_specs_from_db()? ");
		free(n);
		return NULL;
	}

	init_stack(n, stacksize != 0 ? stacksize : DEFAULT_STACKSIZE);
	return n;
}

qreg_t* fdbfs_qreg_compile(regex, case_insens, errmsg)
	char *regex;
	int case_insens;
	char **errmsg;
{
	int rc;
	char emsg[128];
	qreg_t *new;

	new = allocz(sizeof(*new));
	new->tregex = strdup(regex);
	new->re = new_freg(emsg, sizeof(emsg));
	if(new->re == NULL) {
		*errmsg = strdup(emsg);
		free(new->tregex);
		free(new);
		return NULL;
	}
	if((rc = fregcomp(new->re, regex, (case_insens ? FREG_NOCASE : 0) | FREG_NOSUB)) != 0) {
		*errmsg = strdup(new->re->errmsg);
		destroy_freg(new->re);
		free(new->tregex);
		free(new);
		return NULL;
	}

	return new;
}

void fdbfs_db_regex_func(ctx, i, sqval)
	sqlite3_context *ctx;
	int i;
	sqlite3_value **sqval;
{
	fdbfs_t *f = (fdbfs_t*)sqlite3_user_data(ctx);
	inst_t *h = f->curq->insthead;
	inst_t *c;
	qreg_t *oqr;
	const char *regexp;
	const char *string;
	int rc, orc = 0;;

	regexp = sqlite3_value_text(sqval[0]);
	if(regexp == NULL) {
		sqlite3_result_int(ctx, 0);
		return;
	}

	string = sqlite3_value_text(sqval[1]);
	if(regexp == NULL) {
		sqlite3_result_int(ctx, 0);
		return;
	}

	for(c = h; c != NULL; c = c->next) {
		if(c->opcode == OP_REGEXP) {
			oqr = (qreg_t*)c->ops.o3;
			if(strcmp(regexp, oqr->tregex) == 0) {
				rc = fregexec(oqr->re, (char*)string, NULL, 0);
				if(rc == 0)
					orc = 1;
				break;
			}
		}
	}

	sqlite3_result_int(ctx, orc);
}

void fdbfs_qreg_destroy(q)
	qreg_t *q;
{
	destroy_freg(q->re);
	free(q->tregex);
	free(q);
}

/**
 * @brief Free instruction.
 *
 * @param e Instruction to free.
 */
static void free_inst(e)
	inst_t *e;
{
	if(e->opcode == OP_REGEXP)
		fdbfs_qreg_destroy(e->ops.o3);
	else if(e->ops.used & US_DYNA) {
		free(e->ops.o3);
	} else if(e->ops.used & US_FILE) {
#ifdef HAVE_MMAP
		munmap(e->ops.o3, e->ops.o2);
#endif
	}

	free(e);
	return;
}

void fdbfs_query_destroy(q)
	query_t *q;
{
	inst_t *c, *next;

	destroy_stack(q);

	for(c = q->insthead; c != NULL; c = next) {
		next = c->next;
		free_inst(c);
	}

	if(q->catalogue != NULL)
		free(q->catalogue);

	free(q);
}

int fdbfs_query_qi(q, opcode, op1, op2, op3, used)
	query_t *q;
	int opcode;
	int op1;
	unsigned int op2;
	void *op3;
	int used;
{
	inst_t *new = NULL;

	new = allocz(sizeof(*new));
	new->opcode = opcode;
	new->ops.o1 = op1;
	new->ops.o2 = op2;
	new->ops.o3 = op3;
	new->ops.used = used;
	
	if(q->insthead == NULL) {
		q->insthead = new;
		q->lastinst = new;
	} else {
		q->lastinst->next = new;
		new->last = q->lastinst;
		q->lastinst = new;
	}

	return 1;
}


/**
 * @brief Get the column name for the specified column index.
 *
 * Analogous to sqlite3_column_name(). For internal use only.
 * @param q Query to operate on.
 * @param ind Index of column.
 * @return Name of column.
 */
static const char* q_colname(q, ind)
	query_t *q;
	int ind;
{
	inst_t *c;
	int i = 0;

	for(c = q->insthead; c != NULL; c = c->next) {
		if(c->opcode == OP_SELCN && c->ops.used & USED_O3) {
		       if(i++ == ind) {
			       return (const char*)c->ops.o3;
		       }
		}
	}

	return NULL;
}

int fdbfs_query_step(q) /* a pointer to the head of a fields_t list is pushed to the stack by this. */
	query_t *q;
{
	unsigned int colcount = 0, i = 0;
	struct CatElem *cel = NULL;
	int rc, toret;
	short int special = 0;
	inst_t *c;
	char *pname;
	size_t len = 0;
	void *val = NULL;
	const void *tvd = NULL;
	const char *colname;
	fields_t *h = NULL, *cf = NULL, *n = NULL, *lastoth = NULL;

	if(q->exec_state == finished)
		return Q_FINISHED;
	if(q->exec_state == init && q->catalogue == NULL)
		return Q_STEP_ON_UNINIT;

	q->f->curq = q;


	rc = sqlite3_step(q->cst);
	if(rc == SQLITE_ROW) {
		q->exec_state = more;
		toret = Q_NEXT;
	} else if(rc == SQLITE_DONE || rc == SQLITE_OK) {
		toret = Q_FINISHED;
		q->exec_state = finished;
	} else {
		toret = Q_FDBFS_ERROR;
		q->exec_state = finished;
		fdbfs_ferr(q->f, die, "sqlite3_step(): %s", sqlite3_errmsg(q->f->db));
		sqlite3_finalize(q->cst); /* useless... */
		return toret;
	}
		

	if(!q->allcols)
		for(c = q->insthead; c != NULL; c = c->next) {
			if(c->opcode == OP_SELCN) {
				fdbfs_query_push3(q, c->ops.o3);
			}
		}

	colcount = sqlite3_column_count(q->cst);
#ifdef QUERY_DEBUG
	printf("counted %d columns...\n", colcount);
#endif

	for(i = 0; i < colcount; i++) {
		if(q->allcols) {
			colname = sqlite3_column_name(q->cst, i);
		} else {
			colname = q_colname(q, i);
		}
#ifdef QUERY_DEBUG
		printf("sqlite column name (index %d) came out to %p ('%s')\n", i, colname, colname != NULL ? colname : "");
#endif
		if(q->allcols) /* a terrible, terrible hack; why does sqlite3_column_name() return null?! check that the counter var is correct when q->allcols */
			if(strcmp("id", colname) == 0)
					continue;
		if(lastoth != NULL) {
			if(strncmp(colname, OTHER_ELEM_PREFIX, 4) == 0) {
				if(strcmp(colname + 4, lastoth->fieldname) == 0) {
#ifdef QUERY_DEBUG
					printf("other colname is the same as the last one!\n");
#endif
					n = lastoth;
					lastoth = NULL;
					special = 3;
				}
			}
		} else {
			n = allocz(sizeof(*n));
			n->fieldname = strdup(colname);
		}
		if(!q->allcols) {
			fdbfs_query_pop3(q, (void**)&pname); /* we're supposed to do something with this */
		}
		if(special != 3) {
			if(strcmp(n->fieldname, "file") == 0) {
				n->type = string;
				n->fmtname = strdup("Filename");
				special = 1;
			} else if(strcmp(n->fieldname, "lastupdate") == 0) {
				n->type = datime;
				n->fmtname = strdup("Last Updated");
				special = 2;
			} else if(strcmp(n->fieldname, "mime") == 0) {
				n->type = string;
				n->fmtname = strdup("MIME Type");
				special = 1;
			} else if(strcmp(n->fieldname, "ctime") == 0) {
				n->type = datime;
				n->fmtname = strdup("Created");
				special = 2;
			} else {
				cel = fdbfs_find_catelem_by_name(q->ourcat->headelem, n->fieldname);
				if(cel == NULL) {
#ifdef QUERY_DEBUG
					printf("No such element named %s\n", n->fieldname);
#endif
					toret = Q_NO_SUCH_CELEM;
					break;
				}
				n->fmtname = strdup(cel->alias);
				n->type = cel->type;
				switch(n->type) {
					case oenum:
						n->ehead = cel->enumptr;
#ifdef QUERY_DEBUG
						printf("otherelem for (%p - %s) %s is %p\n", n->ehead, n->ehead->name, n->fieldname, n->ehead->otherelem);
#endif
						if(n->ehead->otherelem != NULL) {
							n->othtype = n->ehead->otherelem->othertype;
							lastoth = n;
						} else
							n->othtype = 0;
						break;
					case oenumsub:
					default:
						break;
				}
				special = 0;
			}
		}
		switch(sqlite3_column_type(q->cst, i)) {
			case SQLITE_INTEGER: /* XXX: we need to be 64bit clean */
				if(n->type == datime || n->type == bigint) {
					val = malloc(sizeof(int64_t));
					*(int64_t*)val = sqlite3_column_int64(q->cst, i);
				} else {
					val = malloc(sizeof(int));
					*(int*)val = sqlite3_column_int(q->cst, i);
				}
				break;
			case SQLITE_FLOAT:
				val = malloc(sizeof(FLOATTYPE));
				*(FLOATTYPE*)val = sqlite3_column_double(q->cst, i);
				break;
			case SQLITE_TEXT:
				val = strdup(sqlite3_column_text(q->cst, i));
				len = strlen((char*)val);
				break;
			case SQLITE_BLOB:
				len = sqlite3_column_bytes(q->cst, i);
				tvd = sqlite3_column_blob(q->cst, i);
				val = malloc(len);
				memcpy(val, tvd, len);
				break;
			case SQLITE_NULL:
				val = NULL;
				break;
			default:
				val = NULL;
				len = 0;
				break;
		}

		if(val == NULL) {
			val = malloc(sizeof(unsigned int)); /* hopefully this will become n->val and be freed at a later date */
			*(unsigned int*)val = 0x0;
		}

		if(special == 0 && n->type == oenumsub) {
			/* we better hope that cel is what it should be */
			if(cel != NULL)
				n->subhead = fdbfs_get_subhead_by_enval(cel->subcatel->enumptr->headelem, *(unsigned int*)val);
		}

		if(special == 3) {
			n->otherval = val;
			n->othlen = len;
			special = 0;
		} else {
			n->val = val;
			n->len = len;
			if(h == NULL) {
				h = cf = n;
			} else {
				cf->next = n;
				cf = n;
				n->next = 0x0;
			}
		}
	}

	if(q->exec_state == finished) {
		sqlite3_finalize(q->cst);
	}

	if(toret == Q_NEXT || toret == Q_FINISHED) {
		if(!fdbfs_query_push3(q, h)) {
			toret = Q_STACK_FULL;
		} /* if the application doesn't know what it's doing, we're fucked once we run out of
		stack space! Make sure that anyone who uses querying knows this!! */
	}

	return toret;
}

int fdbfs_query_init_exec(q)
	query_t *q;
{
	inst_t *c;
	int opengrps = 0, ended = 0;
	int ec = 0;
	short int saw_operan = 0;
	short int saw_operat = 0, saw_not = 0;
	const char *tail;
	int indcounter = 1;
	short int foundselcn = 0;
	short int have_cond = 0;
	char *qusql;
#define SPBUFSIZE 64
	char spbuf[SPBUFSIZE];
	short int col_sel_init = 0;
	size_t query_len = sizeof("SELECT  FROM c_ WHERE ") + 7;

	q->f->curq = q;

	for(c = q->insthead; c != NULL; c = c->next) {
		if(ended)
			return Q_INST_AFTER_END;

		switch(c->opcode) {
			case OP_BEGIN_GRP:
				opengrps++;
				query_len++;
				have_cond = 1;
				break;
			case OP_CLOSE_GRP:
				if(opengrps <= 0)
					return Q_UNBALANCED_GROUP;
				else {
					opengrps--;
					query_len++;
				}
				break;
			case OP_SETCAT:
				if(!(c->ops.used & USED_O3) || c->ops.o3 == NULL)
					return Q_INVALID_O3;
				q->catalogue = strdup((char*)c->ops.o3);
				q->ourcat = fdbfs_find_cathead_by_name(q->cath, q->catalogue);
				if(q->ourcat == NULL)
					return Q_NO_SUCH_CAT;
				query_len += strlen(q->catalogue);
				break;
			case OPL_AND:
				query_len++;
			case OPL_OR:
				query_len += 4;
				break;
			case OPL_EQUAL:
			case OPL_NEQU:
			case OPL_GTHEQU:
			case OPL_LTHEQU:
				query_len += 2;
				break;
			case OPL_NOT:
				query_len++;
			case OPL_GTHAN:
			case OPL_LTHAN:
				query_len++;
				break;
			case OP_STRING:
				if(!(c->ops.used & USED_O3) || c->ops.o3 == NULL)
					return Q_MISSING_OPERAND;
				query_len++; /* like smart people */
#if 0
				query_len += 1 /* quote */ + strlen((char*)c->ops.o3) + 1 /* other quote */;
#endif
				have_cond = 1;
				break;
			case OP_COLNAME:
				if(!(c->ops.used & USED_O3) || c->ops.o3 == NULL)
					return Q_MISSING_OPERAND;
				query_len += strlen(c->ops.o3) + 1; /* like smart people */
				have_cond = 1;
				break;
			case OP_UINT:
				if(!(c->ops.used & USED_O2))
					return Q_MISSING_OPERAND;
				query_len += fdbfs_number_size(c->ops.o2); /* traditional because it's unsigned */
				have_cond = 1;
				break;
			case OP_INT:
				if(!(c->ops.used & USED_O1))
					return Q_MISSING_OPERAND;
				query_len++; /* used bind */
				have_cond = 1;
				break;
			case OP_NULL:
				query_len++; /* quest */
				break;
			case OP_VOID:
				if(!(c->ops.used & USED_O2))
					return Q_MISSING_OPERAND;
				/* FALLTHROUGH */
			case OP_FLOAT:
				if(!(c->ops.used & USED_O3) || c->ops.o3 == NULL)
					return Q_INVALID_O3;
				query_len++; /* we just use bind_*(), like smart people */
				break;
			case OP_PUSH:
			case OP_POP:
				/* do nothing */
				break;
			case OP_SELCN:
				if(!(c->ops.used & USED_O3))
					return Q_MISSING_OPERAND;
				if(q->allcols)
					break;
				if(c->ops.o3 == NULL)
					return Q_INVALID_O3;
				if(!col_sel_init) {
					query_len += 2 /* space */;
					col_sel_init = 1;
				}
				query_len += strlen(c->ops.o3) + 1 /* comma */;
				foundselcn = 1;
				break;
			case OP_REGEXP:
				if(!(c->ops.used & USED_O3))
					return Q_MISSING_OPERAND;
				if(c->ops.o3 == NULL)
					return Q_INVALID_O3;
				query_len += strlen(((qreg_t*)c->ops.o3)->tregex) + 4; /* spaces and quotes */
				break;
			case OP_ENDQ:
				ended = 1;
				break;
			case OP_BEGINQ:
				if(!(c->ops.used & USED_O1))
					return Q_MISSING_OPERAND;
				if(c->ops.o1)
					q->allcols = 0;
				else
					q->allcols = 1;
				break;
		}
	}
	

	if(!q->allcols && !foundselcn)
		return Q_NO_COLUMNS;
	if(opengrps > 0)
		return Q_UNBALANCED_GROUP;
	if(q->catalogue == NULL)
		return Q_CATALOGUE_NOT_SET;

	qusql = malloc(query_len);
	ended = 0; /* hacks...we like to conserve memory */
	strlcpy(qusql, "SELECT ", query_len);
	if(col_sel_init == 1)
		for(c = q->insthead; c != NULL; c = c->next) {
			if(c->opcode == OP_SELCN) {
				if(ended)
					strlcat(qusql, ",", query_len);
				else
					ended = 1;
				strlcat(qusql, (char*)c->ops.o3, query_len);
			}
		}
	else
		strlcat(qusql, "*", query_len);
	strlcat(qusql, " FROM c_", query_len);
	strlcat(qusql, q->catalogue, query_len);
	if(have_cond)
		strlcat(qusql, " WHERE ", query_len);
	ended = 0;

	for(c = q->insthead; c != NULL && ec == 0; c = c->next) {
		ec = 0;
		if(c->opcode != OPL_NOT) {
			saw_not = 0;
		}
		switch(c->opcode) {
			case OP_BEGIN_GRP:
				strlcat(qusql, "(", query_len);
				break;
			case OP_CLOSE_GRP:
				strlcat(qusql, ")", query_len);
				break;
			case OPL_AND:
				if(!saw_operan) {
					ec = Q_OPERATION_WITHOUT_OPERANDS;
					break;
				}
				saw_operat = 1;
				saw_operan = 0;
				strlcat(qusql, " AND ", query_len);
				break;
			case OPL_OR:
				if(!saw_operan) {
					ec = Q_OPERATION_WITHOUT_OPERANDS;
					break;
				}
				saw_operat = 1;
				saw_operan = 0;
				strlcat(qusql, " OR ", query_len);
				break;
			case OPL_NOT:
				if(saw_not) {
					ec = Q_DOUBLE_OPERAND;
					break;
				}
				saw_not = 1;
				strlcat(qusql, "! ", query_len);
				break;
			case OPL_EQUAL:
				if(!saw_operan) {
					ec = Q_OPERATION_WITHOUT_OPERANDS;
					break;
				}
				saw_operat = 1;
				saw_operan = 0;
				strlcat(qusql, "==", query_len);
				break;
			case OPL_NEQU:
				if(!saw_operan) {
					ec = Q_OPERATION_WITHOUT_OPERANDS;
					break;
				}
				saw_operat = 1;
				saw_operan = 0;
				strlcat(qusql, "!=", query_len);
				break;
			case OPL_GTHAN:
			case OPL_LTHAN:
				if(!saw_operan) {
					ec = Q_OPERATION_WITHOUT_OPERANDS;
					break;
				}
				saw_operat = 1;
				saw_operan = 0;
				strlcat(qusql, c->opcode == OPL_GTHAN ? ">" : "<",
						query_len);
				break;
			case OPL_GTHEQU:
			case OPL_LTHEQU:
				if(!saw_operan) {
					ec = Q_OPERATION_WITHOUT_OPERANDS;
					break;
				}
				saw_operat = 1;
				saw_operan = 0;
				strlcat(qusql, c->opcode == OPL_GTHEQU ? ">=" : "<=",
						query_len);
				break;
			case OP_UINT:
				if(saw_operan) {
					ec = Q_DOUBLE_OPERAND;
					break;
				}
				saw_operat = 0;
				saw_operan = 1;
				snprintf(spbuf, SPBUFSIZE, "%u", c->ops.o2);
				strlcat(qusql, spbuf, query_len);
				break;
			case OP_COLNAME:
				if(saw_operan) {
					ec = Q_DOUBLE_OPERAND;
					break;
				}
				saw_operat = 0;
				saw_operan = 1;
				strlcat(qusql, c->ops.o3, query_len);
				break;
			case OP_REGEXP:
				if(!saw_operan) {
					ec = Q_OPERATION_WITHOUT_OPERANDS;
					break;
				}
				saw_operat = saw_operan = 0; /* like =~ whatever */
				strlcat(qusql, " REGEXP '", query_len);
				strlcat(qusql,  ((qreg_t*)c->ops.o3)->tregex, query_len); /* this could be done more efficiently, such as passing
											     a special ID instead of an RE string, but oh well */
				strlcat(qusql, "'", query_len);
				break;
			case OP_INT:
			case OP_FLOAT:
			case OP_STRING:
			case OP_VOID:
			case OP_NULL:
				if(saw_operan) {
					ec = Q_DOUBLE_OPERAND;
					break;
				}
				saw_operat = 0;
				saw_operan = 1;
				strlcat(qusql, "?", query_len);
				break;
		}
	}

	if(ec) {
		free(qusql);
		return ec;
	}

#ifdef QUERY_DEBUG
	printf("Query SQL: '%s'\n", qusql);
#endif
	ec = sqlite3_prepare(q->f->db, qusql, strlen(qusql), &q->cst, &tail);
	if(ec != SQLITE_OK) {
		fdbfs_ferr(q->f, die, "prepare of \"%s\": %s", qusql, sqlite3_errmsg(q->f->db));
		free(qusql);
		return Q_FDBFS_ERROR;
	}

	free(qusql);
	

#ifdef QUERY_DEBUG
	printf("bind ");
#endif
	for(c = q->insthead; c != NULL; c = c->next) {
		ec = SQLITE_OK;
		switch(c->opcode) {
			case OP_INT:
#ifdef QUERY_DEBUG
				printf("int %d, ", c->ops.o1);
#endif
				ec = sqlite3_bind_int(q->cst, indcounter++, c->ops.o1);
				break;
			case OP_FLOAT:
#ifdef QUERY_DEBUG
				printf("float %f, ", *(FLOATTYPE*)c->ops.o3);
#endif
				ec = sqlite3_bind_double(q->cst, indcounter++, *(FLOATTYPE*)c->ops.o3);
				break;
			case OP_STRING:
#ifdef QUERY_DEBUG
				printf("str '%s', ", (char*)c->ops.o3);
#endif
				ec = sqlite3_bind_text(q->cst, indcounter++, (const char*)c->ops.o3, strlen((char*)c->ops.o3), SQLITE_STATIC);
				break;
			case OP_VOID:
#ifdef QUERY_DEBUG
				printf("void %p, ", c->ops.o3);
#endif
				ec = sqlite3_bind_blob(q->cst, indcounter++, c->ops.o3, c->ops.o2, SQLITE_STATIC);
				break;
			case OP_NULL:
#ifdef QUERY_DEBUG
				printf("null, ");
#endif
				ec = sqlite3_bind_null(q->cst, indcounter++);
				break;
		}
		if(ec != SQLITE_OK)
			break; /* so c stays the same */
	}

#ifdef QUERY_DEBUG
	printf(" --ebinds\n");
#endif

	if(ec != SQLITE_OK) {
		fdbfs_ferr(q->f, die, "bind of index %d (opcode %x): %s", indcounter, c->opcode, sqlite3_errmsg(q->f->db));
		return Q_FDBFS_ERROR;
	}
	
	return fdbfs_query_step(q);
}

int fdbfs_query_qne(q) /* Query Next/Execute */
	query_t *q;
{
	switch(q->exec_state) {
		case init:
			return fdbfs_query_init_exec(q);
			break;
		case more:
			return fdbfs_query_step(q);
			break;
		case finished:
			return Q_FINISHED;
	}

	return Q_UNKNOWNSTATE;
}

void* fdbfs_query_read_file(f, fn)
	fdbfs_t *f;
	char *fn;
{
	void *tbf = NULL;
#ifdef HAVE_MMAP
	int fd;
	int rc;
	struct stat sst;

	fd = open(fn, O_RDONLY);
	if(fd < 0) {
		ERR(die, "Error opening %s: %s", fn, strerror(errno));
		return NULL;
	}

	if((rc = fstat(fd, &sst)) == -1) {
		ERR(die, "Couldn't stat fd: %s", strerror(errno));
		close(fd);
		return NULL;
	}

	if((tbf = mmap(NULL, sst.st_size, PROT_READ, 0, fd, 0)) == MAP_FAILED) {
		ERR(die, "Couldn't mmap: %s", strerror(errno));
		close(fd);
		return NULL;
	}

	close(fd);

#else
	ERR(die, "Error opening %s: feature not implemented on this OS", fn);
	return NULL;
#endif
	return tbf;
}

int fdbfs_query_parse(q, qstr)
	query_t *q;
	char *qstr;
{
	char *cp = qstr;
	char **cptr = &cp;
	void *pa;
	int token;
	int trc;
	Toke to;
	char ctb[513];

	pa = QParseAlloc(malloc);

	memset(ctb, 0, sizeof(ctb));

	while((trc = fdbfs_query_qtok(cptr, &token, &to, ctb)) != 0) {
		if(token == SPACE)
			continue;
		if(trc == -2) {
			fdbfs_ferr(q->f, die, "NULL regexp in %s!", qstr);
			QParseFree(pa, free);
			return 0;
		}
		q->yytext = ctb;
		QParse(pa, token, to, q);
		if(q->f->error.emsg != NULL || q->error) {
			fdbfs_cferr(q->f, die, "Query parse of '%s'. ", qstr);
			QParseFree(pa, free);
			return 0;
		}
		memset(ctb, 0, sizeof(ctb));
	}
	QParse(pa, EOQUERY, to, q);
	QParse(pa, 0, to, q);
	QParseFree(pa, free);
	return 1;
}

const char* fdbfs_query_error(rc)
	int rc;
{
	switch(rc) {
		case Q_FINISHED:
			return "Query finished.";
		case Q_NEXT:
			return "More rows available.";
		case Q_INST_AFTER_END:
			return "Found instruction(s) after ENDQ opcode.";
		case Q_UNBALANCED_GROUP:
			return "Unbalanced grouping in query.";
		case Q_INVALID_O1:
			return "Invalid/missing operand (O1).";
		case Q_INVALID_O2:
			return "Invalid/missing operand (O2).";
		case Q_INVALID_O3:
			return "Invalid/missing operand (O3).";
		case Q_STEP_ON_UNINIT:
			return "Query stepped before initialisation.";
		case Q_MISSING_OPERAND:
			return "Missing operand.";
		case Q_CATALOGUE_NOT_SET:
			return "Catalogue name not set!";
		case Q_OPERATION_WITHOUT_OPERANDS:
			return "Math operation without operands.";
		case Q_DOUBLE_OPERAND:
			return "Math operation with double operands.";
		case Q_FDBFS_ERROR:
			return "FDBFS internal error. See f->error.emsg.";
		case Q_NO_COLUMNS:
			return "User-defined columns flag in BEGINQ set, but none defined!";
		case Q_NO_SUCH_CAT:
			return "No such catalogue.";
		case Q_NO_SUCH_CELEM:
			return "No such element/field in catalogue.";
		case Q_STACK_FULL:
			return "Stack full! Make sure you are popping the fields_t head off of the stack!";
		case Q_UNKNOWNSTATE:
			return "Query VM is in an unknown state!";
		default:
			return "Unknown error.";
	}
	/* NOTREACHED */
}

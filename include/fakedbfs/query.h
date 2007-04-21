/*
 * Copyright (c) 2005-2007, Dan Ponte
 *
 * query.h - query opcodes and such
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
/* $Amigan: fakedbfs/include/fakedbfs/query.h,v 1.21 2007/04/21 01:58:14 dcp1990 Exp $ */
#ifndef HAVE_QUERY_H
#define HAVE_QUERY_H 1
#ifndef HAVE_DBSPECDATA_H
#include <fakedbfs/dbspecdata.h>
#endif
#include <fakedbfs/fdbfsregex.h>

/* See doc/QUERY_OPCODES for how these are used */
#define OP_BEGINQ	0x1
#define OP_SETCAT	0x2
#define OP_BEGIN_GRP	0x3
#define OP_CLOSE_GRP	0x4
#define OPL_AND		0x5
#define OPL_OR		0x6
#define OPL_NOT		0x7
#define OPL_EQUAL	0x8
#define OP_COLNAME	0x9
#define OP_STRING	0xA
#define OP_INT		0xB
#define OP_UINT		0xC
#define OP_VOID		0xD
#define OP_FLOAT	0xE
#define OP_PUSH		0xF
#define OP_POP		0x10
#define OP_ENDQ		0x11
#define OP_SELCN	0x12
#define OP_REGEXP	0x13
#define OP_NULL		0x14
#define OPL_NEQU	0x15
#define OPL_GTHAN	0x16
#define OPL_LTHAN	0x17
#define OPL_GTHEQU	0x18
#define OPL_LTHEQU	0x19
#define OP_NOTREGEXP	0x1A
#define OP_SQL		0x1B

#define USED_O1		0x1
#define USED_O2		0x2
#define USED_O3		0x4
#define US_DYNA		0x8 /* free free(3) o3 if set */
#define US_FILE		0xF /* mmap or whatever */

#define Q_FINISHED 0x1
#define Q_NEXT 0x2
#define Q_INST_AFTER_END 0x3
#define Q_UNBALANCED_GROUP 0x4
#define Q_INVALID_O1 0x5
#define Q_INVALID_O2 0x6
#define Q_INVALID_O3 0x7
#define Q_STEP_ON_UNINIT 0x8
#define Q_MISSING_OPERAND 0x9
#define Q_CATALOGUE_NOT_SET 0xA
#define Q_OPERATION_WITHOUT_OPERANDS 0xB
#define Q_DOUBLE_OPERAND 0xC
#define Q_FDBFS_ERROR 0xD /* check q->f->error.emsg */
#define Q_NO_COLUMNS 0xE
#define Q_NO_SUCH_CAT 0xF
#define Q_NO_SUCH_CELEM 0x10
#define Q_STACK_FULL 0x11
#define Q_UNKNOWNSTATE 0xFF

typedef struct {
	int o1;
	unsigned int o2;
	void *o3;
	int used;
} operands_t;

typedef struct Inst {
	int opcode;
	operands_t ops;
	struct Inst *next;
	struct Inst *last;
} inst_t;

typedef struct Result {
	struct Field *fld;
	struct Result *next; /* for your convenience; none of libfakedbfs' functions use this */
} result_t;

enum estate {
	init = 0, /* not started yet */
	more, /* more rows waiting */
	finished /* done; reset state and set to init */
};

typedef struct {
	operands_t *stackbase, *top, *csp;
	size_t stacksize; /* int number of elements, not bytes */
	size_t items;
	fdbfs_t *f;
	inst_t *insthead;
	inst_t *lastinst;
	short int error;
	char *yytext;
	/* state info */
	short int allcols;
	char *catalogue;
	enum estate exec_state;
	sqlite3_stmt *cst;
	struct CatalogueHead *ourcat; /* our paticular one */
	struct CatalogueHead *cath;
	struct EnumHead *enumh;
} query_t;

typedef struct {
	freg_t *re;
	char *colname;
	char *tregex;
} qreg_t;

/**
 * @brief Create new query object.
 *
 * Creates a new query object with the specified stacksize.
 * @param f Instance of fakedbfs to link to.
 * @param stacksize If non-zero, number of elements to allocate for stack. Otherwise, DEFAULT_STACK_SIZE.
 * @return New query object, or NULL on error.
 */
query_t* fdbfs_query_new(fdbfs_t *f, size_t stacksize);

/**
 * @brief Destroy query.
 *
 * @param q Query to destroy.
 */
void fdbfs_query_destroy(query_t *q);

/**
 * @brief Queue instruction.
 *
 * fdbfs_query_qi() queues an instruction of the specified opcode and operands into the query object for later execution by the VM.
 * @param q Query to operate on.
 * @param opcode Opcode of the instruction.
 * @param op1 O1 of the instruction.
 * @param op2 O2 of the instruction.
 * @param op3 O3 of the instruction.
 * @param used Used flags.
 * @return Non-zero on success.
 */
int fdbfs_query_qi(query_t *q, int opcode, int op1, unsigned int op2, void *op3, int used);

/**
 * @brief Query next or execute.
 *
 * This is cleaner than calling fdbfs_query_init_exec() or fdbfs_query_step(). It decides what needs to be done
 * based on the execution state of the query. Use it.
 * @param q Query to control.
 * @retval Q_FINISHED Query finished executing; reset before running again.
 * @retval Q_UNKNOWNSTATE Something is royally fucked up. The VM is in a hopeless state of confusion.
 * @return Same as fdbfs_query_init_exec() and fdbfs_query_step().
 */
int fdbfs_query_qne(query_t *q);

/**
 * @brief Step currently-running query.
 *
 * Steps query, pushes fields_t of current result to stack.
 * @param q Query to step.
 * @return Various.
 * @sa query.h
 */
int fdbfs_query_step(query_t *q);

/**
 * @brief Initialise and execute a query.
 *
 * This must be called before the query can be stepped. This also steps the query once.
 * @param q Query to initialise and execute.
 * @return Various.
 */
int fdbfs_query_init_exec(query_t *q);


/**
 * @brief Push a cell to the query stack.
 *
 * @param q Query to operate on.
 * @param o1 Value of o1.
 * @param o2 Value of o2.
 * @param o3 Value of o3.
 * @param used Used flags for this cell.
 * @sa USED_O1
 * @sa USED_O2
 * @sa USED_O3
 * @sa US_DYNA
 * @sa US_FILE
 * @retval 1 Normal.
 * @retval 0 No more space on stack.
 */
int fdbfs_query_spush(query_t *q, int o1, unsigned int o2, void *o3, int used);

/**
 * @brief Pop cell off stack.
 *
 * @param q Query to operate on.
 * @param[out] bf Place to store popped data.
 * @retval 1 Normal.
 * @retval 0 Stack underflow.
 */
int fdbfs_query_spop(query_t *q, operands_t *bf);

/**
 * @brief Pop O1.
 *
 * @param q Query to operate on.
 * @param[out] o1 Place to store O1.
 * @retval 1 Normal.
 * @retval 0 Stack underflow.
 */
int fdbfs_query_pop1(query_t *q, int *o1);

/**
 * @brief Pop O2.
 *
 * @param q Query to operate on.
 * @param[out] o2 Place to store O2.
 * @retval 1 Normal.
 * @retval 0 Stack underflow.
 */
int fdbfs_query_pop2(query_t *q, unsigned int *o2);

/**
 * @brief Pop O3.
 *
 * @param q Query to operate on.
 * @param[out] o3 Place to store O3.
 * @retval 1 Normal.
 * @retval 0 Stack underflow.
 * @retval US_DYNA Free the data, please (free(o3)).
 */
int fdbfs_query_pop3(query_t *q, void **o3);

/**
 * @brief Push O1.
 *
 * @param q Query to operate on.
 * @param o1 Data to push.
 * @return Same as fdbfs_query_spush().
 */
int fdbfs_query_push1(query_t *q, int o1);

/**
 * @brief Push O2.
 *
 * @param q Query to operate on.
 * @param o2 Data to push.
 * @return Same as fdbfs_query_spush().
 */
int fdbfs_query_push2(query_t *q, unsigned int o2);

/**
 * @brief Push O3.
 *
 * @param q Query to operate on.
 * @param o3 Data to push.
 * @return Same as fdbfs_query_spush().
 */
int fdbfs_query_push3(query_t *q, void *o3); /* we could use macros for push*(), but oh well */

/**
 * @brief Create and compile new qreg object.
 *
 * @param regex Regex to compile.
 * @param case_insens 1 if case-insensitive matching will occur, 0 otherwise.
 * @param[out] errmsg Set to error message on error; must free().
 * @return New qreg object, NULL on error.
 */
qreg_t* fdbfs_qreg_compile(const char *regex, int case_insens, char **errmsg);

/**
 * @brief Destroy qreg object.
 *
 * @param q qreg to destroy.
 */
void fdbfs_qreg_destroy(qreg_t *q);

/**
 * @brief Read file.
 *
 * Read the specified file and return a pointer to (usually mmaped) data in it.
 * This isn't really used very much; also we must add support to get the length of data.
 * @param f Instance of fakedbfs to use.
 * @param fn Filename to read.
 * @return Pointer to data, or NULL on error.
 */
void* fdbfs_query_read_file(fdbfs_t *f, const char *fn);

/**
 * @brief Parse a query.
 *
 * Converts a query (as defined by the query grammar) into a usable instruction list for the VM.
 * Applications usually call this directly.
 * @param q Query to operate on.
 * @param qstr String to parse.
 * @return 0 on error.
 */
int fdbfs_query_parse(query_t *q, const char *qstr);

/**
 * @brief Tokeinse query
 *
 * @param[in,out] cp Pointer to string. Next token will be set here each call.
 * @param[out] tval Value of current token.
 * @param[out] toke Token data.
 * @param[out] ctok Buffer for current token...must point to a buffer of at least 512 bytes.
 * @return 0 on error.
 */
int fdbfs_query_qtok(char **cp, int *tval, Toke *toke, char *ctok /* MUST be a buffer at least 512b long */);

/**
 * @brief SQLite regex func. For internal use only.
 */
void fdbfs_db_regex_func(sqlite3_context *ctx, int i, sqlite3_value **sqval);

/**
 * @brief Get description for error code.
 *
 * @param rc The error code from the various VM control functions.
 * @return A human-readable string containing the error message.
 */
const char* fdbfs_query_error(int rc);

#endif

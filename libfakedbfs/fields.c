/*
 * Copyright (c) 2005-2006, Dan Ponte
 *
 * fields.c - field list convenience functions
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
/* $Amigan: fakedbfs/libfakedbfs/fields.c,v 1.1 2006/02/24 17:33:46 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <float.h>
#include <ctype.h>

#include <fakedbfs/fdbfsconfig.h>

#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/debug.h>
#include <fakedbfs/fields.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/fields.c,v 1.1 2006/02/24 17:33:46 dcp1990 Exp $")

static int field_append(name, fmtname, th, tc, type, value, len)
	char *name;
	char *fmtname;
	fields_t **th;
	fields_t **tc;
	enum DataType type;
	void *value;
	size_t len; /* if applicable */
{
	fields_t *n;

	n = malloc(sizeof(*n));
	memset(n, 0, sizeof(*n));
	n->type = type;

	if(*th == NULL) {
		*th = n;
	} else if(*tc != NULL) {
		(*tc)->next = n;
		*tc = n;
	} else {
		free(n);
		return 0;
	}

	n->fieldname = strdup(name);
	n->fmtname = strdup(fmtname);
	n->val = value;
	n->len = len;

	return 1;
}


int fdbfs_field_add_int(name, fmtname, th, tc, value)
	char *name;
	char *fmtname;
	fields_t **th;
	fields_t **tc;
	int value;
{
	int *v;
	v = malloc(sizeof(value));
	*v = value;
	return field_append(name, fmtname, th, tc, number, v, sizeof(int));
}

int fdbfs_field_add_string(name, fmtname, th, tc, value)
	char *name;
	char *fmtname;
	fields_t **th;
	fields_t **tc;
	char *value;
{
	return field_append(name, fmtname, th, tc, string, value, strlen(value));
}

int fdbfs_field_add_image(name, fmtname, th, tc, value, sz)
	char *name;
	char *fmtname;
	fields_t **th;
	fields_t **tc;
	void *value;
	size_t sz;
{
	return field_append(name, fmtname, th, tc, image, value, sz);
}



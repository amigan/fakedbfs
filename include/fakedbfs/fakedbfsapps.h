/*
 * Copyright (c) 2005, Dan Ponte
 *
 * fakedbfsapps.h - misc symbols for applications
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
/* $Amigan: fakedbfs/include/fakedbfs/fakedbfsapps.h,v 1.5 2006/01/29 21:03:55 dcp1990 Exp $ */
#ifndef HAVE_FAKEDBFSAPPS_H
#define HAVE_FAKEDBFSAPPS_H 1

#define FDBFSDBENV "FAKEDBFSDB"

extern const char *fakedbfsver;
extern const char *fakedbfscopyright;
extern const char *fakedbfsvname;
extern const int fakedbfsmaj, fakedbfsmin, fakedbfsmic;

#include <fakedbfs/conf.h>
#include <fakedbfs/db.h>
#include <fakedbfs/dbspecdata.h>
#include <fakedbfs/debug.h>
#include <fakedbfs/fficl.h>
#include <fakedbfs/fields.h>
#include <fakedbfs/indexing.h>
#include <fakedbfs/plugins.h>
#include <fakedbfs/query.h>

#endif

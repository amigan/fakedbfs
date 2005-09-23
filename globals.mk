# $Amigan: fakedbfs/globals.mk,v 1.9 2005/09/23 00:27:35 dcp1990 Exp $
CC=cc
CPPFLAGS=-I../include -I/usr/local/include -DUNIX
CFLAGS+=-g -Wall -ggdb -fPIC
# -ansi
LDFLAGS=-L/usr/local/lib
.if defined(FREEDEBUG)
CPPFLAGS+=-DFREEDEBUG
.endif
.if defined(DMALLOC)
DMALLOC=yes
CPPFLAGS+=-DDMALLOC
DMLIB=-ldmalloc
.endif

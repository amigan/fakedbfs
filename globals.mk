# $Amigan: fakedbfs/globals.mk,v 1.7 2005/09/22 21:25:05 dcp1990 Exp $
CC=cc
CPPFLAGS=-I../include -I/usr/local/include -DUNIX
CFLAGS+=-g -Wall -ggdb
# -ansi
LDFLAGS=-L/usr/local/lib
.if defined(DMALLOC)
DMALLOC=yes
CPPFLAGS+=-DDMALLOC
DMLIB=-ldmalloc
.endif

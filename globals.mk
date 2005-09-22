# $Amigan: fakedbfs/globals.mk,v 1.6 2005/09/22 19:14:56 dcp1990 Exp $
CC=cc
CPPFLAGS=-I../include -I/usr/local/include -DUNIX
CFLAGS+=-g -Wall
# -ansi
LDFLAGS=-L/usr/local/lib
.if defined(DMALLOC)
DMALLOC=yes
CPPFLAGS+=-DDMALLOC
DMLIB=-ldmalloc
.endif

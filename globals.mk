# $Amigan: fakedbfs/globals.mk,v 1.5 2005/09/19 22:23:37 dcp1990 Exp $
CC=cc
CPPFLAGS=-I../include -I/usr/local/include -DUNIX
CFLAGS+=-g -Wall
# -ansi
LDFLAGS=-L/usr/local/lib
.if defined(DMALLOC)
DMALLOC=yes
CPPFLAGS+=-DDMALLOC
LIBS=-ldmalloc
.endif

# $Amigan: fakedbfs/globals.mk,v 1.8 2005/09/22 23:21:34 caelian Exp $
CC=cc
CPPFLAGS=-I../include -I/usr/local/include -DUNIX
CFLAGS+=-g -Wall -ggdb -fPIC
# -ansi
LDFLAGS=-L/usr/local/lib
.if defined(DMALLOC)
DMALLOC=yes
CPPFLAGS+=-DDMALLOC
DMLIB=-ldmalloc
.endif

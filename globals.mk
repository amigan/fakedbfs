# $Amigan: fakedbfs/globals.mk,v 1.10 2005/10/02 16:43:06 dcp1990 Exp $
CC=cc
CPPFLAGS=-I../include -I/usr/local/include -DHAVE_CONFIG $(CPPOPTS)
CFLAGS+=-g -Wall -ggdb -fPIC
# -ansi
LDFLAGS=-L/usr/local/lib

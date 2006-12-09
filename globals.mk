# $Amigan: fakedbfs/globals.mk,v 1.13 2006/12/09 00:27:43 dcp1990 Exp $
CC=cc
CPPFLAGS=-I../include -I/usr/local/include -I../ficl -DHAVE_CONFIG -DHAVE_FICL_H $(CPPOPTS)
CFLAGS+=-g -Wall -ggdb -fPIC
# -ansi
LDFLAGS=-L/usr/local/lib $(LDEXT) -pthread

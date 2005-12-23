# $Amigan: fakedbfs/globals.mk,v 1.12 2005/12/23 19:56:49 dcp1990 Exp $
CC=cc
CPPFLAGS=-I../include -I/usr/local/include -I../ficl -DHAVE_CONFIG -DHAVE_FICL_H $(CPPOPTS)
CFLAGS+=-g -Wall -ggdb -fPIC
# -ansi
LDFLAGS=-L/usr/local/lib $(LDEXT)

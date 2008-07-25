# $Amigan: fakedbfs/globals.mk,v 1.14 2008/07/25 17:40:31 dcp1990 Exp $
CC=cc
CPPFLAGS=-I../include -I${PREFIX}/include -I/usr/local/include -I../ficl -DHAVE_CONFIG -DHAVE_FICL_H $(CPPOPTS)
CFLAGS+=-g -Wall -ggdb -fPIC
# -ansi
LDFLAGS=-L/usr/local/lib $(LDEXT) -L${PREFIX}/lib -pthread

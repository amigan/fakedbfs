# $Amigan: fakedbfs/fcreatedb/Makefile,v 1.3 2005/08/10 00:13:42 dcp1990 Exp $
include ../globals.mk
LDFLAGS+=-L../libfakedbfs
LIBS=-lfakedbfs
SRCS=fcreatedb.c
OBJS!=echo ${SRCS} | sed -e 's/\.c/.o/g'
CFLAGS+=$(CPPFLAGS) # why?
#OBJS=sqlite.o dbspec.o dbinit.o lex.yy.o memory.o error.o indexing.o
all: .depend fcreatedb
.depend: $(SRCS)
	mkdep $(CPPFLAGS) $(SRCS)
fcreatedb: $(OBJS)
	$(CC) $(LDFLAGS) -o fcreatedb ${OBJS} $(LIBS)
%.o: %.c
	$(CC) $(CFLAGS) -c ${.SOURCE}
clean:
	rm -f *.core *.o *~ .depend fcreatedb

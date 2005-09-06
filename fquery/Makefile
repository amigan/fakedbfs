# $Amigan: fakedbfs/fquery/Makefile,v 1.1 2005/09/06 07:28:38 dcp1990 Exp $
include ../globals.mk
LDFLAGS+=-L../libfakedbfs
LIBS=-lfakedbfs
SRCS=fquery.c
OBJS!=echo ${SRCS} | sed -e 's/\.c/.o/g'
CFLAGS+=-ansi $(CPPFLAGS) # why?
#OBJS=sqlite.o dbspec.o dbinit.o lex.yy.o memory.o error.o indexing.o
all: .depend fquery
.depend: $(SRCS)
	mkdep $(CPPFLAGS) $(SRCS)
fquery: $(OBJS)
	$(CC) $(LDFLAGS) -o fquery ${OBJS} $(LIBS)
%.o: %.c
	$(CC) $(CFLAGS) -c ${.SOURCE}
clean:
	rm -f *.core *.o *~ .depend fquery

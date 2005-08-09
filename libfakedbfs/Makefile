# interim makefile
CC=cc
LDFLAGS=-L/usr/local/lib
CPPFLAGS=-I../include -I/usr/local/include -DYY_NO_UNPUT
LIBS=-lsqlite3 -lfl
CFLAGS+=-g -Wall -ansi
SRCS=libfakedbfs.c sqlite.c dbspec.c dbinit.c lex.yy.c memory.c error.c indexing.c
OBJS!=echo ${SRCS} | sed -e 's/\.c/.o/g'
CFLAGS+=$(CPPFLAGS) # why?
#OBJS=sqlite.o dbspec.o dbinit.o lex.yy.o memory.o error.o indexing.o
all: .depend libfakedbfs.so
.depend: $(SRCS)
	mkdep $(CPPFLAGS) $(SRCS)
lex.yy.c: dbspec.l
	flex dbspec.l
dbspec.c: dbspec.y
	../buildtools/lemon -s dbspec.y
libfakedbfs.so: $(OBJS)
	$(CC) $(LDFLAGS) -shared -o libfakedbfs.so ${OBJS} $(LIBS)
%.o: %.c
	$(CC) $(CFLAGS) -c ${.SOURCE}
lint:
	lint -s $(CPPFLAGS) $(SRCS)
clean:
	rm -f *.core *.o *~ dbspec.c dbspec.h dbspec.out libfakedbfs.so lex.yy.c

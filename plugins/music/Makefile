# music plugin, for use with dbspecs/music.dbspec
include ../../globals.mk
LDFLAGS=-L/usr/local/lib
CPPFLAGS+=-I../../include
LIBS=-lid3 -lstdc++ -lz
SRCS=music.c
OBJS!=echo ${SRCS} | sed -e 's/\.c/.o/g'
CFLAGS+=$(CPPFLAGS)
.if defined(DMALLOC)
DMALLOC=yes
CPPFLAGS+=-DDMALLOC
LIBS+=-ldmalloc
.endif

all: .depend music.so
.depend: $(SRCS)
	mkdep $(CPPFLAGS) $(SRCS)
music.so: $(OBJS)
	cc -shared -o music.so $(LDFLAGS) $(OBJS) $(LIBS)
%.o: %.c
	cc $(CFLAGS) -c $(.SOURCE)
clean:
	rm -f $(OBJS) *.core *~ music.so .depend

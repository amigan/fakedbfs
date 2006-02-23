# music plugin, for use with dbspecs/music.dbspec
include ../../config.mk
include ../../globals.mk
LDFLAGS=-L/usr/local/lib
CPPFLAGS+=-I../../include -I../../ficl
LIBS=-lid3 -lstdc++ -lz -logg -lvorbis -lvorbisfile $(LIBEXT)
SRCS=music.c
OBJS=music.o
#OBJS!=echo ${SRCS} | sed -e 's/\.c/.o/g'
CFLAGS+=$(CPPFLAGS)

all: .depend music.so
.depend: $(SRCS)
	mkdep $(CPPFLAGS) $(SRCS)
music.so: $(OBJS)
	cc -shared -o music.so $(LDFLAGS) $(OBJS) $(LIBS)
%.o: %.c
	cc $(CFLAGS) -c $<
clean:
	rm -f $(OBJS) *.core *~ music.so .depend

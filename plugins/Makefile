# BSD $(MAKE) $(MAKE)file for the fakedbfs distribution's plugins
# (C)2005, Dan Ponte
# $Amigan: fakedbfs/plugins/Makefile,v 1.3 2005/10/07 20:35:05 dcp1990 Exp $
include ../config.mk
include ../globals.mk
COMPONENTS=music
all: all-rec
clean: clean-rec
all-rec:
	@for i in $(COMPONENTS) ; do $(MAKE) -C $$i ; done
clean-rec:
	@for i in $(COMPONENTS) ; do $(MAKE) -C $$i clean ; done

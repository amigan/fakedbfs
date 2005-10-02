# BSD make makefile for the fakedbfs distribution's plugins
# (C)2005, Dan Ponte
# $Amigan: fakedbfs/plugins/Makefile,v 1.2 2005/10/02 16:43:16 dcp1990 Exp $
include ../config.mk
include ../globals.mk
COMPONENTS=music
all: all-rec
clean: clean-rec
all-rec:
	@for i in $(COMPONENTS) ; do make -C $$i ; done
clean-rec:
	@for i in $(COMPONENTS) ; do make -C $$i clean ; done

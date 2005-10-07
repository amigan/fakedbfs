# BSD make makefile for the fakedbfs distribution
# (C)2005, Dan Ponte
# $Amigan: fakedbfs/Makefile,v 1.9 2005/10/07 20:35:04 dcp1990 Exp $
include globals.mk
COMPONENTS=buildtools libfakedbfs fcreatedb findex fquery plugins
CLEANFILES=
all: .config all-rec
clean: clean-rec
all-rec:
	@for i in $(COMPONENTS) ; do $(MAKE) -C $$i ; done
clean-rec:
	@for i in $(COMPONENTS) ; do $(MAKE) -C $$i clean ; done
#nothing yet...
.config:
	@echo "ERROR: run config.sh first!"
	@exit 1
#ourclean:
#	rm -f ${CLEANFILES}
configclean:
	rm -f .config config.h config.mk
allclean: clean configclean

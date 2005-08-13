# BSD make makefile for the fakedbfs distribution
# (C)2005, Dan Ponte
# $Amigan: fakedbfs/Makefile,v 1.1 2005/08/13 21:49:51 dcp1990 Exp $
include globals.mk
COMPONENTS=libfakedbfs fcreatedb
COMPSUF=${COMPONENTS:S/$/_cmp/}
CMPSCLEAN=${COMPONENTS:S/$/_cl/}
CLEANFILES=.config
all: ${COMPSUF}
${COMPSUF}: .config
	cd ${@:S/_cmp$//} && make
clean: ${CMPSCLEAN} ourclean
${CMPSCLEAN}:
	cd ${@:S/_cl$//} && make clean
#nothing yet...
.config:
	touch .config
ourclean:
	rm -f ${CLEANFILES}

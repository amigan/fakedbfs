#!/bin/sh
if [ -z $1 ] ; then
	echo "Usage: config.sh <tcl interpreter>"
	echo "Example: ./config.sh tclsh8.4 "
	exit 1
fi

$1 config.tcl
touch .config

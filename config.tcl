#!/usr/local/bin/tclsh8.4
# fakedbfs configure script
set platf NOTSET
set prefset NOTSET

proc platform {pf} {
	global platf
	global mfh
	if {[string compare $pf unix] != 0 && [string compare $pf win32] != 0 && [string compare $pf amiga] != 0} {
		puts "Error: Platform not one of unix|win32|amiga"
		exit 1
	}
	set platf $pf
	puts $mfh [join [list "CPPOPTS+=-D" [string toupper $pf]] "" ]
	if {$pf == "win32"} {
		puts $mfh "LDEXT+=-mno-cygwin -L/mingw/lib"
	}
}

proc pcre {pcreconfpath} {
	global platf
	global mfh
	global chfh
	puts $mfh "CPPOPTS+=`$pcreconfpath/pcre-config --cflags`"
	puts $mfh "LIBEXT+=`$pcreconfpath/pcre-config --libs`"
	puts $chfh "#define USE_PCRE 1"
}

proc freedebug {} {
	global chfh
	puts $chfh "#define FREEDEBUG 1"
}

proc dmalloc {} {
	global mfh
	global chfh
	puts $chfh "#define DMALLOC 1"
	puts $mfh "LIBEXT+=-ldmalloc"
}

proc prefix {path} {
	global chfh
	global platf
	global prefset
	if {$platf == "NOTSET"} {
		puts "Error: platform must be set first!"
		exit 1
	}
	set prefset $path
	puts $chfh "#define PREFIX \"$path\""
	if {$platf == "unix"} {
		puts $chfh "#define LIBPATH PREFIX \"/lib/fakedbfs\""
	} elseif {$platf == "amiga"} {
		puts $chfh "#define LIBPATH PREFIX \"lib/plugins\""
	} elseif {$platf == "win32"} {
		puts $chfh "#define LIBPATH PREFIX \"/PLUGINS\""
	} else {
		set prefset NOTSET
	}
}

set mfh [open config.mk w]
set chfh [open config.h w]
puts $mfh [ list # Generated by config.tcl on [clock format [clock seconds]]]
puts $chfh [ list /* Generated by config.tcl on [clock format [clock seconds]] */ ]
source fdbfsconfig
if {[string compare $prefset NOTSET] == 0} {
	prefix .
}
close $mfh
close $chfh
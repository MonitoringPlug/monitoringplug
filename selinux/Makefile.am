##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a $(SELINUX_LIBS)

bin_PROGRAMS = 

if HAVE_SELINUX
bin_PROGRAMS += check_enforce check_sebool
endif


## vim: set ts=4 sw=4 syn=automake :

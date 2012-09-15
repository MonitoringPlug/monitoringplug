##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a -loping

bin_PROGRAMS = 

if HAVE_OPING
bin_PROGRAMS += check_oping
endif

## vim: set ts=4 sw=4 syn=automake :

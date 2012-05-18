##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib $(XMLRPC_CFLAGS)
AM_LDFLAGS = $(XMLRPC_LDFLAGS)
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/*.a

bin_PROGRAMS = 

if HAVE_XMLRPC
bin_PROGRAMS += check_rhn_entitlements check_koji_hub check_koji_builder
endif


## vim: set ts=4 sw=4 syn=automake :

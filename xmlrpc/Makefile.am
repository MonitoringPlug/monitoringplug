##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = $(XMLRPC_CFLAGS)
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a ../lib/libxmlrpcutils.a $(XMLRPC_LDFLAGS)

bin_PROGRAMS = 

if HAVE_XMLRPC
bin_PROGRAMS += check_rhn_entitlements check_koji_hub check_koji_builder
endif


## vim: set ts=4 sw=4 syn=automake :

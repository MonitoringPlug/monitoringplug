##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib
AM_LDFLAGS = $(XMLRPC_LIBS)
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/*.a

bin_PROGRAMS = 

if HAVE_XMLRPC
bin_PROGRAMS += check_rhn_entitlements
endif



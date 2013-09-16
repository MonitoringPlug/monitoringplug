##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib $(VARNISH_CFLAGS)
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a $(VARNISH_LIBS)

bin_PROGRAMS = 

if HAVE_VARNISH
bin_PROGRAMS += check_varnish check_varnish_backends

endif
## vim: set ts=4 sw=4 syn=automake :

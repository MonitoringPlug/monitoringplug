##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib $(CUPS_CFLAGS)
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a $(CUPS_LDFLAGS)

bin_PROGRAMS = 

if HAVE_CUPS
bin_PROGRAMS += check_cups_jobs

endif
## vim: set ts=4 sw=4 syn=automake :

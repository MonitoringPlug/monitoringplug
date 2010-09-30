##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib

bin_PROGRAMS = 

if HAVE_EXPAT
bin_PROGRAMS += check_clustat

check_clustat_CPPFLAGS = $(AM_CPPFLAGS) $(EXPAT_CFLAGS)
check_clustat_LDFLAGS = $(AM_CPPFLAGS) $(EXPAT_LDFLAGS)
endif

eck_clustat_SOURCE = check_aspsms_credits.c

LDADD = ../lib/libmonitoringplug.a

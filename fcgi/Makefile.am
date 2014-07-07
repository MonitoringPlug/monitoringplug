##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a ../lib/libfcgiutils.a $(FCGI_LIBS)

bin_PROGRAMS = 

if HAVE_FCGI
bin_PROGRAMS += check_fcgi_ping

if HAVE_JSON
bin_PROGRAMS += check_fcgi_phpfpm

check_fcgi_phpfpm_CPPFLAGS = $(JSON_CFLAGS)
check_fcgi_phpfpm_LDADD = $(LDADD) $(JSON_LIBS) ../lib/libjsonutils.a
endif

endif
## vim: set ts=4 sw=4 syn=automake :

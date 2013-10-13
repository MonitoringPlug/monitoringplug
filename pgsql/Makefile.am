##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = $(PQINCPATH)
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a ../lib/libpgsqlutils.a $(LIBPQ_LIBS)

bin_PROGRAMS = 

if HAVE_LIBPQ
bin_PROGRAMS += check_pgsql check_pgsql_slave

endif
## vim: set ts=4 sw=4 syn=automake :

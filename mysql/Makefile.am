##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = $(MYSQL_CFLAGS)
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a ../lib/libmysqlutils.a $(MYSQL_LDFLAGS)

bin_PROGRAMS = 

if HAVE_MYSQL
bin_PROGRAMS += check_mysql check_mysql_rows

endif
## vim: set ts=4 sw=4 syn=automake :

##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib $(MYSQL_CFLAGS)
AM_LDFLAGS = $(MYSQL_LDFLAGS)
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a ../lib/libmysqlutils.a

bin_PROGRAMS = 

if HAVE_MYSQL
bin_PROGRAMS += check_mysql check_mysql_rows

endif

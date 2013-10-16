##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CFLAGS = $(REDIS_CFLAGS)
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a $(REDIS_LIBS)

bin_PROGRAMS = 

if HAVE_SMBCLIENT
bin_PROGRAMS += check_redis

endif
## vim: set ts=4 sw=4 syn=automake :

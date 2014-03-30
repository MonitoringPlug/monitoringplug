##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CFLAGS = $(LDAP_CFLAGS)
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a ../lib/libldaputils.a $(LDAP_LIBS)

bin_PROGRAMS = 

if HAVE_LDAP
bin_PROGRAMS += check_ldap_replication
endif


## vim: set ts=4 sw=4 syn=automake :

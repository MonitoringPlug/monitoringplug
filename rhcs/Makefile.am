##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a ../lib/librhcsutils.a

bin_PROGRAMS = 

if HAVE_EXPAT
bin_PROGRAMS += check_clustat

check_clustat_CPPFLAGS = $(AM_CPPFLAGS) $(EXPAT_CFLAGS)
check_clustat_LDFLAGS = $(AM_LDFLAGS) $(EXPAT_LDFLAGS)
check_clustat_LDADD = $(LDADD) $(EXPAT_LIBS)

if HAVE_NET_SNMP
bin_PROGRAMS += check_rhcsnmp

check_rhcsnmp_CPPFLAGS = $(AM_CPPFLAGS) $(NETSNMP_CFLAGS)
check_rhcsnmp_LDADD = $(LDADD) ../lib/libsnmputils.a $(NETSNMP_LIBS)
endif

endif
## vim: set ts=4 sw=4 syn=automake :

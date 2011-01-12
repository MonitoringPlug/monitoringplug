##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib
AM_LDFLAGS = $(MPLIB_LIBS)

bin_PROGRAMS = 

if HAVE_EXPAT
bin_PROGRAMS += check_clustat

check_clustat_CPPFLAGS = $(AM_CPPFLAGS) $(EXPAT_CFLAGS)
check_clustat_LDFLAGS = $(AM_LDFLAGS) $(EXPAT_LDFLAGS) $(RHCSUTIL_LIBS)
endif

if HAVE_NET_SNMP
bin_PROGRAMS += check_rhcsnmp

check_clustat_LDFLAGS = $(AM_LDFLAGS) $(EXPAT_LDFLAGS) $(RHCSUTIL_LIBS)
check_rhcsnmp_LDFLAGS = $(AM_LDFLAGS) $(NETSNMP_LIBS) $(SNMPUTIL_LIBS) $(RHCSUTIL_LIBS)
endif

check_clustat_SOURCE = check_clustat.c
check_rhcsnmp_SOURCE = check_rhcsnmp.c

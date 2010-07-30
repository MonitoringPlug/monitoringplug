##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib $(LDNS_CFLAGS)

bin_PROGRAMS = 

if HAVE_NET_SNMP
bin_PROGRAMS += check_apc_pdu check_rhc
endif

check_apc_pdu_SOURCE = check_apc_pdu.c
check_rhc_SOURCE = check_rhc.c

LDADD = $(NETSNMP_LIBS) snmp_utils.o ../lib/libmonitoringplug.a

snmp_utils.o: snmp_utils.c snmp_utils.h

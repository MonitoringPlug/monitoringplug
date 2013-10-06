##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = $(NETSNMP_CFLAGS)
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a ../lib/libsnmputils.a $(NETSNMP_LIBS)

bin_PROGRAMS =

if HAVE_NET_SNMP
bin_PROGRAMS += check_akcp \
				check_arc_raid \
				check_apc_pdu \
				check_interface \
				check_qnap_disks \
				check_qnap_vols \
			\
			check_snmp_ups
endif

## vim: set ts=4 sw=4 syn=automake :

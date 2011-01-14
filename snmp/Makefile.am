##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib
AM_LDFLAGS = $(MPLIB_LIBS)
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/*.a

bin_PROGRAMS = 

if HAVE_NET_SNMP
bin_PROGRAMS += check_apc_pdu

check_apc_pdu_LDFLAGS = $(AM_LDFLAGS) $(NETSNMP_LIBS)
endif



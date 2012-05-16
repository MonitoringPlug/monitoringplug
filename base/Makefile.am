##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib 
AM_LDFLAGS = 
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a

bin_PROGRAMS = check_file check_nrped

if OS_LINUX
bin_PROGRAMS += check_bonding check_dhcp check_mem check_multipath check_sockets

check_dhcp_LDADD = $(LDADD) ../lib/libdhcputils.a
endif

## vim: set ts=4 sw=4 syn=automake :

##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a

bin_PROGRAMS = check_file check_memcached check_nrped check_redis

if OS_LINUX
bin_PROGRAMS += check_bonding check_dhcp check_mem check_multipath check_sockets

check_dhcp_LDADD = $(LDADD) ../lib/libdhcputils.a
endif

if HAVE_TERMIOS
bin_PROGRAMS += check_gsm_signal

check_gsm_signal_LDADD = $(LDADD) ../lib/libsmsutils.a
endif

## vim: set ts=4 sw=4 syn=automake :

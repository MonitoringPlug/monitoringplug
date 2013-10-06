##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = $(RPC_CFLAGS)
AM_LDFLAGS = $(RPC_LDFLAGS)
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a ../lib/librpcutils.a

bin_PROGRAMS = 

if HAVE_RPC
bin_PROGRAMS += check_nfs check_rpc_ping
endif

## vim: set ts=4 sw=4 syn=automake :

## Process this file with automake to produce Makefile.in

AM_LDFLAGS = $(NETSNMP_AGENT_LDFLAGS)
AM_CFLAGS = $(NETSNMP_CFLAGS)

if HAVE_NET_SNMP
noinst_PROGRAMS = replayd

replayd_SOURCES = replayd.c replayd.h \
				  parse_type.c
endif

EXTRA_DIST = replays

## vim: set ts=4 sw=4 syn=automake :

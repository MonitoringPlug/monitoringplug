## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = $(NETSNMP_CFLAGS)

LDADD = $(NETSNMP_AGENT_LIBS)

if HAVE_NET_SNMP
noinst_PROGRAMS = replayd

replayd_SOURCES = replayd.c replayd.h \
				  parse_type.c
endif

EXTRA_DIST = replays

## vim: set ts=4 sw=4 syn=automake :

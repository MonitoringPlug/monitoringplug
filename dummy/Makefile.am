##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib 

bin_PROGRAMS = check_dummy check_timeout

check_dummy_SOURCES = check_dummy.c
check_timeout_SOURCES = check_timeout.c

check_dummy_LDADD = ../lib/libmonitoringplug.a
check_timeout_LDADD = ../lib/libmonitoringplug.a

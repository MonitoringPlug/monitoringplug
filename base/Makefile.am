##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib 

bin_PROGRAMS = check_file

check_file_SOURCES = check_file.c

LDADD = ../lib/libmonitoringplug.a

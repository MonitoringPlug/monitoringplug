##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib 

bin_PROGRAMS = check_file

if OS_LINUX
    bin_PROGRAMS += check_sockets
endif

check_file_SOURCES = check_file.c
check_sockets_SOURCES = check_sockets.c

LDADD = ../lib/libmonitoringplug.a

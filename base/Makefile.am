##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib 
AM_LDFLAGS = 
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a

bin_PROGRAMS = check_file

if OS_LINUX
bin_PROGRAMS += check_sockets
endif


##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib
AM_LDFLAGS = $(MPLIB_LIBS)

bin_PROGRAMS = check_nfs

check_nfs_SOURCES = check_nfs.c

##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib $(SMBCLIENT_CFLAGS)
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a $(SMBCLIENT_LIBS)

bin_PROGRAMS = 

if HAVE_SMBCLIENT
bin_PROGRAMS += check_smb_share

endif
## vim: set ts=4 sw=4 syn=automake :

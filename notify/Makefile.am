##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib 
AM_LDFLAGS = 
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplugtemplate.a \
		../lib/libmonitoringplugnotify.a \
		../lib/libmonitoringplug.a

bin_PROGRAMS = 

if HAVE_TERMIOS
bin_PROGRAMS += notify_sms

notify_sms_LDADD = ../lib/libsmsutils.a $(LDADD)
endif

## vim: set ts=4 sw=4 syn=automake :

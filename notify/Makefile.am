##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib 
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplugtemplate.a \
		../lib/libmonitoringplugnotify.a \
		../lib/libmonitoringplug.a

bin_PROGRAMS = notify_mail notify_stdout

if HAVE_TERMIOS
bin_PROGRAMS += notify_sms

notify_sms_LDADD = ../lib/libsmsutils.a $(LDADD)
endif

if HAVE_CURL
bin_PROGRAMS += notify_aspsms

notify_aspsms_CPPFLAGS = $(AM_CPPFLAGS) $(LIBCURL_CPPFLAGS)
notify_aspsms_LDADD = $(LDADD) ../lib/libcurlutils.a $(LIBCURL)
endif

## vim: set ts=4 sw=4 syn=automake :

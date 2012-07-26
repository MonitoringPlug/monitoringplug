##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib $(LIBCURL_CPPFLAGS)
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a ../lib/libcurlutils.a $(LIBCURL)

bin_PROGRAMS = 

if HAVE_CURL
bin_PROGRAMS += check_apache_status check_aspsms_credits check_webdav

if HAVE_EXPAT
check_webdav_LDADD = $(LDADD) ../lib/libexpatutils.a $(EXPAT_LDFLAGS)
else
check_webdav_LDADD = $(LDADD)
endif

if HAVE_JSON
bin_PROGRAMS += check_buildbot_slave

check_buildbot_slave_CPPFLAGS = $(AM_CPPFLAGS) $(JSON_CFLAGS)  -D__STRICT_ANSI__
check_buildbot_slave_LDADD = $(LDADD) $(JSON_LIBS)
endif
endif

if HAVE_CURL_TFTP
bin_PROGRAMS += check_tftp
endif

## vim: set ts=4 sw=4 syn=automake :

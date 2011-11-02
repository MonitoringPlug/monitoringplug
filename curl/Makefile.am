##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib $(LIBCURL_CPPFLAGS)
AM_LDFLAGS = $(LIBCURL)
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a

bin_PROGRAMS = 

if HAVE_CURL
bin_PROGRAMS += check_aspsms_credits

if HAVE_JSON
bin_PROGRAMS += check_buildbot_slave

check_buildbot_slave_CPPFLAGS = $(AM_CPPFLAGS) $(JSON_CFLAGS)  -D__STRICT_ANSI__
check_buildbot_slave_LDFLAGS = $(AM_LDFLAGS) $(JSON_LIBS)
endif

endif

if HAVE_CURL_TFTP
bin_PROGRAMS += check_tftp
endif


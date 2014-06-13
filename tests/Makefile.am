## Process this file with automake to produce Makefile.in

SUBDIRS = base curl gnutls snmp

LDADD = ../lib/libmonitoringplug.a
AM_LDFLAGS = $(CHECK_LIBS)
AM_CFLAGS = $(CHECK_CFLAGS)

if HAVE_CHECK

TESTS = check_monitoringplug \
		check_args \
		check_check \
		check_sms \
		check_subprocess \
		check_template

check_PROGRAMS = $(TESTS)

check_monitoringplug_SOURCES = main.c main.h \
    check_common.c \
    check_eopt.c \
    check_utils.c \
	check_perfdata.c

check_sms_LDADD = ../lib/libsmsutils.a $(LDADD)

check_template_LDADD = ../lib/libmonitoringplugtemplate.a $(LDADD)

if HAVE_EXPAT
TESTS += check_rhcs

check_rhcs_LDADD = ../lib/librhcsutils.a $(LDADD)
check_rhcs_CFLAGS = $(EXPAT_CFLAGS)
check_rhcs_LDFLAGS = $(AM_LDFLAGS) $(EXPAT_LIBS)
endif

if HAVE_NET_SNMP
TESTS += check_snmp

check_snmp_LDADD = ../lib/libsnmputils.a $(LDADD)
check_snmp_CFLAGS = $(NETSNMP_CFLAGS)
check_snmp_LDFLAGS = $(AM_LDFLAGS) $(NETSNMP_LIBS)
endif

endif

EXTRA_DIST = mpcheck.h eopt.ini testdata sharness.sh
## vim: set ts=4 sw=4 syn=automake :

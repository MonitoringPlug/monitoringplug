## Process this file with automake to produce Makefile.in

SUBDIRS = replayd

LDADD = ../lib/libmonitoringplug.a
AM_LDFLAGS = $(CHECK_LIBS)
AM_CFLAGS = $(AM_CPPFLAGS) $(CHECK_CFLAGS)

if HAVE_CHECK

TESTS = check_monitoringplug \
		check_sms \
		check_subprocess \
		check_template
if HAVE_EXPAT
TESTS += check_rhcs
endif

check_PROGRAMS = $(TESTS)

check_monitoringplug_SOURCES = main.c main.h \
    check_check.c \
    check_common.c \
    check_args.c \
    check_eopt.c \
    check_utils.c \
	check_perfdata.c

if HAVE_EXPAT
check_rhcs_LDADD = ../lib/librhcsutils.a $(LDADD)
check_rhcs_CPPFLAGS = $(AM_CPPFLAGS) $(EXPAT_CFLAGS)
check_rhcs_LDFLAGS = $(AM_LDFLAGS) $(EXPAT_LDFLAGS)
endif

check_sms_LDADD = ../lib/libsmsutils.a $(LDADD)

check_template_LDADD = ../lib/libmonitoringplugtemplate.a $(LDADD)

endif

EXTRA_DIST = eopt.ini testdata
## vim: set ts=4 sw=4 syn=automake :

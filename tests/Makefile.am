## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib
LDADD = ../lib/*.a

if HAVE_CHECK

TESTS = check_monitoringplug

check_PROGRAMS = check_monitoringplug

check_monitoringplug_SOURCES = main.c main.h \
    check_check.c \
    check_common.c \
    check_args.c \
    check_popen.c \
    check_eopt.c \
    check_utils.c

check_monitoringplug_CFLAGS = $(AM_CPPFLAGS) $(CHECK_CFLAGS)

check_monitoringplug_LDFLAGS = $(AM_LDFLAGS) $(CHECK_LIBS)

endif

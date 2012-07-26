##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib $(IPMI_CFLAGS)
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a ../lib/libipmiutils.a $(IPMI_LIBS)

bin_PROGRAMS = 

if HAVE_IPMI
bin_PROGRAMS += check_ipmi_fan check_ipmi_mem check_ipmi_psu check_ipmi_sensor

endif

## vim: set ts=4 sw=4 syn=automake :

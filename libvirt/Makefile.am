##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CFLAGS = $(LIBVIRT_CFLAGS)
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a ../lib/libvirtutils.a $(LIBVIRT_LIBS)

bin_PROGRAMS = 

if HAVE_LIBVIRT
bin_PROGRAMS += check_libvirtd check_libvirt_domain

check_libvirt_domain_CFLAGS = $(AM_CFLAGS) $(EXPAT_CFLAGS)
check_libvirt_domain_LDFLAGS = $(AM_LDFLAGS) $(EXPAT_LIBS)
check_libvirt_domain_LDADD = $(LDADD) $(EXPAT_LIBS)

endif
## vim: set ts=4 sw=4 syn=automake :

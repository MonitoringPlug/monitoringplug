##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib $(LDNS_CFLAGS)

bin_PROGRAMS = check_dnssec_expiration

check_dnssec_expiration_SOURCE = check_dnssec_expiration.c

check_dnssec_expiration_LDADD = $(LDNS_LIBS) dns_utils.o ../lib/libmonitoringplug.a

dns_utils.o: dns_utils.c dns_utils.h

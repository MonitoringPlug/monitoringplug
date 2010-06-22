##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib $(LDNS_CFLAGS)

bin_PROGRAMS = check_dns_sync check_dnssec_expiration

check_dns_sync_SOURCE = check_dns_sync.c
check_dnssec_expiration_SOURCE = check_dnssec_expiration.c

check_dns_sync_LDADD = $(LDNS_LIBS) dns_utils.o ../lib/libmonitoringplug.a
check_dnssec_expiration_LDADD = $(LDNS_LIBS) dns_utils.o ../lib/libmonitoringplug.a

dns_utils.o: dns_utils.c dns_utils.h

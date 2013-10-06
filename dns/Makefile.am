##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CFLAGS = $(LDNS_CFLAGS)
AM_DEFAULT_SOURCE_EXT = .c

LDADD = ../lib/libmonitoringplug.a ../lib/libldnsutils.a $(LDNS_LDFLAGS)

bin_PROGRAMS = 

if HAVE_LDNS
bin_PROGRAMS += check_dns_authoritative\
                check_dns_sync \
                check_dnssec_expiration \
                check_dnssec_trace \
                check_dnssec_trust_anchor
endif

## vim: set ts=4 sw=4 syn=automake :

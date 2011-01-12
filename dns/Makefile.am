##Process this file with automake to create Makefile.in

bindir = ${libdir}/nagios/plugins

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib $(LDNS_CFLAGS)
AM_LDFLAGS = $(MPLIB_LIBS) $(LDNSUTIL_LIBS) $(LDNS_LIBS)

bin_PROGRAMS = 

if HAVE_LDNS
bin_PROGRAMS += check_dns_sync \
				check_dnssec_expiration \
				check_dnssec_trace \
				check_dnssec_trust_anchor
endif

check_dns_sync_SOURCE = check_dns_sync.c
check_dnssec_expiration_SOURCE = check_dnssec_expiration.c
check_dnssec_trace_SOURCE = check_dnssec_trace.c
check_dnssec_trust_anchor_SOURCE = check_dnssec_trust_anchor.c


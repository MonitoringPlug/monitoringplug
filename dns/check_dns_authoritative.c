/***
 * Monitoring Plugin - check_dns_authoritative.c
 **
 *
 * check_dns_authoritative - Check a Authoritative DNS server.
 *
 * Copyright (C) 2012 Marius Rieder <marius.rieder@durchmesser.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * $Id$
 */

const char *progname  = "check_dns_authoritative";
const char *progdesc  = "Check a Authoritative DNS server.";
const char *progvers  = "0.1";
const char *progcopy  = "2012";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "-H <host> -D <domain>";

/* MP Includes */
#include "mp_common.h"
#include "ldns_utils.h"
/* Default Includes */
#include <string.h>
#include <signal.h>
#include <unistd.h>
/* Library Includes */
#include <ldns/ldns.h>

/* Global Vars */
const char *hostname = NULL;
char *domainname = NULL;
int recursion = 1;
int tcp = 1;
int udp = 1;
int axfr = 1;

int main (int argc, char **argv) {
    /* Local Vars */
    char            *out = NULL;
    ldns_resolver   *res;
    ldns_rdf        *domain;
    ldns_rdf        *host;
    ldns_rdf        *example;
    ldns_pkt        *pkt;
    ldns_rr         *rr;
    ldns_status     status;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    // Create DNAME from domainname
    domain = ldns_dname_new_frm_str(domainname);
    if (!domain)
        usage("Invalid domainname '%s'", domainname);

    // Create rdf from hostaddr
    host = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_A, hostname);
#ifdef USE_IPV6
    if (!host)
        host = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_AAAA, hostname);
#endif
    if (!host) {
        ldns_rdf_deep_free(domain);
        usage("Invalid hostname '%s'", hostname);
    }

    // Create DNAME from example.com
    example = ldns_dname_new_frm_str("exmple.com");
    if (!example)
        usage("Invalid domainname 'example.com'");

    // Create resolver
    res = ldns_resolver_new();
    if (!res) {
        ldns_rdf_deep_free(domain);
        ldns_rdf_deep_free(host);
        unknown("Create resolver failed.");
    }
    // Add ns to resolver
    status = ldns_resolver_push_nameserver(res, host);
    if (status != LDNS_STATUS_OK) {
        ldns_rdf_deep_free(domain);
        ldns_rdf_deep_free(host);
        ldns_resolver_deep_free(res);
        unknown("Adding %s as NS fails.", domainname);
    }

    if (udp) {
        // Disable TCP
        ldns_resolver_set_usevc(res, 0);

        // Fetch SOA
        pkt = mp_ldns_resolver_query(res, domain, LDNS_RR_TYPE_SOA,
                                  LDNS_RR_CLASS_IN, 0);
        if (pkt == NULL || ldns_pkt_get_rcode(pkt) != LDNS_RCODE_NOERROR) {
            mp_strcat_comma(&out, "No UDP Answer");
        } else if (ldns_pkt_aa(pkt) == 0) {
            mp_strcat_comma(&out, "Non Authoritative UDP Answer");
        }
        if (mp_verbose > 2) {
            printf("[ SOA Answer ]----------\n");
            ldns_pkt_print(stdout,pkt);
        }
        ldns_pkt_free(pkt);

        if (recursion) {
            // Fetch example.com SOA
            ldns_resolver_set_recursive(res, TRUE);
            pkt = mp_ldns_resolver_query(res, example, LDNS_RR_TYPE_SOA,
                    LDNS_RR_CLASS_IN, LDNS_RD);
            ldns_resolver_set_recursive(res, FALSE);
            if (pkt && (ldns_pkt_get_rcode(pkt) != LDNS_RCODE_REFUSED &&
                    ldns_pkt_get_rcode(pkt) != LDNS_RCODE_SERVFAIL)) {
                mp_strcat_comma(&out, "Recursive UDP Answer");
            }
            if (mp_verbose > 2) {
                printf("[ SOA Answer ]----------\n");
                ldns_pkt_print(stdout,pkt);
            }
            ldns_pkt_free(pkt);
        }
    }

    if (tcp) {
        // Enable TCP
        ldns_resolver_set_usevc(res, 1);

        // Fetch SOA
        pkt = mp_ldns_resolver_query(res, domain, LDNS_RR_TYPE_SOA,
                                  LDNS_RR_CLASS_IN, 0);
        if (pkt == NULL || ldns_pkt_get_rcode(pkt) != LDNS_RCODE_NOERROR) {
            mp_strcat_comma(&out, "No TCP Answer");
        } else if (ldns_pkt_aa(pkt) == 0) {
            mp_strcat_comma(&out, "Non Authoritative TCP Answer");
        }
        if (mp_verbose > 2) {
            printf("[ SOA Answer ]----------\n");
            ldns_pkt_print(stdout,pkt);
        }
        ldns_pkt_free(pkt);

        if (recursion) {
            // Fetch example.com SOA
            ldns_resolver_set_recursive(res, TRUE);
            pkt = mp_ldns_resolver_query(res, example, LDNS_RR_TYPE_SOA,
                    LDNS_RR_CLASS_IN, LDNS_RD);
            ldns_resolver_set_recursive(res, FALSE);
            if (pkt && (ldns_pkt_get_rcode(pkt) != LDNS_RCODE_REFUSED &&
                    ldns_pkt_get_rcode(pkt) != LDNS_RCODE_SERVFAIL)) {
                mp_strcat_comma(&out, "Recursive TCP Answer");
            }
            if (mp_verbose > 2) {
                printf("[ SOA Answer ]----------\n");
                ldns_pkt_print(stdout,pkt);
            }
            ldns_pkt_free(pkt);
        }
    }

    if (axfr) {
        status = ldns_axfr_start(res, domain, LDNS_RR_CLASS_IN);
        if (status == LDNS_STATUS_OK) {
            rr = NULL;
            rr = ldns_axfr_next(res);
            if (rr) {
                mp_strcat_comma(&out, "AXFR allowed.");
            }
        }
    }

    if (out)
        critical("Authoritative DNS for %s: %s", domainname, out);

    ok("Authoritative DNS for %s", domainname);
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        {"domainname", required_argument, 0, 'D'},
        {"norecursion", no_argument, &recursion, 0},
        {"notcp", no_argument, &tcp, 0},
        {"noudp", no_argument, &udp, 0},
        {"noaxfr", no_argument, &axfr, 0},
        MP_LONGOPTS_END
    };


    if (argc < 2) {
        print_help();
        exit(STATE_OK);
    }

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"H:D:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        switch (c) {
            /* Host opt */
            case 'H':
                getopt_host_ip(optarg, &hostname);
                break;
            case 'D':
                domainname = optarg;
                break;
        }
    }

    /* Check requirements */
    if (!domainname)
        usage("A domainname is mandatory.");

    return(OK);
}

void print_help (void) {
    print_revision();
    print_revision_ldns();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();

    print_help_default();
    print_help_host();
    printf(" -D, --domain=DOMAIN\n");
    printf("      The name of the domain to check.\n");
    printf("     --norecursion\n");
    printf("      Do not test for disabled recursion.\n");
    printf("     --notcp\n");
    printf("      Do not test TCP queries.\n");
    printf("     --noudp\n");
    printf("      Do not test UDP queries.\n");
    printf("     --noaxfrn\n");
    printf("      Do not test for disabled AXFR.\n");
}

/* vim: set ts=4 sw=4 et syn=c : */

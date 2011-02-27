/***
 * Monitoring Plugin - check_dnssec_trace
 **
 *
 * check_dnssec_trace - Check if the zone signature is tracable.
 * Copyright (C) 2010 Marius Rieder <marius.rieder@durchmesser.ch>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id$
 */

const char *progname  = "check_dnssec_trace";
const char *progvers  = "0.1";
const char *progcopy  = "2009 - 2010";
const char *progauth = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[-H host] -D domain [-T domain] [-k file] [-t timeout]";

/* MP Includes */
#include "mp_common.h"
#include "ldns_utils.h"
/* Default Includes */
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
/* Library Includes */
#include <ldns/ldns.h>

/* Global Vars */
const char *hostname;
char *domainname;
char *domaintrace;
ldns_rr_list *trusted_keys = NULL;
int checkState;

int main(int argc, char **argv) {
    /* Local Vars */
    int             i;
    int             soa_valid = 0;
    int             ns_valid = 0;
    ldns_rdf        *rd_domain;
    ldns_rdf        *rd_trace;
    ldns_rdf        *rd_cdomain;
    ldns_pkt        *pkt;
    ldns_resolver   *res;
    ldns_rr         *rr;
    ldns_rr_list    *rrl;
    ldns_rr_list    *rrl_domain_soa;
    ldns_rr_list    *rrl_domain_soa_rrsig;
    ldns_rr_list    *rrl_domain_ns;
    ldns_rr_list    *rrl_domain_ns_rrsig;
    ldns_rr_list    *rrl_valid_keys;
    ldns_status	    status;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);
    
    rd_domain = ldns_dname_new_frm_str(domainname);
    if (!rd_domain)
        unknown("Illegal domain name");

    if (domaintrace) {
        rd_trace = ldns_dname_new_frm_str(domaintrace);
        if (!rd_trace)
            unknown("Illegal trace domain name");
     } else {
        rd_trace = ldns_dname_new_frm_str(".");
     }
    
    /* Check domain is subdomain from trace start */
    if (!ldns_dname_is_subdomain(rd_domain, rd_trace)) {
        ldns_rr_list_deep_free(trusted_keys);
        ldns_rdf_deep_free(rd_domain);
        ldns_rdf_deep_free(rd_trace);
        unknown("'%s' is not a subdomain of '%s'.", domainname,
                                                    domaintrace);
    }

    /* Add trusted keys for trace domain to rrl_valid_keys. */
    rrl_valid_keys = ldns_rr_list_new();
    for(i = 0; i < ldns_rr_list_rr_count(trusted_keys); i++) {
        rr = ldns_rr_list_rr(trusted_keys, i);
        if (ldns_dname_compare(ldns_rr_owner(rr),rd_trace) == 0)
            ldns_rr_list_push_rr(rrl_valid_keys, ldns_rr_clone(rr));
    }
    ldns_rr_list_deep_free(trusted_keys);
    
    if (ldns_rr_list_rr_count(rrl_valid_keys) == 0) {
        ldns_rdf_deep_free(rd_domain);
        ldns_rdf_deep_free(rd_trace);
        ldns_rr_list_deep_free(rrl_valid_keys);
        critical("No trusted key for trace start '%s'", domaintrace?domaintrace:".");
    }
    
    if (mp_verbose >= 2) {
        printf("--[ Trusted keys used ]-------------------------------------\n");
        ldns_rr_list_sort(rrl_valid_keys);
        ldns_rr_list_print(stdout, rrl_valid_keys);
        printf("------------------------------------------------------------\n");
    }                                 
   
    /* create a new resolver with dns_server or server from /etc/resolv.conf */
    res = createResolver(hostname);
    if (!res) {
        ldns_rdf_deep_free(rd_domain);
        ldns_rdf_deep_free(rd_trace);
        ldns_rr_list_deep_free(rrl_valid_keys);
        unknown("Creating resolver faild.");
    }
    resolverEnableDnssec(res);
    ldns_resolver_set_dnssec_anchors(res, rrl_valid_keys);

    /* check domain exists */
    pkt = mp_ldns_resolver_query(res, rd_domain, LDNS_RR_TYPE_ANY,
                              LDNS_RR_CLASS_IN, LDNS_RD);
    
    if (pkt == NULL || ldns_pkt_get_rcode(pkt) != LDNS_RCODE_NOERROR) {
        ldns_rdf_deep_free(rd_domain);
        ldns_rdf_deep_free(rd_trace);
        ldns_rr_list_deep_free(rrl_valid_keys);
        ldns_resolver_deep_free(res);
        if (pkt && ldns_pkt_get_rcode(pkt) == LDNS_RCODE_NXDOMAIN) {
            ldns_pkt_free(pkt);
            critical("Domain '%s' don't exist.", domainname);
        }
        ldns_pkt_free(pkt);
        critical("Unable to get SOA for %s.", domainname);
    }
    
    rrl_domain_soa = ldns_pkt_rr_list_by_name_and_type(pkt, rd_domain,
                                                       LDNS_RR_TYPE_SOA,
                                                       LDNS_SECTION_ANSWER);

    if (rrl_domain_soa == NULL || ldns_rr_list_rr_count(rrl_domain_soa) == 0) {
        ldns_rdf_deep_free(rd_domain);
        ldns_rdf_deep_free(rd_trace);
        ldns_resolver_deep_free(res);
        ldns_pkt_free(pkt);
        critical("Domain '%s' not found.", domainname);
    }

    rrl_domain_soa_rrsig = ldns_dnssec_pkt_get_rrsigs_for_name_and_type(pkt,
                                rd_domain, LDNS_RR_TYPE_SOA);

    if (rrl_domain_soa_rrsig == NULL ||
        ldns_rr_list_rr_count(rrl_domain_soa_rrsig) == 0) {
        free(domainname);
        free(domaintrace);
        ldns_rdf_deep_free(rd_domain);
        ldns_rdf_deep_free(rd_trace);
        ldns_resolver_deep_free(res);
        ldns_pkt_free(pkt);
        ldns_rr_list_deep_free(rrl_domain_soa);
        critical("Domain '%s' not signed.", domainname);
    }

    rrl_domain_ns = ldns_pkt_rr_list_by_name_and_type(pkt, rd_domain,
                                                      LDNS_RR_TYPE_NS,
                                                      LDNS_SECTION_ANSWER);
    rrl_domain_ns_rrsig = ldns_dnssec_pkt_get_rrsigs_for_name_and_type(pkt,
                                                      rd_domain,
                                                      LDNS_RR_TYPE_NS);
    
    ldns_pkt_free(pkt);

    if (mp_verbose >= 2) {
        printf("--[ Checked Domain ]----------------------------------------\n");
        ldns_rr_list_print(stdout, rrl_domain_soa);
        printf("------------------------------------------------------------\n");
        ldns_rr_list_print(stdout, rrl_domain_soa_rrsig);
        printf("------------------------------------------------------------\n");
        ldns_rr_list_print(stdout, rrl_domain_ns);
        printf("------------------------------------------------------------\n");
        ldns_rr_list_print(stdout, rrl_domain_ns_rrsig);
        printf("------------------------------------------------------------\n");
    }

    /* Fetch valid keys from top down */
    i = ldns_dname_label_count(rd_domain) - ldns_dname_label_count(rd_trace);
    for (; i>=0; i--) {
        rd_cdomain = ldns_dname_clone_from(rd_domain, i);
        if (mp_verbose) {
            char *str = ldns_rdf2str(rd_cdomain);
            printf("Trace: %s\n", str);
            free(str);
        }
        rrl = ldns_fetch_valid_domain_keys(res, rd_cdomain, rrl_valid_keys, &status);

        if (mp_verbose >= 2) {
            printf("--[ Valid Keys ]----------------------------------------\n");
            ldns_rr_list_sort(rrl);
            ldns_rr_list_print(stdout, rrl);
            printf("------------------------------------------------------------\n");
        }


        ldns_rr_list_cat(rrl_valid_keys, rrl);
        ldns_rr_list_free(rrl);

        ldns_rdf_deep_free(rd_cdomain);
    }
    
    ldns_rdf_deep_free(rd_trace);
    ldns_rdf_deep_free(rd_domain);

    /* Validate SOA */
    for(i = 0; i < ldns_rr_list_rr_count(rrl_domain_soa_rrsig); i++) {
        rr = ldns_rr_list_rr(rrl_domain_soa_rrsig, i);
        status = ldns_verify_rrsig_keylist(rrl_domain_soa, rr, rrl_valid_keys, NULL);
        if (status == LDNS_STATUS_OK)
            soa_valid++;
    }

    ldns_rr_list_deep_free(rrl_domain_soa);
    ldns_rr_list_deep_free(rrl_domain_soa_rrsig);

    if (soa_valid == 0) {
        critical("No valid Signatur for SOA of '%s'", domainname);
        free(domainname);
        free(domaintrace);
        ldns_resolver_deep_free(res);
        ldns_rr_list_deep_free(rrl_domain_ns);
        ldns_rr_list_deep_free(rrl_domain_ns_rrsig);
        return checkState;
    }

    /* Validate NS */
    for(i = 0; i < ldns_rr_list_rr_count(rrl_domain_ns_rrsig); i++) {
        rr = ldns_rr_list_rr(rrl_domain_ns_rrsig, i);
        
        status = ldns_verify_rrsig_keylist(rrl_domain_ns, rr, rrl_valid_keys, NULL);
        if (status == LDNS_STATUS_OK)
            ns_valid++;
    }

    ldns_rr_list_deep_free(rrl_domain_ns);
    ldns_rr_list_deep_free(rrl_domain_ns_rrsig);
    ldns_resolver_deep_free(res);
    
    if (ns_valid == 0) {
        critical("No valid Signatur for NS of '%s'", domainname);
        free(domainname);
        free(domaintrace);
        return checkState;
    }

    ok("Trust for '%s' successfull traces from '%s'", domainname, 
        domaintrace);
    free(domainname);
    free(domaintrace);
    return checkState;
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;
    
    static struct option long_opts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        {"domain", required_argument, 0, 'D'},
        {"trace-from", required_argument, 0, 'T'},
        {"trusted-keys", required_argument, 0, 'k'},
        MP_LONGOPTS_TIMEOUT,
        MP_LONGOPTS_END
    };

    if (argc < 2) {
        print_help();
        exit (STATE_OK);
    }
        
    while (1) {
        c = getopt_long(argc, argv, MP_OPTSTR_DEFAULT"t:H:D:T:k:", long_opts, &option);
        if (c == -1 || c == EOF)
            break;

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            /* Host opt */
            case 'H':
                getopt_host_ip(optarg, &hostname);
                break;
            case 'D':
                if (!is_hostname(optarg))
                    usage("Illegal domain name.");
                domainname = optarg;
                break;
            case 'k':
                trusted_keys = loadKeyfile(optarg);
                if (trusted_keys == NULL)
                    usage("Parsing keyfiel faild.");
                break;
            case 'T':
                if (!is_hostname(optarg))
                    usage("Illegal trace domain name.");
                domaintrace = optarg;
                break;
            /* Timeout opt */
            case 't':
                getopt_timeout(optarg);
                break;
        }
    }

    /* Check requirements */
    if(!domainname)
        usage("Domain is mandatory");
    
    return OK;
}

void print_help (void) {
    print_revision();
    print_copyright();

    printf("\n");

    printf("Check if the zone serial are in sync.");

    printf("\n\n");

    print_usage();

    print_help_default();
    print_help_host();
    printf(" -D, --domain=DOMAIN\n");
    printf("      The name of the domain to check.\n");
    printf(" -T, --trace-from=DOMAIN\n");
    printf("      The name of the domain to trace from. (default: .)\n");
    printf(" -k, --trusted-keys=FILE\n");
    printf("      File to read trust-anchors from.\n");
    print_help_warn_time("2 days");
    print_help_crit_time("1 day");
}

/* vim: set ts=4 sw=4 et syn=c.libdns : */

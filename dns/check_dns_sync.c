/***
 * Monitoring Plugin - check_dns_sync
 **
 *
 * check_dns_sync - Check if the zone serial are in sync.
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

const char *progname  = "check_dns_sync";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "-D <domain> [-H <host>]";

/* MP Includes */
#include "mp_common.h"
#include "ldns_utils.h"
/* Default Includes */
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
/* Library Includes */
#include <ldns/ldns.h>

/* Global Vars */
const char *hostname = NULL;
char *domainname = NULL;

int main (int argc, char **argv) {
    /* Local Vars */
    int             i = 0;
    char            *tmp;
    ldns_resolver   *res;
    ldns_rdf        *domain;
    ldns_rdf        *host;
    ldns_pkt        *pkt;
    ldns_rr_list    *rrl;
    ldns_rr         *rr;
    ldns_status     status;
    int             ns_count;
    ldns_rr         **ns_soa;
    ldns_rdf        **ns_name;
    ldns_rr         *master_soa = NULL;
    ldns_rdf        *master_name = NULL;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) == OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    // Create DNAME from domainname
    domain = ldns_dname_new_frm_str(domainname);
    if (!domain)
        usage("Invalid domainname '%s'", domainname);

    if (hostname) {
        // Check one nameserver against master
        if (mp_verbose > 0)
            printf("Check zone sync for %s on %s\n", domainname, hostname);

        // Create rdf from hostaddr
        host = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_A, hostname);
#ifdef USE_IPV6
        if (!host)
            host = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_AAAA, hostname);
#endif
        if (!host) {
            ldns_rdf_deep_free(domain);
            printf("ldns_rdf_get_type %d\n", ldns_rdf_get_type(host));
            usage("Invalid hostname '%s'", hostname);
        }

        // Create resolver
        res = ldns_resolver_new();
        if (!res) {
            ldns_rdf_deep_free(domain);
            ldns_rdf_deep_free(host);
            unknown("Create resolver faild.");
        }

        //  Enable TCP if requested
        ldns_resolver_set_usevc(res, mp_ldns_usevc);

        // Add ns to resolver
        status = ldns_resolver_push_nameserver(res, host);
        if (status != LDNS_STATUS_OK) {
            ldns_rdf_deep_free(domain);
            ldns_rdf_deep_free(host);
            ldns_resolver_deep_free(res);
            unknown("Adding %s as ns fails.", domainname);
        }

        // Fetch SOA
        pkt = mp_ldns_resolver_query(res, domain, LDNS_RR_TYPE_SOA,
                                  LDNS_RR_CLASS_IN, LDNS_RD);

        if (pkt == NULL || ldns_pkt_get_rcode(pkt) != LDNS_RCODE_NOERROR) {
            ldns_rdf_deep_free(domain);
            ldns_rdf_deep_free(host);
            ldns_resolver_deep_free(res);
            if (pkt && ldns_pkt_get_rcode(pkt) == LDNS_RCODE_NXDOMAIN) {
                ldns_pkt_free(pkt);
                critical("Domain '%s' don't exist.", domainname);
            }
            ldns_pkt_free(pkt);
            critical("Unable to get SOA for %s from %s.", domainname, hostname);
        }

        if (mp_verbose > 2) {
            printf("[ SOA Anser ]----------\n");
            ldns_pkt_print(stdout,pkt);
        }

        ns_count = 1;
        ns_soa = (ldns_rr * *) mp_malloc(sizeof(ldns_rr *));
        ns_name = (ldns_rdf * *) mp_malloc(sizeof(ldns_rdf *));

        ns_name[0] = host;

        rrl = ldns_pkt_rr_list_by_name_and_type(pkt, domain, LDNS_RR_TYPE_SOA,
                                                LDNS_SECTION_ANSWER);

        ns_soa[0] = ldns_rr_list_pop_rr(rrl);

        ldns_rr_list_deep_free(rrl);
        ldns_pkt_free(pkt);

        master_name = ldns_rdf_clone(ldns_rr_rdf(ns_soa[0], 0));

        rrl = getaddr_rdf(NULL, master_name);

        ldns_rdf_deep_free(ldns_resolver_pop_nameserver(res));
        ldns_resolver_push_nameserver_rr_list(res, rrl);

        ldns_rr_list_deep_free(rrl);

        // Fetch Master SOA
        pkt = mp_ldns_resolver_query(res, domain, LDNS_RR_TYPE_SOA,
                                  LDNS_RR_CLASS_IN, LDNS_RD);

        if (pkt == NULL || ldns_pkt_get_rcode(pkt) != LDNS_RCODE_NOERROR) {
            ldns_rdf_deep_free(domain);
            ldns_rdf_deep_free(host);
            ldns_resolver_deep_free(res);
            if (pkt && ldns_pkt_get_rcode(pkt) == LDNS_RCODE_NXDOMAIN) {
                ldns_pkt_free(pkt);
                critical("Domain '%s' don't exist.", domainname);
        }
        ldns_pkt_free(pkt);
            critical("Unable to get SOA for %s from master.", domainname);
        }

        rrl = ldns_pkt_rr_list_by_name_and_type(pkt, domain, LDNS_RR_TYPE_SOA,
                                                LDNS_SECTION_ANSWER);

        master_soa = ldns_rr_list_pop_rr(rrl);

        ldns_rr_list_deep_free(rrl);
        ldns_pkt_free(pkt);

    } else {
        // Check all nameserver against master
        if (mp_verbose > 0)
            printf("Check zone sync for %s\n", domainname);

        // Use a DNS server from resolv.conf
        status = ldns_resolver_new_frm_file(&res, NULL);
        if (status != LDNS_STATUS_OK) {
            ldns_rdf_deep_free(domain);
            unknown("Create resolver faild.");
        }

        //  Enable TCP if requested
        ldns_resolver_set_usevc(res, mp_ldns_usevc);

        // Fetch NS
        pkt = mp_ldns_resolver_query(res, domain, LDNS_RR_TYPE_NS,
                                  LDNS_RR_CLASS_IN, LDNS_RD);

        if (pkt == NULL || ldns_pkt_get_rcode(pkt) != LDNS_RCODE_NOERROR) {
            ldns_rdf_deep_free(domain);
            ldns_resolver_deep_free(res);
            if (pkt && ldns_pkt_get_rcode(pkt) == LDNS_RCODE_NXDOMAIN) {
           ldns_pkt_free(pkt);
           critical("Domain '%s' don't exist.", domainname);
        }
        ldns_pkt_free(pkt);
        critical("Unable to get NS for %s.", domainname);
        }

        if (mp_verbose > 2) {
            printf("[ NS Anser ]----------\n");
            ldns_pkt_print(stdout,pkt);
        }

        rrl = ldns_pkt_rr_list_by_name_and_type(pkt, domain, LDNS_RR_TYPE_NS,
                                                LDNS_SECTION_ANSWER);

        ns_count = ldns_rr_list_rr_count(rrl);
        ns_soa = (ldns_rr * *) mp_malloc(ns_count*sizeof(ldns_rr *));
        ns_name = (ldns_rdf * *) mp_malloc(ns_count*sizeof(ldns_rdf *));

        for (i=0; i<ns_count; i++) {
            rr = ldns_rr_list_rr(rrl, i);
            ns_name[i] = ldns_rr_set_rdf(rr, NULL, 0);
        }

        ldns_rr_list_deep_free(rrl);
        ldns_pkt_free(pkt);
        ldns_resolver_deep_free(res);

        // Create resolver
        res = ldns_resolver_new();
        if (!res) {
            ldns_rdf_deep_free(domain);
            unknown("Create resolver faild.");
        }

        //  Enable TCP if requested
        ldns_resolver_set_usevc(res, mp_ldns_usevc);

        for (i=0; i<ns_count; i++) {
            rrl = getaddr_rdf(NULL, ns_name[i]);
            ldns_resolver_push_nameserver_rr_list(res, rrl);

            if (mp_verbose>1) {
                tmp = ldns_rdf2str(ns_name[i]);
                printf("[ Addr for %s ]----------\n", tmp);
                free(tmp);
                ldns_rr_list_print(stdout, rrl);
            }

            ldns_rr_list_deep_free(rrl);

            //
            // Fetch SOA
            pkt = mp_ldns_resolver_query(res, domain, LDNS_RR_TYPE_SOA,
                                      LDNS_RR_CLASS_IN, LDNS_RD);

            if (pkt == NULL || ldns_pkt_get_rcode(pkt) != LDNS_RCODE_NOERROR) {
                ldns_rdf_deep_free(domain);
                ldns_resolver_deep_free(res);
                if (pkt && ldns_pkt_get_rcode(pkt) == LDNS_RCODE_NXDOMAIN) {
                    ldns_pkt_free(pkt);
                    tmp = ldns_rdf2str(ns_name[i]);
                    critical("Domain '%s' don't exist on '%s'.", domainname, tmp);
                }
                ldns_pkt_free(pkt);
                tmp = ldns_rdf2str(ns_name[i]);
                critical("Unable to get SOA for %s from '%s'.", domainname, tmp);
            }

            if (mp_verbose > 2) {
                tmp = ldns_rdf2str(ns_name[i]);
                printf("[ SO Anser from %s ]----------\n", tmp);
                free(tmp);
                ldns_pkt_print(stdout,pkt);
            }

            rrl = ldns_pkt_rr_list_by_name_and_type(pkt, domain,
                                                    LDNS_RR_TYPE_SOA,
                                                    LDNS_SECTION_ANSWER);

            rr = ldns_rr_list_pop_rr(rrl);
            ldns_rr_list_deep_free(rrl);
        ldns_pkt_free(pkt);

            if (mp_verbose>0) {
                tmp = ldns_rdf2str(ns_name[i]);
                printf("[ SOA for %s ]----------\n", tmp);
                free(tmp);
                ldns_rr_print(stdout, rr);
            }

            if (ldns_rdf_compare(ldns_rr_rdf(rr,0), ns_name[i]) == 0) {
                master_soa = rr;
                master_name = ns_name[i];
                ns_soa[i] = NULL;
            } else {
                ns_soa[i] = rr;
            }

            while (ldns_resolver_nameserver_count(res) > 0)
                ldns_rdf_deep_free(ldns_resolver_pop_nameserver(res));
        }
    }

    ldns_resolver_deep_free(res);

    char *error_str = NULL;
    int error_cnt = 0;

    for (i=0; i<ns_count; i++) {
        if (ns_soa[i] == 0)
            continue;
        if (ldns_rr_compare(ns_soa[i],master_soa) != 0) {
            if (error_str == NULL) {
                error_str = ldns_rdf2str(ns_name[i]);
            } else {
                tmp = ldns_rdf2str(ns_name[i]);
                error_str = mp_realloc(error_str, (strlen(error_str) + strlen(tmp) + 2 ));
                strcat(error_str, ", ");
                strcat(error_str, tmp);
                free(tmp);
            }
            error_cnt++;
        }
    ldns_rr_free(ns_soa[i]);
    ldns_rdf_deep_free(ns_name[i]);
    }

    free(ns_soa);
    free(ns_name);
    ldns_rr_free(master_soa);
    ldns_rdf_deep_free(master_name);
    ldns_rdf_deep_free(domain);

    if (error_cnt == 0) {
        if (ns_count == 1)
            ok("%s is in sync with the master.", domainname);
        else
            ok("All NS of %s are in sync with the master.", domainname);
    } else if (error_cnt == 1) {
        critical("%s is out of sync with master", error_str);
    }

    critical("%s are out of sync with master", error_str);
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        {"domainname", required_argument, 0, 'D'},
        MP_LONGOPTS_TIMEOUT,
        MP_LONGOPTS_END
    };

   
    if (argc < 2) {
        print_help();
        exit(STATE_OK);
    }

    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"H:D:t:", longopts, &option);

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
                domainname = optarg;
                break;
            /* Timeout opt */
            case 't':
                getopt_timeout(optarg);
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
   print_copyright();

   printf("\n");

   printf("Check if the zone serial are in sync.");

   printf("\n\n");

   print_usage();

   print_help_default();
   print_help_host();
   printf(" -D, --domain=DOMAIN\n");
   printf("      The name of the domain to check.\n");
}

/* vim: set ts=4 sw=4 et : */

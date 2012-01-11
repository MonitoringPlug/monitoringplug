/***
 * Monitoring Plugin - check_dnssec_trust_anchor.c
 **
 *
 * check_dnssec_trust_anchor - Check if a trust anchor file is uptodate.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id$
 */

const char *progname  = "check_dnssec_trust_anchor";
const char *progdesc  = "Check if a trust anchor file is uptodate.";
const char *progvers  = "0.1";
const char *progcopy  = "2009 - 2010";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[-H host] -k file [-t timeout]";

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
ldns_rr_list *trusted_keys = NULL;

int main(int argc, char **argv) {
    /* Local Vars */
    int             i;
    char            *invalid = NULL;
    ldns_resolver   *res;
    ldns_rdf        *rd_owner;
    ldns_rr         *rr;
    ldns_rr_list    *rrl_keys;


    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK) {
        ldns_rr_list_deep_free(trusted_keys);
        unknown("Parsing arguments faild!");
    }

    /* Start plugin timeout */
    alarm(mp_timeout);

    if (mp_verbose > 1)
        ldns_rr_list_print(stdout,trusted_keys);

    /* Create a new resolver with hostname or server from /etc/resolv.conf */
    res = createResolver(hostname);
    if (!res)
        unknown("Creating resolver faild.");
    resolverEnableDnssec(res);

    /* Test all trust anchors */
    for(i = 0; i < ldns_rr_list_rr_count(trusted_keys); i++) {
        rr = ldns_rr_list_rr(trusted_keys, i);
        rd_owner = ldns_rr_owner(rr);

        if (mp_verbose >= 1)
            printf("Test: %s\n", ldns_rdf2str(rd_owner));



        /* Create a new resolver with hostname or server from /etc/resolv.conf */
        res = createResolver(hostname);
        if (!res)
            unknown("Creating resolver faild.");

        rrl_keys = ldns_validate_domain_dnskey(res, rd_owner, trusted_keys);

        ldns_resolver_deep_free(res);

        if (mp_verbose >= 2) {
            printf("--[ Valid Domain Key ]----------------------------------------\n");
            ldns_rr_list_print(stdout, rrl_keys);
            printf("------------------------------------------------------------\n");
        }

        if (!rrl_keys) {
            if (mp_verbose >= 1)
                printf("  Invalid.\n");
            mp_strcat_comma(&invalid, ldns_rdf2str(rd_owner));
        }
        ldns_rr_list_deep_free(rrl_keys);
    }

    ldns_rr_list_deep_free(trusted_keys);

    if (invalid)
        critical("Invalid KEYs in trusted-keys for '%s'", invalid);
    else
        ok("All keys from trusted-keys valid");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option long_opts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        {"domain", required_argument, 0, 'D'},
        {"trusted-keys", required_argument, 0, 'k'},
        MP_LONGOPTS_TIMEOUT,
        MP_LONGOPTS_END
    };

    if (argc < 2) {
        print_help();
        exit (STATE_OK);
    }

    while (1) {
        c = getopt_long(argc, argv, MP_OPTSTR_DEFAULT"t:H:k:w:c:", long_opts, &option);
        if (c == -1 || c == EOF)
            break;

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            /* Host opt */
            case 'H':
                getopt_host_ip(optarg, &hostname);
                break;
            // Plugin specific args
            case 'k': // -k --trusted-keys
                trusted_keys = loadAnchorfile(optarg);
                if(trusted_keys == NULL
                   || ldns_rr_list_rr_count(trusted_keys) == 0)
                    usage("Can't load trust anchors from file");
                break;
            /* Timeout opt */
            case 't':
                getopt_timeout(optarg);
                break;
        }
    }

    return OK;
}

void print_help(void) {
    print_revision();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();

    print_help_default();
    print_help_host();
    printf(" -k, --trusted-keys=FILE\n");
    printf("      File to read trust-anchors from.\n");
}

/* vim: set ts=4 sw=4 et syn=c : */

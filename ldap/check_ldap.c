/***
 * Monitoring Plugin - check_ldap.c
 **
 *
 * check_ldap - Check a LDAP Server and fetch statistics.
 *
 * Copyright (C) 2014 Marius Rieder <marius.rieder@durchmesser.ch>
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

const char *progname  = "check_ldap";
const char *progdesc  = "Check a LDAP Server and fetch statistics.";
const char *progvers  = "0.1";
const char *progcopy  = "2014";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "-H <LDAP-URI> [-b <binddn>] [-w <bindpw>]";

/* MP Includes */
#include "mp_common.h"
#include "ldap_utils.h"
/* Default Includes */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ldap.h>

/* Global Vars */
thresholds *time_thresholds = NULL;

int main (int argc, char **argv) {
    /* Local Vars */
    struct timeval start_time;
    double time_delta;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);


    /* Start LDAP connection */
    gettimeofday(&start_time, NULL);
    LDAP *ld = NULL;
    ld = mp_ldap_init(NULL);
    LDAPMessage *msg = NULL;

    msg = mp_ldap_search(ld, "cn=Statistics,cn=Monitor", LDAP_SCOPE_ONE,
            NULL,
            (char *[]){"cn", "monitorCounter", '\0'});

    time_delta = mp_time_delta(start_time);

    LDAPMessage *stat = NULL;
    for (stat = ldap_first_entry(ld, msg); stat != NULL;
            stat = ldap_next_entry(ld, stat)) {
        char *cn = NULL;
        char *counter = NULL;
        struct berval **vals;
    
        vals = ldap_get_values_len(ld, stat, "cn");
        if (vals == NULL) {
            if (mp_verbose > 0)
                printf("Skipping %s\n", ldap_get_dn(ld, stat));
            continue;
        }
        cn = strdup(vals[0]->bv_val);
        ldap_value_free_len(vals);
    
        vals = ldap_get_values_len(ld, stat, "monitorCounter");
        if (vals == NULL) {
            if (mp_verbose > 0)
                printf("Skipping %s\n", ldap_get_dn(ld, stat));
            free(cn);
            continue;
        }
        counter = strdup(vals[0]->bv_val);
        ldap_value_free_len(vals);
    
        if (mp_verbose > 0) {
            printf("%s: %s\n", cn, counter);
        }
    
        // Calculate slave lag
        mp_perfdata_int(cn, (int)atol(counter), "c", NULL);
    
        free(cn);
        free(counter);
    }

    mp_perfdata_float("time", (float)time_delta, "s", time_thresholds);

    switch(get_status(time_delta, time_thresholds)) {
        case STATE_OK:
            free_threshold(time_thresholds);
            ok("LDAP");
            break;
        case STATE_WARNING:
            free_threshold(time_thresholds);
            warning("LDAP is slow.");
            break;
        case STATE_CRITICAL:
            free_threshold(time_thresholds);
            critical("LDAP is real slow.");
            break;
    }
    free_threshold(time_thresholds);

    critical("You should never reach this point.");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        LDAP_LONGOPTS,
        MP_LONGOPTS_END
    };

    /* Set default */
    setWarnTime(&time_thresholds, "2s");
    setCritTime(&time_thresholds, "5s");

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"w:c:"LDAP_OPTSTR, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_ldap(c);
        getopt_wc_time(c, optarg, &time_thresholds);
    }

    /* Check requirements */
    if (!mp_ldap_uri)
        usage("LDAP-URI is mandatory.");

    return(OK);
}

void print_help (void) {
    print_revision();
    print_revision_ldap();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();

    print_help_default();
    print_help_ldap();
    print_help_warn_time("2s");
    print_help_crit_time("5s");
}

/* vim: set ts=4 sw=4 et syn=c : */

/***
 * Monitoring Plugin - check_keepalived_vrrp.c
 **
 *
 * check_keepalived_vrrp - Check the VRRP State of Keepalived by snmp.
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

const char *progname  = "check_keepalived_vrrp";
const char *progdesc  = "Check the VRRP State of Keepalived by snmp.";
const char *progvers  = "0.1";
const char *progcopy  = "2014";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--hostname <HOSTNAME>";

/* MP Includes */
#include "mp_common.h"
#include "snmp_utils.h"
/* Default Includes */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Library Includes */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>


/* Global Vars */
const char  *hostname = NULL;
int         port = 161;
char        **instance = NULL;
int         instances = 0;

char* STATENAME[] = { "", "BACKUP", "MASTER" };

int main (int argc, char **argv) {
    /* Local Vars */
    int         i, j;
    int         rc;
    char        *vrrp_name;
    long int    vrrp_id;
    long int    vrrp_state = -1;
    long int    vrrp_state_initial = -1;
    mp_snmp_subtree         table_state;
    netsnmp_session         *ss;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    // PLUGIN CODE
    ss = mp_snmp_init();

    /* OIDs to query */
    rc = mp_snmp_subtree_query(ss, MP_OID(1,3,6,1,4,1,9586,100,5,2,3,1),
        &table_state);
    if (rc != STAT_SUCCESS) {
        char *string;
        snmp_error(ss, NULL, NULL, &string);
        unknown("Keepalived VRRP: Error fetching table: %s", string);
    }

    mp_snmp_deinit();

    vrrp_name = mp_malloc(25);

    for (i = 0; i<table_state.size; i++) {
        rc = mp_snmp_subtree_get_value(&table_state,
            MP_OID(1,3,6,1,4,1,9586,100,5,2,3,1,2), i,
            ASN_OCTET_STR, (void *)&vrrp_name, 24);

        /* No more vrrpInstances */
        if (rc == 0)
            break;

        /* Check if instance should be checked */
        if (instances > 0) {
            for (j=0; j<instances; j++) {
                if (instance[j] == NULL || mp_strmatch(vrrp_name, instance[j]) == 0)
                    continue;
                break;
            }
            if (j >= instances)
                continue;
            /* Set to NULL as allready matched */
            if (instance[j][strlen(instance[j])-1] != '*')
                instance[j] = NULL;
        }

        /* Check State */
        rc = mp_snmp_subtree_get_value(&table_state,
                MP_OID(1,3,6,1,4,1,9586,100,5,2,3,1,3), i,
                ASN_GAUGE, (void *)&vrrp_id, sizeof(long int));

        rc = mp_snmp_subtree_get_value(&table_state,
                MP_OID(1,3,6,1,4,1,9586,100,5,2,3,1,4), i,
                ASN_INTEGER, (void *)&vrrp_state, sizeof(long int));

        rc = mp_snmp_subtree_get_value(&table_state,
                MP_OID(1,3,6,1,4,1,9586,100,5,2,3,1,5), i,
                ASN_INTEGER, (void *)&vrrp_state_initial, sizeof(long int));

        if (mp_verbose) {
            printf("vrrpInstanceName %i => %s\n", i, vrrp_name);
            printf(" ID => %li\n", vrrp_id);
            printf(" Status => %li (should: %li)\n", vrrp_state, vrrp_state_initial);
        }

        if (vrrp_state != vrrp_state_initial) {
            set_warning("%s(%li) is %s", vrrp_name, vrrp_id, STATENAME[vrrp_state]);
        }

    }
    mp_snmp_subtree_free(&table_state);

    /* Check for unmatched instances */
    if (instances > 0) {
        for(i=0; i < instances; i++) {
            if (instance[i] != NULL && instance[i][strlen(instance[i])-1] != '*') {
                set_critical("%s (not found)" , instance[i]);
            }
        }
    }

    /* Output and return */
    if (i == 0)
        unknown("Keepalived VRRP: No Instances found.");
    mp_exit("Keepalived VRRP");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
            MP_LONGOPTS_DEFAULT,
            MP_LONGOPTS_HOST,
            MP_LONGOPTS_PORT,
            {"instance", required_argument, NULL, (int)'i'},
            SNMP_LONGOPTS,
            MP_LONGOPTS_END
    };

    if (argc < 3) {
       print_help();
       exit(STATE_OK);
    }


    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"H:P:i:"SNMP_OPTSTR, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_snmp(c);

        switch (c) {
            /* Hostname opt */
            case 'H':
                getopt_host(optarg, &hostname);
                break;
            /* Port opt */
            case 'P':
                getopt_port(optarg, &port);
                break;
            /* Plugin opt */
            case 'i':
                mp_array_push(&instance, optarg, &instances);
                break;
        }
    }

    return(OK);
}

void print_help (void) {
    print_revision();
    print_revision_snmp();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();
    print_help_default();
    print_help_snmp();
}

/* vim: set ts=4 sw=4 et syn=c : */

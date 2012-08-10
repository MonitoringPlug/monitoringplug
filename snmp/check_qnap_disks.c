/***
 * Monitoring Plugin - check_qnap_disks.c
 **
 *
 * check_qnap_disks - Check the disks of a QNAP NAS by snmp.
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

const char *progname  = "check_qnap_disks";
const char *progdesc  = "Check the disks of a QNAP NAS by snmp.";
const char *progvers  = "0.1";
const char *progcopy  = "2011";
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

/* Global Enums */
enum {
    HDD_Ready = 0,
    HDD_NoDisk = -5,
    HDD_Invalid = -6,
    HDD_RWError = -9,
    HDD_Unknown = -4,
};

/* Global Vars */
const char  *hostname = NULL;
int         port = 161;

int main (int argc, char **argv) {
    /* Local Vars */
    int         i;
    int         rc = 0;
    char        *output = NULL;
    long int    disk_state;
    char        *disk_name;
    int         status = STATE_OK;
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
    status = mp_snmp_subtree_query(ss, MP_OID(1,3,6,1,4,1,24681,1,2,11),
        &table_state);
    if (status != STAT_SUCCESS) {
        char *string;
        snmp_error(ss, NULL, NULL, &string);
        unknown("QNAP: Error fetching table: %s", string);
    }

    mp_snmp_deinit();

    status = STATE_OK;

    disk_name = mp_malloc(64);

    for (i = 0; i<table_state.size; i++) {
        rc = mp_snmp_subtree_get_value(&table_state,
            MP_OID(1,3,6,1,4,1,24681,1,2,11,1,4), i,
            ASN_INTEGER, (void *)&disk_state, sizeof(long int));

        if (rc == 0)
            break;

        if (disk_state == HDD_Ready)
            continue;

        mp_snmp_subtree_get_value(&table_state,
            MP_OID(1,3,6,1,4,1,24681,1,2,11,1,2), i,
            ASN_OCTET_STR, (void *)&disk_name, 64);

        mp_strcat_comma(&output, disk_name);

        switch (disk_state) {
            case HDD_NoDisk:
                mp_strcat_space(&output, "missing");
                break;
            case HDD_Invalid:
                mp_strcat_space(&output, "invalid");
                break;
            case HDD_RWError:
                mp_strcat_space(&output, "r/w-error");
                break;
            case HDD_Unknown:
            default:
                mp_strcat_space(&output, "unknown");
                break;
        }

        status = STATE_CRITICAL;
    }
    mp_snmp_subtree_free(&table_state);
    free(disk_name);

    if (i == 0)
        unknown("QNAP: No Disks found.");

    /* Output and return */
    if (status == STATE_OK)
        ok("QNAP: All Disks \"GOOD\"");
    critical("QNAP: %s", output);
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
            MP_LONGOPTS_DEFAULT,
            MP_LONGOPTS_HOST,
            MP_LONGOPTS_PORT,
            SNMP_LONGOPTS,
            MP_LONGOPTS_END
    };

    if (argc < 3) {
       print_help();
       exit(STATE_OK);
    }


    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"H:P:o:O:"SNMP_OPTSTR, longopts, &option);

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

/***
 * Monitoring Plugin - check_rhcsnmp.c
 **
 *
 * check_rhcsnmp - Check a RedHat Cluster Suite by snmp.
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


const char *progname  = "check_rhc";
const char *progdesc  = "Check a RedHat Cluster Suite by snmp.";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "-H <HOST>";

/* MP Includes */
#include "mp_common.h"
#include "snmp_utils.h"
/* Default Includes */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/* Library Includes */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

/* Global Vars */
const char *hostname = NULL;
int port = 0;

int main (int argc, char **argv) {
    /* Local Vars */
    int status;
    char *clustername;
    long int clusterstatus;
    char *clusterstatusdesc;
    long int clustervotes;
    long int clusterquorum;
    long int clusternodes;
    netsnmp_session *ss;

    /* OIDs to query */
    struct mp_snmp_query_cmd snmpcmd[] = {
        {{1,3,6,1,4,1,2312,8,2,1,0}, 11, ASN_OCTET_STR, (void *)&clustername},
        {{1,3,6,1,4,1,2312,8,2,2,0}, 11, ASN_INTEGER, (void *)&clusterstatus},
        {{1,3,6,1,4,1,2312,8,2,3,0}, 11, ASN_OCTET_STR, (void *)&clusterstatusdesc},
        {{1,3,6,1,4,1,2312,8,2,5,0}, 11, ASN_INTEGER, (void *)&clustervotes},
        {{1,3,6,1,4,1,2312,8,2,4,0}, 11, ASN_INTEGER, (void *)&clusterquorum},
        {{1,3,6,1,4,1,2312,8,2,7,0}, 11, ASN_INTEGER, (void *)&clusternodes},
        {{0}, 0, 0, 0},
    };

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    /* Init Net-SNMP */
    ss = mp_snmp_init();

    /* Query target agent */
    status = mp_snmp_query(ss, snmpcmd);
    switch (status) {
        case STAT_ERROR:
            unknown("Error in libnetsnmp");
        case STAT_TIMEOUT:
            critical("Timeout in libnetsnmp");
    }

    /* Finish Net-SNMP */
    mp_snmp_deinit();

    /* Perfdata */
    if (mp_showperfdata)
        mp_perfdata_int3("votes", clustervotes, "", 1, clusterquorum, 1,
                         clusterquorum-1, 1, 0, 1, clusternodes);

    /* Output and return */
    if (clusterstatus < 2)
        ok("%s [%s]", clustername, clusterstatusdesc);
    if (clusterstatus < 16)
        warning("%s [%s]", clustername, clusterstatusdesc);
    critical("%s [%s]", clustername, clusterstatusdesc);
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
        c = mp_getopt(argc, argv, MP_OPTSTR_DEFAULT"H:p:"SNMP_OPTSTR, longopts, &option);

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

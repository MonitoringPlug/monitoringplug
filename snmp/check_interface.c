/***
 * Monitoring Plugin - check_interface.c
 **
 *
 * check_interface - Check interface status by SNMP IF-MIB.
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

const char *progname  = "check_interface";
const char *progdesc  = "Check interface status by SNMP IF-MIB.";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "-H <HOST> -I <ifIndex>";

/* MP Includes */
#include "mp_common.h"
#include "snmp_utils.h"
/* Default Includes */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
/* Library Includes */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

/* Global Vars */
const char  *hostname = NULL;
int         port = 161;
int         ifIndex = 0;
int         should = 1;

int main (int argc, char **argv) {
    /* Local Vars */
    long int    ifOperStatus = 0;
    char        *ifDescr = NULL;
    long int    ifSpeed = 0;
    long int    ifInOctets = 0;
    long int    ifInErrors = 0;
    long int    ifOutOctets = 0;
    long int    ifOutErrors = 0;
    netsnmp_session         *ss;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    ss = mp_snmp_init();

    /* OIDs to query */
    mp_snmp_query_cmd snmpcmd[] = {
        {{1,3,6,1,2,1,2,2,1,2,ifIndex}, 11,
            ASN_OCTET_STR, (void *)&ifDescr, 0},
        {{1,3,6,1,2,1,2,2,1,5,ifIndex}, 11,
            ASN_GAUGE, (void *)&ifSpeed, sizeof(long int)},
        {{1,3,6,1,2,1,2,2,1,8,ifIndex}, 11,
            ASN_INTEGER, (void *)&ifOperStatus, sizeof(long int)},
        {{1,3,6,1,2,1,2,2,1,10,ifIndex}, 11,
            ASN_COUNTER, (void *)&ifInOctets, sizeof(long int)},
        {{1,3,6,1,2,1,2,2,1,14,ifIndex}, 11,
            ASN_COUNTER, (void *)&ifInErrors, sizeof(long int)},
        {{1,3,6,1,2,1,2,2,1,16,ifIndex}, 11,
            ASN_COUNTER, (void *)&ifOutOctets, sizeof(long int)},
        {{1,3,6,1,2,1,2,2,1,20,ifIndex}, 11,
            ASN_COUNTER, (void *)&ifOutErrors, sizeof(long int)},
        {{0}, 0, 0, 0, 0},
    };
    
    mp_snmp_query(ss, snmpcmd);

    mp_snmp_deinit();

    if (ifDescr == NULL)
        unknown("Interface with index %d not found.", ifIndex);

    mp_perfdata_int("ifInOctets", ifInOctets, "c", NULL);
    mp_perfdata_int("ifInErrors", ifInErrors, "c", NULL);
    mp_perfdata_int("ifOutOctets", ifOutOctets, "c", NULL);
    mp_perfdata_int("ifOutErrors", ifOutErrors, "c", NULL);
    mp_perfdata_int("ifSpeed", ifSpeed, "", NULL);

    if (ifOperStatus == should) {
        ok("%s is %s", ifDescr,  ifOperStatusText[(int)ifOperStatus]);
    } else {
        critical("%s is %s", ifDescr,  ifOperStatusText[(int)ifOperStatus]);
    }
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
            MP_LONGOPTS_DEFAULT,
            MP_LONGOPTS_HOST,
            MP_LONGOPTS_PORT,
            {"interface", required_argument, NULL, (int)'I'},
            {"down",      no_argument,       NULL, (int)'d'},
            {"should",    required_argument, NULL, (int)'s'},
            SNMP_LONGOPTS,
            MP_LONGOPTS_END
    };

    if (argc < 3) {
       print_help();
       exit(STATE_OK);
    }


    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"H:P:I:ds:"SNMP_OPTSTR, longopts, &option);

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
            case 'I':
                ifIndex = (int) strtol(optarg, NULL, 10);
                break;
            case 'd':
                should= 2;
                break;
            case 's':
                for(should = 1; should < 9; should++) {
                    if(strcmp(optarg, ifOperStatusText[should]) == 0)
                        break;
                }
                break;
        }
    }

    if (should > 7)
        usage("should is one of up, down, testing, inknown, dormant, notPresent, lowerLayerDown");

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

    printf(" -I, --interface=[INDEX]\n");
    printf("      Index of Interface to check.\n");
    printf(" -d, --down\n");
    printf("      Check for interface being down.\n");
    printf(" -s --should=[STATE]\n");
    printf("      Check for interface being in STATE.\n");

    print_help_snmp();
}

/* vim: set ts=4 sw=4 et syn=c : */

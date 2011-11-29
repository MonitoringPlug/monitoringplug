/***
 * Monitoring Plugin - check_akcp.c
 **
 *
 * check_akcp - Check interface status by SNMP IF-MIB
 *
 * Copyright (C) 2011 Marius Rieder <marius.rieder@durchmesser.ch>
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

const char *progname  = "check_akcp";
const char *progdesc  = "Check akcp sensor by snmp";
const char *progvers  = "0.1";
const char *progcopy  = "2011";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "-H <HOST>";

/* MP Includes */
#include "mp_common.h"
#include "snmp_utils.h"
/* Default Includes */
#include <getopt.h>
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

int main (int argc, char **argv) {
    /* Local Vars */
    int i;
    char        *temp_name;
    long int    temp;
    long int    temp_unit;
    long int    temp_state;
    long int    temp_warn;
    long int    temp_crit;
    int         state = STATE_OK;
    char        *output;
    netsnmp_session         *ss;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    ss = mp_snmp_init();

    /* Query Temp */
    for (i = 0; i < 10; i++) {
        temp_state = 0;
        struct mp_snmp_query_cmd snmpcmd[] = {
            {{1,3,6,1,4,1,3854,1,2,2,1,16,1,1,i}, 15, ASN_OCTET_STR, (void *)&temp_name},
            {{1,3,6,1,4,1,3854,1,2,2,1,16,1,3,i}, 15, ASN_INTEGER, (void *)&temp},
            {{1,3,6,1,4,1,3854,1,2,2,1,16,1,4,i}, 15, ASN_INTEGER, (void *)&temp_state},
            {{1,3,6,1,4,1,3854,1,2,2,1,16,1,7,i}, 15, ASN_INTEGER, (void *)&temp_warn},
            {{1,3,6,1,4,1,3854,1,2,2,1,16,1,8,i}, 15, ASN_INTEGER, (void *)&temp_crit},
            {{1,3,6,1,4,1,3854,1,2,2,1,16,1,12,i}, 15, ASN_INTEGER, (void *)&temp_unit},
            {{0}, 0, 0, NULL},
        };

        mp_snmp_query(ss, snmpcmd);

        if (temp_state == 0)
            break;

        switch (temp_state) {
            case 2:
                break;
            case 3:
            case 5:
                state = state == STATE_OK ? STATE_WARNING : state;
                mp_strcat_comma(&output, temp_name);
                mp_strcat_space(&output, "warning");
                break;
            default:
                state = STATE_CRITICAL;
                mp_strcat_comma(&output, temp_name);
                mp_strcat_space(&output, "critical");
        }

        mp_perfdata_int3(temp_name, temp, "", 1, temp_warn, 1, temp_crit, 0,0,0,0);
    }

    /* Query Hum */
    for (i = 0; i < 10; i++) {
        temp_state = 0;
        struct mp_snmp_query_cmd snmpcmd[] = {
            {{1,3,6,1,4,1,3854,1,2,2,1,17,1,1,i}, 15, ASN_OCTET_STR, (void *)&temp_name},
            {{1,3,6,1,4,1,3854,1,2,2,1,17,1,3,i}, 15, ASN_INTEGER, (void *)&temp},
            {{1,3,6,1,4,1,3854,1,2,2,1,17,1,4,i}, 15, ASN_INTEGER, (void *)&temp_state},
            {{1,3,6,1,4,1,3854,1,2,2,1,17,1,7,i}, 15, ASN_INTEGER, (void *)&temp_warn},
            {{1,3,6,1,4,1,3854,1,2,2,1,17,1,8,i}, 15, ASN_INTEGER, (void *)&temp_crit},
            {{0}, 0, 0, NULL},
        };

        mp_snmp_query(ss, snmpcmd);

        if (temp_state == 0)
            break;

        switch (temp_state) {
            case 2:
                break;
            case 3:
            case 5:
                state = state == STATE_OK ? STATE_WARNING : state;
                mp_strcat_comma(&output, temp_name);
                mp_strcat_space(&output, "warning");
                break;
            default:
                state = STATE_CRITICAL;
                mp_strcat_comma(&output, temp_name);
                mp_strcat_space(&output, "critical");
        }

        mp_perfdata_int3(temp_name, temp, "%", 1, temp_warn, 1, temp_crit, 1,0,1,100);
    }

    mp_snmp_deinit();

    switch (state) {
        case STATE_OK:
            ok("AKCP");
        case STATE_WARNING:
            warning("AKCP %s", output);
        case STATE_CRITICAL:
            critical("AKCP %s", output);
        default:
            unknown("AKCP %s", output);
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
            MP_LONGOPTS_TIMEOUT,
            MP_LONGOPTS_END
    };

    if (argc < 3) {
       print_help();
       exit(STATE_OK);
    }

    mp_snmp_version = SNMP_VERSION_1;


    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"t:H:p:I:ds:"SNMP_OPTSTR, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_snmp(c);

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            /* Hostname opt */
            case 'H':
                getopt_host(optarg, &hostname);
                break;
            /* Port opt */
            case 'P':
                getopt_port(optarg, &port);
                break;
            /* Plugin opt */
            /* Timeout opt */
            case 't':
                getopt_timeout(optarg);
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

    printf(" -I, --interface=[INDEX]\n");
    printf("      Index of Interface to check.\n");
    printf(" -d, --down\n");
    printf("      Check for interface being down.\n");
    printf(" -sm --should=[STATE]\n");
    printf("      Check for interface being in STATE.\n");

    print_help_snmp();
}

/* vim: set ts=4 sw=4 et syn=c : */

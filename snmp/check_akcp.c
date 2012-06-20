/***
 * Monitoring Plugin - check_akcp.c
 **
 *
 * check_akcp - Check akcp sensor by snmp
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

const char *progname  = "check_akcp";
const char *progdesc  = "Check akcp sensor by snmp.";
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
int         sensorport = -1;
char        *degreeeUnit[] = { "F", "C" };

int main (int argc, char **argv) {
    /* Local Vars */
    int             i;
    int             last = 10;
    int             state = STATE_OK;
    char            *output = NULL;
    char            *buf;
    netsnmp_session *ss;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    ss = mp_snmp_init();

    if (sensorport == -1)
        sensorport = 0;
    else
        last = sensorport + 1;

    buf = mp_malloc(32);

    /* Query Temp */
    for (i = sensorport; i < last; i++) {
        long int temp;
        long int temp_state = 0;
        long int temp_warn, temp_crit;
        long int temp_unit;

        struct mp_snmp_query_cmd snmpcmd[] = {
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
                mp_snprintf(buf, 32, "Temperature%d: %ld%s", i+1, temp, degreeeUnit[temp_unit]);
                break;
            case 3:
            case 5:
                state = state == STATE_OK ? STATE_WARNING : state;
                mp_snprintf(buf, 32, "Warning Temperature%d: %ld%s", i+1, temp, degreeeUnit[temp_unit]);
                break;
            default:
                state = STATE_CRITICAL;
                mp_snprintf(buf, 32, "Critical Temperature%d: %ld%s", i+1, temp, degreeeUnit[temp_unit]);
        }

        mp_strcat_comma(&output, buf);

        mp_snprintf(buf, 32, "temp%d", i+1);
        mp_perfdata_int3(buf, temp, degreeeUnit[temp_unit], 1, temp_warn, 1, temp_crit, 0,0,0,0);
    }

    /* Query Hum */
    for (i = sensorport; i < last; i++) {
        long int    hum;
        long int    hum_state = 0;
        long int    hum_warn, hum_crit;

        struct mp_snmp_query_cmd snmpcmd[] = {
            {{1,3,6,1,4,1,3854,1,2,2,1,17,1,3,i}, 15, ASN_INTEGER, (void *)&hum},
            {{1,3,6,1,4,1,3854,1,2,2,1,17,1,4,i}, 15, ASN_INTEGER, (void *)&hum_state},
            {{1,3,6,1,4,1,3854,1,2,2,1,17,1,7,i}, 15, ASN_INTEGER, (void *)&hum_warn},
            {{1,3,6,1,4,1,3854,1,2,2,1,17,1,8,i}, 15, ASN_INTEGER, (void *)&hum_crit},
            {{0}, 0, 0, NULL},
        };

        mp_snmp_query(ss, snmpcmd);

        if (hum_state == 0)
            break;

        switch (hum_state) {
            case 2:
                mp_snprintf(buf, 32, "Humidity%d: %ld%%", i+1, hum);
                break;
            case 3:
            case 5:
                state = state == STATE_OK ? STATE_WARNING : state;
                mp_snprintf(buf, 32, "Warning Humidity%d: %ld%%", i+1, hum);
                break;
            default:
                state = STATE_CRITICAL;
                mp_snprintf(buf, 32, "Critical Humidity%d: %ld%%", i+1, hum);
        }

        mp_strcat_comma(&output, buf);

        mp_snprintf(buf, 32, "hum%d", i+1);
        mp_perfdata_int3(buf, hum, "%", 1, hum_warn, 1, hum_crit, 1,0,1,100);
    }

    free(buf);

    mp_snmp_deinit();

    switch (state) {
        case STATE_OK:
            ok("AKCP - %s", output);
        case STATE_WARNING:
            warning("AKCP - %s", output);
        case STATE_CRITICAL:
            critical("AKCP - %s", output);
        default:
            unknown("AKCP - %s", output);
    }
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
            MP_LONGOPTS_DEFAULT,
            MP_LONGOPTS_HOST,
            MP_LONGOPTS_PORT,
            {"sensor", required_argument, NULL, (int)'s'},
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
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"t:H:p:s:"SNMP_OPTSTR, longopts, &option);

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
            case 's':
                sensorport = (int)strtol(optarg, NULL, 10);
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

    printf(" -s, --sensor=[INDEX]\n");
    printf("      Index of the sensor to check.\n");

    print_help_snmp();
}

/* vim: set ts=4 sw=4 et syn=c : */

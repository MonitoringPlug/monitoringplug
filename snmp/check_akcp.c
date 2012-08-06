/***
 * Monitoring Plugin - check_akcp.c
 **
 *
 * check_akcp - Check akcp sensor by snmp.
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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
/* Library Includes */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#define MP_LONGOPT_TEMP MP_LONGOPT_PRIV0
#define MP_LONGOPT_HUM  MP_LONGOPT_PRIV1

/* Global Vars */
const char  *hostname = NULL;
int         port = 161;
int         sensor_name = 0;
int         sensor_temp = -1;
int         sensor_hum = -1;
int         sensor_found = 0;
int         *sensor = NULL;
int         sensors = 0;
char        *degreeeUnit[] = { "F", "C" };

int main (int argc, char **argv) {
    /* Local Vars */
    int             i;
    int             idx = 0;
    int             state = STATE_OK;
    char            *output = NULL;
    char            *buf;
    netsnmp_session *ss;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    ss = mp_snmp_init();

    /* Loop over sensor ports */
    for (i=0; i<(sensors?sensors:10); i++) {
        char *name = NULL;
        int found = 0;
        long int sensor_state = 0;
        long int sensor_online = 0;

        // Set index
        idx = sensor ? sensor[i]-1 : i;

        /* Query Temp */
        if (sensor_temp != 0) {
            long int temp;
            long int temp_warn_high, temp_crit_high;
            long int temp_warn_low, temp_crit_low;
            long int temp_unit;

            mp_snmp_query_cmd snmpcmd[] = {
                {{1,3,6,1,4,1,3854,1,2,2,1,16,1,1,idx}, 15,
                    ASN_OCTET_STR, (void *)&name, 0},
                {{1,3,6,1,4,1,3854,1,2,2,1,16,1,3,idx}, 15,
                    ASN_INTEGER,   (void *)&temp, sizeof(long int)},
                {{1,3,6,1,4,1,3854,1,2,2,1,16,1,4,idx}, 15,
                    ASN_INTEGER,   (void *)&sensor_state, sizeof(long int)},
                {{1,3,6,1,4,1,3854,1,2,2,1,16,1,5,idx}, 15,
                    ASN_INTEGER,   (void *)&sensor_online, sizeof(long int)},
                {{1,3,6,1,4,1,3854,1,2,2,1,16,1,7,idx}, 15,
                    ASN_INTEGER,   (void *)&temp_warn_high, sizeof(long int)},
                {{1,3,6,1,4,1,3854,1,2,2,1,16,1,8,idx}, 15,
                    ASN_INTEGER,   (void *)&temp_crit_high, sizeof(long int)},
                {{1,3,6,1,4,1,3854,1,2,2,1,16,1,9,idx}, 15,
                    ASN_INTEGER,   (void *)&temp_warn_low, sizeof(long int)},
                {{1,3,6,1,4,1,3854,1,2,2,1,16,1,10,idx}, 15,
                    ASN_INTEGER,   (void *)&temp_crit_low, sizeof(long int)},
                {{1,3,6,1,4,1,3854,1,2,2,1,16,1,12,idx}, 15,
                    ASN_INTEGER,   (void *)&temp_unit, sizeof(long int)},
                {{0}, 0, 0, NULL},
            };

            if(mp_verbose)
                printf("Query Temp Sensor %d\n", idx);

            if(mp_snmp_query(ss, snmpcmd) != STAT_SUCCESS)
                break;

            // Break on unavailable sensor in all sensor mode
            if (sensors == 0 && sensor_online == 0)
                break;

            // Get string of sensor name.
            if (sensor_name == 0) {
                if (name)
                    free(name);
                mp_asprintf(&name, "%d", i+1);
            }

            if (sensor_online == 1) { // Online
                // Check state
                switch (sensor_state) {
                    case 2:
                        mp_asprintf(&buf, "Temperature %s: %ld%s",
                                name, temp, degreeeUnit[temp_unit]);
                        break;
                    case 3:
                    case 5:
                        state = state == STATE_OK ? STATE_WARNING : state;
                        mp_asprintf(&buf, "Warning Temperature %s: %ld%s",
                                name, temp, degreeeUnit[temp_unit]);
                        break;
                    default:
                        state = STATE_CRITICAL;
                        mp_asprintf(&buf, "Critical Temperature %s: %ld%s",
                                name, temp, degreeeUnit[temp_unit]);
                }
                mp_strcat_comma(&output, buf);

                if (mp_showperfdata) {
                    thresholds *threshold = NULL;

                    //Build threshold
                    threshold = mp_malloc(sizeof(thresholds));
                    threshold->warning = mp_malloc(sizeof(range));
                    memset(threshold->warning, 0, sizeof(range));
                    threshold->critical = mp_malloc(sizeof(range));
                    memset(threshold->critical, 0, sizeof(range));

                    threshold->critical->start = temp_crit_low;
                    threshold->warning->start = temp_warn_low;
                    threshold->warning->end = temp_warn_high;
                    threshold->critical->end = temp_crit_high;

                    mp_asprintf(&buf, "temp_%s", name);

                    mp_perfdata_int(buf, temp, degreeeUnit[temp_unit], threshold);

                    free_threshold(threshold);
                    free(buf);
                }

                found++;
            } else if (sensor_online == 2) { // Offline
                if (sensors > 0 &&  sensor_temp > 0) {
                    state = state == STATE_OK ? STATE_UNKNOWN : state;
                    mp_asprintf(&buf, "Temperature %s: offline", name);
                    mp_strcat_comma(&output, buf);
                    free(buf);
                } else {
                    if(mp_verbose)
                        printf(" skip offline\n");
                }
            } else { //Unknown
                if (sensors > 0 &&  sensor_temp > 0) {
                    state = state == STATE_OK ? STATE_UNKNOWN : state;
                    mp_asprintf(&buf, "Temperature %s: not available", name);
                    mp_strcat_comma(&output, buf);
                    free(buf);
                }
            }

        }
        if(name)
            free(name);

        /* Query Hum */
        name = NULL;
        sensor_state = 0;
        sensor_online = 0;
        if (sensor_hum != 0) {
            long int    hum;
            long int    hum_warn_high, hum_crit_high;
            long int hum_warn_low, hum_crit_low;

            mp_snmp_query_cmd snmpcmd[] = {
                {{1,3,6,1,4,1,3854,1,2,2,1,17,1,1,idx}, 15,
                    ASN_OCTET_STR, (void *)&name},
                {{1,3,6,1,4,1,3854,1,2,2,1,17,1,3,idx}, 15,
                    ASN_INTEGER,   (void *)&hum, sizeof(long int)},
                {{1,3,6,1,4,1,3854,1,2,2,1,17,1,4,idx}, 15,
                    ASN_INTEGER,   (void *)&sensor_state, sizeof(long int)},
                {{1,3,6,1,4,1,3854,1,2,2,1,17,1,5,idx}, 15,
                    ASN_INTEGER,   (void *)&sensor_online, sizeof(long int)},
                {{1,3,6,1,4,1,3854,1,2,2,1,17,1,7,idx}, 15,
                    ASN_INTEGER,   (void *)&hum_warn_high, sizeof(long int)},
                {{1,3,6,1,4,1,3854,1,2,2,1,17,1,8,idx}, 15,
                    ASN_INTEGER,   (void *)&hum_crit_high, sizeof(long int)},
                {{1,3,6,1,4,1,3854,1,2,2,1,17,1,9,idx}, 15,
                    ASN_INTEGER,   (void *)&hum_warn_low, sizeof(long int)},
                {{1,3,6,1,4,1,3854,1,2,2,1,17,1,10,idx}, 15,
                    ASN_INTEGER,   (void *)&hum_crit_low, sizeof(long int)},
                {{0}, 0, 0, NULL},
            };

            if(mp_verbose)
                printf("Query Hum Sensor %d\n", idx);

            if(mp_snmp_query(ss, snmpcmd) != STAT_SUCCESS)
                break;

            // Break on unavailable sensor in all sensor mode
            if (sensors == 0 && sensor_online == 0)
                break;

            // Get string of sensor name.
            if (sensor_name == 0) {
                if (name)
                    free(name);
                mp_asprintf(&name, "%d", i+1);
            }

            if (sensor_online == 1) { // Online
                // Check state
                switch (sensor_state) {
                    case 2:
                        mp_asprintf(&buf, "Humidity %s: %ld%%", name, hum);
                        break;
                    case 3:
                    case 5:
                        state = state == STATE_OK ? STATE_WARNING : state;
                        mp_asprintf(&buf, "Warning Humidity %s: %ld%%",
                                name, hum);
                        break;
                    default:
                        state = STATE_CRITICAL;
                        mp_asprintf(&buf, "Critical Humidity %s: %ld%%",
                                name, hum);
                }

                mp_strcat_comma(&output, buf);

                if (mp_showperfdata) {
                    thresholds *threshold = NULL;

                    //Build threshold
                    threshold = mp_malloc(sizeof(thresholds));
                    threshold->warning = mp_malloc(sizeof(range));
                    memset(threshold->warning, 0, sizeof(range));
                    threshold->critical = mp_malloc(sizeof(range));
                    memset(threshold->critical, 0, sizeof(range));

                    threshold->critical->start = hum_crit_low;
                    threshold->warning->start = hum_warn_low;
                    threshold->warning->end = hum_warn_high;
                    threshold->critical->end = hum_crit_high;

                    mp_asprintf(&buf, "hum_%s", name);

                    mp_perfdata_int(buf, hum, "%", threshold);

                    free_threshold(threshold);
                    free(buf);
                }

                found++;
            } else if (sensor_online == 2) { // Offline
                if (sensors > 0 &&  sensor_temp > 0) {
                    state = state == STATE_OK ? STATE_UNKNOWN : state;
                    mp_asprintf(&buf, "Humidity %s: offline", name);
                    mp_strcat_comma(&output, buf);
                    free(buf);
                } else {
                    if(mp_verbose)
                        printf(" skip offline\n");
                }
            } else { //Unknown
                if (sensors > 0 &&  sensor_temp > 0) {
                    state = state == STATE_OK ? STATE_UNKNOWN : state;
                    mp_asprintf(&buf, "Humidity %s: not available", name);
                    mp_strcat_comma(&output, buf);
                    free(buf);
                }
            }
        }

        if (sensors > 0 && found == 0) {
            state = state == STATE_OK ? STATE_UNKNOWN : state;
            mp_asprintf(&buf, "Humidity %s: not available", name);
            mp_strcat_comma(&output, buf);
            free(buf);
        }

        sensor_found += found;
    }

    mp_snmp_deinit();

    if (sensor_found == 0)
        unknown("No Sensors found.");

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
            {"name", no_argument, NULL, (int)'n'},
            {"temp", no_argument, NULL, MP_LONGOPT_TEMP},
            {"hum", no_argument, NULL, MP_LONGOPT_HUM},
            SNMP_LONGOPTS,
            MP_LONGOPTS_END
    };

    if (argc < 3) {
       print_help();
       exit(STATE_OK);
    }

    mp_snmp_version = SNMP_VERSION_1;


    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"H:P:s:n"SNMP_OPTSTR, longopts, &option);

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
            case 's':
                mp_array_push_int(&sensor, optarg, &sensors);
                break;
            case 'n':
                sensor_name = 1;
                break;
            case MP_LONGOPT_TEMP:
                sensor_temp = 1;
                break;
            case MP_LONGOPT_HUM:
                sensor_hum = 1;
                break;
        }
    }

    // Apply defaults
    if (sensor_temp > 0 ||  sensor_hum > 0) {
        sensor_temp++;
        sensor_hum++;
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
    printf(" -n, --name\n");
    printf("      Print names of sensors.\n");
    printf("     --temp\n");
    printf("      Check Temperature sensors.\n");
    printf("     --hum\n");
    printf("      Check Humidity sensors.\n");

    print_help_snmp();
}

/* vim: set ts=4 sw=4 et syn=c : */

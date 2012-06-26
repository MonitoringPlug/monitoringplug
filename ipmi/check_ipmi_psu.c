/***
 * Monitoring Plugin - check_ipmi_psu.c
 **
 *
 * check_ipmi_sensor - Check PSU by IPMI sensor.
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

const char *progname  = "check_ipmi_psu";
const char *progdesc  = "Check PSU by IPMI sensor.";
const char *progvers  = "0.1";
const char *progcopy  = "2012";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "";

/* MP Includes */
#include "mp_common.h"
#include "ipmi_utils.h"
/* Default Includes */
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char **argv) {
    /* Local Vars */
    int state = STATE_OK;
    int i=0;
    struct mp_ipmi_sensor_list *s = NULL;
    char *psu_critical = NULL;
    char *redundancy = NULL;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    mp_ipmi_init();

    char *buf;
    buf = mp_malloc(32);

    for (s=mp_ipmi_sensors; s; s=s->next) {

        if (ipmi_sensor_get_sensor_type(s->sensor) == IPMI_SENSOR_TYPE_POWER_SUPPLY) {
            if (mp_verbose > 0) {
                printf("%s:\n", s->name);
                for (i=0; i< 16; i++) {
                    if (ipmi_is_state_set(s->states, i))
                        printf(" %d: %s\n", i, ipmi_sensor_reading_name_string(s->sensor, i));
                }
            }
            if (!ipmi_is_state_set(s->states, 0)) {
                state = STATE_CRITICAL;
                mp_strcat_comma(&psu_critical, s->name);
                for (i=1; i< 16; i++)
                    if (ipmi_is_state_set(s->states, i))
                        mp_strcat_space(&psu_critical,
                                (char *)ipmi_sensor_reading_name_string(s->sensor, i));
            }
        }
        if (ipmi_sensor_get_sensor_type(s->sensor) == IPMI_SENSOR_TYPE_POWER_UNIT &&
                ipmi_sensor_get_event_reading_type(s->sensor) == IPMI_EVENT_READING_TYPE_DISCRETE_REDUNDANCY) {
            if (ipmi_is_state_set(s->states, 0)) {
                redundancy = (char *)ipmi_sensor_reading_name_string(s->sensor, i);
            } else {
                state = STATE_CRITICAL;
                for (i=1; i< 16; i++) {
                    if (ipmi_is_state_set(s->states, i)) {
                        redundancy = (char *)ipmi_sensor_reading_name_string(s->sensor, i);
                        break;
                    }
                }
            }

            if (mp_verbose > 0) {
                printf("%s:\n", s->name);
                for (i=0; i< 16; i++) {
                    if (ipmi_is_state_set(s->states, i)) 
                        printf(" %d: %s\n", i, ipmi_sensor_reading_name_string(s->sensor, i));
                }
            }
        }

    }

    free(buf);

    mp_ipmi_deinit();

    /* Output and return */
    if (redundancy) {
        // With Redondancy
        if (state == STATE_OK)
            ok("IPMI PSU: %s", redundancy);
        else
            critical("IPMI PSU: %s (%s)", redundancy, psu_critical);
    } else {
        if (state == STATE_OK)
            ok("IPMI PSU");
        else
            critical("IPMI PSU: %s", psu_critical);
    }
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
            MP_LONGOPTS_DEFAULT,
            MP_LONGOPTS_TIMEOUT,
            MP_LONGOPTS_END
    };

    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"t:S:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
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
    print_revision_ipmi();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();

    print_help_default();

}

/* vim: set ts=4 sw=4 et syn=c : */

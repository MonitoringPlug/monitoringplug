/***
 * Monitoring Plugin - check_ipmi_psu.c
 **
 *
 * check_ipmi_psu - Check one or all PSU by IPMI sensor.
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
const char *progdesc  = "Check one or all PSU by IPMI sensor.";
const char *progvers  = "0.1";
const char *progcopy  = "2012";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "";

/* MP Includes */
#include "mp_common.h"
#include "ipmi_utils.h"
/* Default Includes */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* Global Vars */
char *psu  = NULL;

int main (int argc, char **argv) {
    /* Local Vars */
    int state = STATE_OK;
    int i=0;
    int added=0;
    int entity=0;
    struct mp_ipmi_sensor_list *s = NULL;
    char *psu_critical = NULL;
    char *redundancy = NULL;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    mp_ipmi_init();

    char *buf;
    buf = mp_malloc(32);

    for (s=mp_ipmi_sensors; s; s=s->next) {
        entity = ipmi_sensor_get_entity_id(s->sensor);
        if (entity != IPMI_ENTITY_ID_POWER_SUPPLY &&
                entity != IPMI_ENTITY_ID_POWER_MANAGEMENT_BOARD) {
            continue;
        }
        added = 0;

        // Skip other PSUs
        if(psu && strncmp(psu, s->name, strlen(psu)) != 0)
            continue;

        if (ipmi_sensor_get_sensor_type(s->sensor) == IPMI_SENSOR_TYPE_POWER_SUPPLY) {
            if (mp_verbose > 0) {
                printf("%s:\n", s->name);
                for (i=0; i< 16; i++) {
                    if (ipmi_is_state_set(s->states, i))
                        printf(" %d: %s\n", i, ipmi_sensor_reading_name_string(s->sensor, i));
                }
            }
            // Check Missing PSU
            if (!ipmi_is_state_set(s->states, 0)) {
                state = STATE_CRITICAL;
                mp_strcat_comma(&psu_critical, s->name);
                mp_strcat_space(&psu_critical, "not present");
                added = 1;
            }
            // Check mallfunctions
            for (i=1; i<16; i++) {
                if (ipmi_is_state_set(s->states, i)) {
                    state = STATE_CRITICAL;
                    if (added == 0)
                        mp_strcat_comma(&psu_critical, s->name);
                    mp_strcat_space(&psu_critical,
                            (char *)ipmi_sensor_reading_name_string(s->sensor, i));
                    added = 1;
                }
            }
        }
        if (ipmi_sensor_get_sensor_type(s->sensor) == IPMI_SENSOR_TYPE_POWER_UNIT &&
                ipmi_sensor_get_event_reading_type(s->sensor) == IPMI_EVENT_READING_TYPE_DISCRETE_REDUNDANCY) {
            if (ipmi_is_state_set(s->states, 0)) {
                redundancy = (char *)ipmi_sensor_reading_name_string(s->sensor, i);
            } else {
                state = STATE_CRITICAL;
                for (i=1; i<16; i++) {
                    if (ipmi_is_state_set(s->states, i)) {
                        redundancy = (char *)ipmi_sensor_reading_name_string(s->sensor, i);
                        break;
                    }
                }
                if(!redundancy)
                    redundancy = "non-redundant";
            }

            if (mp_verbose > 0) {
                printf("%s:\n", s->name);
                for (i=0; i<16; i++) {
                    if (ipmi_is_state_set(s->states, i)) 
                        printf(" %d: %s\n", i, ipmi_sensor_reading_name_string(s->sensor, i));
                }
            }
        }

        // Add perfdata if needed
        if (!mp_showperfdata)
            continue;

        if (ipmi_sensor_get_base_unit(s->sensor) == IPMI_UNIT_TYPE_WATTS) {
            mp_perfdata_float(s->name, s->value,
                    ipmi_sensor_get_base_unit_string(s->sensor),
                    s->sensorThresholds);
        }

        if (ipmi_sensor_get_sensor_type(s->sensor) == IPMI_SENSOR_TYPE_TEMPERATURE) {
            mp_perfdata_float(s->name, s->value,
                    ipmi_sensor_get_base_unit_string(s->sensor),
                    s->sensorThresholds);
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
            IPMI_LONGOPTS,
            {"psu", required_argument, NULL, (int)MP_LONGOPT_PRIV1}, 
            MP_LONGOPTS_END
    };

    while (1) {
        c = mp_getopt(argc, argv, MP_OPTSTR_DEFAULT""IPMI_OPTSTR, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_ipmi(c);

        switch (c) {
            /* Plugin opt */
            case MP_LONGOPT_PRIV1:
                psu = optarg;
                break;
        }
    }

    /* Check requirements */
#if OS_LINUX
    if (!mp_ipmi_hostname && !mp_ipmi_smi)
        usage("Hostname or SMI is mandatory.");
#else
    if (!mp_ipmi_hostname)
        usage("Hostname is mandatory.");
#endif

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

    print_help_ipmi();

    printf("     --psu=[PSUNAME]\n");
    printf("      Name of a PSU to check.\n");

}

/* vim: set ts=4 sw=4 et syn=c : */

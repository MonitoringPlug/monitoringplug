/***
 * Monitoring Plugin - check_ipmi_sensor.c
 **
 *
 * check_ipmi_sensor - Check the give or all IPMI Sensors.
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

const char *progname  = "check_ipmi_sensor";
const char *progdesc  = "Check the give or all IPMI Sensors.";
const char *progvers  = "0.1";
const char *progcopy  = "2012";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[-S <SENSOR[,SENSOR]>]";

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

/* Global Vars */
const char  *hostname = NULL;
const char  *port = "623";
char        **sensor  = NULL;
int         sensors = 0;

int main (int argc, char **argv) {
    /* Local Vars */
    int state = STATE_OK;
    int lstate;
    struct mp_ipmi_sensor_list *s = NULL;
    int i;
    char *sensor_ok = NULL;
    char *sensor_warning = NULL;
    char *sensor_critical = NULL;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    mp_ipmi_readingtype = IPMI_EVENT_READING_TYPE_THRESHOLD;
    mp_ipmi_init();

    char *buf;
    buf = mp_malloc(63);

    for (i=0; i<sensors; i++) {
        for (s=mp_ipmi_sensors; s; s=s->next) {
            if (strcmp(s->name, sensor[i]) != 0)
                continue;
            if (mp_verbose > 0)
                printf("%s: %f\n", s->name, s->value);

            mp_perfdata_float(s->name, s->value,
                    ipmi_sensor_get_base_unit_string(s->sensor),
                    s->sensorThresholds);

            lstate = get_status(s->value, s->sensorThresholds);
            if (lstate > state)
                state = lstate;

            mp_snprintf(buf, 63, "%s %.2f%s", s->name, s->value,
                    ipmi_sensor_get_base_unit(s->sensor) ? 
                    ipmi_sensor_get_base_unit_string(s->sensor) : "");
            if (lstate == STATE_OK)
                mp_strcat_comma(&sensor_ok, buf);
            else if (lstate == STATE_WARNING)
                mp_strcat_comma(&sensor_warning, buf);
            else
                mp_strcat_comma(&sensor_critical, buf);
            break;
        }
        if (!s) {
            state = STATE_CRITICAL;
            mp_strcat_comma(&sensor_critical, sensor[i]);
        }
    }

    if (sensors == 0) {
        for (s=mp_ipmi_sensors; s; s=s->next) {
            if (ipmi_sensor_get_event_reading_type(s->sensor) != IPMI_EVENT_READING_TYPE_THRESHOLD)
                continue;

            if (mp_verbose > 0)
                printf("%s: %f\n", s->name, s->value);

            mp_perfdata_float(s->name, s->value,
                    ipmi_sensor_get_base_unit_string(s->sensor),
                    s->sensorThresholds);

            lstate = get_status(s->value, s->sensorThresholds);
            if (lstate > state)
                state = lstate;

            mp_snprintf(buf, 63, "%s %.2f%s", s->name, s->value,
                    ipmi_sensor_get_base_unit(s->sensor) ?
                    ipmi_sensor_get_base_unit_string(s->sensor) : "");
            if (lstate == STATE_OK)
                mp_strcat_comma(&sensor_ok, buf);
            else if (lstate == STATE_WARNING)
                mp_strcat_comma(&sensor_warning, buf);
            else
                mp_strcat_comma(&sensor_critical, buf);
        }
    }

    free(buf);

    mp_ipmi_deinit();

    /* Output and return */
    if (state == STATE_OK)
        ok("IPMI Sensors: %s", sensor_ok);
    else if (state == STATE_WARNING)
        warning("IPMI Sensors: %s", sensor_warning);
    critical("IPMI Sensors: %s", sensor_critical);
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
            MP_LONGOPTS_DEFAULT,
            MP_LONGOPTS_HOST,
            MP_LONGOPTS_PORT,
            IPMI_LONGOPTS,
            {"sensor", required_argument, NULL, (int)'S'},
            MP_LONGOPTS_TIMEOUT,
            MP_LONGOPTS_END
    };

    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"t:S:H:P:"IPMI_OPTSTR, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_ipmi(c);

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            /* Hostname opt */
            case 'H': 
                getopt_host(optarg, &hostname);
                break;
            /* Port opt */
            case 'P': 
                port = optarg;
                break;
            /* Plugin opt */
            case 'S':
                mp_array_push(&sensor, optarg, &sensors);
                break;
            /* Timeout opt */
            case 't':
                getopt_timeout(optarg);
                break;
        }
    }

    /* Check */
    if (hostname && mp_ipmi_smi != -1)
        usage("hostname and smi can not be used together.");
    if (mp_ipmi_smi == -1)
        mp_ipmi_smi = 0;

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

    printf(" -S, --sensor=[SENSOR]\n");
    printf("      Name of a Sensor to check, can be repeated.\n");

}

/* vim: set ts=4 sw=4 et syn=c : */

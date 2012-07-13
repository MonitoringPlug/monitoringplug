/***
 * Monitoring Plugin - check_ipmi_fan.c
 **
 *
 * check_ipmi_fan - Check the give or all FANs by IPMI.
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

const char *progname  = "check_ipmi_fan";
const char *progdesc  = "Check the give or all FANs by IPMI.";
const char *progvers  = "0.1";
const char *progcopy  = "2012";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[--fan <FAN[,FAN]>]";

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
char        **fan  = NULL;
int         fans = 0;

int main (int argc, char **argv) {
    /* Local Vars */
    int state = STATE_OK;
    int lstate;
    struct mp_ipmi_sensor_list *s = NULL;
    int i;
    char *out_ok = NULL;
    char *out_warning = NULL;
    char *out_critical = NULL;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    mp_ipmi_readingtype = IPMI_EVENT_READING_TYPE_THRESHOLD;
    mp_ipmi_entity = IPMI_ENTITY_ID_FAN_COOLING;
    mp_ipmi_init();

    char *buf;
    buf = mp_malloc(63);

    for (i=0; i<fans; i++) {
        for (s=mp_ipmi_sensors; s; s=s->next) {
            if (strcmp(fan[i], s->name) != 0)
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
                mp_strcat_comma(&out_ok, buf);
            else if (lstate == STATE_WARNING)
                mp_strcat_comma(&out_warning, buf);
            else
                mp_strcat_comma(&out_critical, buf);
            break;
        }
        if (!s) {
            state = STATE_CRITICAL;
            mp_strcat_comma(&out_critical, fan[i]);
        }
    }

    if (fans == 0) {
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
                mp_strcat_comma(&out_ok, buf);
            else if (lstate == STATE_WARNING)
                mp_strcat_comma(&out_warning, buf);
            else
                mp_strcat_comma(&out_critical, buf);
        }
    }

    free(buf);

    mp_ipmi_deinit();

    /* Output and return */
    if (state == STATE_OK)
        ok("IPMI Fan: %s", out_ok);
    else if (state == STATE_WARNING)
        warning("IPMI Fan: %s", out_warning);
    critical("IPMI Fan: %s", out_critical);
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
            MP_LONGOPTS_DEFAULT,
            IPMI_LONGOPTS,
            {"fan", required_argument, NULL, (int)'F'},
            MP_LONGOPTS_END
    };

    while (1) {
        c = mp_getopt(argc, argv, MP_OPTSTR_DEFAULT"F:"IPMI_OPTSTR, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_ipmi(c);

        switch (c) {
            /* Plugin opt */
            case 'F':
                mp_array_push(&fan, optarg, &fans);
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

    printf(" -F, --fan=[FAN]\n");
    printf("      Name of a Fan to check, can be repeated.\n");

}

/* vim: set ts=4 sw=4 et syn=c : */

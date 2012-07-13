/***
 * Monitoring Plugin - check_ipmi_mem.c
 **
 *
 * check_ipmi_mem - Check Memory status by IPMI.
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

const char *progname  = "check_ipmi_memory";
const char *progdesc  = "Check Memory status by IPMI.";
const char *progvers  = "0.1";
const char *progcopy  = "2012";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--dim <DIMM>";

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
char        **dimm  = NULL;
int         dimms = 0;

int main (int argc, char **argv) {
    /* Local Vars */
    int state = STATE_OK;
    struct mp_ipmi_sensor_list *s = NULL;
    int i;
    int added;
    char *dimm_ok = NULL;
    char *dimm_critical = NULL;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    mp_ipmi_entity = IPMI_ENTITY_ID_MEMORY_DEVICE;
    mp_ipmi_init();

    char *buf;
    buf = mp_malloc(63);

    for (i=0; i<dimms; i++) {
        for (s=mp_ipmi_sensors; s; s=s->next) {
            added = 0;

            // Skip all but current dimm
            if (strcmp(s->name, dimm[i]) != 0)
                continue;

            if (mp_verbose > 0) {
                printf("%s\n", s->name);
                for (i=0; i< 16; i++) {
                    if (ipmi_is_state_set(s->states, i))
                        printf(" %d: %s\n", i, ipmi_sensor_reading_name_string(s->sensor, i));
                }
            }

            // Check for failer
            for (i=0; i < 16; i++) {
                //Skip state 2
                if (i == 2)
                    continue;

                if (ipmi_is_state_set(s->states, i)) {
                    state = STATE_CRITICAL;
                    if (added == 0)
                        mp_strcat_comma(&dimm_critical, s->name);
                    mp_strcat_space(&dimm_critical,
                            (char *)ipmi_sensor_reading_name_string(s->sensor, i));
                    added = 1;
                }
            }

            // Add if state 2/ok
            if (added == 0 && ipmi_is_state_set(s->states, 2)) {
                mp_strcat_comma(&dimm_ok, s->name);
            } else {
                state = STATE_CRITICAL;
                if (added == 0)
                    mp_strcat_comma(&dimm_critical, s->name);
                mp_strcat_space(&dimm_critical, "slot empty");
            }
            break;
        }
        if (!s) {
            state = STATE_CRITICAL;
            mp_strcat_comma(&dimm_critical, dimm[i]);
            mp_strcat_space(&dimm_critical, "not found");
        }
    }

    if (dimms == 0) {
        for (s=mp_ipmi_sensors; s; s=s->next) {
            added = 0;

            if (mp_verbose > 0) {
                printf("%s\n", s->name);
                for (i=0; i< 16; i++) {
                    if (ipmi_is_state_set(s->states, i))
                        printf(" %d: %s\n", i, ipmi_sensor_reading_name_string(s->sensor, i));
                }
            }

            // Check for error
            for (i=0; i < 16; i++) {
                //Skip state 2
                if (i == 2)
                    continue;

                if (ipmi_is_state_set(s->states, i)) {
                    state = STATE_CRITICAL;
                    if (added == 0)
                        mp_strcat_comma(&dimm_critical, s->name);
                    mp_strcat_space(&dimm_critical,
                            (char *)ipmi_sensor_reading_name_string(s->sensor, i));
                    added = 1;
                }
            }

            // Add if state 2/ok
            if (added == 0 && ipmi_is_state_set(s->states, 2))
                mp_strcat_comma(&dimm_ok, s->name);
        }
    }

    free(buf);

    mp_ipmi_deinit();

    /* Output and return */
    if (state == STATE_OK)
        ok("IPMI Memory: %s", dimm_ok);
    critical("IPMI Memory: %s", dimm_critical);
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
            MP_LONGOPTS_DEFAULT,
            IPMI_LONGOPTS,
            {"dimm", required_argument, NULL, (int)'D'},
            MP_LONGOPTS_END
    };

    while (1) {
        c = mp_getopt(argc, argv, MP_OPTSTR_DEFAULT"D:"IPMI_OPTSTR, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_ipmi(c);

        switch (c) {
            /* Plugin opt */
            case 'D':
                mp_array_push(&dimm, optarg, &dimms);
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

    printf(" -D, --dimm=DIMM\n");
    printf("      Name of the DIMM to check, can be repeated.\n");

}

/* vim: set ts=4 sw=4 et syn=c : */

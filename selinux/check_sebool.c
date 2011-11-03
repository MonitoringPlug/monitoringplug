/***
 * Monitoring Plugin - check_sebool
 **
 *
 * check_sebool - Check SELinux boolean.
 * Copyright (C) 2010 Marius Rieder <marius.rieder@durchmesser.ch>
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

const char *progname  = "check_sebool";
const char *progvers  = "0.1";
const char *progcopy  = "2011";
const char *progauth = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[--on BOOL] [--off BOOL] [--on|off ...]";

/* MP Includes */
#include "mp_common.h"
/* Default Includes */
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
/* Library Includes */
#include <selinux/selinux.h>

/* Global Vars */
char **bool_on = NULL;
char **bool_off = NULL;
int bools_on = 0;
int bools_off = 0;
#define LONGOPT_ON MP_LONGOPT_PRIV0
#define LONGOPT_OFF MP_LONGOPT_PRIV1

int main (int argc, char **argv) {
    /* Local Vars */
    int     i;
    int     active;
    int     state = STATE_OK;
    char    *buf = NULL;
    char    *bools_ok = NULL;
    char    *bools_crit = NULL;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        exit(STATE_CRITICAL);

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        exit(STATE_CRITICAL);

    /* Start plugin timeout */
    alarm(mp_timeout);

    if (is_selinux_enabled() <= 0) {
        critical("SELinux is disabled!");
    }

    buf = mp_malloc(sizeof(char *) * 128);
    if (!buf)
        critical("Out of Memory!");

    for (i=0; i < bools_on; i++) {
        if (mp_verbose)
            printf("bool_on[%d] => %s\n", i, bool_on[i]);
        active = security_get_boolean_active(bool_on[i]);

        if (active  == 1) {
            mp_snprintf(buf, 128, "%s: ON", bool_on[i]);
            mp_strcat_comma(&bools_ok, buf);
        } else if (active == 0) {
            mp_snprintf(buf, 128, "%s: OFF", bool_on[i]);
            mp_strcat_comma(&bools_crit, buf);
            state = STATE_CRITICAL;
        } else {
            mp_snprintf(buf, 128, "%s: Unknown", bool_on[i]);
            mp_strcat_comma(&bools_crit, buf);
            state = STATE_CRITICAL;
        }
    }

    for (i=0; i < bools_off; i++) {
        if (mp_verbose)
            printf("bool_off[%d] => %s\n", i, bool_off[i]);
        active = security_get_boolean_active(bool_off[i]);

        if (active  == 1) {
            mp_snprintf(buf, 128, "%s: ON", bool_off[i]);
            mp_strcat_comma(&bools_crit, buf);
            state = STATE_CRITICAL;
        } else if (active == 0) {
            mp_snprintf(buf, 128, "%s: OFF", bool_off[i]);
            mp_strcat_comma(&bools_ok, buf);
        } else {
            mp_snprintf(buf, 128, "%s: Unknown", bool_off[i]);
            mp_strcat_comma(&bools_crit, buf);
            state = STATE_CRITICAL;
        }
    }

    mp_strcat_comma(&bools_crit, bools_ok);

    switch (state) {
       case STATE_OK:
           ok("SEBool: %s", bools_crit);
       case STATE_CRITICAL:
           critical("SEBool: %s", bools_crit);
    }

    critical("You should never reach this point.");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        {"on", required_argument, NULL, (int)LONGOPT_ON},
        {"off", required_argument, NULL, (int)LONGOPT_OFF},
        MP_LONGOPTS_END
    };

    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            case LONGOPT_ON:
                mp_array_push(&bool_on, optarg, &bools_on);
                break;
            case LONGOPT_OFF:
                mp_array_push(&bool_off, optarg, &bools_off);
                break;
        }
    }

    /* Check requirements */
    if ((bools_on + bools_off) == 0) {
        usage("Please specify al least one --on or --off bool to check.");
    }

    return(OK);
}

void print_help (void) {
    print_revision();
    print_copyright();

    printf("\n");

    printf("This plugin test the selinux boolean state.");

    printf("\n\n");

    print_usage();

    print_help_default();

    printf("     --on\n");
    printf("      SELinux boolean which should be ON.\n");
    printf("     --off\n");
    printf("      SELinux boolean which should be OFF.\n");
}

/* vim: set ts=4 sw=4 et syn=c : */

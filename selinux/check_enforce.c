/***
 * Monitoring Plugin - check_enforce.c
 **
 *
 * check_enforce - Check the selinux state and policy.
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

const char *progname  = "check_enforce";
const char *progdesc  = "Check the selinux state and policy.";
const char *progvers  = "0.1";
const char *progcopy  = "2011";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[--enforcing|--permissive|--disabled]";

/* MP Includes */
#include "mp_common.h"
/* Default Includes */
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <libgen.h>
/* Library Includes */
#include <selinux/selinux.h>

/* Global Vars */
int state_disabled = -1;
int state_permissive = -1;
int state_enforcing = -1;
char *policy;

int main (int argc, char **argv) {
    /* Local Vars */
    int     se_enabled;
    int     se_enforced;
    char    *state_name;
    int     state;
    char    *pol_name;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        exit(STATE_CRITICAL);

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        exit(STATE_CRITICAL);

    /* Start plugin timeout */
    alarm(mp_timeout);

    se_enabled = is_selinux_enabled();
    if (se_enabled < 0)
       critical("is_selinux_enabled faild!");

    if (se_enabled) {

        se_enforced = security_getenforce();
        if (se_enforced < 0)
            critical("security_getenforce faild!");

	if (se_enforced) {
	    state_name = strdup("Enforcing");
	    state = state_enforcing;
	} else {
	    state_name = strdup("Permissive");
	    state = state_permissive;
	}

	pol_name = basename(strdup(selinux_policy_root()));

	if (policy && strcmp(pol_name, policy) != 0) {
	   critical("Wrong SELinux policy %s", pol_name);
	}
    } else {
       state_name = strdup("Disabled");
       state = state_disabled;

       pol_name = strdup("no policy");
    }

    switch (state) {
       case STATE_OK:
	  ok("SELinux: %s (%s)", state_name, pol_name);
       case STATE_WARNING:
	  warning("SELinux: %s (%s)", state_name, pol_name);
       case STATE_CRITICAL:
	  critical("SELinux: %s (%s)", state_name, pol_name);
    }

    critical("You should never reack this point.");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        {"enforcing", no_argument, NULL, (int)'e'},
        {"permissiv", no_argument, NULL, (int)'p'},
        {"disabled", no_argument, NULL, (int)'d'},
        {"policy", required_argument, NULL, (int)'P'},
        MP_LONGOPTS_END
    };

    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"epdP:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            case 'e':
                if (state_enforcing != -1)
                    usage("Only one state argument allowed.");
                state_enforcing = STATE_OK;
                state_permissive = STATE_WARNING;
                state_disabled = STATE_CRITICAL;
                break;
            case 'p':
                if (state_enforcing != -1)
                    usage("Only one state argument allowed.");
                state_enforcing = STATE_WARNING;
                state_permissive = STATE_OK;
                state_disabled = STATE_CRITICAL;
                break;
            case 'd':
                if (state_enforcing != -1)
                    usage("Only one state argument allowed.");
                state_enforcing = STATE_CRITICAL;
                state_permissive = STATE_WARNING;
                state_disabled = STATE_OK;
                break;
            case 'P':
                policy = optarg;
                break;
        }
    }

    /* Check requirements */
    if (state_enforcing == -1) {
        state_enforcing = STATE_OK;
        state_permissive = STATE_WARNING;
        state_disabled = STATE_CRITICAL;
    }

    return(OK);
}

void print_help (void) {
    print_revision();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();

    print_help_default();

    printf(" -e, --enforcing\n");
    printf("      SELinux should be enforcing.\n");
    printf(" -p, --permissive\n");
    printf("      SELinux should be permissive.\n");
    printf(" -d, --disabled\n");
    printf("      SELinux should be disabled.\n");
    printf(" -P, --policy=POLICY\n");
    printf("      SELinux should run with POLICY loaded.\n");
}

/* vim: set ts=4 sw=4 et syn=c : */

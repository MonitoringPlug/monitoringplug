/***
 * Monitoring Plugin - check_enforce
 **
 *
 * check_enforce - Check SELinux state.
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

const char *progname  = "check_enforce";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[--enforcing|--permissive|--disabled]";

#include "mp_common.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <selinux/selinux.h>

/* Global Vars */
int state_disabled = -1;
int state_permissive = -1;
int state_enforcing = -1;
char *policy;

int main (int argc, char **argv) {
    int se_enabled;
    int se_enforced;
    char *state_name;
    int state;
    char *pol_name;

    if (process_arguments (argc, argv) == 1)
        exit(STATE_CRITICAL);

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

	pol_name = strdup(basename(selinux_policy_root()));

	if (policy && strcmp(pol_name, policy) != 0) {
	   critical("Wrong SELinux policy %s", pol_name);
	}
    } else {
       state_name = strdup("Disabled");
       state = state_disabled;

       pol_name = strdup("no policy");
    }

    pol_name = strdup(basename(selinux_policy_root()));

    switch (state) {
       case STATE_OK:
	  ok("SELinux: %s (%s)", state_name, pol_name);
       case STATE_WARNING:
	  warning("SELinux: %s (%s)", state_name, pol_name);
       case STATE_CRITICAL:
	  critical("SELinux: %s (%s)", state_name, pol_name);
    }

    critical("You should never reack this point. %d %d", se_enabled, se_enforced);
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

        getopt_default(c);

	switch (c) {
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

    printf("This plugin test nothing.");

    printf("\n\n");

    print_usage();

    print_help_default();
}

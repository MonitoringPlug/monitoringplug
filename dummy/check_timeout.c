/**
 * Monitoring Plugin - check_timeout
 **
 *
 * check_timeout - Simulate a plugin timeout.
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
 */

const char *progname  = "check_timeout";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[-t <timeout>]";

#include "mp_common.h"

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char **argv) {
    /* Set signal handling and alarm */
    if (signal (SIGALRM, timeout_alarm_handler) == SIG_ERR)
        exit(STATE_CRITICAL);

    if (process_arguments (argc, argv) == 1)
        exit(STATE_CRITICAL);

    alarm(mp_timeout);

    if (mp_verbose) {
        printf("Timeout: %d\n", mp_timeout);
        printf("Sleep:   %d\n", mp_timeout*2);
    }

    sleep(mp_timeout*2);

    ok("check_timeout ended.");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_ARGS_HELP,
        MP_ARGS_VERS,
        MP_ARGS_VERB,
        MP_ARGS_TIMEOUT,
        MP_ARGS_END
    };

    while (1) {
        c = getopt_long (argc, argv, "hVvt:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        switch (c) {
            MP_ARGS_CASE_DEF
            MP_ARGS_CASE_TIMEOUT
        }
    }

    return(OK);
}

void print_help (void) {
    print_revision();
    print_copyright();

    printf("\n");

    printf("This plugin simulate a plugin timeout.");

    printf("\n\n");

    print_usage();

    print_help_default();
    print_help_timeout();
}


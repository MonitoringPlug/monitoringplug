/***
 * Monitoring Plugin - check_dummy.c
 **
 *
 * check_dummy - This plugin test nothing.
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

const char *progname  = "check_dummy";
const char *progdesc  = "This plugin test nothing.";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "<state> [message]";

/* MP Includes */
#include "mp_common.h"
/* Default Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

int main (int argc, char **argv) {
    /* Local Vars */
    int msglen = 1;
    char *msg;
    char *msgc;
    int c = 0;
    int r = 1;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    if (optind < argc && is_integer(argv[c]) == 1) {
        r = (int)strtol(argv[c], NULL, 10);
        optind++;
    }

    if (r < 0 || r > 4)
        r = 2;

    for (c = optind; c < argc; c++)
        msglen += strlen(argv[c]) + 1;
    msg = (char *) mp_malloc((size_t)msglen);

    if (msg == NULL)
        unknown("Can't allocate memory.");

    msgc = msg;
    for (c = optind; c < argc; c++) {
        strncpy(msgc, argv[c], strlen(argv[c]));
        msgc += strlen(argv[c]);
        *msgc = ' ';
        msgc++;
    }
    msgc--;
    *msgc = '\0';

    if (mp_verbose > 0) {
        printf("State:   %d\n", r);
        printf("Message: %s\n", msg);
    }

    switch ( r ) {
        case STATE_OK:
            ok(msg);
            break;
        case STATE_WARNING:
            warning(msg);
            break;
        case STATE_CRITICAL:
            critical(msg);
            break;
        case STATE_UNKNOWN:
            unknown(msg);
            break;
        case STATE_DEPENDENT:
            unknown(msg);
            break;
    }

    critical("You should never reach this point.");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_END
    };

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"", longopts, &option);

        if (c == -1 || c == EOF)
            break;
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
}

/* vim: set ts=4 sw=4 et syn=c : */

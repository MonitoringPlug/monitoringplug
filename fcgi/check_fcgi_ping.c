/***
 * Monitoring Plugin - check_fcgi_ping.c
 **
 *
 * check_fcgi_ping - This plugin ping a fcgid.
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

const char *progname  = "check_fcgi_ping";
const char *progdesc  = "This plugin ping a fcgid.";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[-t <timeout>]";

/* MP Includes */
#include "mp_common.h"
#include "fcgi_utils.h"
/* Default Includes */
#include <fastcgi.h>
#include <fcgios.h>
#include <fcgiapp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Global Vars */
char *fcgisocket = NULL;

int main (int argc, char **argv) {
    /* Local vars */
    int fcgiSock = -1;
    struct timeval  start_time;
    double          time_delta;


    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);
    gettimeofday(&start_time, NULL);

    /* Connect to FCGI-server */
    fcgiSock = mp_fcgi_connect(fcgisocket);

    /* Management Record query */
    mp_fcgi_write(fcgiSock, 0, FCGI_GET_VALUES, NULL, 0);

    /* Wait for answer */
    int type, count;
    char *content = NULL;
    do {
        type = mp_fcgi_read(fcgiSock, &content, &count);
        free(content);
    } while (type != FCGI_GET_VALUES_RESULT);

    /* Close connection */
    OS_Close(fcgiSock);

    time_delta = mp_time_delta(start_time);
    mp_perfdata_float("time", (float)time_delta, "s", NULL);

    ok("FCGI-Pong: %s", fcgisocket);
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        {"socket", required_argument, NULL, (int)'s'},
        MP_LONGOPTS_END
    };

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"s:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        switch (c) {
            case 's':
                fcgisocket = optarg;
            break;
        }

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

    printf(" -s, --socket=<SOCKET>\n");
    printf("      FastCGID socket to connect to.\n");
}

/* vim: set ts=4 sw=4 et syn=c : */

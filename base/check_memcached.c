/***
 * Monitoring Plugin - check_memcached.c
 **
 *
 * check_memcached - Check memcached status.
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

const char *progname  = "check_memcached";
const char *progdesc  = "Check memcached status.";
const char *progvers  = "0.1";
const char *progcopy  = "2012";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[--host <HOSTNAME>] [--port <PORT>]";

/* MP Includes */
#include "mp_common.h"
#include "mp_utils.h"
#include "mp_net.h"
/* Default Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
/* Library Includes */

/* Global Vars */
const char *hostname = "localhost";
int port = 11211;
int ipv = AF_UNSPEC;
thresholds *time_thresholds = NULL;

int main (int argc, char **argv) {
    /* Local Vars */
    int socket;
    char *line;
    char *key;
    char *value;
    char *mc_version = NULL;
    int mc_bytes = -1;              /** < Memory used now. */
    int mc_limit_maxbytes = -1;     /** < Memory limit configured. */
    struct timeval start_time;
    double time_delta;

    /* Set signal handling and alarm */
    if (signal (SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    // Connect to Server
    gettimeofday(&start_time, NULL);
    socket = mp_connect(hostname, port, ipv, SOCK_STREAM);

    if (mp_verbose > 3)
        printf("> 'stats'\n");
    send(socket, "stats\r\n", 7, 0);

    while (1) {
        line = mp_recv_line(socket);

        if (strncmp(line, "STAT ", 5) == 0) {
            value = line+5;
            key = strsep(&value, " ");

            if (strcmp(key, "version") == 0) {
                mc_version = mp_strdup(value);
            } else if (!mp_showperfdata) {
                // End
            } else if (strcmp(key, "limit_maxbytes") == 0) {
                mc_limit_maxbytes = strtol(value, NULL, 10);
            } else if (strcmp(key, "bytes") == 0) {
                mc_bytes = strtol(value, NULL, 10);
            } else if (strcmp(key, "total_connections") == 0) {
                mp_perfdata_int("connections", strtol(value, NULL, 10),
                        "c", NULL);
            } else if (strcmp(key, "total_items") == 0) {
                mp_perfdata_int("total_items", strtol(value, NULL, 10),
                        "c", NULL);
            } else if (strcmp(key, "curr_items") == 0) {
                mp_perfdata_int("items", strtol(value, NULL, 10),
                        "c", NULL);
            } else if (strcmp(key, "evictions") == 0) {
                mp_perfdata_int("evictions", strtol(value, NULL, 10),
                        "c", NULL);
            } else if (strncmp(key, "cmd_", 4) == 0) {
                mp_perfdata_int(key+4, strtol(value, NULL, 10), "c", NULL);
            }
        } else if (strncmp(line, "END", 3) == 0) {
            break;
        } else {
            critical("Memcached don't handle stats command.");
        }
        free(line);
    }
    free(line);

    // Dissconnect
    send(socket, "quit\r\n", 6, 0);
    mp_disconnect(socket);
    time_delta = mp_time_delta(start_time);

    if (mp_showperfdata) {
        mp_perfdata_int3("bytes", mc_bytes, "", 0, 0, 0, 0,
                1, 0, 1, mc_limit_maxbytes);
        mp_perfdata_float("time", (float)time_delta, "s", time_thresholds);
    }

    switch(get_status(time_delta, time_thresholds)) {
        case STATE_OK:
            free_threshold(time_thresholds);
            ok("Memcached %s", mc_version);
            break;
        case STATE_WARNING:
            free_threshold(time_thresholds);
            ok("Memcached %s is slow.", mc_version);
            break;
        case STATE_CRITICAL:
            free_threshold(time_thresholds);
            ok("Memcached %s is real slow.", mc_version);
            break;
    }
    free_threshold(time_thresholds);

    critical("You should never reach this point.");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        MP_LONGOPTS_PORT,
        // PLUGIN OPTS
        MP_LONGOPTS_END
    };

    /* Set default */
    mp_threshold_set_warning_time(&time_thresholds, "3s");
    mp_threshold_set_critical_time(&time_thresholds, "4s");

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"H:P:46w:c:",
                longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_46(c, &ipv);
        getopt_wc_time(c, optarg, &time_thresholds);

        switch (c) {
            /* Hostname opt */
            case 'H':
                getopt_host(optarg, &hostname);
                break;
            /* Port opt */
            case 'P':
                getopt_port(optarg, &port);
                break;
        }
    }

    /* Check requirements */
    if (!hostname || !port)
        usage("Hostname and port mandatory.");

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
    print_help_host();
    print_help_port("none");
#ifdef USE_IPV6
    print_help_46();
#endif //USE_IPV6
    print_help_warn_time("3s");
    print_help_crit_time("4s");
}

/* vim: set ts=4 sw=4 et syn=c : */

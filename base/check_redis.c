/***
 * Monitoring Plugin - check_redis.c
 **
 *
 * check_redis - Check redis status.
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

const char *progname  = "check_redis";
const char *progdesc  = "Check redis status.";
const char *progvers  = "0.1";
const char *progcopy  = "2013";
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
int port = 6379;
int ipv = AF_UNSPEC;
thresholds *time_thresholds = NULL;
thresholds *memory_thresholds = NULL;

int main (int argc, char **argv) {
    /* Local Vars */
    int socket;
    char *line;
    char *redis_version = NULL;
    int answer_size = 0;
    int used_memory = -1;              /** < Memory used now. */
    int max_memory = -1;               /** < Memory allowed. */
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
        printf("> INFO'\n");
    send(socket, "INFO\r\n", 6, 0);

    line = mp_recv_line(socket);
    if (line[0] != '$') {
        free(line);
        unknown("Redis Server did not respong propperly.");
    }

    answer_size = strtol(line+1, NULL, 10);
    free(line);

    if (mp_verbose > 1)
        printf("Redis Info is %d byte long\n", answer_size);

    while (answer_size > 0) {
        line = mp_recv_line(socket);

        if (strncmp(line, "redis_version:", 14) == 0) {
            redis_version = strdup(line+14);
        } else if (strncmp(line, "used_memory:", 12) == 0) {
            used_memory = strtol(line+12, NULL, 10);
        } else if (strncmp(line, "total_connections_received:", 27) == 0) {
            mp_perfdata_int("connections", strtol(line+27, NULL, 10),
                    "c", NULL);
        } else if (strncmp(line, "total_commands_processed:", 25) == 0) {
            mp_perfdata_int("commands", strtol(line+25, NULL, 10),
                    "c", NULL);
        }

        answer_size -= strlen(line) + 2;

        free(line);
    }
    line = mp_recv_line(socket);
    free(line);

    // Query maxmemory
    if (mp_verbose > 3)
        printf("> CONFIG GET maxmemory\n");
    send(socket, "CONFIG GET maxmemory\r\n", 22, 0);

    line = mp_recv_line(socket);
    if (line[0] != '*') {
        free(line);
        unknown("Redis Server did not respong propperly.");
    }
  
    answer_size = strtol(line+1, NULL, 10);
    free(line);

    while (answer_size > 0) {
        line = mp_recv_line(socket);
        free(line);
        line = mp_recv_line(socket);
        if (strcmp(line, "maxmemory") == 0) {
            free(line);
            line = mp_recv_line(socket);
            free(line);
            line = mp_recv_line(socket);
            max_memory = (int)strtol(line, NULL, 10);
            free(line);
            break;
        }
        free(line);

        answer_size--;
    }

    // Dissconnect
    send(socket, "QUIT\r\n", 6, 0);
    mp_disconnect(socket);
    time_delta = mp_time_delta(start_time);

    if (mp_showperfdata) {
        mp_perfdata_int2("bytes", used_memory, "", memory_thresholds,
                1, 0, max_memory==-1?0:1 , max_memory);
        mp_perfdata_float("time", (float)time_delta, "s", time_thresholds);
    }

    switch(get_status(used_memory, memory_thresholds)) {
        case STATE_WARNING:
            free_threshold(memory_thresholds);
            free_threshold(time_thresholds);
            warning("Redis Memory Usage: %s", mp_human_size(used_memory));
            break;
        case STATE_CRITICAL:
            free_threshold(memory_thresholds);
            free_threshold(time_thresholds);
            critical("Redis Memory Usage: %s", mp_human_size(used_memory));
            break;
    }
    free_threshold(memory_thresholds);

    switch(get_status(time_delta, time_thresholds)) {
        case STATE_OK:
            free_threshold(time_thresholds);
            ok("Redis %s", redis_version);
            break;
        case STATE_WARNING:
            free_threshold(time_thresholds);
            warning("Redis %s is slow.", redis_version);
            break;
        case STATE_CRITICAL:
            free_threshold(time_thresholds);
            critical("Redis %s is real slow.", redis_version);
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
        MP_LONGOPTS_WC,
        // PLUGIN OPTS
        {"warning-memory", required_argument, NULL, (int)'W'},
        {"critical-memory", required_argument, NULL, (int)'C'},
        MP_LONGOPTS_END
    };

    /* Set default */
    setWarnTime(&time_thresholds, "3s");
    setCritTime(&time_thresholds, "4s");

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"H:P:46w:c:W:C:",
                longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_46(c, &ipv);
        getopt_wc_time(c, optarg, &time_thresholds);

        switch (c) {
            /* Plugin Opts */
            case 'W':
                if (setWarn(&memory_thresholds, optarg, BISI) == ERROR)
                    usage("Illegal -W threshold '%s'.", optarg);
                break;
            case 'C':
                if (setCrit(&memory_thresholds, optarg, BISI) == ERROR)
                    usage("Illegal -C threshold '%s'.", optarg);
                break;
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
    print_help_port("6379");
#ifdef USE_IPV6
    print_help_46();
#endif //USE_IPV6
    print_help_warn_time("3s");
    print_help_crit_time("4s");
    printf(" -W, --warning-memory=BYTES\n");
    printf("      Return warning if used memory exceeds value.\n");
    printf(" -C, --warning-memory=BYTES\n");
    printf("      Return critical if used memory exceeds value.\n");
}

/* vim: set ts=4 sw=4 et syn=c : */

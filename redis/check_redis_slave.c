/***
 * Monitoring Plugin - check_redis_slave.c
 **
 *
 * check_redis_slave - Check redis slave status.
 *
 * Copyright (C) 2013 Marius Rieder <marius.rieder@durchmesser.ch>
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

const char *progname  = "check_redis_slave";
const char *progdesc  = "Check redis slave status.";
const char *progvers  = "0.1";
const char *progcopy  = "2013";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[--host <HOSTNAME>] [--port <PORT>]";

/* MP Includes */
#include "mp_common.h"
#include "mp_utils.h"
#include "redis_utils.h"
/* Default Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
/* Library Includes */
#include <hiredis/hiredis.h>

/* Global Vars */
const char *hostname = "localhost";
int port = 6379;
const char *socket = NULL;
thresholds *time_thresholds = NULL;
thresholds *memory_thresholds = NULL;

int main (int argc, char **argv) {
    /* Local Vars */
    redisContext *c;
    redisReply *reply;
    char *redis_role = NULL;
    char *redis_link_status = NULL;
    int  redis_delay = -1;

    /* Set signal handling and alarm */
    if (signal (SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    // Connect to Server
    if (socket)
        c = redisConnectUnix(socket);
    else
        c = redisConnect(hostname, port);
    if (c != NULL && c->err) {
        critical("Error: %s", c->errstr);
    }

    /* PING server */
    reply = mp_redisCommand(c,"PING");
    if (reply == NULL)
        critical("Error: %s", c->errstr);
    if (reply->type == REDIS_REPLY_ERROR)
        critical("Error: %s", reply->str);
    freeReplyObject(reply);

    /* Read server info */
    reply = mp_redisCommand(c,"INFO");
    if (reply == NULL)
        critical("Error: %s", c->errstr);
    if (reply->type == REDIS_REPLY_ERROR)
        critical("Error: %s", reply->str);
    char *str = reply->str;
    char *line = NULL;
    while ((line = strsep(&str, "\r\n"))) {
        if (strncmp(line, "role:", 5) == 0) {
            redis_role = strdup(line+5);
        } else if (strncmp(line, "master_link_status:", 19) == 0) {
            redis_link_status = strdup(line+19);
        } else if (strncmp(line, "master_last_io_seconds_ago:", 27) == 0 ) {
            redis_delay = strtol(line+27, NULL, 10);
        } else if (strncmp(line, "master_link_down_since_seconds:", 31) == 0) {
            redis_delay = strtol(line+31, NULL, 10);
        }
    }
    freeReplyObject(reply);

    // Dissconnect
    redisFree(c);

    mp_perfdata_int2("delay", redis_delay, "s", time_thresholds,
            1, 0, 0, 0);

    // Check if slave
    if (strcmp(redis_role, "slave") != 0)
        critical("Redis is not a Slave (but %s)", redis_role);
    free(redis_role);

    // Check link
    if (strcmp(redis_link_status, "up") != 0)
        critical("Redis master link is %s", redis_link_status);
    free(redis_link_status);

    switch(get_status(redis_delay, time_thresholds)) {
        case STATE_OK:
            free_threshold(time_thresholds);
            warning("Redis Slave");
            break;
        case STATE_WARNING:
            free_threshold(time_thresholds);
            warning("Redis Slave is %ds behind.", redis_delay);
            break;
        case STATE_CRITICAL:
            free_threshold(time_thresholds);
            critical("Redis Slave is %ds behind.", redis_delay);
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
        {"socket", required_argument, NULL, (int)'s'},
        MP_LONGOPTS_WC,
        MP_LONGOPTS_END
    };

    /* Set default */
    mp_threshold_set_warning_time(&time_thresholds, "300s");
    mp_threshold_set_critical_time(&time_thresholds, "3600s");

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"H:P:s:w:c:",
                longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_wc_time(c, optarg, &time_thresholds);

        switch (c) {
            /* Plugin Opts */
            /* Hostname opt */
            case 'H':
                getopt_host(optarg, &hostname);
                break;
            /* Port opt */
            case 'P':
                getopt_port(optarg, &port);
                break;
            /* Socket opt */
            case 'S':
                socket = optarg;
                break;
        }
    }

    /* Check requirements */

    return(OK);
}

void print_help (void) {
    print_revision();
    print_revision_redis();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();

    print_help_default();
    print_help_host();
    print_help_port("6379");
    print_help_warn_time("300s");
    print_help_crit_time("3600s");
    printf(" -s, --socket=<SOCKET>\n");
    printf("      Unix socket to connect to.\n");
}

/* vim: set ts=4 sw=4 et syn=c : */

/***
 * Monitoring Plugin - check_rpc_ping.c
 **
 *
 * check_rpc_ping - Check if named RPC programm is responding.
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

const char *progname  = "check_rpc_ping";
const char *progdesc  = "Check if named RPC programm is responding.";
const char *progvers  = "0.1";
const char *progcopy  = "2011";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--host <HOST> --rpcprogramm <PROGRAMM>";

/* MP Includes */
#include "mp_common.h"
#include "rpc_utils.h"
/* Default Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
/* Library Includes */
#include <rpc/rpc.h>

/* Global Vars */
const char *hostname = NULL;
const char *export = NULL;
const char *program_name = NULL;
char *ping_ok = NULL;
char *ping_warn = NULL;
char *ping_failed = NULL;
struct timeval to;
char **rpcversion = NULL;
int rpcversions = 0;
char **rpctransport = NULL;
int rpctransports = 0;
thresholds *time_threshold = NULL;
CLIENT *client = NULL;

/* Function prototype */

int main (int argc, char **argv) {
    /* Local Vars */
    int i, j, ret;
    int tstate;
    char *buf;
    struct rpcent *program;
    struct timeval start_time;
    double time_delta;

    /* Set signal handling and alarm */
    if (signal (SIGALRM, rpc_timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);
    to.tv_sec = mp_timeout;
    to.tv_usec = 0;

    // PLUGIN CODE
    program = rpc_getrpcent(program_name);
    if (program == NULL)
        unknown("Program: %s not known.", program_name);

    for(i=0; i < rpcversions; i++) {
        for(j=0; j < rpctransports; j++) {
            buf = mp_malloc(128);
            mp_snprintf(buf, 128, "%s:%sv%s", rpctransport[j], program->r_name, rpcversion[i]);

            gettimeofday(&start_time, NULL);

            ret = rpc_ping((char *)hostname, program, atoi(rpcversion[i]), rpctransport[j], to);

            time_delta = mp_time_delta(start_time);

            mp_perfdata_float(buf, (float)time_delta, "s", time_threshold);

            tstate = get_status(time_delta, time_threshold);

            if (ret != RPC_SUCCESS || tstate == STATE_CRITICAL) {
                mp_strcat_comma(&ping_failed, buf);
            } else if(tstate == STATE_WARNING) {
                mp_strcat_comma(&ping_warn, buf);
            } else {
                mp_strcat_comma(&ping_ok, buf);
            }
            free(buf);
        }
    }

    buf = strdup(" ");
    if (ping_warn) {
        mp_strcat_space(&buf, "warn:");
        mp_strcat_space(&buf, ping_warn);
    }
    if (ping_ok) {
        mp_strcat_space(&buf, "ok:");
        mp_strcat_space(&buf, ping_ok);
    }

    if (ping_failed) {
        critical("RPC Ping failed: %s%s", ping_failed, buf);
    } else if (ping_warn) {
        warning("RPC Ping%s", buf);
    } else {
        free(buf);
        ok("RPC Ping: %s", ping_ok);
    }

    critical("You should never reach this point.");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        // PLUGIN OPTS
        {"rpcprogram", required_argument, 0, 'P'},
        {"rpcversion", required_argument, 0, 'r'},
        {"transport", required_argument, 0, 'T'},
        MP_LONGOPTS_END
    };

    /* Set default */
    mp_threshold_set_warning_time(&time_threshold, "0.5s");
    mp_threshold_set_critical_time(&time_threshold, "1s");

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"H:P:w:c:r:T:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_wc_time(c, optarg, &time_threshold);

        switch (c) {
            /* Host opt */
            case 'H':
                getopt_host(optarg, &hostname);
                break;
            /* Plugin opts */
            case 'r':
                mp_array_push(&rpcversion, optarg, &rpcversions);
                break;
            case 'T': {
                mp_array_push(&rpctransport, optarg, &rpctransports);
                break;
            case 'P':
                program_name = optarg;
                break;
            }
        }
    }

    /* Check requirements */
    if (!hostname || !program_name)
        usage("Hostname and Programm are mandatory.");

    /* Apply defaults */
    if (rpcversion == NULL)
        mp_array_push(&rpcversion, "3", &rpcversions);
    if (rpctransport == NULL) {
        mp_array_push(&rpctransport, "udp", &rpctransports);
        mp_array_push(&rpctransport, "tcp", &rpctransports);
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
    print_help_host();
    printf(" -P, --rpcprogramm=program\n");
    printf("      RPC program to ping.\n");
    printf(" -T, --transport=transport[,transport]\n");
    printf("      Transports to ping.\n");
    printf(" -r, --rpcversion=version[,version]\n");
    printf("      Versions to ping.\n");
    print_help_warn_time("0.5s");
    print_help_crit_time("1s");

}

/* vim: set ts=4 sw=4 et syn=c : */

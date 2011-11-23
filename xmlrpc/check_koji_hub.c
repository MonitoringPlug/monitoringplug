/***
 * Monitoring Plugin - check_koji_hub.c
 **
 *
 * check_koji_hub - Check if the koji respons
 * Copyright (C) 2011 Marius Rieder <marius.rieder@durchmesser.ch>
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

const char *progname  = "check_koji_hub";
const char *progvers  = "0.1";
const char *progcopy  = "2011";
const char *progauth = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--url URL";

/* MP Includes */
#include "mp_common.h"
#include "xmlrpc_utils.h"
/* Default Includes */
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* Library Includes */
#include <xmlrpc.h>
#include <xmlrpc_client.h>

/* Global Vars */
const char *url = NULL;
thresholds *time_threshold = NULL;

int main (int argc, char **argv) {
    /* Local Vars */
    xmlrpc_env env;
    xmlrpc_value *result;
    int api;
    int i, tasks[6];
    struct timeval start_time;
    double time_delta;
    xmlrpc_value *params;
    xmlrpc_value *calls;
    xmlrpc_value *call;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    /* Create XMLRPC env */
    env = mp_xmlrpc_init();

    /* Get Koji API Version */
    gettimeofday(&start_time, NULL); 
    result = xmlrpc_client_call(&env, url, "getAPIVersion", "()");
    time_delta = mp_time_delta(start_time);

    if (env.fault_occurred) {
       critical("Koji-Hub down: %s (%d)", env.fault_string, env.fault_code);
    }

    xmlrpc_parse_value(&env, result, "i", &api);

    if (mp_showperfdata) {
        mp_perfdata_float("time", (float)time_delta, "s", time_threshold);

        calls = xmlrpc_array_new(&env);

        for (i=0; i< 6; i++) {
           call = xmlrpc_build_value(&env, "{s:s,s:({s:(i)}{s:b})}",
                 "methodName", "listTasks",
                 "params", "state", i, "countOnly", 1);

           xmlrpc_array_append_item(&env, calls, call);
           xmlrpc_DECREF(call);
        }

        params = xmlrpc_array_new(&env);
        xmlrpc_array_append_item(&env, params, calls);
        xmlrpc_DECREF(calls);

        result = xmlrpc_client_call_params(&env, url, "multiCall", params);

	if (!env.fault_occurred) {
            xmlrpc_decompose_value(&env, result, "((i)(i)(i)(i)(i)(i)*)",
		  &tasks[0], &tasks[1], &tasks[2], &tasks[3], &tasks[4],
	          &tasks[5]);

            mp_perfdata_int("task_free", tasks[0], "", NULL);
            mp_perfdata_int("task_open", tasks[1], "", NULL);
            mp_perfdata_int("task_closed", tasks[2], "c", NULL);
            mp_perfdata_int("task_canceled", tasks[3], "c", NULL);
            mp_perfdata_int("task_assigned", tasks[4], "", NULL);
            mp_perfdata_int("task_faild", tasks[5], "c", NULL);
	}
    }

    switch (get_status(time_delta, time_threshold)) {
       case STATE_OK:
	  ok("Koji-Hub running. (API v%d)", api);
       case STATE_WARNING:
	  warning("Koji-Hub running slow. (API v%d)", api);
       default:
	  critical("Koji-Hub running to slow. (API v%d)", api);
    }
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_TIMEOUT,
        MP_LONGOPTS_EOPT,
        {"url", required_argument, NULL, (int)'U'},
        MP_LONGOPTS_END
    };

    /* Set default */
    setWarnTime(&time_threshold, "0.1s");
    setCritTime(&time_threshold, "0.2s");

    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"w:c::U:t:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

	getopt_wc_time(c, optarg, &time_threshold);

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            /* Local opts */
            case 'U':
                url = optarg;
                break;
            /* Timeout opt */
            case 't':
                getopt_timeout(optarg);
                break;
        }

    }

    /* Check requirements */
    if (!url)
        usage("URL is mandatory.");

    return(OK);
}

void print_help (void) {
    print_revision();
    print_copyright();

    printf("\n");

    printf("This plugin check a Koji-Hub.");

    printf("\n\n");

    print_usage();

    print_help_default();

    printf(" -U, --url=URL\n");
    printf("      URL of the Koji-Hub XML-RPC api.\n");
    print_help_warn_time("0.1s");
    print_help_crit_time("0.2s");
}

/* vim: set ts=4 sw=4 et syn=c : */

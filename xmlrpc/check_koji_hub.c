/***
 * Monitoring Plugin - check_koji_hub.c
 **
 *
 * check_koji_hub - Check if the koji-hub is working.
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

const char *progname  = "check_koji_hub";
const char *progdesc  = "Check if the koji-hub is working.";
const char *progvers  = "0.1";
const char *progcopy  = "2011";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--url URL";

/* MP Includes */
#include "mp_common.h"
#include "xmlrpc_utils.h"
/* Default Includes */
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
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

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
    unknown_if_xmlrpc_fault(&env);

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
        }
        if (!env.fault_occurred) {

            mp_perfdata_int("task_free", tasks[0], "", NULL);
            mp_perfdata_int("task_open", tasks[1], "", NULL);
            mp_perfdata_int("task_closed", tasks[2], "c", NULL);
            mp_perfdata_int("task_canceled", tasks[3], "c", NULL);
            mp_perfdata_int("task_assigned", tasks[4], "", NULL);
            mp_perfdata_int("task_failed", tasks[5], "c", NULL);
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
        MP_LONGOPTS_EOPT,
        {"url", required_argument, NULL, (int)'U'},
        MP_LONGOPTS_END
    };

    /* Set default */
    setWarnTime(&time_threshold, "0.1s");
    setCritTime(&time_threshold, "0.2s");

    while (1) {
        c = mp_getopt(argc, argv, MP_OPTSTR_DEFAULT"w:c:U:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_wc_time(c, optarg, &time_threshold);

        switch (c) {
            /* Local opts */
            case 'U':
                getopt_url(optarg, &url);
                break;
        }

    }

    /* Check requirements */
    if (!url)
        usage("URL is mandatory.");
    if (!is_url_scheme(url, "http") && !is_url_scheme(url,"https"))
        usage("Only http(s) urls are allowed.");

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

    printf(" -U, --url=URL\n");
    printf("      URL of the Koji-Hub XML-RPC api.\n");
    print_help_warn_time("0.1s");
    print_help_crit_time("0.2s");
}

/* vim: set ts=4 sw=4 et syn=c : */

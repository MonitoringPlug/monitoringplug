/***
 * Monitoring Plugin - check_rabbitmq.c
 **
 *
 * check_rabbitmq - Check a rabbitmq overview status.
 *
 * Copyright (C) 2014 Marius Rieder <marius.rieder@durchmesser.ch>
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

const char *progname  = "check_rabbitmq";
const char *progdesc  = "Check a rabbitmq overview status.";
const char *progvers  = "0.1";
const char *progcopy  = "2014";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--hostname <RABBITMQHOST>";

/* MP Includes */
#include "mp_common.h"
#include "curl_utils.h"
/* Default Includes */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
/* Library Includes */
#include <curl/curl.h>
#include <json/json.h>
#include "json_utils.h"

/* Global Vars */
const char *hostname = NULL;
int port = 15672;
thresholds *messages_thresholds = NULL;
thresholds *messages_ready_thresholds = NULL;
thresholds *messages_unacknowledged_thresholds = NULL;

/* Function prototype */

int main (int argc, char **argv) {
    /* Local Vars */
    char                *name = "RabbitMQ";
    CURL                *curl;
    char                *url;
    struct mp_curl_data answer;
    long int            code;
    struct json_object  *obj;
    struct json_object  *bufobj;
    struct json_object  *queue_totals;
    long queue_messages = -1;
    long queue_messages_ready = -1;
    long queue_messages_unacknowledged = -1;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    /* Build URL */
    url = mp_curl_url("http", hostname, port, "/api/overview");

    if (mp_verbose > 0) {
        printf("CURL Version: %s\n", curl_version());
        printf("Url: %s\n", url);
    }

    /* Init libcurl */
    curl = mp_curl_init();
    answer.data = NULL;
    answer.size = 0;

    /* Setup request */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, mp_curl_recv_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&answer);

    /* Get url */
    code = mp_curl_perform(curl);

    /* Cleanup libcurl */
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    free(url);

    if (code != 200) {
        critical("RabbitMQ - HTTP Status %ld.", code);
    }

    if (mp_verbose > 2) {
        printf("Answer: '%s'\n", answer.data);
    }

    /* Parse Answer */
    obj = mp_json_tokener_parse(answer.data);

    if (mp_verbose > 1) {
        printf("JSON:\n%s\n", mp_json_object_to_json_string(obj));
    }

    /* Read Server Version */
    if (mp_json_object_object_get(obj,"rabbitmq_version", &bufobj)) {
        mp_asprintf(&name, "RabbitMQ %s:", json_object_get_string(bufobj));
    }

    /* Get Message Counts */
    mp_json_object_object_get(obj, "queue_totals", &queue_totals);
    if (queue_totals != NULL ) {
        if(mp_json_object_object_get(queue_totals,"messages", &bufobj))
            queue_messages = json_object_get_int(bufobj);

        if(mp_json_object_object_get(queue_totals,"messages_ready", &bufobj))
            queue_messages_ready = json_object_get_int(bufobj);

        if(mp_json_object_object_get(queue_totals,"messages_unacknowledged", &bufobj))
            queue_messages_unacknowledged = json_object_get_int(bufobj);

        mp_perfdata_int("messages", queue_messages, "", messages_thresholds);
        mp_perfdata_int("messages_ready", queue_messages_ready,
                "", messages_ready_thresholds);
        mp_perfdata_int("messages_unacknowledged", queue_messages_unacknowledged,
                "", messages_unacknowledged_thresholds);
    } else {
        critical("Could not find queue_totals JSON object");
    }

    /* Read message counters */
    if (mp_showperfdata) {
        mp_json_object_object_get(obj, "message_stats", &queue_totals);
        mp_json_object_object_get(queue_totals, "publish", &bufobj);
        mp_perfdata_int("publish", (long int)json_object_get_int(bufobj), "c", NULL);
    }

    /* free */
    json_object_put(obj);

    /* Check queue size */
    switch(get_status(queue_messages, messages_thresholds)) {
        case STATE_WARNING:
            set_warning("%d messages", queue_messages);
            break;
        case STATE_CRITICAL:
            set_critical("%d messages", queue_messages);
            break;
    }
    switch(get_status(queue_messages_ready, messages_ready_thresholds)) {
        case STATE_WARNING:
            set_warning("%d messages ready", queue_messages_ready);
            break;
        case STATE_CRITICAL:
            set_critical("%d messages ready", queue_messages_ready);
            break;
    }
    switch(get_status(queue_messages_unacknowledged, messages_unacknowledged_thresholds)) {
        case STATE_WARNING:
            set_warning("%d messages unacknowledged", queue_messages_unacknowledged);
            break;
        case STATE_CRITICAL:
            set_critical("%d messages unacknowledged", queue_messages_unacknowledged);
            break;
    }
    
    mp_exit(name);
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        MP_LONGOPTS_PORT,
        CURL_LONGOPTS,
        MP_LONGOPTS_WC,
        {"warning-ready", required_argument, 0, MP_LONGOPT_PRIV1},
        {"critical-ready", required_argument, 0, MP_LONGOPT_PRIV2},
        {"warning-unacknowledged", required_argument, 0, MP_LONGOPT_PRIV3},
        {"critical-unacknowledged", required_argument, 0, MP_LONGOPT_PRIV4},
        MP_LONGOPTS_END
    };

    if (argc < 2) {
        print_help();
        exit(STATE_OK);
    }

    /* Set default */

    while (1) {
        c = mp_getopt(&argc, &argv,
                MP_OPTSTR_DEFAULT"H:P:u:p:"MP_OPTSTR_WC,
                longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_curl(c);
        getopt_wc(c, optarg, &messages_thresholds);

        switch (c) {
            /* Hostname opt */
            case 'H':
                getopt_host(optarg, &hostname);
                break;
            case 'P':
                getopt_port(optarg, &port);
                break;
            /* Warn/Crit opt */
            case MP_LONGOPT_PRIV1:
                if (mp_threshold_set_warning(&messages_ready_thresholds, optarg, BISI) == ERROR)
                    usage("Illegal --warning-ready threshold '%s'.", optarg);
                break;
            case MP_LONGOPT_PRIV2:
                if (mp_threshold_set_critical(&messages_ready_thresholds, optarg, BISI) == ERROR)
                    usage("Illegal --critical-ready threshold '%s'.", optarg);
                break;
            case MP_LONGOPT_PRIV3:
                if (mp_threshold_set_warning(&messages_unacknowledged_thresholds, optarg, BISI) == ERROR)
                    usage("Illegal --warning-unacknowledged threshold '%s'.", optarg);
                break;
            case MP_LONGOPT_PRIV4:
                if (mp_threshold_set_critical(&messages_unacknowledged_thresholds, optarg, BISI) == ERROR)
                    usage("Illegal --critical-unacknowledged threshold '%s'.", optarg);
                break;
        }
    }

    /* Check requirements */
    if (!hostname)
        usage("Hostname is mandatory.");

    return(OK);
}

void print_help (void) {
    print_revision();
    print_revision_curl();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();

    print_help_default();
    print_help_host();
    print_help_port("15672");
    print_help_curl_subpath();
    print_help_curl_basic_auth();
    print_help_curl_https();
    print_help_warn("message count","INF");
    print_help_crit("message count","INF");
    printf("     --warning-ready=LIMIT\n");
    printf("      Return warning if ready message count exceeds limit. Defaults to INF\n");
    printf("     --critical-ready=LIMIT\n");
    printf("      Return critical if ready message count exceeds limit. Defaults to INF\n");
    printf("     --warning-unacknowledged=LIMIT\n");
    printf("      Return warning if unacknowledged message count exceeds limit. Defaults to INF\n");
    printf("     --critical-unacknowledged=LIMIT\n");
    printf("      Return critical if unacknowledged message count exceeds limit. Defaults to INF\n");

}

/* vim: set ts=4 sw=4 et syn=c : */

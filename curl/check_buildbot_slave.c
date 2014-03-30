/***
 * Monitoring Plugin - check_buildbot_slave.c
 **
 *
 * check_buildbot_slave - Check BuildBot slave state by json.
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

const char *progname  = "check_buildbot_slave";
const char *progdesc  = "Check BuildBot slave state by json.";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--hostname <BUILDBOTHOST> [--slave <SLAVENAME>]";

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

/* Global Vars */
const char *hostname = NULL;
int port = 8010;
char **slave = NULL;
int slaves = 0;

/* Function prototype */

int main (int argc, char **argv) {
    /* Local Vars */
    CURL                *curl;
    char                *url;
    int                 i, j;
    char                *buf;
    struct mp_curl_data answer;
    long int            code;
    struct json_object  *obj;
    struct json_object  *slaveobj;
    unsigned int        slave_connected;
    char                *slave_host;
    char                *slave_version;
    char                *connected = NULL;
    char                *failed = NULL;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    /* Build query */
    url = mp_malloc(128);
    if (slaves) {
        mp_snprintf(url, 127, "http://%s:%d/json/slaves?select=%s", hostname, port, slave[0]);
        for(i=1; i<slaves; i++) {
            url = mp_realloc(url, strlen(url) + strlen(slave[i]) + 8 );
            strcat(url, "&select=");
            strcat(url, slave[i]);
        }
    } else {
        mp_snprintf(url, 127, "http://%s:%d/json/slaves", hostname, port);
    }

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
        critical("Buildbot - HTTP Status %ld.", code);
    }

    if (mp_verbose > 1) {
        printf("Answer: '%s'\n", answer.data);
    }

    /* Parse Answer */
    obj = json_tokener_parse(answer.data);

    if (slaves) {
        for(i=0; i<slaves; i++) {
            slaveobj = json_object_object_get(obj, slave[i]);
            if(json_object_object_get(slaveobj,"error")) {
                mp_asprintf(&buf, "%s - %s", slave[i], json_object_get_string(json_object_object_get(slaveobj,"error")));
                mp_strcat_comma(&failed, buf);
                free(buf);
                continue;
            }
            slave_connected = json_object_get_boolean(json_object_object_get(slaveobj, (const char *)"connected"));
            slave_host = (char *)json_object_get_string(json_object_object_get(slaveobj, "host"));
            slave_version = (char *)json_object_get_string(json_object_object_get(slaveobj, "version"));

            for (j = strlen(slave_host) -1; isspace(slave_host[j]); j--) {
                slave_host[j] = '\0';
            }

            mp_asprintf(&buf, "%s - %s (v%s)", slave[i], slave_host, slave_version);

            if (slave_connected) {
                mp_strcat_comma(&connected, buf);
            } else {
                mp_strcat_comma(&failed, buf);
            }
            free(buf);
        }
    } else {
        json_object_object_foreach(obj, key, val) {
            slaveobj = val;
            slave_connected = json_object_get_boolean(json_object_object_get(slaveobj, "connected"));
            slave_host = (char *)json_object_get_string(json_object_object_get(slaveobj, "host"));
            slave_version = (char *)json_object_get_string(json_object_object_get(slaveobj, "version"));

            if (slave_host) {
                for (j = strlen(slave_host) -1; isspace(slave_host[j]); j--) {
                    slave_host[j] = '\0';
                }
            }

            if (slave_connected) {
                mp_asprintf(&buf, "%s - %s (v%s)", key, slave_host, slave_version);
                mp_strcat_comma(&connected, buf);
                free(buf);
            } else {
                mp_strcat_comma(&failed, key);
            }

        }
    }

    /* free */
    free(buf);
    json_object_put(obj);

    if (failed && connected) {
        critical("%s, (OK: %s)", failed, connected);
    } else if (failed) {
        critical(failed);
    } else if (connected) {
        ok(connected);
    }
    warning("No Slaves found");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        MP_LONGOPTS_PORT,
        {"slave", required_argument, 0, 'S'},
        MP_LONGOPTS_END
    };

    if (argc < 2) {
        print_help();
        exit(STATE_OK);
    }

    /* Set default */

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"H:P:S:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        switch (c) {
            case 'S':
                mp_array_push(&slave, optarg, &slaves);
                break;
            /* Hostname opt */
            case 'H':
                getopt_host(optarg, &hostname);
                break;
            case 'P':
                getopt_port(optarg, &port);
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
    print_help_port("8010");
    printf(" -S, --slave=SLAVE\n");
    printf("      Check state of defines SLAVE(s).\n");
}

/* vim: set ts=4 sw=4 et syn=c : */

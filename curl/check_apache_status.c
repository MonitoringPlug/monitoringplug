/***
 * Monitoring Plugin - check_apache_status.c
 **
 *
 * check_apache_status - Check Apache HTTPD mod_status output.
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

const char *progname  = "check_apache_status";
const char *progdesc  = "Check Apache HTTPD mod_status output.";
const char *progvers  = "0.1";
const char *progcopy  = "2012";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--url <URL>";

/* MP Includes */
#include "mp_common.h"
#include "curl_utils.h"
/* Default Includes */
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
/* Library Includes */
#include <curl/curl.h>

/* Global Vars */
const char *url = NULL;
const char *hostname = NULL;
int port = 80;
thresholds *open_thresholds = NULL;

/* Function prototype */

int main (int argc, char **argv) {
    /* Local Vars */
    CURL                *curl;
    struct mp_curl_data answer;
    long int            code;
    int                 sb_wait = 0;
    int                 sb_start = 0;
    int                 sb_recv = 0;
    int                 sb_send = 0;
    int                 sb_keep = 0;
    int                 sb_dns = 0;
    int                 sb_close = 0;
    int                 sb_log = 0;
    int                 sb_grace = 0;
    int                 sb_idle = 0;
    int                 sb_open = 0;
    char                *buf = NULL, *key, *val;
    char                *server = NULL;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    /* Build query */
    if (mp_verbose > 0) {
        printf("CURL Version: %s\n", curl_version());
        printf("Url: %s\n", url);
        print_thresholds("open_thresholds", open_thresholds);
    }

    /* Headers */
    struct mp_curl_header headers[] = {
        {"Server", &server}, {NULL, NULL}
    };

    /* Init libcurl */
    curl = mp_curl_init();
    answer.data = NULL;
    answer.size = 0;

    /* Setup request */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, mp_curl_recv_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&answer);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, mp_curl_recv_header);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *)headers);

    /* Get url */
    code = mp_curl_perform(curl);

    /* Cleanup libcurl */
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    if (mp_verbose > 1) {
        printf("Code: %ld\n", code);
        printf("Server: %s\n", server);
        if (mp_verbose > 2) {
            printf("Answer: '%s'\n", answer.data);
        }
    }

    /* Parse Answer */
    buf = answer.data;

    while(*buf) {
        val = strsep(&buf, "\n");
        key = strsep(&val, ": ");

        if (mp_verbose > 1) {
            printf("%s => %s\n", key, val);
        }

        if(strcmp(key, "Scoreboard") == 0) {
            for(; *val; val++) {
                switch (*val) {
                    case '.':
                        sb_open++;
                        break;
                    case '_':
                        sb_wait++;
                        break;
                    case 'S':
                        sb_start++;
                        break;
                    case 'R':
                        sb_recv++;
                        break;
                    case 'W':
                        sb_send++;
                        break;
                    case 'K':
                        sb_keep++;
                        break;
                    case 'D':
                        sb_dns++;
                        break;
                    case 'C':
                        sb_close++;
                        break;
                    case 'L':
                        sb_log++;
                        break;
                    case 'G':
                        sb_grace++;
                        break;
                    case 'I':
                        sb_idle++;
                        break;
                }
            }
        }
    }

    mp_perfdata_int("open", (long int)sb_open, "c", open_thresholds);
    mp_perfdata_int("start", (long int)sb_start, "c", NULL);
    mp_perfdata_int("read", (long int)sb_recv, "c", NULL);
    mp_perfdata_int("write", (long int)sb_send, "c", NULL);
    mp_perfdata_int("keep", (long int)sb_keep, "c", NULL);
    mp_perfdata_int("dns", (long int)sb_dns, "c", NULL);
    mp_perfdata_int("close", (long int)sb_close, "c", NULL);
    mp_perfdata_int("log", (long int)sb_log, "c", NULL);
    mp_perfdata_int("grace", (long int)sb_grace, "c", NULL);
    mp_perfdata_int("idle", (long int)sb_idle, "c", NULL);
    mp_perfdata_int("open", (long int)sb_open, "c", NULL);

    /* free */
    free(answer.data);

    if (open_thresholds) {
        switch(get_status(sb_open, open_thresholds)) {
        case STATE_OK:
            free_threshold(open_thresholds);
            ok("Apache HTTPD status - %s", server);
        case STATE_WARNING:
            free_threshold(open_thresholds);
            warning("Apache HTTPD low on open slots - %s", server);
        case STATE_CRITICAL:
            free_threshold(open_thresholds);
            critical("Apache HTTPD not enough open slots - %s", server);
        }
    }
    ok("Apache HTTPD status - %s", server);
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        MP_LONGOPTS_PORT,
        {"url", required_argument, 0, 'u'},
        MP_LONGOPTS_WC,
        MP_LONGOPTS_TIMEOUT,
        MP_LONGOPTS_END
    };

    if (argc < 2) {
        print_help();
        exit(STATE_OK);
    }

    /* Set default */

    while (1) {
        c = getopt_long(argc, argv, MP_OPTSTR_DEFAULT"H:P:u:w:c:t:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_wc(c, optarg, &open_thresholds);

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            case 'u':
                getopt_url(optarg, &url);
                break;
            /* Hostname opt */
            case 'H':
                getopt_host(optarg, &hostname);
                break;
            case 'P':
                getopt_port(optarg, &port);
                break;
            /* Timeout opt */
            case 't':
                getopt_timeout(optarg);
                break;
        }
    }

    /* Check requirements */
    if (!is_url_scheme(url, "http") && !is_url_scheme(url, "https"))
        usage("Only http and https url allowed.");
    if (!url && !hostname)
        usage("Url or Hostname is mandatory.");
    if (url && hostname)
        usage("Only Url or Hostname allowed.");

    if (!url) {
        size_t len = strlen(hostname) + 40;
        char *u;
        u = mp_malloc(len);
        mp_snprintf(u, len , "http://%s:%d/server-status?auto", hostname, port);
        url = u;
    } else {
        char *u;
        u = (char *)(url + strlen(url) - 5);
        if (strcmp(u, "?auto") != 0) {
            u = mp_malloc(strlen(url) + 6);
            strcpy(u, url);
            strcat(u, "?auto");
            url = u;
        }
    }

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
    print_help_port("80");
    printf(" -u, --url=URL\n");
    printf("      URL of mod_status.\n");
    print_help_warn("open slots", "none");
    print_help_crit("open slots", "none");
}

/* vim: set ts=4 sw=4 et syn=c : */

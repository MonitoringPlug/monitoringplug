/***
 * Monitoring Plugin - check_tftp.c
 **
 *
 * check_tftp - Check if a file can be downloaded from tftp.
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

const char *progname  = "check_tftp";
const char *progdesc  = "Check if a file can be downloaded from tftp.";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "-H host -F file [-t timeout] [-w warn] [-c crit]";

/* MP Includes */
#include "mp_common.h"
#include "curl_utils.h"
/* Default Includes */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* Library Includes */
#include <curl/curl.h>

/* Global Vars */
const char *hostname = NULL;
char *filename = NULL;
thresholds *fetch_thresholds = NULL;
int port = 0;

/* Function prototype */

int main (int argc, char **argv) {
    /* Local Vars */
    CURL        *curl;
    char        *url;
    double      size;
    double      time;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    /* Magik */

    size_t urllen = strlen(hostname)+strlen(filename) + 9;

    url = mp_malloc(urllen);

    snprintf(url, urllen, "tftp://%s/%s", hostname, filename);

    if (mp_verbose > 0) {
        printf("CURL Version: %s\n", curl_version());
        printf("Try fetch %s\n", url);
        print_thresholds("fetch_thresholds", fetch_thresholds);
    }

    /* Init libcurl */
    curl = mp_curl_init();

    /* Setup request */
    if (curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_TFTP) == CURLE_UNSUPPORTED_PROTOCOL)
        unknown("libcurl don't support tftp.");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, mp_curl_recv_blackhole);
    if (port != 0)
        curl_easy_setopt(curl, CURLOPT_LOCALPORT, port);

    /* Perform Request */
    mp_curl_perform(curl);

    /* Get metric */
    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &time);
    curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD , &size);

    /* Clenup libcurl */
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    mp_perfdata_float("time", (float)time, "s", fetch_thresholds);
    free(url);

    switch(get_status(time, fetch_thresholds)) {
        case STATE_OK:
            free_threshold(fetch_thresholds);
            ok("Received %'.0fbyte in %fs.", size, time);
        case STATE_WARNING:
            free_threshold(fetch_thresholds);
            warning("Received %'.0fbyte in %fs.", size, time);
        case STATE_CRITICAL:
            free_threshold(fetch_thresholds);
            critical("Received %'.0fbyte in %fs.", size, time);
    }
    free_threshold(fetch_thresholds);

    critical("You should never reach this point.");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        MP_LONGOPTS_PORT,
        {"file", required_argument, 0, 'F'},
        MP_LONGOPTS_WC,
        MP_LONGOPTS_END
    };

    if (argc < 4) {
        print_help();
        exit(STATE_OK);
    }

    /* Set default */
    setWarnTime(&fetch_thresholds, "5s");
    setCritTime(&fetch_thresholds, "9s");

    while (1) {
        c = getopt_long(argc, argv, MP_OPTSTR_DEFAULT"H:P:F:w:c:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_wc_time(c, optarg, &fetch_thresholds);

        switch (c) {
            /* Hostname opt */
            case 'H':
                getopt_host(optarg, &hostname);
                break;
            /* Port opt */
            case 'P':
                getopt_port(optarg, &port);
                break;
            case 'F':
                filename = optarg;
                break;
        }
    }

    /* Check requirements */
    if (!filename || !hostname)
        usage("Filename and hostname are mandatory.");

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
    print_help_port("UNDEF");
    printf(" -F, --file=FILENAME\n");
    printf("      The name of the file to download.\n");
    print_help_warn_time("5 sec");
    print_help_crit_time("9 sec");
}

/* vim: set ts=4 sw=4 et syn=c : */

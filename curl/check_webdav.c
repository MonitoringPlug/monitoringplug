/***
 * Monitoring Plugin - check_webdav.c
 **
 *
 * check_webdav - Check BuildBot slave state by json.
 *
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

const char *progname  = "check_webdav";
const char *progdesc  = "Check WebDAV share.";
const char *progvers  = "0.1";
const char *progcopy  = "2011";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--url <URL>";

/* MP Includes */
#include "mp_common.h"
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
char *answer = NULL;
char *dav = NULL;
char *allow = NULL;
char **allowShould = NULL;
int allowShoulds = 0;
char *contentType = NULL;
char *contentTypeShould = NULL;
thresholds *fetch_thresholds = NULL;

/* Function prototype */
static size_t my_hwrite( void *ptr, size_t size, size_t nmemb, void *userdata);

int main (int argc, char **argv) {
    /* Local Vars */
    CURL        *curl;
    CURLcode    res;
    double      time;
    int         i;
    char        *output = NULL;
    int         status = STATE_OK;

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
    }
    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "OPTIONS");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, my_hwrite);

        if (mp_verbose > 2)
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        if(CURLE_OK != res) {
            critical(curl_easy_strerror(res));
        }
    }

    curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &time);
    mp_perfdata_float("time", (float)time, "s", fetch_thresholds);

    curl_global_cleanup();

    for (i=0; i < allowShoulds; i++) {
        char *ptr;
        if ((ptr = strstr(allow, allowShould[i])) != NULL) {
            if ((ptr == allow || *(ptr-1) == ',') &&
                    (*(ptr+strlen(allowShould[i])) == ',' || *(ptr+strlen(allowShould[i])) == '\0')) {
                continue;
            }
        }

        status = STATE_CRITICAL;

        if (output == NULL) {
            mp_strcat_space(&output, "Missing Allow:");
            mp_strcat_space(&output, allowShould[i]);
        } else {
            mp_strcat_comma(&output, allowShould[i]);
        }
    }

    if (contentTypeShould != NULL && strcmp(contentType, contentTypeShould) != 0) {
        status = STATE_CRITICAL;
        mp_strcat_space(&output, "Wrong Content-Type:");
        mp_strcat_space(&output, contentType);
        free(contentType);
    }

    switch(get_status(time, fetch_thresholds)) {
        case STATE_WARNING:
            status = status == STATE_OK ? STATE_WARNING : status;
            mp_strcat_space(&output, "Slow answer");
            break;
        case STATE_CRITICAL:
            status = STATE_CRITICAL;
            mp_strcat_space(&output, "Answer too slow");
    }

    switch(status) {
        case STATE_OK:
            ok("WebDAV %s", dav);
        case STATE_WARNING:
            warning("WebDAV %s - %s", dav, output);
        case STATE_CRITICAL:
            critical("WebDAV %s - %s", dav, output);
    }

    unknown("WebDAV %s - %s", dav, output);
}

static size_t my_hwrite( void *ptr, size_t size, size_t nmemb, void *userdata) {
    if (dav == NULL && strncmp("DAV:", ptr, 4) == 0) {
        dav = mp_malloc(size*nmemb-4);
        strncpy(dav, ptr+5, size*nmemb-5);
        dav[size*nmemb-7] = '\0';
    } else if (allow == NULL && strncmp("Allow:", ptr, 6) == 0) {
        allow = mp_malloc(size*nmemb-6);
        strncpy(allow, ptr+7, size*nmemb-7);
        allow[size*nmemb-9] = '\0';
    } else if (contentType == NULL && strncmp("Content-Type:", ptr, 13) == 0) {
        contentType = mp_malloc(size*nmemb-13);
        strncpy(contentType, ptr+14, size*nmemb-14);
        contentType[size*nmemb-16] = '\0';
    }

    return size*nmemb;
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_PORT,
        {"url", required_argument, 0, (int)'u'},
        {"content-type", required_argument, 0, (int)'C'},
        {"allow", required_argument, 0, (int)'a'},
        MP_LONGOPTS_WC,
        MP_LONGOPTS_TIMEOUT,
        MP_LONGOPTS_END
    };

    if (argc < 2) {
        print_help();
        exit(STATE_OK);
    }

    /* Set default */
    setWarnTime(&fetch_thresholds, "5s");
    setCritTime(&fetch_thresholds, "9s");

    while (1) {
        c = getopt_long(argc, argv, MP_OPTSTR_DEFAULT"u:C:a:w:c:t:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_wc_time(c, optarg, &fetch_thresholds);

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            case 'u':
                url = optarg;
                break;
            case 'C':
                contentTypeShould = optarg;
                break;
            case 'a':
                mp_array_push(&allowShould, optarg, &allowShoulds);
                break;
            /* Timeout opt */
            case 't':
                getopt_timeout(optarg);
                break;
        }
    }

    /* Check requirements */
    if (!url)
        usage("url is mandatory.");

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
    printf(" -u, --url=URL\n");
    printf("      Url to test.\n");
    printf(" -C, --content-type=TYPE\n");
    printf("      Content-Type URL should report.\n");
    printf(" -a, --allow=METHOD[,METHOD]\n");
    printf("      Method or methods which should be allowed.\n");
    print_help_warn_time("5 sec");
    print_help_crit_time("9 sec");
}

/* vim: set ts=4 sw=4 et syn=c : */

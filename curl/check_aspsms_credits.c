/***
 * Monitoring Plugin - check_aspsms_credits.c
 **
 *
 * check_aspsms_credits - Check ASPSMS credits by XML.
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

const char *progname  = "check_aspsms_credits";
const char *progdesc  = "Check ASPSMS credits by XML.";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--userkey <userkey> --password <password>";

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

/* Global Constants */
const char *xml_url = "http://xml1.aspsms.com:5061/xmlsvr.asp";

/* Global Vars */
const char *userkey = NULL;
const char *password = NULL;
thresholds *credit_thresholds = NULL;

/* Function prototype */

int main (int argc, char **argv) {
    /* Local Vars */
    CURL        *curl;
    long int    code;
    double      time;
    struct mp_curl_data query;
    struct mp_curl_data answer;
    struct curl_slist *headers = NULL;
    char        *xmlp;
    char        *c;
    int         errorCode = 0;
    float       credits = 0;
    char        *errorDescription = NULL;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    /* Build query */
    query.data = mp_malloc(strlen(userkey) + strlen(password) + 134);
    mp_sprintf(query.data, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n<aspsms>"
        "\n<Userkey>%s</Userkey>\n<Password>%s</Password>\n"
        "<Action>ShowCredits</Action>\n</aspsms>", userkey, password);
    query.size = strlen(query.data);
    query.start = 0;
    answer.data = NULL;
    answer.size = 0;

    if (mp_verbose > 0) {
        printf("CURL Version: %s\n", curl_version());
        printf("smsXML Url %s\n", xml_url);
        print_thresholds("credit_thresholds", credit_thresholds);
        printf("XML: '%s'\n", query.data);
    }

    /* Init libcurl */
    curl = mp_curl_init();

    /* Setup request */
    curl_easy_setopt(curl, CURLOPT_URL, xml_url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, mp_curl_send_data);
    curl_easy_setopt(curl, CURLOPT_READDATA, (void *)&query);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, mp_curl_recv_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&answer);

    /* Set header */
    c = mp_malloc(24);
    mp_snprintf(c, 24, "Content-Length: %d", (int)query.size);
    headers = curl_slist_append(headers, "Content-Type: text/html");
    headers = curl_slist_append(headers, c);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    free(c);

    /* Perform request */
    code = mp_curl_perform(curl);

    /* Get metric */
    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &time);
    mp_perfdata_float("time", (float)time, "s", NULL);

    /* Cleanup libcurl */
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    free(query.data);

    if (mp_verbose > 1) {
        printf("Answer: '%s'\n", answer.data);
    }

    if (code != 200)
        critical("API HTTP-Response %ld.", code);

    /* Parse Answer */
    xmlp = answer.data;

    while((c = strsep(&xmlp, "<>"))) {
        if (strcmp(c, "ErrorCode") == 0) {
            errorCode = strtol(strsep(&xmlp, "<>"), NULL, 10);
        } else if (strcmp(c, "ErrorDescription") == 0) {
            errorDescription = strdup(strsep(&xmlp, "<>"));
        } else if (strcmp(c, "Credits") == 0) {
            credits = (float) strtod(strsep(&xmlp, "<>"), NULL);
        }
    }

    free(answer.data);

    if (mp_verbose > 0) {
        printf("errorCode %d\n", errorCode);
        if (errorDescription)
            printf("errorDescription %s\n", errorDescription);
        printf("credits %f\n", credits);
    }

    /* XML Error Code */
    if (errorCode != 1)
        unknown(errorDescription);
    if (errorDescription)
        free(errorDescription);

    switch(get_status((int)credits, credit_thresholds)) {
        case STATE_OK:
            free_threshold(credit_thresholds);
            ok("ASP SMS %.2f credits left for %s.", credits, userkey);
        case STATE_WARNING:
            free_threshold(credit_thresholds);
            warning("ASP SMS %.2f credits left for %s.", credits, userkey);
        case STATE_CRITICAL:
            free_threshold(credit_thresholds);
            critical("ASP SMS %.2f credits left for %s.", credits, userkey);
    }

    critical("You should never reach this point.");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        {"userkey", required_argument, 0, 'U'},
        {"password", required_argument, 0, 'P'},
        MP_LONGOPTS_WC,
        MP_LONGOPTS_END
    };

    if (argc < 4) {
        print_help();
        exit(STATE_OK);
    }

    /* Set default */
    setWarnTime(&credit_thresholds, "100:");
    setCritTime(&credit_thresholds, "50:");

    while (1) {
        c = getopt_long(argc, argv, MP_OPTSTR_DEFAULT"U:P:w:c:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_wc_time(c, optarg, &credit_thresholds);

        switch (c) {
            case 'U':
                userkey = optarg;
                break;
            case 'P':
                password = optarg;
                break;
        }
    }

    /* Check requirements */
    if (!userkey || !password)
        usage("Userkey and password are mandatory.");

    return(OK);
}

void print_help (void) {
    print_revision();
    print_revision_curl();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);
    printf ("\n\n\tWARNING: Password is sent unencryptet.");

    printf("\n\n");

    print_usage();

    print_help_default();
    printf(" -U, --userkey=USERKEY\n");
    printf("      The userkey of the ASPSMS account.\n");
    printf(" -P, --password=PASSWORD\n");
    printf("      The password of the ASPSMS account.\n");
    print_help_warn("credits", "100:");
    print_help_crit("credits", "50:");
}

/* vim: set ts=4 sw=4 et syn=c : */

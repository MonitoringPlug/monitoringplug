/***
 * Monitoring Plugin - check_aspsms_credits
 **
 *
 * check_aspsms_credits - Check ASPSMS credits by XML.
 * Copyright (C) 2010 Marius Rieder <marius.rieder@durchmesser.ch>
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

const char *progname  = "check_aspsms_credits";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--userkey <userkey> --password <password>";

/* MP Includes */
#include "mp_common.h"
/* Default Includes */
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* Library Includes */
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

/* Global Constants */
const char *xml_url = "http://xml1.aspsms.com:5061/xmlsvr.asp";

/* Global Vars */
const char *userkey = NULL;
const char *password = NULL;
thresholds *credit_thresholds = NULL;
char *xmlp;
char *answer = NULL;

/* Function prototype */
static size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream);
static size_t my_fread(void *buffer, size_t size, size_t nmemb, void *stream);

int main (int argc, char **argv) {
    /* Local Vars */
    CURL        *curl;
    CURLcode    res;
    struct curl_slist *headers = NULL;
    char        *xml;
    char        *c;
    int         errorCode = 0;
    float       credits = 0;
    char        *errorDescription = "Illegal response from server.";

    /* Set signal handling and alarm */
    if (signal (SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) == OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);
    
    /* Build query */
    xml = mp_malloc(strlen(userkey) + strlen(password) + 134);
    mp_sprintf(xml, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n<aspsms>"
        "\n<Userkey>%s</Userkey>\n<Password>%s</Password>\n"
        "<Action>ShowCredits</Action>\n</aspsms>", userkey, password);
    xmlp = xml;
    
    if (mp_verbose > 0) {
        printf("CURL Version: %s\n", curl_version());
        printf("smsXML Url %s\n", xml_url);
        print_thresholds("credit_thresholds", credit_thresholds);
        printf("XML: '%s'\n", xml);
    }
    
    curl_global_init(CURL_GLOBAL_ALL);
    
    /* Set Header */
    c = mp_malloc(24);
    mp_snprintf(c, 24, "Content-Length: %d", (int)strlen(xml));
    headers = curl_slist_append (headers, "Content-Type: text/html");
    headers = curl_slist_append (headers, c);
    free( c );
    
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, xml_url);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, my_fread);
        
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        if (mp_verbose > 1)
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            
        res = curl_easy_perform(curl);
        
        curl_easy_cleanup(curl);
        if(CURLE_OK != res) {
            critical(curl_easy_strerror(res));
        }
    }
    
    curl_global_cleanup();
    
    if (mp_verbose > 1) {
        printf("Answer: '%s'\n", answer);
    }
    
    /* Parse Answer */
    xmlp = answer;
    
    while((c = strsep(&xmlp, "<>"))) {
        if (strcmp(c, "ErrorCode") == 0) {
            errorCode = strtol(strsep(&xmlp, "<>"), NULL, 10);
        } else if (strcmp(c, "ErrorDescription") == 0) {
            errorDescription = strdup(strsep(&xmlp, "<>"));
        } else if (strcmp(c, "Credits") == 0) {
            credits = strtof(strsep(&xmlp, "<>"), NULL);
        }
    }
    
    if (mp_verbose > 0) {
        printf("errorCode %d\n", errorCode);
        printf("errorDescription %s\n", errorDescription);
        printf("credits %f\n", credits);
    }
    
    /* XML Error Code */
    if (errorCode != 1)
        unknown(errorDescription);

    switch(get_status((int)credits, credit_thresholds)) {
        case STATE_OK:
            ok("ASP SMS %.2f credits left for %s.", credits, userkey);
        case STATE_WARNING:
            warning("ASP SMS %.2f credits left for %s.", credits, userkey);
        case STATE_CRITICAL:
            critical("ASP SMS %.2f credits left for %s.", credits, userkey);
    }
       
    critical("You should never reach this point.");
}

static size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream) {
    if (answer == NULL) {
        answer = mp_malloc(size*nmemb + 1);
        xmlp = answer;
    } else {
        answer = mp_realloc(answer, strlen(answer) + size*nmemb + 1);
    }
    memcpy(xmlp, buffer, size*nmemb);
    xmlp[size*nmemb] = '\0';
    xmlp += size*nmemb;

    return size*nmemb;
}

static size_t my_fread(void *buffer, size_t size, size_t nmemb, void *stream) {
    size_t s = strlen(xmlp);
    if (s > size*nmemb)
        s = size*nmemb;

    strncpy(buffer, xmlp, s);
    xmlp += s;
    return s;
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        {"userkey", required_argument, 0, 'U'},
        {"password", required_argument, 0, 'P'},
        MP_LONGOPTS_WC,
        MP_LONGOPTS_TIMEOUT,
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
        c = getopt_long(argc, argv, MP_OPTSTR_DEFAULT"U:P:w:c:t:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_wc_time(c, optarg, &credit_thresholds);
        
        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            case 'U':
                userkey = optarg;
                break;
            case 'P':
                password = optarg;
                break;
            /* Timeout opt */
            case 't':
                getopt_timeout(optarg);
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
    print_copyright();

    printf("\n");
  
    printf ("This plugin check for available ASPSMS credits.");
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

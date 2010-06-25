/***
 * Monitoring Plugin - check_tftp
 **
 *
 * check_tftp - Check if a file can be downloaded from tftp.
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
 */

const char *progname  = "check_tftp";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "-H host -F file [-t timeout] [-w warn] [-c crit]";

#include "mp_common.h"

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>


/* Global vars */
char *hostname = NULL;
char *filename = NULL;
thresholds *fetch_thresholds = NULL;
int port = 0;

static size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream) {
  return size*nmemb;
}

int main (int argc, char **argv) {
    
    /* C vars */
    CURL        *curl;
    CURLcode    res;
    char        *url;
    double      size;
    double      time;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        unknown("Cannot catch SIGALRM");

    /* Set Default range */
    setWarnTime(&fetch_thresholds, "5s");
    setCritTime(&fetch_thresholds, "9s");

    /* Parse argumens */
    if (process_arguments (argc, argv) == ERROR)
        unknown("Could not parse arguments");
    
    /* Start plugin timeout */
    alarm(mp_timeout);
    
    /* Magik */
    
    size_t urllen = strlen(hostname)+strlen(filename) + 9;
    
    url = malloc(urllen);
    
    snprintf(url, urllen, "tftp://%s/%s", hostname, filename);
    
    if (mp_verbose > 0) {
        printf("CURL Version: %s\n", curl_version());
        printf("Try fetch %s\n", url);
    }
    
    curl_global_init(CURL_GLOBAL_ALL);
    
    curl = curl_easy_init();
    if(curl) {
        if (curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_TFTP) == CURLE_UNSUPPORTED_PROTOCOL)
            unknown("libcurl don't support tftp.");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
        
        if (mp_verbose > 1)
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        
        if (port != 0)
            curl_easy_setopt(curl, CURLOPT_LOCALPORT, port);
            
        res = curl_easy_perform(curl);
        
        
        curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &time);
        
        curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD , &size);
        
        curl_easy_cleanup(curl);
        if(CURLE_OK != res) {
            critical(curl_easy_strerror(res));
        }
    }

    curl_global_cleanup();
    
    ok("Received %'.0fbyte", size, "| time=%'.0f;%'.0f", time, time);
       
    critical("You should never reach this point.");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_ARGS_HELP,
        MP_ARGS_VERS,
        MP_ARGS_VERB,
        MP_ARGS_HOST,
        MP_ARGS_PORT,
        {"file", required_argument, 0, 'F'},
        MP_ARGS_WARN,
        MP_ARGS_CRIT,
        MP_ARGS_TIMEOUT,
        MP_ARGS_END
    };
   
    if (argc < 4) {
        print_help();
        exit(STATE_OK);
    }
    
    while (1) {
        c = getopt_long(argc, argv, "hVvH:P:F:w:c:t:", longopts, &option);

        if (c == -1 || c == EOF)
            break;
        switch (c) {
            MP_ARGS_CASE_DEF
            MP_ARGS_CASE_HOST
            MP_ARGS_CASE_PORT
            case 'F':
                filename = optarg;
                break;
            MP_ARGS_CASE_WARN_TIME(fetch_thresholds)
            MP_ARGS_CASE_CRIT_TIME(fetch_thresholds)
            MP_ARGS_CASE_TIMEOUT
            case '?':
                usage("");
        }
    }
    
    if (!filename)
        usage("A filename is mandatory.");
    
    if (!hostname)
        usage("A hostname is mandatory.");

    return(OK);
}

void print_help (void) {
    print_revision();
    print_copyright();

    printf("\n");
  
    printf ("This plugin check if a file can be downloaded from tftp.");

    printf("\n\n");

    print_usage();

    printf(MP_ARGS_HELP_DEF);
    printf(MP_ARGS_HELP_HOST);
    printf(MP_ARGS_HELP_PORT("UNDEF"));
    printf(" -F, --file=FILENAME\n");
    printf("      The name of the file to download.\n");
    printf(MP_ARGS_HELP_WARN_TIME("5 sec"));
    printf(MP_ARGS_HELP_CRIT_TIME("9 sec"));
    printf(MP_ARGS_HELP_TIMEOUT);
}

/* vim: set ts=4 sw=4 et syn=c.libdns : */
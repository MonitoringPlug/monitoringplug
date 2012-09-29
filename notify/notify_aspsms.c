/***
 * Monitoring Plugin - notify_aspsms.c
 **
 *
 * notify_aspsms - Send a notification by mail.
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

const char *progname  = "notify_mail";
const char *progdesc  = "Send a notification by mail.";
const char *progvers  = "0.1";
const char *progcopy  = "2012";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--to <DEST> --file <TEMPLATE> | --message <MESSAGE>";

/* MP Includes */
#include "mp_notify.h"
#include "curl_utils.h"

/* Default Includes */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>
#include <errno.h>
#include <string.h>

/* Global Constants */
const char *xml_url = "http://xml1.aspsms.com:5061/xmlsvr.asp";

/* Global Vars */
const char *userkey = NULL;
const char *password = NULL;
char **number = NULL;
int numbers = 0;
char *from = NULL;

int main (int argc, char **argv) {
    /* Local Vars */
    CURL        *curl;
    long int    code;
    FILE *fd;
    char *out;
    char *buf;
    int i;
    struct mp_curl_data query;
    struct mp_curl_data answer;
    struct curl_slist *headers = NULL;
    char        *xmlp;
    int         errorCode = 0;
    char        *errorDescription = NULL;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    if (mp_notify_file) {
        fd = fopen(mp_notify_file, "r");
        if (fd == NULL)
            critical("Can't open '%s'", mp_notify_file);
        out = mp_template(fd);
        fclose(fd);
    } else {
        out = mp_template_str(mp_notify_msg);
    }

    memset(&query, 0, sizeof(struct mp_curl_data));
    mp_curl_recv_data("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
            "<aspsms>\n", sizeof(char), 53, &query);

    mp_asprintf(&buf, "<Userkey>%s</Userkey>\n<Password>%s</Password>\n",
            userkey, password);
    mp_curl_recv_data(buf, sizeof(char), strlen(buf), &query);
    free(buf);

    if (from) {
        mp_asprintf(&buf, "<Originator>%s</Originator>\n", from);
        mp_curl_recv_data(buf, sizeof(char), strlen(buf), &query);
        free(buf);
    }

    mp_curl_recv_data("<Recipient>\n", sizeof(char), 12, &query);
    for(i=0; i < numbers; i++) {
        mp_asprintf(&buf, "<PhoneNumber>%s</PhoneNumber>\n", number[i]);
        mp_curl_recv_data(buf, sizeof(char), strlen(buf), &query);
        free(buf);
    }
    mp_curl_recv_data("</Recipient>\n", sizeof(char), 13, &query);

    mp_asprintf(&buf, "<MessageData>%s</MessageData>\n"
            "<Action>SendTextSMS</Action>\n</aspsms>\n", out);
    mp_curl_recv_data(buf, sizeof(char), strlen(buf), &query);
    free(buf);

    if (mp_verbose > 3)
        printf("%s", query.data);

    /* Init libcurl */
    curl = mp_curl_init();
    answer.data = NULL;
    answer.size = 0;


    /* Setup request */
    curl_easy_setopt(curl, CURLOPT_URL, xml_url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, mp_curl_send_data);
    curl_easy_setopt(curl, CURLOPT_READDATA, (void *)&query);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, mp_curl_recv_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&answer);

    /* Set header */
    buf = mp_malloc(24);
    mp_snprintf(buf, 24, "Content-Length: %d", (int)query.size);
    headers = curl_slist_append(headers, "Content-Type: text/html");
    headers = curl_slist_append(headers, buf);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    free(buf);

    /* Perform request */
    code = mp_curl_perform(curl);

    /* Cleanup libcurl */
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    free(query.data);

    if (mp_verbose > 1) {
        printf("Answer: '%s'\n", answer.data);
    }

    if (code != 200) {
        printf("XML-API request failed.\n");
        return 1;
    }

    /* Parse Answer */
    xmlp = answer.data;

    while((buf = strsep(&xmlp, "<>"))) {
        if (strcmp(buf, "ErrorCode") == 0) {
            errorCode = strtol(strsep(&xmlp, "<>"), NULL, 10);
        } else if (strcmp(buf, "ErrorDescription") == 0) {
            errorDescription = strdup(strsep(&xmlp, "<>"));
        }
    }
    free(answer.data);

    /* XML Error Code */
    if (errorCode != 1) {
        printf("SMS sending failed: %s\n", errorDescription);
        free(errorDescription);
        return 1;
    }
    if (errorDescription)
        free(errorDescription);

    printf("SMS sent.\n");

    return 0;
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_NOTIFY,
        {"userkey", required_argument, 0, 'U'},
        {"password", required_argument, 0, 'P'},
        {"to", required_argument, NULL, (int)'T'},
        {"from", required_argument, NULL, (int)'f'},
        MP_LONGOPTS_END
    };

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_NOTIFY, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_notify(c);

        switch (c) {
            case 'U':
                userkey = optarg;
                break;
            case 'P':
                password = optarg;
                break;
            case 'T':
                mp_array_push(&number, optarg, &numbers);
                break;
            case 'f':
                from = optarg;
                break;
        };
    }

    /* Checks */
    if (!userkey || !password)
        usage("Userkey and password are mandatory.");
    if (numbers == 0)
         usage("--to is mandatory.");
    if (!mp_notify_file && !mp_notify_msg)
        usage("--file or --message is mandatory.");

    return(OK);
}

void print_help(void) {
    print_revision();
    print_copyright();

    printf("\n");

    printf("Notify description: %s", progdesc);

    printf("\n\n");

    print_usage();

    print_help_notify();
    printf(" -T, --to=DESTINATION\n");
    printf("      Number to send SMS to.\n");
    printf(" -f, --from=DESTINATION\n");
    printf("      Number to send SMS from.\n");
}

/* vim: set ts=4 sw=4 et syn=c : */

/***
 * Monitoring Plugin - check_webdav.c
 **
 *
 * check_webdav - Check WebDAV share.
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
#include "curl_utils.h"
#ifdef HAVE_EXPAT
#include "expat_utils.h"
#endif
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
#ifdef HAVE_EXPAT
#include <expat.h>
#endif

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
int do_list = 0;

#ifdef HAVE_EXPAT
struct webdav_list {
    char *path;
    char *type;
    char *status;
    struct webdav_list *next;
};

struct webdav_parser {
    char *name;
    struct webdav_list *list;
};
#endif

/* Function prototype */
#ifdef HAVE_EXPAT
void webdav_startElement(void *userData, const char *name, const char **atts);
void webdav_stopElement(void *userData, const char *name);
void webdav_charData(void *userData, const XML_Char *s, int len);
#endif

int main (int argc, char **argv) {
    /* Local Vars */
    CURL        *curl;
    double      time;
    double      time_total;
    long        code;
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

    /* H */
    struct mp_curl_header headers[] = {
        {"DAV", &dav}, {"Allow", &allow}, {"Content-Type", &contentType},
        {NULL, NULL}
    };

    /* Build query */
    curl = mp_curl_init();

    /* WebDAV OPTIONS */
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "OPTIONS");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, mp_curl_recv_header);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *)headers);

    code = mp_curl_perform(curl);

    if (code != 200)
        critical("WebDav - HTTP Response Code %ld", code);

    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &time_total);

    if (do_list) {
        struct curl_slist *header = NULL;
        struct mp_curl_data query;
        struct mp_curl_data answer;

        /* Init query and answer */
        query.data = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\
                      <D:propfind xmlns:D=\"DAV:\"><D:prop><D:resourcetype/>\
                      </D:prop></D:propfind>";
        query.size = strlen(query.data);
        query.start = 0;
        answer.data = NULL;
        answer.size = 0;

        /* Disable header callback */
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, mp_curl_recv_blackhole);

        /* Set header */
        header = curl_slist_append(header, "Depth: 0");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

        /* Set method */
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PROPFIND");
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        /* IO Callback */
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, mp_curl_send_data);
        curl_easy_setopt(curl, CURLOPT_READDATA, (void *)&query);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, mp_curl_recv_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&answer);

        code = mp_curl_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &time);
        time_total += time;

        if (code != 207) {
            mp_perfdata_float("time", (float)time_total, "s", fetch_thresholds);
            critical("WebDav - HTTP Response Code %ld", code);
        }

#ifdef HAVE_EXPAT
        XML_Parser  parser;
        struct webdav_parser *parserInfo;
        struct webdav_list *list, *nextList;

        parser = XML_ParserCreateNS(NULL, 0);

        parserInfo = mp_malloc(sizeof(struct webdav_parser));
        parserInfo->name = NULL;
        parserInfo->list = NULL;

        XML_SetUserData(parser, parserInfo);
        XML_SetElementHandler(parser, webdav_startElement, webdav_stopElement);
        XML_SetCharacterDataHandler(parser, webdav_charData);

        if (!XML_Parse(parser, answer.data, answer.size, 1)) {
            unknown("%s at line %d\n",
                    XML_ErrorString(XML_GetErrorCode(parser)),
                    (int) XML_GetCurrentLineNumber(parser));
        }
        XML_ParserFree(parser);

        /* Check return */
        if (parserInfo->list == NULL)
            critical("WebDAV - Parsing PROPFIND response faild!");
        if (parserInfo->list->next != NULL)
            critical("WebDAV - Too many answers for PROPFIND Depth:0!");
        if(strstr(parserInfo->list->status, "200") == NULL)
            critical("WebDAV - PROPFIND status is %s", parserInfo->list->status);

        /* For DAV:collection list Depth:1 */
        if (parserInfo->list->type && (strcmp(parserInfo->list->type, "DAV:collection") == 0)) {

            /* Free parserInfo */
            if(mp_verbose > 3)
                printf("PROPFIND %s (Depth:0)\n", url);
            for (list = parserInfo->list; list; list = nextList) {
                if(mp_verbose > 3)
                    printf(" * %s (%s) [%s]\n", list->path, list->type, list->status);

                free(list->path);
                free(list->type);
                free(list->status);

                nextList = list->next;
                free(list);
            }
            parserInfo->list = NULL;
            parserInfo->name = NULL;

            /* New Headers */
            curl_slist_free_all(header);
            header = NULL;
            header = curl_slist_append(header, "Depth: 1");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

            /* Reset query and answer*/
            query.start = 0;
            free(answer.data);
            answer.data = NULL;
            answer.size = 0;

            /* Set method */
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PROPFIND");
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

            curl_easy_setopt(curl, CURLOPT_READFUNCTION, mp_curl_send_data);
            curl_easy_setopt(curl, CURLOPT_READDATA, (void *)&query);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, mp_curl_recv_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&answer);

            code = mp_curl_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &time);
            time_total += time;

            if (code != 207) {
                mp_perfdata_float("time", (float)time_total, "s", fetch_thresholds);
                critical("WebDav - HTTP Response Code %ld", code);
            }

            parser = XML_ParserCreateNS(NULL, 0);
            XML_SetUserData(parser, parserInfo);
            XML_SetElementHandler(parser, webdav_startElement, webdav_stopElement);
            XML_SetCharacterDataHandler(parser, webdav_charData);

            if (!XML_Parse(parser, answer.data, answer.size, 1)) {
                unknown("%s at line %d\n",
                        XML_ErrorString(XML_GetErrorCode(parser)),
                        (int) XML_GetCurrentLineNumber(parser));
            }
            XML_ParserFree(parser);

            /* Free parserInfo */
            if(mp_verbose > 3)
                printf("PROPFIND %s (Depth:1)\n", url);
            for (list = parserInfo->list; list; list = nextList) {
                if(mp_verbose > 3)
                    printf(" * %s (%s) [%s]\n", list->path, list->type, list->status);
                free(list->path);
                free(list->type);
                free(list->status);

                nextList = list->next;
                free(list);
            }
        }
#endif
    }

    /* Cleanup libcurl */
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    mp_perfdata_float("time", (float)time_total, "s", fetch_thresholds);

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

    switch(get_status(time_total, fetch_thresholds)) {
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

#ifdef HAVE_EXPAT
void webdav_startElement(void *userData, const char *name, const char **atts) {
    struct webdav_parser *parserInfo = (struct webdav_parser *)userData;
    struct webdav_list *listNew;

    if (parserInfo->name && (strcmp("DAV:resourcetype", parserInfo->name) == 0)) {
        parserInfo->list->type = strdup(name);
    }

    if (parserInfo->name) {
        free(parserInfo->name);
        parserInfo->name = NULL;
    }

    if (strcmp("DAV:response", name) == 0) {
        listNew = mp_malloc(sizeof(struct webdav_list));
        listNew->path = NULL;
        listNew->next = parserInfo->list;
        listNew->type = NULL;
        listNew->status = NULL;

        parserInfo->list = listNew;
    } else if (strcmp("DAV:href", name) == 0) {
        parserInfo->name = strdup(name);
    } else if (strcmp("DAV:status", name) == 0) {
        parserInfo->name = strdup(name);
    } else if (strcmp("DAV:resourcetype", name) == 0) {
        parserInfo->name = strdup(name);
    }
}
void webdav_stopElement(void *userData, const char *name) {
    struct webdav_parser *parserInfo = (struct webdav_parser *)userData;

    if (parserInfo->name) {
        free(parserInfo->name);
        parserInfo->name = NULL;
    }
}
void webdav_charData(void *userData, const XML_Char *s, int len) {
    struct webdav_parser *parserInfo = (struct webdav_parser *)userData;

    if (parserInfo->name == NULL)
        return;

    if (strcmp("DAV:href", parserInfo->name) == 0) {
        parserInfo->list->path = mp_malloc(len+1);
        memcpy(parserInfo->list->path, s, len);
        parserInfo->list->path[len] = '\0';
    } else if (strcmp("DAV:status", parserInfo->name) == 0) {
        parserInfo->list->status = mp_malloc(len+1);
        memcpy(parserInfo->list->status, s, len);
        parserInfo->list->status[len] = '\0';
    }
}
#endif

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_PORT,
        {"url", required_argument, 0, (int)'u'},
        {"content-type", required_argument, 0, (int)'C'},
        {"allow", required_argument, 0, (int)'a'},
        {"ls", no_argument, &do_list, 1},
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
    print_revision_curl();
#ifdef HAVE_EXPAT
    print_revision_expat();
#endif
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
    printf("     --ls\n");
    printf("      List the directory and check the response.\n");
    print_help_warn_time("5 sec");
    print_help_crit_time("9 sec");
}

/* vim: set ts=4 sw=4 et syn=c : */

/***
 * Monitoring Plugin - check_fcgi_phpfpm.c
 **
 *
 * check_fcgi_phpfpm - This plugin simulate a plugin timeout.
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

const char *progname  = "check_fcgi_phpfpm";
const char *progdesc  = "This plugin simulate a plugin timeout.";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[-t <timeout>]";

/* MP Includes */
#include "mp_common.h"
#include "fcgi_utils.h"
/* Default Includes */
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* Library Includes */
#include <fastcgi.h>
#include <fcgios.h>
#include <fcgiapp.h>
#include <json.h>
#include "json_utils.h"

/* Global Vars */
char *fcgisocket = NULL;
char *query = "/status";

int main (int argc, char **argv) {
    int fcgiSock = -1;
    FCGX_Stream *paramsStream;
    char *pool = NULL;
    char *content, *data;
    int type, count;
    struct json_object  *obj, *slaveobj;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    /* Connect to fcgi server */
    fcgiSock = mp_fcgi_connect(fcgisocket);

    /* Prepare Begin Request */
    FCGI_BeginRequestBody body;
    body.roleB1 = 0x00;
    body.roleB0 = FCGI_RESPONDER;
    body.flags  = 0x000;
    memset(body.reserved, 0, sizeof(body.reserved));
    mp_fcgi_write(fcgiSock, 42, FCGI_BEGIN_REQUEST, (char*)&body, sizeof(body));

    /* Set FCGI Params */
    paramsStream = FCGX_CreateWriter(fcgiSock, 1, 8192, FCGI_PARAMS);
    mp_fcgi_putkv(paramsStream, "REQUEST_METHOD", "GET");
    mp_fcgi_putkv(paramsStream, "SCRIPT_NAME", query);
    mp_fcgi_putkv(paramsStream, "SCRIPT_FILENAME", query);
    mp_fcgi_putkv(paramsStream, "QUERY_STRING", "json&");
    FCGX_FClose(paramsStream);
    FCGX_FreeStream(&paramsStream);

    /* Start request processing by stdin closing */
    mp_fcgi_write(fcgiSock, 42, FCGI_STDIN, NULL, 0);

    /* Wait for answer */
    data = NULL;
    do {
        content = NULL;
        type = mp_fcgi_read(fcgiSock, &content, &count);
        if (type == FCGI_STDOUT)
            data = content;
        else if (content)
            free(content);
    } while (type != FCGI_END_REQUEST);

    /* Skip http headers */
    content = data;
    do {
        (void)strsep(&data, "\n");
    } while (data && data[0] != '\r');

    /* Parse JSON */
    obj = mp_json_tokener_parse(data);

    /* Read pool name */
    mp_json_object_object_get(obj, "pool", &slaveobj);
    pool = mp_strdup(json_object_get_string(slaveobj));

    /* Read accepted connections */
    mp_json_object_object_get(obj, "accepted conn", &slaveobj);
    mp_perfdata_int("accepted_conn", json_object_get_int(slaveobj), "c", NULL);

    /* Read listen queue */
    mp_json_object_object_get(obj, "listen queue", &slaveobj);
    mp_perfdata_int("listen_queue", json_object_get_int(slaveobj), "", NULL);

    /* Read idle processes */
    mp_json_object_object_get(obj, "idle processes", &slaveobj);
    mp_perfdata_int("idle_processes", json_object_get_int(slaveobj), "", NULL);

    /* Read active processes */
    mp_json_object_object_get(obj, "active processes", &slaveobj);
    mp_perfdata_int("active_processes", json_object_get_int(slaveobj), "", NULL);

    free(content);
    json_object_put(obj);

    ok("PHP-FPM: %s", pool);
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        {"socket", required_argument, NULL, (int)'s'},
        {"query", required_argument, NULL, (int)'q'},
        MP_LONGOPTS_END
    };

    if (argc < 2) {
        print_help();
        exit(STATE_OK);
    }

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"s:q:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        switch (c) {
            case 's':
                fcgisocket = optarg;
                break;
            case 'q':
                query = optarg;
                break;
        }
    }

    return(OK);
}

void print_help (void) {
    print_revision();
    print_revision_json();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();

    print_help_default();

    printf(" -s, --socket=<SOCKET>\n");
    printf("      FastCGID socket to connect to.\n");
    printf(" -q, --query=<QUERY>\n");
    printf("      Query string for the PHP-FPM status. (Default to: /status)\n");
}

/* vim: set ts=4 sw=4 et syn=c : */

/***
 * Monitoring Plugin - check_dummy.c
 **
 *
 * check_dummy - This plugin test nothing.
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

const char *progname  = "check_dummy";
const char *progdesc  = "This plugin test nothing.";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "<state> [message]";

/* MP Includes */
#include "mp_common.h"
#include "mysql_utils.h"
/* Default Includes */
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

/* Global Vars */
char **variable = NULL;
char **unit = NULL;
int variables;
#define LONGOPT_VARIABLE MP_LONGOPT_PRIV0
long int *values;

char *default_unit[] = {
    "c", "c",
    "c", "c", "c", "c", "c", "c",
    "", "c", "", "",
    "c"
};

char *default_variable[] = {
    "Bytes_received", "Bytes_sent",
    "Com_delete", "Com_insert", "Com_select", "Com_update", "Com_replace", "Queries",
    "Threads_cached", "Threads_created", "Threads_connected", "Threads_running",
    "Connections"
};

int main (int argc, char **argv) {
    /* Local Vars */
    MYSQL *conn;
    MYSQL_RES *result;
    MYSQL_ROW row;
    int ret;
    int i;
    char *server_version;
    struct timeval  start_time;
    double          time_delta;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);
    gettimeofday(&start_time, NULL);

    /* Connectiong to mysqld */
    conn = mp_mysql_init();

    /* Get server version */
    server_version = mysql_get_server_info(conn);

    /* Get status info */
    if (mp_showperfdata) {
        ret = mysql_query(conn, "SHOW /*!50002 GLOBAL */ STATUS;");
        if (ret != 0)
            critical("Query 'SHOW GLOBAL STATUS' faild: %s", mysql_error(conn));

        result = mysql_store_result(conn);

        while ((row = mysql_fetch_row(result))) {
            for (i=0; i < variables; i++) {
                if (strcmp(variable[i], row[0]) == 0) {
                    mp_perfdata_int(row[0], strtol(row[1], NULL, 10), unit[i], NULL);
                }
            }
        }

        mysql_free_result(result);
    }

    mp_mysql_deinit(conn);

    time_delta = mp_time_delta(start_time);

    ok("MySQL v%s", server_version);

    critical("You should never reach this point.");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        {"variable", required_argument, NULL, (int)LONGOPT_VARIABLE},
        MYSQL_LONGOPTS,
        MP_LONGOPTS_END
    };

    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"t:H:"MYSQL_OPTSTR, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_mysql(c);

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            case LONGOPT_VARIABLE: {
                char *u;
                u = optarg;
                optarg = strsep(&u, ":");
                mp_array_push(&variable, optarg, &variables);
                variables--;
                if (u)
                     mp_array_push(&unit, optarg, &variables);
                else
                    mp_array_push(&unit, "", &variables);
                                   }
            /* Timeout opt */
            case 't':
                getopt_timeout(optarg);
                break;
        }
    }

    /* Apply defaults */
    if (variables == 0) {
        variable = default_variable;
        unit = default_unit;
        variables = 13;
    }

    return(OK);
}

void print_help (void) {
    print_revision();
    print_revision_mysql();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();

    print_help_default();
    print_help_mysql();

    printf("     --variable=VARIABLE[:UNIT]\n");
    printf("      Variable, and unit, to report to perfdata.\n");
}

/* vim: set ts=4 sw=4 et syn=c : */

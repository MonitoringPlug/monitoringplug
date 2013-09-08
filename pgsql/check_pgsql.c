/***
 * Monitoring Plugin - check_pgsql.c
 **
 *
 * check_pgsql - Check PostgreSQL connectivity and status.
 *
 * Copyright (C) 2013 Marius Rieder <marius.rieder@durchmesser.ch>
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

const char *progname  = "check_pgsql";
const char *progdesc  = "Check PostgreSQL connectivity and status.";
const char *progvers  = "0.1";
const char *progcopy  = "2013";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[-H <HOST>] [-u <USER>] [-P <PASSWORD>] [-D <DATABASE>]";

/* MP Includes */
#include "mp_common.h"
#include "pgsql_utils.h"
/* Default Includes */
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

int main (int argc, char **argv) {
    /* Local Vars */
    PGconn   *conn;
    PGresult *res;

    int server_version;
    struct timeval  start_time;
    double          time_delta;

    char *val;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);
    gettimeofday(&start_time, NULL);

    /* Connectiong to PostgreSQL server */
    conn = mp_pgsql_init();

    /* Get server version */
    server_version = PQserverVersion(conn);
    /* Get status info */
    if (mp_showperfdata) {
        res = mp_pgsql_exec(conn, "SELECT COUNT(*) AS conn FROM pg_stat_activity;");
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            critical("Query 'SELECT COUNT(*) AS con FROM pg_stat_activity;' failed: %s",
                    PQerrorMessage(conn));
        }

        val = PQgetvalue(res, 0, 0);
        mp_perfdata_int("conn", strtol(val, NULL, 10), "c", NULL);

        PQclear(res);
    }

    mp_pgsql_deinit(conn);

    time_delta = mp_time_delta(start_time);
    mp_perfdata_float("time", (float)time_delta, "s", NULL);

    ok("PostgreSQL v%s", mp_pgsql_version(server_version));

    critical("You should never reach this point.");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        {"variable", required_argument, NULL, (int)LONGOPT_VARIABLE},
        PGSQL_LONGOPTS,
        MP_LONGOPTS_END
    };

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"H:"PGSQL_OPTSTR, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_pgsql(c);

        switch (c) {
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
            break;
        }
    }

    return(OK);
}

void print_help (void) {
    print_revision();
    print_revision_pgsql();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();

    print_help_default();
    print_help_pgsql();
}

/* vim: set ts=4 sw=4 et syn=c : */

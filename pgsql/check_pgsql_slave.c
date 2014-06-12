/***
 * Monitoring Plugin - check_pgsql_slave.c
 **
 *
 * check_pgsql_slave - Check PostgreSQL r/o slave status and delay.
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

const char *progname  = "check_pgsql_slave";
const char *progdesc  = "Check PostgreSQL r/o slave status and delay.";
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
thresholds *delay_thresholds = NULL;

int main (int argc, char **argv) {
    /* Local Vars */
    PGconn   *conn;
    PGresult *res;
    char *val, *val2;
    float delay = 0;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    /* Connectiong to PostgreSQL server */
    conn = mp_pgsql_init();

    /* Check Recovery state */
    res = mp_pgsql_exec(conn, "SELECT pg_is_in_recovery() AS recovery, NOW() - pg_last_xact_replay_timestamp() AS replication_delay;");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        critical("Query 'SELECT pg_is_in_recovery() AS recovery, NOW() - pg_last_xact_replay_timestamp() AS replication_delay;' failed: %s",
                PQerrorMessage(conn));
    }
    
    val = PQgetvalue(res, 0, 0);
    if (val[0] == 'f') {
        mp_pgsql_deinit(conn);
        critical("PostgreSQL is not in recover mode.");
    }

    /* Calculate delay */
    val = PQgetvalue(res, 0, 1);

    delay = strtol(val, &val2, 10) * 3600;
    delay += strtol(++val2, NULL,10) * 60;
    delay += strtof((val2+=3), NULL);

    val = mp_strdup(val);

    mp_pgsql_deinit(conn);

    mp_perfdata_float("delay", (float)delay, "s", delay_thresholds);

    switch(get_status(delay, delay_thresholds)) {
        case STATE_OK:
            free_threshold(delay_thresholds);
            ok("PostgreSQL Slave Delay: %s", val);
            break;
        case STATE_WARNING:
            free_threshold(delay_thresholds);
            warning("PostgreSQL Slave Delay: %s", val);
            break;
        case STATE_CRITICAL:
            free_threshold(delay_thresholds);
            critical("PostgreSQL Slave Delay: %s", val);
            break;
    }
    free_threshold(delay_thresholds);

    critical("You should never reach this point.");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        PGSQL_LONGOPTS,
        MP_LONGOPTS_END
    };

    /* Set default */
    mp_threshold_set_warning_time(&delay_thresholds, "300s");
    mp_threshold_set_critical_time(&delay_thresholds, "3600s");

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"H:w:c:"PGSQL_OPTSTR, longopts, &option);

        getopt_wc_time(c, optarg, &delay_thresholds);

        if (c == -1 || c == EOF)
            break;

        getopt_pgsql(c);

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
    print_help_warn_time("300 sec");
    print_help_crit_time("3600 sec");
}

/* vim: set ts=4 sw=4 et syn=c : */

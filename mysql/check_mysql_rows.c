/***
 * Monitoring Plugin - check_mysql_rows.c
 **
 *
 * check_mysql_rows - Check mysql table row count.
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

const char *progname  = "check_mysql_rows";
const char *progdesc  = "Check mysql table row count.";
const char *progvers  = "0.1";
const char *progcopy  = "2011";
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
char *table = NULL;
thresholds *row_thresholds = NULL;

int main (int argc, char **argv) {
    /* Local Vars */
    MYSQL *conn;
    MYSQL_RES *result;
    MYSQL_ROW row;
    char *query;
    int ret;
    int rows;
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

    /* Build query */
    query = mp_malloc(23+strlen(table));
    mp_sprintf(query, "SELECT COUNT(*) from %s;", table);

    /* Get row count info */
    ret = mysql_query(conn, query);
    if (ret != 0)
        critical("Query '%s' faild: %s", query, mysql_error(conn));

    result = mysql_store_result(conn);

    row = mysql_fetch_row(result);

    rows = strtol(row[0], NULL, 10);

    mysql_free_result(result);

    mp_mysql_deinit(conn);

    time_delta = mp_time_delta(start_time);

    mp_perfdata_int("rows", rows, "", row_thresholds);
    mp_perfdata_float("time", (float)time_delta, "s", NULL);

    switch (get_status(rows, row_thresholds)) {
        case STATE_OK:
            ok("Rowcount for %s.%s %ld", mp_mysql_db, table, rows);
        case STATE_WARNING:
            warning("Rowcount for %s.%s %ld", mp_mysql_db, table, rows);
        case STATE_CRITICAL:
            critical("Rowcount for %s.%s %ld", mp_mysql_db, table, rows);
    }

    critical("You should never reach this point.");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        {"table", required_argument, NULL, (int)'T'},
        MYSQL_LONGOPTS,
        MP_LONGOPTS_END
    };

    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"t:w:c:H:T:"MYSQL_OPTSTR, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_mysql(c);
        getopt_wc(c, optarg, &row_thresholds);

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            case 'T':
                table = optarg;
            /* Timeout opt */
            case 't':
                getopt_timeout(optarg);
                break;
        }
    }

    /* Check requirements */
    if (!mp_mysql_db)
        usage("Database is mandatory.");
    if (!table)
        usage("Table is mandatory.");

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

    printf(" -T, --table=TABLE\n");
    printf("      Table to count rows of.\n");
    print_help_warn("rows", "none");
    print_help_crit("rows", "none");
}

/* vim: set ts=4 sw=4 et syn=c : */

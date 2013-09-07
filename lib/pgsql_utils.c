/***
 * Monitoring Plugin - pgsql_utils.c
 **
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

#include "mp_common.h"
#include "pgsql_utils.h"

#include <libpq-fe.h>

#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char *mp_pgsql_host = NULL;
char *mp_pgsql_user = NULL;
char *mp_pgsql_pass = NULL;
char *mp_pgsql_db = NULL;
char *mp_pgsql_port = NULL;

PGconn *mp_pgsql_init(void) {
    PGconn *conn;

    /* Make a connection to the database */
    conn = PQsetdbLogin(mp_pgsql_host, mp_pgsql_port,
            NULL, NULL, mp_pgsql_db, mp_pgsql_user, mp_pgsql_pass);

    /* Check to see that the backend connection was successfully made */
    if (PQstatus(conn) != CONNECTION_OK) {
        unknown("Connection to database failed: %s", PQerrorMessage(conn));
    }

    return conn;
}

PGresult *mp_pgsql_exec(PGconn *conn, const char *query) {
    PGresult *res;
    
    res = PQexec(conn, query);
    if (mp_verbose > 3 && PQresultStatus(res) ==  PGRES_TUPLES_OK) {
        printf("Query: %s\n", query);
        PQprintOpt options = {0};
        options.header = 1;
        options.fieldSep  = "|";
        PQprint(stdout, res, &options);
    }

    return res;
}

void mp_pgsql_deinit(PGconn *conn) {
    PQfinish(conn);
}

void getopt_pgsql(int c) {
    switch ( c ) {
        case 'H':
            getopt_host(optarg, (const char **)&mp_pgsql_host);
            break;
        case 'u':
            mp_pgsql_user = optarg;
            break;
        case 'p':
            mp_pgsql_pass = optarg;
            break;
        case 'D':
            mp_pgsql_db = optarg;
            break;
        case 'P':
            mp_pgsql_port = optarg;
            break;
    }
}

void print_help_pgsql(void) {
    print_help_host();
    printf(" -u, --user=USER\n");
    printf("      The PostgreSQL user name to use when connecting to the server.\n");
    printf(" -p, --password=PASSWORD\n");
    printf("      The password to use when connecting to the server.\n");
    printf(" -D, --database=DATABASE\n");
    printf("      The database to use.\n");
    printf(" -P, --port=PORT\n");
    printf("      The TCP/IP port number or UNIX Socket-Dir to use for the connection.\n");
}

void print_revision_pgsql(void) {
    int pqlibversion;
    
    pqlibversion = PQlibVersion();

    printf(" libqp v%s", mp_pgsql_version(pqlibversion));
}

char *mp_pgsql_version(int version) {
    char *ret;
    int major, minor, revision;

    revision = version % 100;
    version /= 100;
    minor = version % 100;
    major = version /= 100;

    if (mp_asprintf(&ret, "%d.%d.%d", major, minor, revision) == 0)
        return NULL;

    return ret;
}

/* vim: set ts=4 sw=4 et syn=c : */

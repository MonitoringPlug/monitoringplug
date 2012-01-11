/***
 * Monitoring Plugin - mysql_utils.c
 **
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id$
 */

#include "mp_common.h"
#include "mysql_utils.h"

#include <mysql.h>

#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char *mp_mysql_host = NULL;
char *mp_mysql_user = NULL;
char *mp_mysql_pass = NULL;
char *mp_mysql_db = NULL;
int mp_mysql_port = 0;
char *mp_mysql_socket = NULL;

MYSQL *mp_mysql_init(void) {
    MYSQL *conn, *ret;

    mysql_library_init(0, NULL, NULL);

    conn = mysql_init(NULL);
    if (conn == NULL)
        unknown("MySQL library initialisation faild.");

    mysql_options(conn, MYSQL_READ_DEFAULT_GROUP, progname);

    ret = mysql_real_connect(conn, mp_mysql_host, mp_mysql_user,
            mp_mysql_pass, mp_mysql_db, mp_mysql_port, mp_mysql_socket, 0);

    if (ret == NULL)
        unknown("MySQL connection faild: %s", mysql_error(conn));

    return conn;
}

void mp_mysql_deinit(MYSQL *conn) {
    mysql_close(conn);
    mysql_library_end();
}

void getopt_mysql(int c) {
    switch ( c ) {
        case 'H':
            getopt_host(optarg, (const char **)&mp_mysql_host);
            break;
        case 'u':
            mp_mysql_user = optarg;
            break;
        case 'p':
            mp_mysql_pass = optarg;
            break;
        case 'D':
            mp_mysql_db = optarg;
            break;
        case 'P':
            getopt_port(optarg, &mp_mysql_port);
            break;
        case 'S':
            mp_mysql_socket = optarg;
            break;
    }
}

void print_help_mysql(void) {
    print_help_host();
    printf(" -u, --user=USER\n");
    printf("      The MySQL user name to use when connecting to the server.\n");
    printf(" -p, --password=PASSWORD\n");
    printf("      The password to use when connecting to the server.\n");
    printf(" -D, --database\n");
    printf("      The database to use.\n");
    printf(" -P, --port=PORT\n");
    printf("      The TCP/IP port number to use for the connection.\n");
    printf(" -S, --socket\n");
    printf("      For connections to localhost, the Unix socket file to use.\n");
}

void print_revision_mysql(void) {
    printf(" mysql v%s\n", mysql_get_client_info());
}

/* vim: set ts=4 sw=4 et syn=c : */

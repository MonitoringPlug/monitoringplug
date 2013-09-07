/***
 * Monitoring Plugin - pgsql_utils.h
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * $Id$
 */

#ifndef _PGSQL_UTILS_H_
#define _PGSQL_UTILS_H_

#include "config.h"
#include <libpq-fe.h>

/* The global PostgreSQL vars. */
/** Holds the PostgreSQL hostname. */
extern char *mp_pgsql_host;
/** Holds the PostgreSQL username. */
extern char *mp_pgsql_user;
/** Holds the PostgreSQL password. */
extern char *mp_pgsql_pass;
/** Holds the PostgreSQL DB/Schema name. */
extern char *mp_pgsql_db;
/** Holds the PostgreSQL network port or socket path. */
extern char *mp_pgsql_port;

/** PostgreSQL specific short option string. */
#define PGSQL_OPTSTR "u:p:D:P:"
/** PostgreSQL specific longopt struct. */
#define PGSQL_LONGOPTS {"user", required_argument, NULL, (int)'u'}, \
                       {"password", required_argument, NULL, (int)'p'}, \
                       {"database", required_argument, NULL, (int)'D'}, \
                       {"port", required_argument, NULL, (int)'P'}

/**
 * Init the PostgreSQL connection and return the db handler.
 * \return Return a PostgreSQL connection handler.
 */
PGconn *mp_pgsql_init(void);

/**
 * Execute a Command on a PostgreSQL connection
 * \para[in] conn PostgreSQL connection.
 * \para[in] query SQL Query to execute.
 * \return Return a PGresult Struct or NULL.
 */
PGresult *mp_pgsql_exec(PGconn *conn, const char *query);

/**
 * Cleanup the pq library.
 * \para[in] conn PostgreSQL connection handler to close.
 */
void mp_pgsql_deinit(PGconn *conn);

/**
 * Handle PostgreSQL related command line options.
 * \param[in] c Command line option to handle.
 */
void getopt_pgsql(int c);

/**
 * Print the help for the PostgreSQL related command line options.
 */
void print_help_pgsql(void);

/**
 * Print the PostgreSQL library revision.
 */
void print_revision_pgsql(void);

/**
 * Return the Version string to a version integer
 */
char *mp_pgsql_version(int version);

#endif /* _PGSQL_UTILS_H_ */

/* vim: set ts=4 sw=4 et syn=c : */

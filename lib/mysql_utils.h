/***
 * Monitoring Plugin - mysql_utils.h
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

#ifndef _MYSQL_UTILS_H_
#define _MYSQL_UTILS_H_

#include "config.h"
#include <mysql.h>

/* The global mysql vars. */
/** Holds the MySQL hostname. */
extern char *mp_mysql_host;
/** Holds the MySQL username. */
extern char *mp_mysql_user;
/** Holds the MySQL password. */
extern char *mp_mysql_pass;
/** Holds the MySQL DB/Schema name. */
extern char *mp_mysql_db;
/** Holds the MySQL network port. */
extern int mp_mysql_port;
/** Holds the MySQL socket name. */
extern char *mp_mysql_socket;

/** MySQL specific short option string. */
#define MYSQL_OPTSTR "u:p:D:P:S:"
/** MySQL specific longopt struct. */
#define MYSQL_LONGOPTS {"user", required_argument, NULL, (int)'u'}, \
                       {"password", required_argument, NULL, (int)'p'}, \
                       {"database", required_argument, NULL, (int)'D'}, \
                       {"port", required_argument, NULL, (int)'P'}, \
                       {"socket", required_argument, NULL, (int)'S'}

/**
 * Init the MySQL connection and return the db handler.
 * \return Return a MYSQL connection handler.
 */
MYSQL *mp_mysql_init(void);

/**
 * Cleanup the mysql library.
 * \para[in] conn MYSQL connection handler to close.
 */
void mp_mysql_deinit(MYSQL *conn);

/**
 * Handle MySQL related command line options.
 * \param[in] c Command line option to handle.
 */
void getopt_mysql(int c);

/**
 * Print the help for the MySQL related command line options.
 */
void print_help_mysql(void);

/**
 * Print the MySQL library revision.
 */
void print_revision_mysql(void);

#endif /* _MYSQL_UTILS_H_ */

/* vim: set ts=4 sw=4 et syn=c : */

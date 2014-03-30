/***
 * Monitoring Plugin - ldap_utils.h
 **
 *
 * Copyright (C) 2014 Marius Rieder <marius.rieder@durchmesser.ch>
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

#ifndef _LDAP_UTILS_H_
#define _LDAP_UTILS_H_

#include "config.h"
#include <ldap.h>

/* The global LDAP vars. */
/** Holds the LDAP URI */
extern char *mp_ldap_uri;
/** Holds the Bind DN */
extern char *mp_ldap_binddn;
/** Holds the Bind DN Password */
extern char *mp_ldap_pass;
/** Holds the Base DN */
extern char *mp_ldap_basedn;

/** LDAP specific short option string. */
#define LDAP_OPTSTR "D:W:b:"
/** LDAP specific longopt struct. */
#define LDAP_LONGOPTS {"binddn", required_argument, NULL, (int)'D'}, \
                      {"password", required_argument, NULL, (int)'W'}, \
                      {"basedb", required_argument, NULL, (int)'b'}, \

/**
 * Handle LDAP related command line options.
 * \param[in] c Command line option to handle.
 */
void getopt_ldap(int c);

/**
 * Print the help for the LDAP related command line options.
 */
void print_help_ldap(void);

/**
 * Print the LDAP library revision.
 */
void print_revision_ldap(void);

#endif /* _LDAP_UTILS_H_ */

/* vim: set ts=4 sw=4 et syn=c : */

/***
 * Monitoring Plugin - virt_utils.h
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

#ifndef VIRT_UTILS_H_
#define VIRT_UTILS_H_

#include <libvirt/libvirt.h>

/* The global libvirt vars. */
/** Holds the username for the libvirt connection. */
extern char *mp_virt_username;
/** Holds the password for the libvirt connection. */
extern char *mp_virt_password;
/** Holds the uri for the libvirt connection. */
extern char *mp_virt_uri;

/** LibVirt specific short option string. */
#define VIRT_OPTSTR "C:u:p:"
/** LibVirt specific longopt struct. */
#define VIRT_LONGOPTS {"connect", required_argument, NULL, (int)'C'}, \
                      {"username", required_argument, NULL, (int)'u'}, \
                      {"password", required_argument, NULL, (int)'p'}

/**
 * Open a LibVirt connection.
 * \return New created LibVirt connection.
 */
virConnectPtr virt_connect();

/**
 * Show errors from a LibVirt connection.
 * \param[in] conn LibVirt connection to show errors from.
 */
void virt_showError(virConnectPtr conn);

/**
 * Callback to hanlde libvirt authentication requests.
 * \param[out] cred Array of LibVirt credential questions.
 * \param[in] ncred Number of Credentials asked.
 * \param[in] cbdata User defined callback data.
 */
int virt_authCallback(virConnectCredentialPtr cred, unsigned int ncred, void *cbdata);

/**
 * Handle libvirt related command line options.
 * \param[in] c Command line option to handle.
 */
void getopt_virt(int c);

/**
 * Print the help for the libvirt related command line options.
 */
void print_help_virt(void);

/**
 * Print the libvirt library revision.
 */
void print_revision_virt(void);

#endif /* VIRT_UTILS_H_ */

/* vim: set ts=4 sw=4 et syn=c : */

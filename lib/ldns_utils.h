/***
 * Monitoring Plugin - ldns_utils.h
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

#ifndef _DNS_UTILS_H_
#define _DNS_UTILS_H_

#include <ldns/ldns.h>
#include "config.h"

/* The global libvirt vars. */
/** Should TCP be used by default. */
extern int mp_ldns_usevc;

/** LibVirt specific short option string. */
#define LDNS_OPTSTR "X"
/** LibVirt specific longopt struct. */
#define LDNS_LONGOPTS {"tcp", no_argument, NULL, (int)'X'}

/**
 * create a ldns_resolver
 * \param[in] dnsserver Name of the dns server, may be NULL.
 * \return ldns_resolver The created ldns_resolver.
 */
ldns_resolver* createResolver(const char *dnsserver);

/**
 * enable DNSSEC on a resolver
 * \param[in] res The resolver to dnssec enable.
 * \return void
 */
void resolverEnableDnssec(ldns_resolver *res);

/**
 * ldns_resolver_query wrapper for debug.
 * \para[in] r Resolver to query.
 * \para[in] name Name to ask for.
 * \para[in] t Type to ask for.
 * \para[in] c Class to ask for.
 * \para[in] flags Query flags.
 * \return Return the ldns_pkt received.
 */
ldns_pkt *mp_ldns_resolver_query(const ldns_resolver *r, const ldns_rdf *name,
        ldns_rr_type t, ldns_rr_class c, uint16_t flags);

/**
 * Get A and AAAA records for a hostname
 * \para[in] res Resolver to use for the query.
 * \para[in] hostrdf RDF of the hostname to resolv.
 */
ldns_rr_list* getaddr_rdf(ldns_resolver *res, ldns_rdf *hostrdf);

/**
 * resolve the hostname (or ip) with the give resolver.
 * \param[in] res The resolver to use, may be NULL.
 * \param[in] hostname The hostname to resolv
 * \return ldns_rdf of type A or AAAA if IPv6 is enabled.
 */
ldns_rr_list* getaddr(ldns_resolver *res, const char *hostname);

/**
 * load dnssec keys from a file
 * \param[in] filename The path of the key file.
 * \return The ldns_rr_list of loaded keys or NULL
 */
ldns_rr_list* loadKeyfile(const char *filename);

/**
 * load dnssec anchor keys from a file
 * \param[in] filename The path of the key file.
 * \return The ldns_rr_list of loaded keys or NULL
 */
ldns_rr_list* loadAnchorfile(const char *filename);

/**
 * Handle LDNS related command line options.
 * \param[in] c Command line option to handle.
 */
void getopt_ldns(int c);

/**
 * Print the help for the LDNS related command line options.
 */
void print_help_ldns(void);

#endif /* _DNS_UTILS_H_ */

/* vim: set ts=4 sw=4 et syn=c : */

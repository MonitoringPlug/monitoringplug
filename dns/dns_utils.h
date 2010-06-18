/***
 * Monitoring Plugin - dns_utils.h
 **
 *
 * Copyright (C) 2010 Marius Rieder <marius.rieder@durchmesser.ch>
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
 */

#ifndef _DNS_UTILS_H_
#define _DNS_UTILS_H_

#include <ldns/ldns.h>
#include "config.h"

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
 * resolve the hostname (or ip) with the give resolver.
 * \param[in] res The resolver to use, may be NULL.
 * \param[in] hostname The hostname to resolv
 * \return ldns_rdf of type A or AAAA if IPv6 is enabled.
 */
ldns_rdf* getaddr(ldns_resolver *res, const char *hostname);

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

#endif /* _DNS_UTILS_H_ */

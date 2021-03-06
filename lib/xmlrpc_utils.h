/***
 * Monitoring Plugin - xmlrpc_utils.h
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

#ifndef _XMLRPC_UTILS_H_
#define _XMLRPC_UTILS_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <xmlrpc.h>

/**
 * Init the XML-RPC environment.
 * \return Return a pointer to the xmlrpc_env of NULL.
 */
xmlrpc_env mp_xmlrpc_init(void);

/**
 * Wrapper around unknown to print fault from env.
 * \param[in] env xmlrpc_env to handle fault from.
 */
void unknown_if_xmlrpc_fault(xmlrpc_env *env);

#endif /* _XMLRPC_UTILS_H_ */

/* vim: set ts=4 sw=4 et syn=c : */

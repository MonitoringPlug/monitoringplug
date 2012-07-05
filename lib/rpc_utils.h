/***
 * Monitoring Plugin - rpc_utils.h
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

#ifndef RPC_UTILS_H_
#define RPC_UTILS_H_

#include <rpc/rpc.h>
#include <rpcsvc/mount.h>

#define RPC_BUF_LEN 128

/**
 * Shutdown RPC connection in case of a timeout.
 * \param[in] signo Interupt number.
 */
void rpc_timeout_alarm_handler(int signo);

/**
 * Get a rpcent by name or number.
 * \param[in] prog Program name or number.
 * \return Return a copy of the rpcent matching prog.
 */
struct rpcent *rpc_getrpcent(const char *prog);

/**
 * Get the RPC prognum by name or number.
 * \param[in] prog Program name or number.
 * \return Return prognum matching prog.
 */
unsigned long rpc_getprognum(const char *prog);

/**
 * Ping a RPC-Program
 * \param[in] hostname Host tpo ping.
 * \param[in] programm rpcent of programm to ping.
 * \param[in] version Protocol version number to ping.
 * \param[in] proto Name of the transport protocol.
 * \param[in] to Time to wait for a ansver.
 * \return Return the clnt_call return value.
 */
int rpc_ping(char *hostname, struct rpcent *programm, unsigned long version,
        char *proto, struct timeval to);

/**
 * Decode exports XDSs
 * \param[in] xdrs Data to decode.
 * \param[out] export Linked list of exports.
 * \return Return 0 on success, 1 otherwise.
 */
bool_t mp_xdr_exports(XDR *xdrs, exports *export);

#endif /* RPC_UTILS_H_ */

/* vim: set ts=4 sw=4 et syn=c : */

/***
 * Monitoring Plugin - rpc_utils.h
 **
 *
 * Copyright (C) 2011 Marius Rieder <marius.rieder@durchmesser.ch>
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

#ifndef RPC_UTILS_H_
#define RPC_UTILS_H_

#include <rpc/rpc.h>
#include <rpcsvc/mount.h>

#define RPC_BUF_LEN 128

struct rpcent *rpc_getrpcent(const char *prog);
unsigned long rpc_getprognum(const char *prog);
int rpc_ping(char *hostname, struct rpcent *programm, unsigned long version, char *proto, struct timeval to);
bool_t mp_xdr_exports(XDR *xdrs, exports *export);


#endif /* RPC_UTILS_H_ */

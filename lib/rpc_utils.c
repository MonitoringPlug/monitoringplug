/***
 * monitoringplug - rpc_utils.c
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

#include "rpc_utils.h"
#include <mp_utils.h>
#include <mp_common.h>
#include <ctype.h>
#include <string.h>

#include <rpc/rpc.h>


CLIENT *rpc_udp_connect(struct sockaddr_in *addr, const char *prog, const int vers) {
    unsigned long prognum;
    CLIENT *client;
    struct timeval to;
    int sock = RPC_ANYSOCK;

    to.tv_sec = mp_timeout;
    to.tv_usec = 0;

    prognum = rpc_getprognum(prog);

    client = clntudp_create(addr, prognum, vers, to, &sock);
    if (client == NULL) {
        if (mp_verbose)
            clnt_pcreateerror("rpcinfo");
        printf("program %s is not available", prog);
    }

    return client;
}

CLIENT *rpc_tcp_connect(struct sockaddr_in *addr, const char *prog, const int vers) {
    unsigned long prognum;
    CLIENT *client;
    int sock = RPC_ANYSOCK;

    prognum = rpc_getprognum(prog);

    client = clnttcp_create(addr, prognum, vers, &sock, 0, 0);
    if (client == NULL) {
        if (mp_verbose)
            clnt_pcreateerror("rpcinfo");
        printf("program %s is not available", prog);
    }

    return client;
}

static u_long rpc_getprognum(const char *prog) {
    struct rpcent *rpc;

    if (isalpha(*prog)) {
        rpc = getrpcbyname(prog);
        if (rpc == NULL) {
            unknown("%s is unknown service\n", prog);
        }
        return rpc->r_number;
    } else {
        return (u_long) atoi(prog);
    }
}


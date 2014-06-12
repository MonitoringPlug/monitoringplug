/***
 * Monitoring Plugin - rpc_utils.c
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

#include "rpc_utils.h"
#include <mp_utils.h>
#include <mp_common.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>

#include <unistd.h>
#include <sys/types.h>

#include <rpc/rpc.h>

extern CLIENT *client;

void rpc_timeout_alarm_handler(int signo) {
    if (signo == SIGALRM) {
        if (client) {
            clnt_destroy(client);
        }
        critical("RPC timed out after %d seconds\n", mp_timeout);
    }
}

struct rpcent *rpc_getrpcent(const char *prog) {
    struct rpcent *ent, *ret;

    if (isalpha(*prog)) {
        ent = getrpcbyname((char *)prog);
    } else {
        ent = getrpcbynumber(atoi(prog));
    }

    if (ent == NULL)
       return NULL;

    ret = mp_malloc(sizeof(struct rpcent));
    ret->r_name = mp_strdup(ent->r_name);
    ret->r_number = ent->r_number;

    return ret;
}

unsigned long rpc_getprognum(const char *prog) {
    struct rpcent *rpc;

    if (isalpha(*prog)) {
        rpc = getrpcbyname((char *)prog);
        if (rpc == NULL) {
            return 0;
        }
        return rpc->r_number;
    } else {
        return (u_long) atoi(prog);
    }
}

int rpc_ping(char *hostname, struct rpcent *programm, unsigned long version, char *proto, struct timeval to) {
    int ret;

    client = clnt_create(hostname, programm->r_number, version, proto);
    if (client == NULL) {
        return -1;
    }

    client->cl_auth = authunix_create_default();

    ret = clnt_call(client, ((u_long)0), (xdrproc_t) xdr_void,  NULL,
            (xdrproc_t) xdr_void, NULL, to);

    auth_destroy(client->cl_auth);
    clnt_destroy(client);

    return ret;
}

bool_t mp_xdr_exports(XDR *xdrs, exports *export) {
    bool_t more;

    switch (xdrs->x_op) {
        /* Free export list */
        case XDR_FREE: {
            exportnode *node;
            exportnode *next;
            groupnode *group;
            groupnode *next_group;

            for(node = *export; node; node = next) {
                // Free mount path
                if (!xdr_string(xdrs, &node->ex_dir, MNTPATHLEN))
                    return (FALSE);

                for(group = node->ex_groups; group; group = next_group) {
                    if (!xdr_string(xdrs, &group->gr_name, MNTNAMLEN))
                        return (FALSE);
                    next_group = group->gr_next;
                    free(group);
                }

                next = node->ex_next;
                free(node);
            }
            break;
        }
        case XDR_ENCODE: {
            critical("mp_xdr_exports XDR_ENCODE not supported.");
            break;
        }
        case XDR_DECODE: {
            /* Decode struct */
            exportnode *node;
            exportnode *prev = NULL;
            groupnode *group;
            groupnode *prev_group = NULL;

            *export = NULL;

            if (!xdr_bool(xdrs, &more))
                return (FALSE);

            while (more) {
                // Init new exportnode
                node = (exportnode *)mp_malloc(sizeof (struct exportnode));
                node->ex_dir = NULL;
                node->ex_groups = NULL;
                node->ex_next = NULL;

                // Read mount path
                if (!xdr_string(xdrs, &node->ex_dir, MNTPATHLEN))
                    return (FALSE);

                // Read group list
                if (!xdr_bool(xdrs, &more))
                    return (FALSE);
                while (more) {
                    // Init new groupnode
                    group = (groupnode *)mp_malloc(sizeof (struct groupnode));
                    group->gr_name = NULL;
                    group->gr_next = NULL;

                    // Read group name
                    if (!xdr_string(xdrs, &group->gr_name, MNTNAMLEN))
                        return (FALSE);

                    if (node->ex_groups == NULL || prev_group == NULL) {
                        node->ex_groups = group;
                        prev_group = group;
                    } else {
                        prev_group->gr_next = group;
                        prev_group = group;
                    }

                    // More group list
                    if (!xdr_bool(xdrs, &more))
                        return (FALSE);
                }

                if (*export == NULL) {
                    *export = node;
                    prev = node;
                } else {
                    prev->ex_next = node;
                    prev = node;
                }

                // More export list
                if (!xdr_bool(xdrs, &more))
                    return (FALSE);
            }
            break;
        }
    }
    return (TRUE);
}

/* vim: set ts=4 sw=4 et syn=c : */

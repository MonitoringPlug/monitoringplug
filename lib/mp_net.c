/***
 * monitoringplug - mp_net.c
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

#include "mp_common.h"
#include "mp_net.h"
#include "mp_utils.h"

#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


char *mp_ip2str(const struct sockaddr *sa) {
    char *ip;
#ifdef USE_IPV6
    ip = mp_malloc(INET6_ADDRSTRLEN+1);
#else
    ip = mp_malloc(INET_ADDRSTRLEN+1);
#endif

    switch(sa->sa_family) {
        case AF_INET:
            inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
                    ip, INET_ADDRSTRLEN);
            break;
#ifdef USE_IPV6
        case AF_INET6:
            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
                    ip, INET6_ADDRSTRLEN);
            break;
#endif
        default:
            strcpy(ip, "Unknown AF");
            return ip;
    } // switch(sa->sa_family)
    return ip;
}

int mp_connect(const char *hostname, int port, int family, int type) {
    int sd;
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    char *buffer;

    memset (&hints, 0, sizeof (hints));
#ifdef USE_IPV6
    hints.ai_family = family;
#else
    hints.ai_family = AF_INET;
#endif
    hints.ai_socktype = type;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    buffer = mp_malloc(6);
    mp_snprintf(buffer, 6, "%d", port);

    if (getaddrinfo (hostname, buffer, &hints, &result) != 0)
        unknown("Can't resolv %s", hostname);

    free(buffer);

    for(rp = result; rp != NULL; rp = rp->ai_next) {
        if (mp_verbose >= 1) {
            buffer = mp_malloc(INET6_ADDRSTRLEN+1);
            buffer = mp_ip2str(rp->ai_addr);
            printf("Connect to %s\n", buffer);
            free(buffer);
        }
        sd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        if (sd == -1)
            continue;

        if(connect(sd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;

        close(sd);
    }

    if(rp == NULL)
        critical("Can't connect to %s:%d", hostname, port);

    freeaddrinfo(result);

    return sd;
}

void mp_disconnect(int sd) {
    shutdown(sd, SHUT_RDWR);
    close(sd);
}

/* vim: set ts=4 sw=4 et syn=c : */

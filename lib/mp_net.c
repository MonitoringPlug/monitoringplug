/***
 * Monitoring Plugin - mp_net.c
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

#include "mp_common.h"
#include "mp_net.h"
#include "mp_utils.h"

#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


char *mp_ip2str(const struct sockaddr *sa, socklen_t len) {
    int error;
    char hostname[NI_MAXHOST] = "";

    error = getnameinfo(sa, len, hostname, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

    if (error)
        return strdup(gai_strerror(error));
    return strdup(hostname);
}

struct addrinfo *mp_getaddrinfo(const char *hostname, int port, int family, int type) {
    struct addrinfo hints;
    struct addrinfo *result;
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

    if (getaddrinfo (hostname, buffer, &hints, &result) != 0) {
        free(buffer);
        unknown("Can't resolv %s", hostname);
    }
    free(buffer);

    return result;
}

int mp_connect(const char *hostname, int port, int family, int type) {
    int sd;
    char *name;
    struct addrinfo *result, *rp;

    result = mp_getaddrinfo(hostname, port, family, type);

    for(rp = result; rp != NULL; rp = rp->ai_next) {
        if (mp_verbose >= 1) {
            name = mp_ip2str(rp->ai_addr, rp->ai_addrlen);
            printf("Connect to %s\n", name);
            free(name);
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

unsigned short int mp_ip_csum(unsigned short int *addr, int len) {
    int sum = 0;
    unsigned short int *w = addr;

    for (;len > 0; len -= sizeof (unsigned short int)) {
        sum += *w++;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

/* vim: set ts=4 sw=4 et syn=c : */

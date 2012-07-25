/***
 * Monitoring Plugin - mp_net.h
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

#ifndef _MP_NET_H_
#define _MP_NET_H_

#include <sys/socket.h>
#include <netdb.h>

/**
 * Get a string representation of a sockaddr/IP
 * \para[in] sa Sockaddr to convert.
 * \para[in] len Sockaddr length.
 * \return Return the string representing the sockaddr.
 */
char *mp_ip2str(const struct sockaddr *sa, socklen_t len);

/**
 * Get a addrinfo struct matching the input.
 * Wrapper around getaddrinfo.
 * \para[in] hostname Hostname to get addr info for.
 * \para[in] port Port to get addr info for.
 * \para[in] family Connection protocol to get addr info for.
 * \para[in] type Connection type to get addr info for.
 * \return Return the matching addrinfo struct.
 */
struct addrinfo *mp_getaddrinfo(const char *hostname, int port, int family, int type);

/**
 * Open a network socket.
 * \para[in] hostname Hostname to connect to.
 * \para[in] port Port to connect to.
 * \para[in] family Connection protocol family.
 * \para[in] type Connection type.
 * \return Return the connected socket.
 */
int mp_connect(const char *hostname, int port, int family, int type);

/**
 * Close a open network socket.
 * \para[in] sd Socket to shutdown and close.
 */
void mp_disconnect(int sd);

/**
 * Calculate ip checksum
 * \para[in] addr Pointer to the IP header.
 * \para[in] len Number of bytes to process.
 * \return Return the checksum as short int.
 */
unsigned short int mp_ip_csum(unsigned short int *addr, int len);

/**
 * Receive a line from a socket
 * \para[in] sd Socket to read from.
 * \return Return a new allocated string containing a line or NULL.
 */
char *mp_recv_line(int sd);

#endif /* _MP_NET_H_ */

/* vim: set ts=4 sw=4 et syn=c : */

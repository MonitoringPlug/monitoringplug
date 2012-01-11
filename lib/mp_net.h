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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id$
 */

#ifndef MP_NET_H_
#define MP_NET_H_

#include <sys/socket.h>
#include <netdb.h>

char *mp_ip2str(const struct sockaddr *sa);
struct addrinfo *mp_getaddrinfo(const char *hostname, int port, int family, int type);
int mp_connect(const char *hostname, int port, int family, int type);
void mp_disconnect(int sd);

#endif /* MP_NET_H_ */

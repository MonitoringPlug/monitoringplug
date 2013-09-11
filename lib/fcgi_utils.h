/***
 * Monitoring Plugin - fcgi_utils.h
 **
 *
 * Copyright (C) 2013 Marius Rieder <marius.rieder@durchmesser.ch>
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

#ifndef _FCGI_UTILS_H_
#define _FCGI_UTILS_H_

#include "config.h"
#include <fcgiapp.h>

/**
 * Connect to a fcgi server.
 * \return Return a pointer to the socket.
 */
int mp_fcgi_connect(char *socket);

/**
 * Send a Key/Value pair to a FCGX_Stream.
 * \para[in] paramsStream The stream to write to.
 * \para[in] key The key to send.
 * \para[in] value The coresponding value.
 */
void mp_fcgi_putkv(FCGX_Stream *paramsStream, const char *key, const char *value);

/**
 * Write a FCGI Frame to the server.
 * \para[in] sock Socket of the fcgi connection.
 * \para[in] requestid Requestid to identify the request.
 * \para[in] type Frame type to transmit.
 * \para[in] content Frame content.
 * \para[in] contentLength Length of the conntent.
 * \return Length of the written data.
 */
int mp_fcgi_write(int sock, int requestid, unsigned char type,
        const char *content, int contentLength);

/**
 * Read a FCGI Frame from the server.
 * \para[in] sock Socket of the fcgi connection.
 * \para[out] content Content read from the server.
 * \para[out] contentLength Length of the content read.
 * \return The Frame type read.
 */
int mp_fcgi_read(int sock, char **content, int *contentLength);

#endif /* _FCGI_UTILS_H_ */

/* vim: set ts=4 sw=4 et syn=c : */

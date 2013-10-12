/***
 * Monitoring Plugin - fcgi_utils.c
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
#include "fcgi_utils.h"

#include <string.h>
#include <stdio.h>
#include <fastcgi.h>
#include <fcgios.h>
#include <fcgiapp.h>

int mp_fcgi_connect(char *socket) {
    int sock = -1;

    sock = OS_FcgiConnect(socket);

    if(sock < 0) {
        critical("FcgiConnect to '%s' failed.", socket);
    }

    return sock;
}

void mp_fcgi_putkv(FCGX_Stream *paramsStream, const char *key, const char *value) {
    int  ret;
    char header[8];
    char *ptr = &header[0];
    int keyLength = strlen(key);
    int valueLength = strlen(value);

    if (keyLength < 0x80) {
        *ptr++ = (unsigned char) keyLength;
    } else {
        *ptr++ = (unsigned char) ((keyLength >> 24) | 0x80);
        *ptr++ = (unsigned char)(unsigned char) (keyLength >> 16);
        *ptr++ = (unsigned char)(unsigned char) (keyLength >> 8);
        *ptr++ = (unsigned char)(unsigned char) keyLength;
    }

    if (valueLength < 0x80) {
        *ptr++ = (unsigned char) valueLength;
    } else {
        *ptr++ = (unsigned char) ((valueLength >> 24) | 0x80);
        *ptr++ = (unsigned char)(unsigned char) (valueLength >> 16);
        *ptr++ = (unsigned char)(unsigned char) (valueLength >> 8);
        *ptr++ = (unsigned char)(unsigned char) valueLength;
    }

    /* Put header */
    ret = FCGX_PutStr(&header[0], ptr-&header[0], paramsStream);
    if (ret != ptr-&header[0]) {
        if (mp_verbose > 0)
            printf("Put header for '%s' => '%s'\n", key, value);
        critical("FCGX_PutStr failed.");
    }

    /* Put key */
    ret = FCGX_PutStr(key, keyLength, paramsStream);
    if (ret !=  keyLength) {
        if (mp_verbose > 0)
            printf("Put key for '%s' => '%s'\n", key, value);
        critical("FCGX_PutStr failed.");
    }

    /* Put value */
    ret = FCGX_PutStr(value, valueLength, paramsStream);
    if (ret !=  valueLength) {
        if (mp_verbose > 0)
            printf("Put value for '%s' => '%s'\n", key, value);
        critical("FCGX_PutStr failed.");
    }
}

int mp_fcgi_write(int socket, int requestId, unsigned char type,
        const char *content, int contentLength) {
    FCGI_Header header;
    int count = -1;

    header.version          = FCGI_VERSION_1;
    header.type             = (unsigned char) type;
    header.requestIdB1      = (unsigned char) ((requestId     >> 8) & 0xff);
    header.requestIdB0      = (unsigned char) ((requestId         ) & 0xff);
    header.contentLengthB1  = (unsigned char) ((contentLength >> 8) & 0xff);
    header.contentLengthB0  = (unsigned char) ((contentLength     ) & 0xff);
    header.paddingLength    = 0;
    header.reserved         = 0;

    /* Write fcgi header */
    count = OS_Write(socket, (char *)&header, sizeof(header));
    if (count != sizeof(header))
        unknown("libfcgi OS_Write failed writing header.");

    /* Write content */
    count = OS_Write(socket, (char *)content, contentLength);
    if (count != contentLength)
         unknown("libfcgi OS_Write failed writing content.");

    return sizeof(header) + contentLength;
}

int mp_fcgi_read(int socket, char **content, int *contentLength) {
    FCGI_Header *header;
    int count = -1;

    header = mp_malloc(sizeof(header));

    count = OS_Read(socket, (char *)header, sizeof(header));
    if (count != 8)
        critical("mp_fcgi_read failed reading fcgi header.");

    *contentLength = (header->contentLengthB1 << 8);
    *contentLength += header->contentLengthB0;

    if (*contentLength > 0) {
        *content = mp_malloc(*contentLength+header->paddingLength+1);
        count = OS_Read(socket, (char *)*content, *contentLength+header->paddingLength);
        if (count != *contentLength+header->paddingLength)
            critical("mp_fcgi_read failed reading fcgi content.");
    }

    if (mp_verbose > 3) {
        printf("-[ FCGI IN ]--------------------------------\n");
        printf("Version: %d\n", header->version);
        printf("Type:    %d\n", header->type);
        printf("Length:  %d\n", *contentLength);
        printf("Padding: %d\n", header->paddingLength);
        if (*contentLength > 0) {
            printf("%s\n", *content);
        }
        printf("--------------------------------------------\n");
    }

    // Save type for return
    count = header->type;
    free(header);

    return count;
}

/* vim: set ts=4 sw=4 et syn=c : */

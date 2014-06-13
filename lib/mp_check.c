/***
 * Monitoring Plugin - mp_check.c
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

#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>

/* mp check functions */

int is_integer(const char *number) {
    long int n;
    if (!number)
        return FALSE;
    if (strspn(number, "-0123456789 ") != strlen(number))
        return FALSE;

    errno = 0;
    n = strtol(number, NULL, 10);

    if (errno != ERANGE && n >= INT_MIN && n <= INT_MAX)
        return TRUE;
    else
        return FALSE;
}

int is_hostname(const char *address) {
    char *a, *addr, *part;

    a = addr = mp_strdup(address);

    while ((part = strsep(&addr, "."))) {
        size_t len;

        len = strlen(part);

        if (len == 0) {
            free(a);
            return FALSE;
        }

        if(strspn (part, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789-_") != len) {
            free(a);
            return FALSE;
        }
    }
    free(a);
    return TRUE;
}

int is_hostaddr(const char *address) {

    struct addrinfo hints;
    struct addrinfo *res;

    memset (&hints, 0, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_NUMERICHOST;

    if (getaddrinfo(address, NULL, &hints, &res) == 0) {
        freeaddrinfo(res);
        return TRUE;
    }
#ifdef USE_IPV6
    hints.ai_family = AF_INET6;
    if (getaddrinfo (address, NULL, &hints, &res) == 0) {
        freeaddrinfo(res);
        return TRUE;
    }

#endif /* USE_IPV6*/
    return FALSE;
}

int is_url(const char *url) {
    char *buf, *buf2;
    char *ptr, *remain;
    ptr = remain = mp_strdup(url);

    /* Schema */
    buf = strsep(&remain, ":");
    if (!remain || !isalpha(*buf)) {
        free(ptr);
        return FALSE;
    }
    while(*(++buf)) {
        if (isalnum(*buf) || *buf == '+' || *buf == '-' || *buf == '.')
            continue;
        free(ptr);
        return FALSE;
    }
    if (*(remain) != '/' || *(remain+1) != '/') {
        free(ptr);
        return FALSE;
    }
    remain+=2;

    /* Authority */
    buf = strsep(&remain, "/?#");

    if (*buf != '\0') {

        /* Authority - Userinfo */
        if (strstr(buf, "@")) {
            buf2 = strsep(&buf, "@");

            do {
                if (isalnum(*buf2) || *buf2 == '+' || *buf2 == '-' || *buf2 == '.' ||
                        *buf2 == '~' || *buf2 == '!' || *buf2 == '$' || *buf2 == '&' ||
                        *buf2 == '\'' || *buf2 == '(' || *buf2 == ')' || *buf2 == '*' ||
                        *buf2 == ',' || *buf2 == ';' || *buf2 == '=' || *buf2 == ':')
                    continue;
                if (*buf2 == '%' && isxdigit(*(buf2+1)) && isxdigit(*(buf2+2))) {
                    buf2+=2;
                    continue;
                }
                free(ptr);
                return FALSE;
            } while(*(++buf2));
        }

        /* Authority - Host */
        if (*buf == '[') {
            buf++;
            buf2 = strsep(&buf, "]");
            if(*buf2 == ':')
                buf2++;
            if (!buf) {
                free(ptr);
                return FALSE;
            }
            do {
                if (isxdigit(*buf2) || *buf2 == ':')
                    continue;
                free(ptr);
                return FALSE;
            } while(*(++buf2));
        } else if (isdigit(*buf)) {
            buf2 = strsep(&buf, ":");
            if (!is_hostaddr(buf2)) {
                free(ptr);
                return FALSE;
            }
        } else {
            buf2 = strsep(&buf, ":");
            if (!is_hostname(buf2)) {
                free(ptr);
                return FALSE;
            }
        }

        /* Authority - Port */
        if (buf && *buf != '\0') {
            do {
                if (isdigit(*buf))
                    continue;
                free(ptr);
                return FALSE;
            } while(*(++buf));
        }

    }

    if (!remain || *remain == '\0') {
        free(ptr);
        return TRUE;
    }

    /* Path */
    buf = strsep(&remain, "?#");
    while(1) {
        if (*buf == '\0')
            break;
        if (isalnum(*buf) || *buf == '+' || *buf == '-' || *buf == '.' ||
                *buf == '~' || *buf == '!' || *buf == '$' || *buf == '&' ||
                *buf == '\'' || *buf == '(' || *buf == ')' || *buf == '*' ||
                *buf == ',' || *buf == ';' || *buf == '=' || *buf == ':' ||
                *buf == '@' || *buf == '/') {
            buf++;
            continue;
        }
        if (*buf == '%' && isxdigit(*(buf+1)) && isxdigit(*(buf+2))) {
            buf+=3;
            continue;
        }
        free(ptr);
        return FALSE;
    }

    if (!remain || *remain == '\0') {
        free(ptr);
        return TRUE;
    }

    /* Query & Fragment */
    buf = remain;
    while(1) {
        if (*buf == '\0')
            break;
        if (isalnum(*buf) || *buf == '+' || *buf == '-' || *buf == '.' ||
                *buf == '~' || *buf == '!' || *buf == '$' || *buf == '&' ||
                *buf == '\'' || *buf == '(' || *buf == ')' || *buf == '*' ||
                *buf == ',' || *buf == ';' || *buf == '=' || *buf == ':' ||
                *buf == '@' || *buf == '/' || *buf == '?' || *buf == '#') {
            buf++;
            continue;
        }
        if (*buf == '%' && isxdigit(*(buf+1)) && isxdigit(*(buf+2))) {
            buf+=3;
            continue;
        }
        free(ptr);
        return FALSE;
    }

    free(ptr);
    return TRUE;
}

int is_url_scheme(const char *url, const char *schema) {
    if (strncmp(url, schema, strlen(schema)) != 0)
        return FALSE;
    if (url[strlen(schema)] != ':')
        return FALSE;
    return TRUE;
}

/* vim: set ts=4 sw=4 et syn=c : */

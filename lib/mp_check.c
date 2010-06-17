/**
 * Monitoring Plugin - mp_check.h
 **
 *
 * Copyright (C) 2010 Marius Rieder <marius.rieder@durchmesser.ch>
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
 */

#include "mp_common.h"

#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

/* mp check functions */

int is_integer(const char *number) {
    long int n;
    if (!number || (strspn (number, "-0123456789 ") != strlen (number)))
        return FALSE;

    n = strtol(number, NULL, 10);

    if (errno != ERANGE && n >= INT_MIN && n <= INT_MAX)
        return TRUE;
    else
        return FALSE;
}

int is_hostname(const char *address) {
    char *addr, *part;
    
    addr = strdup(address);
    
    while (part = strsep(&addr, ".")) {
        size_t len;
        
        len = strlen(part);
        
        if (len == 0)
            return FALSE;
        
        if(strspn (part, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789-_") != len)
            return FALSE;
    }
    return TRUE;
}

int is_hostaddr(const char *address) {
    
    struct addrinfo hints;
    struct addrinfo *res;

    memset (&hints, 0, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_NUMERICHOST;
    
    if (getaddrinfo(address, NULL, &hints, &res) == 0)
        return TRUE;
#ifdef USE_IPV6
    hints.ai_family = AF_INET6;
    if (getaddrinfo (address, NULL, &hints, &res) == 0)
        return TRUE;
#endif /* USE_IPV6*/
    return FALSE;
}

/* EOF */
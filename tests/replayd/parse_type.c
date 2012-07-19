/***
 * Monitoring Plugin - parse_type.c
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

#include <string.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "replayd.h"

type_atoi type_map[] = {
    {"INTEGER",     ASN_INTEGER},
    {"STRING",      ASN_OCTET_STR},
    {"Hex-STRING",  ASN_OCTET_STR},
    {"\"\"",        ASN_OCTET_STR},
    {"OID",         ASN_OBJECT_ID},
    {"Timeticks",   ASN_TIMETICKS},
    {"Gauge32",     ASN_GAUGE},
    {"Counter32",   ASN_COUNTER},
    {"IpAddress",   ASN_IPADDRESS},
    {"NULL",        ASN_NULL},
    {NULL,          0},                 // Array end marker
};

int ra_type_atoi(const char *type) {
    type_atoi *map;

    for(map = type_map; map->a; map++) {
        if (strcmp(type, map->a) == 0)
            return map->i;
    }
    return 0;
}

/* vim: set ts=4 sw=4 et syn=c : */

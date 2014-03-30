/***
 * Monitoring Plugin - ldap_utils.c
 **
 *
 * Copyright (C) 2014 Marius Rieder <marius.rieder@durchmesser.ch>
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
#include "ldap_utils.h"

#include <ldap.h>

#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char *mp_ldap_uri = NULL;
char *mp_ldap_binddn = NULL;
char *mp_ldap_pass = NULL;
char *mp_ldap_basedn = NULL;

void getopt_ldap(int c) {
    switch ( c ) {
        case 'H':
            mp_ldap_uri = optarg;
            break;
        case 'D':
            mp_ldap_binddn = optarg;
            break;
        case 'W':
            mp_ldap_pass = optarg;
            break;
        case 'b':
            mp_ldap_basedn = optarg;
            break;
    }
}

void print_help_ldap(void) {
    printf(" -H, --uri=URI\n");
    printf("      The LDAP Server Hostname or URI to connect to.\n");
    printf(" -D, --binddn=BINDDN\n");
    printf("      The LDAP Bind DN.\n");
    printf(" -W, --password=PASSWORD\n");
    printf("      The password to use when binding to the server.\n");
    printf(" -b, --basedn=BASEDB\n");
    printf("      The Base DB.\n");
}

void print_revision_ldap(void) {
    printf(" OpenLDAP v%d.%d.%d\n", LDAP_VENDOR_VERSION_MAJOR,
            LDAP_VENDOR_VERSION_MINOR, LDAP_VENDOR_VERSION_PATCH);
}

/* vim: set ts=4 sw=4 et syn=c : */

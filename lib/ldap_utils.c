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
#include <errno.h>

char *mp_ldap_uri = NULL;
int  mp_ldap_version = LDAP_VERSION3;
int  mp_ldap_auth = LDAP_AUTH_SIMPLE;
char *mp_ldap_binddn = NULL;
char *mp_ldap_pass = NULL;
char *mp_ldap_basedn = NULL;

LDAP *mp_ldap_init(char *uri) {
    LDAP *ld = NULL;
    int ldap_ret;

    if (uri == NULL)
        uri = mp_ldap_uri;

    if (ldap_initialize(&ld, uri) != 0) {
        critical("ldap_initi(%s) failed: %s", uri, strerror(errno));
    }

    ldap_ret = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &mp_ldap_version);
    if (ldap_ret != LDAP_OPT_SUCCESS) {
        critical("Set Protocol Version to %d failed: %s", mp_ldap_version,
                ldap_err2string(ldap_ret));
    }

    struct berval cred;
    cred.bv_val = (char *) mp_ldap_pass;
    if (mp_ldap_pass) {
        cred.bv_len = strlen(mp_ldap_pass);
    } else {
        cred.bv_len = 0;
    }
    ldap_ret = ldap_sasl_bind_s(ld, mp_ldap_binddn, LDAP_SASL_SIMPLE, &cred, NULL, NULL, NULL);
    if (ldap_ret != LDAP_OPT_SUCCESS) {
        critical("LDAP Bind failed: %s: %s", uri, ldap_err2string(ldap_ret));
    }

    return ld;
}

LDAPMessage *mp_ldap_search(LDAP *ld, const char *base, int scope, const char *filter,
                char **attrs) {
    LDAPMessage *msg;
    int ldap_ret;
    char *attr;
    BerElement *ber;
    struct berval **vals;
    int i;

    ldap_ret = ldap_search_ext_s(ld, base, scope, filter, attrs, 0, NULL, NULL, NULL, 0, &msg);
    if (ldap_ret != LDAP_OPT_SUCCESS) {
        critical("LDAP Search failed: %s", ldap_err2string(ldap_ret));
    }

    if (mp_verbose > 2) {
        printf ("LDAP-Search %s %s\n", base, filter);
        LDAPMessage *entry = NULL;
        for (entry = ldap_first_entry(ld, msg); entry != NULL;
                entry = ldap_next_entry(ld, entry)) {
            printf("dn: %s\n", ldap_get_dn(ld, entry));

            for (attr = ldap_first_attribute(ld, entry, &ber); attr != NULL;
                    attr = ldap_next_attribute(ld, entry, ber)) {
                vals = ldap_get_values_len(ld, entry, attr);
                for (i=0; i < ldap_count_values_len(vals); i++) {
                    printf("%s: %s\n", attr, vals[i]->bv_val);
                }
                ldap_value_free_len(vals);
            }
            printf("\n");
        }
    }

    return msg;
}

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

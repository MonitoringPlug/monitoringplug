/***
 * Monitoring Plugin - snmp_utils.h
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

#ifndef _SNMP_UTILS_H_
#define _SNMP_UTILS_H_

#include "config.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

extern char *mp_snmp_community;
extern int mp_snmp_version;
extern int mp_snmp_seclevel;
extern char *mp_snmp_secname;
extern char *mp_snmp_context;
extern char *mp_snmp_authpass;
extern int mp_snmp_autoproto;
extern char *mp_snmp_privpass;


#define SNMPOPTSTRING "C:S:L:U:K:A:a:X:"
#define SNMPLONGOPTS {"community", required_argument, NULL, (int)'C'}, \
                     {"snmp", required_argument, NULL, (int)'S'}, \
                     {"seclevel", required_argument, NULL, (int)'L'}, \
                     {"secname", required_argument, NULL, (int)'U'}, \
                     {"context", required_argument, NULL, (int)'K'}, \
                     {"authpass", required_argument, NULL, (int)'A'}, \
                     {"authproto", required_argument, NULL, (int)'a'}, \
                     {"privpass", required_argument, NULL, (int)'X'}


struct snmp_query_cmd {
    /* OID name to query */
    oid oid[MAX_OID_LEN];
    /* OID lenght */
    size_t len;
    /* OID return type */
    u_char type;
    /* Pointer to store value in */
    void **target;
};


/**
 * SNMP Version prity print.
 */
enum {
    SNMPv1 = 1,           /**< 1 - SNMP Version 1 */
    SNMPv2c = 2,          /**< 2 - SNMP Version 2c */
    SNMPv3 = 3,           /**< 3 - SNMP Version 3 */
};

/**
 * SNMP Security Levels prity print
 */
enum {
    noAuthNoPriv,
    authNoPriv,
    authPriv,
};

/**
 * SNMP Auth Protocol prity print
 */
enum {
    MD5,
    SHA1,
};

void getopt_snmp(int c);
void print_help_snmp(void);
void print_revision_snmp(void);
netsnmp_session *mp_snmp_init(void);
void snmp_query(netsnmp_session *ss, const struct snmp_query_cmd *querycmd);

#endif /* _SNMP_UTILS_H_ */

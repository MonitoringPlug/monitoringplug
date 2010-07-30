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
 * $Id$
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
extern oid *mp_snmp_authproto;
extern char *mp_snmp_privpass;


#define SNMP_OPTSTR "C:S:L:U:K:A:a:X:"
#define SNMP_LONGOPTS {"community", required_argument, NULL, (int)'C'}, \
                      {"snmp", required_argument, NULL, (int)'S'}, \
                      {"secname", required_argument, NULL, (int)'U'}, \
                      {"context", required_argument, NULL, (int)'K'}, \
                      {"authpass", required_argument, NULL, (int)'A'}, \
                      {"authproto", required_argument, NULL, (int)'a'}, \
                      {"privpass", required_argument, NULL, (int)'X'}


struct mp_snmp_query_cmd {
    /** OID name to query */
    oid oid[MAX_OID_LEN];
    /** OID lenght */
    size_t len;
    /** OID return type */
    u_char type;
    /** Pointer to store value in */
    void **target;
};

struct mp_snmp_table {
    /** Table row count */
    int row;
    /** Table column count */
    int col;
    /** Table data count */
    netsnmp_variable_list **var;
};

netsnmp_session *mp_snmp_init(void);
inline void mp_snmp_deinit(void) __attribute__((always_inline));
void snmp_query(netsnmp_session *ss, const struct mp_snmp_query_cmd *querycmd);
void snmp_table_query(netsnmp_session *ss, const struct mp_snmp_query_cmd *querycmd);
inline netsnmp_variable_list *mp_snmp_table_get(const struct mp_snmp_table table, int x, int y) __attribute__((always_inline));

inline void getopt_snmp(int c) __attribute__((always_inline));
inline void print_help_snmp(void) __attribute__((always_inline));
inline void print_revision_snmp(void) __attribute__((always_inline));
#endif /* _SNMP_UTILS_H_ */

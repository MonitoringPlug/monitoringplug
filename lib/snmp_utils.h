/***
 * Monitoring Plugin - snmp_utils.h
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

#ifndef _SNMP_UTILS_H_
#define _SNMP_UTILS_H_

#include "config.h"
#include <getopt.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

/* The global snmp vars. */
/** Holds the community for the snmp connection. */
extern char *mp_snmp_community;
/** Holds the snmp version for the connection. */
extern int mp_snmp_version;
/** Holds the security level for the snmp connection. */
extern int mp_snmp_seclevel;
/** Holds the security name for the snmp connection. */
extern char *mp_snmp_secname;
/** Holds the context for the snmp connection. */
extern char *mp_snmp_context;
/** Holds the authentication password for the snmp connection. */
extern char *mp_snmp_authpass;
/** Holds the authentication protocol for the snmp connection. */
extern oid *mp_snmp_authproto;
/** Holds the privacy password for the snmp connection. */
extern char *mp_snmp_privpass;
/** Holds the query timeout. */
extern int mp_snmp_timeout;
/** Holds the query retransmit count. */
extern int mp_snmp_retries;
/** Maps ifOperStatus to text. */
extern char *ifOperStatusText[];

/** Wrapper to simplify 'oid[], len' notation */
#define MP_OID(...) (oid[]){__VA_ARGS__}, (sizeof((oid[]){__VA_ARGS__})/sizeof(oid))

/** SNMP specific short option string. */
#define SNMP_OPTSTR "C:S:L:U:K:A:a:X:T:R:"
/** SNMP specific longopt struct. */
#define SNMP_LONGOPTS {"community", required_argument, NULL, (int)'C'}, \
                      {"snmp", required_argument, NULL, (int)'S'}, \
                      {"secname", required_argument, NULL, (int)'U'}, \
                      {"context", required_argument, NULL, (int)'K'}, \
                      {"authpass", required_argument, NULL, (int)'A'}, \
                      {"authproto", required_argument, NULL, (int)'a'}, \
                      {"privpass", required_argument, NULL, (int)'X'}, \
                      {"snmptimeout", required_argument, NULL, (int)'T'}, \
                      {"snmpretries", required_argument, NULL, (int)'R'}

/**
 * SNMP Query struct
 */
typedef struct mp_snmp_query_cmd_s {
    /** OID name to query */
    oid oid[MAX_OID_LEN];
    /** OID lenght */
    size_t oid_len;
    /** OID return type */
    u_char type;
    /** Pointer to store value in */
    void **target;
    /** size of target */
    size_t target_len;
} mp_snmp_query_cmd;

/**
 * SNMP Table Query struct
 */
struct mp_snmp_table {
    /** Table row count */
    int row;
    /** Table column count */
    int col;
    /** Table data count */
    netsnmp_variable_list **var;
};

/**
 * SNMP subtree struct
 */
typedef struct {
    /** subtree size */
    size_t size;
    /** subtree data */
    netsnmp_variable_list **vars;
} mp_snmp_subtree;

/**
 * Init the Net-SNMP library and return a new session.
 * \return netsnmp_session created
 */
netsnmp_session *mp_snmp_init(void);

/**
 * Cleanup the Net-SNMP library.
 */
void mp_snmp_deinit(void);

/**
 * Run all querys in querycmd and save result to pointer in querycmd struct.
 * \param[in] ss Session to use.
 * \param[in|out] querycmd Query commands
 * \return return a status value like snmp.
 */
int mp_snmp_query(netsnmp_session *ss, const mp_snmp_query_cmd *querycmd);

/**
 * Run table query for querycmd and save mp_snmp_table to pointer in querycmd.
 * \param[in] ss Session to use.
 * \param[in|out] querycmd Table query command
 * \return return a status value like snmp.
 */
int mp_snmp_table_query(netsnmp_session *ss, const mp_snmp_query_cmd *querycmd, int cols);


/**
 * Get a netsnmp_variable_list out of a mp_snmp_table.
 * \param[in] table Table to fetch value from.
 * \param[in] x X coordinate of value.
 * \param[in] y Y coordinate of value.
 */
netsnmp_variable_list *mp_snmp_table_get(const struct mp_snmp_table table, int x, int y);


typedef struct {
    /** OID name */
    const char *oid;
    /** expected OID type */
    u_char type;
    /** pointer to store OID value in */
    void **target;
    /** size of target */
    size_t target_len;
} mp_snmp_value;

/**
 * Fetch all OIDs given in values and save results to target pointer
 * given in values.
 *
 * \param[in] ss snmp session to use.
 * \param[in|out] values the OIDs to be fetched and where store the results
 * \return returns STAT_SUCESS on success, other value otherwise
 */
int mp_snmp_values_fetch1(netsnmp_session *ss,
                          const mp_snmp_query_cmd *values);


/**
 * Fetch all OIDs given in values and save results to target pointer
 * given in values.
 *
 * \param[in] ss snmp session to use.
 * \param[in|out] values the OIDs to be fetched and where store the results
 * \return returns STAT_SUCESS on success, other value otherwise
 */
int mp_snmp_values_fetch2(netsnmp_session *ss,
                          const mp_snmp_value *values);


int mp_snmp_values_fetch3(netsnmp_session *ss,
                          const mp_snmp_value *values, ...);

/**
 * Fetch a subtree of OIDs starting on subtree_oid and save results to subtree.
 *
 * \param[in] ss snmp session to use.
 * \param[in] subtree_oid start OID
 * \param[in] subtree_oid_len the size of subtree_oid
 * \param[in|out] subtree store fetched OIDs result
 * \return returns STAT_SUCESS on success, other value otherwise
 */
int mp_snmp_subtree_fetch1(netsnmp_session *ss,
                           const oid *subtree_oid,
                           const size_t subtree_oid_len,
                           mp_snmp_subtree *subtree);


/**
 * Fetch a subtree of OIDs starting on subtree_oid and save results to subtree.
 *
 * \param[in] ss snmp session to use.
 * \param[in] subtree_oid start OID (not included in result)
 * \param[in|out] subtree store fetched OIDs result
 * \return returns STAT_SUCESS on success, other value otherwise
 */
int mp_snmp_subtree_fetch2(netsnmp_session *ss,
                           const char *subtree_oid,
                           mp_snmp_subtree *subtree);


/**
 * Get a value from a fetched OID subtree. Use the idx parameter to specify,
 * the row of an OID table.
 *
 * \param[in] subtree the OID subtree
 * \param[in] value_oid the OID to get the value for
 * \param[in] idx the instance of OID (aka row in OID table); 0-based
 * \param[in] type the expected type of the OID value
 * \param[in|out] target a pointer, where to store the OID value
 * \return returns 1, if value was found, 0 otherwise
 */
int mp_snmp_subtree_get_value1(const mp_snmp_subtree *subtree,
                               const oid *value_oid,
                               const size_t value_oid_len,
                               const size_t idx,
                               const u_char type,
                               void **target,
                               const size_t target_len);

/**
 * Get a value from a fetched OID subtree. Use the idx parameter to specify,
 * the row of an OID table.
 *
 * \param[in] subtree the OID subtree
 * \param[in] value_oid the OID to get the value for
 * \param[in] idx the instance of OID (aka row in OID table); 0-based
 * \param[in] type the expected type of the OID value
 * \param[in|out] target a pointer, where to store the OID value
 * \return returns 1, if value was found, 0 otherwise
 */
int mp_snmp_subtree_get_value2(const mp_snmp_subtree *subtree,
                               const char* value_oid,
                               const size_t idx,
                               const u_char type,
                               void **target,
                               const size_t target_len);


/**
 * Get multiple value from a fetched OID subtree. Use the idx parameter to
 * specify, the row of an OID table.
 *
 * \param[in] subtree the OID subtree
 * \param[in] idx the instance of OID (aka row in OID table); 0-based
 * \param[in|out] values the list of OIDs to get and where to store results
 * \return return number of variables found, or 0 of none where found
 */
int mp_snmp_subtree_get_values(const mp_snmp_subtree *subtree,
                               const size_t idx,
                               const mp_snmp_value *values);


/**
 * Free memory used by a subtree.
 *
 * \param[in|out] subtree subtree to free
 */
void mp_snmp_subtree_free(mp_snmp_subtree *subtree);


/**
 * Handle SNMP related command line options.
 * \param[in] c Command line option to handle.
 */
void getopt_snmp(int c);

/**

 * Print the help for the SNMP related command line options.
 */
void print_help_snmp(void);

/**
 * Print the Net-SNMP library revision.
 */
void print_revision_snmp(void);

#endif /* _SNMP_UTILS_H_ */

/* vim: set ts=4 sw=4 et syn=c : */

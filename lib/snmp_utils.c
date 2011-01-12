/***
 * Monitoring Plugin - snmp_utils.c
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

#include "mp_common.h"
#include "snmp_utils.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char *mp_snmp_community;
int mp_snmp_version = SNMP_VERSION_2c;
int mp_snmp_seclevel;
char *mp_snmp_secname;
char *mp_snmp_context;
char *mp_snmp_authpass;
oid *mp_snmp_authproto;
char *mp_snmp_privpass;

extern char* hostname;

netsnmp_session *mp_snmp_init(void) {

    netsnmp_session session, *ss;

    init_snmp(progname);

    snmp_sess_init( &session );

    if (mp_snmp_community == NULL)
        mp_snmp_community = strdup("public");

    session.peername = hostname;

    switch(mp_snmp_version) {
        case SNMP_VERSION_1:
            session.version = SNMP_VERSION_1;
            session.community = (u_char *)strdup(mp_snmp_community);
            session.community_len = strlen((char *)session.community);
            break;
        case SNMP_VERSION_2c:
            session.version = SNMP_VERSION_2c;
            session.community = (u_char *)mp_snmp_community;
            session.community_len = strlen((char *)session.community);
            break;
        case SNMP_VERSION_3:
            session.version = SNMP_VERSION_3;

            session.securityName = strdup(mp_snmp_secname);
            session.securityNameLen = strlen(session.securityName);

            /* set the security level */
            session.securityLevel = mp_snmp_seclevel;
            session.contextName = strdup(mp_snmp_context);

            session.contextNameLen = strlen(session.contextName);

            /* set the authentication method */
            session.securityAuthProto = mp_snmp_authproto;
            session.securityAuthProtoLen = 10;
            session.securityAuthKeyLen = USM_AUTH_KU_LEN;

            int status;
            status = generate_Ku(session.securityAuthProto,
                    session.securityAuthProtoLen,
                    (u_char *) mp_snmp_authpass, strlen(mp_snmp_authpass),
                    session.securityAuthKey,
                    &session.securityAuthKeyLen);
            if (status != SNMPERR_SUCCESS) {
                snmp_perror(progname);
                snmp_log(LOG_ERR,
                        "Error generating Ku from authentication pass phrase. \n%s\n",snmp_api_errstring(status));
                exit(1);
            }

            break;
    }

    netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID,
                           NETSNMP_DS_LIB_DONT_PERSIST_STATE, 1);

    SOCK_STARTUP;
    ss = snmp_open(&session);

    if (!ss) {
      snmp_sess_perror("ack", &session);
      SOCK_CLEANUP;
      exit(1);
    }

    return ss;

}

void mp_snmp_deinit(void) {
    snmp_shutdown(progname);
    SOCK_CLEANUP;
}


int mp_snmp_query(netsnmp_session *ss, const struct mp_snmp_query_cmd *querycmd) {

    netsnmp_pdu *pdu;
    netsnmp_pdu *response;
    netsnmp_variable_list *vars;
    int status;
    const struct mp_snmp_query_cmd *p;

    pdu = snmp_pdu_create(SNMP_MSG_GET);

    for(p = querycmd; p->len; p++) {
        snmp_add_null_var(pdu, p->oid, p->len);
    }

    /* Send the SNMP Query */
    status = snmp_synch_response(ss, pdu, &response);

    /* Process the response. */
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
        for(vars = response->variables; vars; vars = vars->next_variable) {
            for(p = querycmd; p->len; p++) {
                if (snmp_oid_compare(vars->name, vars->name_length, p->oid, p->len) == 0) {
                    if (mp_verbose > 1)
                        print_variable(vars->name, vars->name_length, vars);

                    if (vars->type != p->type) {
                        *p->target = NULL;
                        continue;
                    }
                    switch(vars->type) {
                        case ASN_INTEGER:
                            *(p->target) = (void *)(*vars->val.integer);
                            break;
                        case ASN_OCTET_STR: {
                            char *t = (char *)malloc(1 + vars->val_len);
                            memcpy(t, vars->val.string, vars->val_len);
                            t[vars->val_len] = '\0';

                            *p->target = t;}
                            break;
                    }
                }
            }
        }
    } else {
        /* FAILURE: print what went wrong! */

        if (status == STAT_SUCCESS) {
            fprintf(stderr, "Error in packet\nReason: %s\n",
                    snmp_errstring(response->errstat));
            status = STAT_ERROR;
        }
        else if (status == STAT_TIMEOUT)
            fprintf(stderr, "Timeout: No response from %s.\n",
                    (*ss).peername);
        else
            snmp_sess_perror(progname, ss);

    }

    if (response)
      snmp_free_pdu(response);

    return status;
}


int mp_snmp_table_query(netsnmp_session *ss, const struct mp_snmp_query_cmd *querycmd) {

    netsnmp_pdu *pdu;
    netsnmp_pdu *response;
    netsnmp_variable_list *vars, *last_var;
    int status;
    int row_idx = 0;
    int col_idx = 0;

    struct mp_snmp_table *table;

    table = (struct mp_snmp_table *)querycmd->target;

    table->row = 0;
    table->col = 1;
    table->var = NULL;

    oid current_oid[MAX_OID_LEN];
    size_t current_len;

    memcpy(current_oid, querycmd->oid, querycmd->len * sizeof(oid));
    current_len = querycmd->len;
    status = STAT_SUCCESS;

    while(status == STAT_SUCCESS) {

        if(ss->version== SNMP_VERSION_1) {
            pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
        } else {
            pdu = snmp_pdu_create(SNMP_MSG_GETBULK);
            pdu->non_repeaters = 0;
            pdu->max_repetitions = 10;
        }

        snmp_add_null_var(pdu, current_oid, current_len);

        status = snmp_synch_response(ss, pdu, &response);

        if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
            if (row_idx == 0) {
                vars = response->variables;
                row_idx = querycmd->len + 1;
                col_idx = querycmd->len;

                if (vars->name_length >= row_idx) {
                    row_idx = vars->name_length - 1;
                    col_idx = vars->name_length - 2;
                }
            }

            for(last_var = vars = response->variables; vars; last_var=vars, vars = vars->next_variable) {
                /* Check for leafing of subtree */
                if (snmp_oid_ncompare(querycmd->oid, querycmd->len, vars->name, vars->name_length, querycmd->len) != 0) {
                    status = -1;
                    break;
                }
                if ((int)vars->name[col_idx] == 1) {
                    if ((int)vars->name[row_idx] > table->row) {
                        table->row = (int)vars->name[row_idx];
                        table->var = realloc(table->var, (table->row*table->col)*sizeof(netsnmp_variable_list*));
                    }
                } else {
                    if ((int)vars->name[row_idx] > table->row)
                        printf("ERROR %d\n", __LINE__);
                }
                if ((int)vars->name[col_idx] > table->col) {
                    table->col = (int)vars->name[col_idx];
                    table->var = realloc(table->var, (table->row*table->col)*sizeof(netsnmp_variable_list*));
                }
                if ((int)vars->name[col_idx] > table->col)
                    table->col = (int)vars->name[col_idx];

                int c = (table->col-1)*table->row+(int)vars->name[row_idx]-1;
                if (mp_verbose > 1)
                    print_variable(vars->name, vars->name_length, vars);

                table->var[c] = malloc(sizeof(netsnmp_variable_list));

                snmp_clone_var(vars, table->var[c]);
            }

            memcpy(current_oid, last_var->name, last_var->name_length * sizeof(oid));
            current_len = last_var->name_length;

        } else {
            /* FAILURE: print what went wrong! */

            if (status == STAT_SUCCESS) {
                fprintf(stderr, "Error in packet\nReason: %s\n",
                        snmp_errstring(response->errstat));
                status = STAT_ERROR;
            }
            else if (status == STAT_TIMEOUT)
                fprintf(stderr, "Timeout: No response from %s.\n",
                        (*ss).peername);
            else
                snmp_sess_perror(progname, ss);
        }
        if (response)
          snmp_free_pdu(response);
        if (mp_verbose > 1)
            printf("----------\n");
    }

    return status;
}

netsnmp_variable_list *mp_snmp_table_get(const struct mp_snmp_table table, int x, int y) {
    if( x < 0 || y < 0 || x >= table.col || y >= table.row)
        return NULL;
    return table.var[(x-1)*table.row+y];
}

void getopt_snmp(int c) {
    switch ( c ) {
        case 'C':
            mp_snmp_community = optarg;
            break;
        case 'S':
            switch ((int)strtol(optarg, NULL, 10)) {
                case 1:
                    mp_snmp_version = SNMP_VERSION_1;
                    break;
                case 2:
                    mp_snmp_version = SNMP_VERSION_2c;
                    break;
                case 3:
                    mp_snmp_version = SNMP_VERSION_3;
                    break;
                default:
                    usage("Illegal SNMP version '%s'", optarg);
            }
            break;
        case 'L':
            if (strncmp("noAuthNoPriv",optarg,12) == 0)
                mp_snmp_seclevel = SNMP_SEC_LEVEL_NOAUTH;
            else if (strncmp("authNoPriv",optarg,10) == 0)
                mp_snmp_seclevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
            else if (strncmp("authPriv",optarg,8) == 0)
                mp_snmp_seclevel = SNMP_SEC_LEVEL_AUTHPRIV;
            else
                usage("Illegal snmp security level '%s'.", optarg);
            break;
        case 'U':
            mp_snmp_secname = optarg;
            break;
        case 'K':
            mp_snmp_context = optarg;
            break;
        case 'A':
            mp_snmp_authpass = optarg;
            break;
        case 'a':
            if (strncmp("MD5",optarg,3) == 0)
                mp_snmp_authproto = usmHMACMD5AuthProtocol;
            else if (strncmp("SHA1",optarg,4) == 0)
                mp_snmp_authproto = usmHMACSHA1AuthProtocol;
            else
                usage("Illegal snmp auth protocoll '%s'.", optarg);
            break;
        case 'X':
            mp_snmp_privpass = optarg;
            break;
    }
}

void print_help_snmp(void) {
    printf(" -C, --community=COMUNITY\n");
    printf("      SNMP community (defaults to public, used with SNMP v1 and v2c)\n");
    printf(" -S, --snmp=VERSION\n");
    printf("      SNMP version to use. (default to v1)\n");
    printf("      1 for SNMP v1\n");
    printf("      2 for SNMP v2c (use get_bulk for less overhead)\n");
    printf("      3 for SNMP v3 (requires --secname option)\n");
    printf(" -L, --seclevel=SECLEVEL\n");
    printf("      Security Level to use. (noAuthNoPriv, authNoPriv , or authPriv)\n");
    printf(" -U, --secname=SECNAME\n");
    printf("      User name for SNMPv3 context.\n");
    printf(" -K, --context=CONTEXT\n");
    printf("      SNMPv3 context name (default is empty string)\n");
    printf(" -A, --authpass=PASSWORD\n");
    printf("      Authentication password. (Clear text ASCII or localized key in hex\n");
    printf("      with 0x prefix generated by using \"snmpkey\" utility\n");
    printf(" -a, --authproto=PROTOCOL\n");
    printf("      Authentication protocol (MD5 or SHA1)\n");
    printf(" -X, --privpass=PASSWORD\n");
    printf("      Privacy password. (Clear text ASCII or localized key in hex\n");
    printf("      with 0x prefix generated by using \"snmpkey\" utility\n");
}

void print_revision_snmp(void) {
    printf(" libnetsnmp v%s\n", netsnmp_get_version());
}

/* vim: set ts=4 sw=4 et : */

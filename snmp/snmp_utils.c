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
int mp_snmp_version = 2;
int mp_snmp_seclevel;
char *mp_snmp_secname;
char *mp_snmp_context;
char *mp_snmp_authpass;
int mp_snmp_autoproto;
char *mp_snmp_privpass;

extern char* hostname;

void snmp_query(netsnmp_session *ss, const struct snmp_query_cmd *querycmd) {

    netsnmp_pdu *pdu;
    netsnmp_pdu *response;
    netsnmp_variable_list *vars;
    int status;
    const struct snmp_query_cmd *p;

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

                    if (vars->type != p->type)
                        continue;
                    switch(vars->type) {
                        case ASN_INTEGER:
                            *(p->target) = (void *)*vars->val.integer;
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
      /*
       * FAILURE: print what went wrong!
       */

      if (status == STAT_SUCCESS)
        fprintf(stderr, "Error in packet\nReason: %s\n",
                snmp_errstring(response->errstat));
      else if (status == STAT_TIMEOUT)
        fprintf(stderr, "Timeout: No response from %s.\n",
                (*ss).peername);
      else
        snmp_sess_perror(progname, ss);

    }

    if (response)
      snmp_free_pdu(response);
}


/**
 * Initialize the SNMP library
 */
netsnmp_session *mp_snmp_init(void) {

    netsnmp_session session, *ss;

    init_snmp(progname);

    snmp_sess_init( &session );

    if (mp_snmp_community == NULL)
        mp_snmp_community = strdup("public");

    session.peername = hostname;

    switch(mp_snmp_version) {
        case SNMPv1:
            session.version = SNMP_VERSION_1;
            session.community = (u_char *)strdup(mp_snmp_community);
            session.community_len = strlen((char *)session.community);
            break;
        case SNMPv2c:
            session.version = SNMP_VERSION_2c;
            session.community = (u_char *)strdup(mp_snmp_community);
            session.community_len = strlen((char *)session.community);
            break;
        case SNMPv3:
            session.version = SNMP_VERSION_3;
            break;
    }

    SOCK_STARTUP;
    ss = snmp_open(&session);

    if (!ss) {
      snmp_sess_perror("ack", &session);
      SOCK_CLEANUP;
      exit(1);
    }

    return ss;

}

void getopt_snmp(int c) {
    switch ( c ) {
        case 'C':
            mp_snmp_community = optarg;
            break;
        case 'S':
            if (!is_integer(optarg))
                usage("Illegal snmp version number '%s'.", optarg);
            mp_snmp_version = (int)strtol(optarg, NULL, 10);;
            break;
        case 'L':
            if (strncmp("noAuthNoPriv",optarg,12) == 0)
                mp_snmp_seclevel = noAuthNoPriv;
            else if (strncmp("authNoPriv",optarg,10) == 0)
                mp_snmp_seclevel = authNoPriv;
            else if (strncmp("authPriv",optarg,8) == 0)
                mp_snmp_seclevel = authNoPriv;
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
                mp_snmp_autoproto = MD5;
            else if (strncmp("SHA1",optarg,4) == 0)
                mp_snmp_autoproto = SHA1;
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

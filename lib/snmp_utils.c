/***
 * Monitoring Plugin - snmp_utils.c
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
#include "snmp_utils.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

/* Local functions */
static int copy_value(const netsnmp_variable_list *var, const u_char type,
                      size_t target_len, void **target);

char *mp_snmp_community;
int mp_snmp_version = SNMP_VERSION_2c;
int mp_snmp_seclevel;
char *mp_snmp_secname;
char *mp_snmp_context = "";
char *mp_snmp_authpass;
oid *mp_snmp_authproto;
char *mp_snmp_privpass;
int mp_snmp_timeout = 0;
int mp_snmp_retries = 0;

char *ifOperStatusText[] = {"", "up", "down", "testing", "unknown",
       "dormant", "notPresent", "lowerLayerDown", ""};

extern char* hostname;
extern int* port;

netsnmp_session *mp_snmp_init(void) {

    netsnmp_session session, *ss;
    int status;

    init_snmp(progname);

    snmp_sess_init( &session );

    if (mp_snmp_community == NULL)
        mp_snmp_community = strdup("public");

    mp_asprintf(&(session.peername), "%s:%d", hostname, port);

    switch(mp_snmp_version) {
        case SNMP_VERSION_1:
            session.version = SNMP_VERSION_1;
            session.community = (u_char *)mp_snmp_community;
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

    free(session.peername);

    if (mp_snmp_retries > 0)
        ss->retries = mp_snmp_retries;
    if (mp_snmp_timeout > 0)
        ss->timeout = (long)(mp_snmp_timeout * 1000000L);

    return ss;

}

void mp_snmp_deinit(void) {
    snmp_shutdown(progname);
    SOCK_CLEANUP;
}


int mp_snmp_query(netsnmp_session *ss, const mp_snmp_query_cmd *querycmd) {

    netsnmp_pdu *pdu;
    netsnmp_pdu *response;
    netsnmp_variable_list *vars;
    int status;
    const mp_snmp_query_cmd *p;

    pdu = snmp_pdu_create(SNMP_MSG_GET);

    for(p = querycmd; p->oid_len; p++) {
        snmp_add_null_var(pdu, p->oid, p->oid_len);
    }

    /* Send the SNMP Query */
    do {
        status = snmp_synch_response(ss, pdu, &response);

        if (mp_verbose > 3)
            printf("snmp_synch_response() rc=%d\n", status);

        if (!response)
            return STAT_ERROR;

        if (status == STAT_SUCCESS && response->errindex == 0)
            break;

        if (mp_verbose > 3)
            printf(" errindex=%ld\n", response->errindex);

        pdu = snmp_fix_pdu(response, SNMP_MSG_GET);
        snmp_free_pdu(response);
        response = NULL;
    } while (status == STAT_SUCCESS && pdu);

    if (!response)
        return status;

    /* Process the response. */
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
        for(vars = response->variables; vars; vars = vars->next_variable) {
            if (mp_verbose > 1)
                print_variable(vars->name, vars->name_length, vars);
            // Skip non existing vars
            if (vars->type == SNMP_NOSUCHOBJECT ||
                    vars->type == SNMP_NOSUCHINSTANCE ||
                    vars->type == SNMP_ENDOFMIBVIEW)
                continue;
            for(p = querycmd; p->oid_len; p++) {
                if (snmp_oid_compare(vars->name, vars->name_length,
                                     p->oid, p->oid_len) == 0) {
                    copy_value(vars, p->type, p->target_len, p->target);
                    break;
                }
            }
        }
    } else if (status != STAT_SUCCESS) {
        char *err;
        snmp_error(ss, NULL, NULL, &err);

        if (response)
            snmp_free_pdu(response);
        mp_snmp_deinit();

        critical("SNMP Error: %s", err);
    }

    if (response)
      snmp_free_pdu(response);

    return status;
}

int mp_snmp_values_fetch1(netsnmp_session *ss,
                          const mp_snmp_query_cmd *values) {
    netsnmp_pdu *request;
    netsnmp_pdu *response;
    const netsnmp_variable_list *var;
    const mp_snmp_query_cmd *vp;
    int rc;

    /*
     *  set-up request
     */
    request = snmp_pdu_create(SNMP_MSG_GET);
    for (vp = values; vp->oid_len && vp->oid; vp++) {
        snmp_add_null_var(request, vp->oid, vp->oid_len);
    }

    /*
     * commence request
     */
    do {
        rc = snmp_synch_response(ss, request, &response);

        if (mp_verbose > 3)
            printf("snmp_synch_response(): rc=%d\n", rc);

        /* no result, so something went wrong ... */
        if (!response)
            return STAT_ERROR;

        if ((rc == STAT_SUCCESS) && (response->errindex == 0))
            break;

        /* rety with fixed request */
        request = snmp_fix_pdu(response, SNMP_MSG_GET);
        snmp_free_pdu(response);
        response = NULL;
    } while (request && (rc == STAT_SUCCESS));

    /*
     * process results
     */
    if ((rc == STAT_SUCCESS) && response) {
        if (response->errstat == SNMP_ERR_NOERROR) {
            /*
             * extract values from response
             */
            for(var = response->variables; var; var = var->next_variable) {
                for (vp = values; vp->oid_len && vp->oid; vp++) {
                    if (snmp_oid_compare(var->name, var->name_length,
                                         vp->oid, vp->oid_len) == 0) {
                        if (mp_verbose > 1)
                            print_variable(var->name, var->name_length, var);

                        /* copy value, if not erroneous */
                        if ((var->type != SNMP_NOSUCHOBJECT) &&
                            (var->type != SNMP_NOSUCHINSTANCE) &&
                            (var->type != SNMP_ENDOFMIBVIEW))
                            copy_value(var, vp->type,
                                       vp->target_len, vp->target);
                        else
                            if (mp_verbose > 2)
                                printf("OID not available: type=0x%X\n",
                                       var->type);

                        /* short-circuit to next result variable */
                        break;
                    }
                }
            }
        } else if ((ss->version == SNMP_VERSION_1) &&
                   (response->errstat == SNMP_ERR_NOSUCHNAME)) {
            if (mp_verbose > 3)
                printf("SNMP-V1: end of tree\n");
        } else {
            /*
             * some other error occured
             */
            if (mp_verbose > 0)
                printf("SNMP error: respose->errstat = %ld",
                       response->errstat);
            rc = STAT_ERROR;
        }
    } else {
        /*
         * no response (i.e. all vars have been removed by
         * snmp_pid_fixup()) go ahead an assume an error
         */
        rc = STAT_ERROR;
    }

    if (response)
        snmp_free_pdu(response);

    return rc;
}


int mp_snmp_values_fetch2(netsnmp_session *ss,
                          const mp_snmp_value *values) {
    const mp_snmp_value *vp1;
    mp_snmp_query_cmd *vp2;
    size_t count;
    mp_snmp_query_cmd *oid_values = NULL;
    int rc = 0;

    for (count = 0, vp1 = values; vp1->oid; vp1++, count++)
        ;

    oid_values = (mp_snmp_query_cmd *)
        mp_malloc(count * sizeof(mp_snmp_query_cmd));

    for (vp1 = values, vp2 = oid_values; vp1->oid; vp1++, vp2++) {
        vp2->oid_len = MAX_OID_LEN;
        if (!read_objid(vp1->oid, vp2->oid, &vp2->oid_len)) {
            if (mp_verbose > 3)
                printf("Invalid OID: %s\n", vp1->oid);
            goto done;
        }
        vp2->type       = vp1->type;
        vp2->target     = vp1->target;
        vp2->target_len = vp1->target_len;
    }

    rc = mp_snmp_values_fetch1(ss, oid_values);

 done:
    free(oid_values);
    return rc;
}


int mp_snmp_values_fetch3(netsnmp_session *ss,
                          const mp_snmp_value *values, ...) {

    va_list ap;
    const mp_snmp_value *vp1;
    mp_snmp_query_cmd *vp2;
    size_t count;
    char formatted_oid[1024];
    mp_snmp_query_cmd *oid_values = NULL;
    int rc = 0;

    for (count = 0, vp1 = values; vp1->oid; vp1++, count++)
        ;

    oid_values = (mp_snmp_query_cmd *)
        mp_malloc(count * sizeof(mp_snmp_query_cmd));

    for (vp1 = values, vp2 = oid_values; vp1->oid; vp1++, vp2++) {
        va_start(ap, values);
        vsnprintf(formatted_oid, sizeof(formatted_oid), vp1->oid, ap);
        va_end(ap);

        vp2->oid_len = MAX_OID_LEN;
        if (!read_objid(formatted_oid, vp2->oid, &vp2->oid_len)) {
            if (mp_verbose > 3)
                printf("Invalid OID: %s\n", vp1->oid);
            goto done;
        }
        vp2->type       = vp1->type;
        vp2->target     = vp1->target;
        vp2->target_len = vp1->target_len;
    }

    rc = mp_snmp_values_fetch1(ss, oid_values);

 done:
    free(oid_values);
    return rc;
}


int mp_snmp_subtree_query(netsnmp_session *ss,
                           const oid *subtree_oid,
                           const size_t subtree_len,
                           mp_snmp_subtree *subtree) {

    oid last_oid[MAX_OID_LEN];
    size_t last_len;
    netsnmp_pdu *request  = NULL;
    netsnmp_pdu *response = NULL;
    netsnmp_variable_list *var;
    size_t alloc_size = 0;
    int rc;

    /* prepare result */
    memset(subtree, '\0', sizeof(*subtree));
    subtree->vars = NULL;

    memcpy(last_oid, subtree_oid, subtree_len * sizeof(oid));
    last_len = subtree_len;

    for (;;) {
        /*
         * setup request
         */
        if (ss->version == SNMP_VERSION_1) {
            request = snmp_pdu_create(SNMP_MSG_GETNEXT);
        } else {
            request = snmp_pdu_create(SNMP_MSG_GETBULK);
            request->non_repeaters   = 0;
            request->max_repetitions = 16;
        }
        snmp_add_null_var(request, last_oid, last_len);

        /*
         * commence request
         */
        if (response) {
            snmp_free_pdu(response);
            response = NULL;
        }

        if (mp_verbose > 2) {
            char buf[128];

            snprint_objid((char *) &buf, sizeof(buf), last_oid, last_len);
            printf("Fetching next from OID %s\n", buf);
        }

        rc = snmp_synch_response(ss, request, &response);

        if (mp_verbose > 3)
            printf("snmp_synch_response(): rc=%d, errstat=%ld\n",
                   rc, response->errstat);

        if ((rc == STAT_SUCCESS) && response) {
            if (response->errstat == SNMP_ERR_NOERROR) {
                /*
                 * loop over results (may only be one result in case of SNMP v1)
                 */
                for (var = response->variables; var; var = var->next_variable) {

                    /*
                     * check, if OIDs are incresing to prevent infinite
                     * loop with broken SNMP agents
                     */
                    if (snmp_oidtree_compare(var->name, var->name_length,
                                             last_oid, last_len) < 0) {
                        if (response)
                            snmp_free_pdu(response);

                        mp_snmp_deinit();

                        critical("SNMP error: OIDs are not incresing");
                    }

                    /*
                     * terminate, if oid does not belong to subtree anymore
                     */
                    if ((var->type == SNMP_ENDOFMIBVIEW) ||
                        (snmp_oidtree_compare(subtree_oid,
                                              subtree_len,
                                              var->name,
                                              var->name_length) != 0)) {
                        snmp_free_pdu(response);
                        return rc;
                    }

                    if (mp_verbose > 2)
                        print_variable(var->name, var->name_length, var);

                    if (var->type != SNMP_NOSUCHOBJECT ||
                        var->type != SNMP_NOSUCHINSTANCE) {
                        if (alloc_size <= subtree->size) {
                            alloc_size += 16;
                            subtree->vars =
                                mp_realloc(subtree->vars,
                                           alloc_size *
                                           sizeof(netsnmp_variable_list*));
                        }

                        subtree->vars[subtree->size] =
                            mp_malloc(sizeof(netsnmp_variable_list));
                        snmp_clone_var(var, subtree->vars[subtree->size]);
                        subtree->size++;
                    }

                    /*
                     * save last fetched oid
                     */
                    memcpy(last_oid, var->name,
                           var->name_length * sizeof(oid));
                    last_len = var->name_length;
                } /* for */
            } else if ((ss->version == SNMP_VERSION_1) &&
                       (response->errstat == SNMP_ERR_NOSUCHNAME)) {
                if (mp_verbose > 3)
                    printf("SNMP-V1: end of tree\n");
            } else {
                /*
                 * some other error occured
                 */
                if (mp_verbose > 0)
                    printf("SNMP error: respose->errstat = %ld",
                           response->errstat);

                rc = STAT_ERROR;
                //goto done;
                break;
            }
        } else {
            /* no response, assume an error */
            rc = STAT_ERROR;
            break;
        }
    }

    if (response)
        snmp_free_pdu(response);

    return rc;
}


int mp_snmp_subtree_query_string(netsnmp_session *ss, const char *subtree_oid,
                           mp_snmp_subtree *subtree) {
    oid subtree_oid_prefix[MAX_OID_LEN];
    size_t subtree_oid_prefix_len = MAX_OID_LEN;

    if (!read_objid(subtree_oid, subtree_oid_prefix, &subtree_oid_prefix_len)) {
        if (mp_verbose > 3)
            printf("Invalid OID: %s\n", subtree_oid);

        return 0;
    }

    return mp_snmp_subtree_query(ss, subtree_oid_prefix,
                                  subtree_oid_prefix_len, subtree);
}


int mp_snmp_subtree_get_value(const mp_snmp_subtree *subtree,
                               const oid *oid_prefix,
                               const size_t oid_prefix_len,
                               const size_t idx,
                               const u_char type,
                               void **target,
                               const size_t target_len) {
    size_t i, j = 0;

    if (!subtree || (subtree->size < 1))
        return 0;

    for (i = 0; i < subtree->size; i++) {
        if (snmp_oidtree_compare(oid_prefix,
                                 oid_prefix_len,
                                 subtree->vars[i]->name,
                                 subtree->vars[i]->name_length) == 0) {
            if (j == idx) {
                return copy_value(subtree->vars[i], type, target_len, target);
            }
            j++;
        }
    }
    return 0;
}


int mp_snmp_subtree_get_value_string(const mp_snmp_subtree *subtree,
                               const char* value_oid,
                               const size_t idx,
                               const u_char type,
                               void **target,
                               const size_t target_len) {
    oid oid_prefix[MAX_OID_LEN];
    size_t oid_prefix_len = MAX_OID_LEN;

    if (!subtree || (subtree->size < 1))
        return 0;

    if (!read_objid(value_oid, oid_prefix, &oid_prefix_len)) {
        if (mp_verbose > 3)
            printf("Invalid OID: %s\n", value_oid);

        return 0;
    }

    return mp_snmp_subtree_get_value(subtree, oid_prefix, oid_prefix_len,
                                      idx, type, target, target_len);
}


int mp_snmp_subtree_get_values(const mp_snmp_subtree *subtree,
                               const size_t idx,
                               const mp_snmp_value *values) {
    const mp_snmp_value *vp;
    oid oid_prefix[MAX_OID_LEN];
    size_t oid_prefix_len;
    int count = 0;

    if (!subtree || (subtree->size < 1))
        return 0;


    for (vp = values; vp->oid; vp++) {
        oid_prefix_len = MAX_OID_LEN;
        read_objid(vp->oid, oid_prefix, &oid_prefix_len);

        if (mp_snmp_subtree_get_value(subtree, oid_prefix, oid_prefix_len,
                                       idx, vp->type, vp->target,
                                       vp->target_len) > 0) {
            count++;
        }
    }
    return count;
}


void mp_snmp_subtree_free(mp_snmp_subtree *subtree) {
    size_t i;

    if (!subtree || (subtree->size < 1))
        return;

    for (i = 0; i < subtree->size; i++) {
        free(subtree->vars[i]);
    }
    free(subtree->vars);
    subtree->size = 0;
    subtree->vars = NULL;
}


static int copy_value(const netsnmp_variable_list *var, const u_char type,
                      size_t target_len, void **target) {
    if (var->type != type) {
        if (mp_verbose > 1)
            printf("TYPE Mismatch: 0x%X ~ 0x%X\n", var->type, type);
        return 0;
    }
    switch(var->type) {
        case ASN_INTEGER:       // 0x02
            /* FALL-TROUGH */
        case ASN_COUNTER:       // 0x41
            /* FALL-TROUGH */
        case ASN_GAUGE:         // 0x42
            /* FALL-TROUGH */
        case ASN_TIMETICKS:
            if (var->val_len > target_len) {
                if (mp_verbose > 1)
                    printf("TARGET size mismatch: provided storage "
                           "to small (have %zu, need %zu)\n",
                           target_len, var->val_len);
                return 0;
            } else {
                memcpy(target, var->val.integer, var->val_len);
            }
            break;
        case ASN_OCTET_STR:    // 0x04
            {
                if (target_len > 0) {
                    if ((var->val_len + 1) > target_len) {
                        if (mp_verbose > 1)
                            printf("TARGET size mismatch: provided storage "
                                   "to small (have %zu, need %zu)\n",
                                   target_len, var->val_len);
                        return 0;
                    }
                } else {
                    char *buffer;
                    buffer = mp_malloc(var->val_len + 1);
                    *target = (void*) buffer;
                }
                memcpy(*target, var->val.string, var->val_len);
                ((char *) *target)[var->val_len] = '\0';
            }
            break;
        default:
            printf("TYPE Mismatch: unexpected type 0x%X\n", var->type);
            return 0;
    } /* switch */
    return 1;
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
                    break;
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
        case 'T':
            mp_snmp_timeout = (int)strtol(optarg, NULL, 10);
            break;
        case 'R':
            mp_snmp_retries = (int)strtol(optarg, NULL, 10);
            break;
    }
}

void print_help_snmp(void) {
    print_help_host();
    printf(" -C, --community=COMMUNITY\n");
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
    printf("      with 0x prefix generated by using \"snmpkey\" utility.)\n");
    printf(" -a, --authproto=PROTOCOL\n");
    printf("      Authentication protocol (MD5 or SHA1)\n");
    printf(" -X, --privpass=PASSWORD\n");
    printf("      Privacy password. (Clear text ASCII or localized key in hex\n");
    printf("      with 0x prefix generated by using \"snmpkey\" utility.)\n");
    printf(" -T, --snmptimeout=TIMEOUT\n");
    printf("      SNMP request timeout.\n");
    printf(" -R, --snmpretries=RETRIES\n");
    printf("      SNMP request retries.\n");
}

void print_revision_snmp(void) {
    printf(" libnetsnmp v%s\n", netsnmp_get_version());
}

/* vim: set ts=4 sw=4 et syn=c : */

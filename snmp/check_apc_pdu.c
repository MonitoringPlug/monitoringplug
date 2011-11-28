/***
 * Monitoring Plugin - check_apc_pdu.c
 **
 *
 * check_apc_pdu - Check tyhe outlet status of a APC PDU.
 *
 * Copyright (C) 2011 Marius Rieder <marius.rieder@durchmesser.ch>
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

const char *progname  = "check_apc_pdu";
const char *progdesc  = "Check tyhe outlet status of a APC PDU.";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "-H <HOST> [-on <PORTS>] [-off <PORTS>]";

/* MP Includes */
#include "mp_common.h"
#include "snmp_utils.h"
/* Default Includes */
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/* Library Includes */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

/* Global Vars */
const char  *hostname = NULL;
const char  *stateOn = NULL;
const char  *stateOff = NULL;
int         port = 0;

int main (int argc, char **argv) {
    /* Local Vars */
    char        *output = NULL;
    char        *pdu_name = NULL;
    int         status = STATE_OK;
    long        pdu_psu1;
    long        pdu_psu2;
    int         i;
    int         ret;
    struct mp_snmp_table    table_state;
    netsnmp_session         *ss;
    netsnmp_variable_list   *vars;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    ss = mp_snmp_init();

    /* OIDs to query */
    struct mp_snmp_query_cmd snmpcmd[] = {
        {{1,3,6,1,4,1,318,1,1,12,4,1,1,0}, 14, ASN_INTEGER, (void *)&pdu_psu1},
        {{1,3,6,1,4,1,318,1,1,12,4,1,2,0}, 14, ASN_INTEGER, (void *)&pdu_psu2},
        {{1,3,6,1,2,1,1,5,0}, 9, ASN_OCTET_STR, (void *)&pdu_name},
        {{0}, 0, 0, 0},
    };
    struct mp_snmp_query_cmd snmpcmd_table = {{1,3,6,1,4,1,318,1,1,12,3,5,1}, 13, 0, (void *)&table_state};

    ret = mp_snmp_query(ss, snmpcmd);

    ret = mp_snmp_table_query(ss, &snmpcmd_table, 7);

    mp_snmp_deinit();

    // Check for PSU Failure
    if (pdu_psu1 != 1) {
        status = STATE_CRITICAL;
        output = strdup("Power Supply 1 Faild!");
    } else if (pdu_psu2 != 1) {
        status = STATE_CRITICAL;
        output = strdup("Power Supply 2 Faild!");
    }

    if (stateOn == NULL && stateOff == NULL) {
        // Check all outlets for on.
        for (i = 0; i<table_state.row; i++) {
            vars = mp_snmp_table_get(table_state, 3, i);

            if (*vars->val.integer != 1) {
                vars = mp_snmp_table_get(table_state, 1, i);

                char *t = (char *)malloc(9 + vars->val_len);
                memcpy(t, vars->val.string, vars->val_len);
                t[vars->val_len] = '\0';
                strcat(t, " is off!");

                mp_strcat_space(&output, t);
                free(t);
                status = STATE_CRITICAL;
            }
        }
    } else {
        if (stateOn != NULL) {
            char *c, *s, *p;
            p = s = strdup(stateOn);
            while((c = strsep(&s, ","))) {
                i = strtol(c, NULL, 10);
                if (i == 0) {
                    for (i = 0; i<table_state.row; i++) {
                        vars = mp_snmp_table_get(table_state, 1, i);
                        if (strcmp(c, (char*)vars->val.string) == 0)
                            break;
                    }
                } else {
                    i--;
                }
                if (i >= table_state.row)
                    continue;

                vars = mp_snmp_table_get(table_state, 3, i);
                if (*vars->val.integer != 1) {
                    vars = mp_snmp_table_get(table_state, 1, i);

                    char *t = (char *)malloc(9 + vars->val_len);
                    memcpy(t, vars->val.string, vars->val_len);
                    t[vars->val_len] = '\0';
                    strcat(t, " is off!");
                    mp_strcat_space(&output, t);
                    free(t);
                    status = STATE_CRITICAL;
                }
            }
            free( p );
        }
        if (stateOff != NULL) {
            char *c, *s, *p;
            p = s = strdup(stateOff);
            while((c = strsep(&s, ","))) {
                i = strtol(c, NULL, 10);
                if (i == 0) {
                    for (i = 0; i<table_state.row; i++) {
                        vars = mp_snmp_table_get(table_state, 1, i);
                        if (strcmp(c, (char*)vars->val.string) == 0)
                            break;
                    }
                } else {
                    i--;
                }
                if (i >= table_state.row)
                    continue;

                vars = mp_snmp_table_get(table_state, 3, i);
                if (*vars->val.integer != 2) {
                    vars = mp_snmp_table_get(table_state, 1, i);

                    char *t = (char *)malloc(9 + vars->val_len);
                    memcpy(t, vars->val.string, vars->val_len);
                    t[vars->val_len] = '\0';
                    strcat(t, " is on!");
                    mp_strcat_space(&output, t);
                    free(t);
                    status = STATE_CRITICAL;
                }
            }
            free( p );
        }
    }

    /* Output and return */
    if (status == STATE_OK)
        ok("APC PDU %s", pdu_name);
    if (status == STATE_WARNING)
        warning("APC PDU %s [%s]", pdu_name, output);
    critical("APC PDU %s [%s]", pdu_name, output);
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
            MP_LONGOPTS_DEFAULT,
            MP_LONGOPTS_HOST,
            MP_LONGOPTS_PORT,
            {"on", required_argument, NULL, (int)'o'},
            {"off", required_argument, NULL, (int)'O'},
            SNMP_LONGOPTS,
            MP_LONGOPTS_TIMEOUT,
            MP_LONGOPTS_END
    };

    if (argc < 3) {
       print_help();
       exit(STATE_OK);
    }


    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"t:H:p:o:O:"SNMP_OPTSTR, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_snmp(c);

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            /* Hostname opt */
            case 'H':
                getopt_host(optarg, &hostname);
                break;
            /* Port opt */
            case 'P':
                getopt_port(optarg, &port);
                break;
            case 'o':
                stateOn = optarg;
                break;
            case 'O':
                stateOff = optarg;
                break;
            /* Timeout opt */
            case 't':
                getopt_timeout(optarg);
                break;
        }
    }

    return(OK);
}

void print_help (void) {
    print_revision();
    print_revision_snmp();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();

    printf("\nIf no On/Off-Ports are defines all ports are asumed as should be On.\n");
    printf("\nOn/Off-Ports can be named or nubered. ex: --on '1,Outlet 3,4'\n");

    print_help_default();

    printf(" -o, --on=PORT[,PORTS]\n");
    printf("      Ports which should be On.\n");
    printf(" -O, --off=PORT[,PORTS]\n");
    printf("      Ports which should be Off.\n");

    print_help_snmp();
}

/* vim: set ts=4 sw=4 et syn=c : */

/***
 * Monitoring Plugin - check_arc_raid.c
 **
 *
 * check_arc_raid - Check the raids of a Areca RAID Controller by snmp.
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

const char *progname  = "check_arc_raid";
const char *progdesc  = "Check the raids of a Areca RAID Controller by snmp.";
const char *progvers  = "0.1";
const char *progcopy  = "2011";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--hostname <HOSTNAME>";

/* MP Includes */
#include "mp_common.h"
#include "snmp_utils.h"
/* Default Includes */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/* Library Includes */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>


/* Global Vars */
const char  *hostname = NULL;
int         port = 161;

int main (int argc, char **argv) {
    /* Local Vars */
    int         i;
    char        *output = NULL;
    int         status = STATE_OK;
    struct mp_snmp_table    table_state;
    netsnmp_session         *ss;
    netsnmp_variable_list   *vars, *vars2;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    // PLUGIN CODE
    ss = mp_snmp_init();

    /* OIDs to query */
    struct mp_snmp_query_cmd snmpcmd_table = {{1,3,6,1,4,1,18928,1,2,4,1}, 11, 0, (void *)&table_state};
    table_state.row = 0;
    table_state.col = 0;
    table_state.var = NULL;

    status = mp_snmp_table_query(ss, &snmpcmd_table, 8);
    if (status != STAT_SUCCESS) {
        char *string;
        snmp_error(ss, NULL, NULL, &string);
        printf("Error fetching table: %s", string);
    }

    mp_snmp_deinit();

    status = STATE_OK;

    for (i = 0; i<table_state.row; i++) {
        vars = mp_snmp_table_get(table_state, 3, i);

        if (strcmp((char *)vars->val.string, "Normal") == 0) {
            continue;
        }

        vars2 = mp_snmp_table_get(table_state, 1, i);

        char *t = (char *)(vars2->val.string + vars2->val_len - 1);
        while (*t == ' ') {
            *t = '\0';
            t--;
        }
        t = (char *)malloc(7 + vars->val_len + vars2->val_len);
        sprintf(t, "'%s' is %s", vars2->val.string, vars->val.string);
        mp_strcat_comma(&output, t);
        free(t);

        if (status == STATE_CRITICAL || strcmp((char *)vars->val.string, "Rebuilding") != 0)
            status = STATE_CRITICAL;
        else
            status = STATE_WARNING;
    }
    /* Output and return */
    if (status == STATE_OK)
        ok("ARC: All RAIDs in \"Normal\" state.");
    else if (status == STATE_WARNING)
        warning("ARC: %s", output);
    critical("ARC: %s", output);
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
            MP_LONGOPTS_DEFAULT,
            MP_LONGOPTS_HOST,
            MP_LONGOPTS_PORT,
            SNMP_LONGOPTS,
            MP_LONGOPTS_END
    };

    if (argc < 3) {
       print_help();
       exit(STATE_OK);
    }


    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"H:P:o:O:"SNMP_OPTSTR, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_snmp(c);

        switch (c) {
            /* Hostname opt */
            case 'H':
                getopt_host(optarg, &hostname);
                break;
            /* Port opt */
            case 'P':
                getopt_port(optarg, &port);
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
    print_help_default();
    print_help_snmp();
}

/* vim: set ts=4 sw=4 et syn=c : */

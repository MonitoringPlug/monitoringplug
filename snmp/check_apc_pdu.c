/**
 * Monitoring Plugin - check_apc_pdu
 **
 *
 * check_apc_pdu - Check tyhe outlet status of a APC PDU.
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

const char *progname  = "check_apc_pdu";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "-H <HOST>";

#include "mp_common.h"
#include "snmp_utils.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const char *hostname = NULL;

int port = 0;

int main (int argc, char **argv) {
    /* Set signal handling and alarm */
    if (signal (SIGALRM, timeout_alarm_handler) == SIG_ERR)
        exit(STATE_CRITICAL);

    if (process_arguments (argc, argv) == 1)
        exit(STATE_CRITICAL);

    alarm(mp_timeout);

    netsnmp_session *ss;
    ss = mp_snmp_init();

    struct mp_snmp_table table;


    struct mp_snmp_query_cmd snmpcmd = {{1,3,6,1,4,1,318,1,1,12,3,5,1,1}, 14, 0, (void *)&table};
    snmp_table_query(ss, &snmpcmd);


    printf("table %d:%d\n",table.col, table.row);

    int x, y;
    netsnmp_variable_list *vars;


    for(y=0; y<table.row; y++) {
        for(x=0; x<table.col; x++) {
            printf("%d/%d: (%d)\n", x, y, x*table.col+y);
            vars = mp_snmp_table_get(table, x, y);
            print_variable(vars->name, vars->name_length, vars);
        }
    }

    SOCK_CLEANUP;


    ok("end");
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

        getopt_default(c);
        getopt_host(c, optarg, &hostname);
        getopt_port(c, optarg, &port);
        getopt_snmp( c );
        getopt_timeout(c, optarg);

        if (c == 'o') {

        } else if (c == 'O') {

        }
    }

    return(OK);
}

void print_help (void) {
    print_revision();
    print_revision_snmp();
    print_copyright();

    printf("\n");

    printf("This plugin check the outlet status of a APC PDU.");

    printf("\n\n");

    print_usage();

    print_help_default();

    print_help_snmp();

    print_help_timeout();
}


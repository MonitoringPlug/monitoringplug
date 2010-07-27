/**
 * Monitoring Plugin - check_rhc
 **
 *
 * check_rhc - Simulate a plugin timeout.
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

const char *progname  = "check_rhc";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[-t <timeout>]";

#include "mp_common.h"
#include "snmp_utils.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *hostname = NULL;
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
    
    char *clustername;
    int clusterstatus;
    char *clusterstatusdesc;
    int clustervotes;
    int clusterquorum;
    int clusternodes;
    
    struct snmp_query_cmd snmpcmd[] = {
        {{1,3,6,1,4,1,2312,8,2,1,0}, 11, ASN_OCTET_STR, (void *)&clustername},
        {{1,3,6,1,4,1,2312,8,2,2,0}, 11, ASN_INTEGER, (void *)&clusterstatus},
        {{1,3,6,1,4,1,2312,8,2,3,0}, 11, ASN_OCTET_STR, (void *)&clusterstatusdesc},
	{{1,3,6,1,4,1,2312,8,2,5,0}, 11, ASN_INTEGER, (void *)&clustervotes},
        {{1,3,6,1,4,1,2312,8,2,4,0}, 11, ASN_INTEGER, (void *)&clusterquorum},
	{{1,3,6,1,4,1,2312,8,2,7,0}, 11, ASN_INTEGER, (void *)&clusternodes},
        {{0}, 0, 0, 0},
    };
    
    snmp_query(ss, snmpcmd);
    
    //snmp_close(ss);
    SOCK_CLEANUP;


    if (mp_verbose) {
        printf("clustername: %s\n", clustername);
        printf("clusterstatus: %d\n", clusterstatus);
        printf("clusterstatusdesc: %s\n", clusterstatusdesc);
        printf("SNMP Version: %d\n", mp_snmp_version);
    }

    perfdata_int("votes", clustervotes, "", clusterquorum, clusterquorum-1, 0, clusternodes);
    
    if (clusterstatus < 2)
        ok("%s [%s]", clustername, clusterstatusdesc);
    if (clusterstatus < 16)
        warning("%s [%s]", clustername, clusterstatusdesc);
    critical("%s [%s]", clustername, clusterstatusdesc);
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_ARGS_HELP,
        MP_ARGS_VERS,
        MP_ARGS_VERB,
	MP_ARGS_HOST,
	MP_ARGS_PORT,
        MP_ARGS_TIMEOUT,
        MP_ARGS_END
    };

    if (argc < 3) {
       print_help();
       exit(STATE_OK);
    }


    while (1) {
        c = getopt_long (argc, argv, "hVvt:H:p:"SNMPOPTSTRING, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        switch (c) {
            MP_ARGS_CASE_DEF
	    MP_ARGS_CASE_HOST
	    MP_ARGS_CASE_PORT
            MP_ARGS_CASE_TIMEOUT
        }
        getopt_snmp( c );
    }

    return(OK);
}

void print_help (void) {
    print_revision();
    print_revision_snmp();
    print_copyright();

    printf("\n");

    printf("This plugin simulate a plugin timeout.");

    printf("\n\n");

    print_usage();

    print_help_default();
    
    print_help_snmp();
    
    print_help_timeout();
}


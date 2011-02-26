/***
 * Monitoring Plugin - check_dummy
 **
 *
 * check_clustat - Check clustat plugin.
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

const char *progname  = "check_clustat";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "";

/* MP Includes */
#include "mp_common.h"
#include "rhcs_utils.h"
/* Default Includes */
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>

/* Global Vars */
int nonroot = 0;

int main (int argc, char **argv) {
    /* Local Vars */
    FILE                    *fp;
    char                    *missing = NULL;
    char                    *foreign = NULL;
    rhcs_clustat            *clustat;
    rhcs_clustat_group      **groups;
    rhcs_conf               *conf;
    rhcs_conf_fodom_node    **fodomnode;
    rhcs_conf_service       **service;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) == OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    // Need to be root
    if (nonroot == 0)
        mp_noneroot_die();

    alarm(mp_timeout);

    // Parse clustat
    if (nonroot == 0)
        fp = fopen("/etc/cluster/clustat.xml","r");
    else
        fp = fopen("clustat.xml","r");
    clustat = parse_rhcs_clustat(fp);
    fclose(fp);

    if (clustat->local->rgmanager != 1)
        critical("%s [%s] rgmanager not running!", clustat->name, clustat->local->name);

    // Parse cluster.conf
    if (nonroot == 0)
        fp = mp_popen((char *[]) {"/usr/sbin/clustat","-x", NULL});
    else
        fp = fopen("cluster.conf","r");
    conf = parse_rhcs_conf(fp);
    if (nonroot == 0)
        mp_pclose(fp);
    else
        fclose(fp);

    int localprio;
    int ownerprio;
    int bestprio;

    for(groups = clustat->group; *groups != NULL ; groups++) {
        localprio = 0;
        ownerprio = 0;
        bestprio = INT_MAX;

        if ((*groups)->owner == NULL)
            continue;

        fodomnode = NULL;
        for(service = conf->service; *service != NULL ; service++) {
            if (strcmp((*service)->name, (*groups)->name) == 0) {
                fodomnode = (*service)->fodomain->node;
                break;
            }
        }

        if (fodomnode != NULL) {
            for(; *fodomnode != NULL ; fodomnode++) {
                if (strcmp((*fodomnode)->node->name, clustat->local->name) == 0)
                    localprio = (*fodomnode)->priority;
                if (strcmp((*fodomnode)->node->name, (*groups)->owner->name) == 0)
                    ownerprio = (*fodomnode)->priority;
                bestprio = (*fodomnode)->priority<bestprio?(*fodomnode)->priority:bestprio;
            }

            if ((*groups)->owner == clustat->local && localprio > bestprio)
                mp_strcat_comma(&foreign, (*groups)->name);

            if ((*groups)->owner != clustat->local && localprio < ownerprio)
                mp_strcat_comma(&missing, (*groups)->name);

        }
    }

    if (foreign == NULL &&  missing == NULL)
        ok("%s(%d) on %s", clustat->name, clustat->id, clustat->local->name);
    if (foreign == NULL)
        warning("%s(%d) on %s: Missing %s", clustat->name, clustat->id, clustat->local->name, missing);
    if (missing == NULL)
        warning("%s(%d) on %s: Foreign %s", clustat->name, clustat->id, clustat->local->name, foreign);
    warning("%s(%d) on %s: Missing %s  Foreign %s", clustat->name, clustat->id, clustat->local->name, missing, foreign);
    
    return 0;
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        {"noroot", no_argument, NULL, (int)'n'},
        MP_LONGOPTS_END
    };

    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"nt:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        switch(c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            case 'n':
                nonroot = 1;
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
    print_copyright();

    printf("\n");

    printf("This plugin test nothing.");

    printf("\n\n");

    print_usage();

    print_help_default();
}

/***
 * Monitoring Plugin - check_ldap_replication.c
 **
 *
 * check_ldap_replication - Check a OpenLDAP replication.
 *
 * Copyright (C) 2014 Marius Rieder <marius.rieder@durchmesser.ch>
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

const char *progname  = "check_ldap_replication";
const char *progdesc  = "Check a OpenLDAP replication.";
const char *progvers  = "0.1";
const char *progcopy  = "2014";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "-H <LDAP-URI> [-b <binddn>] [-w <bindpw>]";

#define _GNU_SOURCE
/* MP Includes */
#include "mp_common.h"
#include "ldap_utils.h"
/* Default Includes */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ldap.h>
#include <time.h>

/* Global Vars */
thresholds *time_thresholds = NULL;

int main (int argc, char **argv) {
    char *out_warn = NULL;
    char *out_crit = NULL;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);


    /* Start LDAP connection */
    LDAP *ld = NULL;
    ld = mp_ldap_init(NULL);
    LDAPMessage *msg = NULL;

    msg = mp_ldap_search(ld, "cn=Monitor", LDAP_SCOPE_SUB,
            "(&(namingContexts=*)(MonitorUpdateRef=*))",
            (char *[]){"monitorupdateref","namingContexts", '\0'});

    LDAPMessage *database = NULL;
    for (database = ldap_first_entry(ld, msg); database != NULL;
            database = ldap_next_entry(ld, database)) {
        char *namingContexts = NULL;
        char *updateRef = NULL;
        struct berval **vals;
        struct tm csn_tm;
        time_t local_time, remote_time;
        double lag;
    
        vals = ldap_get_values_len(ld, database, "namingContexts");
        if (vals == NULL) {
            if (mp_verbose > 0)
                printf("Skipping %s\n", ldap_get_dn(ld, database));
            continue;
        }
        namingContexts = mp_strdup(vals[0]->bv_val);
        ldap_value_free_len(vals);
    
        vals = ldap_get_values_len(ld, database, "monitorUpdateRef");
        if (vals == NULL) {
            if (mp_verbose > 0)
                printf("Skipping %s\n", ldap_get_dn(ld, database));
            free(namingContexts);
            continue;
        }
        updateRef = mp_strdup(vals[0]->bv_val);
        ldap_value_free_len(vals);
    
        if (mp_verbose > 0) {
            printf("Database Prefix: %s\n", namingContexts);
            printf("Replication Master: %s\n", updateRef);
        }
    
        // Read Local Change Sequence Number
        LDAPMessage *versionmsg = NULL;
        versionmsg = mp_ldap_search(ld, namingContexts, LDAP_SCOPE_BASE,
                "(objectclass=*)", (char *[]){"contextCSN", '\0'});
        vals = ldap_get_values_len(ld, ldap_first_entry(ld, versionmsg), "contextCSN");
    
        if (mp_verbose > 0)
            printf("Local Date: %s\n", vals[0]->bv_val);
    
        memset(&csn_tm, 0, sizeof(struct tm));
        strptime(vals[0]->bv_val, "%Y%m%d%H%M%S", &csn_tm);
        local_time = mktime(&csn_tm);
    
        ldap_msgfree(versionmsg);
    
        // Read Master Change Sequence Number
        LDAP *masterld = NULL;
        masterld = mp_ldap_init(updateRef);
        versionmsg = mp_ldap_search(ld, namingContexts, LDAP_SCOPE_BASE,
                "(objectclass=*)", (char *[]){"contextCSN", '\0'});
        vals = ldap_get_values_len(ld, ldap_first_entry(ld, versionmsg), "contextCSN");
    
        if (mp_verbose > 0)
            printf("Remote Date: %s\n", vals[0]->bv_val);
    
        memset(&csn_tm, 0, sizeof(struct tm));
        strptime(vals[0]->bv_val, "%Y%m%d%H%M%S", &csn_tm);
        remote_time = mktime(&csn_tm);
    
        ldap_msgfree(versionmsg);
   
        ldap_unbind_ext_s(masterld, NULL, NULL);
    
        // Calculate slave lag
        lag = difftime(remote_time, local_time);
        char *buf;
        mp_asprintf(&buf, "%s is %.1fs behind %s", namingContexts, updateRef);
        switch(get_status(lag, time_thresholds)) {
            case STATE_WARNING:
                mp_strcat_comma(&out_warn, buf);
                break;
            case STATE_CRITICAL:
                mp_strcat_comma(&out_crit, buf);
                break;
        }
        free(buf);
        mp_perfdata_float("lag", (float)lag, "s", time_thresholds);
    
        free(namingContexts);
        free(updateRef);
    }

    if (out_crit != NULL)
        critical("LDAP Slave %s", out_crit);
    if (out_warn != NULL)
        warning("LDAP Slave %s", out_warn);
    
    ok("LDAP Slave");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        LDAP_LONGOPTS,
        MP_LONGOPTS_END
    };

    /* Set default */
    mp_threshold_set_warning_time(&time_thresholds, "600s");
    mp_threshold_set_critical_time(&time_thresholds, "3600s");

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"w:c:"LDAP_OPTSTR, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_ldap(c);
        getopt_wc_time(c, optarg, &time_thresholds);
    }

    /* Check requirements */
    if (!mp_ldap_uri)
        usage("LDAP-URI is mandatory.");

    return(OK);
}

void print_help (void) {
    print_revision();
    print_revision_ldap();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();

    print_help_default();
    print_help_ldap();
    print_help_warn_time("600s");
    print_help_crit_time("3600s");
}

/* vim: set ts=4 sw=4 et syn=c : */

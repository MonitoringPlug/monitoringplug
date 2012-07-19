/***
 * Monitoring Plugin - replyd.c
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

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <ctype.h>

#include <sys/types.h>
#include <dirent.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "replayd.h"

#define REPLAYAGENT_DAEMON "replayd"


/* Global vars */
int         keep_running = 1;
int         verbose = 0;
const char  *directory = "replays";
static oid  reply_oid[2] = { 1, 3 };

static void stop_server(int signo) {
   keep_running = 0;
   if(verbose)
       printf("Exiting on signal %d\n", signo);
}

replay_entry *replay_list = NULL;

netsnmp_variable_list *parse_reply(char *name) {
    FILE *fr;
    char *fname;
    char *line;
    char *oid_str;
    char *type_str;
    char *value;
    int lineno=0;
    netsnmp_variable_list *list = NULL;
    netsnmp_variable_list *lptr = NULL;

    fname = malloc(strlen(directory) + strlen(name) + 2);
    sprintf(fname, "%s/%s", directory, name);
    fr = fopen(fname, "r");

    line = malloc(256);

    while(fgets(line, 256, fr) != NULL) {
        lineno++;
        value = line;
        value[strlen(value)-1] = '\0';
        oid_str = strsep(&value, " ");
        strsep(&value, " "); // Skip '='
        type_str = strsep(&value, ":");
        if (value) {
            value++;
        }

        if (lptr) {
            lptr->next_variable = malloc(sizeof(netsnmp_variable_list));
            lptr = lptr->next_variable;
        } else {
            lptr = malloc(sizeof(netsnmp_variable_list));
            list = lptr;
        }
        lptr->name = malloc(sizeof(oid)*MAX_OID_LEN);

        lptr->name_length = MAX_OID_LEN;
        if (!snmp_parse_oid(oid_str, lptr->name, &lptr->name_length))
            printf("Parsing oid faild. File %s, line %d\n", fname, lineno);

        lptr->type = ra_type_atoi(type_str);

        if (lptr->type == 0)
            printf("Unrecognised '%s'\n", type_str);

        switch(lptr->type) {
            case ASN_INTEGER:
            case ASN_TIMETICKS:
            case ASN_GAUGE:
            case ASN_COUNTER:
                lptr->val.integer = malloc(sizeof(long));
                if (strchr(value, (int)'(')) {
                    value = strchr(value, (int)'(');
                    value++;
                }
                *(lptr->val.integer) = strtol(value, NULL, 10);
                lptr->val_len = sizeof(long);
                break;
            case ASN_OCTET_STR:
                if (!value) {
                    lptr->val.string = NULL;
                    lptr->val_len = 0;
                    break;
                }
                if (value[0] == '"') {
                    value++;
                    value[strlen(value)-1] = '\0';
                }
                lptr->val.string = (u_char *)strdup(value);
                lptr->val_len = strlen(value);
                break;
            case ASN_OBJECT_ID:
                lptr->val.objid = malloc(sizeof(oid)*MAX_OID_LEN);
                lptr->val_len = MAX_OID_LEN;
                snmp_parse_oid(value, lptr->val.objid, &(lptr->val_len));
                lptr->val_len *= sizeof(oid);
                break;
        }

    }

    return list;
}

int replay_handler(netsnmp_mib_handler *handler,
      netsnmp_handler_registration *reginfo,
      netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests) {

    replay_entry *rent;
    for (rent = replay_list; rent; rent = rent->next) {
        if (strcmp(rent->name, reginfo->contextName) == 0)
            break;
    }
    if (!rent)
        return SNMP_ERR_NOERROR;

    if (!rent->vars) {
        if (verbose > 1)
            printf("Load reply for %s.\n", rent->name);
        rent->vars = parse_reply(rent->name);
    }

    while (requests) {
        netsnmp_variable_list *var = requests->requestvb;
        netsnmp_variable_list *ptr = rent->vars;

        if (verbose > 1) {
            printf("[replay_handler] %s ", reginfo->contextName);
            print_variable(var->name, var->name_length, var);
        }

        switch (reqinfo->mode) {
        case MODE_GET:
            while (ptr) {
                if (netsnmp_oid_equals(var->name, var->name_length,
                            ptr->name, ptr->name_length) == 0) {

                    snmp_set_var_typed_value(var, ptr->type,
                            (u_char *) ptr->val.bitstring,
                            ptr->val_len);
                }
                ptr = ptr->next_variable;
            }
            break;

        case MODE_GETNEXT:
            while (ptr) {
                if (snmp_oid_compare(var->name, var->name_length,
                            ptr->name, ptr->name_length) < 0) {
                    snmp_set_var_objid(var, ptr->name, ptr->name_length);
                    snmp_set_var_typed_value(var, ptr->type,
                            (u_char *) ptr->val.bitstring,
                            ptr->val_len);
                    break;
                }
                ptr = ptr->next_variable;
            }
            break;

        default:
            netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_GENERR);
            break;
        }
        requests = requests->next;
    }
    return SNMP_ERR_NOERROR;
}

int main(int argc, char *argv[]) {
    int rv;
    int arg;
    DIR *dir;
    FILE *file;
    struct dirent *dEnt;
    replay_entry *rEnt;

    // Process options
    while ((arg = getopt(argc, argv, "vd:L::")) != EOF) {
        switch (arg) {
            case 'v':
                verbose += 1;
                break;
            case 'd':
                directory = optarg;
                break;
            case 'L':
                snmp_log_options(optarg, arg, argv);
        }
    }

    // Read replays
    dir = opendir(directory);
    if (dir != NULL) {
        while ((dEnt = readdir (dir)) != NULL) {
            if (dEnt->d_name[0] == '.')
                continue;
            rEnt = malloc(sizeof(replay_entry));
            memset(rEnt, 0, sizeof(replay_entry));
            rEnt->name = strdup(dEnt->d_name);
            rEnt->next = replay_list;
            replay_list = rEnt;
        }
        closedir (dir);
    } else {
        perror ("Read replays");
        return 1;
    }

    // Write config
    file = fopen(REPLAYAGENT_DAEMON".conf", "w");
    fprintf(file, "# Auto generated config!\n");
    for (rEnt = replay_list; rEnt; rEnt = rEnt->next) {
        fprintf(file, "com2sec -Cn %s rSec default %s\n", rEnt->name, rEnt->name);
    }
    fprintf(file, "group rGroup v2c rSec\n");
    fprintf(file, "view all included .1 80\n");
    fprintf(file, "access rGroup \"\" any noauth prefix all none none\n");
    fclose(file);
    set_configuration_directory(".");

    if (verbose)
        printf("Start: "REPLAYAGENT_DAEMON"\n");
    SOCK_STARTUP;

    // init agent
    if (verbose)
        printf(" init_agent => ");
    rv = init_agent(REPLAYAGENT_DAEMON);
    if (verbose)
        printf("[%d]\n", rv);

    read_configs();

   // Register Handler
   if (verbose)
       printf(" netsnmp_register_handler:\n");
   for (rEnt = replay_list; rEnt; rEnt = rEnt->next) {
       netsnmp_handler_registration * rh;
       rh = netsnmp_create_handler_registration
               ("replyHandler", replay_handler, reply_oid, 2, HANDLER_CAN_RONLY);
       rh->contextName = rEnt->name;
       rv = netsnmp_register_handler(rh);
       if (verbose)
           printf("  %s [%d]\n", rEnt->name, rv);
   }
   if (verbose)
       printf("[%d]\n", rv);

   netsnmp_ds_set_string(NETSNMP_DS_APPLICATION_ID,
           NETSNMP_DS_AGENT_PORTS, "UDP::1661");
   add_to_init_list(strdup("!smux"));

   // Init master
   if (verbose)
       printf(" init_master_agent => ");
   rv = init_master_agent();
   if (verbose)
       printf("[%d]\n", rv);

   // Setup signal ahndling
   signal(SIGTERM, stop_server);
   signal(SIGINT, stop_server);

   if (verbose)
       printf(" running\n");


   /* Loop */
   while(keep_running) {
      agent_check_and_process(1);
   }

   /* shutdown */
   snmp_shutdown(REPLAYAGENT_DAEMON);

   SOCK_CLEANUP;

   return 0;
}

/* vim: set ts=4 sw=4 et syn=c : */

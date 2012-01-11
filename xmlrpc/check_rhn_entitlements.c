/***
 * Monitoring Plugin - check_rhn_entitlements.c
 **
 *
 * check_rhn_entitlements - Check RHN or a satellite for available entitlements.
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

const char *progname  = "check_rhn_entitlements";
const char *progdesc  = "Check RHN or a satellite for available entitlements.";
const char *progvers  = "0.1";
const char *progcopy  = "2011";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--url URL --user USER --pass PASS [--channel CHANNEl] [--system SYSTEM]";

/* MP Includes */
#include "mp_common.h"
#include "xmlrpc_utils.h"
/* Default Includes */
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* Library Includes */
#include <xmlrpc.h>
#include <xmlrpc_client.h>

/* Global Vars */
const char *url = NULL;
const char *user = NULL;
const char *pass = NULL;
char **channel = NULL;
char **system_name = NULL;
int channels = 0;
int systems = 0;
thresholds *free_thresholds = NULL;

int main (int argc, char **argv) {
    /* Local Vars */
    xmlrpc_env env;
    xmlrpc_value *result;
    xmlrpc_value *array;
    xmlrpc_value *item;
    xmlrpc_value *value;
    char *out = NULL;
    char *label;
    char *name;
    char *key;
    int used_slots;
    int total_slots;
    int size, state=0, i, j;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    /* Create XMLRPC env */
    env = mp_xmlrpc_init();

    /* RHN Login */
    result = xmlrpc_client_call(&env, url, "auth.login", "(ss)", user, pass);
    unknown_if_xmlrpc_fault(&env);

    xmlrpc_parse_value(&env, result, "s", &key);

    if (mp_verbose >= 2) {
        printf("Login Successfull: %s\n", key);
    }

    /* Get RHN Entitlements */
    result = xmlrpc_client_call(&env, url, "satellite.listEntitlements", "(s)",
            key);
    unknown_if_xmlrpc_fault(&env);

    if (mp_verbose >= 2) {
        printf("Get Entitlements successfully\n");
    }

    if(xmlrpc_value_type(result) != XMLRPC_TYPE_STRUCT)
        unknown("Unknow return value.");

    /* Check system entitlements */
    if (system_name) {
        xmlrpc_struct_read_value(&env, result, "system", &array);
        unknown_if_xmlrpc_fault(&env);

        size = xmlrpc_array_size(&env, array);
        unknown_if_xmlrpc_fault(&env);

        for(i=0; i < size; i++) {
            xmlrpc_array_read_item(&env, array, i, &item);
            unknown_if_xmlrpc_fault(&env);

            xmlrpc_struct_read_value(&env, item, "label", &value);
            unknown_if_xmlrpc_fault(&env);

            xmlrpc_parse_value(&env, value, "s", &label);
            unknown_if_xmlrpc_fault(&env);

            if (mp_verbose >= 2) {
                printf("systems => %s\n", label);
            }

            for(j=0; j < systems; j++) {
                if (strcmp(system_name[j], label) == 0) {
                    // Read Name
                    xmlrpc_struct_read_value(&env, item, "name", &value);
                    unknown_if_xmlrpc_fault(&env);
                    xmlrpc_parse_value(&env, value, "s", &name);
                    unknown_if_xmlrpc_fault(&env);

                    if (mp_verbose >= 2) {
                        printf(" name => %s\n", name);
                    }

                    // Read Free
                    xmlrpc_struct_read_value(&env, item, "used_slots", &value);
                    unknown_if_xmlrpc_fault(&env);
                    xmlrpc_parse_value(&env, value, "i", &used_slots);
                    unknown_if_xmlrpc_fault(&env);

                    if (mp_verbose >= 2) {
                        printf(" used_slots => %d\n", used_slots);
                    }

                    // Read Total
                    xmlrpc_struct_read_value(&env, item, "total_slots", &value);
                    unknown_if_xmlrpc_fault(&env);
                    xmlrpc_parse_value(&env, value, "i", &total_slots);
                    unknown_if_xmlrpc_fault(&env);

                    if (mp_verbose >= 2) {
                        printf(" total_slots => %d\n", total_slots);
                    }

                    char *tmp;
                    tmp = mp_malloc(128);
                    mp_snprintf(tmp, 128, "%s (%d/%d)", name, used_slots, total_slots);
                    mp_strcat_space(&out, tmp);
                    free(tmp);

                    mp_perfdata_int3(label, used_slots, "",
                            1, (total_slots - free_thresholds->warning->start),
                            1, (total_slots - free_thresholds->critical->start),
                            1, 0, 1, total_slots);

                    if (state < get_status((total_slots-used_slots),free_thresholds)) {
                        state = get_status((total_slots-used_slots),free_thresholds);
                    }
                }
            }
        }
    }

    /* Check channel entitlements */
    if (channel) {
        xmlrpc_struct_read_value(&env, result, "channel", &array);
        unknown_if_xmlrpc_fault(&env);

        size = xmlrpc_array_size(&env, array);
        unknown_if_xmlrpc_fault(&env);

        for(i=0; i < size; i++) {
            xmlrpc_array_read_item(&env, array, i, &item);
            unknown_if_xmlrpc_fault(&env);

            xmlrpc_struct_read_value(&env, item, "label", &value);
            unknown_if_xmlrpc_fault(&env);

            xmlrpc_parse_value(&env, value, "s", &label);
            unknown_if_xmlrpc_fault(&env);

            if (mp_verbose >= 2) {
                printf("channel => %s\n", label);
            }

            for(j=0; j < channels; j++) {
                if (strcmp(channel[j], label) == 0) {
                    // Read Name
                    xmlrpc_struct_read_value(&env, item, "name", &value);
                    unknown_if_xmlrpc_fault(&env);
                    xmlrpc_parse_value(&env, value, "s", &name);
                    unknown_if_xmlrpc_fault(&env);

                    if (mp_verbose >= 2) {
                        printf(" name => %s\n", name);
                    }

                    // Read Free
                    xmlrpc_struct_read_value(&env, item, "used_slots", &value);
                    unknown_if_xmlrpc_fault(&env);
                    xmlrpc_parse_value(&env, value, "i", &used_slots);
                    unknown_if_xmlrpc_fault(&env);

                    if (mp_verbose >= 2) {
                        printf(" used_slots => %d\n", used_slots);
                    }

                    // Read Total
                    xmlrpc_struct_read_value(&env, item, "total_slots", &value);
                    unknown_if_xmlrpc_fault(&env);
                    xmlrpc_parse_value(&env, value, "i", &total_slots);
                    unknown_if_xmlrpc_fault(&env);

                    if (mp_verbose >= 2) {
                        printf(" total_slots => %d\n", total_slots);
                    }

                    char *tmp;
                    tmp = mp_malloc(128);
                    mp_snprintf(tmp, 128, "%s (%d/%d)", name, used_slots, total_slots);
                    mp_strcat_space(&out, tmp);
                    free(tmp);

                    mp_perfdata_int3(label, used_slots, "",
                            1, (total_slots - free_thresholds->warning->start),
                            1, (total_slots - free_thresholds->critical->start),
                            1, 0, 1, total_slots);

                    if (state < get_status((total_slots-used_slots),free_thresholds)) {
                        state = get_status((total_slots-used_slots),free_thresholds);
                    }
                }
            }
        }
    }

    switch ( state ) {
        case STATE_OK:
            ok(out);
            break;
        case STATE_WARNING:
            warning(out);
            break;
        case STATE_CRITICAL:
            critical(out);
            break;
    }

    unknown(out);
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_TIMEOUT,
        MP_LONGOPTS_EOPT,
        {"url", required_argument, NULL, (int)'U'},
        {"user", required_argument, NULL, (int)'u'},
        {"pass", required_argument, NULL, (int)'p'},
        {"channel", required_argument, NULL, (int)'C'},
        {"system", required_argument, NULL, (int)'S'},
        MP_LONGOPTS_END
    };

    /* Set default */
    setWarn(&free_thresholds, "10:",0);
    setCrit(&free_thresholds, "5:",0);

    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"E::t:U:u:p:C:S:w:c:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            /* Local opts */
            case 'U':
                getopt_url(optarg, &url);
                break;
            case 'u':
                user = optarg;
                break;
            case 'p':
                pass = optarg;
                break;
            case 'C':
                mp_array_push(&channel, optarg, &channels);
                break;
            case 'S':
                mp_array_push(&system_name, optarg, &systems);
                break;
            /* Timeout opt */
            case 't':
                getopt_timeout(optarg);
                break;
        }

        getopt_wc(c, optarg, &free_thresholds);
    }

    /* Check requirements */
    if (!url)
        usage("URL is mandatory.");
    if (!is_url_scheme(url, "http") && !is_url_scheme(url, "https"))
        usage("Only http(s) urls are allowed.");
    if (!user)
        usage("Username is mandatory.");
    if (!pass)
        usage("Password is mandatory.");
    if (!channel && !system_name)
        usage("Channel or System entitlement to check is mandatory.");

    return(OK);
}

void print_help (void) {
    print_revision();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();

    print_help_default();

    printf(" -U, --url=URL\n");
    printf("      URL of the RHN XML-RPC api.\n");
    printf(" -u, --user=USER\n");
    printf("      USER to log in.\n");
    printf(" -p, --pass=PASS\n");
    printf("      PASS to log in.\n");
    printf(" -C, --channel=CHANNEL\n");
    printf("      CHANNEL entitlement to check.\n");
    printf(" -S, --system=SYSTEM\n");
    printf("      SYSTEM  entitlement to check.\n");
}

/* vim: set ts=4 sw=4 et syn=c : */

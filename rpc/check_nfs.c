/***
 * monitoringplug - check_nfs.c
 **
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

const char *progname  = "check_nfs";
const char *progvers  = "0.1";
const char *progcopy  = "2011";
const char *progauth = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "-H hostname [--help] [--timeout TIMEOUT]";

/* MP Includes */
#include "mp_common.h"
#include "rpc_utils.h"
/* Default Includes */
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
/* Library Includes */
#include <rpc/rpc.h>
#include <rpcsvc/mount.h>

/* Global Vars */
const char *hostname = NULL;
const char *export = NULL;
char *noconnection = NULL;
char *callfaild = NULL;
char *noexport = NULL;
char *nonfs = NULL;
char *exportok = NULL;
struct timeval to;
char **rpcversion = NULL;
int rpcversions = 0;
char **rpctransport = NULL;
int rpctransports = 0;

/* Function prototype */
int check_export(struct rpcent *program, unsigned long version, char *proto);

int main (int argc, char **argv) {
    /* Local Vars */
    int i, j;
    char *buf;
    struct rpcent *program;
    struct rpcent *nfs;

    /* Set signal handling and alarm */
    if (signal (SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);
    to.tv_sec = mp_timeout;
    to.tv_usec = 0;

    // PLUGIN CODE
    program = rpc_getrpcent("showmount");
    if (program == NULL)
        program = rpc_getrpcent("mount");
    if (program == NULL)
        program = rpc_getrpcent("mountd");
    nfs = rpc_getrpcent("nfs");

    for(i=0; i < rpcversions; i++) {
        for(j=0; j < rpctransports; j++) {
            check_export(program, atoi(rpcversion[i]), rpctransport[j]);

            if (rpc_ping((char *)hostname, nfs, atoi(rpcversion[i]), rpctransport[j], to) != RPC_SUCCESS) {
                buf = mp_malloc(128);
                mp_snprintf(buf, 128, "%s:v%s", rpctransport[j], rpcversion[i]);
                mp_strcat_comma(&nonfs, buf);
                free(buf);
            }
        }
    }

    free(program->r_name);
    free(program);
    free(nfs->r_name);
    free(nfs);

    if (noconnection || callfaild || noexport || nonfs) {
        char *out = NULL;
        if (noconnection) {
            out = strdup("Can't connect to:");
            mp_strcat_space(&out, noconnection);
        }
        if (callfaild) {
            mp_strcat_space(&out, "Call faild by:");
            mp_strcat_space(&out, callfaild);
        }
        if (noexport) {
            mp_strcat_space(&out, "No export found by:");
            mp_strcat_space(&out, noexport);
        }
        if (nonfs) {
            mp_strcat_space(&out, "NFS not responding:");
            mp_strcat_space(&out, nonfs);
        }
        critical(out);
    } else {
        if(export)
            ok("%s exported by %s", export, exportok);
        ok("mountd export by %s", exportok);
    }

    critical("You should never reach this point.");
}

int check_export(struct rpcent *program, unsigned long version, char *proto) {
    CLIENT *client;
    char *buf;
    int ret;
    exports exportlist;

    buf = mp_malloc(128);
    mp_snprintf(buf, 128, "%s:%sv%ld", proto, program->r_name, version);

    if (mp_verbose >= 1)
        printf("Connect to %s:%sv%ld", proto, program->r_name, version);

    client = clnt_create((char *)hostname, program->r_number, 3, proto);
    if (client == NULL) {
        if (mp_verbose >= 1)
            printf("   faild!\n");
        mp_strcat_comma(&noconnection, buf);
        free(buf);
        return 1;
    }

    if (mp_verbose >= 1)
        printf("   ok\n");

    memset(&exportlist, '\0', sizeof(exportlist));

    ret = clnt_call(client, MOUNTPROC_EXPORT, (xdrproc_t) xdr_void, NULL,
            (xdrproc_t) mp_xdr_exports, (caddr_t) &exportlist, to);
    if (ret != RPC_SUCCESS) {
        if (mp_verbose >= 1)
            printf("Get export tailed. %d: %s\n", ret, clnt_sperrno(ret));
        mp_strcat_comma(&callfaild, buf);
        free(buf);
        clnt_destroy(client);
        return 1;
    }

    if (mp_verbose >= 1) {
        exports exl;
        groups grouplist;
        exl = exportlist;
        while (exl) {

            printf("%-*s ", 40, exl->ex_dir);
            grouplist = exl->ex_groups;
            if (grouplist)
                while (grouplist) {
                    printf("%s%s", grouplist->gr_name,
                            grouplist->gr_next ? "," : "");
                    grouplist = grouplist->gr_next;
                }
            else
                printf("(everyone)");

            printf("\n");
            exl = exl->ex_next;
        }
    }

    if (export != NULL) {

        while (exportlist) {
            if (strcmp(export, exportlist->ex_dir) == 0)
                break;
            exportlist = exportlist->ex_next;
        }

        if (exportlist==NULL) {
            mp_strcat_comma(&noexport, buf);
            free(buf);
            return 1;
        }
    } else {
        if (exportlist==NULL) {
            mp_strcat_comma(&noexport, buf);
            free(buf);
            return 1;
        }
    }

    clnt_freeres(client, (xdrproc_t) mp_xdr_exports, (caddr_t) &exportlist);
    clnt_destroy(client);

    mp_strcat_comma(&exportok, buf);
    free(buf);

    return 0;
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        // PLUGIN OPTS
        {"export", required_argument, 0, 'e'},
        {"rpcversion", required_argument, 0, 'r'},
        {"transport", required_argument, 0, 'T'},
        MP_LONGOPTS_TIMEOUT,
        MP_LONGOPTS_END
    };

    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"H:e:r:T:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            /* Host opt */
            case 'H':
                getopt_host(optarg, &hostname);
                break;
            /* Plugin opts */
            case 'e':
                export = optarg;
                break;
            case 'r':
                mp_array_push(&rpcversion, optarg, &rpcversions);
                break;
            case 'T': {
                mp_array_push(&rpctransport, optarg, &rpctransports);
                break;
            }
            /* Timeout opt */
            case 't':
                getopt_timeout(optarg);
                break;
        }
    }

    /* Check requirements */
    if (!hostname)
        usage("Hostname is mandatory.");

    /* Apply defaults */
    if (rpcversion == NULL)
        mp_array_push(&rpcversion, "3", &rpcversions);
    if (rpctransport == NULL) {
        mp_array_push(&rpctransport, "udp", &rpctransports);
        mp_array_push(&rpctransport, "tcp", &rpctransports);
    }

    return(OK);
}

void print_help (void) {
    print_revision();
    print_copyright();

    printf("\n");

    printf("Check description: Check if the Host is exporting at least one or the named path.");

    printf("\n\n");

    print_usage();

    print_help_default();
    print_help_host();
    printf(" -T, --transport=transport[,transport]\n");
    printf("      Transports to check.\n");
    printf(" -r, --rpcversion=version[,version]\n");
    printf("      Versions to check.\n");
    printf(" -e, --export=parh\n");
    printf("      Check it server exports path.\n");
}

/* vim: set ts=4 sw=4 et syn=c : */

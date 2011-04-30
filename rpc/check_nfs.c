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
const char *progusage = "[--help] [--timeout TIMEOUT]";

/* MP Includes */
#include "mp_common.h"
#include "mp_net.h"
#include "rpc_utils.h"
/* Default Includes */
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <rpc/rpc.h>
#include <rpcsvc/mount.h>
/* Library Includes */

/* Global Vars */
const char *hostname = NULL;
int port = 111;
int udp = 0;
int tcp = 0;
int version = 0;

/* Function prototype */

int main (int argc, char **argv) {
    /* Local Vars */
    struct addrinfo *result, *rp;
    CLIENT *clnt = NULL;
    struct timeval to;
    exports exportlist;

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
    result = mp_getaddrinfo(hostname, port, AF_INET, SOCK_DGRAM);

    for(rp = result; rp != NULL; rp = rp->ai_next) {
        if (mp_verbose >= 1) {
            printf("Connect to %s\n", mp_ip2str(rp->ai_addr));
        }

        clnt = rpc_udp_connect((struct sockaddr_in *)rp->ai_addr, "mountd", 1);

        if (clnt) break;
    }

    if (!clnt)
        critical("Can't connect!");

    //freeaddrinfo(result);

    printf("::1 0x%X\n", clnt);
    int i = clnt_call(clnt, MOUNTPROC_EXPORT, (xdrproc_t) xdr_void, NULL, (xdrproc_t) xdr_exports, (caddr_t) &exportlist, to);
    printf("::2 %d\n", i);


    critical("You should never reach this point.");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        // PLUGIN OPTS
        {"tcp", no_argument, 0, 'T'},
        {"udp", no_argument, 0, 'U'},
        MP_LONGOPTS_TIMEOUT,
        MP_LONGOPTS_END
    };

    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"1234H:TU", longopts, &option);

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
            case '1':
                version |= 1;
                break;
            case '2':
                version |= 2;
                break;
            case '3':
                version |= 4;
                break;
            case '4':
                version |= 8;
                break;
            case 'T':
                tcp = 1;
                break;
            case 'U':
                udp = 1;
                break;
            /* Timeout opt */
            case 't':
                getopt_timeout(optarg);
                break;
        }
    }

    /* Check requirements */
    if (!hostname)
        usage("Hostname is mandatory.");

    /* Aplly defaults */
    udp = (udp || tcp) ? udp : 1;
    version = version ? version : 4;

    return(OK);
}

void print_help (void) {
    print_revision();
    print_copyright();

    printf("\n");

    printf("Check description: check_nfs");

    printf("\n\n");

    print_usage();

    print_help_default();
    //printf(" -f, --file=filename\n");
    //printf("      The file to test.\n");

}

/* vim: set ts=4 sw=4 et syn=c : */

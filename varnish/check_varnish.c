/***
 * Monitoring Plugin - check_varnish.c
 **
 *
 * check_varnish - This plugin checks a Varnish instance.
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

const char *progname  = "check_varnish";
const char *progdesc  = "This plugin checks a Varnish instance.";
const char *progvers  = "0.1";
const char *progcopy  = "2013";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[-n <varnish_name>]";

/* MP Includes */
#include "mp_common.h"
#include "mp_net.h"
/* Default Includes */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
/* Library Includes */
#include <varnishapi.h>
#include <vsm.h>
#include <vcli.h>

/* Global Vars */
int use_shm = 1;
struct VSM_data *vd;
char *secret_file = NULL;
char *admin_socket = NULL;

int mp_varnish_stats_cb(void *priv, const struct VSC_point *const pt);

int main (int argc, char **argv) {
    /* Local vars */
    int  vsock;
    char *instance = "";
    char *answer = NULL;
    char *admin_host = NULL;
    int admin_port = 6082;
    unsigned int status;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Init VSM Data */
    if (use_shm) {
        vd = VSM_New();
        VSC_Setup(vd);
    }

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    if (use_shm) {
        /* Open the Varnish SHM */
        if (VSM_Open(vd, mp_verbose)) {
            critical("Varnish: Could not open shared memory for '%s'", VSM_Name(vd));
        }
        instance = mp_strdup(VSM_Name(vd));

        /* Reading the stats */
        (void)VSC_Iter(vd, mp_varnish_stats_cb, NULL);

        /* Get arg from SHM if needed */
        if (admin_socket == NULL) {
            answer = VSM_Find_Chunk(vd, "Arg", "-T", "", NULL);
            if (answer == NULL)
                critical("Varnish: No -T arg in shared memory");
            admin_socket = mp_strdup(answer);
            if (mp_verbose > 1)
                printf("-T from SHM: %s\n", admin_socket);
        }
        if (secret_file == NULL) {
            answer = VSM_Find_Chunk(vd, "Arg", "-S", "", NULL);
            if (answer != NULL)
                secret_file = mp_strdup(answer);
            if (mp_verbose > 1)
                printf("-S from SHM: %s\n", secret_file);
        }

        /* Close SHM */
        VSM_Close(vd);
    }

    /* Connect to Varnish admin port */
    admin_host = strsep(&admin_socket, ": ");
    admin_port = (int)strtol(admin_socket, NULL, 10);
    vsock = mp_connect(admin_host, admin_port, AF_INET, SOCK_STREAM);

    /* Check auth */
    VCLI_ReadResult(vsock, &status, &answer, 10);
    if (mp_verbose > 2)
        printf("[%d] %s\n", status, answer);
    if (status == CLIS_AUTH) {
        if (secret_file == NULL) {
            close(vsock);
            critical("Varnish: Authentication required");
        }
        int sfd = open(secret_file, O_RDONLY);
        if (sfd < 0) {
            close(vsock);
            critical("Varnish Cannot open \"%s\": %s", secret_file,
                    strerror(errno));
        }
        char *buf;
        buf = mp_malloc(CLI_AUTH_RESPONSE_LEN+1);
        VCLI_AuthResponse(sfd, answer, buf);
        close(sfd);
        free(answer);

        send(vsock, "auth ", 5, 0);
        send(vsock, buf, strlen(buf), 0);
        send(vsock, "\r\n", 2, 0);

        free(buf);

        (void)VCLI_ReadResult(vsock, &status, &answer, 10);
        if (mp_verbose > 2)
            printf("[%d] %s\n", status, answer);
    }

    /* Check Status */
    if (status != CLIS_OK) {
        close(vsock);
        critical("Varnish: Rejected %d - %s", status, answer);
    }
    free(answer);

    /* Send ping */
    send(vsock, "ping\r\n", 6, 0);
    VCLI_ReadResult(vsock, &status, &answer, 10);
    if (mp_verbose > 2)
        printf("[%d] %s\n", status, answer);
    free(answer);
    if (status != CLIS_OK)
        critical("Varnish: PING failed.");

    /* Send status */
    send(vsock, "status\r\n", 8, 0);
    VCLI_ReadResult(vsock, &status, &answer, 10);
    if (mp_verbose > 2)
        printf("[%d] %s\n", status, answer);

    if (status != CLIS_OK)
        critical("Varnish: %d - %s", status, answer);

    if (strcmp(instance, "") == 0)
        ok("Varnish: %s", answer);
    else
        ok("Varnish: '%s' - %s", instance, answer);
}

int mp_varnish_stats_cb(void *priv, const struct VSC_point *const pt) {
    uint64_t val;

    if (strcmp(pt->class, "") != 0)
        return 1;

    val = *(const volatile uint64_t*)pt->ptr;

    if (strncmp(pt->name, "client_", 7) == 0 ||
            strncmp(pt->name, "cache_", 6) == 0 ) {
        mp_perfdata_int(pt->name, (int)val, pt->flag == 'a' ? "c" : "", NULL);
    }

    if (mp_verbose > 2) {
        if (strcmp(pt->class, "") != 0)
            printf("%s.", pt->class);
        if (strcmp(pt->ident, "") != 0)
            printf("%s.", pt->ident);
        printf("%s: ", pt->name);
        if (strcmp(pt->fmt, "uint64_t") == 0) {
            printf("%12ju:%c", val, (char)pt->flag);
        }
        printf("\n");
    }
    return 0;
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        {"no-shm", no_argument, NULL, (int)'N'},
        MP_LONGOPTS_END
    };

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"NS:T:"VSC_ARGS, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        switch (c) {
            case 'N':
                use_shm = 0;
                break;
            case 'n':
                VSC_Arg(vd, c, optarg);
                break;
            case 'S':
                secret_file = optarg;
                break;
            case 'T':
                admin_socket = optarg;
                break;
        }

    }

    if (!use_shm && admin_socket == NULL) {
        usage("Admin Host/Port or SHM is required.");
    }

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

    printf(" -N, --no-shm\n");
    printf("      Do not use shared memory.\n");
    printf(" -n <varnish_name>\n");
    printf("      Name of the Varnish instance to query.\n");
    printf(" -S <secret>\n");
    printf("      Path of the varnish secret.\n");
    printf(" -T <host>:<port>\n");
    printf("      Host and port of the Varnish admin port.\n");

}

/* vim: set ts=4 sw=4 et syn=c : */

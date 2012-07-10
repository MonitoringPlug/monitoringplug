/***
 * Monitoring Plugin - check_multipath.c
 **
 *
 * check_multipath - Check multipath status.
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

const char *progname  = "check_multipath";
const char *progdesc  = "Check multipath status.";
const char *progvers  = "0.1";
const char *progcopy  = "2012";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "";

/* MP Includes */
#include "mp_common.h"
/* Default Includes */
#include <errno.h>
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
    FILE        *fp;
    mp_subprocess_t *subp;
    char        line[128];
    int         failed = 0;
    int         lines = 0;
    uid_t       uid;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    // Need to be root
    if (nonroot == 0)
        mp_noneroot_die();

    alarm(mp_timeout);

    // Parse clustat
    if (nonroot == 0) {
        uid = getuid();
        if (setuid(0) != 0)
            unknown("setuid failed");
        subp = mp_subprocess((char *[]) {"/sbin/multipath","-l", NULL});
        fp = fdopen(subp->stdout, "r");
        close(subp->stdin);
        if (uid != 0)
            setuid(uid);
    } else {
        subp = mp_subprocess((char *[]) {"/usr/bin/sudo", "/sbin/multipath","-l", NULL});
        fp = fdopen(subp->stdout, "r");
        close(subp->stdin);
    }

    if (fp == NULL)
       unknown("Can't exec multipath");

    while ( fgets(line, sizeof line, fp) != NULL ) {
        if (mp_verbose > 1) {
            printf(">> %s", line);
        }

        lines++;
        if (strstr(line, "failed") != NULL) {
            failed++;
        }

    }

    int r = mp_subprocess_close(subp);
    if (r != 0) {
        critical("Executing multipath failed! (%d)", r);
    }

    if (lines == 0)
        warning("No paths defined");
    else if (failed > 0)
        critical("%d paths failed", failed);

    ok("Multipath");
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
        c = mp_getopt(argc, argv, MP_OPTSTR_DEFAULT"n", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        switch(c) {
            case 'n':
                nonroot = 1;
                break;
        }

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
}

/* vim: set ts=4 sw=4 et syn=c : */

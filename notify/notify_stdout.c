/***
 * Monitoring Plugin - notify_stdout.c
 **
 *
 * notify_stdout - Print a notification to stdout for debuging.
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

const char *progname  = "notify_stdout";
const char *progdesc  = "Print a notification to stdout for debuging.";
const char *progvers  = "0.1";
const char *progcopy  = "2012";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--file <TEMPLATE> | --message <MESSAGE>";

/* MP Includes */
#include "mp_notify.h"

/* Default Includes */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>
#include <error.h>
#include <string.h>

/* Global Vars */

int main (int argc, char **argv) {
    /* Local Vars */
    FILE *fd;
    char *out;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    if (mp_notify_file) {
        fd = fopen(mp_notify_file, "r");
        if (fd == NULL)
            critical("Can't open '%s'", mp_notify_file);
        out = mp_template(fd);
    } else {
        out = mp_template_str(mp_notify_msg);
    }

    printf("%s", out);
    return 0;
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_NOTIFY,
        MP_LONGOPTS_END
    };

    while (1) {
        c = mp_getopt(argc, argv, MP_OPTSTR_NOTIFY, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_notify(c);
    }

    /* Checks */
    if (!mp_notify_file && !mp_notify_msg)
        usage("--file or --message is mandatory.");

    return(OK);
}

void print_help(void) {
    print_revision();
    print_copyright();

    printf("\n");

    printf("Notify description: %s", progdesc);

    printf("\n\n");

    print_usage();

    print_help_notify();
}

/* vim: set ts=4 sw=4 et syn=c : */

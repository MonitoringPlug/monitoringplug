/***
 * Monitoring Plugin - check_nrped.c
 **
 *
 * check_nrped - Check if run inside of nrped.
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

const char *progname  = "check_nrped";
const char *progdesc  = "Check if run inside of nrped.";
const char *progvers  = "0.1";
const char *progcopy  = "2012";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[--touch <FILE>]";

/* MP Includes */
#include "mp_common.h"
/* Default Includes */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>

/* Global Vars */
char *filename = NULL;

int main (int argc, char **argv) {
    /* Local Vars */
    int fd;
    char *version;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    /* Read plugin version */
    version = getenv("NRPE_PROGRAMVERSION");

    if (version) {
        if (filename) {
            fd  = open(filename, O_WRONLY | O_CREAT | O_NONBLOCK | O_NOCTTY,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
            if (fd == -1)
                warning("NRPE v%s - open '%s' faild", version, filename);
            fd = close(fd);
            if (fd == -1)
                warning("NRPE v%s - close '%s' faild", version, filename);
            fd = utime(filename, NULL);
            if (fd == -1)
                warning("NRPE v%s - utime '%s' faild", version, filename);
        }
        ok("NRPE v%s", version);
    } else {
        critical("Executed outside of nrped.");
    }
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        {"touch", required_argument, NULL, (int)'T'},
        MP_LONGOPTS_END
    };

    while (1) {
        c = mp_getopt(argc, argv, MP_OPTSTR_DEFAULT"T:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        switch (c) {
            case 'T':
                filename = optarg;
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
    printf(" -T, --touch=filename\n");
    printf("      Touch file while run.\n");
}

/* vim: set ts=4 sw=4 et syn=c : */

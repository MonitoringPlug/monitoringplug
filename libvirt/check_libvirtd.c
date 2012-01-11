/***
 * Monitoring Plugin - check_libvirtd.c
 **
 *
 * check_libvirtd - Check if the libvirtd is responding.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id$
 */

const char *progname  = "check_libvirtd";
const char *progdesc  = "Check if the libvirtd is responding.";
const char *progvers  = "0.1";
const char *progcopy  = "2011";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[--connect qemu+ssh://kvm/system]";

/* MP Includes */
#include "mp_common.h"
#include "virt_utils.h"
/* Default Includes */
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Global Vars */

int main (int argc, char **argv) {
    /* Local Vars */
    virConnectPtr   conn;
    const char      *hvType;
    unsigned long libVer, libMajor, libMinor, libRelease;
    unsigned long hvVer, hvMajor, hvMinor, hvRelease;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    // PLUGIN CODE
    conn = virt_connect();

    hvType = virConnectGetType(conn);
    if (hvType == NULL) {
        if (mp_verbose > 0) {
            virt_showError(conn);
        }
        critical("Failed to get hypervisor type.");
    }

    if (virConnectGetVersion(conn, &hvVer) != 0) {
        if (mp_verbose > 0) {
            virt_showError(conn);
        }
        critical("Failed to get hypervisor version.");
    }

    if (virConnectGetLibVersion(conn, &libVer) != 0) {
        if (mp_verbose > 0) {
            virt_showError(conn);
        }
        critical("Failed to get library version.");
    }

    virConnectClose(conn);

    hvMajor = hvVer / 1000000;
    hvVer %= 1000000;
    hvMinor = hvVer / 1000;
    hvRelease = hvVer % 1000;

    libMajor = libVer / 1000000;
    libVer %= 1000000;
    libMinor = libVer / 1000;
    libRelease = libVer % 1000;

    /* Output and return */
    ok("libvirtd: v.%lu.%lu.%lu Hypervisor: %s (v.%lu.%lu.%lu)", 
            libMajor, libMinor, libRelease,
            hvType, hvMajor, hvMinor, hvRelease);
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
            MP_LONGOPTS_DEFAULT,
            VIRT_LONGOPTS,
            MP_LONGOPTS_TIMEOUT,
            MP_LONGOPTS_END
    };

    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"t:"VIRT_OPTSTR, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_virt(c);

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
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
    print_revision_virt();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();
    print_help_default();
    print_help_virt();
}

/* vim: set ts=4 sw=4 et syn=c : */

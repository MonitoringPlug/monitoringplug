/***
 * Monitoring Plugin - mp_getopt.c
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

#include "mp_getopt.h"
#include "mp_common.h"
#include "mp_notify.h"

#include <stdio.h>

int mp_getopt(int *argc, char **argv[], const char *optstring,
                const struct option *longopts, int *longindex) {
    int c;

    while (1) {
        c = getopt_long(*argc, *argv, optstring, longopts, NULL);

        if (c == -1 || c == EOF)
            return c;

        /* Handle default opts */
        switch (c) {
            case 'h':
                print_help();
                exit(0);
            case 'V':
                print_revision();
                exit(0);
            case 'v':
                mp_verbose++;
                break;
            case MP_LONGOPT_EOPT:
                *argv = mp_eopt(argc, *argv, optarg);
                break;
            case 't':
                mp_timeout = (int)strtol(optarg, NULL, 10);
                break;
            default:
                // Let the caller handle this option
                return c;
        };
    }

    return EOF;
}

/* vim: set ts=4 sw=4 et syn=c : */

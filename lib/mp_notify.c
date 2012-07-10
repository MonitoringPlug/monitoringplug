/***
 * Monitoring Plugin - mp_notify.c
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

#include "mp_notify.h"

#include <stdio.h>

/* Notify Global Vars */
const char *mp_notify_file = NULL;
const char *mp_notify_msg = NULL;

void getopt_notify(int c) {
    /* Handle default opts */
    switch (c) {
        case 'F':
            if (mp_notify_msg)
                usage("--file and --message are exclusive options.");
            mp_notify_file = optarg;
            break;
        case 'm':
            if (mp_notify_file)
                usage("--file and --message are exclusive options.");
            mp_notify_msg = optarg;
            break;
    };
}

/* vim: set ts=4 sw=4 et syn=c : */

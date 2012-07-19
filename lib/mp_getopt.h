/***
 * Monitoring Plugin - mp_getopt.h
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

#ifndef _MP_GETOPT_H_
#define _MP_GETOPT_H_

#include <getopt.h>

/** Longopt only defines */
#define MP_LONGOPT_EOPT         0x0080  //*< --eopt */
#define MP_LONGOPT_PERFDATA     0x0081  //*< --perfdata */
#define MP_LONGOPT_PRIV0        0x0090
#define MP_LONGOPT_PRIV1        0x0091
#define MP_LONGOPT_PRIV2        0x0092
#define MP_LONGOPT_PRIV3        0x0093
#define MP_LONGOPT_PRIV4        0x0094
#define MP_LONGOPT_PRIV5        0x0095
#define MP_LONGOPT_PRIV6        0x0096
#define MP_LONGOPT_PRIV7        0x0097

/**
 * Wrapper around getopt_long for check commands.
 */
int mp_getopt(int *argc, char **argv[], const char *optstring,
        const struct option *longopts, int *longindex);

#endif /* _MP_GETOPT_H_ */

/* vim: set ts=4 sw=4 et syn=c : */

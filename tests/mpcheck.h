/***
 * Monitoring Plugin Tests - mpcheck.h
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

#ifndef _MPCHECK_H_
#define _MPCHECK_H_

#include "mp_common.h"

#include <check.h>
#include <stdlib.h>
#include <stdio.h>

const char *progname  = "TEST";
const char *progvers  = "TEST";
const char *progcopy  = "TEST";
const char *progauth  = "TEST";
const char *progusage = "TEST";

typedef struct {
    char    *in;
    int     out;
} string_int;

typedef struct {
    char    *in;
    double  out;
} string_double;

typedef struct {
   char     *in;
   char     *out;
} string_string;

typedef struct {
    char    *in;
    double  test;
    int     out;
} string_double_int;

typedef struct {
    double  in;
    int     out;
} double_int;

typedef struct {
    char    *ina;
    char    *inb;
    double  test;
    int     out;
} string_string_double_int;

#endif /* _MPCHECK_H_ */

/* vim: set ts=4 sw=4 et syn=c : */

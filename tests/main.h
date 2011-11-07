/***
 * Monitoring Plugin Tests - main.c
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
 */

#ifndef _TESTS_MAIN_H
#define _TESTS_MAIN_H

#include "mp_common.h"

#include <check.h>

struct string_return {
    char    *string;
    double  returning;
};

/* LIB COMMON Suite */
Suite *make_lib_common_suite(void);

/* LIB ARGS Suite */
Suite *make_lib_args_suite(void);

/* LIB CHECK Suite */
Suite *make_lib_check_suite(void);

/* LIB POPEN Suite */
Suite *make_lib_popen_suite(void);

/* LIB EOPT Suite */
Suite *make_lib_eopt_suite(void);

/* LIB UTILS Suite */
Suite *make_lib_utils_suite(void);

#endif /* _TESTS_MAIN_H */

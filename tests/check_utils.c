/***
 * Monitoring Plugin - check_utils.c
 **
 *
 * check_utils - 
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
 * $Id$
 */


#include "main.h"

#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

START_TEST (test_sprintf_ok) {
    char *dest;

    dest = malloc(128);

    mp_sprintf(dest, "%s", "test");

    fail_unless (strcmp(dest, "test") == 0,
            "mp_sprintf faild: %s", dest);
}
END_TEST

START_TEST (test_snprintf_ok) {
    char *dest;

    dest = malloc(128);

    mp_snprintf(dest, 128, "%s", "test");

    fail_unless (strcmp(dest, "test") == 0,
            "mp_sprintf faild: %s", dest);
}
END_TEST

START_TEST (test_snprintf_long) {
    char *dest;

    dest = malloc(128);

    mp_snprintf(dest, 5, "%s", "longteststring");
}
END_TEST

Suite* make_lib_utils_suite(void) {

    Suite *s = suite_create ("Utils");

    /* sprintf test case */
    TCase *tc_sprintf = tcase_create ("sprintf");
    tcase_add_test(tc_sprintf, test_sprintf_ok);
    tcase_add_test(tc_sprintf, test_snprintf_ok);
    tcase_add_exit_test(tc_sprintf, test_snprintf_long, 2);
    suite_add_tcase (s, tc_sprintf);
    return s;
}

/* vim: set ts=4 sw=4 et syn=c : */

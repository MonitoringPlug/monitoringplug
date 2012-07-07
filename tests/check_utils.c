/***
 * Monitoring Plugin - check_utils.c
 **
 *
 * check_utils - 
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


#include "main.h"

#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// mp_sprintf
START_TEST (test_sprintf_ok) {
    char *dest;

    dest = malloc(128);

    mp_sprintf(dest, "%s", "test");

    fail_unless (strcmp(dest, "test") == 0,
            "mp_sprintf faild: %s", dest);
}
END_TEST

START_TEST (test_sprintf_fail) {
    char *dest;

    dest = malloc(128);

    mp_sprintf(dest, "%");
}
END_TEST

// mp_snprintf
START_TEST (test_snprintf_ok) {
    char *dest;

    dest = malloc(128);

    mp_snprintf(dest, 5, "%s", "test");

    fail_unless (strcmp(dest, "test") == 0,
            "mp_snprintf faild: %s", dest);
}
END_TEST

START_TEST (test_snprintf_long) {
    char *dest;

    dest = malloc(128);

    mp_snprintf(dest, 5, "%s", "longteststring");
}
END_TEST

// mp_asprintf
START_TEST (test_asprintf_ok) {
    char *dest;

    mp_asprintf(&dest, "%s", "test");

    fail_unless (strcmp(dest, "test") == 0,
            "mp_asprintf faild: %s", dest);

    free(dest);
}
END_TEST

// mp_malloc
START_TEST (test_malloc_ok) {
    char *dest;

    dest = mp_malloc(10);

    free(dest);
}
END_TEST

// mp_calloc
START_TEST (test_calloc_ok) {
    char *dest;

    dest = mp_calloc(10, 10);

    free(dest);
}
END_TEST

// mp_realloc
START_TEST (test_realloc_ok) {
    char *dest = NULL;

    dest = mp_realloc(dest, 10);

    dest = mp_realloc(dest, 20);

}
END_TEST

Suite* make_lib_utils_suite(void) {

    Suite *s = suite_create ("Utils");

    /* String test case */
    TCase *tc_string = tcase_create ("string");
    tcase_add_test(tc_string, test_sprintf_ok);
    tcase_add_exit_test(tc_string, test_sprintf_fail, 2);
    tcase_add_test(tc_string, test_snprintf_ok);
    tcase_add_exit_test(tc_string, test_snprintf_long, 2);
    tcase_add_test(tc_string, test_asprintf_ok);
    suite_add_tcase (s, tc_string);

    /* Memory test case */
    TCase *tc_mem = tcase_create ("memory");
    tcase_add_test(tc_mem, test_malloc_ok);
    tcase_add_test(tc_mem, test_calloc_ok);
    tcase_add_test(tc_mem, test_realloc_ok);
    suite_add_tcase (s, tc_mem);

    return s;
}

/* vim: set ts=4 sw=4 et syn=c : */

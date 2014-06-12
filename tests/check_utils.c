/***
 * Monitoring Plugin - check_utils.c
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


#include "main.h"

#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// mp_strcat
START_TEST (test_strcat) {
    char *dest = NULL;

    mp_strcat(&dest, "TEST");
    mp_strcat(&dest, "TEST");

    fail_unless (strcmp(dest, "TESTTEST") == 0,
            "mp_strcat failed: %s", dest);
}
END_TEST

// mp_strcat_space
START_TEST (test_strcat_space) {
    char *dest = NULL;

    mp_strcat_space(&dest, "TEST");
    mp_strcat_space(&dest, "TEST");

    fail_unless (strcmp(dest, "TEST TEST") == 0,
            "mp_strcat_space failed: %s", dest);
}
END_TEST

// mp_strcat_comma
START_TEST (test_strcat_comma) {
    char *dest = NULL;

    mp_strcat_comma(&dest, "TEST");
    mp_strcat_comma(&dest, "TEST");

    fail_unless (strcmp(dest, "TEST, TEST") == 0,
            "mp_strcat_comma failed: %s", dest);
}
END_TEST

// mp_strcmp
START_TEST (test_strcmp) {
    fail_unless (mp_strcmp("TEST", "TEST") == 0,
            "mp_strcmp failed: TEST <> TEST");
}
END_TEST

START_TEST (test_strcmp_diff) {
    fail_unless (mp_strcmp("TEST", "NALA") != 0,
            "mp_strcmp failed: TEST <> NALA");
}
END_TEST

START_TEST (test_strcmp_inverse) {
    fail_unless (mp_strcmp("!TEST", "TEST") == 1,
            "mp_strcmp failed: !TEST <> TEST");
}
END_TEST

START_TEST (test_strcmp_inverse_diff) {
    fail_unless (mp_strcmp("!TEST", "NALA") == 0,
            "mp_strcmp failed: !TEST <> NALA");
}
END_TEST

// mp_sprintf
START_TEST (test_sprintf_ok) {
    char *dest;

    dest = malloc(128);

    mp_sprintf(dest, "%s", "test");

    fail_unless (strcmp(dest, "test") == 0,
            "mp_sprintf failed: %s", dest);
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
            "mp_snprintf failed: %s", dest);
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
            "mp_asprintf failed: %s", dest);

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

// mp_array_push
START_TEST (test_array_push) {
    char **array = NULL;
    int num = 0;

    mp_array_push(&array, "I", &num);
    mp_array_push(&array, "LOVE", &num);
    mp_array_push(&array, "NALA", &num);

    fail_unless (num == 3, "mp_array_push failed: num = %d", num);

    fail_unless (strcmp(array[0], "I") == 0,
            "mp_array_push failed: Element 0 %s", array[0]);
    fail_unless (strcmp(array[1], "LOVE") == 0,
            "mp_array_push failed: Element 1 %s", array[1]);
    fail_unless (strcmp(array[2], "NALA") == 0,
            "mp_array_push failed: Element 2 %s", array[2]);
}
END_TEST

START_TEST (test_array_push_multi) {
    char *input = NULL;
    char **array = NULL;
    int num = 0;

    input = mp_strdup("I,LOVE,NALA");

    mp_array_push(&array, input, &num);

    fail_unless (num == 3, "mp_array_push failed: num = %d", num);

    fail_unless (strcmp(array[0], "I") == 0,
            "mp_array_push failed: Element 0 %s", array[0]);
    fail_unless (strcmp(array[1], "LOVE") == 0,
            "mp_array_push failed: Element 1 %s", array[1]);
    fail_unless (strcmp(array[2], "NALA") == 0,
            "mp_array_push failed: Element 2 %s", array[2]);
}
END_TEST

START_TEST (test_array_push_int) {
    int *array = NULL;
    int num = 0;

    mp_array_push_int(&array, "21", &num);
    mp_array_push_int(&array, "23", &num);
    mp_array_push_int(&array, "42", &num);

    fail_unless (num == 3, "mp_array_push_int failed: num = %d", num);

    fail_unless (array[0] == 21,
            "mp_array_push_int failed: Element 0 %d", array[0]);
    fail_unless (array[1] == 23,
            "mp_array_push_int failed: Element 1 %d", array[1]);
    fail_unless (array[2] == 42,
            "mp_array_push_int failed: Element 2 %d", array[2]);
}
END_TEST

// mp_human_size
START_TEST (test_human_size) {
    char *ret = NULL;

    ret = mp_human_size(1);
    fail_unless (strcmp(ret, "1.00 ") == 0,
            "mp_human_size failed: 1 => %s", ret);
    free(ret);

    ret = mp_human_size(2000);
    fail_unless (strcmp(ret, "1.95 KiB") == 0,
            "mp_human_size failed: 2000 => %s", ret);
    free(ret);

    ret = mp_human_size(2000000);
    fail_unless (strcmp(ret, "1.91 MiB") == 0,
            "mp_human_size failed: 2000000 => %s", ret);
    free(ret);

    ret = mp_human_size(2000000000);
    fail_unless (strcmp(ret, "1.86 GiB") == 0,
            "mp_human_size failed: 2000000000 => %s", ret);
    free(ret);

    ret = mp_human_size(2000000000000);
    fail_unless (strcmp(ret, "1.82 TiB") == 0,
            "mp_human_size failed: 2000000000000 => %s", ret);
    free(ret);

}
END_TEST

// mp_strmatch
START_TEST (test_strmatch) {

    fail_unless (mp_strmatch("NALA", "LOVE") == 0,
            "mp_strmatch failed NALA <> LOVE");

    fail_unless (mp_strmatch("NALA", "NALA") == 1,
            "mp_strmatch failed NALA <> NALA");

    fail_unless (mp_strmatch("I LOVE NALA", "I LOVE*") == 1,
            "mp_strmatch failed 'I LOVE NALA' <> 'I LOVE*'");
}
END_TEST

Suite* make_lib_utils_suite(void) {

    Suite *s = suite_create ("Utils");

    /* String test case */
    TCase *tc_string = tcase_create ("string");
    tcase_add_test(tc_string, test_strcat);
    tcase_add_test(tc_string, test_strcat_space);
    tcase_add_test(tc_string, test_strcat_comma);
    tcase_add_test(tc_string, test_strcmp);
    tcase_add_test(tc_string, test_strcmp_diff);
    tcase_add_test(tc_string, test_strcmp_inverse);
    tcase_add_test(tc_string, test_strcmp_inverse_diff);
    tcase_add_test(tc_string, test_sprintf_ok);
#ifndef OS_FREEBSD
    tcase_add_exit_test(tc_string, test_sprintf_fail, 2);
#endif
    tcase_add_test(tc_string, test_snprintf_ok);
    tcase_add_exit_test(tc_string, test_snprintf_long, 2);
    tcase_add_test(tc_string, test_asprintf_ok);
    tcase_add_test(tc_string, test_human_size);
    tcase_add_test(tc_string, test_strmatch);
    suite_add_tcase (s, tc_string);

    /* Memory test case */
    TCase *tc_mem = tcase_create ("memory");
    tcase_add_test(tc_mem, test_malloc_ok);
    tcase_add_test(tc_mem, test_calloc_ok);
    tcase_add_test(tc_mem, test_realloc_ok);
    suite_add_tcase (s, tc_mem);

    /* Array test case */
    TCase *tc_array = tcase_create ("array");
    tcase_add_test(tc_array, test_array_push);
    tcase_add_test(tc_array, test_array_push_multi);
    tcase_add_test(tc_array, test_array_push_int);
    suite_add_tcase (s, tc_array);

    return s;
}

/* vim: set ts=4 sw=4 et syn=c : */

/***
 * Monitoring Plugin - check_args.c
 **
 *
 * check_args - 
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

void perfdata_setup(void);
void perfdata_teardown(void);

void perfdata_setup(void) {
    mp_perfdata = NULL;
}

void perfdata_teardown(void) {
    if(mp_perfdata)
        free(mp_perfdata);
    mp_perfdata = NULL;
}

START_TEST (test_perfdata_int_none) {
    mp_showperfdata = 0;
    mp_perfdata_int("label", 42, "unit", NULL);

    fail_unless (mp_perfdata == NULL,
            "Perfdata is not none.");
}
END_TEST

START_TEST (test_perfdata_int) {
    mp_showperfdata = 1;
    mp_perfdata_int("label", 42, "unit", NULL);

    fail_unless (strcmp(mp_perfdata, "label=42unit;") == 0,
            "Wrong perfdata: '%s'", mp_perfdata);
}
END_TEST

START_TEST (test_perfdata_int_threshold) {
    thresholds *my_thresholds = NULL;

    mp_showperfdata = 1;
    setWarn(&my_thresholds, "10", 0);
    setCrit(&my_thresholds, "20", 0);

    mp_perfdata_int("label", 42, "unit", my_thresholds);

    fail_unless (strcmp(mp_perfdata, "label=42unit;10.000;20.000;") == 0,
            "Wrong perfdata: '%s'", mp_perfdata);
}
END_TEST

START_TEST (test_perfdata_int_threshold_range) {
    thresholds *my_thresholds = NULL;

    mp_showperfdata = 1;
    setWarn(&my_thresholds, "23:46", 0);
    setCrit(&my_thresholds, "21:63", 0);

    mp_perfdata_int("label", 42, "unit", my_thresholds);

    fail_unless (strcmp(mp_perfdata, "label=42unit;23.000:46.000;21.000:63.000;") == 0,
            "Wrong perfdata: '%s'", mp_perfdata);
}
END_TEST

// Float
START_TEST (test_perfdata_float_none) {
    mp_showperfdata = 0;
    mp_perfdata_float("label", 42.23, "unit", NULL);

    fail_unless (mp_perfdata == NULL,
            "Perfdata is not none.");
}
END_TEST

START_TEST (test_perfdata_float) {
    mp_showperfdata = 1;
    mp_perfdata_float("label", 42.23, "unit", NULL);

    fail_unless (strcmp(mp_perfdata, "label=42.230unit;") == 0,
            "Wrong perfdata: '%s'", mp_perfdata);
}
END_TEST

START_TEST (test_perfdata_float_threshold) {
    thresholds *my_thresholds = NULL;

    mp_showperfdata = 1;
    setWarn(&my_thresholds, "10.10", 0);
    setCrit(&my_thresholds, "20.20", 0);

    mp_perfdata_float("label", 42.23, "unit", my_thresholds);

    fail_unless (strcmp(mp_perfdata, "label=42.230unit;10.100;20.200;") == 0,
            "Wrong perfdata: '%s'", mp_perfdata);
}
END_TEST

START_TEST (test_perfdata_float_threshold_range) {
    thresholds *my_thresholds = NULL;

    mp_showperfdata = 1;
    setWarn(&my_thresholds, "23.23:46.46", 0);
    setCrit(&my_thresholds, "21.21:63.63", 0);

    mp_perfdata_float("label", 42.23, "unit", my_thresholds);

    fail_unless (strcmp(mp_perfdata, "label=42.230unit;23.230:46.460;21.210:63.630;") == 0,
            "Wrong perfdata: '%s'", mp_perfdata);
}
END_TEST


Suite* make_lib_perfdata_suite(void) {

    Suite *s = suite_create ("Perfdata");

    /* Int test case */
    TCase *tc_int = tcase_create("Int");
    tcase_add_checked_fixture(tc_int, perfdata_setup, perfdata_teardown);
    tcase_add_test(tc_int, test_perfdata_int_none);
    tcase_add_test(tc_int, test_perfdata_int);
    tcase_add_test(tc_int, test_perfdata_int_threshold);
    tcase_add_test(tc_int, test_perfdata_int_threshold_range);
    suite_add_tcase(s, tc_int);

    /* Floar test case */
    TCase *tc_float = tcase_create("Float");
    tcase_add_checked_fixture(tc_float, perfdata_setup, perfdata_teardown);
    tcase_add_test(tc_float, test_perfdata_float_none);
    tcase_add_test(tc_float, test_perfdata_float);
    tcase_add_test(tc_float, test_perfdata_float_threshold);
    tcase_add_test(tc_float, test_perfdata_float_threshold_range);
    suite_add_tcase(s, tc_float);

    return s;
}

/* vim: set ts=4 sw=4 et syn=c : */

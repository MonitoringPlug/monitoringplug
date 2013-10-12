/***
 * Monitoring Plugin - check_perfdata.c
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

START_TEST (test_perfdata_int_label) {
    mp_showperfdata = 1;
    mp_perfdata_int("l 1", 1, "", NULL);
    mp_perfdata_int("l=2", 2, "", NULL);
    mp_perfdata_int("l+3", 3, "", NULL);

    fail_unless (strcmp(mp_perfdata, "'l 1'=1; 'l=2'=2; 'l+3'=3;") == 0,
            "Wrong perfdata: '%s'", mp_perfdata);
}
END_TEST

START_TEST (test_perfdata_int_minmax) {
    mp_showperfdata = 1;
    mp_perfdata_int2("label", 23, "unit", NULL, 1, 21, 1, 42);

    fail_unless (strcmp(mp_perfdata, "label=23unit;;;21;42;") == 0,
            "Wrong perfdata: '%s'", mp_perfdata);
}
END_TEST

START_TEST (test_perfdata_int3) {
    mp_showperfdata = 1;
    mp_perfdata_int3("label", 42, "unit",
            1, 64, 1, 96, 0, 0, 0, 0);

    fail_unless (strcmp(mp_perfdata, "label=42unit;~:64.000;~:96.000;") == 0,
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

START_TEST (test_perfdata_float_label) {
    mp_showperfdata = 1;
    mp_perfdata_float("l 1", 1, "", NULL);
    mp_perfdata_float("l=2", 2, "", NULL);
    mp_perfdata_float("l+3", 3, "", NULL);

    fail_unless (strcmp(mp_perfdata, "'l 1'=1.000; 'l=2'=2.000; 'l+3'=3.000;") == 0,
            "Wrong perfdata: '%s'", mp_perfdata);
}
END_TEST

START_TEST (test_perfdata_float_minmax) {
    mp_showperfdata = 1;
    mp_perfdata_float2("label", 23, "unit", NULL, 1, 21, 1, 42);

    fail_unless (strcmp(mp_perfdata, "label=23.000unit;;;21.000;42.000;") == 0,
            "Wrong perfdata: '%s'", mp_perfdata);
}
END_TEST

START_TEST (test_perfdata_float3) {
    mp_showperfdata = 1;
    mp_perfdata_float3("label", 42, "unit",
            1, 64, 1, 96, 0, 0, 0, 0);

    fail_unless (strcmp(mp_perfdata, "label=42.000unit;~:64.000;~:96.000;") == 0,
            "Wrong perfdata: '%s'", mp_perfdata);
}
END_TEST

START_TEST (test_perfdata_float_precision) {
    mp_showperfdata = 1;
    mp_perfdata_float("label1", 0, "", NULL);
    mp_perfdata_float("label2", 10000, "", NULL);

    fail_unless (strcmp(mp_perfdata, "label1=0; label2=10000;") == 0,
            "Wrong perfdata: '%s'", mp_perfdata);
}
END_TEST

START_TEST (test_perfdata_percent) {
    thresholds *my_thresholds = NULL;

    mp_showperfdata = 1;
    setWarn(&my_thresholds, "20%:80%", 0);
    setCrit(&my_thresholds, "10%:90%", 0);

    mp_perfdata_int2("label", 50, "unit", my_thresholds, 1, 0, 1, 100);

    fail_unless (strcmp(mp_perfdata, "label=50unit;20.000:80.000;10.000:90.000;0;100;") == 0,
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
    tcase_add_test(tc_int, test_perfdata_int_label);
    tcase_add_test(tc_int, test_perfdata_int_minmax);
    tcase_add_test(tc_int, test_perfdata_int3);
    suite_add_tcase(s, tc_int);

    /* Float test case */
    TCase *tc_float = tcase_create("Float");
    tcase_add_checked_fixture(tc_float, perfdata_setup, perfdata_teardown);
    tcase_add_test(tc_float, test_perfdata_float_none);
    tcase_add_test(tc_float, test_perfdata_float);
    tcase_add_test(tc_float, test_perfdata_float_threshold);
    tcase_add_test(tc_float, test_perfdata_float_threshold_range);
    tcase_add_test(tc_float, test_perfdata_float_label);
    tcase_add_test(tc_float, test_perfdata_float_minmax);
    tcase_add_test(tc_float, test_perfdata_float3);
    tcase_add_test(tc_float, test_perfdata_float_precision);
    suite_add_tcase(s, tc_float);

    TCase *tc_percent = tcase_create("Percent");
    tcase_add_test(tc_percent, test_perfdata_percent);
    suite_add_tcase(s, tc_percent);

    return s;
}

/* vim: set ts=4 sw=4 et syn=c : */

/***
 * Monitoring Plugin - check_args.c
 **
 *
 * check_args - 
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

void range_setup(void);
void range_teardown(void);
void threshold_setup(void);
void threshold_teardown(void);

static struct string_return test_multi_case[] = {
    { "k",  1000 },                     { "K",  1024 },
    { "kB", 1000 },                     { "KB", 1024 },
    { "m",  1000000 },                  { "M",  1048578 },
    { "mB", 1000000 },                  { "MB", 1048578 },
    { "g",  1000000000 },               { "G",  1073741824 },
    { "gB", 1000000000 },               { "GB", 1073741824 },
    { "t",  1000000000000LL },          { "T",  1099511627776LL },
    { "tB", 1000000000000LL },          { "TB", 1099511627776LL },
    { "p",  1000000000000000LL },       { "P",  1125899906842624LL },
    { "pB", 1000000000000000LL },       { "PB", 1125899906842624LL },
    { "e",  1000000000000000000LL },    { "E",  1152921504606846976LL },
    { "eB", 1000000000000000000LL },    { "EB", 1152921504606846976LL },
    {0,0}
};

static struct string_return test_multi_time_case[] = {
    { "s", 1 },         { "sec",  1 },
    { "m", 60 },        { "min",  60 },
    { "h", 3600 },      { "hr",   3600 },
    { "d", 86400 },     { "day",  86400 },
    { "w", 604800 },    { "week", 604800 },
    {0,0}
};

range *my_range;
thresholds *my_thresholds;

void range_setup(void) {
  my_range = (range *) mp_malloc(sizeof(range));
}

void range_teardown(void) {
  free (my_range);
}

void threshold_setup(void) {
  my_thresholds = (thresholds *) mp_malloc(sizeof(thresholds));
  my_thresholds->warning = (range *) mp_malloc(sizeof(range));
  my_thresholds->critical = (range *) mp_malloc(sizeof(range));
}

void threshold_teardown(void) {
  free (my_thresholds->warning);
  free (my_thresholds->critical);
  free (my_thresholds);
}

START_TEST (test_multi) {
    struct string_return *c = &test_multi_case[_i];

    fail_unless (parse_multiplier_string(c->string) == c->returning,
        "Fail: parse_multiplier_string(%s) is not %0.f", c->string,c->returning);
}
END_TEST

START_TEST (test_multi_time) {
    struct string_return *c = &test_multi_time_case[_i];

    fail_unless (parse_time_multiplier_string(c->string) == c->returning,
        "Fail: parse_time_multiplier_string(%s) is not %0.f", c->string,c->returning);
}
END_TEST

START_TEST (test_range_simple) {
    fail_unless (parse_range_string(my_range, "10", 0) == 0,
        "Parse range string '10' faild");

    double i;
    for (i = -10; i < 20; i++) {
        fail_unless (check_range(i, my_range) == (i<0 || i>10),
	       "Range check for %g in '10' faild.", i);
    }
}
END_TEST

START_TEST (test_range_toinf) {
  fail_unless (parse_range_string(my_range, "10:", 0) == 0,
	       "Parse range string '10:' faild");

    double i;
    for (i = -10; i < 20; i++) {
        fail_unless (check_range(i, my_range) == (i<10),
	       "Range check for %g in '10:' faild.", i);
    }
}
END_TEST

START_TEST (test_range_frominf) {
  fail_unless (parse_range_string(my_range, "~:10", 0) == 0,
	       "Parse range string '~:10' faild");

    double i;
    for (i = -10; i < 20; i++) {
        fail_unless (check_range(i, my_range) == (i>10),
	       "Range check for %g in '~:10' faild.", i);
    }		
}
END_TEST

START_TEST (test_range_fromto) {
  fail_unless (parse_range_string(my_range, "10:20", 0) == 0,
	       "Parse range string '10:20' faild");

    double i;
    for (i = -10; i < 30; i++) {
        fail_unless (check_range(i, my_range) == (i<10 || i>20),
	       "Range check for %g in '10:20' faild.", i);
    }
}
END_TEST

START_TEST (test_range_fromto_out) {
  fail_unless (parse_range_string(my_range, "@10:20", 0) == 0,
	       "Parse range string '@10:20' faild");

    double i;
    for (i = -10; i < 30; i++) {
        fail_unless (check_range(i, my_range) == (i>=10 && i<=20),
	       "Range check for %g in '@10:20' faild.", i);
    }
}
END_TEST


START_TEST (test_threshold_simple) {
    fail_unless (setWarn(&my_thresholds, "10", 0) == 0,
        "Parse range string '10' faild");
    fail_unless (setCrit(&my_thresholds, "20", 0) == 0,
        "Parse range string '10' faild");

    double i;
    for (i = -10; i < 20; i++) {
        fail_unless (get_status(i, my_thresholds) == 2-(i>=0 && i<=10)-(i>=0 && i<=20),
	       "Threshold check for %g in w'10' c'20' faild.", i);
    }
}
END_TEST


Suite* make_lib_args_suite(void) {

    Suite *s = suite_create ("Args");

    /* Range test case */
    TCase *tc_multi = tcase_create ("Multiplier");
    tcase_add_loop_test(tc_multi, test_multi, 0, 24);
    tcase_add_loop_test(tc_multi, test_multi_time, 0, 10);
    suite_add_tcase(s, tc_multi);

    /* Range test case */
    TCase *tc_range = tcase_create ("Range");
    tcase_add_checked_fixture (tc_range, range_setup, range_teardown);
    tcase_add_test (tc_range, test_range_simple);
    tcase_add_test (tc_range, test_range_toinf);
    tcase_add_test (tc_range, test_range_frominf);
    tcase_add_test (tc_range, test_range_fromto);
    tcase_add_test (tc_range, test_range_fromto_out);
    suite_add_tcase (s, tc_range);

    /* Threshold test case */
    TCase *tc_threshold = tcase_create ("Threshold");
    tcase_add_checked_fixture (tc_threshold, threshold_setup, threshold_teardown);
    tcase_add_test (tc_threshold, test_threshold_simple);
    suite_add_tcase (s, tc_threshold);

    return s;
}

/* vim: set ts=4 sw=4 et syn=c : */

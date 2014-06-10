/***
 * Monitoring Plugin - check_args.c
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

#include "mpcheck.h"

void range_setup(void);
void range_teardown(void);
void threshold_setup(void);
void threshold_teardown(void);

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


static string_double test_multi_case[] = {
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

START_TEST (test_multi) {
    string_double *c = &test_multi_case[_i];

    fail_unless (parse_multiplier_string(c->in) == c->out,
        "Fail: parse_multiplier_string(%s) is not %0.f", c->in,c->out);
}
END_TEST


static string_double test_multi_time_case[] = {
    { "s", 1 },         { "sec",  1 },
    { "m", 60 },        { "min",  60 },
    { "h", 3600 },      { "hr",   3600 },
    { "d", 86400 },     { "day",  86400 },
    { "w", 604800 },    { "week", 604800 },
    {0,0}
};

START_TEST (test_multi_time) {
    string_double *c = &test_multi_time_case[_i];

    fail_unless (parse_time_multiplier_string(c->in) == c->out,
        "Fail: parse_time_multiplier_string(%s) is not %0.f", c->in,c->out);
}
END_TEST

static string_double_int test_range_case[] = {
    {"10", 0, 0}, {"10", 1, 0}, {"10", 10, 0}, {"10", 11, 1},
    {"~:10", -1, 0}, {"~:10", 0, 0}, {"~:10", 10, 0}, {"~:10", 11, 1},
    {"0:10", -1, 1}, {"0:10", 0, 0}, {"0:10", 10, 0}, {"0:10", 11, 1},
    {"10:", 9, 1}, {"10:", 10, 0},
    {"10:20", 9, 1}, {"10:20", 10, 0}, {"10:20", 20, 0}, {"10:20", 21, 1},
    /* AT*/
    {"@10", 0, 1}, {"@10", 1, 1}, {"@10", 10, 1}, {"@10", 11, 0},
    {"@~:10", -1, 1}, {"@~:10", 0, 1}, {"@~:10", 10, 1}, {"@~:10", 11, 0},
    {"@0:10", -1, 0}, {"@0:10", 0, 1}, {"@0:10", 10, 1}, {"@0:10", 11, 0},
    {"@10:", 9, 0}, {"@10:", 10, 1},
    {"@10:20", 9, 0}, {"@10:20", 10, 1}, {"@10:20", 20, 1}, {"@10:20", 21, 0},
    /* Percent */
    {"10%", 0, 0}, {"10%", 0.01, 0}, {"10%", 0.1, 0}, {"10%", 0.11, 1},
    {"0:10%", -1, 1}, {"0:10%", 0, 0}, {"0:10%", 0.1, 0}, {"0:10%", 0.11, 1},
    {"10%:", 0.09, 1}, {"10%:", 0.1, 0},
};

START_TEST (test_range) {
    string_double_int *c = &test_range_case[_i];

    fail_unless (parse_range_string(my_range, c->in, 0) == 0,
        "Parse range string '%s' failed", c->in);

    fail_unless(check_range(c->test, my_range) == c->out,
            "Range check for %g in '%s' failed.", c->test, c->in);
}
END_TEST

static string_string_double_int test_threshold_case[] = {
    {"10",      "20",       10,     0}, {"10",      "20",       11,     1},
    {"10",      "20",       20,     1}, {"10",      "20",       21,     2},
    {"10:20",   "5:25",     1,      2}, {"10:20",   "5:25",     5,      1},
    {"10:20",   "5:25",     6,      1}, {"10:20",   "5:25",     10,     0},
    {"10:20",   "5:25",     20,     0}, {"10:20",   "5:25",     21,     1},
    {"10:20",   "5:25",     25,     1}, {"10:20",   "5:25",     26,     2},
// Cases for _at */
    {"20",      "10",       21,     0}, {"20",      "10",       20,     1},
    {"20",      "10",       11,     1}, {"20",      "10",       10,     2},
};

START_TEST (test_threshold) {
    string_string_double_int *c = &test_threshold_case[_i];

    fail_unless (mp_threshold_set_warning(&my_thresholds, c->ina, 0) == 0,
        "Parse range string '%s' failed", c->ina);
    fail_unless (mp_threshold_set_critical(&my_thresholds, c->inb, 0) == 0,
        "Parse range string '%s' failed", c->inb);

    fail_unless(get_status(c->test, my_thresholds) == c->out,
            "Threshold check for %g in w'%s' c'%s' failed.",
            c->test, c->ina, c->inb);
}
END_TEST

START_TEST (test_getopt_wc) {
    string_string_double_int *c = &test_threshold_case[_i];

    getopt_wc('w', c->ina, &my_thresholds);
    getopt_wc('c', c->inb, &my_thresholds);

    fail_unless(get_status(c->test, my_thresholds) == c->out,
            "Threshold check for %g in w'%s' c'%s' failed. => %d",
            c->test, c->ina, c->inb, get_status(c->test, my_thresholds));
}
END_TEST

START_TEST (test_getopt_wc_at) {
    string_string_double_int *c = &test_threshold_case[_i];

    getopt_wc_at('w', c->ina, &my_thresholds);
    getopt_wc_at('c', c->inb, &my_thresholds);

    fail_unless(get_status(c->test, my_thresholds) == c->out,
            "Threshold check for %g in w'%s' c'%s' failed. => %d",
            c->test, c->ina, c->inb, get_status(c->test, my_thresholds));
}
END_TEST

int main (void) {
    int number_failed;
    SRunner *sr;

    Suite *s = suite_create ("Args");

    /* Range test case */
    TCase *tc_multi = tcase_create ("Multiplier");
    tcase_add_loop_test(tc_multi, test_multi, 0, 24);
    tcase_add_loop_test(tc_multi, test_multi_time, 0, 10);
    suite_add_tcase(s, tc_multi);

    /* Range test case */
    TCase *tc_range = tcase_create ("Range");
    tcase_add_checked_fixture (tc_range, range_setup, range_teardown);
    tcase_add_loop_test(tc_range, test_range, 0, 46);
    suite_add_tcase (s, tc_range);

    /* Threshold test case */
    TCase *tc_threshold = tcase_create ("Threshold");
    tcase_add_checked_fixture (tc_threshold, threshold_setup, threshold_teardown);
    tcase_add_loop_test(tc_threshold, test_threshold, 0, 12);
    tcase_add_loop_test(tc_threshold, test_getopt_wc, 0, 12);
    tcase_add_loop_test(tc_threshold, test_getopt_wc_at, 4, 16);
    suite_add_tcase (s, tc_threshold);

    sr = srunner_create(s);
    srunner_run_all(sr, CK_VERBOSE);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* vim: set ts=4 sw=4 et syn=c : */

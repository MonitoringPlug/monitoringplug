/***
 * Monitoring Plugin - check_common.c
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

#include "mp_common.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <check.h>

#include "main.h"


void exit_setup(void);
void exit_teardown(void);

void exit_setup(void) {
  close(1);
  close(2);
}

void exit_teardown(void) {
}

START_TEST (test_exit_ok) {
    ok("TEST OK");
}
END_TEST

START_TEST (test_exit_ok_perf) {
    mp_showperfdata = 1;
    mp_perfdata_int("PERF", (long int)0, "", NULL);
    ok("TEST OK");
}
END_TEST

START_TEST (test_exit_warn) {
    warning("TEST WARNING");
}
END_TEST

START_TEST (test_exit_warn_perf) {
    mp_showperfdata = 1;
    mp_perfdata_int("PERF", (long int)0, "", NULL);
    warning("TEST WARNING");
}
END_TEST

START_TEST (test_exit_critical) {
    critical("TEST CRITICAL");
}
END_TEST

START_TEST (test_exit_critical_perf) {
    mp_showperfdata = 1;
    mp_perfdata_int("PERF", (long int)0, "", NULL);
    critical("TEST CRITICAL");
}
END_TEST

START_TEST (test_exit_unknown) {
    unknown("TEST UNKNOWN");
}
END_TEST

START_TEST (test_exit_unknown_perf) {
    mp_showperfdata = 1;
    mp_perfdata_int("PERF", (long int)0, "", NULL);
    unknown("TEST UNKNOWN");
}
END_TEST

START_TEST (test_exit_usage) {
    usage("");
}
END_TEST

START_TEST (test_exit_timeout) {
    signal(SIGALRM, timeout_alarm_handler);
    alarm(1);
    sleep(2);
}
END_TEST

START_TEST (test_exit_noneroot) {
    if (geteuid() == 0)
      setreuid(1, 1);
    mp_noneroot_die();
}
END_TEST

START_TEST (test_print_revision) {
    print_revision();
}
END_TEST

START_TEST (test_print_copyright) {
    print_copyright();
}
END_TEST

Suite* make_lib_common_suite(void) {

    Suite *s = suite_create ("Common");

    /* Range test case */
    TCase *tc_exit = tcase_create ("Exit");
    tcase_add_checked_fixture (tc_exit, exit_setup, exit_teardown);
    tcase_add_exit_test(tc_exit, test_exit_ok, 0);
    tcase_add_exit_test(tc_exit, test_exit_ok_perf, 0);
    tcase_add_exit_test(tc_exit, test_exit_warn, 1);
    tcase_add_exit_test(tc_exit, test_exit_warn_perf, 1);
    tcase_add_exit_test(tc_exit, test_exit_critical, 2);
    tcase_add_exit_test(tc_exit, test_exit_critical_perf, 2);
    tcase_add_exit_test(tc_exit, test_exit_unknown, 3);
    tcase_add_exit_test(tc_exit, test_exit_unknown_perf, 3);
    tcase_add_exit_test(tc_exit, test_exit_usage, 3);
    tcase_add_exit_test(tc_exit, test_exit_timeout, 2);
    tcase_add_exit_test(tc_exit, test_exit_noneroot, 3);
    suite_add_tcase (s, tc_exit);
    TCase *tc_print = tcase_create("Print");
    tcase_add_test(tc_print, test_print_revision);
    tcase_add_test(tc_print, test_print_copyright);
    suite_add_tcase (s, tc_print);
    return s;
}

/* vim: set ts=4 sw=4 et syn=c : */

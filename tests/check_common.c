/***
 * Monitoring Plugin - check_common.c
 **
 *
 * check_common - 
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

#include "mp_common.h"
#include <stdlib.h>
#include <unistd.h>
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

START_TEST (test_exit_warning) {
    warning("TEST WARNING");
}
END_TEST

START_TEST (test_exit_critical) {
    critical("TEST CRITICAL");
}
END_TEST

START_TEST (test_exit_unknown) {
    unknown("TEST UNKNOWN");
}
END_TEST

Suite* make_lib_common_suite(void) {

    Suite *s = suite_create ("Common");

    /* Range test case */
    TCase *tc_exit = tcase_create ("Exit");
    tcase_add_checked_fixture (tc_exit, exit_setup, exit_teardown);
    tcase_add_exit_test(tc_exit, test_exit_ok, 0);
    tcase_add_exit_test(tc_exit, test_exit_warning, 1);
    tcase_add_exit_test(tc_exit, test_exit_critical, 2);
    tcase_add_exit_test(tc_exit, test_exit_unknown, 3);
    suite_add_tcase (s, tc_exit);
    return s;
}

/* vim: set ts=4 sw=4 et syn=c : */

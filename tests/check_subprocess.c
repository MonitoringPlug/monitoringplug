/***
 * Monitoring Plugin - check_subprocess.c
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
#include "mp_subprocess.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>
#include <unistd.h>
#include <signal.h>

const char *progname  = "TEST";
const char *progvers  = "TEST";
const char *progcopy  = "TEST";
const char *progauth  = "TEST";
const char *progusage = "TEST";

START_TEST (test_subprocess_echo) {
    mp_subprocess_t *sph;
    char *cmd[] = { "/bin/echo", "-n", "TEST", (char *)0 };
    char *buffer;
    size_t s;

    buffer = mp_malloc(10);
    memset(buffer, 0, 10);

    sph = mp_subprocess(cmd);
    fail_if(sph == NULL, "subprocess '/bin/echo', '-n', 'TEST' failed!");

    s = read(sph->stdout, buffer, 9);
    fail_unless(s == 4, "subprocess stdout fread failed! %d=>'%s'", s, buffer);

    fail_unless(mp_subprocess_close(sph) == 0, "subprocess close failed!");

    fail_unless(strcmp(cmd[2], buffer) == 0, "subprocess failed! '%s' <> '%s'", cmd[2], buffer);
}
END_TEST

START_TEST (test_subprocess_cat) {
    mp_subprocess_t *sph;
    char *cmd[] = { "/bin/cat", "-", (char *)0 };
    char *buffer;
    size_t s;

    buffer = mp_malloc(10);
    memset(buffer, 0, 10);

    sph = mp_subprocess(cmd);
    fail_if(sph == NULL, "subprocess '/bin/cat', '-' failed!");

    write(sph->stdin, "TEST", 4);

    s = read(sph->stdout, buffer, 9);
    fail_unless(s == 4, "subprocess stdout fread failed! %d=>'%s'", s, buffer);

    kill(sph->pid, SIGHUP);

    fail_unless(mp_subprocess_close(sph) == -1, "subprocess close failed!");

    fail_unless(strcmp("TEST", buffer) == 0, "subprocess failed! 'TEST' <> '%s'", buffer);
}
END_TEST

START_TEST (test_subprocess_false) {
    mp_subprocess_t *sph;
    char *cmd[] = { BIN_FALSE, (char *)0 };

    sph = mp_subprocess(cmd);
    fail_if(sph == NULL, "subprocess " BIN_FALSE " failed!");

    fail_unless(mp_subprocess_close(sph) == 1, "subprocess close failed!");
}
END_TEST

START_TEST (test_subprocess_dir) {
    mp_subprocess_t *sph;
    char *cmd[] = { "/usr/bin/", (char *)0 };

    sph = mp_subprocess(cmd);
    fail_unless(sph == NULL, "subprocess '/usr/bin/' failed!");
}
END_TEST

START_TEST (test_subprocess_dev) {
    mp_subprocess_t *sph;
    char *cmd[] = { "/dev/console", (char *)0 };

    sph = mp_subprocess(cmd);
    fail_unless(sph == NULL, "subprocess '/dev/console' failed!");
}
END_TEST

START_TEST (test_subprocess_nonexist) {
    mp_subprocess_t *sph;
    char *cmd[] = { "/file/dont/exist", (char *)0 };

    sph = mp_subprocess(cmd);
    fail_unless(sph == NULL, "subprocess '/file/dont/exist' failed!");
}
END_TEST

START_TEST (test_subprocess_nonexe) {
    mp_subprocess_t *sph;
    char *cmd[] = { "/etc/hosts", (char *)0 };

    sph = mp_subprocess(cmd);
    fail_unless(sph == NULL, "subprocess '/etc/hosts' failed!");
}
END_TEST

START_TEST (test_subprocess_nonaccess) {
    mp_subprocess_t *sph;
    char *cmd[] = { "/etc/sudoers", (char *)0 };

    sph = mp_subprocess(cmd);
    fail_unless(sph == NULL, "subprocess '/etc/sudoers' failed!");
}
END_TEST

int main (void) {

  int number_failed;
  SRunner *sr;

  Suite *s = suite_create ("Subprocess");

  /* String test case */
  TCase *tc = tcase_create ("subprocess");
  tcase_add_test(tc, test_subprocess_echo);
  tcase_add_test(tc, test_subprocess_cat);
  tcase_add_test(tc, test_subprocess_false);
  tcase_add_test(tc, test_subprocess_dir);
  tcase_add_test(tc, test_subprocess_dev);
  tcase_add_test(tc, test_subprocess_nonexist);
  tcase_add_test(tc, test_subprocess_nonexe);
  tcase_add_test(tc, test_subprocess_nonaccess);
  suite_add_tcase (s, tc);

  sr = srunner_create(s);
  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* vim: set ts=4 sw=4 et syn=c : */

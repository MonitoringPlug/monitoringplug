/***
 * monitoringplug - check_popen.c
 **
 *
 * Copyright (C) 2010 Marius Rieder <marius.rieder@durchmesser.ch>
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
#include "mp_popen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>
#include <unistd.h>

#include "main.h"

START_TEST (test_popen_echo) {
    FILE *fh;
    char *cmd[] = { "/bin/echo", "-n", "TEST", (char *)0 };
    char *buffer;
    size_t s;

    buffer = malloc(10);
    memset(buffer, 0, 10);

    fh = mp_popen(cmd);
    fail_if(fh == NULL, "Popen '/bin/echo', '-n', 'TEST' faild!");


    s = fread(buffer, 1, 10, fh);
    fail_unless(s == 4, "Popen fread faild! %d=>'%s'", s, buffer);

    fail_unless(fclose(fh) == 0, "Popen fclose faild!");

    fail_unless(strcmp(cmd[2], buffer) == 0, "Popen2 faild! '%s' <> '%s'", cmd[2], buffer);
}
END_TEST

START_TEST (test_popen_false) {
    FILE *fh;
    char *cmd[] = { "/bin/false", (char *)0 };

    fh = mp_popen(cmd);
    fail_if(fh == NULL, "Popen '/bin/false' faild!");

    fail_unless(mp_pclose(fh) == 1, "Popen pclose faild!");
}
END_TEST

START_TEST (test_popen_dir) {
    FILE *fh;
    char *cmd[] = { "/usr/bin/", (char *)0 };

    fh = mp_popen(cmd);
    fail_unless(fh == NULL, "Popen '/usr/bin/' faild!");
}
END_TEST

START_TEST (test_popen_dev) {
    FILE *fh;
    char *cmd[] = { "/dev/console", (char *)0 };

    fh = mp_popen(cmd);
    fail_unless(fh == NULL, "Popen '/dev/console' faild!");
}
END_TEST

START_TEST (test_popen_nonexist) {
    FILE *fh;
    char *cmd[] = { "/file/dont/exist", (char *)0 };

    fh = mp_popen(cmd);
    fail_unless(fh == NULL, "Popen '/file/dont/exist' faild!");
}
END_TEST

START_TEST (test_popen_nonexe) {
    FILE *fh;
    char *cmd[] = { "/etc/hosts", (char *)0 };

    fh = mp_popen(cmd);
    fail_unless(fh == NULL, "Popen '/etc/hosts' faild!");
}
END_TEST

START_TEST (test_popen_nonaccess) {
    FILE *fh;
    char *cmd[] = { "/etc/sudoers", (char *)0 };

    fh = mp_popen(cmd);
    fail_unless(fh == NULL, "Popen '/etc/sudoers' faild!");
}
END_TEST

Suite* make_lib_popen_suite(void) {

    Suite *s = suite_create ("Popen");

    /* Range test case */
    TCase *tc_exit = tcase_create ("Popen");
    tcase_add_test(tc_exit, test_popen_echo);
    tcase_add_test(tc_exit, test_popen_false);
    tcase_add_test(tc_exit, test_popen_dir);
    tcase_add_test(tc_exit, test_popen_dev);
    tcase_add_test(tc_exit, test_popen_nonexist);
    tcase_add_test(tc_exit, test_popen_nonexe);
    tcase_add_test(tc_exit, test_popen_nonaccess);
    suite_add_tcase (s, tc_exit);
    return s;
}

/* EOF */

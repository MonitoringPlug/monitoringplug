/***
 * Monitoring Plugin Tests - check_eopt.c
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
 */
 
#include "main.h"

#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "mp_eopt.h"


START_TEST (test_eopt_file) {
    char *argv[] = {"test", "--eopt", "@eopt.ini","--last", 0};
    char **new_argv;
    char *res_argv[] = {"test", "--eopt", "@eopt.ini","-a","b","-c","d","-e",
        "--last", 0};
    int args = 4;
    int res_args = 9;
    int i = 0;
    
    optind = 2;
    
    new_argv = mp_eopt(&args, argv, NULL);
    
    fail_unless (args == res_args, "Wrong arg count. %d %d",args, res_args);
    
    for(i=0; i < args; i++) {
        fail_unless (strcmp(new_argv[i], res_argv[i]) == 0,
        "Wrong arg at index: %d", i);
    }
}
END_TEST

START_TEST (test_eopt_section_file) {
    char *argv[] = {"test", "--eopt", "sectionB@eopt.ini","--last", 0};
    char **new_argv;
    char *res_argv[] = {"test", "--eopt", "sectionB@eopt.ini","--abc","def","--ghi",
        "jkl","--mno","--last", 0};
    int args = 4;
    int res_args = 9;
    int i = 0;
    
    optind = 2;
    
    new_argv = mp_eopt(&args, argv, NULL);
    
    fail_unless (args == res_args, "Wrong arg count. %d %d",args, res_args);
    
    for(i=0; i < args; i++) {
        fail_unless (strcmp(new_argv[i], res_argv[i]) == 0,
        "Wrong arg at index: %d", i);
    }
}
END_TEST


Suite* make_lib_eopt_suite(void) {
    
    Suite *s = suite_create ("EOpt");
    
    /* Range test case */
    TCase *tc_eopt = tcase_create ("EOpt");
    tcase_add_test (tc_eopt, test_eopt_file);
    tcase_add_test (tc_eopt, test_eopt_section_file);
    
    suite_add_tcase (s, tc_eopt);
    
    return s;
}

/* EOF */

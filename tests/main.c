/***
 * Monitoring Plugin - main.c
 **
 *
 * main - 
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
#include <check.h>

#include "main.h"

const char *progname  = "TEST";
const char *progvers  = "TEST";
const char *progcopy  = "TEST";
const char *progauth  = "TEST";
const char *progusage = "TEST";

int main (void) {

  int number_failed;
  SRunner *sr;

  sr = srunner_create( make_lib_common_suite() );
  srunner_add_suite(sr, make_lib_args_suite() );
  srunner_add_suite(sr, make_lib_check_suite() );
  srunner_add_suite(sr, make_lib_popen_suite() );
  srunner_add_suite(sr, make_lib_eopt_suite() );
  srunner_add_suite(sr, make_lib_utils_suite() );
  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

void print_help(void) {
}

/* vim: set ts=4 sw=4 et syn=c : */

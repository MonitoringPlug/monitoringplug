/***
 * Monitoring Plugin - check_rhcs.c
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
#include "rhcs_utils.h"

#include <stdlib.h>
#include <check.h>

const char *progname  = "TEST";
const char *progvers  = "TEST";
const char *progcopy  = "TEST";
const char *progauth  = "TEST";
const char *progusage = "TEST";

const char *clustat_dict[] = {
    "testdata/rhcs_clustat.xml.1",
    "testdata/rhcs_clustat.xml.2",
};

const char *clusterconf_dict[] = {
    "testdata/rhcs_cluster.conf.1",
    "testdata/rhcs_cluster.conf.2",
};

START_TEST (test_parse_rhcs_clustat) {
    rhcs_clustat *clustat;
    const char *filename;
    FILE *in;

    mp_verbose = 10;

    filename = clustat_dict[_i];

    in = fopen(filename, "r");

    clustat = parse_rhcs_clustat(in);

    fail_if(clustat == NULL,
            "parse_rhcs_clustat failed for '%s'", filename);
}
END_TEST

START_TEST (test_parse_rhcs_conf) {
    rhcs_conf *conf;
    const char *filename;
    FILE *in;

    filename = clusterconf_dict[_i];

    in = fopen(filename, "r");

    conf = parse_rhcs_conf(in);

    fail_if(conf == NULL,
            "parse_rhcs_conf failed for '%s'", filename);
}
END_TEST

int main (void) {

  int number_failed;
  SRunner *sr;

  Suite *s = suite_create ("RHCS");

  TCase *tc = tcase_create ("Clustat");
  tcase_add_loop_test(tc, test_parse_rhcs_clustat, 0, 2);
  suite_add_tcase (s, tc);

  tc = tcase_create ("Clusterconf");
  tcase_add_loop_test(tc, test_parse_rhcs_conf, 0, 2);
  suite_add_tcase (s, tc);

  sr = srunner_create(s);
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

//void print_help(void) {
//}

/* vim: set ts=4 sw=4 et syn=c : */

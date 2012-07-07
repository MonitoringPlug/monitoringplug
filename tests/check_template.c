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
#include "mp_template.h"

#include <stdlib.h>
#include <check.h>

const char *progname  = "TEST";
const char *progvers  = "TEST";
const char *progcopy  = "TEST";
const char *progauth  = "TEST";
const char *progusage = "TEST";

typedef struct {
    char *in;
    char *out;
} string_string;

static string_string dict_template[] = {
    /* Comments */
    {"foo[%# COMMENT %]bar", "foobar"},
    {"foo[% # COMMENT \n%]bar", "foobar"},
    {"foo[% 'bar' -%]\n", "foobar"},
    {"foo[% 'bar' -%]", "foobar"},
    {"foo\n[%- 'bar' %]", "foobar"},
    {"foo[%- 'bar' %]", "foobar"},
    /* IF */
    {"foo[% IF 1 %]bar[% END %]", "foobar"},
    {"foo[% IF 1 %]bar[% ELSE %]for[% END %]", "foobar"},
    {"foo[% IF 0 %]foo[% ELSE %]bar[% END %]", "foobar"},
    {"foo[% UNLESS 0 %]bar[% END %]", "foobar"},
    {"foo[% UNLESS 0 %]bar[% ELSE %]for[% END %]", "foobar"},
    {"foo[% UNLESS 1 %]foo[% ELSE %]bar[% END %]", "foobar"},
    {"[% IF 1 %]foo[% ELSE %]bar[% END %]bar", "foobar"},
    {"f[% IF 1 %]o[% IF 1 %]ob[% ELSE %]v[% END %]a[% ELSE %]w[% IF 1 %]x[% ELSE %]y[% END %]y[% END %]r", "foobar"},
    /* Switch */
    {"foo[% SWITCH 1 %][% CASE 1 %]bar[% END %]", "foobar"},
    {"foo[% SWITCH 1 %][% CASE 2 %]foo[% CASE 1 %]bar[% END %]", "foobar"},
    /* Bool */
    {"foo[% IF 'test' %]bar[% END %]", "foobar"},
    {"foo[% UNLESS '' %]bar[% END %]", "foobar"},
    {"foo[% IF 'test' == 'test' %]bar[% END %]", "foobar"},
    {"foo[% UNLESS 'test' == 'test2' %]bar[% END %]", "foobar"},
    {"foo[% IF 1 < 2 %]bar[% END %]", "foobar"},
    {"foo[% IF 1 <= 1  %]bar[% END %]", "foobar"},
    {"foo[% IF 3 > 2 %]bar[% END %]", "foobar"},
    {"foo[% IF 2 >= 2 %]bar[% END %]", "foobar"},
    {"foo[% IF 1.1 < 2 %]bar[% END %]", "foobar"},
    {"foo[% IF 1.0 <= 1  %]bar[% END %]", "foobar"},
    {"foo[% IF 3.3 > 2 %]bar[% END %]", "foobar"},
    {"foo[% IF 2.2 >= 2 %]bar[% END %]", "foobar"},
    /* Int */
    {"[% 3 %]", "3"},
    {"[% 4 + 2 %]", "6"}, {"[% 4 - 2 %]", "2"},
    {"[% 4 * 2 %]", "8"}, {"[% 4 / 2 %]", "2"},
    {"[% 1 + 2 * 3 %]", "7"},
    {"[% 1 * 2 + 3 %]", "5"},
    {"[% 1 + (2 * 3) %]", "7"},
    {"[% (1 + 2) * 3 %]", "9"},
    {"[% 1+2 %]", "3"},
    /* Float */
    {"[% 3.5 %]", "3.500000"},
    {"[% 4.2 + 2 %]", "6.200000"}, {"[% 4.2 - 2 %]", "2.200000"},
    {"[% 4.2 * 2 %]", "8.400000"}, {"[% 4.2 / 2 %]", "2.100000"},
    {"[% 1.2 + 2 * 3 %]", "7.200000"},
    {"[% 1.2 * 2 + 3 %]", "5.400000"},
    {"[% 1.2 + (2 * 3) %]", "7.200000"},
    {"[% (1.2 + 2) * 3 %]", "9.600000"},
    {"[% 1.2+2 %]", "3.200000"}
};

START_TEST (test_template) {
    char *out;
    string_string *c;

    c = &dict_template[_i];

    out = mp_template_str(c->in);

    fail_unless (strcmp(out, c->out) == 0,
            "template failed '%s' => '%s'", c->in, out);

    free(out);
}
END_TEST

START_TEST (test_template_file) {
    FILE *in;
    char *out;

    in = fopen("testdata/template_file.tpl", "r");

    out = mp_template(in);

    fail_unless (strcmp(out, "foobar") == 0,
            "template from file failed => '%s'", out);
    free(out);
}
END_TEST

int main (void) {

  int number_failed;
  SRunner *sr;

  Suite *s = suite_create ("Template");

  /* String test case */
  TCase *tc = tcase_create ("Template");
  tcase_add_loop_test(tc, test_template, 0, 39);
  tcase_add_test(tc, test_template_file);
  suite_add_tcase (s, tc);

  sr = srunner_create(s);
  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

//void print_help(void) {
//}

/* vim: set ts=4 sw=4 et syn=c : */

/***
 * Monitoring Plugin - check_sms.c
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
#include "sms_utils.h"

#include <stdlib.h>
#include <locale.h>
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

static string_string test_encode_number[] = {
    {"+49172123456", "919471123254F6"},
    {"1212", "812121"},
    {"+491722270333", "91947122723033"},
    {"+491721234567", "91947112325476"},
};

static string_string test_encode_text[] = {
    {"Testtext:", "09D4F29C4E2FE3E93A"},
    {"Testtext:€", "0BD4F29C4E2FE3E9BA4D19"},
    {"Re test", "07D232885E9ED301"},
};

START_TEST (test_sms_encode_number) {
    char *dest;
    string_string *c;

    c = &test_encode_number[_i];

    dest = sms_encode_number(c->in);

    fail_unless (strcmp(dest, c->out) == 0,
            "sms_encode_number failed '%s' => '%s'", c->in, dest);

    free(dest);
}
END_TEST

START_TEST (test_sms_encode_text) {
    char *dest;
    string_string *c;

    c = &test_encode_text[_i];

    ck_assert(setlocale(LC_CTYPE, "en_US.UTF-8") != NULL);

    dest = sms_encode_text(c->in);

    fail_unless (strcmp(dest, c->out) == 0,
            "sms_encode_test failed '%s' => '%s'", c->in, dest);

    free(dest);
}
END_TEST

START_TEST (test_sms_encode_pdu) {
    char *pdu;

    ck_assert(setlocale(LC_CTYPE, "en_US.UTF-8") != NULL);

    pdu = sms_encode_pdu(NULL, "+491721234567", "Testtext:€");

    fail_unless (strcmp(pdu, "0005000C9194711232547600000BD4F29C4E2FE3E9BA4D19") == 0,
            "sms_encode_pdufailed=> '%s'", pdu);
}
END_TEST

START_TEST (test_sms_encode_pdu_smsc) {
    char *pdu;

    ck_assert(setlocale(LC_CTYPE, "en_US.UTF-8") != NULL);

    pdu = sms_encode_pdu("+491722270333", "+491721234567", "Testtext:€");

    fail_unless (strcmp(pdu, "079194712272303305000C9194711232547600000BD4F29C4E2FE3E9BA4D19") == 0,
            "sms_encode_pdufailed=> '%s'", pdu);
}
END_TEST

int main (void) {

  int number_failed;
  SRunner *sr;

  Suite *s = suite_create ("SMS");

  /* String test case */
  TCase *tc = tcase_create ("Encode");
  tcase_add_loop_test(tc, test_sms_encode_number, 0, 4);
  tcase_add_loop_test(tc, test_sms_encode_text, 0, 3);
  tcase_add_test(tc, test_sms_encode_pdu_smsc);
  tcase_add_test(tc, test_sms_encode_pdu);
  suite_add_tcase (s, tc);

  sr = srunner_create(s);
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

void print_help(void) {
}

/* vim: set ts=4 sw=4 et syn=c : */

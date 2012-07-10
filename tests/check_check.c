/***
 * Monitoring Plugin - check_check.c
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
#include <check.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"

static struct string_return test_net_addr_case[] = {
    { "127.0.0.1", 1 }, { "127.0.0.256", 0 },
    { "127.0.a.1", 0 }, { "129.132.10.42", 1 },
    {0,0}
};
static struct string_return test_net_addr6_case[] = {
    { "::1", 1 }, { "fe80::1", 1 },
    { "fe80::1::1", 0 }, { "fe80::12345", 0 },
    {0,0}
};
static struct string_return test_net_name_case[] = {
    { "www.durchmesser.ch", 1 }, { ".durchmesser.ch", 0 },
    { "www..durchmesser.ch", 0 }, { "www.1337.net", 1 },
    { "www.!.durchmesser.ch", 0 },
    {0,0}
};
static struct string_return test_net_url_case[] = {
    { "http://www.durchmesser.ch", 1},
    { "http://www.durchmesser.ch/", 1},
    { "h12-+.://www.durchmesser.ch", 1},
    { "1http://www.durchmesser.ch", 0},
    { "ht?tp://www.durchmesser.ch", 0},
    { "http://user@www.durchmesser.ch", 1},
    { "http://user:pass!$&'()*+,;=@www.durchmesser.ch", 1},
    { "http://user:pass!$&'()*?+,;=@www.durchmesser.ch", 0},
    { "http://user:pass%20%aa@www.durchmesser.ch", 1},
    { "http://user:pass%2g@www.durchmesser.ch", 0},
    { "http://user:pass%2@www.durchmesser.ch", 0},
    { "http://[::1]", 1},
    { "http://[::1]/", 1},
    { "http://[127.0.0.1]", 0},
    { "http://127.0.0.1", 1},
    { "http://127.0.0.1/", 1},
    { "http:///", 1},
    { "http:///path", 1},
    {0,0}
};


START_TEST (test_is_integer) {
    char teststring[20];

    sprintf(teststring, "%'.0f", (double) INT_MAX + 1);
    fail_unless (is_integer(teststring) == 0,
        "is_integer(%s) failed1", teststring);
    sprintf(teststring, "%'.0f", (double) INT_MAX);
    fail_unless (is_integer(teststring) == 1,
        "is_integer(%s) failed2", teststring);
    sprintf(teststring, "%'.0f", (double) INT_MIN);
    fail_unless (is_integer(teststring) == 1,
        "is_integer(%s) failed3", teststring);
    sprintf(teststring, "%'.0f", (double) INT_MIN - 1);
    fail_unless (is_integer(teststring) == 0,
        "is_integer(%s) failed4", teststring);
}
END_TEST

START_TEST (test_net_addr) {
    struct string_return *c = &test_net_addr_case[_i];

    fail_unless (is_hostaddr(c->string) == c->returning,
        "Fail: is_hostaddr(%s) is not %0.f", c->string,c->returning);
}
END_TEST

START_TEST (test_net_addr6) {
    struct string_return *c = &test_net_addr6_case[_i];

    fail_unless (is_hostaddr(c->string) == c->returning,
        "Fail: is_hostaddr(%s) is not %0.f", c->string,c->returning);
}
END_TEST

START_TEST (test_net_name) {
    struct string_return *c = &test_net_name_case[_i];

    fail_unless (is_hostname(c->string) == c->returning,
        "Fail: is_hostname(%s) is not %0.f", c->string,c->returning);
}
END_TEST

START_TEST (test_net_url) {
    struct string_return *c = &test_net_url_case[_i];

    fail_unless (is_url(c->string) == c->returning,
            "Fail: is_url(%s) is not %0.f", c->string,c->returning);
}
END_TEST

Suite* make_lib_check_suite(void) {

    Suite *s = suite_create ("Check");

    /* String test case */
    TCase *tc_str = tcase_create ("String");
    tcase_add_test(tc_str, test_is_integer);

    /* Network test case */
    TCase *tc_net = tcase_create ("Network");
    tcase_add_loop_test(tc_net, test_net_addr, 0, 4);
#ifdef USE_IPV6
    tcase_add_loop_test(tc_net, test_net_addr6, 0, 4);
#endif /* USE_IPV6*/
    tcase_add_loop_test(tc_net, test_net_name, 0, 5);
    tcase_add_loop_test(tc_net, test_net_url, 0, 17);
    suite_add_tcase (s, tc_net);
    return s;
}

/* vim: set ts=4 sw=4 et syn=c : */

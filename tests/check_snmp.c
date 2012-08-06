/***
 * Monitoring Plugin - check_snmp.c
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
#include "snmp_utils.h"

#include <stdlib.h>
#include <locale.h>
#include <check.h>
#include <sys/types.h>
#include <signal.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

const char *progname  = "TEST";
const char *progvers  = "TEST";
const char *progcopy  = "TEST";
const char *progauth  = "TEST";
const char *progusage = "TEST";

const char  *hostname = NULL;;
int         port = 1661;
mp_subprocess_t *replayd;
char *cmd[] = { "replayd/replayd", "-d", "replayd/replays", (char *)0 };

void snmp_replay_setup_v1(void);
void snmp_replay_setup_v2(void);
void snmp_replay_teardown(void);

void snmp_replay_setup_v1(void) {
    mp_snmp_community = "unittest";
    mp_snmp_version = SNMP_VERSION_1;
    hostname = strdup("localhost");
    replayd = mp_subprocess(cmd);
}

void snmp_replay_setup_v2(void) {
    mp_snmp_community = "unittest";
    mp_snmp_version = SNMP_VERSION_2c;
    hostname = strdup("localhost");
    replayd = mp_subprocess(cmd);
}

void snmp_replay_teardown(void) {
    kill(replayd->pid, SIGINT);
    mp_subprocess_close(replayd);
}

START_TEST (test_snmp_query_cmd) {
    netsnmp_session *ss;
    char *sysName;
    long sysUpTime;
    mp_snmp_query_cmd snmpcmd[] = {
        {{1,3,6,1,2,1,1,3,0}, 9,
            ASN_TIMETICKS, (void *)&sysUpTime, sizeof(long int)},
        {{1,3,6,1,2,1,1,5,0}, 9,
            ASN_OCTET_STR, (void *)&sysName, 0},
        {{0}, 0, 0, NULL},
    };
    ss = mp_snmp_init();
    mp_snmp_query(ss, snmpcmd);

    fail_unless(sysUpTime == 743743388,
            "Query sysUpTimeInstance failed. Got %d", sysUpTime);
    fail_unless(strcmp(sysName, "deeppurple") == 0,
            "Query sysNamefailed. Got %s", sysName);

    mp_snmp_deinit();
}
END_TEST

START_TEST (test_snmp_query_int) {
    netsnmp_session *ss;
    long int i1 = -1;
    long int i2 = -1;
    long int i3 = -1;
    long int i4 = -1;
    long int i5 = -1;

    mp_snmp_query_cmd snmpcmd[] = {
        {{1,3,6,1,4,1,31865,9999,42,2,1}, 11,
            ASN_INTEGER, (void *)&i1, sizeof(long int)},
        {{1,3,6,1,4,1,31865,9999,42,2,2}, 11,
            ASN_INTEGER, (void *)&i2, sizeof(long int)},
        {{1,3,6,1,4,1,31865,9999,42,2,3}, 11,
            ASN_INTEGER, (void *)&i3, sizeof(long int)},
        {{1,3,6,1,4,1,31865,9999,42,2,4}, 11,
            ASN_INTEGER, (void *)&i4, sizeof(long int)},
        {{1,3,6,1,4,1,31865,9999,42,2,5}, 11,
            ASN_INTEGER, (void *)&i5, sizeof(long int)},
        {{0}, 0, 0, NULL},
    };
    ss = mp_snmp_init();
    mp_snmp_query(ss, snmpcmd);

    fail_unless(i1 == 1,
            "DURCHMESSER-MIB::durchmesserExperimental.42.2.1 is not 1");
    fail_unless(i2 == -2,
            "DURCHMESSER-MIB::durchmesserExperimental.42.2.2 is not -2");
    fail_unless(i3 == -1,
            "DURCHMESSER-MIB::durchmesserExperimental.42.2.3 is not -1");
    fail_unless(i4 == 4,
            "DURCHMESSER-MIB::durchmesserExperimental.42.2.4 is not 4");
    fail_unless(i5 == -5,
            "DURCHMESSER-MIB::durchmesserExperimental.42.2.5 is not -5");

    mp_snmp_deinit();
}
END_TEST

START_TEST (test_snmp_query_string) {
    netsnmp_session *ss;
    char *s1 = NULL;
    char *s2 = NULL;
    char *s3 = NULL;
    char *s4 = NULL;
    char *s5 = NULL;
    mp_snmp_query_cmd snmpcmd[] = {
        {{1,3,6,1,4,1,31865,9999,42,4,1}, 11, ASN_OCTET_STR, (void *)&s1, 0},
        {{1,3,6,1,4,1,31865,9999,42,4,2}, 11, ASN_OCTET_STR, (void *)&s2, 0},
        {{1,3,6,1,4,1,31865,9999,42,4,3}, 11, ASN_OCTET_STR, (void *)&s3, 0},
        {{1,3,6,1,4,1,31865,9999,42,4,4}, 11, ASN_OCTET_STR, (void *)&s4, 0},
        {{1,3,6,1,4,1,31865,9999,42,4,5}, 11, ASN_OCTET_STR, (void *)&s5, 0},
        {{0}, 0, 0, NULL},
    };
    ss = mp_snmp_init();

    mp_snmp_query(ss, snmpcmd);

    fail_unless(strcmp(s1, "String1") == 0,
            "DURCHMESSER-MIB::durchmesserExperimental.42.4.1 is not String1");
    fail_unless(strcmp(s2, "String2") == 0,
            "DURCHMESSER-MIB::durchmesserExperimental.42.4.2 is not String2");
    fail_unless(s3 == NULL,
            "DURCHMESSER-MIB::durchmesserExperimental.42.4.3 is not -1");
    fail_unless(strcmp(s4, "String4") == 0,
            "DURCHMESSER-MIB::durchmesserExperimental.42.4.4 is not String4");
    fail_unless(strcmp(s5, "String5") == 0,
            "DURCHMESSER-MIB::durchmesserExperimental.42.4.5 is not String5");

    mp_snmp_deinit();
}
END_TEST

START_TEST (test_snmp_query_string_pre) {
    netsnmp_session *ss;
    char *s1 = NULL;
    char *s2 = NULL;
    char *s3 = NULL;
    char *s4 = NULL;
    char *s5 = NULL;

    s1 = mp_malloc(24);
    s2 = mp_malloc(24);
    s3 = strdup("FOOBAR");
    s4 = mp_malloc(24);
    s5 = mp_malloc(24);

    mp_snmp_query_cmd snmpcmd[] = {
        {{1,3,6,1,4,1,31865,9999,42,4,1}, 11, ASN_OCTET_STR, (void *)&s1, 0},
        {{1,3,6,1,4,1,31865,9999,42,4,2}, 11, ASN_OCTET_STR, (void *)&s2, 0},
        {{1,3,6,1,4,1,31865,9999,42,4,3}, 11, ASN_OCTET_STR, (void *)&s3, 0},
        {{1,3,6,1,4,1,31865,9999,42,4,4}, 11, ASN_OCTET_STR, (void *)&s4, 0},
        {{1,3,6,1,4,1,31865,9999,42,4,5}, 11, ASN_OCTET_STR, (void *)&s5, 0},
        {{0}, 0, 0, NULL},
    };
    ss = mp_snmp_init();

    mp_snmp_query(ss, snmpcmd);

    fail_unless(strcmp(s1, "String1") == 0,
            "DURCHMESSER-MIB::durchmesserExperimental.42.4.1 is not String1");
    fail_unless(strcmp(s2, "String2") == 0,
            "DURCHMESSER-MIB::durchmesserExperimental.42.4.2 is not String2");
    fail_unless(strcmp(s3, "FOOBAR") == 0,
            "DURCHMESSER-MIB::durchmesserExperimental.42.4.3 is not FOOBAR");
    fail_unless(strcmp(s4, "String4") == 0,
            "DURCHMESSER-MIB::durchmesserExperimental.42.4.4 is not String4");
    fail_unless(strcmp(s5, "String5") == 0,
            "DURCHMESSER-MIB::durchmesserExperimental.42.4.5 is not String5");

    mp_snmp_deinit();

    free(s1);
    free(s2);
    free(s3);
    free(s4);
    free(s5);
}
END_TEST


START_TEST (test_snmp_query_counter) {
    netsnmp_session *ss;
    long int i1 = -1;
    long int i2 = -1;
    long int i3 = -1;
    long int i4 = -1;
    long int i5 = -1;

    mp_snmp_query_cmd snmpcmd[] = {
        {{1,3,6,1,4,1,31865,9999,42,65,1}, 11,
            ASN_COUNTER, (void *)&i1, sizeof(long int)},
        {{1,3,6,1,4,1,31865,9999,42,65,2}, 11,
            ASN_COUNTER, (void *)&i2, sizeof(long int)},
        {{1,3,6,1,4,1,31865,9999,42,65,3}, 11,
            ASN_COUNTER, (void *)&i3, sizeof(long int)},
        {{1,3,6,1,4,1,31865,9999,42,65,4}, 11,
            ASN_COUNTER, (void *)&i4, sizeof(long int)},
        {{1,3,6,1,4,1,31865,9999,42,65,5}, 11,
            ASN_COUNTER, (void *)&i5, sizeof(long int)},
        {{0}, 0, 0, NULL},
    };
    ss = mp_snmp_init();
    mp_snmp_query(ss, snmpcmd);

    fail_unless(i1 == 1,
            "DURCHMESSER-MIB::durchmesserExperimental.42.65.1 is not 1");
    fail_unless(i2 == 4294967294,
            "DURCHMESSER-MIB::durchmesserExperimental.42.65.2 is not 4294967294 (-2)");
    fail_unless(i3 == -1,
            "DURCHMESSER-MIB::durchmesserExperimental.42.65.3 is not -1");
    fail_unless(i4 == 4,
            "DURCHMESSER-MIB::durchmesserExperimental.42.65.4 is not 4");
    fail_unless(i5 == 4294967291,
            "DURCHMESSER-MIB::durchmesserExperimental.42.65.5 is not 4294967291 (-5)");

    mp_snmp_deinit();
}
END_TEST

START_TEST (test_snmp_query_gauge) {
    netsnmp_session *ss;
    long int i1 = -1;
    long int i2 = -1;
    long int i3 = -1;
    long int i4 = -1;
    long int i5 = -1;

    mp_snmp_query_cmd snmpcmd[] = {
        {{1,3,6,1,4,1,31865,9999,42,66,1}, 11,
            ASN_GAUGE, (void *)&i1, sizeof(long int)},
        {{1,3,6,1,4,1,31865,9999,42,66,2}, 11,
            ASN_GAUGE, (void *)&i2, sizeof(long int)},
        {{1,3,6,1,4,1,31865,9999,42,66,3}, 11,
            ASN_GAUGE, (void *)&i3, sizeof(long int)},
        {{1,3,6,1,4,1,31865,9999,42,66,4}, 11,
            ASN_GAUGE, (void *)&i4, sizeof(long int)},
        {{1,3,6,1,4,1,31865,9999,42,66,5}, 11,
            ASN_GAUGE, (void *)&i5, sizeof(long int)},
        {{0}, 0, 0, NULL},
    };
    ss = mp_snmp_init();
    mp_snmp_query(ss, snmpcmd);

    fail_unless(i1 == 1,
            "DURCHMESSER-MIB::durchmesserExperimental.42.66.1 is not 1");
    fail_unless(i2 == 4294967294,
            "DURCHMESSER-MIB::durchmesserExperimental.42.66.2 is not 4294967294 (-2)");
    fail_unless(i3 == -1,
            "DURCHMESSER-MIB::durchmesserExperimental.42.66.3 is not -1");
    fail_unless(i4 == 4,
            "DURCHMESSER-MIB::durchmesserExperimental.42.66.4 is not 4");
    fail_unless(i5 == 4294967291,
            "DURCHMESSER-MIB::durchmesserExperimental.42.66.5 is not 4294967291 (-5)");

    mp_snmp_deinit();
}
END_TEST

int main (void) {

  int number_failed;
  SRunner *sr;

  Suite *s = suite_create ("SNMP");

  TCase *tc = tcase_create ("SNMPv1");
  tcase_add_unchecked_fixture(tc, snmp_replay_setup_v1, snmp_replay_teardown);
  tcase_add_test(tc, test_snmp_query_cmd);
  tcase_add_test(tc, test_snmp_query_int);
  tcase_add_test(tc, test_snmp_query_string);
  tcase_add_test(tc, test_snmp_query_string_pre);
  tcase_add_test(tc, test_snmp_query_counter);
  tcase_add_test(tc, test_snmp_query_gauge);
  suite_add_tcase(s, tc);

  tc = tcase_create ("SNMPv2");
  tcase_add_unchecked_fixture(tc, snmp_replay_setup_v2, snmp_replay_teardown);
  tcase_add_test(tc, test_snmp_query_cmd);
  tcase_add_test(tc, test_snmp_query_int);
  tcase_add_test(tc, test_snmp_query_string);
  tcase_add_test(tc, test_snmp_query_string_pre);
  tcase_add_test(tc, test_snmp_query_counter);
  tcase_add_test(tc, test_snmp_query_gauge);
  suite_add_tcase(s, tc);

  sr = srunner_create(s);
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

void print_help(void) {
}

/* vim: set ts=4 sw=4 et syn=c : */

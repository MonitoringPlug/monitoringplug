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

void snmp_replay_setup(void) {
    mp_snmp_community = "unittest";
    mp_snmp_version = SNMP_VERSION_2c;
    hostname = strdup("localhost");
    char *cmd[] = { "replayd/replayd", "-d", "replayd/replays", "-vv", (char *)0 };
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
    struct mp_snmp_query_cmd snmpcmd[] = {
        {{1,3,6,1,2,1,1,3,0}, 9, ASN_TIMETICKS, (void *)&sysUpTime},
        {{1,3,6,1,2,1,1,5,0}, 9, ASN_OCTET_STR, (void *)&sysName},
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

int main (void) {

  int number_failed;
  SRunner *sr;

  Suite *s = suite_create ("SNMP");

  /* String test case */
  TCase *tc = tcase_create ("Query");
  tcase_add_unchecked_fixture(tc, snmp_replay_setup, snmp_replay_teardown);
  tcase_add_test(tc, test_snmp_query_cmd);
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

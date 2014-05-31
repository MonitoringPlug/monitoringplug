#!/bin/sh

BASE="${PWD}/../.."

test_description='Test check_akcp'

. ../sharness.sh

test_expect_success 'check_akcp' "
    test_expect_code 0 $BASE/snmp/check_akcp -H test.mp.durchmesser.ch -P 1661\
        -C akcp
"

test_expect_success 'check_akcp w/ one sensor only' "
    test_expect_code 0 $BASE/snmp/check_akcp -H test.mp.durchmesser.ch -P 1661\
        -C akcp_s1_offline
"

test_done

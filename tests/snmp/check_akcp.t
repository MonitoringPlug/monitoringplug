#!/bin/sh

. ./setup.sh

test_expect_success HAVE_NET_SNMP 'check_akcp' "
    test_expect_code 0 $WRAPPER $BASE/snmp/check_akcp -H test.mp.durchmesser.ch -P 1661\
        -C akcp
"

test_expect_success HAVE_NET_SNMP 'check_akcp w/ one sensor only' "
    test_expect_code 0 $WRAPPER $BASE/snmp/check_akcp -H test.mp.durchmesser.ch -P 1661\
        -C akcp_s1_offline
"

test_done

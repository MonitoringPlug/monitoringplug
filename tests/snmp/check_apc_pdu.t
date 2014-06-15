#!/bin/sh

. ./setup.sh

test_expect_success HAVE_NET_SNMP 'check_apc_pdu' "
    test_expect_code 0 $BASE/snmp/check_apc_pdu -H test.mp.durchmesser.ch -P 1661\
        -C apc_pdu
"

test_expect_success HAVE_NET_SNMP 'check_akcp w/ unknown sensor' "
    test_expect_code 3 $BASE/snmp/check_apc_pdu -H test.mp.durchmesser.ch -P 1661\
        -C apc_pdu --on 9
"

test_expect_success HAVE_NET_SNMP 'check_akcp w/ sensor in wrong state' "
    test_expect_code 2 $BASE/snmp/check_apc_pdu -H test.mp.durchmesser.ch -P 1661\
        -C apc_pdu --off 1
"

test_done

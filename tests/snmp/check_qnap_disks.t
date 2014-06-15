#!/bin/sh

. ./setup.sh

test_expect_success HAVE_NET_SNMP 'check_qnap_disks' "
    test_expect_code 0 $WRAPPER $BASE/snmp/check_qnap_disks -H test.mp.durchmesser.ch -P 1661 \
        -C qnap
"

test_expect_success HAVE_NET_SNMP 'check_qnap_disks w/ one disk missing' "
    test_expect_code 2 $WRAPPER $BASE/snmp/check_qnap_disks -H test.mp.durchmesser.ch -P 1661 \
        -C qnap_disk_missing
"

test_done

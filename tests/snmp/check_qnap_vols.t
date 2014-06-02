#!/bin/sh

BASE="${PWD}/../.."

test_description='Test check_qnap_vols'

. ../sharness.sh

test_expect_success 'check_qnap_vols' "
    test_expect_code 0 $WRAPPER $BASE/snmp/check_qnap_vols -H test.mp.durchmesser.ch -P 1661 \
        -C qnap
"

test_expect_success 'check_qnap_vols w/ one disk missing' "
    test_expect_code 2 $WRAPPER $BASE/snmp/check_qnap_vols -H test.mp.durchmesser.ch -P 1661 \
        -C qnap_disk_missing
"

test_done

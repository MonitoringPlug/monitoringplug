#!/bin/sh

BASE="${PWD}/../.."

test_description='Test check_qnap_disks'

. ../sharness.sh

test_expect_success 'check_qnap_disks' "
    test_expect_code 0 $BASE/snmp/check_qnap_disks -H test.mp.durchmesser.ch -P 1661 \
        -C qnap
"

test_expect_success 'check_qnap_disks w/ one disk missing' "
    test_expect_code 2 $BASE/snmp/check_qnap_disks -H test.mp.durchmesser.ch -P 1661 \
        -C qnap_disk_missing
"

test_done

#!/bin/sh

BASE="${PWD}/../.."

test_description='Test check_arc_raid'

. ../sharness.sh

test_expect_success 'check_arc_raid' "
    test_expect_code 0 $BASE/snmp/check_arc_raid -H test.mp.durchmesser.ch -P 1661\
        -C areca
"

test_done

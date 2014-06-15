#!/bin/sh

. ./setup.sh

test_expect_success HAVE_NET_SNMP 'check_arc_raid' "
    test_expect_code 0 $WRAPPER $BASE/snmp/check_arc_raid -H test.mp.durchmesser.ch -P 1661\
        -C areca
"

test_done

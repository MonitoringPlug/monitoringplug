#!/bin/sh

. ./setup.sh

test_expect_success HAVE_NET_SNMP 'check_check_keepalived_vrrp' "
    test_expect_code 0 $BASE/snmp/check_keepalived_vrrp -H test.mp.durchmesser.ch -P 1661 \
        -C keepalived_vrrp_master
"

test_expect_success HAVE_NET_SNMP 'check_check_keepalived_vrrp w/ master instance' "
    test_expect_code 0 $BASE/snmp/check_keepalived_vrrp -H test.mp.durchmesser.ch -P 1661 \
        -C keepalived_vrrp_master --instance staginglb
"

test_expect_success HAVE_NET_SNMP 'check_check_keepalived_vrrp w/ slave instance' "
    test_expect_code 0 $BASE/snmp/check_keepalived_vrrp -H test.mp.durchmesser.ch -P 1661 \
        -C keepalived_vrrp_master --instance staginglb2
"

test_expect_success HAVE_NET_SNMP 'check_check_keepalived_vrrp w/ wildcard instance' "
    test_expect_code 0 $BASE/snmp/check_keepalived_vrrp -H test.mp.durchmesser.ch -P 1661 \
        -C keepalived_vrrp_master --instance stagingl*
"

test_expect_success HAVE_NET_SNMP 'check_check_keepalived_vrrp w/ unknown instance' "
    test_expect_code 2 $BASE/snmp/check_keepalived_vrrp -H test.mp.durchmesser.ch -P 1661 \
        -C keepalived_vrrp_master --instance staginglb1
"

test_expect_success HAVE_NET_SNMP 'check_check_keepalived_vrrp w/0 instance' "
    test_expect_code 3 $BASE/snmp/check_keepalived_vrrp -H test.mp.durchmesser.ch -P 1661 \
        -C keepalived_vrrp_none
"

test_expect_success HAVE_NET_SNMP 'check_check_keepalived_vrrp w/ actvie slave' "
    test_expect_code 1 $BASE/snmp/check_keepalived_vrrp -H test.mp.durchmesser.ch -P 1661 \
        -C keepalived_vrrp_slave_active
"

test_expect_success HAVE_NET_SNMP 'check_check_keepalived_vrrp w/ actvie slave but different instance' "
    test_expect_code 0 $BASE/snmp/check_keepalived_vrrp -H test.mp.durchmesser.ch -P 1661 \
        -C keepalived_vrrp_slave_active --instance staginglb2
"

test_done

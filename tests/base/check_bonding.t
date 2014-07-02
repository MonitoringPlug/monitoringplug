#!/bin/sh

. ./setup.sh

test_expect_success OS_LINUX 'Check all bonds' "
    PROC_BONDING_DIR=$BASE/tests/testdata/bonding \
    $WRAPPER $BASE/base/check_bonding
"

test_expect_success OS_LINUX 'Check w/o bond driver loaded' "
    export PROC_BONDING_DIR=$BASE/tests/testdata/bonding2
    test_expect_code 2 $WRAPPER $BASE/base/check_bonding
    unset PROC_BONDING_DIR
"

test_done

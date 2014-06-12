#!/bin/sh

BASE="${PWD}/../.."

test_description='Test check_tile'

. ../sharness.sh

# Setup Test Files
touch test_1
chmod 640 test_1
echo TEST > test_1
sleep 2

test_expect_success 'File exists' "
    $WRAPPER $BASE/base/check_file -f test_1
"

test_expect_success 'File do not exists' "
    test_expect_code 2 $WRAPPER $BASE/base/check_file -f test_2
"

test_expect_success 'File belongs to user' "
    $WRAPPER $BASE/base/check_file -f test_1 -o $USER
"

test_expect_success 'File belongs to nobody' "
    test_expect_code 2 $WRAPPER $BASE/base/check_file -f test_1 -o nobody
"

test_expect_success 'File belongs to group' "
    $WRAPPER $BASE/base/check_file -f test_1 -g $GROUPS
"

test_expect_success 'File belongs to group nobody' "
    test_expect_code 2 $WRAPPER $BASE/base/check_file -f test_1 -g nobody
"

test_expect_success 'File mode u+r' "
    $WRAPPER $BASE/base/check_file -f test_1 -a u+r
"

test_expect_success 'File mode u=r' "
    test_expect_code 2 $WRAPPER $BASE/base/check_file -f test_1 -a u=r
"

test_expect_success 'File mode u+x' "
    test_expect_code 2 $WRAPPER $BASE/base/check_file -f test_1 -a u+x
"

test_expect_success 'File mode u-x' "
    $WRAPPER $BASE/base/check_file -f test_1 -a u-x
"

test_expect_success 'File size not empty' "
    $WRAPPER $BASE/base/check_file -f test_1 -C 0:
"

test_expect_success 'File size >= 10' "
    test_expect_code 2 $WRAPPER $BASE/base/check_file -f test_1 -C 10:
"

test_expect_success 'File size <= 10' "
    $WRAPPER $BASE/base/check_file -f test_1 -C :10
"

test_expect_success 'File size warn not empty' "
    $WRAPPER $BASE/base/check_file -f test_1 -W 0:
"

test_expect_success 'File size warn >= 10' "
    test_expect_code 1 $WRAPPER $BASE/base/check_file -f test_1 -W 10:
"

test_expect_success 'File size warn <= 10' "
    $WRAPPER $BASE/base/check_file -f test_1 -W :10
"

test_expect_success 'File age older then 1s' "
    $WRAPPER $BASE/base/check_file -f test_1 -c 1s:
"

test_expect_success 'File age newer then 1d' "
    $WRAPPER $BASE/base/check_file -f test_1 -c :1d
"

test_expect_success 'File age older then 1d' "
    test_expect_code 2 $WRAPPER $BASE/base/check_file -f test_1 -c 1d:
"

test_expect_success 'File age warn older then 1s' "
    $WRAPPER $BASE/base/check_file -f test_1 -w 1s:
"

test_expect_success 'File age warn newer then 1d' "
    $WRAPPER $BASE/base/check_file -f test_1 -w :1d
"

test_expect_success 'File age warn older then 1d' "
    test_expect_code 1 $WRAPPER $BASE/base/check_file -f test_1 -w 1d:
"

test_done

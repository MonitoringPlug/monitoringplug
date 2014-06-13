#!/bin/sh

BASE="${PWD}/../.."

test_description='Test check_redis'

. ../sharness.sh

# Set REDISSERVER prerequisite if interpreter is available.
command -v redis-server >/dev/null && test_set_prereq REDISSERVER

if ! test_have_prereq REDISSERVER; then
  skip_all='skipping check_redis. redis-server not available.'
  test_done
fi

redis-server --port 7777 > /dev/null 2<&1 &
sleep 1

test_expect_success 'check_redis' "
    $WRAPPER $BASE/base/check_redis -P 7777
"

test_expect_success 'check_redis wrong port' "
   test_expect_code 2 $WRAPPER $BASE/base/check_redis -P 7778
"

test_expect_success 'check_redis w/ memory' "
    $WRAPPER $BASE/base/check_redis -P 7777 -W 1M -C 2M
"

test_expect_success 'check_redis w/ memory warning' "
   test_expect_code 1 $WRAPPER $BASE/base/check_redis -P 7777 -W 1 -C 2M
"

test_expect_success 'check_redis w/ memory critical' "
   test_expect_code 2 $WRAPPER $BASE/base/check_redis -P 7777 -W 1 -C 2
"

test_expect_success 'check_redis w/ low warning time' "
   test_expect_code 1 $WRAPPER $BASE/base/check_redis -P 7777 -w 0.001s
"

test_expect_success 'check_redis w/ low critical time' "
   test_expect_code 2 $WRAPPER $BASE/base/check_redis -P 7777 -c 0.001s
"

# Shutdown Redis
redis-cli -p 7777 shutdown

test_done

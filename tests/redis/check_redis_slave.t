#!/bin/sh

. ./setup.sh

# Set REDISSERVER prerequisite if interpreter is available.
command -v redis-server >/dev/null && test_set_prereq REDISSERVER

if ! test_have_prereq REDISSERVER; then
  skip_all='skipping check_redis. redis-server not available.'
  test_done
fi

redis-server --port 7777 --unixsocket redis.sock> /dev/null 2>&1 &
redis-server --port 8888 --slaveof 127.0.0.1 7777 --unixsocket redis_slave.sock> /dev/null 2>&1 &
touch redis.sock3
sleep 1

test_expect_success HAVE_REDIS 'check_redis_slave' "
    $WRAPPER $BASE/redis/check_redis_slave -P 8888
"

test_expect_success HAVE_REDIS 'check_redis_slave wrong port' "
    test_expect_code 2 $WRAPPER $BASE/redis/check_redis_slave -P 7778
"

test_expect_success HAVE_REDIS 'check_redis_slave unixsocket' "
    $WRAPPER $BASE/redis/check_redis_slave -s redis_slave.sock
"

test_expect_success HAVE_REDIS 'check_redis_slave unixsocket unknown file' "
    test_expect_code 2 $WRAPPER $BASE/redis/check_redis_slave redis.sock2
"

test_expect_success HAVE_REDIS 'check_redis_slave unixsocket file' "
    test_expect_code 2 $WRAPPER $BASE/redis/check_redis_slave redis.sock3
"

test_expect_success HAVE_REDIS 'check_redis_slave connect to master' "
   test_expect_code 2 $WRAPPER $BASE/redis/check_redis_slave -P 7777
"

# Shutdown Redis Master
redis-cli -p 7777 shutdown

test_expect_success HAVE_REDIS 'check_redis_slave w/ master shutdown' "
    test_expect_code 2 $WRAPPER $BASE/redis/check_redis_slave -P 8888
"

# Shutdown Redis
redis-cli -p 8888 shutdown

test_done

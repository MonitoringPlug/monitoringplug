#!/bin/sh

. ./setup.sh

# Set MEMCACHED prerequisite if interpreter is available.
command -v memcached >/dev/null && test_set_prereq MEMCACHED

if ! test_have_prereq MEMCACHED; then
  skip_all='skipping check_memcached. memcached not available.'
  test_done
fi

memcached -p 7777 -A -d > /dev/null
sleep 2

test_expect_success 'check_memcached' "
    $WRAPPER $BASE/base/check_memcached -P 7777
"

test_expect_success 'check_memcached wrong port' "
    test_expect_code 2 $WRAPPER $BASE/base/check_memcached -P 7778
"

# Shutdown Memcached
echo "shutdown" | nc localhost 7777

test_done

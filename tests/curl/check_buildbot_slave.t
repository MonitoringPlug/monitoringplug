#!/bin/sh

. ./setup.sh

test_expect_success HAVE_CURL,HAEV_JSON 'check_buildbot_slave' "
    test_expect_code 2 $WRAPPER $BASE/curl/check_buildbot_slave -H test.mp.durchmesser.ch \
        --port 80 --subpath /buildbot
"

test_expect_success HAVE_CURL,HAEV_JSON 'check_buildbot_slave w/ online slave' "
    $WRAPPER $BASE/curl/check_buildbot_slave -H test.mp.durchmesser.ch \
        --port 80 --subpath /buildbot --slave debian7-64
"

test_expect_success HAVE_CURL,HAEV_JSON 'check_buildbot_slave w/ offline slave' "
    test_expect_code 2 $WRAPPER $BASE/curl/check_buildbot_slave -H test.mp.durchmesser.ch \
        --port 80 --subpath /buildbot --slave fedora17-64
"

test_expect_success HAVE_CURL,HAEV_JSON 'Rabbit w\ garbage input' "
    test_expect_code 2 $WRAPPER $BASE/curl/check_buildbot_slave -H test.mp.durchmesser.ch --port 80 --subpath /garbage
"

test_expect_success HAVE_CURL,HAEV_JSON 'Rabbit w\ garbage json input' "
    test_expect_code 2 $WRAPPER $BASE/curl/check_buildbot_slave -H test.mp.durchmesser.ch --port 80 --subpath /garbage/json
"

test_done

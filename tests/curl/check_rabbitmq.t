#!/bin/sh

BASE="${PWD}/../.."

test_description='Test check_rabbitmq'

. ../sharness.sh

test_expect_success 'Rabbit 2.7.1' "
    $WRAPPER $BASE/curl/check_rabbitmq -H test.mp.durchmesser.ch --port 80 --subpath /rabbitmq_2.7.1
"

test_expect_success 'Rabbit 2.7.1 w/ w:c:' "
    $WRAPPER $BASE/curl/check_rabbitmq -H test.mp.durchmesser.ch --port 80 --subpath /rabbitmq_2.7.1 \
        --warning 150 --critical 200
"

test_expect_success 'Rabbit 2.7.1 w/ w:c: => warning' "
    test_expect_code 1 $WRAPPER $BASE/curl/check_rabbitmq -H test.mp.durchmesser.ch --port 80 --subpath /rabbitmq_2.7.1 \
        --warning 50 --critical 200
"

test_expect_success 'Rabbit 2.7.1 w/ w:c: => critical' "
    test_expect_code 2 $WRAPPER $BASE/curl/check_rabbitmq -H test.mp.durchmesser.ch --port 80 --subpath /rabbitmq_2.7.1 \
        --warning 50 --critical 100
"

test_expect_success 'Rabbit 3.1.1' "
    $WRAPPER $BASE/curl/check_rabbitmq -H test.mp.durchmesser.ch --port 80 --subpath /rabbitmq_3.1.1
"

test_expect_success 'Rabbit 3.1.1 w/ w:c:' "
    $WRAPPER $BASE/curl/check_rabbitmq -H test.mp.durchmesser.ch --port 80 --subpath /rabbitmq_3.1.1 \
        --warning 150 --critical 200
"

test_expect_success 'Rabbit 3.1.1 w/ w:c: => warning' "
    test_expect_code 1 $WRAPPER $BASE/curl/check_rabbitmq -H test.mp.durchmesser.ch --port 80 --subpath /rabbitmq_3.1.1 \
        --warning 1 --critical 200
"

test_expect_success 'Rabbit 3.1.1 w/ w:c: => critical' "
    test_expect_code 2 $WRAPPER $BASE/curl/check_rabbitmq -H test.mp.durchmesser.ch --port 80 --subpath /rabbitmq_3.1.1 \
        --warning 1 --critical 1
"

test_expect_success 'Rabbit w\ garbage input' "
    test_expect_code 2 $WRAPPER $BASE/curl/check_rabbitmq -H test.mp.durchmesser.ch --port 80 --subpath /garbage
"

test_expect_success 'Rabbit w\ garbage json input' "
    test_expect_code 2 $WRAPPER $BASE/curl/check_rabbitmq -H test.mp.durchmesser.ch --port 80 --subpath /garbage/json
"

test_done

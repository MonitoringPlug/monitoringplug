#!/bin/sh

BASE="${PWD}/../.."

test_description='Test check_rabbitmq'

. ../sharness.sh

test_expect_success 'Rabbit 2.7.1' "
    $BASE/curl/check_rabbitmq -H test.mp.durchmesser.ch -S /rabbitmq_2.7.1
"

test_expect_success 'Rabbit 2.7.1 w/ w:c:' "
    $BASE/curl/check_rabbitmq -H test.mp.durchmesser.ch -S /rabbitmq_2.7.1 \
        --warning 150 --critical 200
"

test_expect_success 'Rabbit 2.7.1 w/ w:c: => warning' "
    test_expect_code 1 $BASE/curl/check_rabbitmq -H test.mp.durchmesser.ch -S /rabbitmq_2.7.1 \
        --warning 50 --critical 200
"

test_expect_success 'Rabbit 2.7.1 w/ w:c: => critical' "
    test_expect_code 2 $BASE/curl/check_rabbitmq -H test.mp.durchmesser.ch -S /rabbitmq_2.7.1 \
        --warning 50 --critical 100
"

test_expect_success 'Rabbit w\ garbage input' "
    test_expect_code 2 $BASE/curl/check_rabbitmq -H test.mp.durchmesser.ch -S /garbage
"

test_expect_success 'Rabbit w\ garbage json input' "
    test_expect_code 2 $BASE/curl/check_rabbitmq -H test.mp.durchmesser.ch -S /garbage/json
"

test_done

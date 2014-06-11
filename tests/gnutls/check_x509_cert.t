#!/bin/sh

BASE="${PWD}/../.."

test_description='Test check_x509_cert'

. ../sharness.sh

certtool --generate-privkey --outfile test-key.pem > /dev/null 2>&1
certtool --generate-self-signed --template $BASE/tests/gnutls/template.cfg \
    --load-privkey test-key.pem --outfile test-cert.pem > /dev/null 2>&1
certtool --generate-self-signed --template $BASE/tests/gnutls/template2.cfg \
    --load-privkey test-key.pem --outfile test-cert2.pem > /dev/null 2>&1

# Set CERTTOOL prerequisite if interpreter is available.
command -v python >/dev/null && test_set_prereq CERTTOOL

if ! test_have_prereq CERTTOOL; then
  skip_all='skipping check_x509_cert. certtool not available'
  test_done
fi

test_expect_success 'check_x509_cert' "
    $WRAPPER $BASE/gnutls/check_x509_cert -C test-cert.pem
"

test_expect_success 'check_x509_cert w/ two certs' "
    $WRAPPER $BASE/gnutls/check_x509_cert -C test-cert.pem -C test-cert2.pem
"

test_expect_success 'check_x509_cert w/ warning cert' "
    test_expect_code 1 $WRAPPER $BASE/gnutls/check_x509_cert -w 120d -C test-cert.pem
"

test_expect_success 'check_x509_cert w/ one of two warning cert' "
    test_expect_code 1 $WRAPPER $BASE/gnutls/check_x509_cert -w 120d -C test-cert.pem -C test-cert2.pem
"

test_expect_success 'check_x509_cert w/ critical cert' "
    test_expect_code 2 $WRAPPER $BASE/gnutls/check_x509_cert -w 120d -c 100d -C test-cert.pem
"

test_expect_success 'check_x509_cert w/ warning and critical cert' "
    test_expect_code 2 $WRAPPER $BASE/gnutls/check_x509_cert -w 220d -c 100d -C test-cert.pem -C test-cert2.pem
"

test_expect_success 'check_x509_cert glob *' "
    $WRAPPER $BASE/gnutls/check_x509_cert -C '*cert*.pem'
"

test_expect_success 'check_x509_cert glob {}' "
    $WRAPPER $BASE/gnutls/check_x509_cert -C 'test-{cert,cert2}.pem' -v
"

test_done

#!/usr/bin/env bash

test_description="send info data directly to the daemon"

. `dirname $0`/sharness.sh

FLUX_PAYLOAD=${SHARNESS_TEST_SRCDIR}/scripts/flux_payload.py

test_under_flux 1
test_start_priorisea

test_expect_success 'valid: info.get returns version, seanerbus url and info' '
    echo "{}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.info.get payload.json > result.json &&
    jq -e ".data.version == 5" result.json &&
    jq -e ".data.seanerbus_url | type == \"string\"" result.json &&
    jq -e ".data.info.t_inactive == 0" result.json &&
    jq -e ".data.info.user_period == 7776000" result.json
'

test_expect_success 'valid: info.user_period.update changes the period' '
    jq -n "{data: 12345.0}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.info.user_period.update payload.json &&
    echo "{}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.info.get payload.json > result.json &&
    jq -e ".data.info.user_period == 12345" result.json
'

test_expect_success 'invalid: info.user_period.update with non-number data fails' '
    jq -n "{data: \"abc\"}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.info.user_period.update payload.json 2>err.txt
'

test_done

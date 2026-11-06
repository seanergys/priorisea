#!/usr/bin/env bash

test_description="send timewin data directly to the daemon"

. `dirname $0`/sharness.sh

FLUX_PAYLOAD=${SHARNESS_TEST_SRCDIR}/scripts/flux_payload.py

test_under_flux 1
test_start_priorisea

test_expect_success 'valid: no time windows before creation' '
    echo "{}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.timewin.list payload.json > result.json &&
    jq -e ".data == []" result.json
'

test_expect_success 'invalid: create with a bad cron expression fails' '
    jq -n "{data: {name: \"badwin\", cron: \"notacron\", duration: 10, score: 1.0}}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.timewin.create payload.json 2>err.txt
'

test_expect_success 'invalid: create with a negative duration reports the field' '
    jq -n "{data: {name: \"negwin\", cron: \"*/5 * * * * *\", duration: -1, score: 1.0}}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.timewin.create payload.json 2>err.txt &&
    grep -q "Field duration expected the following type: positive integer" err.txt
'

test_expect_success 'invalid: create with a missing name reports the field' '
    jq -n "{data: {cron: \"*/5 * * * * *\", duration: 10, score: 1.0}}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.timewin.create payload.json 2>err.txt &&
    grep -q "Field name is missing" err.txt
'

test_expect_success 'valid: create a time window' '
    jq -n "{data: {name: \"testwin\", cron: \"*/5 * * * * *\", duration: 300, score: 10.0}}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.timewin.create payload.json &&
    jq -n "{data: \"testwin\"}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.timewin.get payload.json > result.json &&
    jq -e ".data.name == \"testwin\"" result.json &&
    jq -e ".data.cron == \"*/5 * * * * *\"" result.json &&
    jq -e ".data.duration == 300" result.json &&
    jq -e ".data.score == 10" result.json
'

test_expect_success 'valid: the time window appears in the list' '
    echo "{}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.timewin.list payload.json > result.json &&
    jq -e ".data | length == 1" result.json &&
    jq -e ".data[0].name == \"testwin\"" result.json
'

test_expect_success 'valid: update the time window and read it back' '
    jq -n "{data: {name: \"testwin\", cron: \"*/10 * * * * *\", duration: 600, score: 20.0}}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.timewin.update payload.json &&
    jq -n "{data: \"testwin\"}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.timewin.get payload.json > result.json &&
    jq -e ".data.cron == \"*/10 * * * * *\"" result.json &&
    jq -e ".data.duration == 600" result.json &&
    jq -e ".data.score == 20" result.json
'

test_expect_success 'invalid: creating the same time window twice fails' '
    jq -n "{data: {name: \"testwin\", cron: \"*/5 * * * * *\", duration: 10, score: 1.0}}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.timewin.create payload.json 2>err.txt
'

test_expect_success 'invalid: update of a time window that does not exist fails' '
    jq -n "{data: {name: \"NOT_A_TIMEWIN\", cron: \"*/5 * * * * *\", duration: 10, score: 1.0}}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.timewin.update payload.json 2>err.txt
'

test_expect_success 'invalid: timewin.get with non-string data fails' '
    jq -n "{data: 123}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.timewin.get payload.json 2>err.txt
'

test_expect_success 'invalid: timewin.get for an unknown time window fails' '
    jq -n "{data: \"NOT_A_TIMEWIN\"}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.timewin.get payload.json 2>err.txt
'

test_expect_success 'invalid: timewin.delete with non-array data reports the type' '
    jq -n "{data: \"testwin\"}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.timewin.delete payload.json 2>err.txt &&
    grep -q "Field data expected the following type: array" err.txt
'

test_expect_success 'valid: delete the time window' '
    jq -n "{data: [\"testwin\"]}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.timewin.delete payload.json &&
    echo "{}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.timewin.list payload.json > result.json &&
    jq -e ".data == []" result.json &&
    jq -n "{data: \"testwin\"}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.timewin.get payload.json 2>err.txt
'

test_done

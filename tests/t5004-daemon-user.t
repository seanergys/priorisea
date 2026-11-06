#!/usr/bin/env bash

test_description="send user data directly to the daemon"

. `dirname $0`/sharness.sh

FLUX_PAYLOAD=${SHARNESS_TEST_SRCDIR}/scripts/flux_payload.py

test_under_flux 1
test_start_priorisea

test_expect_success 'valid: no users before any update' '
    echo "{}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.user.list payload.json > result.json &&
    jq -e ".data == []" result.json
'

test_expect_success 'valid: updating a user creates it' '
    jq -n "{data: [$(id -u)]}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.user.update payload.json
'

test_expect_success 'valid: user.get returns the created user' '
    jq -n "{data: {userid: $(id -u)}}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.user.get payload.json > result.json &&
    jq -e ".data.user.userid == $(id -u)" result.json &&
    jq -e ".data.user | has(\"score\") | not" result.json &&
    jq -e ".data.user.w_expr | type == \"string\"" result.json
'

test_expect_success 'valid: the updated user appears in the list' '
    echo "{}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.user.list payload.json > result.json &&
    jq -e ".data | length == 1" result.json &&
    jq -e ".data[0].userid == $(id -u)" result.json
'

test_expect_success 'valid: a user may read itself' '
    jq -n "{data: [424242]}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.user.update payload.json &&
    jq -n "{data: {userid: 424242}}" > payload.json &&
    ${FLUX_PAYLOAD} --userid 424242 priorisea.daemon.user.get payload.json > result.json &&
    jq -e ".data.user.userid == 424242" result.json
'

test_expect_success 'invalid: a user cannot read another user' '
    jq -n "{data: {userid: 999999}}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} --userid 424242 priorisea.daemon.user.get payload.json 2>err.txt &&
    grep -q "Access denied to user.999999" err.txt
'

test_expect_success 'invalid: user.list with non-array data reports the type' '
    jq -n "{data: \"nope\"}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.user.list payload.json 2>err.txt &&
    grep -q "Field data expected the following type: array" err.txt
'

test_expect_success 'invalid: user.list with a non-integer element reports it' '
    jq -n "{data: [\"abc\"]}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.user.list payload.json 2>err.txt &&
    grep -q "Field data\[0\] expected the following type: integer" err.txt
'

test_expect_success 'invalid: user.update with non-array data reports the type' '
    jq -n "{data: \"nope\"}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.user.update payload.json 2>err.txt &&
    grep -q "Field data expected the following type: array" err.txt
'

test_expect_success 'invalid: user.update with a non-integer element reports it' '
    jq -n "{data: [\"abc\"]}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.user.update payload.json 2>err.txt &&
    grep -q "Field data\[0\] expected the following type: integer" err.txt
'

test_expect_success 'invalid: user.get with a non-integer userid fails' '
    jq -n "{data: {userid: \"abc\"}}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.user.get payload.json 2>err.txt
'

test_expect_success 'invalid: user.get with a missing userid fails' '
    jq -n "{data: {}}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.user.get payload.json 2>err.txt
'

test_done

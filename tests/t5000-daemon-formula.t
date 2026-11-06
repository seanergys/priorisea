#!/usr/bin/env bash

test_description="send formula data directly to the daemon"

. `dirname $0`/sharness.sh

FLUX_PAYLOAD=${SHARNESS_TEST_SRCDIR}/scripts/flux_payload.py

test_under_flux 1
test_start_priorisea

test_expect_success 'valid: formula.get returns the default job_score' '
    jq -n "{data: \"job_score\"}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.formula.get payload.json > result.json &&
    jq -e ".data.name == \"job_score\"" result.json &&
    jq -e ".data.expr == \"<var:duration_ratio> + <var:timewin_score>\"" result.json &&
    jq -e ".data.defaults[\"var:duration_ratio\"] == 0" result.json
'

test_expect_success 'valid: formula.list with no data returns every formula' '
    echo "{}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.formula.list payload.json > result.json &&
    jq -e ".data | length == 3" result.json &&
    jq -e "[.data[].name] | sort | join(\",\") == \"job_priority,job_score,job_weight\"" result.json
'

test_expect_success 'valid: formula.list by name preserves request order' '
    jq -n "{data: [\"job_weight\", \"job_score\"]}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.formula.list payload.json > result.json &&
    jq -e "[.data[].name] == [\"job_weight\", \"job_score\"]" result.json
'

test_expect_success 'valid: formula.update stores a new expression' '
    jq -n "{data: {name: \"job_score\", expr: \"42\", defaults: {}}}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.formula.update payload.json &&
    jq -n "{data: \"job_score\"}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.formula.get payload.json > result.json &&
    jq -e ".data.expr == \"42\"" result.json &&
    jq -e "(.data.defaults | length) == 0" result.json
'

test_expect_success 'valid: formula.reset restores the default expression' '
    jq -n "{data: [\"job_score\"]}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.formula.reset payload.json &&
    jq -n "{data: \"job_score\"}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.formula.get payload.json > result.json &&
    jq -e ".data.expr == \"<var:duration_ratio> + <var:timewin_score>\"" result.json &&
    jq -e ".data.defaults[\"var:duration_ratio\"] == 0" result.json
'

test_expect_success 'invalid: formula.get with non-string data fails' '
    jq -n "{data: 123}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.formula.get payload.json 2>err.txt
'

test_expect_success 'invalid: formula.get for an unknown formula fails' '
    jq -n "{data: \"NOT_A_FORMULA\"}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.formula.get payload.json 2>err.txt
'

test_expect_success 'invalid: formula.update missing expr reports the field' '
    jq -n "{data: {name: \"job_score\"}}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.formula.update payload.json 2>err.txt &&
    grep -q "Field expr is missing" err.txt
'

test_expect_success 'invalid: formula.update with a bad expression fails validation' '
    jq -n "{data: {name: \"job_score\", expr: \"1 +\", defaults: {}}}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.formula.update payload.json 2>err.txt
'

test_expect_success 'invalid: formula.update of a formula that does not exist fails' '
    jq -n "{data: {name: \"NOT_A_FORMULA\", expr: \"1\", defaults: {}}}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.formula.update payload.json 2>err.txt
'

test_expect_success 'invalid: formula.list with non-array data reports the type' '
    jq -n "{data: \"nope\"}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.formula.list payload.json 2>err.txt &&
    grep -q "Field data expected the following type: array" err.txt
'

test_expect_success 'invalid: formula.list with a non-string element reports the index' '
    jq -n "{data: [123]}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.formula.list payload.json 2>err.txt &&
    grep -q "Field data\[0\] expected the following type: string" err.txt
'

test_expect_success 'invalid: formula.reset with non-array data reports the type' '
    jq -n "{data: 5}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.formula.reset payload.json 2>err.txt &&
    grep -q "Field data expected the following type: array" err.txt
'

test_done

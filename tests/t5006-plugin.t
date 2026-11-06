#!/usr/bin/env bash

test_description="send data directly to the plugin"

. `dirname $0`/sharness.sh

PRIORITY_PLUGIN=${SHARNESS_BUILD_DIRECTORY}/lib/flux/job-manager/plugins/sea_priority.so
FLUX_PAYLOAD=${SHARNESS_TEST_SRCDIR}/scripts/flux_payload.py

test_under_flux 1
test_start_priorisea

test_expect_success 'load the priority plugin' '
    flux jobtap load ${PRIORITY_PLUGIN}
'

test_expect_success 'valid: plugin debug is empty before sync' '
    echo "{}" > payload.json &&
    ${FLUX_PAYLOAD} job-manager.priorisea.plugin.debug payload.json > result.json &&
    jq -e ".data.formula.name == \"\"" result.json &&
    jq -e ".data.formula.expr == \"\"" result.json &&
    jq -e "(.data.formula.defaults | length) == 0" result.json &&
    jq -e "(.data.scores | length) == 0" result.json
'

test_expect_success 'valid: plugin sync with a formula and scores' '
    jq -n --argjson uid $(id -u) \
        "{data: {formula: {name: \"job_priority\", expr: \"42\", defaults: {}}, scores: {(\$uid|tostring): 42.0}}}" > payload.json &&
    ${FLUX_PAYLOAD} job-manager.priorisea.plugin.sync payload.json
'

test_expect_success 'valid: plugin debug shows the synced formula and score' '
    echo "{}" > payload.json &&
    ${FLUX_PAYLOAD} job-manager.priorisea.plugin.debug payload.json > result.json &&
    jq -e ".data.formula.name == \"job_priority\"" result.json &&
    jq -e ".data.formula.expr == \"42\"" result.json &&
    jq --argjson uid $(id -u) -e ".data.scores[\$uid|tostring] == 42" result.json
'

test_expect_success 'invalid: plugin sync with non-object scores reports the type' '
    jq -n --argjson uid $(id -u) \
        "{data: {formula: {name: \"job_priority\", expr: \"42\", defaults: {}}, scores: []}}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} job-manager.priorisea.plugin.sync payload.json 2>err.txt &&
    grep -q "Field data.scores expected the following type: object" err.txt
'

test_expect_success 'invalid: plugin sync with a non-real score reports the type' '
    jq -n --argjson uid $(id -u) \
        "{data: {formula: {name: \"job_priority\", expr: \"42\", defaults: {}}, scores: {(\$uid|tostring): \"abc\"}}}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} job-manager.priorisea.plugin.sync payload.json 2>err.txt &&
    grep -q "expected the following type: double" err.txt
'

test_expect_success 'invalid: plugin sync with a non-uid score key reports the type' '
    jq -n \
        "{data: {formula: {name: \"job_priority\", expr: \"42\", defaults: {}}, scores: {\"not_a_uid\": 42.0}}}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} job-manager.priorisea.plugin.sync payload.json 2>err.txt &&
    grep -q "expected the following type: uid" err.txt
'

test_done

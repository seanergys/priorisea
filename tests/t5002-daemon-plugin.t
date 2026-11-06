#!/usr/bin/env bash

test_description="send plugin data directly to the daemon"

. `dirname $0`/sharness.sh

PRIORITY_PLUGIN=${SHARNESS_BUILD_DIRECTORY}/lib/flux/job-manager/plugins/sea_priority.so
FLUX_PAYLOAD=${SHARNESS_TEST_SRCDIR}/scripts/flux_payload.py

test_under_flux 1
test_start_priorisea

test_expect_success 'invalid: plugin.debug before the plugin is loaded fails' '
    echo "{}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.plugin.debug payload.json 2>err.txt
'

test_expect_success 'invalid: plugin.sync before the plugin is loaded fails' '
    echo "{}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.plugin.sync payload.json 2>err.txt
'

test_expect_success 'load the priority plugin' '
    flux jobtap load ${PRIORITY_PLUGIN}
'

test_expect_success 'valid: plugin.debug is empty before sync' '
    echo "{}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.plugin.debug payload.json > result.json &&
    jq -e ".data.formula.name == \"\"" result.json &&
    jq -e ".data.formula.expr == \"\"" result.json &&
    jq -e "(.data.formula.defaults | length) == 0" result.json &&
    jq -e "(.data.scores | length) == 0" result.json
'

test_expect_success 'valid: plugin.sync pushes the database formula into the plugin' '
    echo "{}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.plugin.sync payload.json
'

test_expect_success 'valid: plugin.debug shows job_priority after sync' '
    echo "{}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.plugin.debug payload.json > result.json &&
    jq -e ".data.formula.name == \"job_priority\"" result.json &&
    jq -e ".data.formula.expr == \"<var:prev_priority> * 0.5 + <var:user_score> * 0.5\"" result.json &&
    jq -e ".data.scores | type == \"object\"" result.json
'

test_done

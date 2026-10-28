#!/usr/bin/env bash

test_description="test plugin command"

. `dirname $0`/sharness.sh
PRIORITY_PLUGIN=${SHARNESS_BUILD_DIRECTORY}/lib/flux/job-manager/plugins/sea_priority.so

test_under_flux 1
test_start_priorisea

test_expect_success 'test error when the plugin is not loaded' '
    test_must_fail priorisea plugin debug
'

test_expect_success 'load the priority plugin' '
    flux jobtap load ${PRIORITY_PLUGIN}
'

test_expect_success 'plugin debug is empty before sync' '
    priorisea plugin debug --json > plugin-before.json &&
    jq -e ".formula.name == \"\"" plugin-before.json &&
    jq -e ".formula.expr == \"\"" plugin-before.json &&
    jq -e "(.formula.defaults | length) == 0" plugin-before.json &&
    jq -e "(.scores | length) == 0" plugin-before.json
'

test_expect_success 'plugin sync pushes the database formula into the plugin' '
    priorisea plugin sync
'

test_expect_success 'plugin debug shows job_priority after sync' '
    priorisea plugin debug --json > plugin-after.json &&
    jq -e ".formula.name == \"job_priority\"" plugin-after.json &&
    jq -e ".formula.expr == \"<var:prev_priority> * 0.5 + <var:user_score> * 0.5\"" plugin-after.json &&
    jq -e ".formula.defaults[\"var:prev_priority\"] == 16" plugin-after.json &&
    jq -e ".formula.defaults[\"var:user_score\"] == 16" plugin-after.json &&
    jq -e ".scores | type == \"object\"" plugin-after.json
'

test_done

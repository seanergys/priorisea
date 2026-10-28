#!/usr/bin/env bash

test_description="test job priority"

. `dirname $0`/sharness.sh

SUBMIT=${SHARNESS_TEST_SRCDIR}/scripts/submit.sh
PRIORITY_PLUGIN=${SHARNESS_BUILD_DIRECTORY}/lib/flux/job-manager/plugins/sea_priority.so

test_under_flux 1
test_start_priorisea

test_expect_success 'set the user score to a known value' '
    priorisea formula update job_score --expr "42" &&
    ${SUBMIT} pwd &&
    priorisea job update --new &&
    priorisea user update $(whoami)
'

test_expect_success 'set the job_priority formula from the user score' '
    priorisea formula update job_priority --expr "<var:user_score> * 0.8"
'

test_expect_success 'load the priority plugin' '
    flux jobtap load ${PRIORITY_PLUGIN}
'

test_expect_success 'sync the plugin with the formula and user score' '
    priorisea plugin sync
'

test_expect_success 'the job priority follows the formula and user score' '
    jobid=$(${SUBMIT} pwd) &&
    uscore=$(priorisea user get $(whoami) --json | jq .score) &&
    flux job wait-event -t 5 -f json ${jobid} priority \
        | jq .context.priority > job.priority &&
    expected=$(awk "BEGIN {print int($uscore * 0.8)}") &&
    test "$(cat job.priority)" = "$expected"
'

#TODO
# Add tests for formula variables

test_done

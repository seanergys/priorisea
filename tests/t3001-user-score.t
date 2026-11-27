#!/usr/bin/env bash

test_description="test user score"

. `dirname $0`/sharness.sh

SUBMIT=${SHARNESS_TEST_SRCDIR}/scripts/submit.sh
PRIORITY_PLUGIN=${SHARNESS_BUILD_DIRECTORY}/lib/flux/job-manager/plugins/sea_priority.so

test_under_flux 1
test_start_priorisea

test_expect_success 'set the job_score formula to a known value' '
    priorisea formula update job_score "42" &&
    priorisea formula get job_score --json > formula.json &&
    jq -e ".expr == \"42\"" formula.json
'

test_expect_success 'the running user is scored like their job' '
    jobid=$(${SUBMIT} pwd) &&
    priorisea job update ${jobid} &&
    priorisea job get ${jobid} --json > job.json &&
    jq -e ".score == 42" job.json
'

test_expect_success 'the user score equals the job score with 1 weight' '
    priorisea formula update job_weight "1" &&
    priorisea user update $(whoami) &&
    priorisea user get $(whoami) --json > user.json &&
    jq -e ".userid == $(id -u)" user.json &&
    jq -e ".score == 42" user.json
'

test_expect_success 'reset restores the default job_weight formula' '
    priorisea formula reset job_weight &&
    priorisea formula get job_weight --json > formula.json &&
    jq -e ".expr == \"max(0, (<var:duration_seconds> / <var:max_duration_seconds>) - (<var:age_days> / 90))\"" formula.json
'

test_expect_success 'syncing pushes the user score into the plugin' '
    flux jobtap load ${PRIORITY_PLUGIN}
    priorisea plugin sync &&
    priorisea plugin debug --json > plugin.json &&
    jq -e ".scores[\"$(id -u)\"] == 42" plugin.json
'

test_expect_success 'duration weighting pulls the user score toward longer jobs' '
    jobA=$(${SUBMIT} "sleep 30") &&
    jobB=$(${SUBMIT} "sleep 1") &&
    priorisea formula update job_score "100" &&
    priorisea job update ${jobA} &&
    priorisea formula update job_score "2" &&
    priorisea job update ${jobB} &&
    priorisea formula update job_weight "<var:duration_seconds>" &&
    priorisea user update $(whoami) &&
    priorisea user get $(whoami) --json >u_w.json &&
    jq -e ".score > 90" u_w.json
'

test_done

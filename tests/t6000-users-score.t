#!/usr/bin/env bash

test_description="test multi-user scoring with fake users"

. `dirname $0`/sharness.sh

FLUX_PAYLOAD=${SHARNESS_TEST_SRCDIR}/scripts/flux_payload.py
PRIORITY_PLUGIN=${SHARNESS_BUILD_DIRECTORY}/lib/flux/job-manager/plugins/sea_priority.so

test_under_flux 1
test_fake_user_setup
test_start_priorisea

test_expect_success 'set the job_score and job_weight formulas' '
    priorisea formula update job_score "42" &&
    priorisea formula update job_weight "1"
'

test_expect_success 'fake user 424242 submits, runs and scores a job' '
    jobA=$(flux_run_fake 424242 hostname) &&
    flux job wait-event ${jobA} clean &&
    priorisea job update ${jobA} &&
    priorisea job get ${jobA} --json >jobA.json &&
    jq -e ".userid == 424242" jobA.json &&
    jq -e ".score == 42" jobA.json
'

test_expect_success 'fake user 424242 score is computed independently' '
    jq -n "{data: [424242]}" >payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.user.update payload.json &&
    jq -n "{data: {userid: 424242}}" >payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.user.get payload.json >userA.json &&
    jq -e ".data.user.userid == 424242" userA.json &&
    jq -e ".data.user.score == 42" userA.json
'

test_expect_success 'a second fake user scores independently of the first' '
    priorisea formula update job_score "7" &&
    jobB=$(flux_run_fake 424243 hostname) &&
    flux job wait-event ${jobB} clean &&
    priorisea job update ${jobB} &&
    priorisea job get ${jobB} --json >jobB.json &&
    jq -e ".userid == 424243" jobB.json &&
    jq -e ".score == 7" jobB.json &&
    jq -n "{data: [424243]}" >payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.user.update payload.json &&
    jq -n "{data: {userid: 424243}}" >payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.user.get payload.json >userB.json &&
    jq -e ".data.user.score == 7" userB.json &&
    jq -n "{data: {userid: 424242}}" >payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.user.get payload.json >userA2.json &&
    jq -e ".data.user.score == 42" userA2.json
'

test_expect_success 'a job priority follows the running user score' '
    priorisea formula update job_priority "<var:user_score>" \
        --default var:user_score 0 &&
    flux jobtap load ${PRIORITY_PLUGIN} &&
    jq -n --argjson a 424242 --argjson b 424243 \
        "{data: {formula: {name: \"job_priority\", expr: \"<var:user_score>\", defaults: {\"var:user_score\": 0.0}}, scores: {(\$a|tostring): 42.0, (\$b|tostring): 7.0}}}" >payload.json &&
    ${FLUX_PAYLOAD} job-manager.priorisea.plugin.sync payload.json &&
    jobC=$(flux_run_fake 424242 hostname) &&
    flux job wait-event -t 5 -f json ${jobC} priority \
        | jq -e ".context.priority == 42"
'

test_done

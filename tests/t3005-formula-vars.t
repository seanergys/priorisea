#!/usr/bin/env bash

test_description="test formula variable resolution and defaults"

. `dirname $0`/sharness.sh

SUBMIT=${SHARNESS_TEST_SRCDIR}/scripts/submit.sh

test_under_flux 1
test_start_priorisea

test_expect_success 'a null variable falls back to its formula default' '
    priorisea formula update job_score --expr "<var:user_score>" \
        --default var:user_score 0.75 &&
    jobid=$(${SUBMIT} pwd) &&
    priorisea job update ${jobid} &&
    priorisea job get ${jobid} --json >job.json &&
    jq -e ".score == 0.75" job.json
'

test_expect_success 'var:duration_ratio resolves to a value in [0,1]' '
    priorisea formula update job_score --expr "<var:duration_ratio>" \
        --default var:duration_ratio 0 &&
    jobid=$(${SUBMIT} "sleep 1") &&
    priorisea job update ${jobid} &&
    priorisea job get ${jobid} --json >job.json &&
    jq -e "has(\"score\")" job.json &&
    jq -e ".score >= 0" job.json &&
    jq -e ".score <= 1" job.json
'

test_done

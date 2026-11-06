#!/usr/bin/env bash

test_description="test without a seanerbus server"

. `dirname $0`/sharness.sh

SUBMIT=${SHARNESS_TEST_SRCDIR}/scripts/submit.sh

test_under_flux 1
test_start_priorisea

test_expect_success 'no error is raised if seanerbus is not used' '
    priorisea formula update job_score --expr "42" &&
    jobid=$(${SUBMIT} pwd) &&
    priorisea job update ${jobid} &&
    priorisea job get ${jobid} --json >job.json &&
    jq -e ".score == 42" job.json
'

test_expect_success 'error is raised if seanerbus is used' '
    priorisea formula update job_score --expr "<rpc:11111111-1111-1111-1111-111111111111>" &&
    jobid=$(${SUBMIT} pwd) &&
    test_must_fail priorisea job update ${jobid}
'

test_done

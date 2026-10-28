#!/usr/bin/env bash

test_description="test job score"

. `dirname $0`/sharness.sh

SUBMIT=${SHARNESS_TEST_SRCDIR}/scripts/submit.sh

test_under_flux 1
test_start_priorisea

test_expect_success 'set the job_score formula to a known value' '
    priorisea formula update job_score --expr "42" &&
    priorisea formula get job_score --json >formula.json &&
    jq -e ".expr == \"42\"" formula.json
'

test_expect_success 'a job receives the score the formula computes' '
    jobid=$(${SUBMIT} pwd) &&
    priorisea job update ${jobid} &&
    priorisea job get ${jobid} --json >job.json &&
    jq -e ".score == 42" job.json
'

test_expect_success 'idempotence of job update' '
    priorisea job update ${jobid} &&
    priorisea job get ${jobid} --json >job2.json &&
    test_cmp job.json job2.json
'

test_expect_success 'check the default t_inactive' '
    priorisea daemon debug --json > debug.json &&
    jq -e ".info.t_inactive == 0" debug.json
'

test_expect_success 'update the "new" jobs' '
    priorisea formula update job_score --expr "67" &&
    priorisea job update --new &&
    priorisea job get ${jobid} --json >job.json &&
    jq -e ".score == 67" job.json
'

test_expect_success 'check t_inactive has changed' '
    priorisea daemon debug --json > debug.json &&
    jq -e ".info.t_inactive != 0" debug.json
'

test_expect_success 'old jobs are unchanged when new jobs are updated' '
    priorisea formula update job_score --expr "69" &&
    jobid2=$(${SUBMIT} pwd) &&
    priorisea job update --new &&
    priorisea job get ${jobid} --json >job.json &&
    priorisea job get ${jobid2} --json >job2.json &&
    jq -e ".score == 67" job.json &&
    jq -e ".score == 69" job2.json
'

#TODO
# Add tests for formula variables

test_done

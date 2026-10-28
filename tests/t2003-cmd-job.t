#!/usr/bin/env bash

test_description="test job command"

. `dirname $0`/sharness.sh

SUBMIT=${SHARNESS_TEST_SRCDIR}/scripts/submit.sh

test_under_flux 1
test_start_priorisea

test_expect_success 'test interact with a false job' '
    test_must_fail priorisea job get NOT_A_JOB &&
    test_must_fail priorisea job list NOT_A_JOB &&
    test_must_fail priorisea job update NOT_A_JOB
'

test_expect_success 'test retrieve empty list of jobs' '
    priorisea job list --json >jobs.json &&
    jq -e ". == []" jobs.json
'

test_expect_success 'test update and read a job' '
    jobid=$(${SUBMIT}) &&
    priorisea job update ${jobid} &&
    priorisea job get ${jobid} --json >job.json &&
    priorisea job list ${jobid} --json >jobs_1.json &&
    priorisea job list --json >jobs.json &&
    test_cmp jobs.json jobs_1.json &&
    jq -S . job.json >job.norm.json &&
    jq -S ".[0]" jobs_1.json >job0.norm.json &&
    test_cmp job.norm.json job0.norm.json
'

test_expect_success 'list/update fail if one job is not found' '
    jobid=$(${SUBMIT} pwd) &&
    test_must_fail priorisea job list ${jobid} NOT_A_JOB &&
    test_must_fail priorisea job update ${jobid} NOT_A_JOB
'

test_expect_success 'a job records its duration' '
    jobid=$(${SUBMIT} "sleep 2") &&
    priorisea job update ${jobid} &&
    priorisea job get ${jobid} --json >job.json &&
    jq -e "has(\"duration\")" job.json &&
    jq -e ".duration > 0" job.json
'

test_done

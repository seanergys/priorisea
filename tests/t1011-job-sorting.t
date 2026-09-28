#!/usr/bin/env bash

test_description="test initial states of commands"

. `dirname $0`/sharness.sh
PRIORITY_PLUGIN=${SHARNESS_BUILD_DIRECTORY}/src/plugins/.libs/sea_priority.so
SUBMIT_AS=${SHARNESS_TEST_SRCDIR}/scripts/submit_as.py
test_under_flux 1

test_expect_success 'basic initialization' '
    flux jobtap load ${PRIORITY_PLUGIN}
    priorisea init
'

test_expect_success 'jobs are sorted by t_inactive and count is limited' '
    jobid1=$(flux python ${SUBMIT_AS} 1000 hostname) &&
    sleep 1 &&
    jobid2=$(flux python ${SUBMIT_AS} 1000 hostname) &&
    sleep 1 &&
    jobid3=$(flux python ${SUBMIT_AS} 1000 hostname) &&
    priorisea score update --new &&
    priorisea score job -n 2 --json >jobs.json &&
    dec1=$(flux job id ${jobid1}) &&
    dec2=$(flux job id ${jobid2}) &&
    dec3=$(flux job id ${jobid3}) &&
    jq -e "has(\"${dec2}\")" jobs.json &&
    jq -e "has(\"${dec3}\")" jobs.json &&
    test_must_fail jq -e "has(\"${dec1}\")" jobs.json
'

test_expect_success 'score job -n and --limit produce the same output' '
    priorisea score job -n 2 --json >jobs_short.json &&
    priorisea score job --limit 2 --json >jobs_long.json &&
    test_cmp jobs_short.json jobs_long.json
'

test_done

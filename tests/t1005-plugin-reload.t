#!/usr/bin/env bash

test_description="test flux plugin initial sync"

. `dirname $0`/sharness.sh
SUBMIT_AS=${SHARNESS_TEST_SRCDIR}/scripts/submit_as.py
USERID=1003
SCORE_MULT=0.8
PRIORITY_PLUGIN=${SHARNESS_BUILD_DIRECTORY}/src/plugins/.libs/sea_priority.so

export TEST_UNDER_FLUX_NO_JOB_EXEC=y
export TEST_UNDER_FLUX_SCHED_SIMPLE_MODE="limited=1"
test_under_flux 1

test_expect_success 'basic initialization' '
    flux jobtap load ${PRIORITY_PLUGIN} &&
    priorisea init
'

test_expect_success 'submit a job' '
job1=$(flux python ${SUBMIT_AS} ${USERID} hostname) &&
priorisea score update ${job1} &&
priorisea score user --json ${USERID} \
    | jq -r --arg uid "$USERID" '.[$uid]' > user.score
'

test_expect_success 'edit job priority formula' '
priorisea formula set job_priority "<var:user_score> * ${SCORE_MULT}" --default var:user_score 4
'

test_expect_success 'reload plugin' '
flux jobtap remove sea_priority.so &&
flux jobtap load ${PRIORITY_PLUGIN}
'

test_expect_success 'submit job and verify priority' '
job2=$(flux python ${SUBMIT_AS} ${USERID} hostname) &&
flux job wait-event -vt 5 -f json ${job2} priority \
	| jq '.context.priority' > job2.priority &&
uscore=$(cat user.score) &&
jscore=$(awk "BEGIN {print int($uscore * 0.8)}") &&
grep "${jscore}" job2.priority
'

test_done

#!/usr/bin/env bash

test_description="test seanergys score subcommand"

. `dirname $0`/sharness.sh
SUBMIT_AS=${SHARNESS_TEST_SRCDIR}/scripts/submit_as.py
USERID=1003
PRIORITY_PLUGIN=${FLUX_BUILD_DIR}/src/plugins/.libs/sea_priority.so

export TEST_UNDER_FLUX_NO_JOB_EXEC=y
export TEST_UNDER_FLUX_SCHED_SIMPLE_MODE="limited=1"
test_under_flux 1

test_expect_success 'basic initialization' '
    flux jobtap load ${PRIORITY_PLUGIN} &&
    priorisea init
'

test_expect_success 'retrieve new user score' '
test_must_fail priorisea score user ${USERID}
'

test_expect_success 'retrieve information before/after update' '
jobid=$(flux python ${SUBMIT_AS} ${USERID} hostname) &&
test_must_fail priorisea score job ${jobid} &&
priorisea score update ${jobid} &&
priorisea score job ${jobid}
'

test_expect_success 'update new jobs' '
jobid=$(flux python ${SUBMIT_AS} ${USERID} hostname) &&
priorisea score update --new &&
priorisea score job ${jobid}
'

test_done

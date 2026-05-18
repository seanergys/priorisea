#!/usr/bin/env bash

test_description="test seanergys score subcommand"

SUBMIT_AS=${SHARNESS_TEST_SRCDIR}/scripts/submit_as.py
USERID=1003

. `dirname $0`/sharness.sh

PRIORITY_PLUGIN=${FLUX_BUILD_DIR}/src/plugins/.libs/sea_priority.so
test_under_flux 1

test_expect_success 'load priority plugin' '
    flux jobtap load ${PRIORITY_PLUGIN}
'

test_expect_success 'check that priority plugin is loaded' '
    flux jobtap list | grep sea_priority
'

test_expect_success 'retrieve default user score' '
priorisea score user ${USERID} --json >out
jq -e "map(select(.user_id == ${USERID})) |length == 1 and .[0].score == 0.5" out
'

test_expect_success 'retrieve information before/after update' '
jobid=$(flux python ${SUBMIT_AS} ${USERID} hostname) &&
test_must_fail priorisea score job ${jobid} &&
priorisea score update ${jobid} &&
priorisea score job ${jobid} --json >out &&
jq -e "map(select(.job_id == ${jobid})) |length == 1 and .[0].score" out
'

test_expect_success 'update new jobs' '
jobid=$(flux python ${SUBMIT_AS} ${USERID} hostname) &&
priorisea score update new &&
priorisea score job ${jobid} --json >out &&
jq -e "map(select(.job_id == ${jobid})) |length == 1 and .[0].score" out
'

test_done

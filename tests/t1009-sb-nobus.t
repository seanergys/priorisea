#!/usr/bin/env bash

test_description="test priorisea when seanerbus is not available"

. `dirname $0`/sharness.sh
PRIORITY_PLUGIN=${SHARNESS_BUILD_DIRECTORY}/src/plugins/.libs/sea_priority.so
SUBMIT_AS=${SHARNESS_TEST_SRCDIR}/scripts/submit_as.py
RPC_UUID="11111111-1111-1111-1111-111111111111"

test_under_flux 1

test_expect_success 'basic initialization' '
    flux jobtap load ${PRIORITY_PLUGIN} &&
    priorisea init
'

test_expect_success 'error when seanerbus is not available' '
    export SEANERBUS_URL="demobus://localhost:4321" &&
    priorisea formula set job_score "<rpc:$RPC_UUID>" &&
    jobid=$(flux python ${SUBMIT_AS} 1000 hostname) &&
    test_must_fail priorisea score update ${jobid} &&
    unset SEANERBUS_URL
'

test_expect_success 'default value prevents client error when seanerbus is not available' '
    export SEANERBUS_URL="demobus://localhost:4321" &&
    priorisea formula set job_score "<rpc:$RPC_UUID>" --default "rpc:$RPC_UUID" 100 &&
    jobid=$(flux python ${SUBMIT_AS} 1000 hostname) &&
    priorisea score update ${jobid} &&
    priorisea score job ${jobid} | grep 100 &&
    unset SEANERBUS_URL
'

test_done

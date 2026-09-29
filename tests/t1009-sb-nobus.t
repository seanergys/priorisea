#!/usr/bin/env bash

test_description="test priorisea when seanerbus is not available"

. `dirname $0`/sharness.sh
PRIORITY_PLUGIN=${SHARNESS_BUILD_DIRECTORY}/src/plugins/.libs/sea_priority.so
SUBMIT_AS=${SHARNESS_TEST_SRCDIR}/scripts/submit_as.py
RPC_UUID="11111111-1111-1111-1111-111111111111"

export SEANERBUS_PORT=59013

test_under_flux 1

test_expect_success 'basic initialization' '
    flux jobtap load ${PRIORITY_PLUGIN} &&
    priorisea init &&
    priorisea config get >test.out
'

test_expect_success 'error when seanerbus is not available' '
    priorisea formula set job_score "<rpc:$RPC_UUID>"&&
    jobid=$(flux python ${SUBMIT_AS} 1000 hostname) &&
    test_must_fail priorisea score update "$jobid"
'

test_expect_success 'default value prevents client error when seanerbus is not available' '
    test_stop_seanerbus &&
    priorisea formula set job_score "<rpc:$RPC_UUID>" --default "rpc:$RPC_UUID" 100 &&
    jobid=$(flux python ${SUBMIT_AS} 1000 hostname) &&
    priorisea score update "$jobid" &&
    priorisea score job "$jobid" | grep 100
'

test_done

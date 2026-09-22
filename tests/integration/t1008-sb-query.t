#!/usr/bin/env bash

test_description="test flux plugin update many jobs"

. `dirname $0`/sharness.sh
PRIORITY_PLUGIN=${FLUX_BUILD_DIR}/src/plugins/.libs/sea_priority.so
SUBMIT_AS=${SHARNESS_TEST_SRCDIR}/scripts/submit_as.py
SB_RESPONSE=${SHARNESS_TEST_SRCDIR}/scripts/sb_response
RPC_UUID="11111111-1111-1111-1111-111111111111"
test_under_flux 1
test_start_seanerbus

test_expect_success 'basic initialization' '
    flux jobtap load ${PRIORITY_PLUGIN} &&
    priorisea init
'

start_sb_response () {
    "$SB_RESPONSE" "$1" 100 \
        >sb_response.out 2>&1 </dev/null &
    SB_RESPONSE_PID=$!
}

stop_sb_response () {
    test -z "$SB_RESPONSE_PID" ||
    ! kill -0 "$SB_RESPONSE_PID" 2>/dev/null ||
    kill "$SB_RESPONSE_PID"

    wait "$SB_RESPONSE_PID" 2>/dev/null || :
    SB_RESPONSE_PID=
}

start_sb_naming () {
    echo "[{\"entity\":\"$RPC_UUID\", \"name\":{\"service\": \"test\"}}]" > services.json
    naming_server >sb_naming.out 2>&1 </dev/null &
    SB_NAMING_PID=$!
}

stop_sb_naming () {
    test -z "$SB_NAMING_PID" ||
    ! kill -0 "$SB_NAMING_PID" 2>/dev/null ||
    kill "$SB_NAMING_PID"

    wait "$SB_NAMING_PID" 2>/dev/null || :
    SB_NAMING_PID=
}

test_expect_success 'response client can be reached through UUID' '
    start_sb_response "$RPC_UUID" &&
    test_when_finished stop_sb_response &&
    priorisea formula set job_score "<rpc:$RPC_UUID>" &&
    jobid=$(flux python ${SUBMIT_AS} 1000 hostname) &&
    priorisea score update "$jobid" &&
    priorisea score job "$jobid" | grep 100
'

test_expect_success 'error when response client is not available' '
    priorisea formula set job_score "<rpc:$RPC_UUID>" &&
    jobid=$(flux python ${SUBMIT_AS} 1000 hostname) &&
    test_must_fail priorisea score update "$jobid"
'

test_expect_success 'response client can be reached through naming service' '
    start_sb_response "$RPC_UUID" &&
    start_sb_naming &&
    test_when_finished stop_sb_naming &&
    test_when_finished stop_sb_response &&
    priorisea formula set job_score "<rpc_ns:{\"service\": \"test\"}>" &&
    jobid=$(flux python ${SUBMIT_AS} 1000 hostname) &&
    priorisea score update "$jobid" &&
    priorisea score job "$jobid" | grep 100
'

test_expect_success 'error when both naming service and response client are not available' '
    priorisea formula set job_score "<rpc_ns:{\"service\": \"test\"}>" &&
    jobid=$(flux python ${SUBMIT_AS} 1000 hostname) &&
    test_must_fail priorisea score update "$jobid"
'

test_expect_success 'error when naming service is available but response client is not' '
    start_sb_naming &&
    test_when_finished stop_sb_naming &&
    priorisea formula set job_score "<rpc_ns:{\"service\": \"test\"}>" &&
    jobid=$(flux python ${SUBMIT_AS} 1000 hostname) &&
    test_must_fail priorisea score update "$jobid"
'

test_expect_success 'default value prevents client error' '
    priorisea formula set job_score "<rpc:$RPC_UUID>" --default "rpc:$RPC_UUID" 100  &&
    jobid=$(flux python ${SUBMIT_AS} 1000 hostname) &&
    priorisea score update "$jobid" &&
    priorisea score job "$jobid" | grep 100 &&
    priorisea formula set job_score "<rpc_ns:{\"service\": \"test\"}>" --default "rpc_ns:{\"service\": \"test\"}" 100 &&
    jobid=$(flux python ${SUBMIT_AS} 1000 hostname) &&
    priorisea score update "$jobid" &&
    priorisea score job "$jobid" | grep 100
'

test_done

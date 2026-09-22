#!/usr/bin/env bash

test_description="test flux plugin update many jobs"

. `dirname $0`/sharness.sh
PRIORITY_PLUGIN=${FLUX_BUILD_DIR}/src/plugins/.libs/sea_priority.so
SUBMIT_AS=${SHARNESS_TEST_SRCDIR}/scripts/submit_as.py
SB_RESPONSE=${SHARNESS_TEST_SRCDIR}/scripts/sb_response
SB_RESPONSE_UUID="11111111-1111-1111-1111-111111111111"
test_under_flux 1
test_start_seanerbus

test_expect_success 'basic initialization' '
    flux jobtap load ${PRIORITY_PLUGIN} &&
    priorisea init
'

start_sb_response () {
    "$SB_RESPONSE" "$SB_RESPONSE_UUID" 100 \
        >sb_response.out 2>&1 </dev/null &
    SB_CLIENT_PID=$!
}

stop_sb_client () {
    test -z "$SB_CLIENT_PID" ||
    ! kill -0 "$SB_CLIENT_PID" 2>/dev/null ||
    kill "$SB_CLIENT_PID"

    wait "$SB_CLIENT_PID" 2>/dev/null || :
    SB_CLIENT_PID=
}

test_expect_success 'response client can be reached through UUID' '
    start_sb_response &&
    test_when_finished stop_sb_client &&
    priorisea formula set job_score "{{rpc:$SB_RESPONSE_UUID}}" &&
    jobid=$(flux python "$SUBMIT_AS" 1000 hostname) &&
    priorisea score update --new &&
    priorisea score job "$jobid" | grep 100
'

test_done

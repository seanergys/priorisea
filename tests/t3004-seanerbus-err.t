#!/usr/bin/env bash

test_description="test seanerbus communication"

. `dirname $0`/sharness.sh

SUBMIT=${SHARNESS_TEST_SRCDIR}/scripts/submit.sh
SEANERBUS_PORT=3004
SEANERBUS_URL="demobus://127.0.0.1:${SEANERBUS_PORT}"
RPC_UUID="11111111-1111-1111-1111-111111111111"

test_under_flux 1
test_start_priorisea priorisea_test_db "$SEANERBUS_URL"
test_start_demobus "$SEANERBUS_URL"
test_start_sb_response "$RPC_UUID" 100 "$SEANERBUS_URL"
test_start_naming_server "$RPC_UUID" "$SEANERBUS_URL"

test_expect_success 'rpc response through seanerbus' '
    priorisea formula update job_score --expr "<rpc:${RPC_UUID}>" &&
    jobid=$(${SUBMIT} pwd) &&
    priorisea job update ${jobid} &&
    priorisea job get ${jobid} | grep 100
'

test_expect_success 'response client can be reached through naming service' '
    echo "[{\"entity\":\"${RPC_UUID}\", \"name\":{\"service\": \"test\"}}]" > services.json &&
    priorisea formula update job_score --expr "<rpc_ns:{\"service\": \"test\"}>" &&
    jobid=$(${SUBMIT} pwd) &&
    priorisea job update ${jobid} &&
    priorisea job get ${jobid} | grep 100
'

test_done

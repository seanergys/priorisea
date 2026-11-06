#!/usr/bin/env bash

test_description="test seanerbus communication"

. `dirname $0`/sharness.sh

SUBMIT=${SHARNESS_TEST_SRCDIR}/scripts/submit.sh
SEANERBUS_PORT=3003
SEANERBUS_URL="demobus://127.0.0.1:${SEANERBUS_PORT}"
RPC_UUID="11111111-1111-1111-1111-111111111111"
NO_RPC_UUID="01111111-1111-1111-1111-111111111111"

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

test_expect_success 'a failed rpc errors' '
    priorisea formula update job_score --expr "<rpc:${NO_RPC_UUID}>" &&
    jobid=$(${SUBMIT} pwd) &&
    test_must_fail priorisea job update ${jobid}
'

test_expect_success 'a failed name service errors' '
    priorisea formula update job_score --expr "<rpc_ns:{\"service\": \"not_a_service\"}>" &&
    jobid=$(${SUBMIT} pwd) &&
    test_must_fail priorisea job update ${jobid}
'

test_expect_success 'a malformed rpc target errors' '
    priorisea formula update job_score --expr "<rpc:not-a-uuid>" &&
    jobid=$(${SUBMIT} pwd) &&
    test_must_fail priorisea job update ${jobid}
'

test_expect_success 'the rpc default is used when the rpc fails' '
    priorisea formula update job_score --expr "<rpc:${NO_RPC_UUID}>" \
        --default "rpc:${NO_RPC_UUID}" 0.5 &&
    jobid=$(${SUBMIT} pwd) &&
    priorisea job update ${jobid} &&
    priorisea job get ${jobid} --json >job.json &&
    jq -e ".score == 0.5" job.json
'

test_done

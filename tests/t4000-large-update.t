#!/usr/bin/env bash

test_description="test user score"

. `dirname $0`/sharness.sh

SUBMIT=${SHARNESS_TEST_SRCDIR}/scripts/submit.sh
SEANERBUS_PORT=4000
SEANERBUS_URL="seanerbroker://127.0.0.1:${SEANERBUS_PORT}"
RPC_UUID="11111111-1111-1111-1111-111111111111"

test_under_flux 1
test_start_priorisea priorisea_test_db "$SEANERBUS_URL"
test_start_seanerbroker "$SEANERBUS_URL"
test_start_sb_response "$RPC_UUID" 100 "$SEANERBUS_URL"
test_start_naming_server "$RPC_UUID" "$SEANERBUS_URL"

test_expect_success 'submit 100 jobs' '
	for i in $(seq 1 100); do
		${SUBMIT} pwd >>jobids.txt || return 1
	done
'
test_expect_success 'update and read jobs' '
    priorisea job update $(cat jobids.txt) &&
    priorisea job list $(cat jobids.txt)
'

test_expect_success 'use seanerbus rpc' '
	priorisea formula update job_score --expr "<rpc:$RPC_UUID>" &&
	priorisea job update $(cat jobids.txt)
'

test_expect_success 'use seanerbus naming service' '
    echo "[{\"entity\":\"${RPC_UUID}\", \"name\":{\"service\": \"test\"}}]" > services.json &&
    priorisea formula update job_score --expr "<rpc_ns:{\"service\": \"test\"}>" &&
	priorisea job update $(cat jobids.txt)
'

test_done

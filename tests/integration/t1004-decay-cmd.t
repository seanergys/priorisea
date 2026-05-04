#!/usr/bin/env bash

test_description="test seanergys decay subcommand"

SUBMIT_AS=${SHARNESS_TEST_SRCDIR}/scripts/submit_as.py

. `dirname $0`/sharness.sh

test_expect_success 'get default decay' '
priorisea decay get --json >out &&
jq -e ".[0].duration == TODO" out &&
jq -e ".[0].weight == TODO" out &&
jq -e ".[1].duration == TODO" out &&
jq -e ".[1].weight == TODO" out &&
jq -e ".[2].duration == TODO" out &&
jq -e ".[2].weight == TODO" out
'

test_expect_success 'set decay' '
priorisea decay set "3600:1,7200:0.5" &&
priorisea decay get --json >out &&
jq -e ".[0].duration == 3600" out &&
jq -e ".[0].weight == 1" out &&
jq -e ".[1].duration == 7200" out &&
jq -e ".[1].weight == 0.5" out &&
test_must_fail jq -e ".[2]" out
'

test_expect_success 'reset decay' '
priorisea decay reset &&
priorisea decay get --json >out &&
jq -e ".[0].duration == TODO" out &&
jq -e ".[0].weight == TODO" out &&
jq -e ".[1].duration == TODO" out &&
jq -e ".[1].weight == TODO" out &&
jq -e ".[2].duration == TODO" out &&
jq -e ".[2].weight == TODO" out
'

test_done

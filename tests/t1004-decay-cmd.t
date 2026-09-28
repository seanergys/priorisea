#!/usr/bin/env bash

test_description="test seanergys decay subcommand"

. `dirname $0`/sharness.sh

PRIORITY_PLUGIN=${SHARNESS_BUILD_DIRECTORY}/src/plugins/.libs/sea_priority.so
DEFAULT_DURATIONS_0=86400
DEFAULT_DURATIONS_1=604800
DEFAULT_DURATIONS_2=1209600
DEFAULT_PRIORITIES_0=1.0
DEFAULT_PRIORITIES_1=0.5
DEFAULT_PRIORITIES_2=0.25
test_under_flux 1

test_expect_success 'basic initialization' '
    flux jobtap load ${PRIORITY_PLUGIN} &&
    priorisea init
'

test_expect_success 'get default decay' '
priorisea decay get --json >out &&
jq -e ".[0].duration == ${DEFAULT_DURATIONS_0}" out &&
jq -e ".[0].weight == ${DEFAULT_PRIORITIES_0}" out &&
jq -e ".[1].duration == ${DEFAULT_DURATIONS_1}" out &&
jq -e ".[1].weight == ${DEFAULT_PRIORITIES_1}" out &&
jq -e ".[2].duration == ${DEFAULT_DURATIONS_2}" out &&
jq -e ".[2].weight == ${DEFAULT_PRIORITIES_2}" out
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
jq -e ".[0].duration == ${DEFAULT_DURATIONS_0}" out &&
jq -e ".[0].weight == ${DEFAULT_PRIORITIES_0}" out &&
jq -e ".[1].duration == ${DEFAULT_DURATIONS_1}" out &&
jq -e ".[1].weight == ${DEFAULT_PRIORITIES_1}" out &&
jq -e ".[2].duration == ${DEFAULT_DURATIONS_2}" out &&
jq -e ".[2].weight == ${DEFAULT_PRIORITIES_2}" out
'

test_done

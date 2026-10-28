#!/usr/bin/env bash

test_description="test plugin reload"

. `dirname $0`/sharness.sh
PRIORITY_PLUGIN=${SHARNESS_BUILD_DIRECTORY}/lib/flux/job-manager/plugins/sea_priority.so

test_under_flux 1
test_start_priorisea

test_expect_success 'load the priority plugin' '
    flux jobtap load ${PRIORITY_PLUGIN}
'

test_expect_success 'verify idempotency when reloading' '
    priorisea plugin sync &&
    priorisea plugin debug --json > plugin-before.json &&
    flux jobtap remove sea_priority.so &&
    flux jobtap load ${PRIORITY_PLUGIN} &&
    priorisea plugin debug --json > plugin-after.json &&
    test_cmp plugin-before.json plugin-after.json
'

test_done

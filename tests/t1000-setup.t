#!/usr/bin/env bash

test_description="test basic setup"

. `dirname $0`/sharness.sh
PRIORITY_PLUGIN=${SHARNESS_BUILD_DIRECTORY}/lib/flux/job-manager/plugins/sea_priority.so

test_under_flux 1
test_start_priorisea

test_expect_success 'daemon responds after setup' '
    priorisea daemon debug
'

test_expect_success 'priority plugin is loaded and reachable' '
    flux jobtap load ${PRIORITY_PLUGIN} &&
    priorisea plugin debug
'

test_done

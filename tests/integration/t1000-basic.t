#!/usr/bin/env bash

test_description="test basic integration"

. `dirname $0`/sharness.sh

PRIORITY_PLUGIN=${FLUX_BUILD_DIR}/src/plugins/.libs/sea_priority.so
test_under_flux 1

test_expect_success 'load priority plugin' '
    flux jobtap load ${PRIORITY_PLUGIN}
'

test_expect_success 'check that priority plugin is loaded' '
    flux jobtap list | grep sea_priority
'

test_expect_success 'initialize priorisea' '
    priorisea init
'

test_done

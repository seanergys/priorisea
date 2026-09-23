#!/usr/bin/env bash

test_description="test initial states of commands"

. `dirname $0`/sharness.sh
PRIORITY_PLUGIN=${FLUX_BUILD_DIR}/src/plugins/.libs/sea_priority.so
test_under_flux 1

test_expect_success 'basic initialization' '
    flux jobtap load ${PRIORITY_PLUGIN}
'

test_expect_success 'list empty before init' '
    test_must_fail priorisea decay get &&
    test_must_fail priorisea formula list &&
    priorisea score job --all &&
    priorisea score user --all &&
    priorisea timewin list
'

test_expect_success 'list empty after init' '
    priorisea init &&
    priorisea decay get &&
    priorisea formula list
'

test_done

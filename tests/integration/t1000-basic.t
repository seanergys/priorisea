#!/usr/bin/env bash

PRIORITY_PLUGIN=TODO

test_description="test basic integration"

. `dirname $0`/sharness.sh

test_expect_success 'load priority plugin' '
    flux jobtap load ${PRIORITY_PLUGIN}
'

test_expect_success 'check that priority plugin is loaded' '
    flux jobtap list | grep sea_priority
'

test_done

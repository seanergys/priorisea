#!/usr/bin/env bash
#
#  Integration test for the priorisea sharness setup helper: it creates the
#  database and starts the daemon, then checks the daemon answers and the
#  priority plugin can be loaded against it.
#
test_description="test priorisea database + daemon setup helper"

. `dirname $0`/sharness.sh
PRIORITY_PLUGIN=${SHARNESS_BUILD_DIRECTORY}/src/plugins/.libs/sea_priority.so

# Bring up the stack the daemon depends on: a flux instance and the seanerbus
# bus, then the priorisea database + daemon via the helper under test.
test_under_flux 1
test_start_seanerbus
test_start_priorisea          # <- undefined at first: test must fail here

test_expect_success 'daemon responds after setup' '
    priorisea daemon debug
'

test_expect_success 'priority plugin can load against the daemon db' '
    flux jobtap load ${PRIORITY_PLUGIN}
'

test_done

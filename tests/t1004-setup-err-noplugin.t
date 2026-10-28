#!/usr/bin/env bash

test_description="test setup errors when the flux plugin is not available"

. `dirname $0`/sharness.sh

test_under_flux 1
test_start_priorisea

test_expect_success 'send a message to the plugin before loading it' '
    test_must_fail priorisea plugin debug
'

test_done

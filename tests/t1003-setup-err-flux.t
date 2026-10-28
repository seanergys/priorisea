#!/usr/bin/env bash

test_description="test setup errors when flux is available"

. `dirname $0`/sharness.sh

test_under_flux 1

test_expect_success 'start the daemon without database' '
    test_must_fail priorisea daemon start ./priorisea.db
'

test_expect_success 'send a message without the daemon' '
    test_must_fail priorisea daemon debug
'

test_done

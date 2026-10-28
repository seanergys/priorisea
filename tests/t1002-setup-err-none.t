#!/usr/bin/env bash

test_description="test setup errors when flux is not available"

. `dirname $0`/sharness.sh

test_expect_success 'start the daemon without database and flux' '
    test_must_fail priorisea daemon start ./priorisea.db
'

test_expect_success 'create the database twice' '
    priorisea database create ./priorisea.db &&
    test_must_fail priorisea database create ./priorisea.db
'

test_expect_success 'start the daemon without flux' '
    test_must_fail priorisea daemon start ./priorisea.db
'

test_expect_success 'send a message to the daemon without flux' '
    test_must_fail priorisea daemon debug
'

test_done

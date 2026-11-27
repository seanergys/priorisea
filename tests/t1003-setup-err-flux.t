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

test_expect_success 'every other subcommand fails without a running daemon' '
    test_must_fail priorisea daemon update-user-period 123 &&

    test_must_fail priorisea formula get NOT_A_FORMULA &&
    test_must_fail priorisea formula list &&
    test_must_fail priorisea formula update job_score "1" &&
    test_must_fail priorisea formula reset job_score &&

    test_must_fail priorisea job get NOT_A_JOB &&
    test_must_fail priorisea job list &&
    test_must_fail priorisea job update NOT_A_JOB &&

    test_must_fail priorisea plugin sync &&
    test_must_fail priorisea plugin debug &&

    test_must_fail priorisea timewin get NOT_A_TIMEWIN &&
    test_must_fail priorisea timewin list &&
    test_must_fail priorisea timewin update NOT_A_TIMEWIN -c "* * * * *" -d 60 -s 1 &&
    test_must_fail priorisea timewin create win -c "* * * * *" -d 60 -s 1 &&
    test_must_fail priorisea timewin delete NOT_A_TIMEWIN &&

    test_must_fail priorisea user get NOT_A_USER &&
    test_must_fail priorisea user list &&
    test_must_fail priorisea user update NOT_A_USER
'

test_done

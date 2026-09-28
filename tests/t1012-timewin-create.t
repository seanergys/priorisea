#!/usr/bin/env bash

test_description="test the creation of time windows"

. `dirname $0`/sharness.sh
PRIORITY_PLUGIN=${SHARNESS_BUILD_DIRECTORY}/src/plugins/.libs/sea_priority.so
SUBMIT_AS=${SHARNESS_TEST_SRCDIR}/scripts/submit_as.py
test_under_flux 1

test_expect_success 'basic initialization' '
    flux jobtap load ${PRIORITY_PLUGIN} &&
    priorisea init
'

test_expect_success 'create time window using parameters' '
    priorisea timewin add test_01 -c "* * * * * *" -d 60 -s 100 &&
    priorisea timewin list | grep test_01
'

test_expect_success 'create time window using json' '
    priorisea timewin add --json "{\"name\": \"test_02\", \"cron\": \"* * * * * *\", \"duration\": 60, \"score\": 100.0}" &&
    priorisea timewin list | grep test_02
'

test_done

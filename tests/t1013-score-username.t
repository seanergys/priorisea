#!/usr/bin/env bash

test_description="test score when using usernames"

. `dirname $0`/sharness.sh
PRIORITY_PLUGIN=${SHARNESS_BUILD_DIRECTORY}/src/plugins/.libs/sea_priority.so
test_under_flux 1

test_expect_success 'basic initialization' '
    flux jobtap load ${PRIORITY_PLUGIN} &&
    priorisea init
'

test_expect_success 'run a job' '
    jobid=$(flux submit hostname) &&
    flux job wait-event ${jobid} finish &&
    priorisea score update ${jobid}
'

test_expect_success 'retrieve the score using user id' '
    priorisea score user $(id -u) --json >user.json &&
    jq -e ".[] | select(.userid == $(id -u))" user.json
'

test_expect_success 'retrieve the score using username' '
    priorisea score user $(id -un) --json >user.json &&
    jq -e ".[] | select(.user == \"$(id -un)\")" user.json
'

test_expect_success 'retrieve the score using an unknown userid or username' '
    test_must_fail priorisea score user 1 &&
    test_must_fail priorisea score user "unknown_username"
'

test_done

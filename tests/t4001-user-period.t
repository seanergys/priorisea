#!/usr/bin/env bash

test_description="test user period"

. `dirname $0`/sharness.sh

SUBMIT=${SHARNESS_TEST_SRCDIR}/scripts/submit.sh

test_under_flux 1
test_start_priorisea

test_expect_success 'submit a job and check that the user score take it into account' '
    priorisea formula update job_score --expr "42" &&
    priorisea formula update job_weight --expr "1" &&
    ${SUBMIT} pwd &&
    priorisea job update --new &&
    priorisea user update $(whoami) &&
    priorisea user get $(whoami) --json > user1.json &&
    jq -e ".userid == $(id -u)" user1.json &&
    jq -e ".score == 42" user1.json
'

test_expect_success 'update user_period to 0 and read it' '
	priorisea daemon update-user-period 0 &&
	priorisea daemon debug --json > daemon.json &&
	jq -e ".info.user_period == 0" daemon.json
'

test_expect_success 'the user does not have a score' '
    priorisea user update $(whoami) &&
    priorisea user get $(whoami) --json > user2.json &&
    jq -e ".userid == $(id -u)" user2.json &&
    jq -e "has(\"score\") | not" user2.json
'

test_expect_success 'revert user_period' '
	priorisea daemon update-user-period 999999 &&
	priorisea user update $(whoami) &&
    priorisea user get $(whoami) --json > user3.json &&
    jq -e ".userid == $(id -u)" user3.json &&
    jq -e ".score == 42" user3.json
'

test_done

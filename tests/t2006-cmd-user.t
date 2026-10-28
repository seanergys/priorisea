#!/usr/bin/env bash

test_description="test user command"

. `dirname $0`/sharness.sh

test_under_flux 1
test_start_priorisea

test_expect_success 'user commands fail for an unknown user' '
    test_must_fail priorisea user get NOT_A_USER &&
    test_must_fail priorisea user list NOT_A_USER &&
    test_must_fail priorisea user update NOT_A_USER
'

test_expect_success 'get/list command fail for a user not yet updated' '
    test_must_fail priorisea user get $(whoami) &&
    test_must_fail priorisea user list $(whoami)
'

test_expect_success 'no users before any update' '
    priorisea user list --json > users.json &&
    jq -e ". == []" users.json
'

test_expect_success 'updating a user creates it without score' '
    priorisea user update $(whoami) &&
    priorisea user get $(whoami) --json > user.json &&
    jq -e ".userid == $(id -u)" user.json &&
    jq -e "has(\"score\") | not" user.json
'

test_expect_success 'the updated user appears in the list' '
    priorisea user list --json > users.json &&
    jq -e "length == 1" users.json &&
    jq -e ".[0].userid == $(id -u)" users.json
'

test_expect_success 'list command fail if any user is not updated' '
    test_must_fail priorisea user list $(whoami) NOT_A_USER
'

test_expect_success 'updating twice deliver the same results' '
    priorisea user get $(whoami) --json > user1.json &&
    priorisea user update $(whoami) &&
    priorisea user get $(whoami) --json > user2.json &&
    test_cmp user1.json user2.json
'

test_done

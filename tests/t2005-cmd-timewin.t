#!/usr/bin/env bash

test_description="test timewin command"

. `dirname $0`/sharness.sh

test_under_flux 1
test_start_priorisea

test_expect_success 'timewin commands fail for an unknown time window' '
    test_must_fail priorisea timewin get NOT_A_TIMEWIN &&
    test_must_fail priorisea timewin list NOT_A_TIMEWIN &&
    test_must_fail priorisea timewin update NOT_A_TIMEWIN -c "* * * * *" -d 60 -s 1 &&
    test_must_fail priorisea timewin delete NOT_A_TIMEWIN
'

test_expect_success 'no time windows before creation' '
    priorisea timewin list --json > timewins.json &&
    jq -e "length == 0" timewins.json
'

test_expect_success 'create a time window and read it back' '
    priorisea timewin create testwin -c "*/5 * * * * *" -d 300 -s 10 &&
    priorisea timewin get testwin --json > testwin.json &&
    jq -e ".name == \"testwin\"" testwin.json &&
    jq -e ".cron == \"*/5 * * * * *\"" testwin.json &&
    jq -e ".duration == 300" testwin.json &&
    jq -e ".score == 10" testwin.json
'

test_expect_success 'the time window appears in the list' '
    priorisea timewin list --json > timewins.json &&
    jq -e "length == 1" timewins.json &&
    jq -e ".[0].name == \"testwin\"" timewins.json
'

test_expect_success 'creating the same time window twice fails' '
    test_must_fail priorisea timewin create testwin -c "* * * * * *" -d 0 -s 0
'

test_expect_success 'update the time window and read it back' '
    priorisea timewin update testwin -c "*/10 * * * * *" -d 600 -s 20 &&
    priorisea timewin get testwin --json > testwin.json &&
    jq -e ".cron == \"*/10 * * * * *\"" testwin.json &&
    jq -e ".duration == 600" testwin.json &&
    jq -e ".score == 20" testwin.json
'

test_expect_success 'delete the time window' '
    priorisea timewin delete testwin &&
    priorisea timewin list --json > timewins.json &&
    jq -e "length == 0" timewins.json &&
    test_must_fail priorisea timewin get testwin
'

test_expect_success 'fail to create time window' '
    test_must_fail priorisea timewin create testwin -c "* * * * *" -d 0 -s 0 &&
    test_must_fail priorisea timewin create testwin -c "* * * * * * * *" -d 0 -s 0 &&
    test_must_fail priorisea timewin create testwin -c "not a cron" -d 0 -s 0 &&
    test_must_fail priorisea timewin create testwin -c "* * * * * *" -d -1 -s 0
'

test_expect_success 'list/delete fail if one time window is unknown' '
    priorisea timewin create testwin -c "* * * * * *" -d 0 -s 0 &&
    test_must_fail priorisea timewin list testwin NOT_A_TIMEWIN &&
    test_must_fail priorisea timewin delete testwin NOT_A_TIMEWIN
'

test_done

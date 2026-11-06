#!/usr/bin/env bash

test_description="test user command options"

. `dirname $0`/sharness.sh

SUBMIT=${SHARNESS_TEST_SRCDIR}/scripts/submit.sh

test_under_flux 1
test_start_priorisea

test_expect_success 'user get --detail exposes the weight formula and jobs' '
    priorisea formula update job_score --expr "42" &&
    priorisea formula update job_weight --expr "1" &&
    jobid=$(${SUBMIT} pwd) &&
    priorisea job update ${jobid} &&
    priorisea user update $(whoami) &&
    priorisea user get $(whoami) --detail --json >user.detail.json &&
    jq -e ".user.userid == $(id -u)" user.detail.json &&
    jq -e "has(\"jobs\")" user.detail.json &&
    jq -e ".user | has(\"w_expr\")" user.detail.json &&
    priorisea user get $(whoami) --json >user.plain.json &&
    jq -e ".userid == $(id -u)" user.plain.json &&
    jq -e "(has(\"jobs\") | not)" user.plain.json
'

test_expect_success 'user list --order asc/desc are accepted and ordered' '
    priorisea user list --order asc --json >u.asc.json &&
    jq -e "type == \"array\"" u.asc.json &&
    priorisea user list --order desc --json >u.desc.json &&
    jq -e "type == \"array\"" u.desc.json &&
    test_must_fail priorisea user list --order bogus
'

test_expect_success 'user update --all updates every user' '
    priorisea user update --all &&
    priorisea user list --json >u.all.json &&
    jq -e "type == \"array\"" u.all.json &&
    jq -e "any(.[]; .userid == $(id -u))" u.all.json
'

test_done

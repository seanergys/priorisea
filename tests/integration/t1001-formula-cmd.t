#!/usr/bin/env bash

test_description="test seanergys formula subcommand"

DEFAULT_JOB_FORMULA="TODO"
DEFAULT_PRIORITY_FORMULA="TODO"

. `dirname $0`/sharness.sh

test_expect_success 'check that formula list contains default formulas' '
    out=$(priorisea formula list) &&
    echo "$out" | grep "job_score" &&
    echo "$out" | grep "priority"
'

test_expect_success 'check default formulas' '
priorisea formula get job_score priority --json >out &&
jq -e "map(select(.name == \"job_score\")) | length == 1 and .[0].formula == ${DEFAULT_JOB_FORMULA}" out &&
jq -e "map(select(.name == \"priority\")) | length == 1 and .[0].formula == ${DEFAULT_PRIORITY_FORMULA}" out
'

test_expect_success 'check the edition of default formulas' '
job_score="TODO" &&
priority="TODO" &&

priorisea formula set job_score "$job_score" &&
priorisea formula set priority "$priority" &&

priorisea formula get job_score priority --json >out &&
jq -e "map(select(.name == \"job_score\")) | length == 1 and .[0].formula == ${job_score}" out &&
jq -e "map(select(.name == \"priority\")) | length == 1 and .[0].formula == ${priority}" out
'

test_expect_success 'check the edition of default formulas with invalid formula' '
job_score="TODO" &&
priority="TODO" &&

test_must_fail priorisea formula set job_score "$job_score" &&
test_must_fail priorisea formula set priority "$priority"
'

test_expect_success 'reset default formulas' '
priorisea formula reset job_score priority &&

priorisea formula get job_score priority --json >out &&
jq -e "map(select(.name == \"job_score\")) | length == 1 and .[0].formula == ${DEFAULT_JOB_FORMULA}" out &&
jq -e "map(select(.name == \"priority\")) | length == 1 and .[0].formula == ${DEFAULT_PRIORITY_FORMULA}" out
'

test_done

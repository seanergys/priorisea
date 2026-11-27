#!/usr/bin/env bash

test_description="test formula command"

. `dirname $0`/sharness.sh

test_under_flux 1
test_start_priorisea

test_expect_success 'formula commands fail for an unknown formula' '
    test_must_fail priorisea formula get NOT_A_FORMULA &&
    test_must_fail priorisea formula list NOT_A_FORMULA &&
    test_must_fail priorisea formula update NOT_A_FORMULA "10" &&
    test_must_fail priorisea formula reset NOT_A_FORMULA &&

    test_must_fail priorisea formula list job_score NOT_A_FORMULA &&
    test_must_fail priorisea formula reset job_score NOT_A_FORMULA
'

test_expect_success 'exactly the three default formulas are stored' '
    priorisea formula list --json > formulas.json &&
    jq -e "length == 3" formulas.json &&
    jq -e "[.[].name] | sort | join(\",\") == \"job_priority,job_score,job_weight\"" formulas.json
'

test_expect_success 'default formula expressions and values are as expected' '
    priorisea formula get job_score --json > job_score.json &&
    jq -e ".expr == \"<var:duration_ratio> + <var:timewin_score>\"" job_score.json &&
    jq -e ".defaults[\"var:duration_ratio\"] == 0" job_score.json &&

    priorisea formula get job_priority --json > job_priority.json &&
    jq -e ".expr == \"<var:prev_priority> * 0.5 + <var:user_score> * 0.5\"" job_priority.json &&
    jq -e ".defaults[\"var:prev_priority\"] == 16" job_priority.json &&
    jq -e ".defaults[\"var:user_score\"] == 16" job_priority.json &&

    priorisea formula get job_weight --json > job_weight.json &&
    jq -e ".expr == \"max(0, (<var:duration_seconds> / <var:max_duration_seconds>) - (<var:age_days> / 90))\"" job_weight.json &&
    jq -e "(.defaults | length) == 0" job_weight.json
'

test_expect_success 'update a formula (single default) and read it back' '
    priorisea formula update job_score "<var:duration_ratio> * 2" --default var:duration_ratio 5 &&
    priorisea formula get job_score --json > job_score.json &&
    jq -e ".expr == \"<var:duration_ratio> * 2\"" job_score.json &&
    jq -e ".defaults[\"var:duration_ratio\"] == 5" job_score.json
'

test_expect_success 'update a formula with repeatable defaults and read it back' '
    priorisea formula update job_priority "<var:prev_priority> + <var:user_score>" --default var:prev_priority 10 --default var:user_score 20 &&
    priorisea formula get job_priority --json > job_priority.json &&
    jq -e ".expr == \"<var:prev_priority> + <var:user_score>\"" job_priority.json &&
    jq -e ".defaults[\"var:prev_priority\"] == 10" job_priority.json &&
    jq -e ".defaults[\"var:user_score\"] == 20" job_priority.json
'

test_expect_success 'reset restores the default formulas' '
    priorisea formula reset job_score job_priority &&
    priorisea formula get job_score --json > job_score.json &&
    jq -e ".expr == \"<var:duration_ratio> + <var:timewin_score>\"" job_score.json &&
    jq -e ".defaults[\"var:duration_ratio\"] == 0" job_score.json &&
    priorisea formula get job_priority --json > job_priority.json &&
    jq -e ".expr == \"<var:prev_priority> * 0.5 + <var:user_score> * 0.5\"" job_priority.json &&
    jq -e ".defaults[\"var:prev_priority\"] == 16" job_priority.json &&
    jq -e ".defaults[\"var:user_score\"] == 16" job_priority.json
'

test_expect_success 'list a single formula by name' '
    priorisea formula list job_score --json >one.json &&
    jq -e "length == 1" one.json &&
    jq -e ".[0].name == \"job_score\"" one.json
'

test_done

#!/usr/bin/env bash

test_description="test seanergys formula subcommand"

. `dirname $0`/sharness.sh

PRIORITY_PLUGIN=${FLUX_BUILD_DIR}/src/plugins/.libs/sea_priority.so
DEFAULT_JOB_SCORE_FORMULA="duration_ratio + timewin_score"
DEFAULT_JOB_SCORE_VARS_DURATION_RATIO=0.0
DEFAULT_JOB_SCORE_VARS_TIMEWIN_SCORE=0.0
DEFAULT_JOB_PRIORITY_FORMULA="prev_priority * 0.5 + user_score * 0.5"
DEFAULT_JOB_PRIORITY_VARS_PREV_PRIORITY=16.0
DEFAULT_JOB_PRIORITY_VARS_USER_SCORE=16.0
test_under_flux 1

test_expect_success 'basic initialization' '
    flux jobtap load ${PRIORITY_PLUGIN} &&
    priorisea init
'

test_expect_success 'check that formula list contains default formulas' '
    out=$(priorisea formula list) &&
    echo "$out" | grep "job_score" &&
    echo "$out" | grep "job_priority"
'

test_expect_success 'check default formulas' '
priorisea formula get job_score job_priority --json >out &&

jq -e "
  .job_score.expr == \"${DEFAULT_JOB_SCORE_FORMULA}\" and
  .job_score.vars.duration_ratio == ${DEFAULT_JOB_SCORE_VARS_DURATION_RATIO} and
  .job_score.vars.timewin_score == ${DEFAULT_JOB_SCORE_VARS_TIMEWIN_SCORE}
" out &&

jq -e "
  .job_priority.expr == \"${DEFAULT_JOB_PRIORITY_FORMULA}\" and
  .job_priority.vars.prev_priority == ${DEFAULT_JOB_PRIORITY_VARS_PREV_PRIORITY} and
  .job_priority.vars.user_score == ${DEFAULT_JOB_PRIORITY_VARS_USER_SCORE}
" out
'

test_expect_success 'check the edition of default formulas' '
job_score="8" &&
job_priority="max(1,prev_priority * user_score)" &&
job_priority_prev_priority=10 &&
job_priority_user_score=5 &&

priorisea formula set job_score "$job_score" &&
priorisea formula set job_priority "$job_priority" \
  --var prev_priority "$job_priority_prev_priority" \
  --var user_score "$job_priority_user_score" &&

priorisea formula get job_score job_priority --json >out &&

jq -e "
  .job_score.expr == \"$job_score\"
" out &&

jq -e "
  .job_priority.expr == \"$job_priority\" and
  .job_priority.vars.prev_priority == $job_priority_prev_priority and
  .job_priority.vars.user_score == $job_priority_user_score
" out
'

test_expect_success 'check the edition of default formulas with invalid formula' '
job_score="var" &&
job_priority="prev_priority" &&

test_must_fail priorisea formula set job_score "$job_score" &&
test_must_fail priorisea formula set job_priority "$job_priority"
'

test_expect_success 'reset default formulas' '
priorisea formula reset job_score job_priority &&
priorisea formula get job_score job_priority --json >out &&

jq -e "
  .job_score.expr == \"${DEFAULT_JOB_SCORE_FORMULA}\"
" out &&

jq -e "
  .job_priority.expr == \"${DEFAULT_JOB_PRIORITY_FORMULA}\" and
  .job_priority.vars.prev_priority == ${DEFAULT_JOB_PRIORITY_VARS_PREV_PRIORITY} and
  .job_priority.vars.user_score == ${DEFAULT_JOB_PRIORITY_VARS_USER_SCORE}
" out
'

test_done

#!/usr/bin/env bash

test_description="test seanergys timewin subcommand"

. `dirname $0`/sharness.sh

PRIORITY_PLUGIN=${FLUX_BUILD_DIR}/src/plugins/.libs/sea_priority.so
test_under_flux 1

test_expect_success 'basic initialization' '
    flux jobtap load ${PRIORITY_PLUGIN} &&
    priorisea init
'

test_expect_success 'add a time window' '
priorisea timewin add test_01 "* * * * * *" 3600 100
'

test_expect_success 'find a time window' '
priorisea timewin list | grep test_01
'

test_expect_success 'get a time window' '
priorisea timewin get test_01 --json >out &&
jq -e "
  .test_01.cron == \"* * * * * *\" and
  .test_01.duration == 3600 and
  .test_01.priority == 100
" out
'

test_expect_success 'edit a time window' '
priorisea timewin edit test_01 --cron "1 */1 * * * *" --duration 60 --priority 10 &&
priorisea timewin get test_01 --json >out &&
jq -e "
  .test_01.cron == \"1 */1 * * * *\" and
  .test_01.duration == 60 and
  .test_01.priority == 10
" out
'

test_expect_success 'set negative priority' '
priorisea timewin edit test_01 --priority -100 &&
priorisea timewin get test_01 --json >out &&
jq -e "
  .test_01.priority == -100
" out
'

test_expect_success 'edit a time window with invalid cron' '
test_must_fail priorisea timewin edit test_01 --cron "* * * * * * *" &&
test_must_fail priorisea timewin edit test_01 --cron "* * * * *"
'

test_expect_success 'edit a time window with invalid duration' '
test_must_fail priorisea timewin edit test_01 --duration -3600 &&
test_must_fail priorisea timewin edit test_01 --duration nan
'

test_expect_success 'edit a time window with invalid priority' '
test_must_fail priorisea timewin edit test_01 --priority nan
'

test_expect_success 'delete a time window' '
priorisea timewin delete test_01 &&
priorisea timewin list >out &&
test_must_fail grep test_01 out &&
test_must_fail priorisea timewin get test_01
'

test_done

#!/usr/bin/env bash

test_description="test seanergys time subcommand"

. `dirname $0`/sharness.sh

test_expect_success 'add a time range' '
priorisea time add test_01 "* * * * *" 3600 100
'

test_expect_success 'find a time range' '
priorisea time list | grep test_01
'

test_expect_success 'get a time range' '
priorisea time get test_01 --json >out &&
jq -e "map(select(.name == \"test_01\")) |
length == 1
and .[0].cron == \"* * * * *\"
and .[0].duration == 3600
and .[0].priority == 100" out
'

test_expect_success 'edit a time range' '
priorisea time edit test_01 --cron "1 */1 * * *" --duration 60 --priority 10 &&
priorisea time get test_01 --json >out &&
jq -e "map(select(.name == \"test_01\")) |
length == 1
and .[0].cron == \"1 */1 * * *\"
and .[0].duration == 60
and .[0].priority == 10" out
'

test_expect_success 'set negative priority' '
priorisea time edit test_01 --priority -100 &&
priorisea time get test_01 --json >out &&
jq -e "map(select(.name == \"test_01\")) |length == 1 and .[0].priority == -100" out
'

test_expect_success 'edit a time range with invalid cron' '
test_must_fail priorisea time edit test_01 --cron "* * * * * *" &&
test_must_fail priorisea time edit test_01 --cron "* * * *"
'

test_expect_success 'edit a time range with invalid duration' '
test_must_fail priorisea time edit test_01 --duration -3600 &&
test_must_fail priorisea time edit test_01 --duration nan
'

test_expect_success 'edit a time range with invalid priority' '
test_must_fail priorisea time edit test_01 --priority nan
'

test_expect_success 'delete a time range' '
priorisea time delete test_01 &&
priorisea time list >out &&
test_must_fail grep test_01 out &&
test_must_fail priorisea time get test_01
'

test_done

#!/usr/bin/env bash

test_description="test flux plugin sync with database"

SUBMIT_AS=${SHARNESS_TEST_SRCDIR}/scripts/submit_as.py
PRIORITY_FORMULA="TODO"
USERID=1005

. `dirname $0`/sharness.sh

test_expect_success 'set priority formula' '
priorisea formula set priority $PRIORITY_FORMULA
'

test_expect_success 'get priority formula' '
priorisea formula get priority --json >out &&
jq -e ".name == \"priority\"" out &&
jq -e ".formula == ${PRIORITY_FORMULA}" out
'

test_expect_success 'retrieve formula from plugin' '
cat <<-EOF >get_formula.py &&
import flux
import json
formula_data = ["priority"]
priority = flux.Flux().rpc("job-manager.sea_priority.get_formula", json.dumps(formula_data)).get()
print(json.dumps(priority))
EOF
flux python get_formula.py >out &&
jq -e "map(select(.name == \"priority\")) |length == 1 and .[0].formula == ${PRIORITY_FORMULA}" out
'

test_expect_success 'check user score sync' '
jobid=$(flux python ${SUBMIT_AS} ${USERID} hostname) &&
priorisea score update ${jobid} &&
priorisea score user ${USERID} --json >out &&
jq -e "map(select(.user_id == ${USERID})) |length == 1 and .[0].score" out &&
user_score=$(jq -r ".[] | select(.user_id == ${USERID}) | .score" out) &&

cat <<-EOF >get_user.py &&
import flux
import json
user_data = [${USERID}]
score = flux.Flux().rpc("job-manager.sea_priority.get_user", json.dumps(user_data)).get()
print(json.dumps(score))
EOF
flux python get_user.py >out &&
jq -e "map(select(.user_id == ${USERID})) |length == 1 and .[0].score == ${user_score}" out
'

test_done

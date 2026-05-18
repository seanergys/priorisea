#!/usr/bin/env bash

test_description="test plugin RPC endpoints"
USERID=1006

. `dirname $0`/sharness.sh

PRIORITY_PLUGIN=${FLUX_BUILD_DIR}/src/plugins/.libs/sea_priority.so
test_under_flux 1

test_expect_success 'load priority plugin' '
    flux jobtap load ${PRIORITY_PLUGIN}
'

test_expect_success 'check that priority plugin is loaded' '
    flux jobtap list | grep sea_priority
'

test_expect_success 'set priority formula' '
formula="TODO"

cat <<-EOF >set_formula.py &&
import flux
import json
formula_data = {
    "data": [
        {
            "name": "priority",
            "formula": ${formula}
        }
    ]
}
flux.Flux().rpc("job-manager.sea_priority.set_formula", formula_data).get()
EOF
flux python set_formula.py &&

cat <<-EOF >get_formula.py &&
import flux
import json
formula_data = {
    "data": [
        "priority"
    ]
}
priority = flux.Flux().rpc("job-manager.sea_priority.get_formula", formula_data).get()
print(json.dumps(priority))
EOF
flux python get_formula.py >out &&
jq -e "map(select(.name == \"priority\")) |length == 1 and .[0].formula == ${formula}" out
'

test_expect_success 'set user score' '
score=1.234

cat <<-EOF >set_score.py &&
import flux
import json
score_data = {
    "data": [
        {
            "user_id": ${USERID},
            "score": ${score}
        }
    ]
}
flux.Flux().rpc("job-manager.sea_priority.set_score", score_data).get()
EOF
flux python set_score.py &&

cat <<-EOF >get_score.py &&
import flux
import json
score_data = {
    "data": [
        ${USERID}
    ]
}
score = flux.Flux().rpc("job-manager.sea_priority.get_score", score_data).get()
print(json.dumps(score))
EOF
flux python get_score.py >out &&
jq -e "map(select(.user_id == ${USERID})) |length == 1 and .[0].score == ${score}" out
'

test_done

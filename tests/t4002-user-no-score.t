#!/usr/bin/env bash

test_description="test user default score"

. `dirname $0`/sharness.sh

SUBMIT=${SHARNESS_TEST_SRCDIR}/scripts/submit.sh
PRIORITY_PLUGIN=${SHARNESS_BUILD_DIRECTORY}/lib/flux/job-manager/plugins/sea_priority.so

test_under_flux 1
test_start_priorisea

test_expect_success 'updating a user without a job creates it without a score' '
    priorisea user update $(whoami) &&
    priorisea user get $(whoami) --json > user1.json &&
    jq -e ".userid == $(id -u)" user1.json &&
    jq -e "has(\"score\") | not" user1.json
'

test_expect_success 'submit a job and check that the user score take it into account' '
    priorisea formula update job_score "42" &&
    priorisea formula update job_weight "1" &&
    ${SUBMIT} pwd &&
    priorisea job update --new &&
    priorisea user update $(whoami) &&
    flux jobtap load ${PRIORITY_PLUGIN} &&
    priorisea plugin sync &&
    priorisea user get $(whoami) --json > user2.json &&
    priorisea plugin debug --json > plugin.json &&
    jq -e ".userid == $(id -u)" user2.json &&
    jq -e ".score == 42" user2.json &&
    jq -e ".scores[\"$(id -u)\"] == 42" plugin.json
'

# t4001-user-period already test that the score disappear if no job
# ran during the user_period

test_expect_success '0 job-weight prevent score computation' '
    priorisea formula update job_weight "0" &&
    priorisea user update $(whoami) &&
    priorisea user get $(whoami) --json > user3.json &&
    jq -e ".userid == $(id -u)" user3.json &&
    jq -e "has(\"score\") | not" user3.json
'

test_expect_success 'no-score users are not sent to the plugin' '
    priorisea plugin sync &&
    priorisea plugin debug --json >plugin.json &&
    jq -e --arg u "$(id -u)" ".scores | has(\$u) | not" plugin.json
'

test_expect_success 'priority plugin use the default user_score' '
    priorisea formula update job_priority "<var:user_score>" --default var:user_score 100 &&
    priorisea plugin sync &&
    jobid=$(${SUBMIT} pwd) &&
    flux job wait-event -t 5 -f json ${jobid} priority \
        | jq .context.priority > job.priority &&
    test "$(cat job.priority)" = 100
'

test_done

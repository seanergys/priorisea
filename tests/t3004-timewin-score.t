#!/usr/bin/env bash

test_description="test time window influence on job score"

. `dirname $0`/sharness.sh

SUBMIT=${SHARNESS_TEST_SRCDIR}/scripts/submit.sh

test_under_flux 1
test_start_priorisea

test_expect_success 'job score is the sum of the matching time windows' '
    priorisea formula update job_score "<var:timewin_score>" \
        --default var:timewin_score 0 &&
    priorisea timewin create winpos -c "*/5 * * * * *" -d 3600 -s 10 &&
    job1=$(${SUBMIT} pwd) &&
    priorisea job update ${job1} &&
    priorisea job get ${job1} --json >job1.json &&
    jq -e ".score == 10" job1.json &&
    priorisea timewin create winneg -c "*/5 * * * * *" -d 3600 --score=-3 &&
    job2=$(${SUBMIT} pwd) &&
    priorisea job update ${job2} &&
    priorisea job get ${job2} --json >job2.json &&
    jq -e ".score == 7" job2.json
'

test_expect_success 'deleting a window stops influencing new jobs' '
    priorisea timewin delete winpos &&
    job3=$(${SUBMIT} pwd) &&
    priorisea job update ${job3} &&
    priorisea job get ${job3} --json >job3.json &&
    jq -e ".score == -3" job3.json
'

test_expect_success 'a job during no window scores zero' '
    priorisea timewin delete winneg &&
    job4=$(${SUBMIT} pwd) &&
    priorisea job update ${job4} &&
    priorisea job get ${job4} --json >job4.json &&
    jq -e ".score == 0" job4.json
'

test_done

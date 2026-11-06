#!/usr/bin/env bash

test_description="test job command options"

. `dirname $0`/sharness.sh

SUBMIT=${SHARNESS_TEST_SRCDIR}/scripts/submit.sh

test_under_flux 1
test_start_priorisea

test_expect_success 'job get --detail exposes the score formula and weight' '
    priorisea formula update job_score --expr "42" &&
    jobid=$(${SUBMIT} pwd) &&
    priorisea job update ${jobid} &&
    priorisea job get ${jobid} --detail --json >job.detail.json &&
    jq -e ".userid == $(id -u)" job.detail.json &&
    jq -e "has(\"s_expr\")" job.detail.json &&
    jq -e "has(\"s_defaults\")" job.detail.json &&
    jq -e "has(\"s_values\")" job.detail.json &&
    jq -e "has(\"weight\") | not" job.detail.json &&
    jq -e "has(\"w_values\") | not" job.detail.json &&
    priorisea job get ${jobid} --json >job.plain.json &&
    jq -e "(has(\"s_expr\") | not)" job.plain.json &&
    jq -e "(has(\"s_defaults\") | not)" job.plain.json &&
    jq -e "(has(\"s_values\") | not)" job.plain.json
'

test_expect_success 'job get accepts every id format and rejects an unknown one' '
    jobid=$(${SUBMIT} pwd) &&
    priorisea job update ${jobid} &&
    for fmt in dec hex kvs dothex words f58 f58plain; do
        priorisea job get ${jobid} --format ${fmt} || return 1
    done &&
    test_must_fail priorisea job get ${jobid} --format not_a_format
'

test_expect_success 'dec format prints a numeric job id' '
    jobid=$(${SUBMIT} pwd) &&
    priorisea job update ${jobid} &&
    priorisea job get ${jobid} --format dec --json >job.dec.json &&
    jq -e ".jobid | test(\"^[0-9]+$\")" job.dec.json
'

test_expect_success 'job list honours limit, order and sort' '
    priorisea formula update job_score --expr "10" &&
    job1=$(${SUBMIT} pwd) &&
    priorisea job update ${job1} &&
    priorisea formula update job_score --expr "90" &&
    job2=$(${SUBMIT} pwd) &&
    priorisea job update ${job2} &&
    priorisea job list --limit 1 --json >lim.json &&
    jq -e "length == 1" lim.json &&
    priorisea job list --sort score --order desc --json >desc.json &&
    jq -e ".[0].score == 90" desc.json &&
    priorisea job list --sort score --order asc --json >asc.json &&
    jq -e ".[0].score == 10" asc.json &&
    priorisea job list ${job1} ${job2} --json >both.json &&
    jq -e "length == 2" both.json
'

test_done

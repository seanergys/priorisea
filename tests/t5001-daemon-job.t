#!/usr/bin/env bash

test_description="send job data directly to the daemon"

. `dirname $0`/sharness.sh

SUBMIT=${SHARNESS_TEST_SRCDIR}/scripts/submit.sh
FLUX_PAYLOAD=${SHARNESS_TEST_SRCDIR}/scripts/flux_payload.py

test_under_flux 1
test_start_priorisea

test_expect_success 'set the job_score formula to a known value' '
    jq -n "{data: {name: \"job_score\", expr: \"42\", defaults: {}}}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.formula.update payload.json
'

test_expect_success 'valid: job.update scores and stores a given job' '
    jobid=$(${SUBMIT} pwd) &&
    jq -n "{data: {new: false, jobids: [\"${jobid}\"]}}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.job.update payload.json &&
    echo "${jobid}" > jobid.txt
'

test_expect_success 'valid: job.get returns the scored job' '
    jobid=$(cat jobid.txt) &&
    jq -n "{data: \"${jobid}\", format: \"f58\"}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.job.get payload.json > result.json &&
    jq -e ".data.userid == $(id -u)" result.json &&
    jq -e ".data.score == 42" result.json &&
    jq -e ".data.s_expr == \"42\"" result.json &&
    jq -e ".data.s_defaults == {}" result.json
'

test_expect_success 'valid: job.list returns the stored job' '
    jq -n "{format: \"f58\", limit: 0}" > payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.job.list payload.json > result.json &&
    jq -e ".data | length == 1" result.json &&
    jq -e ".data[0].userid == $(id -u)" result.json &&
    jq -e ".data[0].score == 42" result.json
'

test_expect_success 'invalid: a non-owner cannot read another user job' '
    jobid=$(cat jobid.txt) &&
    jq -n "{data: \"${jobid}\", format: \"f58\"}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} --userid 424242 priorisea.daemon.job.get payload.json 2>err.txt &&
    grep -q "Access denied to job" err.txt
'

test_expect_success 'invalid: job.get with non-string data fails' '
    jq -n "{data: 123, format: \"f58\"}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.job.get payload.json 2>err.txt
'

test_expect_success 'invalid: job.get for an unparseable job id reports it' '
    jq -n "{data: \"not-a-job\", format: \"f58\"}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.job.get payload.json 2>err.txt &&
    grep -q "Error parsing job id" err.txt
'

test_expect_success 'invalid: job.get for a job not in the database fails' '
    jq -n "{data: \"f583\", format: \"f58\"}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.job.get payload.json 2>err.txt
'

test_expect_success 'invalid: job.update with non-array jobids reports the type' '
    jq -n "{data: {new: false, jobids: \"nope\"}}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.job.update payload.json 2>err.txt &&
    grep -q "Field jobids expected the following type: array" err.txt
'

test_expect_success 'invalid: job.update with a non-string jobid element reports it' '
    jq -n "{data: {new: false, jobids: [123]}}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.job.update payload.json 2>err.txt &&
    grep -q "Field jobids\[0\] expected the following type: string" err.txt
'

test_expect_success 'invalid: job.list with non-array data reports the type' '
    jq -n "{data: \"nope\", format: \"f58\", limit: 0}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.job.list payload.json 2>err.txt &&
    grep -q "Field data expected the following type: array" err.txt
'

test_expect_success 'invalid: job.list with a non-string jobid element reports it' '
    jq -n "{data: [123], format: \"f58\", limit: 0}" > payload.json &&
    test_must_fail ${FLUX_PAYLOAD} priorisea.daemon.job.list payload.json 2>err.txt &&
    grep -q "Field data\[0\] expected the following type: string" err.txt
'

test_done

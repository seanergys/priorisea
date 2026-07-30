#!/usr/bin/env bash

test_description="test flux plugin update many jobs"

. `dirname $0`/sharness.sh
PRIORITY_PLUGIN=${FLUX_BUILD_DIR}/src/plugins/.libs/sea_priority.so
test_under_flux 1

test_expect_success 'basic initialization' '
    flux jobtap load ${PRIORITY_PLUGIN} &&
    priorisea init
'

test_expect_success 'submit 100 small jobs' '
	for i in $(seq 1 100); do
		flux submit hostname >>jobids.txt || return 1
	done
'

test_expect_success 'wait for all 100 jobs to finish' '
	while read jid; do
		flux job wait-event -vt 30 ${jid} clean || return 1
	done <jobids.txt
'

test_expect_success 'score all jobs' '
    priorisea score update --new &&
	priorisea score job $(cat jobids.txt)
'

test_done

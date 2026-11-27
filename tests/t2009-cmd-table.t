#!/usr/bin/env bash

test_description="test command table output"

. `dirname $0`/sharness.sh

FLUX_PAYLOAD=${SHARNESS_TEST_SRCDIR}/scripts/flux_payload.py
PRIORITY_PLUGIN=${SHARNESS_BUILD_DIRECTORY}/lib/flux/job-manager/plugins/sea_priority.so
SUBMIT=${SHARNESS_TEST_SRCDIR}/scripts/submit.sh
FORMULA_GET=${SHARNESS_TEST_SRCDIR}/expected/t2009-cmd-table/formula-get.txt
FORMULA_LIST=${SHARNESS_TEST_SRCDIR}/expected/t2009-cmd-table/formula-list.txt
TIMEWIN_GET=${SHARNESS_TEST_SRCDIR}/expected/t2009-cmd-table/timewin-get.txt
TIMEWIN_LIST=${SHARNESS_TEST_SRCDIR}/expected/t2009-cmd-table/timewin-list.txt
USER_GET=${SHARNESS_TEST_SRCDIR}/expected/t2009-cmd-table/user-get.txt
USER_LIST=${SHARNESS_TEST_SRCDIR}/expected/t2009-cmd-table/user-list.txt
PLUGIN_DEBUG=${SHARNESS_TEST_SRCDIR}/expected/t2009-cmd-table/plugin-debug.txt
DAEMON_DEBUG=${SHARNESS_TEST_SRCDIR}/expected/t2009-cmd-table/daemon-debug.txt

test_under_flux 1
test_fake_user_setup
test_start_priorisea

test_expect_success 'formula get prints the expected table' '
    priorisea formula update job_score "1 + <var:y>" --default "var:y" 20 &&
    priorisea formula get job_score >job_score.txt &&
    test_cmp job_score.txt $FORMULA_GET
'

test_expect_success 'formula list prints the expected table' '
    priorisea formula update job_priority "1 + <var:x>" --default "var:x" 10 &&
    priorisea formula update job_weight "1 + <var:z>" --default "var:z" 30 &&
    priorisea formula list >formulas.txt &&
    test_cmp formulas.txt $FORMULA_LIST
'

test_expect_success 'timewin get prints the expected table' '
    priorisea timewin create carbon -c "0 0 12 * * * *" -d 3600 -s 100 &&
    priorisea timewin get carbon >timewin.txt &&
    test_cmp timewin.txt $TIMEWIN_GET
'

test_expect_success 'timewin list prints the expected table' '
    priorisea timewin create carbon2 -c "0 0 * * * * *" -d 2400 -s 200 &&
    priorisea timewin list >timewins.txt &&
    test_cmp timewins.txt $TIMEWIN_LIST
'

#test_expect_success 'job get prints the expected table' '
#    priorisea formula update job_score "42" &&
#    jobid=$(${SUBMIT} pwd) &&
#    priorisea job update ${jobid} &&
#    priorisea job get ${jobid} >job.txt &&
#    test_cmp job.txt $JOB_GET
#'
#
#test_expect_success 'job list prints the expected table' '
#    priorisea formula update job_score "69" &&
#    jobid=$(${SUBMIT} pwd) &&
#    priorisea job update ${jobid} &&
#    priorisea job list >jobs.txt &&
#    test_cmp jobs.txt $JOB_LIST
#'

test_expect_success 'user get prints the expected table' '
    priorisea formula update job_score "42" &&
    jobid=$(${SUBMIT} pwd) &&
    priorisea job update ${jobid} &&
    priorisea formula update job_score "69" &&
    jobid=$(${SUBMIT} pwd) &&
    priorisea job update ${jobid} &&
    priorisea formula update job_weight "1" &&
    priorisea user update $(whoami) &&
    priorisea user get $(whoami) >user.txt &&
    test_cmp user.txt $USER_GET
'

test_expect_success 'user list prints the expected table' '
    jobid=$(flux_run_fake 424242 hostname) &&
    flux job wait-event ${jobid} clean &&
    priorisea job update ${jobid} &&
    jq -n "{data: [424242]}" >payload.json &&
    ${FLUX_PAYLOAD} priorisea.daemon.user.update payload.json &&
    priorisea user list >users.txt &&
    test_cmp users.txt $USER_LIST
'

test_expect_success 'plugin-debug prints the expected table' '
    flux jobtap load ${PRIORITY_PLUGIN} &&
    priorisea plugin sync &&
    priorisea plugin debug >plugin.txt &&
    test_cmp plugin.txt $PLUGIN_DEBUG
'

test_expect_success 'daemon-debug prints the expected table' '
    priorisea daemon update-user-period 12345 &&
    priorisea daemon debug >daemon.txt &&
    test_cmp daemon.txt $DAEMON_DEBUG
'


test_done

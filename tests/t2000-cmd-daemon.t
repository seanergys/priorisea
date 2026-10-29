#!/usr/bin/env bash

test_description="test daemon command"

. `dirname $0`/sharness.sh

test_under_flux 1
test_start_priorisea

test_expect_success 'daemon debug shows expected info' '
    priorisea daemon debug --json > debug.json &&
    jq -e ".version == 4" debug.json &&
    jq -e ".info.t_inactive == 0" debug.json &&
    jq -e ".info.user_period == 7776000" debug.json
'

test_done

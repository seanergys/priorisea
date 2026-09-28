#!/usr/bin/env bash

test_description="test flux seanerbus configuration"

. `dirname $0`/sharness.sh
PRIORITY_PLUGIN=${SHARNESS_BUILD_DIRECTORY}/src/plugins/.libs/sea_priority.so
XDG_CONFIG_HOME="$HOME/.config"
PRIORISEA_CONFIG_FILE="$XDG_CONFIG_HOME/priorisea/config.json"
DEFAULT_SB_URL="demobus://localhost:5398"
test_under_flux 1

test_expect_success 'basic initialization' '
    flux jobtap load ${PRIORITY_PLUGIN} &&
    priorisea init &&
    mkdir -p $XDG_CONFIG_HOME/priorisea
'

test_expect_success 'check default seanerbus config' '
	priorisea config get --json >out &&
	jq -e "
  .seanerbus == \"${DEFAULT_SB_URL}\"
	" out
'

test_expect_success 'check default seanerbus in plugin' '
	priorisea plugin seanerbus get --json >out &&
	jq -e "
  . == \"${DEFAULT_SB_URL}\"
	" out
'

test_expect_success 'check json config file on CLI with default path' '
    echo "{\"seanerbus\": \"demobus://1.2.3.4:1234\"}" > $PRIORISEA_CONFIG_FILE &&
	priorisea config get --json >out &&
	jq -e "
  .seanerbus == \"demobus://1.2.3.4:1234\"
	" out &&
	rm $PRIORISEA_CONFIG_FILE
'

test_expect_success 'check json config file on CLI with custom path' '
    echo "{\"seanerbus\": \"demobus://1.2.3.4:1234\"}" >config.json &&
	priorisea --config ./config.json config get --json >out &&
	jq -e "
  .seanerbus == \"demobus://1.2.3.4:1234\"
	" out
'

test_expect_success 'check json config file on plugin' '
    echo "{\"seanerbus\": \"demobus://1.2.3.4:1234\"}" > $PRIORISEA_CONFIG_FILE &&
    flux jobtap remove sea_priority.so &&
    flux jobtap load ${PRIORITY_PLUGIN} &&

	priorisea plugin seanerbus get --json >out &&
	jq -e "
  . == \"demobus://1.2.3.4:1234\"
	" out &&
	rm $PRIORISEA_CONFIG_FILE
'

test_expect_success 'check env config' '
    export SEANERBUS_URL="demobus://4.3.2.1:4321"
	priorisea config get --json >out &&
	jq -e "
  .seanerbus == \"demobus://4.3.2.1:4321\"
	" out &&
	unset SEANERBUS_URL
'

test_expect_success 'check precedence of env over file' '
    echo "{\"seanerbus\": \"demobus://1.2.3.4:1234\"}" > $PRIORISEA_CONFIG_FILE &&
    export SEANERBUS_URL="demobus://4.3.2.1:4321"
	priorisea config get --json >out &&
	jq -e "
  .seanerbus == \"demobus://4.3.2.1:4321\"
	" out &&
	rm $PRIORISEA_CONFIG_FILE &&
	unset SEANERBUS_URL
'

test_expect_success 'check direct seanerbus set on plugin' '
    priorisea plugin seanerbus set "demobus://5.6.7.8:5678" &&
	priorisea plugin seanerbus get --json >out &&
	jq -e "
  . == \"demobus://5.6.7.8:5678\"
	" out
'

test_expect_success 'check Flux config precedence' '
    flux config set -t string priorisea.seanerbus "demobus://1.3.5.7:1357" &&
	priorisea plugin seanerbus get --json >out &&
	jq -e "
  . == \"demobus://1.3.5.7:1357\"
	" out
'

test_done

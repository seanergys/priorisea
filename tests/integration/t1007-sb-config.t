#!/usr/bin/env bash

test_description="test flux plugin update many jobs"

. `dirname $0`/sharness.sh
PRIORITY_PLUGIN=${FLUX_BUILD_DIR}/src/plugins/.libs/sea_priority.so
XDG_CONFIG_HOME="$HOME/.config"
PRIORISEA_CONFIG_FILE="$XDG_CONFIG_HOME/priorisea/config.json"
test_under_flux 1

test_expect_success 'basic initialization' '
    flux jobtap load ${PRIORITY_PLUGIN} &&
    priorisea init &&
    mkdir -p $XDG_CONFIG_HOME/priorisea
'

test_expect_success 'check default seanerbus config' '
	priorisea config get --json >out &&
	jq -e "
  .seanerbus.ip == \"localhost\" and
  .seanerbus.port == 5398
	" out
'

test_expect_success 'check default seanerbus in plugin' '
	priorisea plugin seanerbus get --json >out &&
	jq -e "
  .ip == \"localhost\" and
  .port == 5398
	" out
'

test_expect_success 'check json config file on CLI with default path' '
    echo "{\"seanerbus\": {\"ip\": \"1.2.3.4\",\"port\": 1234}}" > $PRIORISEA_CONFIG_FILE &&
	priorisea config get --json >out &&
	jq -e "
  .seanerbus.ip == \"1.2.3.4\" and
  .seanerbus.port == 1234
	" out &&
	rm $PRIORISEA_CONFIG_FILE
'

test_expect_success 'check json config file on CLI with custom path' '
    echo "{\"seanerbus\": {\"ip\": \"1.2.3.4\",\"port\": 1234}}" >config.json &&
	priorisea --config ./config.json config get --json >out &&
	jq -e "
  .seanerbus.ip == \"1.2.3.4\" and
  .seanerbus.port == 1234
	" out
'

test_expect_success 'check json config file on plugin' '
    echo "{\"seanerbus\": {\"ip\": \"1.2.3.4\",\"port\": 1234}}" > $PRIORISEA_CONFIG_FILE &&
    flux jobtap remove sea_priority.so &&
    flux jobtap load ${PRIORITY_PLUGIN} &&

	priorisea plugin seanerbus get --json >out &&
	jq -e "
  .ip == \"1.2.3.4\" and
  .port == 1234
	" out &&
	rm $PRIORISEA_CONFIG_FILE
'

test_expect_success 'check env config' '
    export SEANERBUS_IP=4.3.2.1
    export SEANERBUS_PORT=4321
	priorisea config get --json >out &&
	jq -e "
  .seanerbus.ip == \"4.3.2.1\" and
  .seanerbus.port == 4321
	" out &&
	unset SEANERBUS_IP SEANERBUS_PORT
'

test_expect_success 'check precedence of env over file' '
    echo "{\"seanerbus\": {\"ip\": \"1.2.3.4\",\"port\": 1234}}" > $PRIORISEA_CONFIG_FILE &&
    export SEANERBUS_IP=4.3.2.1
    export SEANERBUS_PORT=4321
	priorisea config get --json >out &&
	jq -e "
  .seanerbus.ip == \"4.3.2.1\" and
  .seanerbus.port == 4321
	" out &&
	rm $PRIORISEA_CONFIG_FILE &&
	unset SEANERBUS_IP SEANERBUS_PORT
'

test_expect_success 'check direct seanerbus set on plugin' '
    priorisea plugin seanerbus set "5.6.7.8" 5678 &&
	priorisea plugin seanerbus get --json >out &&
	jq -e "
  .ip == \"5.6.7.8\" and
  .port == 5678
	" out
'

test_done

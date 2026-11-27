#!/usr/bin/env bash

test_description="test bash completion coverage"

. "$(dirname "$0")/sharness.sh"

COMPLETION_SCRIPT="${SHARNESS_BUILD_DIRECTORY}/src/cli/priorisea.bash"
. "$COMPLETION_SCRIPT"

#
# _comp_run <cword> <word>... -- simulate a completion request at the given
# cursor position and store the proposed completions in COMPREPLY
#
_comp_run() {
	local cword=$1
	shift
	COMP_WORDS=("$@")
	COMP_CWORD=$cword
	COMPREPLY=()
	_priorisea
}

#
# _comp_has <token>... -- succeed only if every token is present in COMPREPLY
#
_comp_has() {
	local out token
	out=$(printf '%s\n' "${COMPREPLY[@]}")
	for token in "$@"; do
		printf '%s\n' "$out" | grep -Fxq -- "$token" || return 1
	done
}

test_expect_success 'completion script is present' '
	test -f "$COMPLETION_SCRIPT"
'

test_expect_success 'top level lists all subcommands and global options' '
	_comp_run 1 priorisea &&
	_comp_has daemon database formula job plugin timewin user --help
'

test_expect_success 'top level does not advertise unknown subcommands' '
	_comp_run 1 priorisea &&
	! _comp_has unknown
'

# daemon subcommands
test_expect_success 'daemon subcommands' '
	_comp_run 2 priorisea daemon &&
	_comp_has start debug update-user-period --help
'

test_expect_success 'daemon start flags' '
	_comp_run 3 priorisea daemon start &&
	_comp_has --seanerbus --background --help
'

test_expect_success 'daemon debug flags' '
	_comp_run 3 priorisea daemon debug &&
	_comp_has --json --help
'

test_expect_success 'daemon update-user-period flags' '
	_comp_run 3 priorisea daemon update-user-period &&
	_comp_has --help
'

# database subcommands
test_expect_success 'database subcommands' '
	_comp_run 2 priorisea database &&
	_comp_has create --help
'

test_expect_success 'database create flags' '
	_comp_run 3 priorisea database create &&
	_comp_has --help
'

# formula subcommands
test_expect_success 'formula subcommands' '
	_comp_run 2 priorisea formula &&
	_comp_has get list update reset --help
'

test_expect_success 'formula get flags' '
	_comp_run 3 priorisea formula get &&
	_comp_has --json --help
'

test_expect_success 'formula list flags' '
	_comp_run 3 priorisea formula list &&
	_comp_has --json --help
'

test_expect_success 'formula update flags' '
	_comp_run 3 priorisea formula update &&
	_comp_has --default --help
'

test_expect_success 'formula reset flags' '
	_comp_run 3 priorisea formula reset &&
	_comp_has --help
'

# job subcommands
test_expect_success 'job subcommands' '
	_comp_run 2 priorisea job &&
	_comp_has get list update --help
'

test_expect_success 'job get flags' '
	_comp_run 3 priorisea job get &&
	_comp_has --json --detail --format --help
'

test_expect_success 'job list flags' '
	_comp_run 3 priorisea job list &&
	_comp_has --json --detail --format --limit --order --sort --help
'

test_expect_success 'job update flags' '
	_comp_run 3 priorisea job update &&
	_comp_has --new --help
'

test_expect_success 'job get --format values' '
	_comp_run 4 priorisea job get --format &&
	_comp_has dec hex kvs dothex words f58 f58plain
'

test_expect_success 'job list --format values' '
	_comp_run 4 priorisea job list --format &&
	_comp_has dec hex kvs dothex words f58 f58plain
'

test_expect_success 'job list --order values' '
	_comp_run 4 priorisea job list --order &&
	_comp_has asc desc
'

test_expect_success 'job list --sort values' '
	_comp_run 4 priorisea job list --sort &&
	_comp_has userid t_inactive score duration
'

# plugin subcommands
test_expect_success 'plugin subcommands' '
	_comp_run 2 priorisea plugin &&
	_comp_has sync debug --help
'

test_expect_success 'plugin sync flags' '
	_comp_run 3 priorisea plugin sync &&
	_comp_has --help
'

test_expect_success 'plugin debug flags' '
	_comp_run 3 priorisea plugin debug &&
	_comp_has --json --help
'

# timewin subcommands
test_expect_success 'timewin subcommands' '
	_comp_run 2 priorisea timewin &&
	_comp_has get list update create delete --help
'

test_expect_success 'timewin get flags' '
	_comp_run 3 priorisea timewin get &&
	_comp_has --json --help
'

test_expect_success 'timewin list flags' '
	_comp_run 3 priorisea timewin list &&
	_comp_has --json --help
'

test_expect_success 'timewin update flags' '
	_comp_run 3 priorisea timewin update &&
	_comp_has --cron --duration --score --help
'

test_expect_success 'timewin create flags' '
	_comp_run 3 priorisea timewin create &&
	_comp_has --cron --duration --score --help
'

test_expect_success 'timewin delete flags' '
	_comp_run 3 priorisea timewin delete &&
	_comp_has --help
'

# user subcommands
test_expect_success 'user subcommands' '
	_comp_run 2 priorisea user &&
	_comp_has get list update --help
'

test_expect_success 'user get flags' '
	_comp_run 3 priorisea user get &&
	_comp_has --detail --json --help
'

test_expect_success 'user list flags' '
	_comp_run 3 priorisea user list &&
	_comp_has --json --order --help
'

test_expect_success 'user list --order values' '
	_comp_run 4 priorisea user list --order &&
	_comp_has asc desc
'

test_expect_success 'user update flags' '
	_comp_run 3 priorisea user update &&
	_comp_has --all --help
'

test_done

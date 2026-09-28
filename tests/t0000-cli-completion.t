#!/usr/bin/env bash

test_description="test priorisea bash completion coverage"

. "$(dirname "$0")/sharness.sh"

COMPLETION_SCRIPT="${SHARNESS_BUILD_DIRECTORY}/src/cli/priorisea.bash"
. "$COMPLETION_SCRIPT"

#
# _comp_run <cword> <word>... -- simulate a completion request at the given
# cursor position and store the proposed completions in COMPREPLY.
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
# _comp_has <token>... -- succeed only if every token is present in COMPREPLY.
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
	_comp_has init config formula decay score timewin plugin --config -h --help
'

# config subcommands
test_expect_success 'config subcommands' '
	_comp_run 2 priorisea config &&
	_comp_has get export -h --help
'

test_expect_success 'config get flags' '
	_comp_run 3 priorisea config get &&
	_comp_has --json -h --help
'

## export not tested since it requires a file path

# formula subcommands
test_expect_success 'formula subcommands' '
	_comp_run 2 priorisea formula &&
	_comp_has init list get set reset -h --help
'

test_expect_success 'formula init flags' '
	_comp_run 3 priorisea formula init &&
	_comp_has -h --help
'

test_expect_success 'formula list flags' '
	_comp_run 3 priorisea formula list &&
	_comp_has --json -h --help
'

test_expect_success 'formula get flags' '
	_comp_run 3 priorisea formula get &&
	_comp_has -h --help --json
'

test_expect_success 'formula set flags' '
	_comp_run 3 priorisea formula set &&
	_comp_has -d --default --json -h --help
'

test_expect_success 'formula reset flags' '
	_comp_run 3 priorisea formula reset &&
	_comp_has -h --help
'

# decay subcommands
test_expect_success 'decay subcommands' '
	_comp_run 2 priorisea decay &&
	_comp_has init get set reset -h --help
'

test_expect_success 'decay init flags' '
	_comp_run 3 priorisea decay init &&
	_comp_has -h --help
'

test_expect_success 'decay get flags' '
	_comp_run 3 priorisea decay get &&
	_comp_has --json -h --help
'

test_expect_success 'decay set flags' '
	_comp_run 3 priorisea decay set &&
	_comp_has --json -h --help
'

test_expect_success 'decay reset flags' '
	_comp_run 3 priorisea decay reset &&
	_comp_has -h --help
'

# score subcommands
test_expect_success 'score subcommands' '
	_comp_run 2 priorisea score &&
	_comp_has user job update init -h --help
'

test_expect_success 'score job flags' '
	_comp_run 3 priorisea score job &&
	_comp_has --json --all -n --limit --id-format -h --help
'

test_expect_success 'score job --id-format values' '
	_comp_run 4 priorisea score job --id-format &&
	_comp_has dec hex kvs dothex words f58 f58plain
'

test_expect_success 'score update flags' '
	_comp_run 3 priorisea score update &&
	_comp_has --new -h --help
'

test_expect_success 'score init flags' '
	_comp_run 3 priorisea score init &&
	_comp_has -h --help
'

# timewin subcommands
test_expect_success 'timewin subcommands' '
	_comp_run 2 priorisea timewin &&
	_comp_has list get add delete edit -h --help
'

test_expect_success 'timewin list flags' '
	_comp_run 3 priorisea timewin list &&
	_comp_has --json -h --help
'

test_expect_success 'timewin get flags' '
	_comp_run 3 priorisea timewin get &&
	_comp_has -j --json -h --help
'

test_expect_success 'timewin add flags' '
	_comp_run 3 priorisea timewin add &&
	_comp_has -s --score -c --cron -d --duration --json -h --help
'

test_expect_success 'timewin delete flags' '
	_comp_run 3 priorisea timewin delete &&
	_comp_has -h --help
'

test_expect_success 'timewin edit flags' '
	_comp_run 3 priorisea timewin edit &&
	_comp_has -s --score -c --cron -d --duration --json -h --help
'

# plugin subcommands
test_expect_success 'plugin subcommands' '
	_comp_run 2 priorisea plugin &&
	_comp_has seanerbus -h --help
'

test_expect_success 'plugin seanerbus subcommands' '
	_comp_run 3 priorisea plugin seanerbus &&
	_comp_has get set -h --help
'

test_expect_success 'plugin seanerbus get flags' '
	_comp_run 4 priorisea plugin seanerbus get &&
	_comp_has --json -h --help
'

test_expect_success 'plugin seanerbus set flags' '
	_comp_run 4 priorisea plugin seanerbus set &&
	_comp_has -h --help
'

test_expect_success 'top level does not advertise unknown subcommands' '
	_comp_run 1 priorisea &&
	! _comp_has unknown
'

test_done

#!/usr/bin/env bash

test_description="test database command"

. `dirname $0`/sharness.sh

test_expect_success 'create the database' '
    priorisea database create ./priorisea.db
'

test_expect_success 'create the database twice' '
    test_must_fail priorisea database create ./priorisea.db
'

test_expect_success 'database has the expected schema version' '
    echo 4 >expected_version.out &&
    sqlite3 ./priorisea.db "PRAGMA user_version;" >version.out &&
    test_cmp expected_version.out version.out
'

test_expect_success 'database has all expected tables' '
    cat >expected_tables.out <<EOF
database_info
formulas
jobs
timewins
users
EOF
    sqlite3 ./priorisea.db "SELECT name FROM sqlite_master WHERE type = '"'"'table'"'"' ORDER BY name;" >tables.out &&
    test_cmp expected_tables.out tables.out
'

test_done

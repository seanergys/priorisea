#
#  Global helpers to create the priorisea database and start the priorisea
#  daemon during sharness tests. Mirrors seanerbus.sh: the daemon is launched
#  in the background, its PID is recorded, and it is reaped on every exit path
#  so it can never outlive the test.
#
#  Usage:
#
#      test_start_priorisea                          # db in trash dir,
#                                                    #   seanerbus from $SEANERBUS_URL
#      test_start_priorisea /path/to.db              # custom database path
#      test_start_priorisea /path/to.db URL          # custom db + seanerbus url
#
#  Requires a running flux instance (test_under_flux); the daemon also connects
#  to seanerbus, so start that first (test_start_seanerbus).

#
#  Default SeanerBUS endpoint the daemon connects to when $SEANERBUS_URL is
#  unset, matching the priorisea CLI default.
#
: "${SEANERBUS_ENDPOINT_DEFAULT:=demobus://localhost:5398}"

#
#  Default path of the test database: inside the trash directory so it is
#  removed with the test's scratch space (never the real /var/lib path).
#
priorisea_test_db() {
    echo "${PRIORISEA_TEST_DB:-$SHARNESS_TRASH_DIRECTORY/priorisea.db}"
}

#
#  Stop the priorisea daemon started by test_start_priorisea and remove its
#  pid/out files. Idempotent and safe to call when no daemon is running.
#
test_stop_priorisea() {
    rc=$?
    if test -f priorisea-daemon.pid; then
        kill "$(cat priorisea-daemon.pid)" 2>/dev/null
        rm -f priorisea-daemon.pid priorisea-daemon.out
    fi
    return "$rc"
}

#
#  Create the priorisea database and start the daemon in the background, wait
#  until it answers `priorisea daemon debug`, and install an EXIT trap so the
#  daemon is killed on every exit path - including a failing assertion, which
#  sharness' test_done does not run the cleanup for.
#
#  The daemon's stdout/stderr/stdin are fully redirected so it never touches
#  the harness' fd 3/4 (which would corrupt TAP output or hang the harness
#  waiting for EOF). The forked PID is read back from the CLI's own
#  "Daemon started in background with PID N" line (--background).
#
#  $1 - database path (optional, default trash dir)
#  $2 - seanerbus url (optional, default $SEANERBUS_URL or the priorisea default)
#
test_start_priorisea() {
    db_path=${1:-$(priorisea_test_db)}
    sb_url=${2:-${SEANERBUS_URL:-$SEANERBUS_ENDPOINT_DEFAULT}}

    if ! priorisea database create "$db_path"; then
        error "could not create priorisea database at $db_path"
    fi

    # `--background` forks the daemon and prints the child PID on stdout; match
    # that specific line so a stray "PID N" on stderr can't be captured instead.
    priorisea daemon start "$db_path" --seanerbus "$sb_url" --background \
        >priorisea-daemon.out 2>&1 </dev/null

    daemon_pid=$(sed -n 's/.*background with PID \([0-9][0-9]*\).*/\1/p' priorisea-daemon.out | head -n1)
    if test -z "$daemon_pid"; then
        error "priorisea daemon did not start (no PID reported); \
see priorisea-daemon.out: $(sed 's/^/    /' priorisea-daemon.out)"
    fi
    echo "$daemon_pid" >priorisea-daemon.pid
    echo "# test_start_priorisea: started priorisea daemon pid $daemon_pid on $sb_url" >&5

    # Reap on every exit path, mirroring test_stop_seanerbus:
    #  * cleanup() runs at test_done only when every test passes, and crucially
    #    before the trash dir (and pid file) is removed, so the pid is readable.
    #  * the EXIT trap covers the failing path, where test_done skips cleanup.
    cleanup test_stop_priorisea
    trap 'test_stop_priorisea; die' EXIT

    # Wait until the daemon answers so the first test command sees a live one,
    # not a half-started process.
    for _ in $(seq 1 50); do
        if priorisea daemon debug >/dev/null 2>&1; then
            return 0
        fi
        if ! kill -0 "$daemon_pid" 2>/dev/null; then
            error "priorisea daemon (pid $daemon_pid) exited early; \
see priorisea-daemon.out: $(sed 's/^/    /' priorisea-daemon.out)"
        fi
        sleep 0.1
    done
    error "priorisea daemon did not become ready on $sb_url (pid $daemon_pid); \
see priorisea-daemon.out: $(sed 's/^/    /' priorisea-daemon.out)"
}

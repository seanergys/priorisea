#
#  Global helpers to run a demobus (seanerbus) server during sharness tests.
#
#
#  Usage:
#
#      test_start_seanerbus          # start a server on the default port 5398
#      test_start_seanerbus 6000     # start a server on a custom port
#
#  The launch command is taken from $SEANERBUS_BIN when set, otherwise the
#  installed demobus binary is used. demobus takes a single "HOST:PORT"
#  positional argument (e.g. 0.0.0.0:5398); the helper builds it from the
#  port. Point SEANERBUS_BIN at however demobus is built in your image, e.g.:

#
#  Default TCP port the seanerbus server listens on
#
: "${SEANERBUS_PORT:=5398}"

#
#  Return 0 if a TCP port is already accepting connections, 1 otherwise.
#  Single-shot (unlike test_wait_for_port): meant to detect a leftover
#  listener quickly, before we try to bind the port ourselves.
#
#  $1 - port
#
test_port_in_use() {
    (exec 3<>/dev/tcp/127.0.0.1/"$1") 2>/dev/null
}

#
#  Block until a TCP port accepts connections, or fail after ~5s.
#
#  $1 - port
#
test_wait_for_port() {
    port=$1
    for _ in $(seq 1 50); do
        if (exec 3<>/dev/tcp/"127.0.0.1"/"$port") 2>/dev/null; then
            return 0
        fi
        sleep 0.1
    done
    return 1
}

#
#  Stop the demobus seanerbus server started by test_start_seanerbus and
#  remove its pid/out files. Idempotent and safe to call when no server is
#  running.
#
test_stop_seanerbus() {
    rc=$?
    if test -f seanerbus.pid; then
        kill "$(cat seanerbus.pid)" 2>/dev/null
        rm -f seanerbus.pid seanerbus.out
    fi
    return "$rc"
}

#
#  Start a demobus seanerbus server in the background on the given port
#  (default $SEANERBUS_PORT, i.e. 5398), wait until it is reachable, and
#  install an EXIT trap so it is killed on every exit path - including a
#  failing assertion, which sharness' test_done does not run the cleanup for.
#
#  The server's stdout/stderr/stdin are fully redirected so the daemon never
#  touches the harness' fd 3/4 (which would corrupt TAP output or hang the
#  harness waiting for EOF).
#
#  $1 - port (optional)
#
test_start_seanerbus() {
    port=${1:-$SEANERBUS_PORT}

    if test -z "$SEANERBUS_BIN" && ! test -x /usr/local/bin/demobus; then
        error "SEANERBUS_BIN is not set and /usr/local/bin/demobus is missing; \
set SEANERBUS_BIN to the demobus launch command (see sharness.d/seanerbus.sh)"
    fi

    # Refuse to start if the port is already taken.
    if test_port_in_use "$port"; then
        error "port $port already in use before starting seanerbus; a leftover \
demobus is still running - kill it (e.g. pkill -f demobus) or choose another \
port with 'test_start_seanerbus <port>'"
    fi

    # demobus takes a single "HOST:PORT" positional argument.
    addr="0.0.0.0:$port"

    if test -n "$SEANERBUS_BIN"; then
        # Intentionally unquoted: SEANERBUS_BIN holds the command + its flags.
        # shellcheck disable=SC2086
        $SEANERBUS_BIN "$addr" >seanerbus.out 2>&1 </dev/null &
    else
        /usr/local/bin/demobus "$addr" >seanerbus.out 2>&1 </dev/null &
    fi
    echo $! >seanerbus.pid
    echo "# test_start_seanerbus: started demobus pid $(cat seanerbus.pid) on $addr" >&5

    # Reap the server on every exit path, via two complementary mechanisms:
    #  * cleanup() runs at test_done only when every test passes, but crucially
    #    it runs BEFORE sharness deletes the test's trash directory - where
    #    seanerbus.pid lives - so the pid file is still there to be read.
    #  * the EXIT trap covers the failing path: test_done's "failed" branch
    #    skips cleanup() and leaves the trash dir (and pid file) in place, so
    #    the trap can still find and kill the server. It chains onto sharness'
    #    own 'die' trap; test_stop_seanerbus returns the captured status so
    #    die still sees the right exit code.
    cleanup test_stop_seanerbus
    trap 'test_stop_seanerbus; die' EXIT

    if ! test_wait_for_port "$port"; then
        error "seanerbus server did not start listening on port $port (see seanerbus.out)"
    fi

    # Confirm the server we started is the one still alive.
    if ! kill -0 "$(cat seanerbus.pid)" 2>/dev/null; then
        error "seanerbus server (pid $(cat seanerbus.pid)) exited early; seanerbus.out says:\
$(sed 's/^/    /' seanerbus.out)"
    fi
}

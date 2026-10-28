#
#  Global helpers to run a demobus server during sharness tests.
#
#
#  Usage:
#
#      test_start_demobus <url>   # start a server on the given seanerbus url
#
#  The url is a mandatory positional argument (e.g. demobus://127.0.0.1:3003);
#  there is no default, so the caller must always pass one. The launch command
#  is taken from $DEMOBUS_BIN, which must point at the demobus binary (plus any
#  flags). demobus takes a single "HOST:PORT" positional argument; the helper
#  derives it from the url by stripping the demobus:// scheme. Point DEMOBUS_BIN
#  at however demobus is built in your image.

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
#  Stop the demobus server started by test_start_demobus and remove its
#  pid/out files. Idempotent and safe to call when no server is running.
#
test_stop_demobus() {
    rc=$?
    if test -f demobus.pid; then
        kill "$(cat demobus.pid)" 2>/dev/null
        rm -f demobus.pid demobus.out
    fi
    return "$rc"
}

#
#  Start a demobus server in the background on the given url, wait until it is
#  reachable, and install an EXIT trap so it is killed on every exit path -
#  including a failing assertion, which sharness' test_done does not run the
#  cleanup for.
#
#  The server's stdout/stderr/stdin are fully redirected so the daemon never
#  touches the harness' fd 3/4 (which would corrupt TAP output or hang the
#  harness waiting for EOF).
#
#  The launch command comes solely from $DEMOBUS_BIN; set it to however
#  demobus is built in your image.
#
#  $1 - seanerbus url to bind (mandatory, demobus://host:port)
#
test_start_demobus() {
    url=$1

    if test -z "$url"; then
        error "test_start_demobus: missing seanerbus url; pass it as the first \
argument, e.g. demobus://127.0.0.1:3003 (see sharness.d/demobus.sh)"
    fi

    if test -z "$DEMOBUS_BIN"; then
        error "DEMOBUS_BIN is not set; set it to the demobus launch command \
(see sharness.d/demobus.sh)"
    fi

    # Strip the demobus:// scheme to get the host:port demobus binds, then the
    # port to wait for.
    addr=${url#demobus://}
    port=${addr##*:}

    # Refuse to start if the port is already taken.
    if test_port_in_use "$port"; then
        error "port $port already in use before starting demobus; a leftover \
demobus is still running - kill it (e.g. pkill -f demobus) or choose another \
url with 'test_start_demobus demobus://0.0.0.0:<port>'"
    fi

    # demobus takes a single "HOST:PORT" positional argument (already derived
    # from the url above as $addr).

    # Intentionally unquoted: DEMOBUS_BIN holds the command + its flags.
    # shellcheck disable=SC2086
    $DEMOBUS_BIN "$addr" >demobus.out 2>&1 </dev/null &
    echo $! >demobus.pid
    echo "# test_start_demobus: started demobus pid $(cat demobus.pid) on $addr" >&5

    # Reap the server on every exit path, via two complementary mechanisms:
    #  * cleanup() runs at test_done only when every test passes, but crucially
    #    it runs BEFORE sharness deletes the test's trash directory - where
    #    demobus.pid lives - so the pid file is still there to be read.
    #  * the EXIT trap covers the failing path: test_done's "failed" branch
    #    skips cleanup() and leaves the trash dir (and pid file) in place, so
    #    the trap can still find and kill the server. It chains onto sharness'
    #    own 'die' trap; test_stop_demobus returns the captured status so
    #    die still sees the right exit code.
    cleanup test_stop_demobus
    trap 'test_stop_demobus; die' EXIT

    if ! test_wait_for_port "$port"; then
        error "demobus server did not start listening on port $port (see demobus.out)"
    fi

    # Confirm the server we started is the one still alive.
    if ! kill -0 "$(cat demobus.pid)" 2>/dev/null; then
        error "demobus server (pid $(cat demobus.pid)) exited early; demobus.out says:\
$(sed 's/^/    /' demobus.out)"
    fi
}

#
#  Global helpers to run a seanerbroker server during sharness tests.
#
#
#  Usage:
#
#      test_start_seanerbroker <url>   # start a server on the given seanerbus url
#
#  The url is a mandatory positional argument (e.g. seanerbroker://127.0.0.1:3003)
#  The launch command is taken from $SEANERBROKER_BIN, which must point at the seanerbroker binary.
#  seanerbroker takes a single "HOST:PORT" positional argument; the helper
#  derives it from the url by stripping the seanerbroker:// scheme.

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
#  Stop the seanerbroker server started by test_start_seanerbroker and remove its
#  pid/out files. Idempotent and safe to call when no server is running.
#
test_stop_seanerbroker() {
    rc=$?
    if test -f seanerbroker.pid; then
        kill "$(cat seanerbroker.pid)" 2>/dev/null
        rm -f seanerbroker.pid seanerbroker.out
    fi
    return "$rc"
}

#
#  Start a seanerbroker server in the background on the given url, wait until it is
#  reachable, and install an EXIT trap so it is killed on every exit path -
#  including a failing assertion, which sharness' test_done does not run the
#  cleanup for.
#
#  The server's stdout/stderr/stdin are fully redirected so the daemon never
#  touches the harness' fd 3/4 (which would corrupt TAP output or hang the
#  harness waiting for EOF).
#
#  The launch command comes solely from $SEANERBROKER_BIN; set it to however
#  seanerbroker is built in your image.
#
#  $1 - seanerbus url to bind (mandatory, seanerbroker://host:port)
#
test_start_seanerbroker() {
    url=$1

    if test -z "$url"; then
        error "test_start_seanerbroker: missing seanerbus url; pass it as the first \
argument, e.g. seanerbroker://127.0.0.1:3003 (see sharness.d/seanerbroker.sh)"
    fi

    if test -z "$SEANERBROKER_BIN"; then
        error "SEANERBROKER_BIN is not set; set it to the seanerbroker launch command \
(see sharness.d/seanerbroker.sh)"
    fi

    # Strip the seanerbroker:// scheme to get the host:port seanerbroker binds, then the
    # port to wait for.
    addr=${url#seanerbroker://}
    port=${addr##*:}

    # Refuse to start if the port is already taken.
    if test_port_in_use "$port"; then
        error "port $port already in use before starting seanerbroker; a leftover \
seanerbroker is still running - kill it (e.g. pkill -f seanerbroker) or choose another \
url with 'test_start_seanerbroker seanerbroker://0.0.0.0:<port>'"
    fi

    # seanerbroker takes a single "HOST:PORT" positional argument (already derived
    # from the url above as $addr).

    # Intentionally unquoted: SEANERBROKER_BIN holds the command + its flags.
    # shellcheck disable=SC2086
    $SEANERBROKER_BIN "$addr" >seanerbroker.out 2>&1 </dev/null &
    echo $! >seanerbroker.pid
    echo "# test_start_seanerbroker: started seanerbroker pid $(cat seanerbroker.pid) on $addr" >&5

    # Reap the server on every exit path, via two complementary mechanisms:
    #  * cleanup() runs at test_done only when every test passes, but crucially
    #    it runs BEFORE sharness deletes the test's trash directory - where
    #    seanerbroker.pid lives - so the pid file is still there to be read.
    #  * the EXIT trap covers the failing path: test_done's "failed" branch
    #    skips cleanup() and leaves the trash dir (and pid file) in place, so
    #    the trap can still find and kill the server. It chains onto sharness'
    #    own 'die' trap; test_stop_seanerbroker returns the captured status so
    #    die still sees the right exit code.
    cleanup test_stop_seanerbroker
    trap 'test_stop_seanerbroker; die' EXIT

    if ! test_wait_for_port "$port"; then
        error "seanerbroker server did not start listening on port $port (see seanerbroker.out)"
    fi

    # Confirm the server we started is the one still alive.
    if ! kill -0 "$(cat seanerbroker.pid)" 2>/dev/null; then
        error "seanerbroker server (pid $(cat seanerbroker.pid)) exited early; seanerbroker.out says:\
$(sed 's/^/    /' seanerbroker.out)"
    fi
}

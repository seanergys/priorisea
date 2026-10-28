#
#  Global helpers to run the sb_response seanerbus client during sharness tests.
#
#
#  Usage:
#
#      test_start_sb_response <uuid> <message> <url>   # start the response client
#      test_stop_sb_response                           # stop it (idempotent)
#
#  sb_response is built from the repo (tests/scripts/sb_response.cpp) into
#  ${SHARNESS_BUILD_DIRECTORY}/tests/scripts/sb_response, so - unlike the
#  external demobus and naming-server binaries - it is always present after a
#  build and needs no optional-dependency check. It connects to the seanerbus
#  url (e.g. demobus://127.0.0.1:3003) that
#  test_start_demobus has already brought up.

#
#  Stop the response client started by test_start_sb_response and remove its
#  pid/out files. Idempotent and safe to call when no client is running.
#
test_stop_sb_response() {
    rc=$?
    if test -f sb_response.pid; then
        kill "$(cat sb_response.pid)" 2>/dev/null
        rm -f sb_response.pid sb_response.out
    fi
    return "$rc"
}

#
#  Start the sb_response client in the background, registering the given entity
#  and replying to its first HelloWorldReqV1 with the given message.
#
#  The client's stdout/stderr/stdin are fully redirected so it never touches
#  the harness' fd 3/4 (which would corrupt TAP output or hang the harness
#  waiting for EOF).
#
#  $1 - UUID (entity) to register
#  $2 - reply message
#  $3 - seanerbus url to connect to (mandatory, demobus://host:port)
#
test_start_sb_response() {
    entity=$1
    message=$2
    url=$3

    if test -z "$entity"; then
        error "test_start_sb_response: missing uuid (first argument) \
(see sharness.d/sb_response.sh)"
    fi
    if test -z "$message"; then
        error "test_start_sb_response: missing reply message (second argument) \
(see sharness.d/sb_response.sh)"
    fi
    if test -z "$url"; then
        error "test_start_sb_response: missing seanerbus url (third argument); \
pass e.g. demobus://127.0.0.1:3003 (see sharness.d/sb_response.sh)"
    fi

    sb_response_bin="${SHARNESS_BUILD_DIRECTORY}/tests/scripts/sb_response"
    if ! test -x "$sb_response_bin"; then
        error "sb_response client not built at $sb_response_bin; build the \
project first (tests/scripts/sb_response.cpp)"
    fi

    # Parse host and port from the url; the client takes them as positional
    # ip/port arguments.
    addr=${url#demobus://}
    host=${addr%:*}
    port=${addr##*:}

    "$sb_response_bin" "$host" "$port" "$entity" "$message" >sb_response.out 2>&1 </dev/null &
    echo $! >sb_response.pid
    echo "# test_start_sb_response: started sb_response pid $(cat sb_response.pid) for entity $entity on $url" >&5

    # Confirm the client we started did not exit immediately (e.g. it could
    # not reach the demobus).
    if ! kill -0 "$(cat sb_response.pid)" 2>/dev/null; then
        error "sb_response client (pid $(cat sb_response.pid)) exited early; sb_response.out says:\
$(sed 's/^/    /' sb_response.out)"
    fi
}

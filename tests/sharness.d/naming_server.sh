#
#  Global helpers to run a seanerbus naming server during sharness tests.
#
#
#  Usage:
#
#      test_start_naming_server <rpc-uuid> <url>   # register + start it
#      test_stop_naming_server                     # stop the server (idempotent)
#
#  The launch command is taken from $NAMING_SERVER_BIN, which must point at
#  the naming server binary (plus any flags). The server reads its service
#  registration from services.json in the current (trash) directory, which the
#  helper writes from the entity (RPC UUID) passed as its first argument, and
#  connects to the seanerbus url passed as its second argument (the naming
#  server takes that url positionally). Point NAMING_SERVER_BIN at however the
#  naming server is built in your image, e.g.
#  NAMING_SERVER_BIN=/opt/seanerbus/bin/naming_server.
#
#  Unlike the seanerbroker helper (a single server for the whole file), the naming
#  server is started and stopped per test, so cleanup is left to the caller's
#  test_when_finished - mirroring the sb_response helpers.

#
#  Stop the naming server started by test_start_naming_server and remove its
#  pid/out files. Idempotent and safe to call when no server is running.
#
test_stop_naming_server() {
    rc=$?
    if test -f naming_server.pid; then
        kill "$(cat naming_server.pid)" 2>/dev/null
        rm -f naming_server.pid naming_server.out
    fi
    return "$rc"
}

#
#  Register a service and start the naming server in the background.
#
#  The server's stdout/stderr/stdin are fully redirected so it never touches
#  the harness' fd 3/4 (which would corrupt TAP output or hang the harness
#  waiting for EOF).
#
#  $1 - RPC UUID (entity) to register in services.json
#  $2 - seanerbus url the server connects to (mandatory, seanerbroker://host:port)
#
test_start_naming_server() {
    entity=$1
    url=$2

    if test -z "$url"; then
        error "test_start_naming_server: missing seanerbus url; pass it as the \
second argument, e.g. seanerbroker://127.0.0.1:3003 (see sharness.d/naming_server.sh)"
    fi

    if test -z "$NAMING_SERVER_BIN"; then
        error "NAMING_SERVER_BIN is not set; set it to the naming server \
launch command (see sharness.d/naming_server.sh)"
    fi

    # The naming server reads its service registration from services.json in
    # the current (trash) directory.
    echo "[{\"entity\":\"$entity\", \"name\":{\"service\": \"test\"}}]" > services.json

    # The naming server takes the endpoint as a positional argument, without
    # the seanerbroker:// scheme - crop it out of the url to get host:port.
    addr=${url#seanerbroker://}

    # Intentionally unquoted: NAMING_SERVER_BIN holds the command + its flags.
    # shellcheck disable=SC2086
    $NAMING_SERVER_BIN "$addr" >naming_server.out 2>&1 </dev/null &
    echo $! >naming_server.pid
    echo "# test_start_naming_server: started naming server pid $(cat naming_server.pid) on $url" >&5

    # Confirm the server we started did not exit immediately.
    if ! kill -0 "$(cat naming_server.pid)" 2>/dev/null; then
        error "naming server (pid $(cat naming_server.pid)) exited early; naming_server.out says:\
$(sed 's/^/    /' naming_server.out)"
    fi
}

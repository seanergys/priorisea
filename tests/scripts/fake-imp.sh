#!/bin/bash
#
# fake-imp.sh - minimal stand-in for flux-imp(1) used for fake-user
#  (multiuser) jobs in the test suite.
#
# job-exec invokes it as:
#     fake-imp.sh exec <job-shell> <jobid>
#     fake-imp.sh kill <signal> <pid> ...
#
# Unlike the real, setuid-root IMP this helper does NOT change userid, so the
# job's processes run as the invoking (instance owner) user while the job
# record shows the fake userid that was stamped at submission time.
#
cmd=$1
case "$cmd" in
    exec)
        # Replace ourselves with the job shell so job-exec observes its exit.
        shift
        exec "$@" ;;
    kill)
        signal=$2
        pid=$3
        shift 3
        kill -"$signal" -"$pid" 2>/dev/null ;;
    *)
        printf 'fake-imp: unknown command "%s"\n' "$cmd" >&2
        exit 1 ;;
esac

# vi: ts=4 sw=4 expandtab

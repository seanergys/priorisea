#!/bin/bash
#
# fake-shell.sh - minimal job shell for fake-user (multiuser) jobs.
#
# Usage: fake-shell.sh JOBID
#
# The real flux job shell refuses to run a job whose userid (mocked via the
# FLUX_HANDLE_USERID testing hook) differs from the shell's own getuid()
# (see src/shell/rexec.c rexec_auth_cb).  This stand-in skips that check so a
# job stamped with an arbitrary/fake userid will actually execute its command.
#

JOBID=$1
if test -z "$JOBID"; then
    echo "fake-shell: JOBID argument required" >&2
    exit 1
fi

# Announce shell start so job-exec treats the shell as running.
flux kvs eventlog append exec.eventlog shell.start 2>/dev/null

jobspec=$(flux job info "$JOBID" jobspec 2>/dev/null)
if test -z "$jobspec"; then
    echo "fake-shell: unable to read jobspec for $JOBID" >&2
    exit 1
fi

# Export environment variables declared in the jobspec.
while IFS= read -r kv; do
    test -n "$kv" && export "$kv"
done < <(printf '%s' "$jobspec" \
    | jq -r '(.attributes.system.environment // {}) | to_entries[] | "\(.key)=\(.value)"')

# Change to the requested working directory.
cwd=$(printf '%s' "$jobspec" | jq -r '.attributes.system.cwd // "."')
if test "$cwd" != "null" && test -n "$cwd"; then
    cd "$cwd" 2>/dev/null
fi

# Extract the task command as an argv array (preserves argument boundaries)
# and exec it so its exit status propagates to job-exec.
mapfile -t cmd < <(printf '%s' "$jobspec" | jq -r '.tasks[0].command[]')
if test "${#cmd[@]}" -eq 0; then
    echo "fake-shell: no command found in jobspec" >&2
    exit 1
fi
exec "${cmd[@]}"

# vi: ts=4 sw=4 expandtab

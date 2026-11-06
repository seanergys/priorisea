#
#  Fake-user job support.
#
#  These helpers let a test submit a job as an arbitrary
#  (possibly non-existent) userid while the instance owner keeps submitting
#  normal jobs on the same instance.
#

fake_user_imp_path () {
    echo "${SHARNESS_TEST_SRCDIR}/scripts/fake-imp.sh"
}

fake_user_job_shell_path () {
    echo "${SHARNESS_TEST_SRCDIR}/scripts/fake-shell.sh"
}

#  Point job-exec at the fake IMP so multiuser (fake-user) jobs launch.
#  Only multiuser jobs use the IMP, so owner (single-user) jobs are
#  unaffected.  Idempotent; safe to call more than once.
test_fake_user_setup () {
    if ! flux module list 2>/dev/null | grep -q '^job-exec'; then
        error "fake_user_setup: job-exec module is not loaded"
        return 1
    fi
    flux config set --type string exec.imp "$(fake_user_imp_path)"
    flux module reload -f job-exec
}

#  Submit a job recorded as a fake userid and print the new job id.
#
#  Usage: flux_run_fake <fake-uid> <command> [args...]
#
flux_run_fake () {
    test "$#" -ge 2 ||
        error "run_as_fake expects <fake-uid> <command> [args...]"
    fake_uid=$1
    shift
    tmpd=${SHARNESS_TRASH_DIRECTORY:-/tmp}
    raw="$tmpd/fake-job.raw"
    signed="$tmpd/fake-job.signed"

    #  Build the jobspec for the requested command, then stamp this job's
    #  shell (attributes.system.exec.job_shell) so only this job uses the
    #  fake shell; owner jobs keep the real one.
    flux run --dry-run -n1 "$@" > "$raw"
    jq --arg s "$(fake_user_job_shell_path)" \
        '.attributes.system.exec.job_shell = $s' "$raw" \
      | flux python "${SHARNESS_TEST_SRCDIR}/scripts/sign-none.py" "$fake_uid" \
      > "$signed"
    FLUX_HANDLE_USERID="$fake_uid" \
        flux job submit --flags=signed "$signed"
}

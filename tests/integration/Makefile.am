SUBDIRS = scripts

TESTS = \
    t1000-basic.t \
    t1001-formula-cmd.t \
    t1002-timewin-cmd.t \
    t1003-score-cmd.t \
    t1004-decay-cmd.t \
    t1005-plugin-reload.t \
    t1006-large-update.t \
    t1007-sb-config.t \
    t1008-sb-query.t \
    t1009-sb-nobus.t

EXTRA_DIST = \
    $(TESTS) \
    sharness.sh \
    sharness.d \
    test-lib.sh

TESTS_ENVIRONMENT = \
    SHARNESS_TEST_SRCDIR=$(abs_srcdir)

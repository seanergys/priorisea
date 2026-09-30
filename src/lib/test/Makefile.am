# SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
#
# SPDX-License-Identifier: MIT

noinst_LIBRARIES = libcatchmain.a
libcatchmain_a_SOURCES = main.cpp

AM_CPPFLAGS = \
    -I$(top_srcdir)/src \
    -I$(top_srcdir)/external/tinyexpr \
    -I$(top_srcdir)/external/croncpp/include \
    $(JANSSON_CFLAGS) \
    $(SEANERBUS_CFLAGS) \
    $(FLUX_CORE_CFLAGS) \
    $(CATCH2_CPPFLAGS)

check_PROGRAMS = \
    config_test \
    decay_test \
    formula_test \
    score_test \
    timewin_test \
    util_test

TESTS = $(check_PROGRAMS)

LDADD = \
    libcatchmain.a \
    $(top_builddir)/src/lib/libpriorisea.la \
    $(top_builddir)/external/libtinyexpr.la \
    $(JANSSON_LIBS) \
    $(SEANERBUS_LIBS) \
    $(FLUX_CORE_LIBS)

config_test_SOURCES = config.cpp
decay_test_SOURCES = decay.cpp
formula_test_SOURCES = formula.cpp
score_test_SOURCES = score.cpp
timewin_test_SOURCES = timewin.cpp
util_test_SOURCES = util.cpp

# SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
#
# SPDX-License-Identifier: MIT

noinst_LTLIBRARIES = libpriorisea.la

libpriorisea_la_CPPFLAGS= \
    -I$(top_srcdir)/external/tinyexpr \
    -I$(top_srcdir)/external/croncpp/include \
    $(JANSSON_CFLAGS) \
    $(SEANERBUS_CFLAGS) \
    $(FLUX_CORE_CFLAGS)

priorisea_LDADD = \
    $(top_builddir)/external/libtinyexpr.la \
    $(JANSSON_LIBS) \
    $(SEANERBUS_LIBS) \
    $(FLUX_CORE_LIBS)

libpriorisea_la_SOURCES = \
    decay.cpp decay.hpp \
    flux.cpp flux.hpp \
    formula.cpp formula.hpp \
    score.cpp score.hpp \
    timewin.cpp timewin.hpp \
    util.cpp util.hpp

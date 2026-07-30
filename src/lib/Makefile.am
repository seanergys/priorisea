# SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
#
# SPDX-License-Identifier: MIT

noinst_LTLIBRARIES = libpriorisea.la

libpriorisea_la_CPPFLAGS= \
    -I$(top_srcdir)/external/tinyexpr \
    -I$(top_srcdir)/external/croncpp/include \
    $(JANSSON_CFLAGS) \
    $(FLUX_CORE_CFLAGS)

#libpriorisea_la_LIBADD = \
#    $(top_builddir)/external/libtinyexpr.la \
#    $(JANSSON_LIBS) \
#    $(FLUX_CORE_LIBS)

libpriorisea_la_SOURCES = \
    decay.cpp decay.hpp \
    error.cpp error.hpp \
    flux.cpp flux.hpp \
    formula.cpp formula.hpp \
    score.cpp score.hpp \
    timewin.cpp timewin.hpp \
    types.hpp \
    util.cpp util.hpp

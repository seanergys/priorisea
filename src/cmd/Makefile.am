# SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
#
# SPDX-License-Identifier: MIT

bin_PROGRAMS = priorisea
priorisea_SOURCES = main.cpp \
    formula.cpp formula.hpp \
    time.cpp time.hpp \
    score.cpp score.hpp \
    decay.cpp  decay.hpp \
    init.cpp init.hpp

priorisea_CPPFLAGS = \
    -I$(top_srcdir)/external/tinyexpr \
    -I$(top_srcdir)/external/croncpp/include \
    -I$(top_srcdir)/src \
    -I$(top_srcdir)/src/cmd/ \
    $(JANSSON_CFLAGS) \
    $(SEANERBUS_CFLAGS) \
    $(FLUX_CORE_CFLAGS)

priorisea_LDADD = \
    $(top_builddir)/src/lib/libpriorisea.la \
    $(top_builddir)/external/libtinyexpr.la \
    $(JANSSON_LIBS) \
    $(SEANERBUS_LIBS) \
    $(FLUX_CORE_LIBS)

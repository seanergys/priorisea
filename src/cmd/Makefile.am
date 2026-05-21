# SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
#
# SPDX-License-Identifier: MIT

SUBDIRS = lib

bin_PROGRAMS = priorisea
priorisea_SOURCES = main.cpp formula.cpp time.cpp score.cpp decay.cpp
priorisea_CPPFLAGS = \
    -I$(top_srcdir)/external/tinyexpr \
    -I$(top_srcdir)/external/croncpp/include \
    -I$(top_srcdir)/src/common \
    -I$(top_srcdir)/src/cmd/ \
    $(JANSSON_CFLAGS) \
    $(SEANERBUS_CFLAGS) \
    $(FLUX_CORE_CFLAGS)

priorisea_LDADD = \
    $(top_builddir)/external/libtinyexpr.la \
    $(top_builddir)/src/common/libcommon.la \
    $(top_builddir)/src/cmd/lib/libflux.la \
    $(top_builddir)/src/cmd/lib/libkvs.la \
    $(JANSSON_LIBS) \
    $(SEANERBUS_LIBS) \
    $(FLUX_CORE_LIBS)

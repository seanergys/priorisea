# SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
#
# SPDX-License-Identifier: MIT

noinst_LTLIBRARIES = libflux.la libkvs.la

libflux_la_SOURCES = flux.cpp flux.hpp
libflux_la_CPPFLAGS = \
    -I$(top_srcdir)/src/common \
    $(FLUX_CORE_CFLAGS)
libflux_la_LIBADD = \
    $(top_builddir)/src/common/libcommon.la \
    $(FLUX_CORE_LIBS)

libkvs_la_SOURCES = kvs.cpp kvs.hpp
libkvs_la_CPPFLAGS = $(FLUX_CORE_CFLAGS)
libkvs_la_LIBADD = $(FLUX_CORE_LIBS)

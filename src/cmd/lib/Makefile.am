noinst_LTLIBRARIES = libflux.la

libflux_la_SOURCES = flux.cpp flux.hpp
libflux_la_CPPFLAGS = \
    -I$(top_srcdir)/src/common \
    $(FLUX_CORE_CFLAGS)
libflux_la_LIBADD = \
    $(top_builddir)/src/common/libcommon.la \
    $(FLUX_CORE_LIBS)

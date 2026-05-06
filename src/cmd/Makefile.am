bin_PROGRAMS = priorisea
priorisea_SOURCES = main.cpp formula.cpp time.cpp score.cpp decay.cpp
priorisea_CPPFLAGS = \
    -I$(top_srcdir)/src/lib/tinyexpr \
    -I$(top_srcdir)/src/common \
    $(JANSSON_CFLAGS) \
    $(FLUX_CORE_CFLAGS)

priorisea_LDADD = \
    $(JANSSON_LIBS) \
    $(top_builddir)/src/lib/tinyexpr/libtinyexpr.la \
    $(top_builddir)/src/common/libcommon.la \
    $(FLUX_CORE_LIBS)

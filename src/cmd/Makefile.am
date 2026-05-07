bin_PROGRAMS = priorisea
priorisea_SOURCES = main.cpp formula.cpp time.cpp score.cpp decay.cpp
priorisea_CPPFLAGS = \
    -I$(top_srcdir)/external/tinyexpr \
    -I$(top_srcdir)/external/croncpp \
    -I$(top_srcdir)/src/common \
    $(JANSSON_CFLAGS) \
    $(FLUX_CORE_CFLAGS)

priorisea_LDADD = \
    $(top_builddir)/external/tinyexpr/libtinyexpr.la \
    $(top_builddir)/src/common/libcommon.la \
    $(JANSSON_LIBS) \
    $(FLUX_CORE_LIBS)

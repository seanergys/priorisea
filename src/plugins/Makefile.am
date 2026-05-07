jobtapdir = $(libdir)/flux/job-manager/plugins/
jobtap_LTLIBRARIES = sea_debug.la sea_priority.la

sea_debug_la_SOURCES = sea_debug.cpp
sea_debug_la_CPPFLAGS = $(JANSSON_CFLAGS) $(FLUX_CORE_CFLAGS)
sea_debug_la_LIBADD = $(JANSSON_LIBS)
sea_debug_la_LDFLAGS = -module -avoid-version

sea_priority_la_SOURCES = sea_priority.cpp

sea_priority_la_CPPFLAGS = \
   -I$(top_srcdir)/src/lib/tinyexpr \
   -I$(top_srcdir)/src/common \
    $(JANSSON_CFLAGS) \
    $(FMT_CFLAGS) \
    $(FLUX_CORE_CFLAGS)

sea_priority_la_LIBADD = \
    $(JANSSON_LIBS) \
    $(FMT_LIBS) \
    $(top_builddir)/src/lib/tinyexpr/libtinyexpr.la \
    $(top_builddir)/src/common/libcommon.la

sea_priority_la_LDFLAGS = -module -avoid-version

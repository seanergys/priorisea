jobtapdir = $(libdir)/flux/job-manager/plugins/
jobtap_LTLIBRARIES = sea_priority.la

sea_priority_la_SOURCES = sea_priority.cpp
sea_priority_la_CPPFLAGS = -I$(top_srcdir)/include $(FLUX_CORE_CFLAGS) $(JANSSON_CFLAGS) $(FMT_CFLAGS)
sea_priority_la_LIBADD = $(JANSSON_CFLAGS) $(FMT_LIBS)
sea_priority_la_LDFLAGS = -module -avoid-version

AC_INIT([sea_priority_plugin], [1.0], [you@example.com])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall foreign subdir-objects])
AC_CONFIG_SRCDIR([src/plugins/sea_priority.cpp])
AC_CONFIG_HEADERS([config.h])
AM_SILENT_RULES([yes])
AC_LANG_PUSH([C++])

# Compilers / tools
AC_PROG_CC
AC_PROG_CXX
AM_PROG_AR
LT_INIT

# Compiler flags
CXXFLAGS="$CXXFLAGS -O2 -g0 -fPIC"
AC_ARG_ENABLE([coverage],
  [AS_HELP_STRING([--enable-coverage], [enable code coverage instrumentation with gcov])],
  [enable_coverage=$enableval],
  [enable_coverage=no])
if test "x$enable_coverage" = "xyes"; then
  CXXFLAGS="$CXXFLAGS --coverage -O0 -g"
  LDFLAGS="$LDFLAGS --coverage"
fi

# Package configuration
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([JANSSON], [jansson])
PKG_CHECK_MODULES([FMT], [fmt])
PKG_CHECK_MODULES([SEANERBUS], [seanerbus-c])

# Catch2
AC_CHECK_HEADER([catch2/catch.hpp],
    [CATCH2_CPPFLAGS=],
    [AC_MSG_ERROR([Catch2 not found. Install the catch2-devel package.])])
AC_SUBST([CATCH2_CPPFLAGS])

# CLI11
AC_CHECK_HEADER([CLI/CLI.hpp],
    [CLI11_CPPFLAGS=],
    [AC_MSG_ERROR([CLI11 not found. Install the cli11-devel package.])])
AC_SUBST([CLI11_CPPFLAGS])

# Flux core
AX_FLUX_CORE

#
# SeanerBUS tools required by the integration tests: the demobus server and
# the naming service. They are not needed to build the plugin itself, so a
# missing tool is reported as a warning rather than failing configuration.
#
AC_PATH_PROG([DEMOBUS], [demobus], [no])
AC_PATH_PROG([NAMING_SERVER], [naming_server], [no])
if test "x$DEMOBUS" = "xno"; then
  AC_MSG_WARN([demobus (the SeanerBUS server) was not found in PATH; some SeanerBUS integration tests may fail])
fi
if test "x$NAMING_SERVER" = "xno"; then
  AC_MSG_WARN([the SeanerBUS naming service (naming_server) was not found in PATH; some SeanerBUS integration tests may fail])
fi
AC_SUBST([DEMOBUS])
AC_SUBST([NAMING_SERVER])

# Output files
AC_CONFIG_FILES([
 Makefile
 external/Makefile
 src/Makefile
 src/cli/Makefile
 src/lib/Makefile
 src/plugins/Makefile
 src/test/Makefile
 tests/Makefile
 tests/scripts/Makefile
])

AC_OUTPUT

AC_INIT([sea_priority_plugin], [1.0], [you@example.com])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall foreign subdir-objects])
AC_CONFIG_SRCDIR([src/plugins/sea_priority.cpp])
AC_CONFIG_HEADERS([config.h])

AM_SILENT_RULES([yes])

# Compilers / tools
AC_PROG_CC
AC_PROG_CXX
AM_PROG_AR
LT_INIT

# Compiler flags
CXXFLAGS="$CXXFLAGS -O2 -g0 -fPIC"

PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([JANSSON], [jansson])
PKG_CHECK_MODULES([FMT], [fmt])
AX_FLUX_CORE

# Output files
AC_CONFIG_FILES([
 Makefile
 external/Makefile
 external/tinyexpr/Makefile
 src/Makefile
 src/cmd/Makefile
 src/common/Makefile
 src/plugins/Makefile
 tests/integration/Makefile
])

AC_OUTPUT

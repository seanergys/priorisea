AC_DEFUN([AX_FLUX_CORE], [

  AC_ARG_WITH([flux],
    [AS_HELP_STRING([--with-flux=DIR],
      [Path to existing flux-core installation])],
    [FLUX_DIR="$withval"],
    [FLUX_DIR=""]
  )

  saved_PKG_CONFIG_PATH=$PKG_CONFIG_PATH

  # If user provided a custom path, use it
  if test -n "$FLUX_DIR"; then
    PKG_CONFIG_PATH="$FLUX_DIR/lib/pkgconfig:$FLUX_DIR/lib64/pkgconfig:$PKG_CONFIG_PATH"
  else
    # Optional fallback: keep old behavior
    PKG_CONFIG_PATH="${prefix}/lib/pkgconfig:$PKG_CONFIG_PATH"
  fi

  export PKG_CONFIG_PATH

  PKG_CHECK_MODULES([FLUX_CORE], [flux-core >= 0.81.0],
    [
      FLUX_PREFIX=`pkg-config --variable=prefix flux-core`
      LIBFLUX_VERSION=`pkg-config --modversion flux-core`

      if test -z "${FLUX_PREFIX}" ; then
        AC_MSG_ERROR([failed to determine flux-core prefix from pkg-config])
      fi

      AC_SUBST(FLUX_PREFIX)
      AC_SUBST(LIBFLUX_VERSION)

      # Ensure matching flux binary
      AC_PATH_PROG([FLUX], [flux],
        [AC_MSG_ERROR([failed to find flux executable])],
        [$FLUX_PREFIX/bin:$PATH]
      )

      # Detect Python version
      FLUX_PYTHON_VERSION=$($FLUX python -c 'import sys; print(".".join(map(str, sys.version_info[[:2]])))')
      AC_SUBST(FLUX_PYTHON_VERSION)
    ],
    [AC_MSG_ERROR([flux-core package not installed])]
  )

  # Other libraries
  PKG_CHECK_MODULES([FLUX_IDSET], [flux-idset], [], [])
  PKG_CHECK_MODULES([FLUX_HOSTLIST], [flux-hostlist], [], [])

  PKG_CONFIG_PATH=$saved_PKG_CONFIG_PATH
  export PKG_CONFIG_PATH
])

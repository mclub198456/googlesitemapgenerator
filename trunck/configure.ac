#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(google-sitemap-generator, 0.0.1, opensource@google.com)
AC_CONFIG_SRCDIR([src/common/apacheconfig.cc])

APACHE_VERSION=2.0
APACHE_ARCH=32

AC_ARG_WITH(apache-bin,
[[ --with-apache-bin=FILE
  Build module for this apache instance.]],
[
  if test -x "$with_apache_bin"; then :; else
    AC_MSG_ERROR("$with_apache_bin" is not an executable.)
  fi

  "$with_apache_bin" -V 1>/dev/null 2>/dev/null
  if test $? -eq 0; then :; else
    AC_MSG_ERROR("$with_apache_bin" should support -V option.)
  fi

  APACHE_BIN="$with_apache_bin"

  APACHE_VERSION=`"$with_apache_bin" -V | sed 's/[[://]]/ /g' \
   | awk '/version/ { print $4 }' | awk -F. '{ printf("%s.%s", $1, $2) }'`

  if test "x$APACHE_VERSION" != "x1.3" && \
    test "x$APACHE_VERSION" != "x2.0" && \
    test "x$APACHE_VERSION" != "x2.2"; then
    AC_MSG_ERROR(["$with_apache_bin is not a supported apache instance."])
  fi

  if test "x$APACHE_VERSION" = "x1.3" ; then
    EAPI=`"$APACHE_BIN" -V | awk '/EAPI/ { print $2 }'`
    if test "x$EAPI" = "xEAPI" ; then
      APACHE_VERSION="1.3.e"
    fi
  fi

  if test "x$APACHE_VERSION" = "x1.3" || \
    test "x$APACHE_VERSION" = "x1.3.e" ; then
    APACHE_ARCH=32
  fi

  line=`"$with_apache_bin" -V | grep Architecture | grep 32`
  if test "x$line" != "x"; then
    APACHE_ARCH=32
  fi
  line=`"$with_apache_bin" -V | grep Architecture | grep 64`
  if test "x$line" != "x"; then
    APACHE_ARCH=64
  fi
],
[
  AC_MSG_RESULT(no --with-apache-bin given)
]
)

AC_ARG_WITH(apache-version,
[[ --with-apache-version=VERSION
  Build module for this apache version.]],
[
  if test "x$with_apache_version" != "x1.3" && \
    test "x$with_apache_version" != "x1.3.e" && \
    test "x$with_apache_version" != "x2.0" && \
    test "x$with_apache_version" != "x2.2"; then
    AC_MSG_ERROR(["apache-version should be 1.3, 1.3.3, 2.0 or 2.2"])
  fi

  APACHE_VERSION="$with_apache_version"
],
[
  AC_MSG_RESULT(no --with-apache-version given)
]
)
if test "x$APACHE_VERSION" = "x"; then
  AC_MSG_ERROR([Apache version can't be determined.])
fi


AC_ARG_WITH(apache-arch,
[[ --with-apache-arch=32|64
  Targeted cpu arch of apache binary.]],
[
  if test "x$with_apache_arch" != "x32" && \
    test "x$with_apache_arch" != "x64"; then
    AC_MSG_ERROR(["apache-arch should be 32 or 64."])
  fi

  APACHE_ARCH="$with_apache_arch"
],
[
  AC_MSG_RESULT(no --with-apache-arch given)
]
)
if test "x$APACHE_ARCH" = "x"; then
  AC_MSG_ERROR([Apache arch can't be determined.])
fi

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_RANLIB

topdir=`pwd`
srcdir="$topdir/src"

CPPFLAGS="$CPPFLAGS -I\"$topdir\" -I\"$srcdir\""
if test "x$APACHE_VERSION" = "x1.3.e" ; then
  CPPFLAGS="$CPPFLAGS -DEAPI"
fi

LDFLAGS="-lpthread -lm"

if test "x$APACHE_ARCH" = "x32"; then
  LIB_STDCXX="$topdir/third_party/libstdc++-v3/32/libstdc++.a"
  CFLAGS="$CFLAGS -m32 -fPIC"
  CXXFLAGS="$CXXFLAGS -m32 -fPIC"
elif test "x$APACHE_ARCH" = "x64"; then
  LIB_STDCXX="$topdir/third_party/libstdc++-v3/64/libstdc++.a"
  CFLAGS="$CFLAGS -m64 -fPIC"
  CXXFLAGS="$CXXFLAGS -m64 -fPIC"
else
  AC_MSG_ERROR([Invalid apache arch: $APACHE_ARCH])
fi

case "x$APACHE_VERSION" in
  "x1.3")
  INCLUDE_APACHE="$topdir/third_party/apache-1.3"
  INCLUDE_APR="$topdir"
  ;;
  "x1.3.e")
  INCLUDE_APACHE="$topdir/third_party/apache-1.3"
  INCLUDE_APR="$topdir"
  ;;
  "x2.0")
  INCLUDE_APACHE="$topdir/third_party/apache-2.0"
  INCLUDE_APR="$topdir/third_party/apr-0/$APACHE_ARCH"
  ;;
  "x2.2")
  INCLUDE_APACHE="$topdir/third_party/apache-2.2"
  INCLUDE_APR="$topdir/third_party/apr-1/$APACHE_ARCH"
  ;;
  *)
  AC_MSG_ERROR([Invalid apache version: $APACHE_VERSION])
  ;;
esac

AC_SUBST(LIB_STDCXX)
AC_SUBST(INCLUDE_APACHE)
AC_SUBST(INCLUDE_APR)

AC_SUBST(APACHE_VERSION)
AC_SUBST(APACHE_BIN)
AC_SUBST(APACHE_ARCH)

# Checks for libraries.

# Checks for header files.
# AC_HEADER_DIRENT
# AC_HEADER_STDC
# AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h malloc.h netdb.h netinet/in.h stdlib.h string.h sys/file.h sys/socket.h sys/time.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
# AC_HEADER_STDBOOL
# AC_C_CONST
# AC_TYPE_UID_T
# AC_C_INLINE
# AC_TYPE_PID_T
# AC_TYPE_SIZE_T
# AC_HEADER_TIME
# AC_STRUCT_TM
# AC_C_VOLATILE

# Checks for library functions.
# AC_FUNC_FORK
# AC_FUNC_LSTAT
# AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
# AC_FUNC_MKTIME
# AC_TYPE_SIGNAL
# AC_FUNC_STAT
# AC_FUNC_STRFTIME
# AC_FUNC_STRTOD
# AC_CHECK_FUNCS([dup2 gethostbyname gethostname inet_ntoa memset socket strcasecmp strchr strncasecmp strstr])

AC_CONFIG_FILES([Makefile
third_party/tinyxml/Makefile third_party/zlib/Makefile third_party/md5/Makefile
src/common/Makefile src/apache_module/Makefile src/sitemapservice/Makefile])
AC_OUTPUT


#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(google-sitemap-generator, 0.0.1, opensource@google.com)
AC_CONFIG_SRCDIR([src/common/apacheconfig.cc])

current_dir=`pwd`
srcdir="$current_dir/src"

AC_ARG_WITH(apache-include,
[[ --with-apache-include=DIR
  Apache development header file directory.]],
[
  if test -d "$with_apache_include"; then :; else
    AC_MSG_ERROR("$with_apache_include" is not an accessible directory.)
  fi

  INCLUDE_APACHE="$with_apache_include"
  if echo "$INCLUDE_APACHE" | grep '^/'; then :; else
    INCLUDE_APACHE="$current_dir/$INCLUDE_APACHE"
  fi
],
[
  AC_MSG_ERROR(Apache include directory is required.)
]
)

AC_ARG_WITH(apr-include,
[[ --with-apr-include=DIR
  Apache portable runtime header file directory.]],
[
  if test -d "$with_apr_include"; then :; else
    AC_MSG_ERROR("$with_apr_include" is not an accessible directory.)
  fi

  INCLUDE_APR="$with_apr_include"
  if echo "$INCLUDE_APR" | grep '^/'; then :; else
    INCLUDE_APR="$current_dir/$INCLUDE_APR"
  fi
],
[
 INCLUDE_APR=""
]
)

AC_ARG_WITH(static-libstdcxx,
[[ --with-static-libstdcxx=FILE
  Link with static libstdcxx.]],
[
  if test -f "$with_static_libstdcxx"; then :; else
    AC_MSG_ERROR(["$with_static_libstdcxx is not accessible."])
  fi

  LIB_STDCXX="$with_static_libstdcxx"
  if echo "$LIB_STDCXX" | grep '^/'; then :; else
    LIB_STDCXX="$current_dir/$LIB_STDCXX"
  fi
],
[
  AC_MSG_RESULT(Shared libstdc++ will be used.)
  LIB_STDCXX=""
]
)

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

CFLAGS="$CFLAGS -fPIC"

CPPFLAGS="$CPPFLAGS -I\"$current_dir\" -I\"$srcdir\""

AC_SUBST(LIB_STDCXX)
AC_SUBST(INCLUDE_APACHE)
AC_SUBST(INCLUDE_APR)

APACHE_VERSION=""
AC_PREPROC_IFELSE(
   [AC_LANG_SOURCE([[
#include "$INCLUDE_APACHE/ap_mmn.h"
#if MODULE_MAGIC_COOKIE != 0x41503230UL
#error "Not AP20"
#endif
   ]])],
  [APACHE_VERSION="2.0"],
  []
)

AC_PREPROC_IFELSE(
  [AC_LANG_SOURCE([[
#include "$INCLUDE_APACHE/ap_mmn.h"
#if MODULE_MAGIC_COOKIE != 0x41503232UL
#error "Not AP22"
#endif
  ]])],
  [APACHE_VERSION="2.2"],
  []
)

AC_PREPROC_IFELSE(
  [AC_LANG_SOURCE([[
#include "$INCLUDE_APACHE/ap_mmn.h"
#if MODULE_MAGIC_COOKIE != 0x45415049UL
#error "Not EAPI"
#endif
  ]])],
  [APACHE_VERSION="1.3.e"],
  []
)

AC_PREPROC_IFELSE(
  [AC_LANG_SOURCE([[
#include "$INCLUDE_APACHE/ap_mmn.h"
#if MODULE_MAGIC_COOKIE != 0x41503133UL
#error "Not AP13"
#endif
  ]])],
  [APACHE_VERSION="1.3"],
  []
)

if test "x$APACHE_VERSION" = "x"; then
  AC_MSG_ERROR([Invalid Apache include directory.])
else
  AC_MSG_RESULT([Module will be compiled for Apache-$APACHE_VERSION])
  if test "$APACHE_VERSION" = "2.0" \
    || test "$APACHE_VERSION" = "2.2"; then
    if test "x$INCLUDE_APR" = "x"; then
      AC_MSG_ERROR([Apr include directory is required for Apache-$APACHE_VERSION])
    fi
  fi
fi

AC_SUBST(APACHE_VERSION)

# Checks for libraries.
AC_CHECK_LIB(pthread, pthread_create,
  [LDFLAGS="$LDFLAGS -lpthread"],
  [LDFLAGS="$LDFLAGS -pthread"]
)
AC_CHECK_LIB(m, log,
  [LDFLAGS="$LDFLAGS -lm"]
)

if test "x$LIB_STDCXX" = "x"; then
  LDFLAGS="$LDFLAGS -lstdc++"
fi

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h malloc.h memory.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STAT
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_C_VOLATILE
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_ERROR_AT_LINE
# AC_REPLACE_FNMATCH
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_FUNC_WAIT3
AC_CHECK_FUNCS([bzero dup2 gethostbyaddr gethostbyname gethostname inet_ntoa memchr memset munmap regcomp select socket strcasecmp strchr strdup strerror strncasecmp strstr])

AC_CONFIG_FILES([Makefile
                 src/apache_module/Makefile
                 src/common/Makefile
                 src/sitemapservice/Makefile
                 src/admin_console_cgi/Makefile
                 third_party/md5/Makefile
                 third_party/tinyxml/Makefile
                 third_party/zlib/Makefile])

AC_OUTPUT


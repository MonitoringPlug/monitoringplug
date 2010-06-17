#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)

AC_INIT([monitorplug], [0.0], [marius.rieder@durchmesser.ch])
AC_CONFIG_AUX_DIR(build-aux)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])


AM_INIT_AUTOMAKE([1.9 foreign])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

EXTRASUBDIRS=

# Checks for libraries.
PKG_CHECK_MODULES([CHECK], [check >= 0.9.4],,[AC_MSG_WARN([Skipping check])])

# IPv6 Option
AC_ARG_WITH([ipv6],
    [AS_HELP_STRING([--with-ipv6], [support IPv6 @<:@default=check@:>@])],
    [], [with_ipv6=check])

# Check for AF_INET6 support - unistd.h required for Darwin
if test "$with_ipv6" != "no"; then
    AC_CACHE_CHECK([for IPv6 support], mp_cv_sys_ipv6, [
        AC_TRY_COMPILE(
            [#ifdef HAVE_UNISTD_H
            #include <unistd.h>
            #endif
            #include <netinet/in.h>
            #include <sys/socket.h>],
            [struct sockaddr_in6 sin6;
            void *p;

            sin6.sin6_family = AF_INET6;
            sin6.sin6_port = 587;
            p = &sin6.sin6_addr;],
            [mp_cv_sys_ipv6=yes],
            [mp_cv_sys_ipv6=no])
        ])
    if test "$mp_cv_sys_ipv6" = "no" -a "$with_ipv6" != "check"; then
        AC_MSG_FAILURE([--with-ipv6 was given, but test for IPv6 support failed])
    fi
    if test "$mp_cv_sys_ipv6" = "yes"; then
        AC_DEFINE(USE_IPV6,1,[Enable IPv6 support])
    fi
    with_ipv6="$mp_cv_sys_ipv6"
fi

# Checks for header files.
AC_CHECK_HEADERS([limits.h netdb.h stdlib.h string.h strings.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([alarm memset strdup strerror strspn strstr strtol])

AC_SUBST(EXTRASUBDIRS)

AC_CONFIG_FILES([Makefile
                 lib/Makefile
                 tests/Makefile])
AC_OUTPUT

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])

AC_INIT([monitoringplug],[0.4],[marius.rieder@durchmesser.ch])
AC_CONFIG_AUX_DIR(build-aux)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.9 -Wall -Werror foreign])

AC_CANONICAL_HOST
AC_MSG_CHECKING([for kernel type ($host_os)])
case $host_os in
	*linux*)
	AC_DEFINE([OS_LINUX], 1, [True if program is to be compiled for a Linux kernel])
	ac_system="Linux"
	;;
	*darwin*)
	AC_DEFINE([OS_DARWIN], 1, [True if program is to be compiled for a Linux kernel])
	ac_system="Darwin"
	;;
	*)
	ac_system="unknown"
esac
AC_MSG_RESULT([$ac_system])
AM_CONDITIONAL([OS_LINUX], [test x$ac_system == xLinux])
AM_CONDITIONAL([OS_DARWIN], [test x$ac_system == xDarwin])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AC_C_CONST

# Checks for libraries.
PKG_CHECK_MODULES([CHECK], [check >= 0.9.4],[],[AC_MSG_WARN([Skipping check])])
AX_LIB_EXPAT()
AC_LIB_LDNS([],[], [AC_MSG_WARN([Skipping ldns])])
AC_LIB_CUPS([1.4.0],[], [AC_MSG_WARN([Skipping cups])])
LIBCURL_CHECK_CONFIG([yes],[7.7.2], [], [AC_MSG_WARN([Skipping curl])])
PKG_CHECK_MODULES([XMLRPC], [xmlrpc xmlrpc_client],[],[AC_MSG_WARN([Skipping xmlrpc-c])])
PKG_CHECK_MODULES([SELINUX], [libselinux],[],[AC_MSG_WARN([Skipping SELinux])])
PKG_CHECK_MODULES([GNUTLS], [gnutls >= 2.8.0], [],[AC_MSG_WARN([Skipping gnutls])])
PKG_CHECK_MODULES([JSON], [json], [],[AC_MSG_WARN([Skipping json-c])])
PKG_CHECK_MODULES([LIBVIRT],[libvirt], [],[AC_MSG_WARN([Skipping libvirt])])

# SNMP Option
AC_ARG_WITH([netsnmp],
    [AS_HELP_STRING([--with-netsnmp], [support SNMP @<:@default=check@:>@])],
    [], [with_netsnmp=check])

if test "$with_netsnmp" != "no"; then
	AC_CHECK_LIB(netsnmp, netsnmp_get_version,[
	     NETSNMP_LIBS="-lnetsnmp"
	     AC_DEFINE(HAVE_NET_SNMP, 1, Net-SNMP Version) 
		 HAVE_NET_SNMP=yes
		 AC_SUBST(NETSNMP_LIBS) ],
		AC_MSG_RESULT(Cannot find Net-SNMP libraries.))
fi

# IPv6 Option
AC_ARG_WITH([ipv6],
    [AS_HELP_STRING([--with-ipv6], [support IPv6 @<:@default=check@:>@])],
    [], [with_ipv6=check])

# Check for AF_INET6 support - unistd.h required for Darwin
if test "$with_ipv6" != "no"; then
    AC_CACHE_CHECK([for IPv6 support], mp_cv_sys_ipv6, [
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#ifdef HAVE_UNISTD_H
            #include <unistd.h>
            #endif
            #include <netinet/in.h>
            #include <sys/socket.h>]], [[struct sockaddr_in6 sin6;
            void *p;

            sin6.sin6_family = AF_INET6;
            sin6.sin6_port = 587;
            p = &sin6.sin6_addr;]])],[mp_cv_sys_ipv6=yes],[mp_cv_sys_ipv6=no])
        ])
    if test "$mp_cv_sys_ipv6" = "no" -a "$with_ipv6" != "check"; then
        AC_MSG_FAILURE([--with-ipv6 was given, but test for IPv6 support failed])
    fi
    if test "$mp_cv_sys_ipv6" = "yes"; then
        AC_DEFINE(USE_IPV6,1,[Enable IPv6 support])
    fi
    with_ipv6="$mp_cv_sys_ipv6"
fi

AC_PATH_PROGS(BIN_FALSE, false)
AC_DEFINE_UNQUOTED([BIN_FALSE], ["$ac_cv_path_BIN_FALSE"],
            [false path.])

# Conditionals
AM_CONDITIONAL([HAVE_LDNS], [test "x$LDNS_CFLAGS" != x])
AM_CONDITIONAL([HAVE_CUPS], [test "x$CUPS_CFLAGS" != x])
AM_CONDITIONAL([HAVE_CURL], [test "x$LIBCURL" != x])
AM_CONDITIONAL([HAVE_CURL_TFTP], [test "x$libcurl_protocol_TFTP" != x])
AM_CONDITIONAL([HAVE_EXPAT], [test "x$EXPAT_CFLAGS" != x])
AM_CONDITIONAL([HAVE_NET_SNMP], [test "x$NETSNMP_LIBS" != x])
AM_CONDITIONAL([HAVE_XMLRPC], [test "x$XMLRPC_LIBS" != x])
AM_CONDITIONAL([HAVE_SELINUX], [test "x$SELINUX_LIBS" != x])
AM_CONDITIONAL([HAVE_CHECK], [test "x$CHECK_LIBS" != x])
AM_CONDITIONAL([HAVE_GNUTLS], [test "x$GNUTLS_LIBS" != x])
AM_CONDITIONAL([HAVE_JSON], [test "x$JSON_LIBS" != x])
AM_CONDITIONAL([HAVE_LIBVIRT], [test "x$LIBVIRT_LIBS" != x])

# GCC Opts
#AX_CFLAGS_WARN_ALL()
AX_CFLAGS_GCC_OPTION(-Wall)
AX_CFLAGS_GCC_OPTION(-Wchar-subscripts)
AX_CFLAGS_GCC_OPTION(-Wmissing-declarations)
AX_CFLAGS_GCC_OPTION(-Wnested-externs)
AX_CFLAGS_GCC_OPTION(-Wcast-align)
AX_CFLAGS_GCC_OPTION(-Wformat)
AX_CFLAGS_GCC_OPTION(-Wformat-security)
AX_CFLAGS_GCC_OPTION(-Winline)

# MP VARS
CPPFLAGS="-I\$(top_srcdir) -I\$(top_srcdir)/lib";
LDFLAGS="$LDFLAGS -L\$(top_srcdir)/lib";

MPLIB_LIBS="-monitoringplug"
SNMPUTIL_LIBS="-lsnmputils"
RHCSUTIL_LIBS="-lrhcsutils"
LDNSUTIL_LIBS="-lldnsutils"
LIBVIRTUTIL_LIBS="-lvirturils"
XMLRPCUTIL_LIBS="-lxmlrpcutils"

AC_SUBST(MPLIB_LIBS)
AC_SUBST(SNMPUTIL_LIBS)
AC_SUBST(RHCSUTIL_LIBS)
AC_SUBST(LDNSUTIL_LIBS)
AC_SUBST(XMLRPCUTIL_LIBS)

# Checks for header files.
AC_CHECK_HEADERS([limits.h netdb.h stdlib.h string.h strings.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIGNAL
#AC_TYPE_UINT32_T

# Checks for library functions.
#AC_FUNC_MALLOC
#AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_CHECK_FUNCS([alarm memset strdup strerror strspn strstr strtol])

AC_CONFIG_FILES([Makefile
                 lib/Makefile
                 base/Makefile
		 cups/Makefile
                 curl/Makefile
                 dns/Makefile
                 dummy/Makefile
                 gnutls/Makefile
		 libvirt/Makefile
                 rhcs/Makefile
                 rpc/Makefile
                 selinux/Makefile
                 snmp/Makefile
                 xmlrpc/Makefile
                 contrib/Makefile
                 doc/Makefile
		 policy/Makefile
                 tests/Makefile])
AC_OUTPUT

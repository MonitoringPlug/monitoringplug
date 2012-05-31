#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])

AC_INIT([monitoringplug],[0.9],[marius.rieder@durchmesser.ch])
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
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

AC_C_CONST
AC_TYPE_INTPTR_T

# Checks for libraries.
# Ckeck
AC_ARG_WITH([check], AS_HELP_STRING([--without-check], [Build without the check library]))
AS_IF([test "x$with_check" != "xno"], [
       PKG_CHECK_MODULES([CHECK], [check >= 0.9.4],[],[AC_MSG_WARN([Skipping check])])
       ],[AC_MSG_WARN([Skipping check])])
# Expat
AX_LIB_EXPAT()
AX_LIB_LDNS()
AX_LIB_CUPS([1.4.0])
AX_LIB_MYSQL()
LIBCURL_CHECK_CONFIG([yes],[7.7.2], [], [AC_MSG_WARN([Skipping curl])])
# xmlrpc-c
AX_LIB_XMLRPC()
# selinux
AC_ARG_WITH([selinux], AS_HELP_STRING([--without-selinux], [Build without selinux]))
AS_IF([test "x$with_selinux" != "xno"], [
       PKG_CHECK_MODULES([SELINUX], [libselinux],[],[AC_MSG_WARN([Skipping SELinux])])
       ],[AC_MSG_WARN([Skipping SELinux])])
# gnutls
AC_ARG_WITH([gnutls], AS_HELP_STRING([--without-gnutls], [Build without gnutls]))
AS_IF([test "x$with_gnutls" != "xno"], [
       PKG_CHECK_MODULES([GNUTLS], [gnutls >= 2.8.0], [],[AC_MSG_WARN([Skipping gnutls])])
       ],[AC_MSG_WARN([Skipping gnutls])])
# json-c
AC_ARG_WITH([json], AS_HELP_STRING([--without-json], [Build without json]))
AS_IF([test "x$with_json" != "xno"], [
       PKG_CHECK_MODULES([JSON], [json], [],[AC_MSG_WARN([Skipping json-c])])
       ],[AC_MSG_WARN([Skipping json-c])])
# libvirt
AC_ARG_WITH([libvirt], AS_HELP_STRING([--without-libvirt], [Build without libvirt]))
AS_IF([test "x$with_libvirt" != "xno"], [
       PKG_CHECK_MODULES([LIBVIRT],[libvirt], [],[AC_MSG_WARN([Skipping libvirt])])
       ],[AC_MSG_WARN([Skipping libvirt])])
# smbclient
PKG_CHECK_MODULES([SMBCLIENT],[smbclient], [],[AC_MSG_WARN([Skipping libsmbclient])])

# SNMP Option
AX_LIB_NETSNMP()

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
AC_PATH_PROG([XSLTPROC], [xsltproc])
AC_PATH_PROG([DEBUILD], [debuild])
AC_PATH_PROG([RPMBUILD], [rpmbuild])

# Conditionals
AM_CONDITIONAL([HAVE_LDNS], [test "x$LDNS_CFLAGS" != x])
AM_CONDITIONAL([HAVE_CUPS], [test "x$CUPS_LDFLAGS" != x])
AM_CONDITIONAL([HAVE_CURL], [test "x$LIBCURL" != x])
AM_CONDITIONAL([HAVE_CURL_TFTP], [test "x$libcurl_protocol_TFTP" != x])
AM_CONDITIONAL([HAVE_EXPAT], [test "x$EXPAT_CFLAGS" != x])
AM_CONDITIONAL([HAVE_NET_SNMP], [test "x$NETSNMP_CFLAGS" != x])
AM_CONDITIONAL([HAVE_XMLRPC], [test "x$XMLRPC_CFLAGS" != x])
AM_CONDITIONAL([HAVE_SELINUX], [test "x$SELINUX_LIBS" != x])
AM_CONDITIONAL([HAVE_CHECK], [test "x$CHECK_LIBS" != x])
AM_CONDITIONAL([HAVE_GNUTLS], [test "x$GNUTLS_LIBS" != x])
AM_CONDITIONAL([HAVE_JSON], [test "x$JSON_LIBS" != x])
AM_CONDITIONAL([HAVE_LIBVIRT], [test "x$LIBVIRT_LIBS" != x])
AM_CONDITIONAL([HAVE_SMBCLIENT], [test "x$SMBCLIENT_LIBS" != x])
AM_CONDITIONAL([HAVE_MYSQL], [test "x$MYSQL_CFLAGS" != x])
AM_CONDITIONAL([HAVE_XSLTPROC], [test "x$XSLTPROC" != x])
AM_CONDITIONAL([HAVE_DEBUILD], [test "x$DEBUILD" != x])
AM_CONDITIONAL([HAVE_RPMBUILD], [test "x$RPMBUILD" != x])

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

SNMPUTIL_LIBS="-lsnmputils"
RHCSUTIL_LIBS="-lrhcsutils"
LDNSUTIL_LIBS="-lldnsutils"
LIBVIRTUTIL_LIBS="-lvirturils"
XMLRPCUTIL_LIBS="-lxmlrpcutils"

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
		 mysql/Makefile
                 rhcs/Makefile
                 rpc/Makefile
                 selinux/Makefile
		 smb/Makefile
                 snmp/Makefile
                 xmlrpc/Makefile
                 contrib/Makefile
                 doc/Makefile
		 policy/Makefile
		 policy/monitoringplug.te
                 tests/Makefile])
AC_OUTPUT

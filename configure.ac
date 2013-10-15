#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])

AC_INIT([monitoringplug],[0.16],[marius.rieder@durchmesser.ch])
AC_CONFIG_AUX_DIR(build-aux)
AM_INIT_AUTOMAKE([1.9 -Wall -Werror foreign])
AC_CONFIG_SRCDIR([lib/mp_common.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_PROG_YACC
AC_PROG_LEX

AC_C_CONST
AC_TYPE_INTPTR_T

## Build MAN Pages
AC_ARG_WITH([man], AS_HELP_STRING(
    [--without-man], [Do not build/install the MAN pages.]))
AS_IF([test "x$with_man" != "xno"],
      [AC_PATH_PROG([XSLTPROC], [xsltproc])],
      [])
AM_CONDITIONAL([HAVE_XSLTPROC], [test "x$XSLTPROC" != x])
AM_CONDITIONAL([DISABLE_MAN], [test "x$with_man" = "xno"])

## Buildsystem RPM
AC_PATH_PROG([RPMBUILD], [rpmbuild])
AM_CONDITIONAL([HAVE_RPMBUILD], [test "x$RPMBUILD" != x])

## Buildsystem DEB
AC_PATH_PROG([DEBUILD], [debuild])
AM_CONDITIONAL([HAVE_DEBUILD], [test "x$DEBUILD" != x])

## Runtime dependencies
AC_PATH_PROGS(BIN_FALSE, false)
AC_DEFINE_UNQUOTED([BIN_FALSE], ["$ac_cv_path_BIN_FALSE"],
                               [false path.])
AC_PATH_PROG([BIN_SENDMAIL], [sendmail], [/usr/sbin/sendmail])
AC_DEFINE_UNQUOTED([BIN_SENDMAIL], ["$ac_cv_path_BIN_SENDMAIL"],
                                          [sendmail path.])


# Check for OS
AC_CANONICAL_HOST
AC_MSG_CHECKING([for kernel type ($host_os)])
case $host_os in
    *linux*)
    AC_DEFINE([OS_LINUX], 1, [True if program is to be compiled for a Linux kernel])
    ac_system="Linux"
    ;;
    *freebsd*)
    AC_DEFINE([OS_FREEBSD], 1, [True if program is to be compiled for a FreeBSD kernel])
    AC_DEFINE([_WITH_DPRINTF], 1, [Expose dprintf])
    ac_system="Freebsd"
    ;;
    *darwin*)
    AC_DEFINE([OS_DARWIN], 1, [True if program is to be compiled for a Darwin kernel])
    ac_system="Darwin"
    ;;
    *)
    ac_system="unknown"
esac
AC_MSG_RESULT([$ac_system])
AM_CONDITIONAL([OS_LINUX], [test x$ac_system == xLinux])
AM_CONDITIONAL([OS_FREEBSD], [test x$ac_system == xFreebsd])
AM_CONDITIONAL([OS_DARWIN], [test x$ac_system == xDarwin])

# Checks for libraries.
## Don't depend on pkg-config
m4_ifdef([PKG_CHECK_MODULES], [], [
	  m4_errprint([warning: pkg-config checks are not available])
	  m4_defun([PKG_CHECK_MODULES], [
		    AC_MSG_WARN([pkg-config not available, cannot check for $2])
		    $4
		    ])
	  ])
## Check
AC_ARG_WITH([check], AS_HELP_STRING(
    [--without-check], [Build without the check library]))
AS_IF([test "x$with_check" != "xno"],
      [PKG_CHECK_MODULES([CHECK], [check >= 0.9.4],
                         [have_check=yes], [have_check=no])],
      [have_ldns=disable])
AM_CONDITIONAL([HAVE_CHECK], [test "x$CHECK_LIBS" != x])

## CUPS
AX_LIB_CUPS([1.4.0])
AS_IF([test "x$want_cupy" = "xno"],
      [have_cups=disabled], [have_cups=$found_cups])
AM_CONDITIONAL([HAVE_CUPS], [test "x$have_cups" = "xyes"])

## cURL
LIBCURL_CHECK_CONFIG([yes],[7.7.2], [have_curl=yes], [have_curl=no])
AM_CONDITIONAL([HAVE_CURL], [test "x$have_curl" = "xyes"])
AM_CONDITIONAL([HAVE_CURL_TFTP], [test "x$libcurl_protocol_TFTP" = "xyes"])

## lDNS
AX_LIB_LDNS()
AS_IF([test "x$want_ldns" = "xno"],
      [have_ldns=disabled], [have_ldns=$found_ldns])
AM_CONDITIONAL([HAVE_LDNS], [test "x$have_ldns" = "xyes"])

## Expat
AX_LIB_EXPAT()
AS_IF([test "x$expat_requested" = "xno"],
      [have_expat=disabled], [have_expat=$expat_lib_found])
AM_CONDITIONAL([HAVE_EXPAT], [test "x$have_expat" = "xyes"])

## FastCGI
AC_ARG_WITH([fcgi],
    [AS_HELP_STRING([--without-fcgi], [Build without the fcgi library])])
AS_IF([test "x$with_fcgi" != "xno"],
      [AC_CHECK_LIB([fcgi], [FCGX_Init], [have_fcgi=yes], [have_fcgi=no])],
      [have_fcgi=disabled])
AS_IF([test "x$have_fcgi" = "xyes"],
      [
        FCGI_LIBS='-lfcgi'
        AC_SUBST(FCGI_LIBS)        
      ],
      [AS_IF([test "x$with_fcgi" = "xyes"],
             [AC_MSG_ERROR([FastCGI requested but not found])
      ])
])
AM_CONDITIONAL([HAVE_FCGI], [test "x$have_fcgi" == "xyes"])

## GnuTLS
AC_ARG_WITH([gnutls],
    [AS_HELP_STRING([--without-gnutls], [Build without gnutls])])
AS_IF([test "x$with_gnutls" != "xno"],
      [PKG_CHECK_MODULES([GNUTLS], [gnutls >= 2.8.0], [have_gnutls=yes], [have_gnutls=no])],
      [have_gnutls=disabled])
AS_IF([test "x$have_gnutls" = "xyes"],
      [],
      [AS_IF([test "x$with_gnutls" = "xyes"],
             [AC_MSG_ERROR([GnuTLS requested but not found.])
      ])
])
AM_CONDITIONAL([HAVE_GNUTLS], [test "x$have_gnutls"  = "xyes"])

## IPMI
AC_ARG_WITH([ipmi],
    [AS_HELP_STRING([--without-ipmi], [Build without (Open)IPMI support.])])
AS_IF([test "x$with_ipmi" != "xno"],
      [PKG_CHECK_MODULES([IPMI],[OpenIPMI OpenIPMIposix], [have_ipmi=yes], [have_ipmi=no])],
      [have_ipmi=disabled])
AS_IF([test "x$have_ipmi" = "xyes"],
      [],
      [AS_IF([test "x$with_ipmi" = "xyes"],
             [AC_MSG_ERROR([OpenIPMI requested but not found.])
      ])
])
AM_CONDITIONAL([HAVE_IPMI], [test "x$have_ipmi" = "xyes"])

## JSON
AC_ARG_WITH([json],
    [AS_HELP_STRING([--without-json], [Build without JSON support.])])
AS_IF([test "x$with_json" != "xno"],
      [PKG_CHECK_MODULES([JSON], [json], [have_json=yes], [have_json=no])],
      [have_json=disabled])
AS_IF([test "x$have_json" = "xyes"],
      [],
      [AS_IF([test "x$with_json" = "xyes"],
             [AC_MSG_ERROR([JSON requested but json-c not found.])
      ])
])
AM_CONDITIONAL([HAVE_JSON], [test "x$have_json" = "xyes"])

## LibVirt
AC_ARG_WITH([libvirt],
    [AS_HELP_STRING([--without-libvirt], [Build without LibVirt support.])])
AS_IF([test "x$with_libvirt" != "xno"],
      [PKG_CHECK_MODULES([LIBVIRT],[libvirt], [have_libvirt=yes], [have_libvirt=no])],
      [have_libvirt=disabled])
AS_IF([test "x$have_libvirt" = "xyes"],
      [],
      [AS_IF([test "x$with_libvirt" = "xyes"],
             [AC_MSG_ERROR([LibVirt requested but not found.])
      ])
])
AM_CONDITIONAL([HAVE_LIBVIRT], [test "x$have_libvirt" = "xyes"])

## MySQL
AX_LIB_MYSQL()
AS_IF([test "x$want_mysql" = "xno"],
    [have_mysql=disabled], [have_mysql=$found_mysql])
AM_CONDITIONAL([HAVE_MYSQL], [test "x$have_mysql" = "xyes"])

## OPing
AC_ARG_WITH([oping],
    [AS_HELP_STRING([--without-oping], [Build without the oping library.])])
AS_IF([test "x$with_oping" != "xno"],
      [
       AC_SEARCH_LIBS([ping_construct],[oping],[have_oping=yes],[have_oping=no])
       LIBS=$ac_func_search_save_LIBS
       OPING_LIBS=$ac_res
       AC_SUBST([OPING_LIBS])
       AS_IF([test "x$have_oping" != "xyes"],
             [AC_CHECK_HEADERS([oping.h],[],[have_oping=yes],[have_oping=no])])
      ],
      [have_oping=disabled])
AS_IF([test "x$have_oping" = "xyes"],
      [],
      [AS_IF([test "x$with_oping" = "xyes"],
             [AC_MSG_ERROR([oping requested but not found.])
      ])
])
AM_CONDITIONAL([HAVE_OPING], [test "x$have_oping" = "xyes"])

## PostgreSQL
AS_IF([test "$PQINCPATH" == ""], [
       AC_PATH_PROGS([PG_CONFIG], [pg_config], [true])
       PQINCPATH="-I`$PG_CONFIG --includedir` $PQINCPATH"
       ])
AX_CHECK_POSTGRES_DB([], [AC_MSG_WARN([Skipping PostgreSQL])])
AC_CHECK_LIB([pq], [PQlibVersion], [
    AC_DEFINE([HAVE_PQLIBVERSION], [1], [Define to 1 if PQlibVersion is available.])
    ], [])
AM_CONDITIONAL([HAVE_LIBPQ], [test "x$LIBPQ_LIBS" != x])

## (Sun)RPC
AX_LIB_RPC()
AS_IF([test "x$rpc_requested" = "xno"],
    [have_rpc=disabled], [have_rpc=$HAVE_RPC])
AM_CONDITIONAL([HAVE_RPC], [test "x$have_rpc" = "xyes"])

## SELinux
AC_ARG_WITH([selinux],
    [AS_HELP_STRING([--without-selinux], [Build without SELinux support.])])
AS_IF([test "x$with_selinux" != "xno"],
      [PKG_CHECK_MODULES([SELINUX], [libselinux], [have_selinux=yes], [have_selinux=no])],
      [have_selinux=disabled])
AS_IF([test "x$have_selinux" = "xyes"],
      [],
      [AS_IF([test "x$with_selinux" = "xyes"],
             [AC_MSG_ERROR([SELinux requested but not found.])
      ])
])
AM_CONDITIONAL([HAVE_SELINUX], [test "x$have_selinux" = "xyes"])

## SMBClient
AC_ARG_WITH([smbclient],
    [AS_HELP_STRING([--without-smbclient], [Build without SMB-Client support.])])
AS_IF([test "x$with_smbclient" != "xno"],
      [
       PKG_CHECK_MODULES([SMBCLIENT],[smbclient], [have_smbclient=yes], [have_smbclient=no])
       AS_IF([test "x$have_smbclient" != "xyes"], [
           AC_SEARCH_LIBS([smbc_new_context],[smbclient],[have_smbclient=yes],[have_smbclient=no])
           LIBS=$ac_func_search_save_LIBS
           SMBCLIENT_LIBS=$ac_res
           AC_SUBST([SMBCLIENT_LIBS])
           AS_IF([test "x$have_smbclient" = "xyes"],
             [AC_CHECK_HEADERS([libsmbclient.h],[have_smbclient=yes],[have_smbclient=no])])
           ])
      ],
      [have_smbclient=disabled])
AS_IF([test "x$have_smbclient" = "xyes"],
      [],
      [AS_IF([test "x$with_smbclient" = "xyes"],
             [AC_MSG_ERROR([SMB-Client requested but not found.])
      ])
])
AM_CONDITIONAL([HAVE_SMBCLIENT], [test "x$have_smbclient" = "xyes"])

## SNMP
AX_LIB_NETSNMP()
AS_IF([test "x$want_netsnmp" = "xno"],
      [have_netsnmp=disabled], [have_netsnmp=$found_netsnmp])
AM_CONDITIONAL([HAVE_NET_SNMP], [test "x$have_netsnmp" = "xyes"])

## Termios
AC_ARG_WITH([termios],
    [AS_HELP_STRING([--without-termios], [Build without termios support.])])
AS_IF([test "x$with_termios" != "xno"],
      [
       AC_SYS_POSIX_TERMIOS()
       have_termios=$ac_cv_sys_posix_termios
      ],
      [have_termios=disabled])
AS_IF([test "x$have_termios" = "xyes"],
      [],
      [AS_IF([test "x$with_termios" = "xyes"],
             [AC_MSG_ERROR([termios requested but not found.])
      ])
])
AM_CONDITIONAL([HAVE_TERMIOS], [test "x$have_termios" = "xyes"])

## Varnish
AC_ARG_WITH([varnish],
    [AS_HELP_STRING([--without-varnish], [Build without Varnish support.])])
AS_IF([test "x$with_varnish" != "xno"],
      [PKG_CHECK_MODULES([VARNISH], [varnishapi >= 3.0.0], [have_varnish=yes], [have_varnish=no])],
      [have_varish=disabled])
AS_IF([test "x$have_varnish" = "xyes"],
      [],
      [AS_IF([test "x$with_varnish" = "xyes"],
             [AC_MSG_ERROR([Varnish requested but varnishapi not found.])
      ])
])
AM_CONDITIONAL([HAVE_VARNISH], [test "x$have_varnish" = "xyes"])


## XMLRPC-C
AX_LIB_XMLRPC()
AS_IF([test "x$want_xmlrpc" = "xno"],
      [have_xmlrpc=disabled], [have_xmlrpc=$found_xmlrpc])
AM_CONDITIONAL([HAVE_XMLRPC], [test "x$have_xmlrpc" = "xyes"])

## IPv6
AC_ARG_WITH([ipv6],
    [AS_HELP_STRING([--with-ipv6], [support IPv6 @<:@default=check@:>@])])
AS_IF([test "x$with_ipv6" != "xno"],
      [
       AC_CACHE_CHECK([for IPv6 support], mp_cv_have_iv6, [
            AC_COMPILE_IFELSE([
                AC_LANG_PROGRAM(
                    [[#ifdef HAVE_UNISTD_H
                      #include <unistd.h>
                      #endif
                      #include <netinet/in.h>
                      #include <sys/socket.h>]],
                    [[struct sockaddr_in6 sin6;
                      void *p;

                      sin6.sin6_family = AF_INET6;
                      sin6.sin6_port = 587;
                      p = &sin6.sin6_addr;]])],
                [mp_cv_have_ipv6=yes],[mp_cv_have_ipv6=no])
            ])
       have_ipv6=$mp_cv_have_ipv6
      ],
      [have_ipv6=disabled])
AS_IF([test "x$have_ipv6" = "xyes"],
      [AC_DEFINE(USE_IPV6,1,[Enable IPv6 support])],
      [AS_IF([test "x$with_ipv6" = "xyes"],
             [AC_MSG_ERROR([IPv6 requested but not found.])
      ])
])

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

# Prevent from overlinking
AC_ARG_ENABLE([as-needed],
    [AC_HELP_STRING([--disable-as-needed], [Disable overlinking protection])])
AS_IF([test "x$enable_as_needed" != "xno"],
      [AX_CHECK_LINK_FLAG(["-Wl,--as-needed"],
                          [LDFLAGS="$LDFLAGS -Wl,--as-needed"])
      ])

# MP VARS
CPPFLAGS="$CPPFLAGS -I\$(top_srcdir) -I\$(top_srcdir)/lib";
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
                 fcgi/Makefile
                 gnutls/Makefile
                 ipmi/Makefile
                 libvirt/Makefile
                 mysql/Makefile
                 oping/Makefile
                 pgsql/Makefile
                 rhcs/Makefile
                 rpc/Makefile
                 selinux/Makefile
                 smb/Makefile
                 snmp/Makefile
                 varnish/Makefile
                 xmlrpc/Makefile
                 contrib/Makefile
                 doc/Makefile
                 policy/Makefile
                 policy/monitoringplug.te
                 notify/Makefile
                 tests/Makefile
                 tests/replayd/Makefile])
AC_OUTPUT

echo "
--------------------------------------------

${PACKAGE_NAME} ${PACKAGE_VERSION}

Prefix: '${prefix}'
Compiler '${CC} ${CFLAGS} ${CPPFLAGS}'

Unittest:    ${have_check}

Ipv6:        ${have_ipv6}

Dependencies:
 CUPS:       ${have_cups}
 CURL:       ${have_curl}
 DNS:        ${have_ldns}
 Expat:      ${have_expat}
 FCGI:       ${have_fcgi}
 GnuTLS:     ${have_gnutls}
 IPMI:       ${have_ipmi}
 JSON:       ${have_json}
 LibVirt:    ${have_libvirt}
 MySQL:      ${have_mysql}
 OPING:      ${have_oping}
 PostgreSQL: ${have_pqlib}
 (Sun)RPC:   ${have_rpc}
 SELinux:    ${have_selinux}
 SMB:        ${have_smbclient}
 SNMP:       ${have_netsnmp}
 Termios:    ${have_termios}
 Varnish:    ${have_varnish}
 XMLRPM:     ${have_xmlrpc}

Now type 'make @<:@<target>@:>@'

--------------------------------------------"

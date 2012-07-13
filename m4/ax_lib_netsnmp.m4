# ===========================================================================
#
# SYNOPSIS
#
#   AX_LIB_NETSNMP([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   This macro provides tests of availability of Net-SNMP client library of
#   particular version or newer.
#
#   AX_LIB_NETSNMP macro takes only one argument which is optional. If there
#   is no required version passed, then macro does not run version test.
#
#   The --with-netsnmp option takes one of three possible values:
#
#   no - do not check for Net-SNMP client library
#
#   yes - do check for Net-SNMP library in standard locations (netsnmp_config
#   should be in the PATH)
#
#   path - complete path to netsnmp_config utility, use this option if
#   netsnmp_config can't be found in the PATH
#
#   This macro calls:
#
#     AC_SUBST(NETSNMP_CFLAGS)
#     AC_SUBST(NETSNMP_LDFLAGS)
#     AC_SUBST(NETSNMP_VERSION)
#
#   And sets:
#
#     HAVE_NETSNMP
#
# LICENSE
#
#   Copyright (c) 2012 Marius Rieder <marius.rieder@durchmesser.ch>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 2

AC_DEFUN([AX_LIB_NETSNMP],
[
    AC_ARG_WITH([netsnmp],
        AS_HELP_STRING([--with-netsnmp=@<:@ARG@:>@],
            [use Net-SNMP client library @<:@default=yes@:>@, optionally specify path to net-snmp-config]
        ),
        [
        if test "$withval" = "no"; then
            want_netsnmp="no"
        elif test "$withval" = "yes"; then
            want_netsnmp="yes"
        else
            want_netsnmp="yes"
            NETSNMP_CONFIG="$withval"
        fi
        ],
        [want_netsnmp="yes"]
    )
    AC_ARG_VAR([NETSNMP_CONFIG], [Full path to net-snmp-config program])

    NETSNMP_CFLAGS=""
    NETSNMP_LDFLAGS=""
    NETSNMP_VERSION=""

    dnl
    dnl Check Net-SNMP libraries
    dnl

    if test "$want_netsnmp" = "yes"; then

        if test -z "$NETSNMP_CONFIG" ; then
            AC_PATH_PROGS([NETSNMP_CONFIG], [net-snmp-config], [no])
        fi

        if test "$NETSNMP_CONFIG" != "no"; then
            NETSNMP_CFLAGS="`$NETSNMP_CONFIG --cflags`"
            NETSNMP_LDFLAGS="`$NETSNMP_CONFIG --libs`"

            NETSNMP_VERSION=`$NETSNMP_CONFIG --version`

            found_netsnmp="yes"
        else
            found_netsnmp="no"
        fi
    fi

    dnl
    dnl Check if required version of Net-SNMP is available
    dnl


    netsnmp_version_req=ifelse([$1], [], [], [$1])

    if test "$found_netsnmp" = "yes" -a -n "$netsnmp_version_req"; then

        AC_MSG_CHECKING([if Net-SNMP version is >= $netsnmp_version_req])

        dnl Decompose required version string of netsnmp
        dnl and calculate its number representation
        netsnmp_version_req_major=`expr $netsnmp_version_req : '\([[0-9]]*\)'`
        netsnmp_version_req_minor=`expr $netsnmp_version_req : '[[0-9]]*\.\([[0-9]]*\)'`
        netsnmp_version_req_micro=`expr $netsnmp_version_req : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
        if test "x$netsnmp_version_req_micro" = "x"; then
            netsnmp_version_req_micro="0"
        fi

        netsnmp_version_req_number=`expr $netsnmp_version_req_major \* 1000000 \
                                   \+ $netsnmp_version_req_minor \* 1000 \
                                   \+ $netsnmp_version_req_micro`

        dnl Decompose version string of installed netsnmp
        dnl and calculate its number representation
        netsnmp_version_major=`expr $NETSNMP_VERSION : '\([[0-9]]*\)'`
        netsnmp_version_minor=`expr $NETSNMP_VERSION : '[[0-9]]*\.\([[0-9]]*\)'`
        netsnmp_version_micro=`expr $NETSNMP_VERSION : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
        if test "x$netsnmp_version_micro" = "x"; then
            netsnmp_version_micro="0"
        fi

        netsnmp_version_number=`expr $netsnmp_version_major \* 1000000 \
                                   \+ $netsnmp_version_minor \* 1000 \
                                   \+ $netsnmp_version_micro`

        netsnmp_version_check=`expr $netsnmp_version_number \>\= $netsnmp_version_req_number`
        if test "$netsnmp_version_check" = "1"; then
            AC_MSG_RESULT([yes])
        else
            AC_MSG_RESULT([no])
	    found_netsnmp="no"
        fi
    fi

    if test "$found_netsnmp" = "yes" ; then
        AC_DEFINE([HAVE_NETSNMP], [1],
                  [Define to 1 if Net-SNMP libraries are available])
    else
        NETSNMP_CFLAGS=""
        NETSNMP_LDFLAGS=""
        NETSNMP_VERSION=""
    fi

    AC_SUBST([NETSNMP_VERSION])
    AC_SUBST([NETSNMP_CFLAGS])
    AC_SUBST([NETSNMP_LDFLAGS])
])

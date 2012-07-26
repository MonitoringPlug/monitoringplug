# ===========================================================================
#
# SYNOPSIS
#
#   AX_LIB_XMLRPC([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   This macro provides tests of availability of xmlrpc-c library of
#   particular version or newer.
#
#   AX_LIB_XMLRPC macro takes only one argument which is optional. If there
#   is no required version passed, then macro does not run version test.
#
#   The --with-xmlrpc option takes one of three possible values:
#
#   no - do not check for xmlrpc-c library
#
#   yes - do check for xmlrpc-c library in standard locations (xmlrpc_config
#   should be in the PATH)
#
#   path - complete path to xmlrpc_config utility, use this option if
#   xmlrpc_config can't be found in the PATH
#
#   This macro calls:
#
#     AC_SUBST(XMLRPC_CFLAGS)
#     AC_SUBST(XMLRPC_LDFLAGS)
#     AC_SUBST(XMLRPC_VERSION)
#
#   And sets:
#
#     HAVE_XMLRPC
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

AC_DEFUN([AX_LIB_XMLRPC],
[
    AC_ARG_WITH([xmlrpc],
        AS_HELP_STRING([--with-xmlrpc=@<:@ARG@:>@],
            [use xmlrpc-c client library @<:@default=yes@:>@, optionally specify path to xmlrpc-c-config]
        ),
        [
        if test "$withval" = "no"; then
            want_xmlrpc="no"
        elif test "$withval" = "yes"; then
            want_xmlrpc="yes"
        else
            want_xmlrpc="yes"
            XMLRPC_CONFIG="$withval"
        fi
        ],
        [want_xmlrpc="yes"]
    )
    AC_ARG_VAR([XMLRPC_CONFIG], [Full path to xmlrpc-c-config program])

    XMLRPC_CFLAGS=""
    XMLRPC_LDFLAGS=""
    XMLRPC_VERSION=""

    dnl
    dnl Check xmlrpc-c libraries
    dnl

    if test "$want_xmlrpc" = "yes"; then

        if test -z "$XMLRPC_CONFIG" ; then
            AC_PATH_PROGS([XMLRPC_CONFIG], [xmlrpc-c-config], [no])
        fi

        if test "$XMLRPC_CONFIG" != "no"; then
            XMLRPC_CFLAGS="`$XMLRPC_CONFIG --cflags` `$XMLRPC_CONFIG client --cflags`"
            XMLRPC_LDFLAGS="`$XMLRPC_CONFIG --libs` `$XMLRPC_CONFIG client --libs`"

            XMLRPC_VERSION=`$XMLRPC_CONFIG client --version`

            found_xmlrpc="yes"
        else
            found_xmlrpc="no"
        fi
    fi

    dnl
    dnl Check if required version of xmlrpc-c is available
    dnl


    xmlrpc_version_req=ifelse([$1], [], [], [$1])

    if test "$found_xmlrpc" = "yes" -a -n "$xmlrpc_version_req"; then

        AC_MSG_CHECKING([if xmlrpc-c version is >= $xmlrpc_version_req])

        dnl Decompose required version string of xmlrpc
        dnl and calculate its number representation
        xmlrpc_version_req_major=`expr $xmlrpc_version_req : '\([[0-9]]*\)'`
        xmlrpc_version_req_minor=`expr $xmlrpc_version_req : '[[0-9]]*\.\([[0-9]]*\)'`
        xmlrpc_version_req_micro=`expr $xmlrpc_version_req : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
        if test "x$xmlrpc_version_req_micro" = "x"; then
            xmlrpc_version_req_micro="0"
        fi

        xmlrpc_version_req_number=`expr $xmlrpc_version_req_major \* 1000000 \
                                   \+ $xmlrpc_version_req_minor \* 1000 \
                                   \+ $xmlrpc_version_req_micro`

        dnl Decompose version string of installed xmlrpc
        dnl and calculate its number representation
        xmlrpc_version_major=`expr $XMLRPC_VERSION : '\([[0-9]]*\)'`
        xmlrpc_version_minor=`expr $XMLRPC_VERSION : '[[0-9]]*\.\([[0-9]]*\)'`
        xmlrpc_version_micro=`expr $XMLRPC_VERSION : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
        if test "x$xmlrpc_version_micro" = "x"; then
            xmlrpc_version_micro="0"
        fi

        xmlrpc_version_number=`expr $xmlrpc_version_major \* 1000000 \
                                   \+ $xmlrpc_version_minor \* 1000 \
                                   \+ $xmlrpc_version_micro`

        xmlrpc_version_check=`expr $xmlrpc_version_number \>\= $xmlrpc_version_req_number`
        if test "$xmlrpc_version_check" = "1"; then
            AC_MSG_RESULT([yes])
        else
            AC_MSG_RESULT([no])
            found_xmlrpc="no"
        fi
    fi

    if test "$found_xmlrpc" = "yes" ; then
        AC_DEFINE([HAVE_XMLRPC], [1],
                  [Define to 1 if xmlrpc-c libraries are available])
    else
        XMLRPC_CFLAGS=""
        XMLRPC_LDFLAGS=""
        XMLRPC_VERSION=""
    fi

    AC_SUBST([XMLRPC_VERSION])
    AC_SUBST([XMLRPC_CFLAGS])
    AC_SUBST([XMLRPC_LDFLAGS])
])

# ===========================================================================
#
# SYNOPSIS
#
#   AX_LIB_LDNS([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   This macro provides tests of availability of xmlrcp-c library of
#   particular version or newer.
#
#   AX_LIB_LDNS macro takes only one argument which is optional. If there
#   is no required version passed, then macro does not run version test.
#
#   The --with-ldns option takes one of three possible values:
#
#   no - do not check for ldns library
#
#   yes - do check for ldns library in standard locations (ldns_config
#   should be in the PATH)
#
#   path - complete path to ldns_config utility, use this option if
#   ldns_config can't be found in the PATH
#
#   This macro calls:
#
#     AC_SUBST(LDNS_CFLAGS)
#     AC_SUBST(LDNS_LDFLAGS)
#     AC_SUBST(LDNS_VERSION)
#
#   And sets:
#
#     HAVE_LDNS
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

AC_DEFUN([AX_LIB_LDNS],
[
    AC_ARG_WITH([ldns],
        AS_HELP_STRING([--with-ldns=@<:@ARG@:>@],
            [use ldns library @<:@default=yes@:>@, optionally specify path to ldns-config]
        ),
        [
        if test "$withval" = "no"; then
            want_ldns="no"
        elif test "$withval" = "yes"; then
            want_ldns="yes"
        else
            want_ldns="yes"
            LDNS_CONFIG="$withval"
        fi
        ],
        [want_ldns="yes"]
    )
    AC_ARG_VAR([LDNS_CONFIG], [Full path to ldns-config program])

    LDNS_CFLAGS=""
    LDNS_LDFLAGS=""
    LDNS_VERSION=""

    dnl
    dnl Check ldns-c libraries
    dnl

    if test "$want_ldns" = "yes"; then

        if test -z "$LDNS_CONFIG" ; then
            AC_PATH_PROGS([LDNS_CONFIG], [ldns-config], [no])
        fi

        if test "$LDNS_CONFIG" != "no"; then
            LDNS_CFLAGS="`$LDNS_CONFIG --cflags` `$LDNS_CONFIG client --cflags`"
            LDNS_LDFLAGS="`$LDNS_CONFIG --libs` `$LDNS_CONFIG client --libs`"

            LDNS_VERSION=`$LDNS_CONFIG client --version`

            found_ldns="yes"
        else
            found_ldns="no"
        fi
    fi

    dnl
    dnl Check if required version of ldns is available
    dnl


    ldns_version_req=ifelse([$1], [], [], [$1])

    if test "$found_ldns" = "yes" -a -n "$ldns_version_req"; then

        AC_MSG_CHECKING([if ldns version is >= $ldns_version_req])

        dnl Decompose required version string of ldns
        dnl and calculate its number representation
        ldns_version_req_major=`expr $ldns_version_req : '\([[0-9]]*\)'`
        ldns_version_req_minor=`expr $ldns_version_req : '[[0-9]]*\.\([[0-9]]*\)'`
        ldns_version_req_micro=`expr $ldns_version_req : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
        if test "x$ldns_version_req_micro" = "x"; then
            ldns_version_req_micro="0"
        fi

        ldns_version_req_number=`expr $ldns_version_req_major \* 1000000 \
                                   \+ $ldns_version_req_minor \* 1000 \
                                   \+ $ldns_version_req_micro`

        dnl Decompose version string of installed ldns
        dnl and calculate its number representation
        ldns_version_major=`expr $LDNS_VERSION : '\([[0-9]]*\)'`
        ldns_version_minor=`expr $LDNS_VERSION : '[[0-9]]*\.\([[0-9]]*\)'`
        ldns_version_micro=`expr $LDNS_VERSION : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
        if test "x$ldns_version_micro" = "x"; then
            ldns_version_micro="0"
        fi

        ldns_version_number=`expr $ldns_version_major \* 1000000 \
                                   \+ $ldns_version_minor \* 1000 \
                                   \+ $ldns_version_micro`

        ldns_version_check=`expr $ldns_version_number \>\= $ldns_version_req_number`
        if test "$ldns_version_check" = "1"; then
            AC_MSG_RESULT([yes])
        else
            AC_MSG_RESULT([no])
            found_ldns="no"
        fi
    fi

    if test "$found_ldns" = "yes" ; then
        AC_DEFINE([HAVE_LDNS], [1],
                  [Define to 1 if xmlrcp-c libraries are available])
    else
        LDNS_CFLAGS=""
        LDNS_LDFLAGS=""
        LDNS_VERSION=""
    fi

    AC_SUBST([LDNS_VERSION])
    AC_SUBST([LDNS_CFLAGS])
    AC_SUBST([LDNS_LDFLAGS])
])

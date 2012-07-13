# ===========================================================================
# ===========================================================================
#
# SYNOPSIS
#
#   AX_LIB_CUPS([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   This macro provides tests of availability of CUPS client library of
#   particular version or newer.
#
#   AX_LIB_CUPS macro takes only one argument which is optional. If there
#   is no required version passed, then macro does not run version test.
#
#   The --with-cups option takes one of three possible values:
#
#   no - do not check for CUPS client library
#
#   yes - do check for CUPS library in standard locations (cups_config
#   should be in the PATH)
#
#   path - complete path to cups_config utility, use this option if
#   cups_config can't be found in the PATH
#
#   This macro calls:
#
#     AC_SUBST(CUPS_CFLAGS)
#     AC_SUBST(CUPS_LDFLAGS)
#     AC_SUBST(CUPS_VERSION)
#
#   And sets:
#
#     HAVE_CUPS
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

AC_DEFUN([AX_LIB_CUPS],
[
    AC_ARG_WITH([cups],
        AS_HELP_STRING([--with-cups=@<:@ARG@:>@],
            [use CUPS client library @<:@default=yes@:>@, optionally specify path to cups_config]
        ),
        [
        if test "$withval" = "no"; then
            want_cups="no"
        elif test "$withval" = "yes"; then
            want_cups="yes"
        else
            want_cups="yes"
            CUPS_CONFIG="$withval"
        fi
        ],
        [want_cups="yes"]
    )
    AC_ARG_VAR([CUPS_CONFIG], [Full path to cups_config program])

    CUPS_CFLAGS=""
    CUPS_LDFLAGS=""
    CUPS_VERSION=""

    dnl
    dnl Check CUPS libraries
    dnl

    if test "$want_cups" = "yes"; then

        if test -z "$CUPS_CONFIG" ; then
            AC_PATH_PROGS([CUPS_CONFIG], [cups-config], [no])
        fi

        if test "$CUPS_CONFIG" != "no"; then
            CUPS_CFLAGS="`$CUPS_CONFIG --cflags`"
            CUPS_LDFLAGS="`$CUPS_CONFIG --ldflags` `$CUPS_CONFIG --libs`"

            CUPS_VERSION=`$CUPS_CONFIG --version`

            found_cups="yes"
        else
            found_cups="no"
        fi
    fi

    dnl
    dnl Check if required version of CUPS is available
    dnl


    cups_version_req=ifelse([$1], [], [], [$1])

    if test "$found_cups" = "yes" -a -n "$cups_version_req"; then

        AC_MSG_CHECKING([if CUPS version is >= $cups_version_req])

        dnl Decompose required version string of CUPS
        dnl and calculate its number representation
        cups_version_req_major=`expr $cups_version_req : '\([[0-9]]*\)'`
        cups_version_req_minor=`expr $cups_version_req : '[[0-9]]*\.\([[0-9]]*\)'`
        cups_version_req_micro=`expr $cups_version_req : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
        if test "x$cups_version_req_micro" = "x"; then
            cups_version_req_micro="0"
        fi

        cups_version_req_number=`expr $cups_version_req_major \* 1000000 \
                                   \+ $cups_version_req_minor \* 1000 \
                                   \+ $cups_version_req_micro`

        dnl Decompose version string of installed CUPS
        dnl and calculate its number representation
        cups_version_major=`expr $CUPS_VERSION : '\([[0-9]]*\)'`
        cups_version_minor=`expr $CUPS_VERSION : '[[0-9]]*\.\([[0-9]]*\)'`
        cups_version_micro=`expr $CUPS_VERSION : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
        if test "x$cups_version_micro" = "x"; then
            cups_version_micro="0"
        fi

        cups_version_number=`expr $cups_version_major \* 1000000 \
                                   \+ $cups_version_minor \* 1000 \
                                   \+ $cups_version_micro`

        cups_version_check=`expr $cups_version_number \>\= $cups_version_req_number`
        if test "$cups_version_check" = "1"; then
            AC_MSG_RESULT([yes])
        else
            AC_MSG_RESULT([no])
	    found_cups="no"
        fi
    fi

    if test "$found_cups" = "yes" ; then
        AC_DEFINE([HAVE_CUPS], [1],
                  [Define to 1 if CUPS libraries are available])
    else
        CUPS_CFLAGS=""
        CUPS_LDFLAGS=""
        CUPS_VERSION=""
    fi

    AC_SUBST([CUPS_VERSION])
    AC_SUBST([CUPS_CFLAGS])
    AC_SUBST([CUPS_LDFLAGS])
])

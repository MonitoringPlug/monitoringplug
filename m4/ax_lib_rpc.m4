# ===========================================================================
#
# SYNOPSIS
#
#   AX_LIB_RPC()
#
# DESCRIPTION
#
#   This macro provides tests of availability of RPC Library of
#   particular version or newer.
#
#   This macro calls:
#
#     AC_SUBST(RPC_CFLAGS)
#     AC_SUBST(RPC_LDFLAGS)
#
#   And sets:
#
#     HAVE_RPC
#
# LICENSE
#
#   Copyright (c) 2012 Marius Rieder <marius.rieder@durchmesser.ch>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 1

AC_DEFUN([AX_LIB_RPC],
[
    AH_TEMPLATE([HAVE_RPC], [Define if rpc is available])
    AC_ARG_WITH([rpc],
        AS_HELP_STRING([--with-rpc=@<:@ARG@:>@],
            [use RPC Library from given prefix (ARG=path); check standard prefixes (ARG=yes); disable (ARG=no)]
        ),
        [
        if test "$withval" = "yes"; then
            if test -f /usr/local/include/rpc/rpc.h ; then
                rpc_prefix=/usr/local
            elif test -f /usr/include/rpc/rpc.h ; then
                rpc_prefix=/usr
            else
                rpc_prefix=""
            fi
            rpc_requested="yes"
        elif test -d "$withval"; then
            rpc_prefix="$withval"
            rpc_requested="yes"
        else
            rpc_prefix=""
            rpc_requested="no"
        fi
        ],
        [
        dnl Default behavior is implicit yes
        if test -f /usr/local/include/rpc/rpc.h ; then
            rpc_prefix=/usr/local
        elif test -f /usr/include/rpc/rpc.h ; then
            rpc_prefix=/usr
        else
            rpc_prefix=""
        fi
        ]
    )

    AC_ARG_WITH([rpc-inc],
        AS_HELP_STRING([--with-rpc-inc=@<:@DIR@:>@],
            [path to the RPC headers]
        ),
        [rpc_include_dir="$withval"],
        [rpc_include_dir=""]
    )
    AC_ARG_WITH([rpc-lib],
        AS_HELP_STRING([--with-rpc-lib=@<:@ARG@:>@],
            [link options for the RPC libraries]
        ),
        [rpc_lib_flags="$withval"],
        [rpc_lib_flags=""]
    )

    RPC_CFLAGS=""
    RPC_LDFLAGS=""
    RPC_VERSION=""

    dnl
    dnl Collect include/lib paths and flags
    dnl
    run_rpc_test="no"

    if test -n "$rpc_prefix"; then
        rpc_include_dir="$rpc_prefix/include"
        rpc_lib_flags="-L$rpc_prefix/lib -lrpcsvc"
        run_rpc_test="yes"
    elif test "$rpc_requested" = "yes"; then
        if test -n "$rpc_include_dir" -a -n "$rpc_lib_flags"; then
            run_rpc_test="yes"
        fi
    else
        run_rpc_test="no"
    fi

    dnl
    dnl Check RPC Library files
    dnl
    if test "$run_rpc_test" = "yes"; then

        saved_CPPFLAGS="$CPPFLAGS"
        CPPFLAGS="$CPPFLAGS -I$rpc_include_dir"

        saved_LDFLAGS="$LDFLAGS"
        LDFLAGS="$LDFLAGS $rpc_lib_flags"

        dnl
        dnl Check Expat headers
        dnl
        AC_MSG_CHECKING([for RPC Library headers in $rpc_include_dir])

        AC_LANG_PUSH([C])
        AC_COMPILE_IFELSE([
            AC_LANG_PROGRAM(
                [[
@%:@include <rpc/rpc.h>
                ]],
                [[]]
            )],
            [
            RPC_CFLAGS="-I$rpc_include_dir"
            rpc_header_found="yes"
            AC_MSG_RESULT([found])
            ],
            [
            rpc_header_found="no"
            AC_MSG_RESULT([not found])
            ]
        )
        AC_LANG_POP([C])

        dnl
        dnl Check Expat libraries
        dnl
        if test "$rpc_header_found" = "yes"; then

            AC_MSG_CHECKING([for RPC libraries])

            AC_LANG_PUSH([C])
            AC_LINK_IFELSE([
                AC_LANG_PROGRAM(
                    [[
@%:@include <rpc/rpc.h>
                    ]],
                    [[
struct rpcent *ent;
ent = getrpcbyname("nfs");
                    ]]
                )],
                [
                RPC_LDFLAGS="$rpc_lib_flags"
                rpc_lib_found="yes"
                AC_MSG_RESULT([found])
                ],
                [
                rpc_lib_found="no"
                AC_MSG_RESULT([not found])
                ]
            )
            AC_LANG_POP([C])
        fi

        CPPFLAGS="$saved_CPPFLAGS"
        LDFLAGS="$saved_LDFLAGS"
    fi

    AC_MSG_CHECKING([for RPC Library])

    if test "$run_rpc_test" = "yes"; then
        if test "$rpc_header_found" = "yes" -a "$rpc_lib_found" = "yes"; then

	    AC_DEFINE(HAVE_RPC,1,
	                 [Define to 1 if you have a functional rpc library.])
            AC_SUBST([RPC_CFLAGS])
            AC_SUBST([RPC_LDFLAGS])

            HAVE_RPC="yes"
        else
            HAVE_RPC="no"
        fi

        AC_MSG_RESULT([$HAVE_RPC])

    else
        HAVE_RPC="no"
        AC_MSG_RESULT([$HAVE_RPC])

        if test "$rpc_requested" = "yes"; then
            AC_MSG_WARN([RPC Library support requested but headers or library not found. Specify valid prefix of the RPC Libarary using --with-rpc=@<:@DIR@:>@ or provide include directory and linker flags using --with-rpc-inc and --with-rpc-lib])
        fi
    fi
])


#
#
#
#

AC_DEFUN([AX_LIB_LDNS],
[
    AC_ARG_WITH([ldns],
        AS_HELP_STRING([--with-ldns=@<:@ARG@:>@],
            [use ldns from given prefix (ARG=path); check standard prefixes (ARG=yes); disable (ARG=no)]
        ),
        [
        if test "$withval" = "yes"; then
            if test -f /usr/local/include/ldns/ldns.h ; then
                ldns_prefix=/usr/local
            elif test -f /usr/include/ldns/ldns.h ; then
                ldns_prefix=/usr
            else
                ldns_prefix=""
            fi
            ldns_requested="yes"
        elif test -d "$withval"; then
            ldns_prefix="$withval"
            ldns_requested="yes"
        else
            ldns_prefix=""
            ldns_requested="no"
        fi
        ],
        [
        dnl Default behavior is implicit yes
        if test -f /usr/local/include/ldns/ldns.h ; then
            ldns_prefix=/usr/local
        elif test -f /usr/include/ldns/ldns.h ; then
            ldns_prefix=/usr
        else
            ldns_prefix=""
        fi
        ]
    )
    AC_ARG_WITH([ldns-inc],
        AS_HELP_STRING([--with-ldns-inc=@<:@DIR@:>@],
            [path to ldns headers]
        ),
        [ldns_include_dir="$withval"],
        [ldns_include_dir=""]
    )
    AC_ARG_WITH([ldns-lib],
        AS_HELP_STRING([--with-ldns-lib=@<:@ARG@:>@],
            [link options for ldns libraries]
        ),
        [ldns_lib_flags="$withval"],
        [ldns_lib_flags=""]
    )

    LDNS_CFLAGS=""
    LDNS_LIBS=""
    LDNS_VERSION=""

    dnl
    dnl Collect include/lib paths and flags
    dnl
    run_ldns_test="no"

    if test -n "$ldns_prefix"; then
        ldns_include_dir="$ldns_prefix/include"
        ldns_lib_flags="-L$ldns_prefix/lib -lldns"
        run_ldns_test="yes"
    elif test "$ldns_requested" = "yes"; then
        if test -n "$ldns_include_dir" -a -n "$ldns_lib_flags"; then
            run_ldns_test="yes"
        fi
    else
        run_ldns_test="no"
    fi

    dnl
    dnl Check ldns files
    dnl
    if test "$run_ldns_test" = "yes"; then

        saved_CPPFLAGS="$CPPFLAGS"
        CPPFLAGS="$CPPFLAGS -I$ldns_include_dir"

        saved_LDFLAGS="$LDFLAGS"
        LIBS="$LDFLAGS $ldns_lib_flags"

        dnl
        dnl Check ldns headers
        dnl
        AC_MSG_CHECKING([for ldns headers in $ldns_include_dir])

        AC_LANG_PUSH([C])
        AC_COMPILE_IFELSE([
            AC_LANG_PROGRAM(
                [[
@%:@include <ldns/ldns.h>
                ]],
                [[]]
            )],
            [
            LDNS_CFLAGS="-I$ldns_include_dir"
            ldns_header_found="yes"
            AC_MSG_RESULT([found])
            ],
            [
            ldns_header_found="no"
            AC_MSG_RESULT([not found])
            ]
        )
        AC_LANG_POP([C])

        dnl
        dnl Check ldns libraries
        dnl
        if test "$ldns_header_found" = "yes"; then

            AC_MSG_CHECKING([for ldns libraries])

            AC_LANG_PUSH([C])
            AC_LINK_IFELSE([
                AC_LANG_PROGRAM(
                    [[
@%:@include <ldns/ldns.h>
                    ]],
                    [[
ldns_rdf *domain;
domain = ldns_dname_new_frm_str("nlnetlabs.nl");
ldns_rdf_deep_free(domain);
domain = NULL;
                    ]]
                )],
                [
                LDNS_LIBS="$ldns_lib_flags"
                ldns_lib_found="yes"
                AC_MSG_RESULT([found])
                ],
                [
                ldns_lib_found="no"
                AC_MSG_RESULT([not found])
                ]
            )
            AC_LANG_POP([C])
        fi

        CPPFLAGS="$saved_CPPFLAGS"
        LDFLAGS="$saved_LDFLAGS"
    fi

    AC_MSG_CHECKING([for ldns])

    if test "$run_ldns_test" = "yes"; then
        if test "$ldns_header_found" = "yes" -a "$ldns_lib_found" = "yes"; then

            AC_SUBST([LDNS_CFLAGS])
            AC_SUBST([LDNS_LIBS])

            HAVE_LDNS="yes"
        else
            HAVE_LDNS="no"
        fi

        AC_MSG_RESULT([$HAVE_LDNS])

        dnl
        dnl Check ldns version
        dnl
        if test "$HAVE_LDNS" = "yes"; then

            ldns_version_req=ifelse([$1], [], [], [$1])

            if test  -n "$ldns_version_req"; then

                AC_MSG_CHECKING([if ldns version is >= $ldns_version_req])

		if test -f "$ldns_include_dir/ldns/util.h"; then
		    LDNS_VERSION=`grep '^#define.*LDNS_VERSION.*"[.0-9]*"$' $ldns_include_dir/ldns/util.h | \
                                    sed -e 's/^.*\"\(.*\)\"/\1/'`
                    AC_SUBST([LDNS_VERSION])
		fi

		saved_CPPFLAGS="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS -I$ldns_include_dir"

		saved_LDFLAGS="$LDFLAGS"
		LIBS="$LDFLAGS $ldns_lib_flags"

                AC_LANG_PUSH([C])
                AC_RUN_IFELSE([
                    AC_LANG_PROGRAM(
                        [[
@%:@include <ldns/ldns.h>
                        ]],
                        [[
int major, minor, micro, minvers;
char *tmp_version;

tmp_version = strdup("$ldns_version_req");

if (strlen(tmp_version) == 0)
  return 0;

if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
  printf("%s, bad version string\n", "$min_version");
  return 1;
}

minvers =  ((major<<16)|(minor<<8)|(micro));
if (minvers > LDNS_REVISION) {
  return 1;
}

return 0;
		        ]]
                    )],
                    [
                    ldns_version_check="yes"
                    AC_MSG_RESULT([yes])
                    ],
                    [
                    ldns_version_check="no"
                    AC_MSG_RESULT([no])
                    AC_MSG_WARN([Found ldns $LDNS_VERSION, which is older than required. Possible compilation failure.])
                    ]
                )
                AC_LANG_POP([C])

	        CPPFLAGS="$saved_CPPFLAGS"
		LDFLAGS="$saved_LDFLAGS"
            fi
        fi

    else
        HAVE_LDNS="no"
        AC_MSG_RESULT([$HAVE_LDNS])

        if test "$ldns_requested" = "yes"; then
            AC_MSG_WARN([ldns support requested but headers or library not found. Specify valid prefix of ldns using --with-ldns=@<:@DIR@:>@ or provide include directory and linker flags using --with-ldns-inc and --with-ldns-lib])
        fi
    fi
])

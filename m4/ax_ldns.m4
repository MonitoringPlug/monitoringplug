dnl AC_LIB_LDNS([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])

AC_DEFUN([AC_LIB_LDNS], [
  AH_TEMPLATE([HAVE_LDNS], [Define if ldns is available])
  AC_ARG_WITH([ldns],
    AS_HELP_STRING([--with-ldns=PATH], [prefix where check is installed [default=auto]]))

  min_version=ifelse([$1], ,,$1)
  if test "x$min_version" = "x"; then
    AC_MSG_CHECKING(for ldns)
  else
    AC_MSG_CHECKING(for ldns - version >= $min_version)
  fi

  if test "x$with_ldns" = "xdisabled"; then
    AC_MSG_RESULT(disabled)
    ifelse([$3], , AC_MSG_ERROR([disabling ldns is not supported]), [$3])
  else
   if test "x$with_ldns" != "xno"; then
      LDNS_CFLAGS="-I$with_ldns/include"
      LDNS_LIBS="-L$with_ldns/lib -lldns"
    else
      LDNS_CFLAGS=""
      LDNS_LIBS="-lldns"
    fi
 
    ac_save_CFLAGS="$CFLAGS"
    ac_save_LIBS="$LIBS"

    CFLAGS="$CFLAGS $LDNS_CFLAGS"
    LIBS="$LDNS_LIBS $LIBS"

    AC_COMPILE_IFELSE([AC_LANG_SOURCE([AC_INCLUDES_DEFAULT([])
#include <ldns/ldns.h>

int main () {
  int major, minor, micro, minvers;
  char *tmp_version;

  tmp_version = strdup("$min_version");

  if (strlen(tmp_version) == 0)
    return 0;

  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
    printf("%s, bad version string\n", "$min_version");
    return 1;
  }
   
  minvers =  ((major<<16)|(minor<<8)|(micro));
  if (minvers < LDNS_REVISION) {
    printf("\n*** An old version of ldns was found: %s\n", ldns_version());
    printf("*** You need a version of ldns being at least %s.\n", tmp_version);
    printf("***\n"); 
    printf("*** If you have already installed a sufficiently new version, this error\n");
    printf("*** probably means that the wrong copy of the ldns library and header\n");
    printf("*** file is being found. Rerun configure with the --with-ldns=PATH option\n");
    printf("*** to specify the prefix where the correct version was installed.\n");
    return 1;
  }
  return 0;
}
    ])],, no_ldns=yes, [echo $ac_n "cross compiling; assumed OK... $ac_c"])

    CFLAGS="$ac_save_CFLAGS"
    LIBS="$ac_save_LIBS"

    if test "x$no_ldns" = x ; then
      AC_MSG_RESULT(yes)
      ifelse([$2], , :, [$2])
    else
      AC_MSG_RESULT(no)
      ifelse([$3], , AC_MSG_ERROR([check not found]), [$3])
      LDNS_CFLAGS=""
      LDNS_LIBS=""
    fi

    AC_SUBST(LDNS_CFLAGS)
    AC_SUBST(LDNS_LIBS)

  fi
])

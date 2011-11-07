dnl AC_LIB_CUPS([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])

AC_DEFUN([AC_LIB_CUPS], [
  AH_TEMPLATE([HAVE_CUPS], [Define if cups is available])
  AC_ARG_WITH([cups],
    AS_HELP_STRING([--with-cups=PATH], [prefix where check is installed [default=auto]]))

  if test "x$with_cups" = "xdisabled"; then
    AC_MSG_CHECKING(for cups)
    AC_MSG_RESULT(disabled)
    ifelse([$3], , AC_MSG_ERROR([disabling cups is not supported]), [$3])
  else
    if test -d "$with_cups" ; then
      AC_PATH_PROG([cups_config],[cups-config],["$withval/bin"],["$withval/bin"])
    else 
      AC_PATH_PROG([cups_config],[cups-config])
    fi
  fi

  if test x$cups_config != "x" ; then
    min_version=ifelse([$1], ,,$1)
    if test "x$min_version" = "x"; then
      AC_MSG_CHECKING(for cups)
    else
      AC_MSG_CHECKING(for cups - version >= $min_version)
    fi
    if test "x$with_cups" != "xno"; then
      CUPS_CFLAGS="-I$with_cups/include"
      CUPS_LDFLAGS="-L$with_cups/lib `$cups_config --libs`"
    else
      CUPS_CFLAGS=""
      CUPS_LDFLAGS="`$cups_config --libs`"
    fi
 
    ac_save_CFLAGS="$CFLAGS"
    ac_save_LIBS="$LIBS"

    CFLAGS="$CFLAGS $CUPS_CFLAGS"
    LIBS="$CUPS_LIBS $LIBS"

    AC_RUN_IFELSE([AC_LANG_SOURCE([AC_INCLUDES_DEFAULT([])
#include <cups/cups.h>

int main () {
  int major, minor, micro, minvers, vers;
  char *tmp_version;

  tmp_version = strdup("$min_version");

  if (strlen(tmp_version) == 0)
    return 0;

  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
    printf("%s, bad version string\n", "$min_version");
    return 1;
  }

  minvers =  ((major<<16)|(minor<<8)|(micro));
  vers = ((CUPS_VERSION_MAJOR<<16)|(CUPS_VERSION_MINOR<<8)|(CUPS_VERSION_PATCH));

  if (minvers > vers) {
    printf("\n*** An old version of cups was found: %f\n", CUPS_VERSION);
    printf("*** You need a version of cups being at least %s.\n", tmp_version);
    printf("***\n"); 
    printf("*** If you have already installed a sufficiently new version, this error\n");
    printf("*** probably means that the wrong copy of the cups library and header\n");
    printf("*** file is being found. Rerun configure with the --with-cups=PATH option\n");
    printf("*** to specify the prefix where the correct version was installed.\n");
    return 1;
  } else {
    printf("\n*** %s > %d.%d.%d ***\n", tmp_version, CUPS_VERSION_MAJOR, CUPS_VERSION_MINOR, CUPS_VERSION_PATCH);
  }

  return 0;

}
    ])],, no_cups=yes,)

    CFLAGS="$ac_save_CFLAGS"
    LIBS="$ac_save_LIBS"

    if test "x$no_cups" = x ; then
      AC_MSG_RESULT(yes)
      ifelse([$2], , :, [$2])
    else
      AC_MSG_RESULT(no)
      ifelse([$3], , AC_MSG_ERROR([cups not found]), [$3])
      CUPS_CFLAGS=""
      CUPS_LIBS=""
    fi

    AC_SUBST(CUPS_CFLAGS)
    AC_SUBST(CUPS_LDFLAGS)

  fi
])

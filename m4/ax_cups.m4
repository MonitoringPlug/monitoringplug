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
    AC_MSG_CHECKING(for cups)
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

    AC_COMPILE_IFELSE([AC_LANG_SOURCE([AC_INCLUDES_DEFAULT([])
#include <cups/cups.h>

int main () {
  char *server;

  server = cupsServer();

  return 0;

}
    ])],, no_cups=yes, [echo $ac_n "cross compiling; assumed OK... $ac_c"])

    CFLAGS="$ac_save_CFLAGS"
    LIBS="$ac_save_LIBS"

    if test "x$no_cups" = x ; then
      AC_MSG_RESULT(yes)
      ifelse([$2], , :, [$2])
    else
      AC_MSG_RESULT(no)
      ifelse([$3], , AC_MSG_ERROR([check not found]), [$3])
      CUPS_CFLAGS=""
      CUPS_LIBS=""
    fi

    AC_SUBST(CUPS_CFLAGS)
    AC_SUBST(CUPS_LDFLAGS)

  fi
])

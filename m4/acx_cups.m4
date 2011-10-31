# $Id$

AC_DEFUN([ACX_CUPS],[
	AC_ARG_WITH(cups, 
		[AC_HELP_STRING([--with-cups=PATH],[specify prefix of path of cups library to use])],
        	[
			CUPS_PATH="$withval"
			AC_PATH_PROGS(CUPS_CONFIG, cups-config, cups-config, $CUPS_PATH/bin)
		],[
			AC_PATH_PROGS(CUPS_CONFIG, cups-config, cups-config, $PATH)
		])

	if test -x "$CUPS_CONFIG"
	then
		AC_MSG_CHECKING(what are the cups includes)
		CUPS_INCLUDES="`$CUPS_CONFIG --cflags`"
		AC_MSG_RESULT($CUPS_INCLUDES)

		AC_MSG_CHECKING(what are the cups libs)
		CUPS_LIBS="`$CUPS_CONFIG --libs`"
		AC_MSG_RESULT($CUPS_LIBS)
	else
		AC_MSG_CHECKING(what are the cups includes)
		CUPS_INCLUDES="-I$CUPS_PATH/include"
		AC_MSG_RESULT($CUPS_INCLUDES)

		AC_MSG_CHECKING(what are the cups libs)
		CUPS_LIBS="-L$CUPS_PATH/lib -lcups"
		AC_MSG_RESULT($CUPS_LIBS)
	fi

	tmp_CPPFLAGS=$INCLUDES
	tmp_LIBS=$LIBS

	CPPFLAGS="$CPPFLAGS $CUPS_INCLUDES"
	LIBS="$LIBS $CUPS_LIBS"

	AC_CHECK_LIB(cups, cups_rr_new,,[AC_MSG_ERROR([Can't find cups library])])
	LIBS=$tmp_LIBS

	CPPFLAGS=$tmp_INCLUDES

	AC_SUBST(CUPS_INCLUDES)
	AC_SUBST(CUPS_LIBS)
])

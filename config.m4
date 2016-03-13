dnl $Id$
dnl config.m4 for extension mfcobol

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(mfcobol, for mfcobol support,
dnl Make sure that the comment is aligned:
dnl [  --with-mfcobol             Include mfcobol support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(mfcobol, whether to enable mfcobol support,
dnl Make sure that the comment is aligned:
[  --enable-mfcobol           Enable mfcobol support])

if test "$PHP_MFCOBOL" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-mfcobol -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/mfcobol.h"  # you most likely want to change this
  dnl if test -r $PHP_MFCOBOL/$SEARCH_FOR; then # path given as parameter
  dnl   MFCOBOL_DIR=$PHP_MFCOBOL
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for mfcobol files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       MFCOBOL_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$MFCOBOL_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the mfcobol distribution])
  dnl fi

  dnl # --with-mfcobol -> add include path
  dnl PHP_ADD_INCLUDE($MFCOBOL_DIR/include)

  dnl # --with-mfcobol -> check for lib and symbol presence
  dnl LIBNAME=mfcobol # you may want to change this
  dnl LIBSYMBOL=mfcobol # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $MFCOBOL_DIR/lib, MFCOBOL_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_MFCOBOLLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong mfcobol lib version or lib not found])
  dnl ],[
  dnl   -L$MFCOBOL_DIR/lib -lm -ldl
  dnl ])
  dnl
  dnl PHP_SUBST(MFCOBOL_SHARED_LIBADD)

  AC_DEFINE(MFCOBOL, 1, [ ])
  dnl AC_DEFINE(DEBUG, 1, [ ])
  dnl AC_DEFINE(MFC_DEBUG, 1, [ ])
  dnl AC_DEFINE(MFC_DDEBUG, 1, [ ])
  PHP_ADD_INCLUDE(vbisam)
  PHP_NEW_EXTENSION(mfcobol, mfcobol.c \
  	vbisam/isDecimal.c \
	vbisam/isHelper.c \
	vbisam/isaudit.c \
	vbisam/isbuild.c \
	vbisam/isdelete.c \
	vbisam/isopen.c \
	vbisam/isread.c \
	vbisam/isrecover.c \
	vbisam/isrewrite.c \
	vbisam/istrans.c \
	vbisam/iswrite.c \
	vbisam/vbBlockIO.c \
	vbisam/vbDataIO.c \
	vbisam/vbIndexIO.c \
	vbisam/vbKeysIO.c \
	vbisam/vbLocking.c \
	vbisam/vbLowLevel.c \
	vbisam/vbMemIO.c \
	vbisam/vbNodeMemIO.c \
	vbisam/vbVarLenIO.c \
   , $ext_shared)
fi

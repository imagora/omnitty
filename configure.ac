dnl Process this file with autoconf to produce a configure script.
AC_INIT([omnitty], 0.3.0)

dnl Make the symbol PACKAGE_VERSION get exported
AC_SUBST(PACKAGE_VERSION)

dnl Checks for libraries.
AC_MSG_CHECKING([whether libROTE is installed])
rote-config --version >/dev/null
if test $? -ne 0; then
   AC_MSG_RESULT([no])
   echo "rote-config not found in path. Is the ROTE library installed?"
   echo "You can download it from http://rote.sf.net"
   exit 1
fi
AC_MSG_RESULT([yes])

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T

dnl Checks for library functions.
AC_TYPE_SIGNAL
AC_CHECK_FUNCS(strdup)

dnl Appends ROTE flags to CFLAGS and LIBS
ROTE_CFLAGS=`rote-config --cflags`
if test $? -ne 0; then
   AC_MSG_ERROR([Error running rote-config --cflags], [1])
fi
CFLAGS="$CFLAGS $ROTE_CFLAGS"

ROTE_LIBS=`rote-config --libs`
if test $? -ne 0; then
   AC_MSG_ERROR([Error running rote-config --libs], [1])
fi
LIBS="$LIBS $ROTE_LIBS"

AC_OUTPUT(Makefile)


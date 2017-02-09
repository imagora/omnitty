dnl Process this file with autoconf to produce a configure script.

AC_INIT([rote], 0.2.8)

AC_SUBST(PACKAGE_NAME)
AC_SUBST(PACKAGE_VERSION)

AC_CONFIG_SRCDIR([rote.c])

dnl Checks for programs.
AC_PROG_CC

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([sys/types.h unistd.h stdlib.h string.h])

dnl Check for libraries
AC_CHECK_LIB([ncurses], [initscr], [],
                AC_MSG_ERROR([Need ncurses to compile librote.]))
AC_CHECK_LIB([util], [forkpty], [],
      AC_MSG_ERROR([Need util library (forkpty function) to compile librote.]))

dnl Checks for library functions
AC_CHECK_FUNCS([memset select setenv])
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIGNAL

AC_CONFIG_FILES([Makefile rote-config])
AC_OUTPUT


#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.59)
AC_INIT(mgplus-samples, 1.0.0)
AC_CONFIG_SRCDIR([src/mgcompositedemo.c])

AC_CANONICAL_SYSTEM

dnl checks for programs
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LD
AC_PROG_RANLIB
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AM_INIT_AUTOMAKE

dnl ========================================================================
dnl Put the name and version of the package here

# Checks for libraries.

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/ioctl.h sys/time.h termio.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([bzero dup2 getcwd memmove memset putenv strchr strdup strerror strstr])

PKG_CHECK_MODULES([MINIGUI], [minigui >= 3.0.10])
PKG_CHECK_MODULES([MGPLUS], [mgplus >= 1.2.0])
APP_LIBS="$APP_LIBS $MGPLUS_LIBS $MINIGUI_LIBS"

dnl ========================================================================
dnl check for run-time mode of MiniGUI
dnl ========================================================================
threads_version="no"
AC_CHECK_DECLS(_MGRM_THREADS, threads_version="yes", foo=bar, [#include <minigui/common.h>])
AM_CONDITIONAL(MGRM_THREADS, test "x$threads_version" = "xyes")
procs_version="no"
AC_CHECK_DECLS(_MGRM_PROCESSES, procs_version="yes", foo=bar, [#include <minigui/common.h>])
AM_CONDITIONAL(MGRM_PROCESSES, test "x$procs_version" = "xyes")
standalone_version="no"
AC_CHECK_DECLS(_MGRM_STANDALONE, standalone_version="yes", foo=bar, [#include <minigui/common.h>])
AM_CONDITIONAL(MGRM_STANDALONE, test "x$standalone_version" = "xyes")

if test "$ac_cv_prog_gcc" = "yes"; then
    CFLAGS="$CFLAGS -Wall -Wstrict-prototypes -pipe"
fi

AC_SUBST(APP_LIBS)

AC_OUTPUT(
    Makefile
    animate/Makefile
    animate/common_animates/Makefile
    src/Makefile
    src/res/Makefile
    pen/Makefile 
    path/Makefile 
    brush/Makefile 
    colorcomposite/Makefile 
    tcldemo/Makefile 
    tcldemo/res/Makefile 
    glyphoutline/Makefile 
    worldtransform/Makefile 
)


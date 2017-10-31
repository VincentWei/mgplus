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

have_libminigui="no"
AC_CHECK_HEADERS(minigui/common.h, have_libminigui=yes, foo=bar)

dnl ========================================================================
dnl check for run-time mode of MiniGUI
dnl ========================================================================
threads_version="no"
AC_CHECK_DECLS(_MGRM_THREADS, threads_version="yes", foo=bar, [#include <minigui/common.h>])

procs_version="no"
AC_CHECK_DECLS(_MGRM_PROCESSES, procs_version="yes", foo=bar, [#include <minigui/common.h>])

standalone_version="no"
AC_CHECK_DECLS(_MGRM_STANDALONE, standalone_version="yes", foo=bar, [#include <minigui/common.h>])

if test "x$threads_version" = "xyes"; then
    CFLAGS="$CFLAGS -D_REENTRANT"
    APP_LIBS="$APP_LIBS -lpthread -lminigui_ths -lstdc++ -ldl"
else
if test "x$standalone_version" = "xyes"; then
    CFLAGS="$CFLAGS -D_REENTRANT"
    APP_LIBS="$APP_LIBS -lminigui_sa -lstdc++ -ldl"
else
    APP_LIBS="$APP_LIBS -lminigui_procs -lstdc++ -ldl"
fi  
fi

AC_CHECK_DECLS(_HAVE_MATH_LIB, APP_LIBS="$APP_LIBS -lm", foo=bar, [#include <minigui/common.h>])
AC_CHECK_DECLS(_MGIMAGE_PNG, APP_LIBS="$APP_LIBS -lpng", foo=bar, [#include <minigui/common.h>])
AC_CHECK_DECLS(_MGIMAGE_PNG, APP_LIBS="$APP_LIBS -lpthread", foo=bar, [#include <minigui/common.h>])
AC_CHECK_DECLS(_MGIMAGE_JPG, APP_LIBS="$APP_LIBS -ljpeg", foo=bar, [#include <minigui/common.h>])
AC_CHECK_DECLS(_MGFONT_TTF, APP_LIBS="$APP_LIBS -lttf", foo=bar, [#include <minigui/common.h>])
AC_CHECK_DECLS(_MGFONT_FT2, APP_LIBS="$APP_LIBS -lfreetype", foo=bar, [#include <minigui/common.h>])
AC_CHECK_DECLS(_MGFONT_SEF, APP_LIBS="$APP_LIBS -lscripteasy", foo=bar, [#include <minigui/common.h>])

AM_CONDITIONAL(MGRM_THREADS, test "x$threads_version" = "xyes")
AM_CONDITIONAL(MGRM_PROCESSES, test "x$procs_version" = "xyes")
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
dnl tcldemo/Makefile 
dnl tcldemo/res/Makefile 
glyphoutline/Makefile 
worldtransform/Makefile 

)

if test "x$have_libminigui" != "xyes"; then
    AC_MSG_WARN([
		MiniGUI is not properly installed on the system. You need 
        MiniGUI Ver 2.2.x or later for building this package. 
        Please configure and install MiniGUI Ver 2.2.x first.
    ])
fi

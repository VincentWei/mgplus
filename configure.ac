#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.60)
AC_INIT(libmgplus, 1.4.0)
AC_CONFIG_SRCDIR(src/agg/agg_platform_support.cpp)

dnl Set various version strings
#
# Making releases:
# Change the version, then:
#   _MICRO_VERSION += 1;
#   _INTERFACE_AGE += 1;
#   _BINARY_AGE += 1;
# if any functions have been added, set _INTERFACE_AGE to 0.
# if backwards compatibility has been broken,
# set _BINARY_AGE and _INTERFACE_AGE to 0.
#
MGPLUS_MAJOR_VERSION=1
MGPLUS_MINOR_VERSION=4
MGPLUS_MICRO_VERSION=0
MGPLUS_INTERFACE_AGE=0
MGPLUS_BINARY_AGE=0
MGPLUS_VERSION=$MGPLUS_MAJOR_VERSION.$MGPLUS_MINOR_VERSION.$MGPLUS_MICRO_VERSION

AC_SUBST(MGPLUS_MAJOR_VERSION)
AC_SUBST(MGPLUS_MINOR_VERSION)
AC_SUBST(MGPLUS_MICRO_VERSION)
AC_SUBST(MGPLUS_INTERFACE_AGE)
AC_SUBST(MGPLUS_BINARY_AGE)
AC_SUBST(MGPLUS_VERSION)

# libtool versioning
LT_RELEASE=$MGPLUS_MAJOR_VERSION.$MGPLUS_MINOR_VERSION
LT_CURRENT=`expr $MGPLUS_MICRO_VERSION - $MGPLUS_INTERFACE_AGE`
LT_REVISION=$MGPLUS_INTERFACE_AGE
LT_AGE=`expr $MGPLUS_BINARY_AGE - $MGPLUS_INTERFACE_AGE`

AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

AC_DEFINE_UNQUOTED(MGPLUS_MAJOR_VERSION, $MGPLUS_MAJOR_VERSION,
            [Major version of mGPlus])
AC_DEFINE_UNQUOTED(MGPLUS_MINOR_VERSION, $MGPLUS_MINOR_VERSION,
            [Minor version of mGPlus])
AC_DEFINE_UNQUOTED(MGPLUS_MICRO_VERSION, $MGPLUS_MICRO_VERSION,
            [Micro version of mGPlus])
AC_DEFINE_UNQUOTED(MGPLUS_INTERFACE_AGE, $MGPLUS_INTERFACE_AGE,
            [Interface age of mGPlus])
AC_DEFINE_UNQUOTED(MGPLUS_BINARY_AGE, $MGPLUS_BINARY_AGE,
            [Binary age of mGPlus])

dnl ========================================================================
dnl Need for cross-compiling
AC_CANONICAL_SYSTEM

dnl ========================================================================
dnl Init automake
AM_INIT_AUTOMAKE

dnl ========================================================================
# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LD
AC_PROG_RANLIB
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

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
#AC_FUNC_MALLOC
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([bzero dup2 getcwd memmove memset putenv strchr strdup strerror strstr])

devel_mode="no"
detail_debug="no"
build_fashion_lfrdr="yes"
build_colorcomposite_support="yes"
build_gradientfill_support="yes"
build_ft2_support="yes"
#pixfmt_argb32_support="yes"
pixfmt_rgba32_support="yes"
pixfmt_abgr32_support="yes"
pixfmt_bgra32_support="yes"
pixfmt_bgr24_support="yes"
pixfmt_rgb24_support="yes"
pixfmt_rgb555_support="yes"
pixfmt_rgb565_support="yes"

AC_ARG_ENABLE(develmode,
[  --enable-develmode       developer mode <default=no>],
devel_mode=$enableval)

AC_ARG_ENABLE(detail_debug,
[  --enable-detaildebug     detailed debug information <default=no>],
detail_debug=$enableval)

AC_ARG_ENABLE(fashionlf,
[  --enable-fashionlf           build Fashion LFRDR <default=yes>],
build_fashion_lfrdr=$enableval)

AC_ARG_ENABLE(colorcomposite,
[  --enable-colorcomposite      build support for color composite <default=yes>],
build_colorcomposite_support=$enableval)

AC_ARG_ENABLE(gradientfill,
[  --enable-gradientfill        build support for gradient fill <default=yes>],
build_gradientfill_support=$enableval)

AC_ARG_ENABLE(ft2support,
[  --enable-ft2support          build support for FreeType2 library <default=no>],
build_ft2_support=$enableval)

#AC_ARG_ENABLE(argb32,
#[  --enable-argb32              build support for pixfmt argb32 <default=yes>],
#pixfmt_argb32_support=$enableval)

AC_ARG_ENABLE(rgba32,
[  --enable-rgba32              build support for pixfmt rgba32 <default=yes>],
pixfmt_rgba32_support=$enableval)

AC_ARG_ENABLE(abgr32,
[  --enable-abgr32              build support for pixfmt abgr32 <default=yes>],
pixfmt_abgr32_support=$enableval)

AC_ARG_ENABLE(bgra32,
[  --enable-bgra32              build support for pixfmt bgra32 <default=yes>],
pixfmt_bgra32_support=$enableval)

AC_ARG_ENABLE(bgr24,
[  --enable-bgr24               build support for pixfmt bgr24 <default=yes>],
pixfmt_bgr24_support=$enableval)

AC_ARG_ENABLE(rgb24,
[  --enable-rgb24 build support for pixfmt rgb24 <default=yes>],
pixfmt_rgb24_support=$enableval)

AC_ARG_ENABLE(rgb555,
[  --enable-rgb555 build support for pixfmt rgb555 <default=yes>],
pixfmt_rgb555_support=$enableval)

AC_ARG_ENABLE(rgb565,
[  --enable-rgb565 build support for pixfmt rgb565 <default=yes>],
pixfmt_rgb565_support=$enableval)

AC_ARG_WITH(libsuffix,
  [  --with-libsuffix=ARG    configure the suffix of the library name.])

AM_CONFIG_HEADER(mgplusconfig.h)

if test "x$build_fashion_lfrdr" = "xyes"; then
    AC_DEFINE(_MGPLUS_LFRDR_FASHION, 1,
            [Define if include Fashion LFRDR])
    build_gradientfill_support="yes"
    build_colorcomposite_support="yes"
fi

if test "x$build_colorcomposite_support" = "xyes"; then
    AC_DEFINE(_MGPLUS_COLORCOMPOSITE, 1,
            [Define if support color composite])
fi


if test "x$build_gradientfill_support" = "xyes"; then
    AC_DEFINE(_MGPLUS_GRADIENTFILL, 1,
            [Define if support gradient fill])
fi

if test "x$build_ft2_support" == "xyes"; then
    AC_DEFINE(_MGPLUS_FONT_FT2, 1,
             [Define if support TrueType font based on FreeType2])
fi

#if test "x$pixfmt_argb32_support" == "xyes"; then
#    AC_DEFINE(_MGPLUS_PIXFMT_ARGB32, 1,
#             [Define if support pixel format ARGB32])
#fi

if test "x$pixfmt_rgba32_support" == "xyes"; then
    AC_DEFINE(_MGPLUS_PIXFMT_RGBA32, 1,
             [Define if support pixel format RGBA32])
fi

if test "x$pixfmt_abgr32_support" == "xyes"; then
    AC_DEFINE(_MGPLUS_PIXFMT_ABGR32, 1,
             [Define if support pixel format ABGR32])
fi

if test "x$pixfmt_bgra32_support" == "xyes"; then
    AC_DEFINE(_MGPLUS_PIXFMT_BGRA32, 1,
             [Define if support pixel format BGRA32])
fi

if test "x$pixfmt_bgr24_support" == "xyes"; then
    AC_DEFINE(_MGPLUS_PIXFMT_BGR24, 1,
             [Define if support pixel format BGR24])
fi

if test "x$pixfmt_rgb24_support" == "xyes"; then
    AC_DEFINE(_MGPLUS_PIXFMT_RGB24, 1,
             [Define if support pixel format RGB24])
fi

if test "x$pixfmt_rgb555_support" == "xyes"; then
    AC_DEFINE(_MGPLUS_PIXFMT_RGB555, 1,
             [Define if support pixel format RGB555])
fi

if test "x$pixfmt_rgb565_support" == "xyes"; then
    AC_DEFINE(_MGPLUS_PIXFMT_RGB565, 1,
             [Define if support pixel format RGB565])
fi

if test "x$with_libsuffix" = "x"; then
  LIB_SUFFIX=""
else
  LIB_SUFFIX="_$with_libsuffix"
fi

AC_SUBST(LIB_SUFFIX)
AM_CONDITIONAL(MGPLUS_NOSUFFIX, test "x$with_libsuffix" = "x")

DEP_LIBS=
dnl Check for FreeType library
if test "x$build_ft2_support" != "xno"; then
    AC_CHECK_LIB(freetype, FT_Library_Version,
        foo=bar,
        build_ft2_support=no)

    FT2_CFLAGS="`freetype-config --cflags`"
    FT2_LIBS="`freetype-config --libs`"
fi

if test "x$devel_mode" = "xyes"; then
    CPPFLAGS="$CPPFLAGS -D_DEBUG"
    if test "x$detail_debug" = "xyes"; then
        CPPFLAGS="$CPPFLAGS -DDEBUG"
    fi

    if test "$ac_cv_prog_gcc" = "yes"; then
        CPPFLAGS="$CPPFLAGS -Wall -Werror"
    fi
else
    CPPFLAGS="$CPPFLAGS -DNDEBUG"
fi

if test "$ac_cv_prog_gcc" = "yes"; then
    CFLAGS="$CFLAGS -Wstrict-prototypes -pipe"
fi

CPPFLAGS="$CPPFLAGS $FT2_CFLAGS"
DEP_LIBS="$DEP_LIBS $MINIGUI_LIBS $FT2_LIBS -lpthread -lstdc++"

AM_CONDITIONAL(MGPLUS_FONT_FT2, test "x$build_ft2_support" = "xyes")

dnl ========================================================================
dnl check the installation of MiniGUI
dnl ========================================================================
PKG_CHECK_MODULES([MINIGUI], [minigui >= 3.2.0])

APP_LIBS="$APP_LIBS -lmgplus$LIB_SUFFIX $DEP_LIBS"
AC_SUBST(APP_LIBS)
AC_SUBST(DEP_LIBS)

dnl AC_CONFIG_FILES
AC_OUTPUT([
        mgplus.pc
        Makefile
        m4/Makefile
        cmake/Makefile
        include/Makefile
        src/Makefile
        src/agg/Makefile
        src/api/Makefile
])


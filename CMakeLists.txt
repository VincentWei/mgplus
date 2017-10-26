cmake_minimum_required (VERSION 2.6)

project (mGPlus)

# use for MiniGUI mgplus.h to find mgplusconfig.h
add_definitions (-D__CMAKE_PROJECT__)
# define if this is a MGPLUS project
add_definitions(-D__MGPLUS_LIB__)

set (MGPLUS_MAJOR_VERSION 1)
set (MGPLUS_MINOR_VERSION 2)
set (MGPLUS_MICRO_VERSION 5)

set (MGPLUS_NAME "mgplus")
set (MGPLUS_VERSION ${MGPLUS_MAJOR_VERSION}.${MGPLUS_MINOR_VERSION}.${MGPLUS_MICRO_VERSION})
set (MGPLUS_SOVERSION ${MGPLUS_MAJOR_VERSION})

# set CMAKE_MODULE_PATH
if (NOT "x$ENV{CMAKE_INSTALL_PREFIX}" STREQUAL "x")
    set (CMAKE_INSTALL_PREFIX $ENV{CMAKE_INSTALL_PREFIX})
else ()
    if (NOT "x$ENV{CMAKE_PREFIX_PATH}" STREQUAL "x")
        set (CMAKE_INSTALL_PREFIX $ENV{CMAKE_PREFIX_PATH})
    else ()
        if (NOT "x$ENV{PREFIX}" STREQUAL "x")
            set (CMAKE_INSTALL_PREFIX $ENV{PREFIX})
        endif (NOT "x$ENV{PREFIX}" STREQUAL "x")
    endif (NOT "x$ENV{CMAKE_PREFIX_PATH}" STREQUAL "x")
endif (NOT "x$ENV{CMAKE_INSTALL_PREFIX}" STREQUAL "x")
LIST (APPEND CMAKE_MODULE_PATH ${CMAKE_INSTALL_PREFIX}/share/minigui/cmake/Modules)

include (common RESULT_VARIABLE _FOUND_COMMON)
if (${_FOUND_COMMON} STREQUAL NOTFOUND)
    message (FATAL_ERROR "Can't find file `common.cmake'. Please set the environment variable `CMAKE_PREFIX_PATH' properly.")
endif (${_FOUND_COMMON} STREQUAL NOTFOUND)

########################### check header and function ########################
include (CheckIncludeFiles)
check_include_files (fcntl.h HAVE_FCNTL_H)
check_include_files (stdlib.h HAVE_STDLIB_H)
check_include_files (string.h HAVE_STRING_H)
check_include_files (sys/ioctl.h HAVE_SYS_IOCTL_H)
check_include_files (sys/time.h HAVE_SYS_TIME_H)
check_include_files (termio.h HAVE_TERMIO_H)
check_include_files (termios.h HAVE_TERMIOS_H)
check_include_files (unistd.h HAVE_UNISTD_H)

include (CheckFunctionExists)
check_function_exists (bzero HAVE_BZERO)
check_function_exists (dup2 HAVE_DUP2)
check_function_exists (getcwd HAVE_GETCWD)
check_function_exists (memmove HAVE_MEMMOVE)
check_function_exists (memset HAVE_MEMSET)
check_function_exists (putenv HAVE_PUTENV)
check_function_exists (strchr HAVE_STRCHR)
check_function_exists (strdup HAVE_STRDUP)
check_function_exists (strerror HAVE_STRERROR)
check_function_exists (strstr HAVE_STRSTR)

####################### option ############################
mg_declare_option (with_libsuffix STRING "" "configure the suffix of the library name.")
if (NOT with_libsuffix STREQUAL "")
    set (LIB_SUFFIX "_${with_libsuffix}")
endif (NOT with_libsuffix STREQUAL "")
option(disable_shared "Disable shared library" OFF)

mg_declare_option_and_set_value (_MGPLUS_LFRDR_FASHION fashion_lfrdr BOOL ON "build Fashion LFRDR")
mg_declare_option (colorcomposite_support BOOL ON "build support for color composite")
mg_declare_option (gradientfill_support BOOL ON "build support for gradient fill")
mg_declare_option (ft2_support BOOL ON "build support for FreeType2 library")
# mg_declare_option_and_set_value (_MGPLUS_PIXFMT_ARGB32 pixfmt_argb32_support BOOL ON "build support for pixfmt argb32")
mg_declare_option_and_set_value (_MGPLUS_PIXFMT_RGBA32 pixfmt_rgba32_support BOOL ON "build support for pixfmt rgba32")
mg_declare_option_and_set_value (_MGPLUS_PIXFMT_ABGR32 pixfmt_abgr32_support BOOL ON "build support for pixfmt abgr32")
mg_declare_option_and_set_value (_MGPLUS_PIXFMT_BGRA32 pixfmt_bgra32_support BOOL ON "build support for pixfmt bgra32")
mg_declare_option_and_set_value (_MGPLUS_PIXFMT_BGR24  pixfmt_bgr24_support  BOOL ON "build support for pixfmt bgr24")
mg_declare_option_and_set_value (_MGPLUS_PIXFMT_RGB24  pixfmt_rgb24_support  BOOL ON "build support for pixfmt rgb24")
mg_declare_option_and_set_value (_MGPLUS_PIXFMT_RGB555 pixfmt_rgb555_support BOOL ON "build support for pixfmt rgb555")
mg_declare_option_and_set_value (_MGPLUS_PIXFMT_RGB565 pixfmt_rgb565_support BOOL ON "build support for pixfmt rgb565")
if (fashion_lfrdr)
    mg_set_variables_on (gradientfill_support)
    mg_set_variables_on (colorcomposite_support)
endif (fashion_lfrdr)
mg_set_value (_MGPLUS_COLORCOMPOSITE colorcomposite_support)
mg_set_value (_MGPLUS_GRADIENTFILL gradientfill_support)
if (ft2_support)
    include(FindFREETYPE)
    if (FREETYPE_FOUND)
        LIST (APPEND PC_LIBS_LIST "freetype") # for mgplus.pc
        mg_set_value(_MGPLUS_FONT_FT2 ${ft2_support})
        mg_set_variables_on (unicode_support) # ?? unicode_support is no use
        INCLUDE_DIRECTORIES(${FREETYPE_INCLUDE_DIR})
    else()
        set (ft2_support OFF)
    endif (FREETYPE_FOUND)
endif (ft2_support)

####################### conditional ######################
mg_set_conditional (MGPLUS_LFRDR_FASHION    fashion_lfrdr ON)
mg_set_conditional (MGPLUS_FONT_FT2         ft2_support ON)

#################### find minigui ########################

INCLUDE (FindPkgConfig)
PKG_CHECK_MODULES (MINIGUI minigui)

if (NOT MINIGUI_FOUND)
    message(FATAL_ERROR 
       "MiniGUI is not properly installed on the system. You need 
        MiniGUI Ver 3.0.x or later for building this package. 
        Please configure and install MiniGUI Ver 3.0.x first."
        )
endif (NOT MINIGUI_FOUND)

INCLUDE_DIRECTORIES (${MINIGUI_INCLUDE_DIRS})
INCLUDE_DIRECTORIES ("${PROJECT_BINARY_DIR}")
INCLUDE_DIRECTORIES ("${PROJECT_SOURCE_DIR}/include")

LINK_DIRECTORIES (${MINIGUI_LIBRARY_DIRS})

IF (NOT WIN32)
    LIST (APPEND PC_LIBS_LIST "stdc++")
ENDIF (NOT WIN32)
LIST (APPEND PC_LIBS_LIST "pthread")
LIST (APPEND PC_LIBS_LIST ${MINIGUI_LIBRARIES})

#################### .pc and mgplusconfig.h #########################
FOREACH (l ${PC_LIBS_LIST})
    SET (PC_LIBS "${PC_LIBS} -l${l}")
ENDFOREACH (l ${PC_LIBS_LIST})
configure_file (
    "${PROJECT_SOURCE_DIR}/cmake_mgplusconfig.h.in"
    "${PROJECT_BINARY_DIR}/mgplusconfig.h"
    )

set (prefix ${CMAKE_INSTALL_PREFIX})
set (exec_prefix "\${prefix}")
set (libdir "\${exec_prefix}/lib")
set (includedir "\${prefix}/include")
set (mGPlus_VERSION ${MGPLUS_VERSION})
configure_file(
    "${PROJECT_SOURCE_DIR}/mgplus.pc.in"
    "${PROJECT_BINARY_DIR}/${MGPLUS_NAME}.pc")

##################### add sub directories #######################
add_subdirectory (src)
# add_subdirectory (test)

#################### files install ##########################
install (FILES "${PROJECT_BINARY_DIR}/mgplusconfig.h"
    DESTINATION include/${MGPLUS_NAME}
    )

install (DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
    DESTINATION include/${MGPLUS_NAME}
    FILES_MATCHING PATTERN "*.h"
                   PATTERN ".svn" EXCLUDE
    )

# Cmake Modules
install (DIRECTORY "${PROJECT_SOURCE_DIR}/cmake/"
    DESTINATION share/minigui/cmake/Modules
    FILES_MATCHING PATTERN "*.cmake"
    PATTERN "*private*" EXCLUDE
    PATTERN ".svn" EXCLUDE
    )

install (FILES "${PROJECT_BINARY_DIR}/${MGPLUS_NAME}.pc"
    DESTINATION lib/pkgconfig)

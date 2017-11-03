SUBDIRS=

AM_CPPFLAGS=-D__MGPLUS_LIB__ -I.. -I../agg -I../../include -I../agg/font_freetype

noinst_LTLIBRARIES=libapi.la

SRC_FILES = path.cpp graphics.cpp pen.cpp brush.cpp lf_fashion.cpp  \
			opt_argb32.cpp opt_rgba32.cpp opt_bgra32.cpp opt_abgr32.cpp \
			opt_rgb24.cpp opt_bgr24.cpp opt_rgb555.cpp opt_rgb565.cpp

HDR_FILES = path.h graphics.h pen.h brush.h self_stroke.h

libapi_la_SOURCES = $(SRC_FILES) $(HDR_FILES)
LIB_NAME=libapi

EXTRA_DIST = $(SRC_FILES) opt_basic.cpp

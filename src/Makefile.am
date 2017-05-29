TOP_DIR=..
INCLUDES = -I$(TOP_DIR)/include

COMMON_SUBDIRS = agg api
DIST_SUBDIRS = $(COMMON_SUBDIRS) 
SUBDIRS = $(COMMON_SUBDIRS)

COMMON_LIBADD = api/libapi.la \
				agg/libagg.la
MPSOURCES = 
MPLDFLAGS = -release $(LT_RELEASE) -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

CLEANFILES=*.a
EXTRA_DIST=

lib_LTLIBRARIES = libmgplus@LIB_SUFFIX@.la

if MGPLUS_FONT_FT2
libmgplus@LIB_SUFFIX@_la_LIBADD = $(COMMON_LIBADD) -lstdc++
else
libmgplus@LIB_SUFFIX@_la_LIBADD = $(COMMON_LIBADD) -lstdc++
endif
libmgplus@LIB_SUFFIX@_la_SOURCES = $(MPSOURCES)
libmgplus@LIB_SUFFIX@_la_LDFLAGS = $(MPLDFLAGS) 


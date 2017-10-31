INCLUDES = -I../animate -I../animate/common_animates

COMMON_PROGS = mgplustest gradientdemo mgglyph_outline coverflow \
	       mgcompositedemo blinds centersplit cubicrotate lion_test mgplus_efficiency

noinst_PROGRAMS = $(COMMON_PROGS)

mgplus_efficiency_SOURCES = mgplus_efficiency.c
mgplus_efficiency_LDADD = -lmgplus @APP_LIBS@

lion_test_SOURCES = lion_test.c
lion_test_LDADD = -lmgplus @APP_LIBS@

gradientdemo_SOURCES = gradientdemo.c
gradientdemo_LDADD = -lmgplus @APP_LIBS@

coverflow_SOURCES = coverflow.c
coverflow_LDADD = -lmgplus @APP_LIBS@ -lstdc++

mgplustest_SOURCES = mgplustest.c
mgplustest_LDADD = -lmgplus @APP_LIBS@

mgglyph_outline_SOURCES = mgglyph_outline.c
mgglyph_outline_LDADD = -lmgplus @APP_LIBS@

mgcompositedemo_SOURCES = mgcompositedemo.c
mgcompositedemo_LDADD = -lmgplus @APP_LIBS@

blinds_SOURCES = blinds.c
blinds_LDADD = -lmgplus @APP_LIBS@

LDADD = ../animate/libanimate.la -lmgplus $(INTLLIBS) @APP_LIBS@

cubicrotate_SOURCES = cubicrotate.c

centersplit_SOURCES = centersplit.c

SUBDIRS = res


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

LDADD =../animate/common_animates/.libs/libcommon_animates.a  \
	   ../animate/.libs/libanimate.a -lmgplus $(INTLLIBS) @APP_LIBS@

cubicrotate_SOURCES = cubicrotate.c
#cubicrotate_LDADD = -lmgplus -lanimate -lcommon_animates @APP_LIBS@
#cubicrotate_LDADD = -lmgplus -L../animate -lanimate -L../animate/common_animates -lcommon_animates @APP_LIBS@
#cubicrotate_LDADD =  -lmgplus @APP_LIBS@

centersplit_SOURCES = centersplit.c
#centersplit_LDADD = -lmgplus -L../animate -lanimate -L../animate/common_animates -lcommon_animates @APP_LIBS@

SUBDIRS = res


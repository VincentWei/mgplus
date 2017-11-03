AM_CPPFLAGS = -I../animate -I../animate/common_animates

COMMON_PROGS = tcldemo 

noinst_PROGRAMS = $(COMMON_PROGS)

tcldemo_SOURCES = tcldemo.c newlistview.c newlistview.h tclanimate.c tclanimate.h centersplit.c font_table.c
tcldemo_LDADD = ../animate/libanimate.la  @APP_LIBS@

SUBDIRS = res
EXTRA_DIST = 


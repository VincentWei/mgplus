COMMON_PROGS= glyphoutline

noinst_PROGRAMS=$(COMMON_PROGS)

glyphoutline_SOURCES= glyphoutlinetest.c 

if MGRM_PROCESSES
glyphoutline_LDADD = -lminigui_procs -lmgutils -lmgplus @APP_LIBS@

else

glyphoutline_LDADD = -lminigui_ths -lmgutils -lmgplus @APP_LIBS@
endif
 


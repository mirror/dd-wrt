AM_CFLAGS=-Wall

sbin_PROGRAMS = sercd

sercd_SOURCES = sercd.c sercd.h win.c win.h unix.c unix.h
sercd_LDADD = 

if OS_IS_WIN32
sercd_LDADD += -lws2_32
endif

man_MANS = sercd.8

EXTRA_DIST =  $(man_MANS) sercd.xinetd ChangeLog.old 

INCLUDES = \
	-I$(top_srcdir)/Core \
	-I$(top_srcdir)/Shared

lib_LTLIBRARIES = libGobiImageMgmt.la

libGobiImageMgmt_la_CPPFLAGS = \
	-D WDS_SUPPORT \
	-D DMS_SUPPORT \
	-D NAS_SUPPORT \
	-D PDS_SUPPORT \
	-D CAT_SUPPORT \
	-D RMS_SUPPORT \
	-D OMA_SUPPORT \
	-D UIM_SUPPORT \
	-D WMS_SUPPORT \
	-D IMG2K_SUPPORT \
	-D IMG_SUPPORT \
	-D VOICE_SUPPORT

libGobiImageMgmt_la_SOURCES = \
	GobiImageMgmtAPI.h \
	GobiImageMgmt.h \
	GobiImageMgmt.cpp \
	GobiImageMgmtExports.cpp

libGobiImageMgmt_la_LIBADD = \
	$(top_builddir)/Database/QMI/libQMIDB.la \
	$(top_builddir)/Shared/libShared.la \
	$(top_builddir)/Core/libCore.la \
	-lpthread


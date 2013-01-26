INCLUDES = \
	-I$(top_srcdir)/Core \
	-I$(top_srcdir)/Shared

bin_PROGRAMS = GobiQDLService

GobiQDLService_CPPFLAGS = \
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

GobiQDLService_SOURCES = Main.cpp

GobiQDLService_CXXFLAGS = -pthread

GobiQDLService_LDADD = \
	$(top_builddir)/Database/QMI/libQMIDB.la \
	$(top_builddir)/Shared/libShared.la \
	$(top_builddir)/Core/libCore.la \
	-lrt \
	-lpthread


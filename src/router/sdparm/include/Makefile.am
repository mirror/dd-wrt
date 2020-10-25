
scsiincludedir      = $(includedir)/scsi

scsiinclude_HEADERS = \
	sg_lib.h \
	sg_lib_data.h \
	sg_cmds.h \
	sg_cmds_basic.h \
	sg_cmds_extra.h \
	sg_cmds_mmc.h \
	sg_pr2serr.h \
	sg_unaligned.h \
	sg_pt.h \
	sg_pt_nvme.h

if OS_LINUX
scsiinclude_HEADERS += \
	sg_linux_inc.h \
	sg_io_linux.h \
	sg_pt_linux.h
	
noinst_HEADERS = \
	sg_pt_win32.h
endif

if OS_WIN32_MINGW
scsiinclude_HEADERS += sg_pt_win32.h
	
noinst_HEADERS = \
	sg_linux_inc.h \
	sg_io_linux.h
endif

if OS_WIN32_CYGWIN
scsiinclude_HEADERS += sg_pt_win32.h
	
noinst_HEADERS = \
	sg_linux_inc.h \
	sg_io_linux.h
endif

if OS_FREEBSD
noinst_HEADERS = \
	sg_linux_inc.h \
	sg_io_linux.h \
	sg_pt_win32.h
endif

if OS_SOLARIS
noinst_HEADERS = \
	sg_linux_inc.h \
	sg_io_linux.h \
	sg_pt_win32.h
endif

if OS_OSF
noinst_HEADERS = \
	sg_linux_inc.h \
	sg_io_linux.h \
	sg_pt_win32.h
endif


bin_PROGRAMS = sdparm

# for C++/clang testing
## CC = gcc-8
## CC = g++
## CC = clang
## CC = clang++

if DEBUG
DBG_CFLAGS = -Wextra -Wmisleading-indentation -Wduplicated-cond -Wlogical-op -Wnull-dereference -Wshadow -Wjump-misses-init
DBG_CPPFLAGS = -DDEBUG
else
DBG_CFLAGS =
DBG_CPPFLAGS =
endif

# -std=<s> can be c99, c11, gnu11, etc. Default is gnu11
AM_CPPFLAGS = -iquote ${top_srcdir}/include $(DBG_CPPFLAGS)
AM_CFLAGS = -Wall -W $(DBG_CFLAGS)
# AM_CFLAGS = -Wall -W -Wextra -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wshadow -Wjump-misses-init
# AM_CFLAGS = -Wall -W -Werror=misleading-indentation
# AM_CFLAGS = -Wall -W -pedantic -std=c11
# AM_CFLAGS = -Wall -W -pedantic -std=c++14
# AM_CFLAGS = -Wall -W -pedantic -std=c++20

sdparm_SOURCES =	sdparm.c	\
			sdparm.h	\
			sdparm_data.c	\
			sdparm_data_vendor.c	\
			sdparm_access.c	\
			sdparm_vpd.c	\
			sdparm_cmd.c

if OS_WIN32_MINGW
sdparm_SOURCES +=	sdparm_wscan.c
endif

if OS_WIN32_CYGWIN
sdparm_SOURCES +=	sdparm_wscan.c
endif


sglib_SOURCES =		../lib/sg_lib.c \
			../include/sg_lib.h	\
			../lib/sg_lib_data.c	\
			../include/sg_lib_data.h	\
			../lib/sg_cmds_basic.c	\
			../lib/sg_cmds_basic2.c	\
			../include/sg_cmds_basic.h	\
			../lib/sg_cmds_mmc.c	\
			../include/sg_cmds_mmc.h	\
			../include/sg_pt.h	\
			../include/sg_pt_nvme.h	\
			../lib/sg_pt_common.c

if HAVE_SGUTILS
INCLUDES = -I/scsi
sdparm_LDADD = @GETOPT_O_FILES@ @SGUTILS_LIBS@
sdparm_DEPENDENCIES = @GETOPT_O_FILES@
else
INCLUDES = -I$(top_srcdir)/include
sdparm_SOURCES += $(sglib_SOURCES)
sdparm_LDADD = @os_deps@ @GETOPT_O_FILES@ @SGUTILS_LIBS@
sdparm_DEPENDENCIES = @os_deps@ @GETOPT_O_FILES@
endif

EXTRA_sdparm_SOURCES =	../lib/sg_pt_linux.c	\
			../lib/sg_pt_linux_nvme.c	\
			../include/sg_pt_linux.h	\
			../include/sg_linux_inc.h	\
			../lib/sg_pt_freebsd.c	\
			../lib/sg_pt_osf1.c	\
			../lib/sg_pt_solaris.c	\
			../lib/sg_pt_win32.c	\
			../include/sg_pt_win32.h	\
			getopt_long.c	\
			port_getopt.h

distclean-local:
	rm -rf .deps

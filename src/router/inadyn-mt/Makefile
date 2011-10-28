INADYN_BASE = .
SRCDIR = $(INADYN_BASE)/src
OUTDIR=bin

######
#make parameters
#####

#configure preprocessor options via config.h,
#this makefile, or make command line -- define
#this if either of the two latter

EXCLUDE_CONFIG_H=1

ifdef EXCLUDE_CONFIG_H

	CFLAGS += -DEXCLUDE_CONFIG_H
endif

#DEBUG=1

#dynamically load shared libs/function -- disable if problems
#NO_DYN_LOAD=1
ifdef NO_DYN_LOAD

	CFLAGS += -DNO_DYN_LOAD
endif

#FreeBSD, for instance, does not use -ldl for
#dlopen, dlsym, etc., so define accordingly
#NO_LIBDL=1

#audible alerts on detect offline
#USE_SOUND=1

#threads default on with USE_SOUND
#DISABLE_THREADS=1

#threads default off without sound
#USE_THREADS=1

#simple arch config. No smart detection.
#possible values: linux, mac, solaris, freebsd, yourown
ifndef TARGET_ARCH
	TARGET_ARCH = linux
endif

OUTFILE=$(OUTDIR)/$(TARGET_ARCH)/inadyn

# Make command to use for dependencies
MAKECMD=gmake

ifneq ($(TARGET_ARCH),mac)
	ARCH_SPECIFIC_CFLAGS += -pedantic 
endif

ifeq ($(TARGET_ARCH),solaris)
	ARCH_SPECIFIC_LIBS += -lsocket -lnsl
	ARCH_SPECIFIC_CFLAGS += 
endif

ifeq ($(TARGET_ARCH),linux)
	ARCH_SPECIFIC_LIBS += 
	ARCH_SPECIFIC_CFLAGS += $(COPTS)
endif

ifdef DEBUG
	CFLAGS += -g -DDEBUG
endif

CFLAGS += -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/nvram -L$(TOP)/libutils -I$(TOP)/../include.bcm

ifndef USE_SOUND
	ifdef USE_THREADS

		ARCH_SPECIFIC_OBJS = $(OUTDIR)/threads_wrapper.o

		ARCH_SPECIFIC_LIBS += -lpthread
		CFLAGS += -DUSE_THREADS
	endif
else
	ARCH_SPECIFIC_OBJS = $(OUTDIR)/wave_util.o $(OUTDIR)/waveout.o

	ARCH_SPECIFIC_LIBS += -lm

	ifdef NO_DYN_LOAD

		ARCH_SPECIFIC_LIBS += -lao
		CFLAGS += -DNO_DYN_LOAD
	else
		ifndef NO_LIBDL
			ARCH_SPECIFIC_LIBS += -ldl
		endif
	endif

	CFLAGS += -DUSE_SNDFILE
	
	ifndef DISABLE_THREADS
		ARCH_SPECIFIC_OBJS += $(OUTDIR)/threads_wrapper.o

		ARCH_SPECIFIC_LIBS += -lpthread
		CFLAGS += -DUSE_THREADS
	endif

endif

COMMON_OBJ=$(OUTDIR)/base64utils.o $(OUTDIR)/dyndns.o \
	$(OUTDIR)/errorcode.o $(OUTDIR)/get_cmd.o $(OUTDIR)/http_client.o \
	$(OUTDIR)/ip.o $(OUTDIR)/main.o $(OUTDIR)/os.o $(OUTDIR)/os_psos.o \
	$(OUTDIR)/os_unix.o $(OUTDIR)/os_windows.o $(OUTDIR)/tcp.o \
	$(OUTDIR)/inadyn_cmd.o $(OUTDIR)/unicode_util.o $(OUTDIR)/numbers.o \
	$(OUTDIR)/dblhash.o
#	$(OUTDIR)/lang.o 
	$(OUTDIR)/path.o $(OUTDIR)/safe_mem.o \
	$(OUTDIR)/md5.o	$(ARCH_SPECIFIC_OBJS)

OBJ=$(COMMON_OBJ) $(CFG_OBJ)

COMPILE=$(CC)  -Wall -pedantic -c $(ARCH_SPECIFIC_CFLAGS) $(CFLAGS) -o "$(OUTDIR)/$(*F).o" $(CFG_INC) "$<" $(CMP_DEF)
LINK=$(CC) $(CFLAGS) -o "$(OUTFILE)" $(OBJ) $(CFG_LIB) $(ARCH_SPECIFIC_LIBS) -lnvram -lutils

# Pattern rules
$(OUTDIR)/%.o : $(SRCDIR)/%.c
	$(COMPILE)

# Build rules
all: $(OUTFILE)

$(OUTFILE): $(OUTDIR)/$(TARGET_ARCH)  $(OBJ)
	$(LINK)

$(OUTDIR)/$(TARGET_ARCH):
	-mkdir -p "$@"

# Rebuild this project
rebuild: cleanall all

# Clean this project
clean:
	rm -f $(OUTFILE)
	rm -f $(OUTDIR)/*.o

# Clean this project and all dependencies
cleanall: clean


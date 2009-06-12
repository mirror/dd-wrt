##############################################################################
# Microwindows template Makefile
# Copyright (c) 2000, 2002 Martin Jolicoeur, Greg Haerr
##############################################################################

include $(CONFIG)

VPATH := $(TOP)/ecos

######################## Additional Flags section ############################

# Directories list for header files
INCLUDEDIRS +=
# Defines for preprocessor
DEFINES +=

# Compilation flags for C files OTHER than include directories
CFLAGS +=
# Preprocessor flags OTHER than defines
CPPFLAGS +=
# Linking flags
LDFLAGS += -Wl,--gc-sections -Wl,-static -g -O2 -nostdlib -L$(ECOS_PREFIX)/lib -Ttarget.ld

############################# targets section ################################

# If you want to create a library with the objects files, define the name here
LIBNAME =
LIBNAMESO =

# List of objects to compile
OBJS = ecos_app.o ecos_init.o
NANO_OBJS = 
#DEFINES += -DUSE_ROMDISK
OBJS += nanox_thread.o
#NANO_OBJS += $(BUILD)/bin/nano-X.o $(BUILD)/bin/nwidgets.o
NANO_OBJS += $(BUILD)/bin/nano-X.o
OBJS += nanowm_thread.o
NANO_OBJS += $(BUILD)/bin/nanowm.o
OBJS += nxkbd_thread.o
NANO_OBJS += $(BUILD)/bin/nxkbd.o
ifeq ($(DEMO), Y)
DEFINES += -DUSE_NXSCRIBBLE
OBJS += nxscribble_thread.o
NANO_OBJS += $(BUILD)/bin/nxscribble.o 
DEFINES += -DUSE_LANDMINE
OBJS += landmine_thread.o
NANO_OBJS += $(BUILD)/bin/landmine.o 
DEFINES += -DUSE_NTETRIS
OBJS += ntetris_thread.o
NANO_OBJS += $(BUILD)/bin/ntetris.o 
DEFINES += -DUSE_WORLD
OBJS += world_thread.o
NANO_OBJS += $(BUILD)/bin/world.o 
endif
ifeq ($(AGDEMO), Y)
DEFINES += -DUSE_IMG_DEMO
OBJS += img_demo.o
endif

# demos should be built after the libs !
all: ecos_app

######################### Makefile.rules section #############################

include $(TOP)/Makefile.rules

######################## Tools targets section ###############################

ecos_app: $(OBJS) $(NANO_OBJS)

ecos_app.o: Makefile



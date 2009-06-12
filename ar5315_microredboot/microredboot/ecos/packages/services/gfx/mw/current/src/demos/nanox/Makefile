##############################################################################
# Microwindows template Makefile
# Copyright (c) 2000, 2002 Martin Jolicoeur, Greg Haerr
##############################################################################

ifndef TOP
TOP = ../..
CONFIG = $(TOP)/config
endif

include $(CONFIG)

VPATH := $(TOP)/demos/nanox

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
LDFLAGS += -lm

############################# targets section ################################

ifeq ($(NANOX), Y)
ifeq ($(NANOXDEMO), Y)

# If you want to create a library with the objects files, define the name here
LIBNAME =

# List of objects to compile
ifeq ($(ARCH), ECOS)
OBJS = landmine.o
TARGETS = $(BUILD)/bin/landmine.o
OBJS += ntetris.o
TARGETS += $(BUILD)/bin/ntetris.o
OBJS += world.o
TARGETS += $(BUILD)/bin/world.o
else
OBJS =	demo.o \
	move.o \
	landmine.o \
	world.o \
	nxclock.o \
	nxterm.o \
	nxview.o \
	nxlsclients.o \
	nxev.o \
	npanel.o \
	nterm.o \
	demo2.o \
	demo5.o \
	demo6.o \
	t1demo.o \
	ftdemo.o \
	logfont.o \
	slider.o \
	ntetris.o \
	launcher.o \
	nsaver.o \
	getselection.o \
	setselection.o

TARGETS = $(TOP)/bin/demo $(TOP)/bin/move $(TOP)/bin/landmine \
	$(TOP)/bin/world $(TOP)/bin/nxclock $(TOP)/bin/nxterm \
	$(TOP)/bin/nxview $(TOP)/bin/nxlsclients $(TOP)/bin/nxev \
	$(TOP)/bin/npanel $(TOP)/bin/nterm \
	$(TOP)/bin/demo2 $(TOP)/bin/demo5 $(TOP)/bin/demo6 \
	$(TOP)/bin/t1demo $(TOP)/bin/ftdemo $(TOP)/bin/logfont \
	$(TOP)/bin/slider $(TOP)/bin/ntetris $(TOP)/bin/launcher \
	$(TOP)/bin/nsaver $(TOP)/bin/getselection $(TOP)/bin/setselection
endif

all: default $(TARGETS)	support

support: world.map nanogui.ppm ntetris.ppm launcher.cnf tux.gif
	$(CP) $^ $(BUILD)/bin
endif
endif

######################### Makefile.rules section #############################

include $(TOP)/Makefile.rules

ifeq ($(SHAREDLIBS), Y)
LD_NANOXCLIENTLIBS = $(CCNANOXCLIENTLIBS)
else
LD_NANOXCLIENTLIBS = $(NANOXCLIENTLIBS)
endif

######################## Tools targets section ###############################

$(BUILD)/bin/demo: demo.o $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/move: move.o $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/landmine: landmine.o $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/world: world.o $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/nxclock: nxclock.o $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/nxterm: nxterm.o $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/nxview: nxview.o $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/nxlsclients: nxlsclients.o $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/nxev: nxev.o $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/npanel: npanel.o $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/nterm: nterm.o $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/demo2: demo2.o $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/demo5: demo5.o $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/demo6: demo6.o $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/t1demo: t1demo.o $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/ftdemo: ftdemo.o $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/logfont: logfont.o $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/slider: slider.o $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/ntetris: ntetris.o $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/launcher: launcher.o $($NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/nsaver: nsaver.o $($NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/getselection: getselection.o $($NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

$(BUILD)/bin/setselection: setselection.o $($NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LD_NANOXCLIENTLIBS)

ifeq ($(ARCH), ECOS)
# Special build rules for linked in applications
$(BUILD)/bin/landmine.o: landmine.o
#	$(CC) $(CFLAGS) $(LDFLAGS) $< -Wl,-r -Wl,--retain-symbols-file -Wl,landmine.syms -Wl,--defsym -Wl,landmine_main=main -o XX.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -Wl,-r -Wl,--defsym -Wl,landmine_main=main -o XX.o
	$(NM) XX.o | grep -v _main | grep ' T' | awk -f $(TOP)/ecos/retain.awk | xargs $(OBJCOPY) XX.o $@
	rm -f XX.o

$(BUILD)/bin/ntetris.o: ntetris.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -Wl,-r -Wl,--defsym -Wl,ntetris_main=main -o XX.o
	$(NM) XX.o | grep -v _main | grep ' T' | awk -f $(TOP)/ecos/retain.awk | xargs $(OBJCOPY) XX.o $@
	rm -f XX.o

$(BUILD)/bin/world.o: world.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -Wl,-r -Wl,--defsym -Wl,world_main=main -o XX.o
	$(NM) XX.o | grep -v _main | grep ' T' | awk -f $(TOP)/ecos/retain.awk | xargs $(OBJCOPY) XX.o $@
	rm -f XX.o
endif


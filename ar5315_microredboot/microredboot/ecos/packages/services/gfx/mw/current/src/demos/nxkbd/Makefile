##############################################################################
# Microwindows template Makefile
# Copyright (c) 2000, 2002 Martin Jolicoeur, Greg Haerr
##############################################################################

include $(CONFIG)

VPATH := $(TOP)/demos/nxkbd

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
LDFLAGS +=

############################# targets section ################################

ifeq ($(NANOWM), Y)

# If you want to create a library with the objects files, define the name here
LIBNAME =

# List of objects to compile
OBJS = nxkbd.o srvconn.o keynorm.o keyctrl.o keyshft.o keynum.o

ifeq ($(ARCH), ECOS)
TARGET = $(BUILD)/bin/nxkbd.o
else
TARGET = $(BUILD)/bin/nxkbd
endif

all: default $(TARGET)

endif

######################### Makefile.rules section #############################

include $(TOP)/Makefile.rules

######################## Tools targets section ###############################

ifeq ($(SHAREDLIBS), Y)
$(BUILD)/bin/nxkbd: $(OBJS) $(NANOXCLIENTLIBS) $(BUILd)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@ $(CCNANOXCLIENTLIBS)
else
ifeq ($(ARCH), ECOS)
$(BUILD)/bin/nxkbd.o: $(OBJS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -Wl,-r -Wl,--defsym -Wl,nxkbd_main=main -o XX.o
	$(NM) XX.o | grep -v _main | grep ' T' | awk -f $(TOP)/ecos/retain.awk | xargs $(OBJCOPY) XX.o $@
	rm -f XX.o
else
$(BUILD)/bin/nxkbd: $(OBJS) $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@ $(NANOXCLIENTLIBS)
endif
endif

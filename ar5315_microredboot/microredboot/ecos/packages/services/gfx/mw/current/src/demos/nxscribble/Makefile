##############################################################################
# Microwindows template Makefile
# Copyright (c) 2000, 2002 Martin Jolicoeur, Greg Haerr
##############################################################################

ifndef TOP
TOP = ../..
CONFIG = $(TOP)/config
endif

include $(CONFIG)

VPATH := $(TOP)/demos/nxscribble

######################## Additional Flags section ############################

# Directories list for header files
INCLUDEDIRS += -I$(VPATH)
# Defines for preprocessor
DEFINES +=

# Compilation flags for C files OTHER than include directories
CFLAGS +=
# Preprocessor flags OTHER than defines
CPPFLAGS +=
# Linking flags
LDFLAGS +=

############################# targets section ################################

ifeq ($(NANOX), Y)

# If you want to create a library with the objects files, define the name here
LIBNAME =

# List of objects to compile
OBJS = nxscribble.o scribwidget.o\
	hre_api.o li_recognizer.o bitvector.o matrix.o sc.o util.o

ifeq ($(ARCH), ECOS)
DEFINES += -DREC_DEFAULT_USER_DIR=\"/bin\"
TARGET = $(BUILD)/bin/nxscribble.o
else
TARGET = $(BUILD)/bin/nxscribble
endif

all: default $(TARGET) support

support: letters.cl digits.cl punc.cl
	$(CP) $^ $(BUILD)/bin

endif

######################### Makefile.rules section #############################

include $(TOP)/Makefile.rules

######################## Tools targets section ###############################

ifeq ($(SHAREDLIBS), Y)
$(BUILD)/bin/nxscribble: $(OBJS) $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@ $(CCNANOXCLIENTLIBS) -lm
else
ifeq ($(ARCH), ECOS)
$(BUILD)/bin/nxscribble.o: $(OBJS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -Wl,-r -Wl,--defsym -Wl,nxscribble_main=main -o XX.o
	$(NM) XX.o | grep -v _main | grep ' T' | awk -f $(TOP)/ecos/retain.awk | xargs $(OBJCOPY) XX.o $@
	rm -f XX.o
else
$(BUILD)/bin/nxscribble: $(OBJS) $(NANOXCLIENTLIBS) $(BUILD)/config
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@ $(NANOXCLIENTLIBS) -lm
endif
endif

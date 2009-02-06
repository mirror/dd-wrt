################################################################################
# File: Makefile                                                               #
################################################################################
# Makefile for the socketcc library.  This will first make all the object      #
# files before linking to create the final library file.  There is also the    #
# option of installing and removing the library to /usr/lib and /usr/include.  #
# Object files are made in the obj sub-directory.                              #
################################################################################
# Notes:                                                                       #
#    Version 1.00 - Original Version of library.                               #
#                                                                              #
#    Version 1.11 - Support for Solaris, many thanks to Andreas Almroth for    #
#                   his effort.                                                #
#                 - Combined dual makefiles into a single makefile.            #
#                                                                              #
#    Version 1.30 - Support for MacOS X.1.?, many thanks to Desmond Schmidt.   #
#                 - Support for MacOS X.2, many thanks to Daniel Grimm.        #
#                 - Changed installation directory to /usr/local/(lib|include) #
#                                                                              #
#    Version 1.38 - Support for FreeBSD, many thanks to Clayborne Taylor.      #
################################################################################

################################################################################
# Library compile information.                                                 #
################################################################################
#     SRC_DIR      - Directory containing source and header files.             #
#     SRC_FILES    - List of source files to be compiled.                      #
#     OBJ_DIR      - Directory containing compiled and linked binaries.        #
#     DEPEND_FILE  - Contains any dependencies for conditional compilation.    #
#     DEP_FILTER   - sed script to run on output of dependencies generation.   #
#     MAJ_VER      - Major Version Number of library.                          #
#     MIN_VER      - Minor Version Number of library.                          #
#     LIB_DIR      - Installation directory for compiled library.              #
#     HEAD_DIR     - Installation directory for library header files.          #
#     LIB_HEADERS  - Header files from SRC_DIR to be copied to HEAD_DIR.       #
#     PLATFORM     - Target platform for library, valid values are:            #
#                    linux:         Linux                                      #
#                    freebsd:       FreeBSD (4.7 - use gmake)                  #
#                    solaris:       Solaris (Sun)                              #
#                    macosx_1:      MacOS X.1.?                                #
#                    macosx_jaguar: MacOS X.2 Jaguar                           #
#     PRE_CFLAGS   - Specifies platform macro for conditional compilation.     #
################################################################################
SRC_DIR     = src
SRC_FILES   = socketexception.cpp ipaddress.cpp socketbase.cpp tcpsockets.cpp udpsockets.cpp
OBJ_DIR     = obj
DEPEND_FILE = $(OBJ_DIR)/.depend
DEP_FILTER  = sed 's/[a-zA-Z_0-9]*\.o/$(OBJ_DIR)\/&/g'
MAJ_VER     = 1
MIN_VER     = 38
LIB_DIR     = /usr/lib
HEAD_DIR    = /usr/include
LIB_HEADERS = socketcc.h
PLATFORM    = linux
PRE_CFLAGS  = -DPLATFORM_$(PLATFORM)
OPENWRT     = /opt/3.3.4

################################################################################
# Multiple Platform Support.                                                   #
#                                                                              #
# Defines variables to enable compilation/linking/installation on a specific   #
# platform.                                                                    #
# CC         - Name of compiler to use.                                        #
# CFLAGS     - Compile flags, may be dependant on compiler rather than platform#
# LIB_NAME   - Name of library as used for linking by other applications.      #
# LIB_SONAME - Name of library as used for dynamic linking (soname).           #
# LIB_TARGET - Final target filename of library.                               #
# LD         - Name of linker to use.                                          #
# LDFLAGS    - Link flags, may be dependant on linker rather than platform.    #
# CMD_DYLINK - Command to create linking file for dynamic linking if required. #
# CMD_STLINK - Command to create linking file for static linking if required.  #
################################################################################
# Linux.                                                                       #
################################################################################
ifeq ($(PLATFORM), linux)
ifneq ($(strip $(OPENWRT)),)
PRE_CFLAGS  += -DOPENWRT
CFLAGS      += -I../pthreadcc/src $(COPTS) -DNEED_PRINTF -fPIC
CXX         = $(ARCH)-linux-g++
CC          = $(ARCH)-linux-g++
LD	    = $(ARCH)-linux-g++
LDFLAGS    = -L../pthreadcc -lpthreadcc -shared
else
CC         = g++
CFLAGS     = -I$/usr/include -I../pthreadcc/src -Wall -fPIC
LD		   = g++
LDFLAGS    = -lpthreadcc -shared -Wl,-soname,$(LIB_SONAME)
endif
LIB_NAME   = libsocketcc.so
LIB_SONAME = $(LIB_NAME).$(MAJ_VER)
LIB_TARGET = $(LIB_SONAME).$(MIN_VER)
CMD_DYLINK = /sbin/ldconfig $(LIB_DIR)
CMD_STLINK = ln -s $(LIB_DIR)/$(LIB_TARGET) $(LIB_DIR)/$(LIB_NAME)
endif

################################################################################
# FreeBSD.                                                                     #
################################################################################
ifeq ($(PLATFORM), freebsd)
CC         = g++
CFLAGS     = $(SYSTEM_CFLAGS) -I../pthreadcc/src
LIB_NAME   = libsocketcc.so
LIB_SONAME = $(LIB_NAME).$(MAJ_VER)
LIB_TARGET = $(LIB_SONAME).$(MIN_VER)
LD         = g++
LDFLAGS    = -shared -Wl,-soname,$(LIB_SONAME)
CMD_DYLINK =
CMD_STLINK = ln -s $(LIB_DIR)/$(LIB_TARGET) $(LIB_DIR)/$(LIB_NAME)
endif

################################################################################
# Solaris.                                                                     #
################################################################################
ifeq ($(PLATFORM), solaris)
CC         = cc
CFLAGS     = -I$/usr/include -XO2 -KPIC
LIB_NAME   = libsocketcc.so
LIB_SONAME = $(LIB_NAME).$(MAJ_VER)
LIB_TARGET = $(LIB_SONAME).$(MIN_VER)
LD         = ld
LDFLAGS    = -G -Bdynamic -z text
CMD_DYLINK =
CMD_STLINK = ln -s $(LIB_DIR)/$(LIB_TARGET) $(LIB_DIR)/$(LIB_NAME)
endif

################################################################################
# MacOS X.1.?.                                                                 #
################################################################################
ifeq ($(PLATFORM), macosx_1)
CC         = c++
CFLAGS     = -I$/usr/include -Wall -Wstrict-prototypes -O2 -fPIC
LIB_NAME   = libsocketcc.dylib
LIB_SONAME = libsocketcc.$(MAJ_VER).dylib
LIB_TARGET = libsocketcc.$(MAJ_VER).$(MIN_VER).dylib
LD         = c++
LDFLAGS    = -lpthreadcc -dynamiclib -install_name $(LIB_DIR)/$(LIB_TARGET) -compatibility_version $(MAJ_VER).$(MIN_VER) -current_version $(MAJ_VER).$(MIN_VER)
CMD_DYLINK = ln -s $(LIB_DIR)/$(LIB_TARGET) $(LIB_DIR)/$(LIB_SONAME)
CMD_STLINK = ln -s $(LIB_DIR)/$(LIB_TARGET) $(LIB_DIR)/$(LIB_NAME)
endif

################################################################################
# MacOS X Jaguar.                                                              #
################################################################################
ifeq ($(PLATFORM), macosx_jaguar)
CC         = g++
CFLAGS     = -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386 -arch ppc -I$/usr/include -I../pthreadcc/src -Wall -Wstrict-prototypes -O2 -fPIC
LIB_NAME   = libsocketcc.dylib
LIB_SONAME = libsocketcc.$(MAJ_VER).dylib
LIB_TARGET = libsocketcc.$(MAJ_VER).$(MIN_VER).dylib
LD         = g++
LDFLAGS    = -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386 -arch ppc -L../pthreadcc -lpthreadcc.1.11 -dynamiclib -install_name $(LIB_DIR)/$(LIB_TARGET) -compatibility_version $(MAJ_VER).$(MIN_VER) -current_version $(MAJ_VER).$(MIN_VER)
CMD_DYLINK = ln -s $(LIB_DIR)/$(LIB_TARGET) $(LIB_DIR)/$(LIB_SONAME)
CMD_STLINK = ln -s $(LIB_DIR)/$(LIB_TARGET) $(LIB_DIR)/$(LIB_NAME)
endif

################################################################################
# Makeup of introduction message.											   #
################################################################################
INFO_LINE =  "=========================================================\n"
INFO_TITLE = "  socket : C++ Class based pthread object libraries.\n"
INFO_VER = " Version      : $(MAJ_VER).$(MIN_VER)\n"
INFO_DEV = " Developed By : CTIE - Monash University\n"
INFO_PLAT = " Platform     : $(PLATFORM)\n"

INFO_PROG = $(INFO_LINE)$(INFO_TITLE)$(INFO_LINE)$(INFO_VER)$(INFO_DEV)$(INFO_PLAT)$(INFO_LINE)

################################################################################
# Descriptions of certain make tasks.                                          #
################################################################################
DESC_ALL =		   "Compile and make $(LIB_TARGET).\n"
DESC_APPLICATION = $(DESC_ALL)
DESC_OBJECTS =     "Compile all object files in $(OBJ_DIR) sub-directory.\n"
DESC_INSTALL =     "Install $(LIB_NAME) to $(LIB_DIR) and $(LIB_HEADERS) to $(HEAD_DIR)\n"
DESC_REMOVE =      "Remove $(LIB_NAME) from $(LIB_DIR) and $(LIB_HEADERS) from $(HEAD_DIR)\n"
DESC_CLEAN =       "Remove any files created for a clean install.\n"

################################################################################
# ALL																		   #
# LIBRARY																	   #
################################################################################
# make all and make library perform the same task of resolving to making the   #
# target libary file.                                                          #
################################################################################
all:				library
	@echo Run \'make install\' to use this library with other projects...

library:			pre-compile-info $(LIB_TARGET)

################################################################################
# make pre-compile-info echos pre-compilation information to the screen.       #
################################################################################
pre-compile-info:
	@echo -e $(INFO_PROG)

################################################################################
# make LIB_TARGET depends on the on all the substituent object files.  These   #
# are linked to create the final target.									   #
################################################################################
$(LIB_TARGET):		objects
	@echo Linking Libary...
	@echo -e "\tLinking: Object files in $(OBJ_DIR) --> $(LIB_TARGET)"
	@$(LD) $(LDFLAGS) -o $(LIB_TARGET) $(OBJ_DIR)/*.o
	@echo Library Linked...
	@echo

################################################################################
# OBJECTS																	   #
################################################################################
# make objects makes all the object files in the OBJ_DIR sub-directory, it     #
# depends on the dependencies file, ensuring that any dependencies are updated #
# if the source code changes.                                                  #
################################################################################
objects:		depend pre-object-info $(addprefix $(OBJ_DIR)/,$(SRC_FILES:.cpp=.o))
	@echo Object Files Compiled...
	@echo

################################################################################
# make pre-object-info echos pre-compilation information to the screen.        #
################################################################################
pre-object-info:
	@echo Making Object Files...

################################################################################
# Standard GCC compiler commands to compile source files.                      #
################################################################################
$(OBJ_DIR)/%.o:			$(SRC_DIR)/%.cpp
	@echo -e "\tCompiling: $< --> $@"
	@$(CC) $(PRE_CFLAGS) $(CFLAGS) $(CFLAGS_$@) -c -o $@ $<

################################################################################
# DEPEND                                                                       #
################################################################################
# Include dependencies file if it exists.                                      #
################################################################################
ifeq ($(DEPEND_FILE), $(wildcard $(DEPEND_FILE)))
include $(DEPEND_FILE)
endif

################################################################################
# make dep, fastdepend and depend depends on the .depend file.                 #
################################################################################
dep fastdep depend:		$(DEPEND_FILE)

################################################################################
# make the dependencies file depends on all the SRC files and any header files #
# in the src directory.  We use gcc to output the dependencies into the        #
# dependencies sed script before it is saved to .tmp which then replaces the   #
# old dependencies file.                                                       #
################################################################################
$(DEPEND_FILE):			$(addprefix $(SRC_DIR)/,$(SRC_FILES)) $(SRC_DIR)/*.h
	@echo Generating Dependencies...
	@$(CC) $(PRE_CFLAGS) $(CFLAGS) -M $(addprefix $(SRC_DIR)/,$(SRC_FILES)) | $(DEP_FILTER) > .tmp
	@mv -f .tmp $(DEPEND_FILE)
	@echo

################################################################################
# INSTALL																	   #
################################################################################
# Installs the compiled library to the system, depends on the library being    #
# compiled.  The target file is copied to the /usr/lib directory, we run       #
# ldconfig to ensure that the dynamic link is created for the give soname.     #
# We also create a symbolic link to the library name for compilation/linking   #
# purposes, finally, the header files must be copied to /usr/include.          #
################################################################################
install:			library
	@echo Installing Libary...
	@echo -e "\tCopying $(LIB_TARGET) to $(LIB_DIR)..."
	@cp $(LIB_TARGET) $(LIB_DIR)
	@echo -e "\tCreating dynamic linking file $(LIB_SONAME) if required..."
	@$(CMD_DYLINK)
	@echo -e "\tCreating static linking file $(LIB_NAME) if required..."
	@$(CMD_STLINK)
	@echo -e "\tInstalling header files $(LIB_HEADERS) to $(HEAD_DIR)..."
	@cp $(SRC_DIR)/$(LIB_HEADERS) $(HEAD_DIR)
	@chmod 0644 $(HEAD_DIR)/$(LIB_HEADERS)
	@echo Install Complete...
	@echo

################################################################################
# REMOVE																	   #
################################################################################
# Removes the compiled library from the system, deletes the library file two   #
# symbolic links from the /usr/lib directory and the header files from the     #
# /usr/include directory.													   #
################################################################################
remove:				pre-compile-info
	@echo Removing Libary...
	@echo -e "\tRemoving $(LIB_TARGET) and symbolic links from $(LIB_DIR)..."
	@rm -f $(LIB_DIR)/$(LIB_TARGET) $(LIB_DIR)/$(LIB_SONAME) $(LIB_DIR)/$(LIB_NAME)
	@echo -e "\tRemoving header files $(LIB_HEADERS) from $(HEAD_DIR)..."
	@rm -f $(HEAD_DIR)/$(LIB_HEADERS)
	@echo Remove Complete...
	@echo

################################################################################
# CLEAN																		   #
# MRPROPER																	   #
################################################################################
# Remove any files created for a clean install.                                #
################################################################################
clean mrproper::	pre-compile-info
	@echo Removing all generated files in $(OBJ_DIR)...
	@rm -f $(OBJ_DIR)/*.o $(OBJ_DIR)/*.a $(DEPEND_FILE)
	@echo Removing $(LIB_TARGET)...
	@rm -f $(LIB_TARGET)

################################################################################
# HELP                                                                         #
################################################################################
# Help messages.                                                               #
################################################################################
HLP_HELP =    "help\tthis screen.\n"
HLP_ALL =     "all\t"$(DESC_ALL)
HLP_LIBRARY = "library\t"$(DESC_APPLICATION)
HLP_OBJECTS = "objects\t"$(DESC_OBJECTS)
HLP_INSTALL = "install\t"$(DESC_INSTALL)
HLP_REMOVE =  "remove\t"$(DESC_REMOVE)
HLP_CLEAN =   "clean\t"$(DESC_CLEAN)

HLP_COMMANDS = $(HLP_HELP)$(HLP_ALL)$(HLP_LIBRARY)$(HLP_OBJECTS)$(HLP_INSTALL)$(HLP_REMOVE)$(HLP_CLEAN)

################################################################################
# make help resolves to echoing available make options.                        #
################################################################################
help:					pre-compile-info
	@echo Makefile Help
	@echo
	@echo Usage: make [help all library objects install remove clean]
	@echo
	@echo -e $(HLP_COMMANDS)

################################################################################
# End of File: Makefile														   #
################################################################################

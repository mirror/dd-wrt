################################################################################
# File: Makefile                                                               #
################################################################################
# Makefile for the socketcc library sample application.  This will first make  #
# the sample code into a usable echo program.                                  #
################################################################################
# Written By: Jason But                                                        #
# Copyright:  CTIE - Monash University 2001                                    #
#                                                                              #
# Notes:                                                                       #
#    Version 1.00 - Original Version of application.                           #
#                                                                              #
#    Version 1.32 - Added new sample application.                              #
################################################################################

################################################################################
# Module compile information.                                                  #
################################################################################
#     ECHO_SRC     - Source filename of echo application.                      #
#     ECHO_TARGET  - Final target filename of echo applicaiton.                #
################################################################################
ECHO_SRC = myecho.cpp
ECHO_TARGET = myecho
DEAMON_SRC = application.cpp daemon.cpp tcpserver.cpp
DEAMON_TARGET = mydeamon

################################################################################
# Linker and associated flags.                                                 #
################################################################################
GCC = gcc
CFLAGS = -I$/usr/include -Wall -Wstrict-prototypes -O2
LDFLAGS = -lpthreadcc -lsocketcc

INFO_PROG = $(INFO_LINE)$(INFO_TITLE)$(INFO_LINE)$(INFO_VER)$(INFO_DEV)$(INFO_LINE)

################################################################################
# Descriptions of certain make tasks.                                          #
################################################################################
DESC_ALL =         "Compile and make $(ECHO_TARGET) and $(DEAMON_TARGET).\n"
DESC_CLEAN =       "Remove any files created for a clean install.\n"

################################################################################
# ALL                                                                          #
################################################################################
# make all and make library perform the same task of resolving to making the   #
# target libary file.                                                          #
################################################################################
all:                pre-compile-info $(ECHO_TARGET) $(DEAMON_TARGET)

################################################################################
# make pre-compile-info echos pre-compilation information to the screen.       #
################################################################################
pre-compile-info:
	@echo -e $(INFO_PROG)

################################################################################
# make LIB_TARGET depends on the on all the substituent object files.  These   #
# are linked to create the final target.                                       #
################################################################################
$(ECHO_TARGET):      $(ECHO_SRC)
	@echo -e "Compiling and Linking $(ECHO_SRC) --> $(ECHO_TARGET)..."
	@$(GCC) $(LDFLAGS) $(CFLAGS) -o $(ECHO_TARGET) $(ECHO_SRC)
	@echo Done
	@echo

$(DEAMON_TARGET):   objects
	@echo Linking Executable...
	@echo -e "\tLinking: Object files --> $(DEAMON_TARGET)"
	@$(GCC) $(LDFLAGS) -o $(DEAMON_TARGET) *.o
	@echo Executable Linked...
	@echo

################################################################################
# OBJECTS																	   #
################################################################################
# make objects makes all the object files in the OBJ_DIR sub-directory, it     #
# depends on the dependencies file, ensuring that any dependencies are updated #
# if the source code changes.                                                  #
################################################################################
objects:		pre-object-info $(DEAMON_SRC:.cpp=.o)
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
%.o:			%.cpp
	@echo -e "\tCompiling: $< --> $@"
	@$(GCC) $(CFLAGS) -c -o $@ $<

################################################################################
# CLEAN                                                                        #
# MRPROPER                                                                     #
################################################################################
# Remove any files created for a clean install.                                #
################################################################################
clean mrproper::    pre-compile-info
	@echo Removing $(ECHO_TARGET)...
	@rm -f $(ECHO_TARGET)
	@echo Removing $(DEAMON_TARGET)...
	@rm -f $(DEAMON_TARGET)
	@echo Removing Object files...
	@rm -f *.o

################################################################################
# HELP                                                                         #
################################################################################
# Help messages.                                                               #
################################################################################
HLP_HELP =    "help\tthis screen.\n"
HLP_ALL =     "all\t"$(DESC_ALL)
HLP_CLEAN =   "clean\t"$(DESC_CLEAN)

HLP_COMMANDS = $(HLP_HELP)$(HLP_ALL)$(HLP_CLEAN)

################################################################################
# make help resolves to echoing available make options.                        #
################################################################################
help:               pre-compile-info
	@echo Makefile Help
	@echo
	@echo Usage: make [help all clean]
	@echo
	@echo -e $(HLP_COMMANDS)

################################################################################
# End of File: Makefile                                                        #
################################################################################

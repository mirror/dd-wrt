#
# Top level OSAL Makefile
#
# 
# @par
# IXP400 SW Release Crypto version 2.1
# 
# -- Copyright Notice --
# 
# @par
# Copyright (c) 2001-2005, Intel Corporation.
# All rights reserved.
# 
# @par
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the Intel Corporation nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
# 
# 
# @par
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
# 
# 
# @par
# -- End of Copyright Notice --

################################################################
# Determine the target OS and Endian mode


## IX_TARGET needs to be set as one of linuxbe/linuxle, vxle/vxbe.
## wince has its own build system.

## Reuse IX_TARGET for TARGET_ENDIAN. 
IX_OSAL_MK_TARGET_ENDIAN := $(IX_TARGET)

# NOTE - this is case-sensitive
ifneq (,$(filter $(IX_TARGET), linuxbe linuxle))
IX_OSAL_MK_TARGET_OS := linux
IX_OSAL_MK_LINUXVER := -$(IX_LINUXVER)
else
IX_OSAL_MK_TARGET_OS := vxworks
endif


################################################################
# Determine the target platform
#

# NOTE: Add support for other platforms here
# NOTE - this is case-sensitive

IX_OSAL_MK_PLATFORM := ixp400

# NOTE: Add support for various Target Device Type 
#       for example, ixp425, ixp465 and etc.
PLATFORM_DEVICE := ixp425 ixp465

ifneq (,$(filter $(IX_DEVICE), ixp46X))
IX_OSAL_MK_TGT_DEVICE := ixp465
else
IX_OSAL_MK_TGT_DEVICE := ixp425
endif

################################################################
# Determine the OSAL Optional Modules to build

# NOTE - this is case-sensitive

MODULES := ioMem bufferMgt


################################################################
# OSAL sub-directories (this layout is replicated in many places)
#

OSAL_DIRS = . \
            core \
            platforms/$(IX_OSAL_MK_PLATFORM) \
            $(PLATFORM_DEVICE:%=platforms/$(IX_OSAL_MK_PLATFORM)/%) \
            $(MODULES:%=modules/%)

################################################################
# Output directories 
#

MAIN_SRC_PREFIX := src
OS_SRC_PREFIX := os/$(IX_OSAL_MK_TARGET_OS)/$(MAIN_SRC_PREFIX)

MAIN_INC_PREFIX := include
OS_INC_PREFIX := os/$(IX_OSAL_MK_TARGET_OS)/$(MAIN_INC_PREFIX)

OBJ_BASE_DIR := lib/$(IX_OSAL_MK_TGT_DEVICE)/$(IX_OSAL_MK_TARGET_OS)/$(IX_OSAL_MK_TARGET_ENDIAN)$(IX_OSAL_MK_LINUXVER)

SRC_DIRS := $(OSAL_DIRS:%=$(MAIN_SRC_PREFIX)/%) $(OSAL_DIRS:%=$(OS_SRC_PREFIX)/%)


# Due to limitations of 'mkdir' on Windows, we need to make each
# subdirectory level in turn, so we build the list here


# We replicate the directory structure of the 'src' directory
# This makes it easier to relate dependencies between build objects, source files
# and the respective 'component.mk' files.
OUTPUT_DIRS := lib \
               lib/$(IX_OSAL_MK_TGT_DEVICE) \
               lib/$(IX_OSAL_MK_TGT_DEVICE)/$(IX_OSAL_MK_TARGET_OS) \
               lib/$(IX_OSAL_MK_TGT_DEVICE)/$(IX_OSAL_MK_TARGET_OS)/$(IX_OSAL_MK_TARGET_ENDIAN)$(IX_OSAL_MK_LINUXVER) \
               $(OBJ_BASE_DIR) \
               $(OBJ_BASE_DIR)/$(MAIN_SRC_PREFIX) \
               $(OBJ_BASE_DIR)/$(MAIN_SRC_PREFIX)/core \
               $(OBJ_BASE_DIR)/$(MAIN_SRC_PREFIX)/platforms \
               $(OBJ_BASE_DIR)/$(MAIN_SRC_PREFIX)/platforms/$(IX_OSAL_MK_PLATFORM) \
               $(PLATFORM_DEVICE:%=$(OBJ_BASE_DIR)/$(MAIN_SRC_PREFIX)/platforms/$(IX_OSAL_MK_PLATFORM)/%) \
               $(OBJ_BASE_DIR)/$(MAIN_SRC_PREFIX)/modules \
               $(MODULES:%=$(OBJ_BASE_DIR)/$(MAIN_SRC_PREFIX)/modules/%) \
               $(OBJ_BASE_DIR)/os \
               $(OBJ_BASE_DIR)/os/$(IX_OSAL_MK_TARGET_OS) \
               $(OBJ_BASE_DIR)/$(OS_SRC_PREFIX) \
               $(OBJ_BASE_DIR)/$(OS_SRC_PREFIX)/core \
               $(OBJ_BASE_DIR)/$(OS_SRC_PREFIX)/platforms \
               $(OBJ_BASE_DIR)/$(OS_SRC_PREFIX)/platforms/$(IX_OSAL_MK_PLATFORM) \
               $(PLATFORM_DEVICE:%=$(OBJ_BASE_DIR)/$(OS_SRC_PREFIX)/platforms/$(IX_OSAL_MK_PLATFORM)/%) \
               $(OBJ_BASE_DIR)/$(OS_SRC_PREFIX)/modules \
               $(MODULES:%=$(OBJ_BASE_DIR)/$(OS_SRC_PREFIX)/modules/%) 


ifdef IX_OSAL_INTEG_TEST
OUTPUT_DIRS +=  $(OBJ_BASE_DIR)/test
endif

################################################################
# Compiler & Linker commands and options
#

# These macros are defined for the Target OS in the following file
# - IX_OSAL_MK_HOST_OS
# - CC
# - LD
# - AR
# - CFLAGS
# - LDFLAGS
include os/${IX_OSAL_MK_TARGET_OS}/make/macros.mk

# include any additional flags set by the user
CFLAGS += $(IX_OSAL_MK_CFLAGS) -I$(WIND_BASE)/target/h/wrn/coreip
LDFLAGS += $(IX_OSAL_MK_LDFLAGS)

ifdef IX_EXTRA_WARNINGS
# Optional "aggressive" warning flags. These will produce a lot of warning
# messages about OS header files, but there should still be
# no warnings in Intel production code.
ifeq ($(TOOL_FAMILY),diab)
CFLAGS += -Wsign-compare -Wformat -Wstrict-prototypes -Wshadow -Wmissing-prototypes \
	-Wmissing-declarations -Wnested-externs -Winline \
	-fstrict-aliasing -fno-builtin -fsigned-char
else	
CFLAGS += -Wsign-compare -Wformat -Wstrict-prototypes -Wshadow -Wmissing-prototypes \
	-Wmissing-declarations -Wnested-externs -Winline -Waggregate-return \
	-fstrict-aliasing -fno-builtin -fsigned-char
endif	# ifeq ($(TOOL_FAMILY),diab)
endif


################################################################
# Include directories
#

INCLUDE_DIRS = include $(OSAL_DIRS:%=$(MAIN_INC_PREFIX)/%) $(OSAL_DIRS:%=$(OS_INC_PREFIX)/%)  

ifeq ($(IX_OSAL_MK_HOST_OS), linux)
INCLUDE_DIRS += $(LINUX_SRC)/include/asm-arm/arch-ixp425/
endif

CFLAGS += $(INCLUDE_DIRS:%=-I%) 


################################################################
# Other commands
#

# A sed command to process the output of $(CC) -M. It prepends a
# directory name to the object filename and adds the .d file to the
# rule as a rule target, and then appends the name of this makefile
# and the component makefile corresponding to the particular object as
# a dependency.
#
# Note the use of '=' rather than ':=' so that the value of $* will
# be evaluated later, when we use the command within a template rule.
#
# Note where '$$' is used in the sed commands, 'make' interprets it as
# a single '$'.

ifeq ($(TOOL_FAMILY),diab)
FILTER_DEPS = sed -e $(SAFE_QUOTE)s,$(notdir $*)\.o:,$(OBJ_BASE_DIR)$*.d $(OBJ_BASE_DIR)/$*.o: $(dir $*)component.mk Makefile,g$(SAFE_QUOTE)
else
FILTER_DEPS = sed -e $(SAFE_QUOTE)s,$(notdir $*)\.o,$(OBJ_BASE_DIR)$*.d $(OBJ_BASE_DIR)/$*.o,g$(SAFE_QUOTE) \
			-e $(SAFE_QUOTE)$$s,$$, $(dir $*)component.mk Makefile,$(SAFE_QUOTE)
endif

################################################################
# Build platform specific macros
#
# These are : CMD_SEP : the string we use to seperate multiple
# 			commands on a single line.
#             ECHO_QUOTE : to print leading whitespace with the unix
#                       shells' echo command, we need quotes.
#             SAFE_QUOTE : the qoute character needed to enclose a 
#                       'sed' command.
#	      ECHO_BLANKLINE : The command to print an empty line.
#             RM : unconditionally delete file(s)

ifeq ($(IX_OSAL_MK_HOST_OS),windows)
# Windows settings
CMD_SEP := &&
ECHO_QUOTE := # cmd.exe's echo command doesn't need quotes
ECHO_BACKSLASH := \# comment needed to keep \ away from end of line
SAFE_QUOTE := "# " this comment is here to fix emacs syntax highlighting 
ECHO_BLANKLINE := @cmd.exe /c echo.
RM := vxrm 
# Windows doesn't have a 'touch' command. Warning! Unlike Unix touch,
# this will overwrite the existing file!
TOUCH_EMPTY_FILE := echo. >
else
# Unix settings
CMD_SEP := ;
ECHO_QUOTE := "# " this comment is here to fix emacs syntax highlighting 
ECHO_BACKSLASH := \\
SAFE_QUOTE := '# ' this comment is here to fix emacs syntax highlighting 
ECHO_BLANKLINE := @echo
RM := rm -f
TOUCH_EMPTY_FILE := touch
endif

# A command for printing trace messages in the makefile.
MAKEFILE_TRACE := @echo $(ECHO_QUOTE)--- Makefile: $(ECHO_QUOTE) # Use this setting to enable messages
#MAKEFILE_TRACE = @\#  # Use this setting to disable messages


################################################################
# Component makefiles
#

# Each source directory should contain a component.mk file which contains 
# a list of objects to be built and extra CFLAGS specific to the code in that directory

# include component,mk files from all source directories
include $(SRC_DIRS:%=%/component.mk)

# include test directory if want integration test suite
ifdef IX_OSAL_INTEG_TEST
include test/component.mk
endif



################################################################
# Platform makefiles
#

PLATFORM_MAIN_DIR := $(MAIN_SRC_PREFIX)/platforms/$(IX_OSAL_MK_PLATFORM)
PLATFORM_OS_DIR := $(OS_SRC_PREFIX)/platforms/$(IX_OSAL_MK_PLATFORM)

# PLATFORM_OBJ lists all of the object files in the platform components with path info
PLATFORM_OBJ := $($(IX_OSAL_MK_PLATFORM)_OBJ:%=$(OBJ_BASE_DIR)/$(PLATFORM_MAIN_DIR)/%) \
								$( foreach m, $(PLATFORM_DEVICE), $($(m)_OBJ:%=$(OBJ_BASE_DIR)/$(PLATFORM_MAIN_DIR)/$(m)/%) ) \
								$($(IX_OSAL_MK_PLATFORM)_os_OBJ:%=$(OBJ_BASE_DIR)/$(PLATFORM_OS_DIR)/%) \
								$( foreach m, $(PLATFORM_DEVICE), $($(m)_os_OBJ:%=$(OBJ_BASE_DIR)/$(PLATFORM_OS_DIR)/$(m)/%) )


################################################################
# Test makefiles
#
ifdef IX_OSAL_INTEG_TEST
TEST_MAIN_DIR := test
TEST_OBJ := $(test_OBJ:%=$(OBJ_BASE_DIR)/test/%)  
endif


# The correct CFLAGS on a per file basis (in the compile rule) and not globally.
# CFLAGS += $($(IX_OSAL_MK_PLATFORM)_os_CFLAGS)


################################################################
# Core makefiles
#

CORE_MAIN_DIR := $(MAIN_SRC_PREFIX)/core
CORE_OS_DIR := $(OS_SRC_PREFIX)/core

# CORE_OBJ lists all of the object files in the OSAL Core directories, with path info
CORE_OBJ := $(core_OBJ:%=$(OBJ_BASE_DIR)/$(CORE_MAIN_DIR)/%) \
            $(core_os_OBJ:%=$(OBJ_BASE_DIR)/$(CORE_OS_DIR)/%)

# The correct CFLAGS on a per file basis (in the compile rule), not globally
# CFLAGS += $(osal_$(IX_OSAL_MK_TARGET_OS)_CFLAGS) \
#          $(core_shared_CFLAGS)

################################################################
# Optional module makefiles
#

ifneq (0,$(words $(MODULES)))

MODULE_MAIN_BASE_DIR := $(MAIN_SRC_PREFIX)/modules
MODULE_OS_BASE_DIR := $(OS_SRC_PREFIX)/modules

# MODULE_OBJ lists all of the object files in the OSAL module directories, with path info
MODULE_OBJ := $(foreach m, $(MODULES), $($(m)_OBJ:%=$(OBJ_BASE_DIR)/$(MODULE_MAIN_BASE_DIR)/$(m)/%)) \
              $(foreach m, $(MODULES), $($(m)_os_OBJ:%=$(OBJ_BASE_DIR)/$(MODULE_OS_BASE_DIR)/$(m)/%))

# The correct CFLAGS on a per file basis (in the compile rule), not globally
# CFLAGS += $(foreach m, $(MODULES), $($(m)_os_CFLAGS)) \
#           $(foreach m, $(MODULES), $($(m)_shared_CFLAGS))

endif


################################################################
# Combined object list macros
#

ALL_OBJ := $(PLATFORM_OBJ) $(CORE_OBJ) $(MODULE_OBJ)

# Add test suite if needed
ifdef IX_OSAL_INTEG_TEST
ALL_OBJ += $(TEST_OBJ)
endif

################################################################
# Rules
#

# Default target
usage:
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)-------------------------------------------- $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE)osal/Makefile usage (for vxworks and linux) $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE)-------------------------------------------- $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE)Requirements: $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE) - set IX_TARGET to one of the: vxle, vxbe, linuxle, linuxbe $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE) - source the same environment.tcsh as ixp400_xscale$(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE)-------------------------------------------- $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE)Build commands: $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE)make libosal:  to build osal library$(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE)make clean:    to remove library and dependency files $(ECHO_QUOTE) 
	$(ECHO_BLANKLINE)



################################################################
# Output directories

#
# Other rules should depend on OUTPUT_DIR_FLAGS, never OUTPUT_DIRS.
# Depending on OUTPUT_DIRS would cause a rule to fire whenever
# any files in the output directories are modified.
#
# Because the automatic dependency rules and 'Makefile:' depend on
# OUTPUT_DIR_FLAGS, no other rules need to depend on them, as the
# dependencies are always built first.

OUTPUT_DIR_FLAGS := $(OUTPUT_DIRS:%=%/.dirCreationFlag)

$(OUTPUT_DIR_FLAGS):
	$(MAKEFILE_TRACE) Creating output directory
ifeq ($(IX_OSAL_MK_HOST_OS),windows)
# The MSDOS 'mkdir' command must use '\' as path seperator and will
# fail if the dir already exists, so we need a '-'
	-mkdir $(subst /,\,$(patsubst %/.dirCreationFlag,%,$@))
else
	mkdir -p $(patsubst %/.dirCreationFlag,%,$@)
endif
	$(TOUCH_EMPTY_FILE) $@

# If the makefile depends on something, it will always be built. Note
# that unless IX_BYPASS_DEPS is defined, the output directories are
# made anyway, because they depend on the %.d rule, which is processed
# before this one.
Makefile: $(OUTPUT_DIR_FLAGS)


################################################################
# Template rules to compile C and assembler
#

# Place per-directory CFLAGS here if needed here.

# Place the use per file CFLAGS instead of per-directory CFLAGS, probably easier to implement here.

# Place CFLAGS to pass the target os name to the C code
CFLAGS += -DIX_OSAL_TARGET_OS_EXT=_$(IX_OSAL_MK_TARGET_OS)
CFLAGS += -DIX_OSAL_PLATFORM_EXT=_$(IX_OSAL_MK_PLATFORM)

# Place CFLAGS supplied by the host buildSystem (e.g. xscale_sw for IXP400) here.


CFLAGS += -DIX_COMPONENT_NAME=ix_osal

# Rule to build .d dependency files for C files (the object filename contains the path info to match the .c file)
$(OBJ_BASE_DIR)/%.d: %.c $(OUTPUT_DIR_FLAGS)
	$(MAKEFILE_TRACE) Building dependency info for $@ from C
	$(CC) $(MAKE_DEP_FLAG) $(CFLAGS) $(ASM_C_RULES_CFLAGS_FOR_FILE) -DIX_GENERATING_DEPENDENCIES $< > $@.tmp
	$(FILTER_DEPS) < $@.tmp > $@
	$(RM) $@.tmp

# Rule to build dependency info for assembler (the object filename contains the path info to match the .s file)
$(OBJ_BASE_DIR)/%.d: %.s $(OUTPUT_DIR_FLAGS)
	$(MAKEFILE_TRACE) Building dependency info for $@ from asm
ifeq ($(TOOL_FAMILY),diab)
	$(CC) $(MAKE_DEP_FLAG) $(CFLAGS) -Xpreprocess-assembly -W:as:,-Xlabel-colon,-Xsemi-is-newline \
	    $(ASM_C_RULES_CFLAGS_FOR_FILE) $< > $@.tmp
else
	$(CC) $(MAKE_DEP_FLAG) $(CFLAGS) -x assembler-with-cpp $(ASM_C_RULES_CFLAGS_FOR_FILE) $< > $@.tmp
endif
	$(FILTER_DEPS) < $@.tmp > $@
	$(RM) $@.tmp


# Rule to build object file from c code (the object filename contains the path info to match the .c file)
$(OBJ_BASE_DIR)/%.o: %.c
	$(MAKEFILE_TRACE) Building object file $@ from C
	$(CC) $(CFLAGS) $< -c -o $@

# Rule to build object file from assembler (the object filename contains the path info to match the .s file)
$(OBJ_BASE_DIR)/%.o: %.s
	$(MAKEFILE_TRACE) Building object file $@ from asm
ifeq ($(TOOL_FAMILY),diab)
	$(CC) $(CFLAGS)	-Xpreprocess-assembly -W:as:,-Xlabel-colon,-Xsemi-is-newline \
	    $< -c -o $@
else
	$(CC) $(CFLAGS)	-P -x assembler-with-cpp $< -c -o $@
endif

DEPS := $(ALL_OBJ:.o=.d)
include $(DEPS)




# Rule to build OSAL library
libosal : $(OBJ_BASE_DIR)/libosal.a
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)  CFLAGS = $(CFLAGS) $(ECHO_QUOTE) 
	$(ECHO_BLANKLINE)

$(OBJ_BASE_DIR)/libosal.a : $(ALL_OBJ)
	$(MAKEFILE_TRACE) Building OSAL library containing components $(COMPONENTS:%=\"%\")
	$(AR) rvs $(OBJ_BASE_DIR)/libosal.a $^
 
# Simple rule to build everything
.PHONY: all
all: libosal

# To build loadable 
ALL_LOADABLES := $(OBJ_BASE_DIR)/osalTest.out

OSAL_MODULES := $(OBJ_BASE_DIR)/ixp_osal.o
module : $(OSAL_MODULES) 

.phony: loadable
loadable: $(ALL_LOADABLES)

.phony: loadable-test
loadable-test: $(OBJ_BASE_DIR)/osalTest.out

################################################
# Full paths to the component and test objects going into the image
LOADABLE_RULE_OBJ = $(ALL_OBJ)
# The names of the libraries that will be linked to a loadable
LOADABLE_RULE_LIB_FILES = $(OBJ_BASE_DIR)/libosal.a


# Rule to build a loadable object
$(ALL_LOADABLES) : $(OBJ_BASE_DIR)/%.out :
	$(MAKEFILE_TRACE) Building relocatable object for component $*
ifeq ($(IX_TARGET_OS),vxworks)
	$(NM) $(LOADABLE_RULE_OBJ) | $(MUNCH) > $(PATTERN_OBJ_BASE_DIR)/ctdt.c
	$(COMPILE_TRADITIONAL) $(CFLAGS) $(PATTERN_OBJ_DIR)/ctdt.c \
		-o  $(PATTERN_OBJ_DIR)/ctdt.o
	$(LD) $(LDFLAGS) $(LOADABLE_RULE_OBJ) $(PATTERN_OBJ_DIR)/ctdt.o\
		-L$(OBJ_BASE_DIR)  -o $(OBJ_BASE_DIR)/@
else # IX_TARGET_OS == vxworks
	$(LD) $(LDFLAGS)  $(LOADABLE_RULE_OBJ) \
		-L$(OBJ_BASE_DIR)  -o $@
endif # IX_TARGET_OS == vxworks

$(OBJ_BASE_DIR)/ixp_osal.o : $(filter-out $(TEST_OBJ), $(ALL_OBJ))
	$(LD) $(LDFLAGS) $(LOADABLE_RULE_OBJ) -L$(OBJ_BASE_DIR)  -o $@

################################################################
# Rulte to clean the build output directory
#

.PHONY: clean
clean:
	$(MAKEFILE_TRACE) Deleting all build products
ifeq ($(IX_OSAL_MK_HOST_OS),windows)
	rmdir /q /s lib
else
	$(RM) -r lib
endif

doc_clean :
	$(MAKEFILE_TRACE) Cleaning up documentation
	$(RM) $(DOXYGEN_OUTPUT_PATTERN)


################################################################
# showmacro - prints the value of a variable. For makefile debugging.
.PHONY: showmacro
showmacro:
	@echo $($(MACRO))


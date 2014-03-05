#
# Top level OSAL Makefile
#
# 
# @par
# IXP400 SW Release version 2.4
# 
# -- Copyright Notice --
# 
# @par
# Copyright (c) 2001-2007, Intel Corporation.
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
ifeq ($(IX_OSAL_OS_LEVEL), user_space)
IX_OSAL_MK_TARGET_OS := linux_user
else
IX_OSAL_MK_TARGET_OS := linux
endif

IX_OSAL_MK_LINUXVER := -$(IX_LINUXVER)
else
IX_OSAL_MK_TARGET_OS := vxworks
endif

################################################################
#  assign the target platform
#

IX_OSAL_MK_TGT_DEVICE := $(IX_DEVICE)


include platforms/$(IX_OSAL_MK_TGT_DEVICE)/os/$(IX_OSAL_MK_TARGET_OS)/make/OsalConfig.mk
################################################################
# Determine the OSAL Optional Modules to build

# NOTE - this is case-sensitive

#OPTIONAL_MODULES := ioMem bufferMgt

OPTIONAL_MODULES :=  

ifdef ENABLE_IOMEM
OPTIONAL_MODULES := ioMem 
endif

ifdef ENABLE_BUFFERMGT 
OPTIONAL_MODULES := $(OPTIONAL_MODULES) bufferMgt
endif

ifeq ($(IX_OSAL_OS_LEVEL), user_space)
MODULES := $(OPTIONAL_MODULES)
else
# In kernel space ddk is an important module and cannot
# be made optional.
MODULES := ddk $(OPTIONAL_MODULES)
endif


################################################################
# OSAL sub-directories (this layout is replicated in many places)
#

OSAL_DIRS = . \
            core \
            $(MODULES:%=modules/%)

################################################################
# Output directories 
#

MAIN_SRC_PREFIX := common/src
OS_SRC_PREFIX := common/os/$(IX_OSAL_MK_TARGET_OS)/src

MAIN_INC_PREFIX := common/include
OS_INC_PREFIX := common/os/$(IX_OSAL_MK_TARGET_OS)/include

OBJ_BASE_DIR := lib/$(IX_OSAL_MK_TGT_DEVICE)/$(IX_OSAL_MK_TARGET_OS)/$(IX_OSAL_MK_TARGET_ENDIAN)

SRC_DIRS := $(OSAL_DIRS:%=$(MAIN_SRC_PREFIX)/%) $(OSAL_DIRS:%=$(OS_SRC_PREFIX)/%)
SRC_DIRS += platforms/$(IX_OSAL_MK_TGT_DEVICE)/os/$(IX_OSAL_MK_TARGET_OS)/src

ifeq ($(IX_OSAL_OS_LEVEL), user_space)
SRC_DIRS += common/POSIX/src
endif

# Due to limitations of 'mkdir' on Windows, we need to make each
# subdirectory level in turn, so we build the list here


# We replicate the directory structure of the 'src' directory
# This makes it easier to relate dependencies between build objects, source files
# and the respective 'component.mk' files.
OUTPUT_DIRS := lib \
               lib/$(IX_OSAL_MK_TGT_DEVICE) \
               lib/$(IX_OSAL_MK_TGT_DEVICE)/$(IX_OSAL_MK_TARGET_OS) \
               lib/$(IX_OSAL_MK_TGT_DEVICE)/$(IX_OSAL_MK_TARGET_OS)/$(IX_OSAL_MK_TARGET_ENDIAN) \
               $(OBJ_BASE_DIR) \
               $(OBJ_BASE_DIR)/common \
               $(OBJ_BASE_DIR)/$(MAIN_SRC_PREFIX) \
               $(OBJ_BASE_DIR)/$(MAIN_SRC_PREFIX)/core \
               $(OBJ_BASE_DIR)/$(MAIN_SRC_PREFIX)/modules \
               $(MODULES:%=$(OBJ_BASE_DIR)/$(MAIN_SRC_PREFIX)/modules/%) \
               $(OBJ_BASE_DIR)/common/os \
               $(OBJ_BASE_DIR)/common/os/$(IX_OSAL_MK_TARGET_OS) \
               $(OBJ_BASE_DIR)/$(OS_SRC_PREFIX) \
               $(OBJ_BASE_DIR)/$(OS_SRC_PREFIX)/core \
               $(OBJ_BASE_DIR)/$(OS_SRC_PREFIX)/modules \
               $(MODULES:%=$(OBJ_BASE_DIR)/$(OS_SRC_PREFIX)/modules/%) 


OUTPUT_DIRS +=  $(OBJ_BASE_DIR)/platforms \
                $(OBJ_BASE_DIR)/platforms/$(IX_OSAL_MK_TGT_DEVICE) \
                $(OBJ_BASE_DIR)/platforms/$(IX_OSAL_MK_TGT_DEVICE)/os \
                $(OBJ_BASE_DIR)/platforms/$(IX_OSAL_MK_TGT_DEVICE)/os/$(IX_OSAL_MK_TARGET_OS) \
		$(OBJ_BASE_DIR)/platforms/$(IX_OSAL_MK_TGT_DEVICE)/os/$(IX_OSAL_MK_TARGET_OS)/src


# POSIX directory structure for output
ifeq ($(IX_OSAL_OS_LEVEL), user_space)


OUTPUT_DIRS +=  $(OBJ_BASE_DIR)/common/POSIX \
		$(OBJ_BASE_DIR)/common/POSIX/src

endif

ifdef IX_OSAL_INTEG_TEST
OUTPUT_DIRS +=  $(OBJ_BASE_DIR)/common/test
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
include platforms/$(IX_OSAL_MK_TGT_DEVICE)/os/$(IX_OSAL_MK_TARGET_OS)/make/OemMake.mk
include common/os/${IX_OSAL_MK_TARGET_OS}/make/macros.mk



# include any additional flags set by the user
CFLAGS += $(IX_OSAL_MK_CFLAGS)
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

INCLUDE_DIRS += common/include $(OSAL_DIRS:%=$(MAIN_INC_PREFIX)/%) $(OSAL_DIRS:%=$(OS_INC_PREFIX)/%)  

INCLUDE_DIRS +=  platforms/$(IX_OSAL_MK_TGT_DEVICE)/include
INCLUDE_DIRS +=  platforms/$(IX_OSAL_MK_TGT_DEVICE)/os/$(IX_OSAL_MK_TARGET_OS)/include

### Include OSAL POSIX directories in user space.
ifeq ($(IX_OSAL_OS_LEVEL), user_space)
INCLUDE_DIRS +=  common/POSIX/include

endif

CFLAGS += $(INCLUDE_DIRS:%=-I%) -DIX_COMPONENT=1

ifdef IX_HW_COHERENT_MEMORY
CFLAGS +=  -DIX_HW_COHERENT_MEMORY
endif

ifdef IX_OSAL_INTEG_TEST
CFLAGS += -DIX_OSAL_INTEG_TEST
endif

ifdef IX_OSAL_OSSL_SHIMLAYER_SUPPORT
CFLAGS += -DIX_OSAL_OSSL_SHIMLAYER_SUPPORT
endif

# Enable appropriate flags in the code depending
# on the modules selected for the build.

ifneq (,$(findstring ioMem,$(MODULES)))
CFLAGS += -DENABLE_IOMEM
endif

ifneq (,$(findstring bufferMgt,$(MODULES)))
CFLAGS += -DENABLE_BUFFERMGT
endif

ifneq (,$(findstring ddk,$(MODULES)))
CFLAGS += -DENABLE_DDK
endif

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
ifneq ($(VX_VERSION),vx55)
FILTER_DEPS = sed -e $(SAFE_QUOTE)s,$(notdir $*)\.o,$(OBJ_BASE_DIR)$*.d $(OBJ_BASE_DIR)/$*.o,g$(SAFE_QUOTE) \
			-e '$$s,$$, $(dir $*)component.mk Makefile,'
else
FILTER_DEPS = sed -e $(SAFE_QUOTE)s,$(notdir $*)\.o,$(OBJ_BASE_DIR)$*.d $(OBJ_BASE_DIR)/$*.o,g$(SAFE_QUOTE) \
			-e $(SAFE_QUOTE)$$s,$$, $(dir $*)component.mk Makefile,$(SAFE_QUOTE)
endif #VX_VERSION
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
ifeq ($(VXSHELL),ZSH)
# windows zsh settings
CMD_SEP := &&
ECHO_QUOTE := "# " this comment is here to fix emacs syntax highlighting 
ECHO_BACKSLASH := \# comment needed to keep \ away from end of line
SAFE_QUOTE := "# " this comment is here to fix emacs syntax highlighting 
ECHO_BLANKLINE := echo " "
RM := vxrm 
TOUCH_EMPTY_FILE := touch
else
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
endif
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
include common/test/component.mk
endif



################################################################
# Platform makefiles
#


PLATFORM_MAIN_DIR := platforms/$(IX_OSAL_MK_TGT_DEVICE)/src
PLATFORM_OBJ := $(platform_OBJ:%=$(OBJ_BASE_DIR)/platforms/$(IX_OSAL_MK_TGT_DEVICE)/os/$(IX_OSAL_MK_TARGET_OS)/src/%)

################################################################
# Test makefiles
#
ifdef IX_OSAL_INTEG_TEST
TEST_MAIN_DIR := common/test
TEST_OBJ := $(test_OBJ:%=$(OBJ_BASE_DIR)/common/test/%)  
endif



################################################################
# Core makefiles
#

CORE_MAIN_DIR := $(MAIN_SRC_PREFIX)/core
CORE_OS_DIR := $(OS_SRC_PREFIX)/core

# CORE_OBJ lists all of the object files in the OSAL Core directories, with path info
CORE_OBJ := $(core_OBJ:%=$(OBJ_BASE_DIR)/$(CORE_MAIN_DIR)/%) \
            $(core_os_OBJ:%=$(OBJ_BASE_DIR)/$(CORE_OS_DIR)/%)

################################################################
# POSIX makefiles
#

ifeq ($(IX_OSAL_OS_LEVEL), user_space)
POSIX_MAIN_DIR := common/POSIX/src
POSIX_OBJ := $(POSIX_OBJS:%=$(OBJ_BASE_DIR)/common/POSIX/src/%)
endif

################################################################
# Optional module makefiles
#

ifneq (0,$(words $(MODULES)))

MODULE_MAIN_BASE_DIR := $(MAIN_SRC_PREFIX)/modules
MODULE_OS_BASE_DIR := $(OS_SRC_PREFIX)/modules

# MODULE_OBJ lists all of the object files in the OSAL module directories, with path info
MODULE_OBJ := $(foreach m, $(MODULES), $($(m)_OBJ:%=$(OBJ_BASE_DIR)/$(MODULE_MAIN_BASE_DIR)/$(m)/%)) \
              $(foreach m, $(MODULES), $($(m)_os_OBJ:%=$(OBJ_BASE_DIR)/$(MODULE_OS_BASE_DIR)/$(m)/%))


endif


################################################################
# Combined object list macros
#

ALL_OBJ := $(PLATFORM_OBJ) $(CORE_OBJ) $(MODULE_OBJ)

# Include POSIX directory when building for user space.
ifeq ($(IX_OSAL_OS_LEVEL),user_space)
ALL_OBJ += $(POSIX_OBJ)
endif
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
	@echo $(ECHO_QUOTE)osal/Makefile usage (for linux) $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE)-------------------------------------------- $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE)Requirements: $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE) - IX_TARGET       : [linuxle | linuxbe] $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE) - IX_LINUXVER     : 2.6 $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE) - LINUX_SRC       : <Path of Linux kernel src> $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE) - IX_OSAL_OS_LEVEL: [kernel_space | user_space] $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE) - IX_TARGET       : linuxle $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE) - IX_OSAL_PLATFORM: platform_name $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE) - IX_HW_COHERENT_MEMORY: 1 $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE) - IX_OSAL_INTEG_TEST: [1 | 0] $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE) - source the same environment.tcsh as ixp400_xscale$(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE)-------------------------------------------- $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE)osal/Makefile usage (for vxworks) $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE)-------------------------------------------- $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE)Requirements: $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE) - IX_TARGET: [vxle | vxbe] $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE) - Source env_scripts/env_vxworksXY_xscale.bat in ixp_osal/ $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE)-------------------------------------------- $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE)Build commands: $(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE)make lkm:      to build osal as a loadable module$(ECHO_QUOTE) 
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

OUTPUT_DIR_FLAGS := $(sort $(OUTPUT_DIRS:%=%/.dirCreationFlag))

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
CFLAGS += -DIX_OSAL_PLATFORM_EXT=_$(IX_OSAL_MK_TGT_DEVICE)

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
-include $(DEPS)

    
# Rule to build loadable osal module
#EXTRA_LDFLAGS := --whole-archive
#obj-m := ixp_osal.o
#ixp_osal-objs := libosal.a

lkm : CFLAGS += -DIX_OSAL_MODULE    
lkm : $(OBJ_BASE_DIR)/libosal.a
ifneq ($(IX_OSAL_MK_TARGET_OS), vxworks)
ifndef IX_OSAL_INTEG_TEST
ifeq ($(IX_OSAL_MK_TARGET_OS), linux)
ifeq ($(IX_LINUXVER), 2.6)
	@echo 'LINUX_SRC := $(LINUX_SRC)' > $(IX_OSAL_KERN26_MAKEFILE)
	@echo 'EXTRA_LDFLAGS := --whole-archive' > $(IX_OSAL_KERN26_MAKEFILE)
	@echo ' ' >> $(IX_OSAL_KERN26_MAKEFILE)
	@echo 'obj-m := ixp_osal.o' >> $(IX_OSAL_KERN26_MAKEFILE)
	@echo ' ' >> $(IX_OSAL_KERN26_MAKEFILE)
	@echo 'ixp_osal-objs := libosal.a' >> $(IX_OSAL_KERN26_MAKEFILE)
	@echo ' ' >> $(IX_OSAL_KERN26_MAKEFILE)
	@echo 'default:' >> $(IX_OSAL_KERN26_MAKEFILE)
	@echo -ne "\t" >> $(IX_OSAL_KERN26_MAKEFILE)
	@echo 'make $(IX_KBUILD_CROSSCOMPILE) -C $(LINUX_SRC) M=$(shell pwd)/$(OBJ_BASE_DIR) V=1 modules' >> $(IX_OSAL_KERN26_MAKEFILE)
	make -C $(OBJ_BASE_DIR)
endif # ifeq IX_LINUXVER
endif # ifeq IX_OSAL_MK_TARGET_OS
else
	@echo "Error: Unset IX_OSAL_INTEG_TEST to build osal as loadable module"
	@exit 1
endif # IX_OSAL_INTEG_TEST
endif # ifeq IX_OSAL_MK_TARGET_OS, vxworks

	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)  CFLAGS = $(CFLAGS) $(ECHO_QUOTE) 
	$(ECHO_BLANKLINE)


IX_OSAL_KERN26_MAKEFILE := $(OBJ_BASE_DIR)/Makefile

# Rule to build OSAL library
libosal : $(OBJ_BASE_DIR)/libosal.a
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)  CFLAGS = $(CFLAGS) $(ECHO_QUOTE) 
	$(ECHO_BLANKLINE)

$(OBJ_BASE_DIR)/libosal.a : $(ALL_OBJ)
	$(MAKEFILE_TRACE) Building OSAL library containing components $(COMPONENTS:%=\"%\")
	$(AR) rvs $(OBJ_BASE_DIR)/libosal.a $^
 
	$(MAKEFILE_TRACE) Copy the static library file to a fixed location - OSAL source root.
ifneq ($(IX_OSAL_OS_LEVEL),user_space)
	cp $(OBJ_BASE_DIR)/libosal.a libosal.a
else
	cp $(OBJ_BASE_DIR)/libosal.a libosal_user.a
endif

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
	$(MAKEFILE_TRACE) Copy the testframe work file to the fixed location.
	cp $(OBJ_BASE_DIR)/osalTest.out .

################################################
# Full paths to the component and test objects going into the image
LOADABLE_RULE_OBJ = $(ALL_OBJ)
# The names of the libraries that will be linked to a loadable
LOADABLE_RULE_LIB_FILES = $(OBJ_BASE_DIR)/libosal.a


# Rule to build a loadable object
$(ALL_LOADABLES) : $(OBJ_BASE_DIR)/%.out :
	$(MAKEFILE_TRACE) Building relocatable object for component $*
ifeq ($(IX_OSAL_MK_TARGET_OS),vxworks)
	$(NM) $(LOADABLE_RULE_OBJ) | $(MUNCH) > $(OBJ_BASE_DIR)/ctdt.c
	$(COMPILE_TRADITIONAL) $(CFLAGS) $(OBJ_BASE_DIR)/ctdt.c \
		-o  $(OBJ_BASE_DIR)/ctdt.o
	$(LD) $(LDFLAGS) $(LOADABLE_RULE_OBJ) $(OBJ_BASE_DIR)/ctdt.o\
		-L$(OBJ_BASE_DIR)  -o $@
else
ifeq ($(IX_OSAL_MK_TARGET_OS),linux)
	$(LD) $(LDFLAGS)  $(LOADABLE_RULE_OBJ) \
		-L$(OBJ_BASE_DIR)  -o $@
else
	# Link with the realtime and pthread user space libraries.
	$(CC) $(LOADABLE_RULE_LIB_FILES) -lrt -pthread -o $@
endif
endif # IX_OSAL_MK_TARGET_OS == vxworks

$(OBJ_BASE_DIR)/ixp_osal.o : $(filter-out $(TEST_OBJ), $(ALL_OBJ))
	$(LD) $(LDFLAGS) $(LOADABLE_RULE_OBJ) -L$(OBJ_BASE_DIR)  -o $@

################################################################
# Rule to clean the build output directory
#

.PHONY: clean
clean:
	$(MAKEFILE_TRACE) Deleting all build products
ifeq ($(IX_OSAL_MK_HOST_OS),windows)
	rm -rf lib
else
	$(RM) -r lib
	$(RM) *.ko *.o *.a *.out 
endif

################################################################
# Rule specific to Platfrom specific release build rule 
#
-include platforms/$(IX_OSAL_MK_TGT_DEVICE)/os/$(IX_OSAL_MK_TARGET_OS)/make/RelMake.mk

################################################################
# showmacro - prints the value of a variable. For makefile debugging.
.PHONY: showmacro
showmacro:
	@echo $($(MACRO))


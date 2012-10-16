#
# Top level xscale_sw Makefile, will call OSAL & BSP Makefile
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
#
#

# Target OS selection
ifneq (,$(filter $(IX_TARGET), linuxbe linuxle))
IX_TARGET_OS := linux
else
IX_TARGET_OS := vxworks
endif

ifndef IX_LINUXVER
IX_LINUXVER := 2.6
endif

ifeq ($(IX_DEVICE), ixp43X)
    include Makefile_ixp43X
else
ifeq ($(IX_DEVICE), ixp46X)
    include Makefile_ixp46X
else
ifeq ($(IX_DEVICE), ixp42X)
    include Makefile_ixp42X
else
    @exit 1
    @echo "Error: IX_DEVICE not defined"
endif
endif
endif



################################################################
# List of all the components we know about
ALL_COMPONENTS := $(COMPONENTS) 
ALL_COMPONENTS += $(CODELETS_COMPONENTS:%=codelets_%)

################################################################
# vxWorks BSP selection
#

BSP_DIR := $(WIND_BASE)/target/config/$(BSP)


################################################################
# Linux kernel source

LINUX_SRC := $($(IX_TARGET)_KERNEL_DIR)

################################################################
# Inclusion of common Makefile

include Makefile.common

# Windows paths must use '\' as seperator for the MSDOS 'cd' command
ifeq ($(IX_HOST_OS),windows)
ifeq ($(VXSHELL),ZSH)
 BSP_DIR := $(subst \,/,$(BSP_DIR))
 WIND_BASE := $(subst \,/,$(WIND_BASE))
 IX_XSCALE_SW := $(subst \,/,$(IX_XSCALE_SW))
else
 BSP_DIR := $(subst /,\,$(BSP_DIR))
 WIND_BASE := $(subst /,\,$(WIND_BASE))
 IX_XSCALE_SW := $(subst /,\,$(IX_XSCALE_SW))
endif
endif

################################################################
# OSAL selection

OSAL_SRCDIR := $(IX_XSCALE_SW)/../ixp_osal
ifeq ($(IX_TARGET_OS),linux)
OSAL_IMAGE := ../ixp_osal/lib/$(IX_TGT_DEVICE)/$(IX_TARGET_OS)/$(IX_TARGET)/libosal.a
OSAL_MODULE := ../ixp_osal/lib/$(IX_TGT_DEVICE)/$(IX_TARGET_OS)/$(IX_TARGET)/ixp_osal.o
else
OSAL_IMAGE := ../ixp_osal/lib/$(IX_TGT_DEVICE)/$(IX_TARGET_OS)/$(IX_TARGET)/libosal.a
OSAL_MODULE := ../ixp_osal/lib/$(IX_TGT_DEVICE)/$(IX_TARGET_OS)/$(IX_TARGET)/ixp_osal.o
endif 

.PHONY: osal_build
osal_build :
	echo cd into $(OSAL_SRCDIR)
	cd $(OSAL_SRCDIR) $(CMD_SEP) $(MAKE) libosal IX_TARGET=$(IX_TARGET) IX_LINUXVER=${IX_LINUXVER}

$(OSAL_IMAGE) : osal_build

.PHONY: osal_module
osal_module :
	echo cd into $(OSAL_SRCDIR)
	cd $(OSAL_SRCDIR) $(CMD_SEP) $(MAKE) module IX_TARGET=$(IX_TARGET) IX_LINUXVER=${IX_LINUXVER}

$(OSAL_MODULE) : osal_module

################################################################
# Compiler & linker commands
#

ifeq ($(IX_TARGET_OS),vxworks)

# Suffix for Tornado tool names
ifeq ($(IX_TARGET),vxsim)
ifeq ($(IX_HOST_OS),solaris)
ifeq ($(VX_VERSION), vx55)
  VX_TOOL_SUFFIX = simso
else
  VX_TOOL_SUFFIX = sparc
endif #VX_VERSION
else # Linux or Windows
VX_TOOL_SUFFIX = pentium
endif
else
VX_TOOL_SUFFIX = arm
endif

ifeq ($(TOOL_FAMILY),)
ifeq  ($(findstring gnu,$(TOOL)),gnu)
TOOL_FAMILY	= gnu
else # findstring gnu
ifeq  ($(findstring diab,$(TOOL)),diab)
TOOL_FAMILY	= diab
endif
endif # findstring gnu
endif # TOOL_FAMILY

 
ifeq ($(TOOL_FAMILY),diab)
CC := dcc
else
CC := cc$(VX_TOOL_SUFFIX)
endif



# These are tools used to make a .out file with vxWorks ctor/dtor table.
ifeq ($(TOOL_FAMILY),diab)
LD := dld
AR := dar
ifeq ($(VX_VERSION), vx55)
  VXWORKS_VER := vxworks55
else
  VXWORKS_VER := vxworks62
endif #VX_VERSION
NM := ddump -M
else
LD := $(CC)
AR := ar$(VX_TOOL_SUFFIX)
NM := nm$(VX_TOOL_SUFFIX)
endif

ifeq ($(VXSHELL),ZSH)
# use default munch command
else
ifeq ($(TOOL_FAMILY),diab)
  ifeq ($(VX_VERSION), vx55)
    MUNCH := wtxtcl $(WIND_HOST_BASE)/host/src/hutils/munch.tcl -c $(VX_TOOL_SUFFIX)
  else
    MUNCH := wtxtcl $(WIND_HOST_BASE)/host/resource/hutils/tcl/munch.tcl -c $(VX_TOOL_SUFFIX)
  endif #VX_VERSION
else
  ifeq ($(VX_VERSION), vx55)
    MUNCH := wtxtcl $(WIND_HOST_BASE)/host/src/hutils/munch.tcl -asm $(VX_TOOL_SUFFIX)
  else
    MUNCH := wtxtcl $(WIND_HOST_BASE)/host/resource/hutils/tcl/munch.tcl -asm $(VX_TOOL_SUFFIX)
  endif #VX_VERSION
endif
endif

ifeq ($(TOOL_FAMILY),diab)
COMPILE_TRADITIONAL := $(CC) -c -Xdollar-in-ident
else
COMPILE_TRADITIONAL := $(CC) -c -fdollars-in-identifiers
endif

# This is used for the 'memusage' target
OBJDUMP := objdump$(VX_TOOL_SUFFIX)

else # IX_TARGET_OS == vxworks

# Linux tool names
ifeq ($(IX_TARGET), linuxbe)
LINUX_CROSS_COMPILE := ${IX_LINUX_CROSSCOMPILER}
else
LINUX_CROSS_COMPILE := ${IX_LINUX_CROSSCOMPILER}
endif

LD := $(LINUX_CROSS_COMPILE)ld
CC := $(LINUX_CROSS_COMPILE)gcc
AR := $(LINUX_CROSS_COMPILE)ar

# This is used for the 'memusage' target
OBJDUMP := $(LINUX_CROSS_COMPILE)objdump

endif # IX_TARGET_OS == vxworks

################################################################
# Compiler & linker options
#

ifeq ($(IX_TARGET_OS),vxworks)

CFLAGS := -DRW_MULTI_THREAD -D_REENTRANT -D_WRS_KERNEL \
	-Isrc/include -I$(WIND_BASE)/target/h \
	-I$(WIND_BASE)/target/h/wrn/coreip \
	-I$(WIND_BASE)/target/h/drv \
	-I$(WIND_BASE)/target/h/drv/intrCtl \
	-I$(WIND_BASE)/target/h/drv/i2c \
	-I$(WIND_BASE)/target/h/drv/timer \
	-I$(WIND_BASE)/target/h/drv/sio \
	-I$(BSP_DIR) \
	-I$(BSP_DIR)/../all


# Set additions to the compiler flag based on vxWorks version chosen
ifeq ($(VX_VERSION),vx55)
  CFLAGS += -DVX_VERSION55
endif #VX_VERSION

ifeq ($(TOOL_FAMILY),diab)
# compiler flags using Diab compiler
  DIAB_CFLAGS := -w -Xdialect-ansi -Xno-common -D__vxworks -D_DIAB_TOOL 
  MAKE_DEP_FLAG := -Xmake-dependency
else
# compiler flags using GNU compiler
  GNU_CFLAGS := -Wall -ansi -fno-common -fno-builtin
  ifneq ($(VX_VERSION), vx55)
    GNU_CFLAGS += -finline-limit=750
  endif #VX_VERSION
  MAKE_DEP_FLAG := -M
endif

# linker flags

ifeq ($(TOOL_FAMILY),diab)
  LDFLAGS := -r -W:as:,-x,-X
else
  LDFLAGS := -nostdlib -r -Wl,-X
endif

else # IX_TARGET_OS == vxworks
# linux compiler flags 
MAKE_DEP_FLAG := -M
ifeq ($(IX_LINUXVER),2.6) #kernel 2.6 arch flag
LINUX_MACH_CFLAGS := -D__LINUX_ARM_ARCH__=5 -march=armv5te -Wa,-mcpu=xscale \
		     -mtune=xscale 
else
LINUX_MACH_CFLAGS := -D__LINUX_ARM_ARCH__=5 -mcpu=xscale -mtune=xscale
endif

ifeq ($(IX_LINUXVER),2.6) #kernel 2.6 CFLAGS
CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -Wall -Wno-trigraphs -fno-common -pipe -msoft-float -DMODULE\
 -Isrc/include -D__linux -DCPU=33 -DXSCALE=33 $(LINUX_MACH_CFLAGS) 
else
CFLAGS := -D__KERNEL__ -I$(LINUX_SRC)/include -Wall -Wno-trigraphs -fno-common -pipe -mshort-load-bytes -msoft-float\
 -DMODULE -Isrc/include -D__linux -DCPU=33 -DXSCALE=33 $(LINUX_MACH_CFLAGS) -mapcs-32
endif

# Read microcode from file only
CFLAGS += -DIX_NPEDL_READ_MICROCODE_FROM_FILE


# Linux linker flags
LDFLAGS := -r

ifeq ($(IX_TARGET),linuxbe)
CFLAGS += -mbig-endian
LDFLAGS += -EB
else
CFLAGS += -mlittle-endian
LDFLAGS += -EL
endif

endif # IX_TARGET_OS == vxworks


ifdef IX_EXTRA_WARNINGS
# Optional "aggressive" warning flags. These will produce a lot of warning
# messages about Linux and vxWorks header files, but there should still be
# no warnings in Intel production code.

DIAB_CFLAGS += -Xlint=0x400 -Xforce-declarations -Xforce-prototypes -Xchar-signed \
    -Xmacro-undefined-warn -Xmismatch-warning -Xstatic-addr-warning 
GNU_CFLAGS += -Wsign-compare -Wformat -Wstrict-prototypes -Wshadow -Wmissing-prototypes \
	-Wmissing-declarations -Wnested-externs -Winline -Waggregate-return \
	-fstrict-aliasing -fno-builtin -fsigned-char

endif

ifndef IX_MPHY
ifndef IX_NPE_HSS_MPHY4PORT
CFLAGS += -DIX_UTOPIAMODE
endif
endif

ifdef IX_MPHYSINGLEPORT
CFLAGS += -DIX_MPHYSINGLEPORT
endif

ifdef IX_HSSETH_COEXIST
CFLAGS += -DIX_HSSETH_COEXIST
endif

ifdef IX_NPE_HSS_MPHY4PORT
CFLAGS += -DIX_NPE_HSS_MPHY4PORT
endif

ifeq ($(IX_TARGET_OS),linux)
# Default to build without debug symbols for Linux
IX_NOSYM := 1
endif

ifndef IX_NOSYM
#CFLAGS += -ggdb3
DIAB_CFLAGS += -g
GNU_CFLAGS += -ggdb3
endif

ifndef IX_NOOPT
#CFLAGS += -O2
    DIAB_CFLAGS += -Xsize-opt -XO -Xparse-size=8000 -Xinline=80 -Xunroll-size=80 -Xkill-reorder=0x11
    GNU_CFLAGS += -O2
endif

ifdef IX_ENSURE_OFF
CFLAGS += -DIX_ENSURE_OFF
endif

ifdef IX_PRIVATE_OFF
CFLAGS += -DIX_PRIVATE_OFF
endif

ifndef IX_DEBUG
# Note: If you change the flags added here, be sure to modify the
# (filter-out) statement in the definition of
# ASM_C_RULES_UNITTEST_FLAGS to remove the same flags.
CFLAGS += -DNDEBUG
endif

# Target platform dependent options
ifeq ($(IX_TARGET),vxsim)
  ifeq ($(IX_HOST_OS),solaris)
    CFLAGS += -DCPU=SIMSPARCSOLARIS
  else
    ifeq ($(IX_HOST_OS),linux)
      CFLAGS += -DCPU=SIMLINUX
    else # Windows
      $(MAKEFILE_TRACE) vxsim_unsupported_in_Windows
      # CFLAGS += -DCPU=SIMNT
    endif # .. Windows or Linux
  endif # .. solaris
else
  ifeq ($(IX_TARGET_OS),vxworks)
    LDFLAGS += -Wl
    CFLAGS += -DCPU=XSCALE -DCPU_XSCALE -DARMMMU=ARMMMU_XSCALE \
      	-DARMCACHE=ARMCACHE_XSCALE
    GNU_CFLAGS += -mno-sched-prolog -mcpu=xscale
    ifeq ($(VX_VERSION), vx55)
      GNU_CFLAGS += -mapcs-32
    endif #VX_VERSION
    ifeq ($(IX_TARGET),vxle)
      CFLAGS += -DARMEL -D__ARMEL__ -DLITTLE_ENDIAN_MODE
      DIAB_CFLAGS += -tARMXLS:$(VXWORKS_VER)
      ifeq ($(VX_VERSION), vx55)
        GNU_CFLAGS += -mlittle-endian
      else
        GNU_CFLAGS += -txscale -Wa,-EL
      endif #VX_VERSION
      ifeq ($(TOOL_FAMILY),diab)
        LDFLAGS += -tARMXLS:$(VXWORKS_VER)
      else
        ifeq ($(VX_VERSION), vx55)
          LDFLAGS += -mlittle-endian
        else
          LDFLAGS += -txscale -Wa,-EL
        endif #VX_VERSION
      endif
    else # $(IX_TARGET) == vxle
      CFLAGS += -DARMEB -D__ARMEB__ -DBIG_ENDIAN_MODE
      DIAB_CFLAGS += -tARMXES:$(VXWORKS_VER)
      ifeq ($(VX_VERSION), vx55)
        GNU_CFLAGS += -mbig-endian
      else
        GNU_CFLAGS += -txscalebe -Wa,-EB
      endif #VX_VERSION
      ifeq ($(TOOL_FAMILY),diab)
        LDFLAGS += -tARMXES:$(VXWORKS_VER)
      else
        ifeq ($(VX_VERSION), vx55)
          LDFLAGS += -mbig-endian
        else
          LDFLAGS += -txscalebe -Wa,-EB
        endif #VX_VERSION
      endif
    endif # $(IX_TARGET) == vxle
  endif # $(IX_TARGET_OS) == vxworks
endif # $(IX_TARGET) == vxsim

CFLAGS += $(IX_CFLAGS)
IX_OSAL_MK_TARGET_ENDIAN := $(IX_TARGET)
include $(OSAL_SRCDIR)/platforms/$(IX_TGT_DEVICE)/os/$(IX_TARGET_OS)/make/OsalConfig.mk
CFLAGS += ${OSAL_INCLUDES} ${OSAL_CFLAGS}
LDFLAGS += $(IX_LDFLAGS)


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

FILTER_DEPS = sed -e $(SAFE_QUOTE)s,$(notdir $*)\.o:,$(OBJ_DIR)/$*.d $(OBJ_DIR)/$*.o: src/$(dir $*)component.mk Makefile,g$(SAFE_QUOTE)

else

FILTER_DEPS = sed -e $(SAFE_QUOTE)s,$(notdir $*)\.o,$(OBJ_DIR)/$*.d $(OBJ_DIR)/$*.o,g$(SAFE_QUOTE) \
			-e $(SAFE_QUOTE)$$s,$$, src/$(dir $*)component.mk Makefile,$(SAFE_QUOTE)

endif



# A command for printing trace messages in the makefile.
MAKEFILE_TRACE := @echo $(ECHO_QUOTE)--- Makefile: $(ECHO_QUOTE) # Use this setting to enable messages
#MAKEFILE_TRACE = @\#  # Use this setting to disable messages

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

ifeq ($(IX_HOST_OS),windows)
ifeq ($(VXSHELL),ZSH)
# windows zsh settings
CMD_SEP := &&
ECHO_QUOTE := "# " this comment is here to fix emacs syntax highlighting 
ECHO_BACKSLASH := \# comment needed to keep \ away from end of line
SAFE_QUOTE := "# " this comment is here to fix emacs syntax highlighting 
ECHO_BLANKLINE := echo " "
RM := vxrm 
RMDIR := rm -f -r
TOUCH_EMPTY_FILE := touch
else
# Windows settings
CMD_SEP := &&
ECHO_QUOTE := # cmd.exe's echo command doesn't need quotes
ECHO_BACKSLASH := \# comment needed to keep \ away from end of line
SAFE_QUOTE := "# " this comment is here to fix emacs syntax highlighting 
ECHO_BLANKLINE := @cmd.exe /c echo.
RMDIR := rm -f -r
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
RMDIR := rm -f -r
TOUCH_EMPTY_FILE := touch
endif

################################################################
# Component makefiles
#

# Include component code makefiles
ifneq (0,$(words $(COMPONENTS)))
include $(COMPONENTS:%=src/%/component.mk)
endif
# COMPONENT_OBJ lists all of the object files in components
COMPONENT_OBJ := $(foreach c, $(COMPONENTS), $($(c)_OBJ:%=$(OBJ_DIR)/$(c)/%))

# Include codelet makefiles
ifneq (0,$(words $(CODELETS_COMPONENTS)))
include $(CODELETS_COMPONENTS:%=src/codelets/%/component.mk)
endif
# CODELETS_OBJ lists all the codelet objects
CODELETS_OBJ := $(foreach c, $(CODELETS_COMPONENTS), \
	$(codelets_$(c)_OBJ:%=$(OBJ_DIR)/codelets/$c/%))


# ALL_OBJ list all of the object files we know about
ALL_OBJ := $(COMPONENT_OBJ) $(CODELETS_OBJ)

# ALL_EXPORT_OBJ lists all of the object files that export symbols to
# the Linux kernel symbol table (This has no effect for non-linux
# builds).  Note that this list should be a subset of ALL_OBJ.
ALL_EXPORT_OBJ := $(foreach c, $(ALL_COMPONENTS), \
			$($(c)_EXPORT_OBJ:%=$(OBJ_DIR)/$(subst _,/,$(c))/%) \
		   )


################################################################
# Rules
#

# Default target
usage:
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)xscale_sw/Makefile usage$(ECHO_QUOTE) 
	@echo $(ECHO_QUOTE)------------------------$(ECHO_QUOTE) 
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)Targets:$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)--------$(ECHO_QUOTE)
ifeq ($(IX_TARGET_OS),vxworks)
	@echo $(ECHO_QUOTE)make vxWorks COMP=(component)$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  builds a vxWorks OS image containing component code and $(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  and component dependencies$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)make vxWorks.st COMP=(component)$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  similar to above, but builds a standalone image.$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)make vxWorks-all$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  runs a 'make vxWorks' for each component. Note each image will$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  overwrite the previous one, so this is useful only to check for $(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  build errors.$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)make loadable COMP=(component)$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  builds a relocatable .out image of component code, test code$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  and component test dependencies$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)make loadable-all$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  builds all possible loadable objects.$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)make lib COMP=(component)$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  builds a library of component code$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
endif
ifeq ($(IX_TARGET_OS),linux)
	@echo $(ECHO_QUOTE)make module COMP=(component)$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  builds a linux kernel module of component code$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)make ixp400.o$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  builds a linux kernel module containing the access drivers$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)make modules$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  builds all of the linux kernel modules$(ECHO_QUOTE)
endif
	@echo $(ECHO_QUOTE)make lib/(armobjs or simsoobjs)/(component)/(filename).o$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  compiles a single source file from src/(component)$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)make lib/(armobjs or simsoobjs)/(component)/(filename).lst$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  preprocesses a single source file from src/(component)$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)make all$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  Compiles everything that can be built on the current host OS.$(ECHO_QUOTE)
ifeq ($(IX_TARGET_OS),linux)
	@echo $(ECHO_QUOTE)    For linux this is equivalent to$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)      'make modules'$(ECHO_QUOTE)
endif
ifeq ($(IX_TARGET_OS),vxworks)
	@echo $(ECHO_QUOTE)    For vxWorks it's equivalent to$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)      'make IX_TARGET=vxsim loadable-all'$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)      'make IX_TARGET=vxle loadable-all vxWorks-all'$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)      'make IX_TARGET=vxbe loadable-all vxWorks-all'$(ECHO_QUOTE)
endif
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)Macros / Environment Variables:$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)-------------------------------$(ECHO_QUOTE)
ifeq ($(IX_TARGET_OS),vxworks)
	@echo $(ECHO_QUOTE)WIND_BASE, WIND_HOST_BASE$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  For vxWorks builds : The root of the Tornado installation$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  and the directory containing the \"host\" directory$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
endif
ifeq ($(IX_TARGET_OS),linux)
	@echo $(ECHO_QUOTE)linuxbe_KERNEL_DIR, linuxle_KERNEL_DIR, LINUX_SRC$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  For linux builds : The linux kernel source code directory for big and$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  little-endian builds respectively. If LINUX_SRC is set, it will override$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  both linuxbe_KERNEL_DIR and linuxle_KERNEL_DIR.$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)HARDHAT_BASE$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  For linux builds : the root of the Monta Vista development environment install.$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
endif
	@echo $(ECHO_QUOTE)IX_TARGET$(ECHO_QUOTE)
ifeq ($(IX_TARGET_OS),vxworks)
	@echo $(ECHO_QUOTE)  Set to \"vxbe\" or \"vxle\" to build for XScale, \"vxsim\" for Solaris,$(ECHO_QUOTE)
endif
ifeq ($(IX_TARGET_OS),linux)
	@echo $(ECHO_QUOTE)  \"linuxbe\" or \"linuxle\" for linux targets.$(ECHO_QUOTE)
endif
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)COMP$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  Selects a single component to build. Valid components are:$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  $(strip $(ALL_COMPONENTS))$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
ifeq ($(IX_TARGET_OS),vxworks)
	@echo $(ECHO_QUOTE)IX_INCLUDE_ALL_OBJS$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  Define this to include all access library code, and all code for the selected$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  component and all its dependencies in a vxWorks OS image. Otherwise only$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  object files that are referenced by code in the selected component will$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  be included.$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
endif
	@echo $(ECHO_QUOTE)IX_BYPASS_DEPS$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  Define this to disable the automatic tracking of header file dependencies.$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  This will speed up builds but may not recompile all files affected by an edit.$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)IX_BYPASS_NPE$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  Define this to disable building of NPE microcode.$(ECHO_QUOTE)
ifeq ($(IX_TARGET_OS),vxworks)
	@echo $(ECHO_QUOTE)  This is the default if IX_TARGET is vxsim.$(ECHO_QUOTE)
endif
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)IX_CFLAGS$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  Additional compiler flags.$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)IX_ENSURE_OFF$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  Define this to disable the IX_ENSURE macro$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)IX_PRIVATE_OFF$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  Define this to disable the PRIVATE macro$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)IX_LDFLAGS$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  Additional linker flags.$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)IX_DEBUG$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  Define this to make NDEBUG undefined to enable debugging code.$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)IX_NOOPT$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  Define to disable compiler optimisations$(ECHO_QUOTE)
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)IX_NOSYM$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  Define to omit symbol information from the build.$(ECHO_QUOTE)
ifeq ($(IX_TARGET_OS),linux)
	@echo $(ECHO_QUOTE)  Default for Linux.$(ECHO_QUOTE)
endif
	$(ECHO_BLANKLINE)
	@echo $(ECHO_QUOTE)IX_EXTRA_WARNINGS$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  Define to enable additional compiler warnings (This will cause a lot of$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)  warnings due to OS header files).$(ECHO_QUOTE)
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

ifeq ($(IX_DEVICE),ixp42X)
OBJ_DIR := lib/$(IX_TARGET)
OBJ_DIR_EXIT := ../..
else
OBJ_DIR := $(IX_DEVICE)/lib/$(IX_TARGET)
OBJ_DIR_EXIT := ../../..
endif

OUTPUT_DIR_FLAGS := $(OUTPUT_DIRS:%=%/.dirCreationFlag)

$(OUTPUT_DIR_FLAGS):
	$(MAKEFILE_TRACE) Creating output directory
ifeq ($(IX_HOST_OS),windows)
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
ifdef IX_DEPS
Makefile: $(OUTPUT_DIR_FLAGS)
endif

################################################################
# Template rules to compile C and assembler
#

# This substitution should find the component directory, including a
# trailing '/', corresponding to any source file. Note we use '=', not
# ':=', so $* is evaluated later, when it contains the stem of the
# sourcefile, relative to the src/ directory.
ASM_C_RULES_COMP_DIR = $(patsubst %/test/,%/,$(dir $*))

# This will return the include path flags and values *_CFLAGS macros
# relevant to a file whose stem is in $*.  For test code it returns
# <COMPONENT>_test_CFLAGS and <COMPONENT>_CFLAGS.  For non unit test
# code, it returns <COMPONENT>_CFLAGS twice. Hopefully that doesn't
# become a problem.
ASM_C_RULES_COMP_FOR_FILE_TEMP = $(subst /,_,$(ASM_C_RULES_COMP_DIR))
ASM_C_RULES_COMP_FOR_FILE = $(patsubst %_,%,$(ASM_C_RULES_COMP_FOR_FILE_TEMP))

ASM_C_RULES_CFLAGS_FOR_FILE = -Isrc/$(dir $*) -Isrc/$(ASM_C_RULES_COMP_DIR) \
         $($(subst /,_,$(dir $*))CFLAGS) \
         $($(subst /,_,$(ASM_C_RULES_COMP_DIR))CFLAGS) \
         -DIX_COMPONENT_NAME=ix_$(ASM_C_RULES_COMP_FOR_FILE)


ifeq ($(IX_TARGET_OS),linux)
# Define KBUILD_BASENAME to be the filename
ASM_C_RULES_CFLAGS_FOR_FILE += -DKBUILD_BASENAME=$(basename $(notdir $*))
# Define EXPORT_SYMTAB for any object file listed in ALL_EXPORT_OBJ
ASM_C_RULES_CFLAGS_FOR_FILE += $(patsubst %,-DEXPORT_SYMTAB, \
					$(filter $@,$(ALL_EXPORT_OBJ)) \
				)
endif

# Rule to build object file from c code


$(OBJ_DIR)/%.o: src/%.c
	$(MAKEFILE_TRACE) Building object file $@ from C
ifeq ($(TOOL_FAMILY),diab)
	$(CC) $(CFLAGS) $(DIAB_CFLAGS) \
		$(ASM_C_RULES_CFLAGS_FOR_FILE) $< -c -o $@
else
	$(CC) $(CFLAGS) $(GNU_CFLAGS) \
		$(ASM_C_RULES_CFLAGS_FOR_FILE) $< -c -o $@
endif


# Rule to build object file from assembler
$(OBJ_DIR)/%.o: src/%.s
	$(MAKEFILE_TRACE) Building object file $@ from asm
ifeq ($(TOOL_FAMILY),diab)
	$(CC) $(CFLAGS) $(DIAB_CFLAGS) -Xpreprocess-assembly -W:as:,-Xlabel-colon,-Xsemi-is-newline \
	    $(ASM_C_RULES_CFLAGS_FOR_FILE) 	$< -c -o $@
else
	$(CC) $(CFLAGS)	$(GNU_CFLAGS) -P -x assembler-with-cpp \
		$(ASM_C_RULES_CFLAGS_FOR_FILE) 	$< -c -o $@
endif



# Rule to preprocess C code into a list file
$(OBJ_DIR)/%.lst: src/%.c
	$(MAKEFILE_TRACE) Preprocessing C code to generate $@
ifeq ($(TOOL_FAMILY),diab)
	$(CC) $(CFLAGS) $(DIAB_CFLAGS) \
		$(ASM_C_RULES_CFLAGS_FOR_FILE) $< -E -o $@
else
	$(CC) $(CFLAGS) $(GNU_CFLAGS) \
		$(ASM_C_RULES_CFLAGS_FOR_FILE) $< -E -o $@
endif

################################################################
# Automatic source file dependencies

# $(OUTPUT_DIR_FLAGS) is included as a dependency of these rules
# because they are the first rules processed when 'make' starts,
# unless IX_BYPASS_DEPS is defined, in which case the 'Makefile:' rule is
# first.

# Rule to build dependency info for c
# Note the -DIX_GENERATING_DEPENDENCIES is here to bypass a #error
# directive in the CVS repository version of IxNpeMicrocode.h. See the
# comment in that file for details.
$(OBJ_DIR)/%.d: src/%.c $(OUTPUT_DIR_FLAGS)
	$(MAKEFILE_TRACE) Building dependency info for $@ from C
	$(MAKEFILE_TRACE) Building dependency by defining IX_DEPS=1 in make command 
ifeq ($(TOOL_FAMILY),diab)
	$(CC) $(MAKE_DEP_FLAG) $(CFLAGS) $(DIAB_CFLAGS) $(ASM_C_RULES_CFLAGS_FOR_FILE) -DIX_GENERATING_DEPENDENCIES $< > $@.tmp
else
	$(CC) $(MAKE_DEP_FLAG) $(CFLAGS) $(GNU_CFLAGS) $(ASM_C_RULES_CFLAGS_FOR_FILE) -DIX_GENERATING_DEPENDENCIES $< > $@.tmp
endif
	$(FILTER_DEPS) < $@.tmp > $@
	$(RM) $@.tmp

# Rule to build dependency info for assembler
$(OBJ_DIR)/%.d: src/%.s $(OUTPUT_DIR_FLAGS)
	$(MAKEFILE_TRACE) Building dependency info for $@ from asm
	$(MAKEFILE_TRACE) Building dependency by defining IX_DEPS=1 in make command 
ifeq ($(TOOL_FAMILY),diab)
	$(CC) $(MAKE_DEP_FLAG) $(CFLAGS) $(DIAB_CFLAGS) -Xpreprocess-assembly -W:as:,-Xlabel-colon,-Xsemi-is-newline \
	    $(ASM_C_RULES_CFLAGS_FOR_FILE) $< > $@.tmp
else
	$(CC) $(MAKE_DEP_FLAG) $(CFLAGS) $(GNU_CFLAGS) -x assembler-with-cpp $(ASM_C_RULES_CFLAGS_FOR_FILE) $< > $@.tmp
endif
	$(FILTER_DEPS) < $@.tmp > $@
	$(RM) $@.tmp

# Rules to build dependency info for unit test objects
$(OBJ_DIR)/%.test.d: src/%.c $(OUTPUT_DIR_FLAGS)
	$(MAKEFILE_TRACE) Building dependency info for unit test $@ from C
	$(MAKEFILE_TRACE) Building dependency by defining IX_DEPS=1 in make command
	$(CC) -M $(ASM_C_RULES_UNITTEST_FLAGS) $(ASM_C_RULES_CFLAGS_FOR_FILE) \
                -DIX_GENERATING_DEPENDENCIES $< > $@.tmp
	$(FILTER_DEPS) $(UTEST_FILTER_ARGS) < $@.tmp > $@
	$(RM) $@.tmp

$(OBJ_DIR)/%.test.d: src/%.s $(OUTPUT_DIR_FLAGS)
	$(MAKEFILE_TRACE) Building dependency info for $@ from asm
	$(MAKEFILE_TRACE) Building dependency by defining IX_DEPS=1 in make command
	$(CC) -M $(ASM_C_RULES_UNITTEST_FLAGS) -x assembler-with-cpp \
                $(ASM_C_RULES_CFLAGS_FOR_FILE) $< > $@.tmp
	$(FILTER_DEPS) $(UTEST_FILTER_ARGS) < $@.tmp > $@
	$(RM) $@.tmp

# Include automatic source file dependencies, unless we've been asked
# to skip them.
DEPS := $(ALL_OBJ:.o=.d) 


$(NPE_PRODUCTION_HEADER_OBJ) : $(NPE_PRODUCTION_HEADER)
ifeq ($(TOOL_FAMILY),diab)
	$(CC) $(CFLAGS) $(DIAB_CFLAGS) $< -c -o $@
else
	$(CC) $(CFLAGS) $(GNU_CFLAGS) $< -c -o $@
endif

NPE_CONVERTER := $(IX_XSCALE_SW)/src/npeDl/ixNpeDlImageConverter
NPE_DAT := $(IX_XSCALE_SW)/$(OBJ_DIR)/IxNpeMicrocode.dat
NPE_CONVERTER_SRC := $(IX_XSCALE_SW)/src/npeDl/ixNpeDlImageConverter.c


$(NPE_CONVERTER) : $(NPE_CONVERTER_SRC)
	cd $(IX_XSCALE_SW)/src/npeDl && \
  gcc -fno-builtin -I$(IX_XSCALE_SW)/src/include ixNpeDlImageConverter.c -o $@

$(NPE_DAT) : $(NPE_CONVERTER)
	cd $(IX_XSCALE_SW)/$(OBJ_DIR) && $(NPE_CONVERTER)



################################################################
# Generation of IxNpeMicrocode.dat

ifeq ($(IX_HOST_OS),linux)
Makefile: $(NPE_CONVERTER) $(NPE_DAT)
endif

################################################################
# Rules to check that macros are defined.
# Other targets can depend on these to ensure they have valid
# parameters.

# Making a rule a dependency of the Makefile means it will always be
# executed.
Makefile: assert_ix_target_valid

VX_HW_IX_TARGETS :=  vxle vxbe
VALID_IX_TARGETS :=  $(VX_HW_IX_TARGETS) vxsim linuxbe linuxle

# assert_ix_target_valid - the Makefile itself depends on this, so it's always
# executed.
.PHONY: assert_ix_target_valid
assert_ix_target_valid:
ifeq (,$(filter $(IX_TARGET), $(VALID_IX_TARGETS)))
	@echo $(ECHO_QUOTE)ERROR: You must define IX_TARGET as one of:$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)    $(VALID_IX_TARGETS)$(ECHO_QUOTE)
	@false
endif

#assert_vx_hw_target - for rules that only make sense for hardware vxWorks targets
.PHONY: assert_vx_hw_target
assert_vx_hw_target:
ifeq (,$(filter $(IX_TARGET), $(VX_HW_IX_TARGETS)))
	@echo $(ECHO_QUOTE)ERROR: This rule only works for the IX_TARGET values:$(ECHO_QUOTE)
	@echo $(ECHO_QUOTE)    $(VX_HW_IX_TARGETS)$(ECHO_QUOTE)
	@false
endif


# assert_comp_valid - check that COMP is defined to a valid component name
# used by 'lib', 'loadable', etc.
.PHONY: assert_comp_valid
assert_comp_valid:
ifndef COMP
	@echo $(ECHO_QUOTE)ERROR: You must define COMP for this rule to work$(ECHO_QUOTE)
	@false
endif
ifeq (,$(filter $(COMP), $(ALL_COMPONENTS)))
	@echo $(ECHO_QUOTE)ERROR: \"$(COMP)\" is not a valid component name$(ECHO_QUOTE)
	@false
endif

################################################################
# Macros for per-component rules

# The component directory name for a pattern rule
PATTERN_COMP_DIR = $(subst _,/,$*)
# The component object directory name for a pattern rule
PATTERN_OBJ_DIR = $(OBJ_DIR)/$(PATTERN_COMP_DIR)

################################################################
# Component code library

.PHONY: force
force:

# lib-all - build all libraries
.PHONY: lib-all
lib-all: $(COMPONENTS:%=$(OBJ_DIR)/lib%.a)

# The 'lib' target is a shortcut for specifying the entire library
# name.
.PHONY: lib
lib: $(OBJ_DIR)/lib$(COMP).a

# The list of object files for a lib%.a rule
LIB_OBJ = $($*_OBJ:%=$(PATTERN_OBJ_DIR)/%)

# This rule builds the library. The dependencies are provided by %-component-dependencies.d
$(ALL_COMPONENTS:%=$(OBJ_DIR)/lib%.a):$(OBJ_DIR)/lib%.a:
	$(MAKEFILE_TRACE) Building component library for component $*
	$(AR) rvs $@ $(LIB_OBJ)

# This rule fires if someone tries to build a library not included in the 
# rules above, i.e. one that doesn't correspond to a component. The only
# purpose of this rule is to provide a more informative error message than
# "no rule to make target librubbish.a"
$(OBJ_DIR)/lib%.a : force
	@echo $(ECHO_QUOTE)ERROR: Can't build $@ because \"$*\" is not a valid component name$(ECHO_QUOTE) # ' this comment is here to fix emacs syntax highlighting
	@false

################################################################
# Relocatable object
#
# These rules are similar to the component code library rules
# above. Read the comments on those rules to understand what's
# happening here.

ALL_LOADABLES := $(ALL_COMPONENTS:%=$(OBJ_DIR)/%.out)
.PHONY: loadable-all
loadable-all: $(ALL_LOADABLES)

# The 'loadable' is a shortcut for the full filename
.PHONY: loadable
loadable: $(OBJ_DIR)/$(COMP).out assert_comp_valid


# Full paths to the component and test objects going into the image
LOADABLE_RULE_OBJ = $($*_OBJ:%=$(PATTERN_OBJ_DIR)/%)
# The names of the libraries that will be linked to a loadable
LOADABLE_RULE_LIB_FILES = $($*_test_DEPS:%=$(OBJ_DIR)/lib%.a)

# Rule to build a loadable object
# The dependencies are provided by %-component-dependencies.d
$(ALL_LOADABLES) : $(OBJ_DIR)/%.out : osal_build $(NPE_PRODUCTION_HEADER_OBJ)
	$(MAKEFILE_TRACE) Building relocatable object for component $*
ifeq ($(IX_TARGET_OS),vxworks)
	$(NM) $(LOADABLE_RULE_OBJ) | $(MUNCH) > $(PATTERN_OBJ_DIR)/ctdt.c
ifeq ($(TOOL_FAMILY),diab)
	$(COMPILE_TRADITIONAL) $(CFLAGS) $(DIAB_CFLAGS) $(PATTERN_OBJ_DIR)/ctdt.c \
		-o  $(PATTERN_OBJ_DIR)/ctdt.o
else
	$(COMPILE_TRADITIONAL) $(CFLAGS) $(GNU_CFLAGS) $(PATTERN_OBJ_DIR)/ctdt.c \
		-o  $(PATTERN_OBJ_DIR)/ctdt.o
endif # ifeq ($(TOOL_FAMILY),diab)
	$(LD) $(LDFLAGS) $(LOADABLE_RULE_OBJ) $(PATTERN_OBJ_DIR)/ctdt.o\
	$(NPE_PRODUCTION_HEADER_OBJ) -L$(OBJ_DIR) -L$(OSAL_SRCDIR)/lib/$(IX_TGT_DEVICE)/$(IX_TARGET_OS)/$(IX_TARGET) $($*_test_DEPS:%=-l%) -losal -o $@
ifdef USE_REAL_TEST
	$(RM) rtrtobjlist.lst TP.out TP.o products.h # Left behind by attolcc
endif # ifdef USE_REAL_TEST
else # IX_TARGET_OS == vxworks
	$(LD) $(LDFLAGS)  $(LOADABLE_RULE_OBJ) \
		-L$(OBJ_DIR) $($*_test_DEPS:%=-l%) -o $@
endif # IX_TARGET_OS == vxworks

# Rule for invalid component names.
$(OBJ_DIR)/%.out : force
	@echo $(ECHO_QUOTE)ERROR: Can't build $@ because \"$*\" is not a valid component name$(ECHO_QUOTE) # ' this comment is here to fix emacs syntax highlighting
	@echo $(ECHO_QUOTE)Valid targets are: $(ALL_LOADABLES)$(ECHO_QUOTE)
	@false



################################################################
# Rules to build loadable object modules for components.
# 
# These rules are similar to the component code library rules
# above. Read the comments on those rules to understand what's
# happening here.

# The 'module' rule is a shortcut for the full filename
ifeq ($(IX_LINUXVER),2.6)
module: $(OBJ_DIR)/ixp400_$(COMP).ko
else
module: $(OBJ_DIR)/ixp400_$(COMP).o
endif


# The list of object files for a ixp400_%.o rule
MOD_OBJ = $($*_OBJ:%=$(PATTERN_OBJ_DIR)/%)

# Rule to build the 2.6 module.  Correct dependency info for this rule is \
  provided by %-component-dependencies.d.
$(ALL_COMPONENTS:%=$(OBJ_DIR)/ixp400_%.ko): $(OBJ_DIR)/ixp400_%.ko : $(OBJ_DIR)/ixp400_%.o
	$(MAKEFILE_TRACE) Building kernel 2.6 component object module for component $*
	@echo 'EXTRA_LDFLAGS := --whole-archive' > $(OBJ_DIR)/Makefile
	@echo ' ' >> $(OBJ_DIR)/Makefile
	@echo 'lib-m := $($*_OBJ:%=$(PATTERN_COMP_DIR)/%)' >> $(OBJ_DIR)/Makefile
	@echo ' ' >> $(OBJ_DIR)/Makefile
	@echo 'obj-m := ixp400_$*.o' >> $(OBJ_DIR)/Makefile
	@echo ' ' >> $(OBJ_DIR)/Makefile
	@echo 'ixp400_$*-objs := lib.a' >> $(OBJ_DIR)/Makefile
	@echo ' ' >> $(OBJ_DIR)/Makefile
	@echo 'default:' >> $(OBJ_DIR)/Makefile
	@echo -ne "\t" >> $(OBJ_DIR)/Makefile
	@echo 'make ${IX_KBUILD_CROSSCOMPILE} -C $(LINUX_SRC) M=$(shell pwd)/$(OBJ_DIR) V=1 modules' >> $(OBJ_DIR)/Makefile
	make -C $(OBJ_DIR)

# Rule to build the module.  Correct dependency info for this rule is provided by %-component-dependencies.d.
$(ALL_COMPONENTS:%=$(OBJ_DIR)/ixp400_%.o) : $(OBJ_DIR)/ixp400_%.o :
	$(MAKEFILE_TRACE) Building component object module for component $*
	$(LD) $(LDFLAGS) $(MOD_OBJ)  -o $@

# Rule for invalid component names.
$(OBJ_DIR)/ixp400_%.o : force
	@echo $(ECHO_QUOTE)ERROR: Can't build $@ because \"$*\" is not a valid component name$(ECHO_QUOTE) # ' this comment is here to fix emacs syntax highlighting
	@echo $(ECHO_QUOTE)Valid targets are: $(ALL_LOADABLES)$(ECHO_QUOTE)
	@false

$(OBJ_DIR)/ixp400_%.ko : force
	@echo $(ECHO_QUOTE)ERROR: Can't build $@ because \"$*\" is not a valid component name$(ECHO_QUOTE) # ' this comment is here to fix emacs syntax highlighting
	@echo $(ECHO_QUOTE)Valid targets are: $(ALL_LOADABLES)$(ECHO_QUOTE)
	@false

# The 'modules' target builds every module
ifneq (,$(COMPONENTS))
ALL_MODULES := $(IX_DEVICE_FAMILY) 
endif

TEMP_CODELETS_COMPONENTS := $(CODELETS_COMPONENTS:%=codelets_%)
ifeq ($(IX_LINUXVER),2.6)
ALL_MODULES += $(TEMP_CODELETS_COMPONENTS:%=$(OBJ_DIR)/ixp400_%.ko)
else
ALL_MODULES += $(TEMP_CODELETS_COMPONENTS:%=$(OBJ_DIR)/ixp400_%.o)
endif

modules: $(ALL_MODULES)



################################################################
# Rules to generate memory usage reports
.PHONY: memusage


ifdef COMP
MEMUSAGE_FILES := $($(COMP)_OBJ:%=$(OBJ_DIR)/$(COMP)/%)
MEMUSAGE_REPORT := $(METRICS_DIR)/$(COMP)-$(IX_TARGET)MemUsage.txt
memusage: $(MEMUSAGE_REPORT)

$(MEMUSAGE_REPORT) : $(MEMUSAGE_FILES) assert_comp_valid force
	$(MAKEFILE_TRACE) Generating memory usage report for $(COMP)
	$(OBJDUMP) -h $(MEMUSAGE_FILES) | buildUtils/memusage.pl | tee $@

else # def COMP

MEMUSAGE_REPORT := $(METRICS_DIR)/$(IX_TARGET)MemUsage.txt

ifeq ($(IX_TARGET_OS),vxworks)
MEMUSAGE_ACCESS_FILES := $(COMPONENTS:%=$(OBJ_DIR)/lib%.a)
MEMUSAGE_ACCESS_SINGLE_FILE :=  $(OBJ_DIR)/lib$(IX_DEVICE_TYPE).a
MEMUSAGE_CODELET_FILES := $(CODELETS_COMPONENTS:%=$(OBJ_DIR)/libcodelets_%.a)
else
MEMUSAGE_ACCESS_FILES := $(COMPONENTS:%=$(OBJ_DIR)/$(IX_DEVICE_FAMILY)_%.o)
MEMUSAGE_ACCESS_SINGLE_FILE :=  $(OBJ_DIR)/$(IX_DEVICE_FAMILY).o
MEMUSAGE_CODELET_FILES := $(CODELETS_COMPONENTS:%=$(OBJ_DIR)/$(IX_DEVICE_FAMILY)_codelets_%.o)
endif

# Note: MEMUSAGE_*_FILES macros must _not_ contain a mix of libraries
# and object files.  See documentation in memusage.pl for details on
# why.
memusage: $(MEMUSAGE_REPORT)

$(MEMUSAGE_REPORT) : $(MEMUSAGE_ACCESS_FILES) $(MEMUSAGE_ACCESS_SINGLE_FILE) $(MEMUSAGE_CODELET_FILES) force
	$(MAKEFILE_TRACE) Generating memory usage report
	( echo "Access component memory usage:"; \
	  $(OBJDUMP) -h $(MEMUSAGE_ACCESS_FILES) | buildUtils/memusage.pl; \
	  echo; \
	  $(OBJDUMP) -h $(MEMUSAGE_ACCESS_SINGLE_FILE) | buildUtils/memusage.pl; \
	  echo; \
	  echo "Codelet memory usage:"; \
	  $(OBJDUMP) -h $(MEMUSAGE_CODELET_FILES) | buildUtils/memusage.pl; \
	) | tee $@ 


endif # def COMP


################################################################
# vxWorks OS image
# This rule builds an image containing the objects for component 
# $(COMP), linked against libIxp400.a or libIxp425.a

VXWORKS_COMP_OBJ = $($*_OBJ:%=$(PATTERN_OBJ_DIR)/%)

# The names of all dependencies of $* that are not included in
# libIxp400.a or libIxp425.a and $(COMPONENT_OBJ).
VXWORKS_NON_ACCESS_DRIVER_DEPS = $(filter-out $(COMPONENTS), $($*_test_DEPS))

ifdef IX_INCLUDE_ALL_OBJS
# If we've been asked to include everything, include every access driver 
# object, plus the object of any other dependencies of $(COMP).
# The 'filter-out' function is there to avoid duplicates.
VXWORKS_LIB_EXTRA = $(VXWORKS_COMP_OBJ) \
	$(filter-out $(VXWORKS_COMP_OBJ), $(COMPONENT_OBJ)) \
	$(foreach d, $(VXWORKS_NON_ACCESS_DRIVER_DEPS), \
		$($d_OBJ:%=$(OBJ_DIR)/$(subst _,/,$d)/%) \
	)
else # IX_INCLUDE_ALL_OBJS
# Otherwise, include $*'s object files, plus the access driver lib
# and the libs for any other dependencies.
# The difference between this and the IX_INCLUDE_ALL_OBJS case above is 
# that only objects whose code is referenced in $(COMP) will appear in 
# the final image.

ifeq ($(IX_DEVICE),ixp42X)
VXWORKS_LIB_EXTRA = $(VXWORKS_COMP_OBJ) $(patsubst %, $(OBJ_DIR)/lib%.a, $(VXWORKS_NON_ACCESS_DRIVER_DEPS)) $(OBJ_DIR)/libIxp425.a
else
VXWORKS_LIB_EXTRA = $(VXWORKS_COMP_OBJ) $(patsubst %, $(OBJ_DIR)/lib%.a, $(VXWORKS_NON_ACCESS_DRIVER_DEPS)) $(OBJ_DIR)/libIxp400.a
endif
endif # IX_INCLUDE_ALL_OBJS

# The name of the image file created by the BSP makefile when called by
# one of the OS image rules. Either "vxWorks" or "vxWorks.st"
VXWORKS_IMAGE_NAME =   $(patsubst %vxWorks,vxWorks, \
			 $(patsubst %vxWorks.st,vxWorks.st, $@) \
			)

# The BSP makefile will not rebuild the image if only the value of LIB_EXTRA
# passed to it has changed, so we delete it to always force a rebuild.
define VXWORKS_RM_IMAGE
	$(RM) $(BSP_DIR)/$(VXWORKS_IMAGE_NAME)
endef

# The version of make shipped with Tornado for Windows won't reliably
# accept macro values from the command line, so we need this workaround.
define VXWORKS_CALL_BSP_MK_WIN  
	cd $(BSP_DIR) $(CMD_SEP) \
	set LIB_EXTRA=$(VXWORKS_LIB_EXTRA:%=$(IX_XSCALE_SW)/%) $(CMD_SEP) \
	$(MAKE) $(VXWORKS_IMAGE_NAME) IX_TARGET=$(IX_TARGET) IX_PLATFORM=$(IX_PLATFORM) IX_DEVICE=$(IX_DEVICE) $(CMD_SEP) \
	copy /y $(subst /,\,$(BSP_DIR)/$(VXWORKS_IMAGE_NAME) $(IX_XSCALE_SW)/$@)
endef

define VXWORKS_CALL_BSP_MK_UNIX
	cd $(BSP_DIR) $(CMD_SEP) \
	$(MAKE) $(VXWORKS_IMAGE_NAME) \
      	   LIB_EXTRA="$(VXWORKS_LIB_EXTRA:%=$(IX_XSCALE_SW)/%)" \
	   IX_TARGET=$(IX_TARGET) IX_PLATFORM=$(IX_PLATFORM) IX_DEVICE=$(IX_DEVICE)
	cp $(BSP_DIR)/$(VXWORKS_IMAGE_NAME) $@
endef

ifeq ($(IX_HOST_OS),windows)
ifeq ($(VXSHELL),ZSH)
VXWORKS_CALL_BSP_MAKE =	$(VXWORKS_CALL_BSP_MK_UNIX)
else
VXWORKS_CALL_BSP_MAKE = $(VXWORKS_CALL_BSP_MK_WIN)
endif
else
VXWORKS_CALL_BSP_MAKE = $(VXWORKS_CALL_BSP_MK_UNIX)
endif

# The vxWorks and vxWorks.st rules are shorthand for the full image names
.PHONY: vxWorks vxWorks.st

ifndef COMP
# If COMP is undefined, we build a plain vxWorks image with no xscale_sw code
vxWorks : $(OBJ_DIR)/vxWorks
vxWorks.st : $(OBJ_DIR)/vxWorks.st
else # ndef COMP
# If COMP is set, defer to a different rule to build the image with
# that component's code
vxWorks : assert_comp_valid $(OBJ_DIR)/$(COMP)-vxWorks
vxWorks.st : assert_comp_valid $(OBJ_DIR)/$(COMP)-vxWorks.st
endif # ndef comp

# Rule to build a vxWorks.st image with xscale_sw code
# Correct dependency info for this rule is provided by %-component-dependencies.d.
$(OBJ_DIR)/%-vxWorks.st : assert_vx_hw_target libIxp400 force
	$(MAKEFILE_TRACE) Calling BSP makefile to build OS image for component $*
	$(VXWORKS_RM_IMAGE)
	$(VXWORKS_CALL_BSP_MAKE)
# Rule to build a vxWorks image with xscale_sw code
# Correct dependency info for this rule is provided by %-component-dependencies.d.
$(OBJ_DIR)/%-vxWorks : assert_vx_hw_target libIxp400 force
	$(MAKEFILE_TRACE) Calling BSP makefile to build OS image for component $*
	$(VXWORKS_RM_IMAGE)
	$(VXWORKS_CALL_BSP_MAKE)

# Rule to build a plain vxWorks.st image with no xscale_sw code
$(OBJ_DIR)/vxWorks.st : assert_vx_hw_target libIxp400 force
	$(MAKEFILE_TRACE) Calling BSP makefile to build OS image
	$(VXWORKS_RM_IMAGE)
	$(VXWORKS_CALL_BSP_MAKE)
# Rule to build a plain vxWorks image with no xscale_sw code
$(OBJ_DIR)/vxWorks : assert_vx_hw_target libIxp400 force
	$(MAKEFILE_TRACE) Calling BSP makefile to build OS image
	$(VXWORKS_RM_IMAGE)
	$(VXWORKS_CALL_BSP_MAKE)


# vxWorks-all - build a vxWorks image for all components.
.PHONY: vxWorks-all
vxWorks-all: $(ALL_COMPONENTS:%=$(OBJ_DIR)/%-vxWorks) $(OBJ_DIR)/vxWorks


################################################################
# Dependencies for per-component rules

# It is not possible to specify something like $($*_OBJ) as the
# dependencies of a rule like "lib%.a" rule, so we generate a .d file
# which explicitly states the dependencies for all such rules, and
# "include" it in the makefile.

$(OBJ_DIR)/%-component-dependencies.d: $(OUTPUT_DIR_FLAGS)
	$(MAKEFILE_TRACE) Writing dependency info to $@
# The objects required for the component library
	echo $(OBJ_DIR)/lib$*.a : $(LIB_OBJ) >$@
# Objects and libs for the 'loadable' rule
	echo $(OBJ_DIR)/$*.out : $(LOADABLE_RULE_OBJ) $(LOADABLE_RULE_LIB_FILES) >>$@
# Objects the 'module' rule
	echo $(OBJ_DIR)/$(IX_DEVICE_FAMILY)_$*.o : $(MOD_OBJ) >>$@
# Objects and libs for 'vxWorks' rules
	echo $(OBJ_DIR)/$*-vxWorks $(OBJ_DIR)/$*-vxWorks.st : $(VXWORKS_LIB_EXTRA) >>$@
# Make sure the .d gets rebuilt for makefile changes
	echo $@ : Makefile src/$(PATTERN_COMP_DIR)/component.mk  >>$@

ifndef IX_BYPASS_DEPS
include $(ALL_COMPONENTS:%=$(OBJ_DIR)/%-component-dependencies.d)
endif


################################################################
# Global rules

# all - build everything that can be built on the current host
#
# To allow 'make -j' to work effectively, we have seperate rules for
# the parts of a 'make all' that can run in paralell


ifndef COMP # defining COMP will restrict the 'make all' to one component

ifeq ($(IX_TARGET_OS),vxworks)
ALL_SUB_RULES := all_vxbe all_vxle

ALL_XSCALE_SW_TARGETS := usage loadable-all
ALL_BSP_TARGETS := vxWorks-all bootrom

# The following rules execute builds for each of the IX_TARGET
# values for vxworks
all_vxle all_vxbe : all_% :
	$(MAKE) IX_TARGET=$* IX_PLATFORM=$(IX_PLATFORM) IX_DEVICE=$(IX_DEVICE) $(ALL_XSCALE_SW_TARGETS)
# The BSP makefile targets cannot run in parallel, so we use -j1 to cancel
# any -j flags already specified.
ifeq ($(IX_HOST_OS),windows)
	$(MAKE) IX_TARGET=$* IX_PLATFORM=$(IX_PLATFORM) IX_DEVICE=$(IX_DEVICE) $(ALL_BSP_TARGETS)
else
	$(MAKE) -j1 IX_TARGET=$* IX_PLATFORM=$(IX_PLATFORM) IX_DEVICE=$(IX_DEVICE) $(ALL_BSP_TARGETS)
endif

all_vxsim:
	$(MAKE) IX_TARGET=vxsim IX_PLATFORM=$(IX_PLATFORM) IX_DEVICE=$(IX_DEVICE) $(ALL_XSCALE_SW_TARGETS)

else # IX_TARGET_OS

ALL_SUB_RULES += all_linuxbe

all_linuxle :
	$(MAKE) IX_TARGET=linuxle IX_PLATFORM=$(IX_PLATFORM) IX_DEVICE=$(IX_DEVICE) modules

all_linuxbe :
	$(MAKE) IX_TARGET=linuxbe IX_PLATFORM=$(IX_PLATFORM) IX_DEVICE=$(IX_DEVICE) modules


endif # IX_TARGET_OS

.PHONY: all $(ALL_SUB_RULES)
all: $(ALL_SUB_RULES)

else # COMP is defined

ifeq ($(IX_TARGET_OS),vxworks)
all: force
	$(MAKE) IX_TARGET=vxbe IX_PLATFORM=$(IX_PLATFORM) IX_DEVICE=$(IX_DEVICE) loadable vxWorks
	$(MAKE) IX_TARGET=vxle IX_PLATFORM=$(IX_PLATFORM) IX_DEVICE=$(IX_DEVICE) loadable vxWorks
	$(MAKE) IX_TARGET=vxsim IX_PLATFORM=$(IX_PLATFORM) IX_DEVICE=$(IX_DEVICE) loadable
else # IX_TARGET_OS
all:
	$(MAKE) IX_TARGET=linuxbe IX_PLATFORM=$(IX_PLATFORM) IX_DEVICE=$(IX_DEVICE) modules
	$(MAKE) IX_TARGET=linuxle IX_PLATFORM=$(IX_PLATFORM) IX_DEVICE=$(IX_DEVICE) modules
endif # IX_TARGET_OS

endif # COMP

################################################################
# vxWorks bootrom

.PHONY: bootrom
bootrom: libIxp400
	$(MAKEFILE_TRACE) Calling BSP makefile to build $@
	cd $(BSP_DIR) $(CMD_SEP) $(MAKE) bootrom IX_TARGET=$(IX_TARGET) IX_DEVICE=$(IX_DEVICE)

################################################################
# Library containing all component code

libIxp400 : $(OBJ_DIR)/lib$(IX_DEVICE_TYPE).a $(NPE_PRODUCTION_HEADER_OBJ) $(OSAL_MODULE)

$(OBJ_DIR)/lib$(IX_DEVICE_TYPE).a : $(COMPONENT_OBJ) $(NPE_PRODUCTION_HEADER_OBJ) $(OSAL_MODULE) 
	$(MAKEFILE_TRACE) Building Ixp400 library containing components $(COMPONENTS:%=\"%\")
	$(AR) rvs $(OBJ_DIR)/lib$(IX_DEVICE_TYPE).a $^


################################################################
# Loadable module containing all component code

ifeq ($(IX_LINUXVER),2.6)
$(IX_DEVICE_FAMILY) : $(OBJ_DIR)/$(IX_DEVICE_FAMILY).o
	@echo 'EXTRA_LDFLAGS := --whole-archive' > $(OBJ_DIR)/Makefile
	@echo ' ' >> $(OBJ_DIR)/Makefile
	@echo 'lib-m := $(COMPONENTS:%=$(IX_DEVICE_FAMILY)_%.o) $(OBJ_DIR_EXIT)/$(OSAL_MODULE)'>> $(OBJ_DIR)/Makefile
	@echo ' ' >> $(OBJ_DIR)/Makefile
	@echo 'obj-m := $(IX_DEVICE_FAMILY).o' >> $(OBJ_DIR)/Makefile
	@echo ' ' >> $(OBJ_DIR)/Makefile
	@echo 'ixp400-objs := lib.a' >> $(OBJ_DIR)/Makefile
	@echo ' ' >> $(OBJ_DIR)/Makefile
	@echo 'default:' >> $(OBJ_DIR)/Makefile
	@echo -ne "\t" >> $(OBJ_DIR)/Makefile
	@echo 'make ${IX_KBUILD_CROSSCOMPILE} -C $(LINUX_SRC) M=$(shell pwd)/$(OBJ_DIR) V=1 modules' >> $(OBJ_DIR)/Makefile
	make -C $(OBJ_DIR)
else
$(IX_DEVICE_FAMILY) : $(OBJ_DIR)/$(IX_DEVICE_FAMILY).o
endif

$(OBJ_DIR)/ixp400.o: $(COMPONENTS:%=$(OBJ_DIR)/ixp400_%.o) $(OSAL_MODULE)
ifneq ($(IX_LINUXVER),2.6)
	$(LD) $(LDFLAGS) $^ -o $@
endif



################################################################
# clean - deletes everything that can be rebuilt
.PHONY: clean bsp_clean xscale_sw_clean osal_clean

clean: xscale_sw_clean osal_clean

ifeq ($(IX_TARGET_OS),vxworks)
clean: bsp_clean
endif


osal_clean:
	$(MAKEFILE_TRACE) Deleting all build products in osal
	$(RMDIR) $(OSAL_SRCDIR)/lib/$(IX_TGT_DEVICE)/$(IX_TARGET_OS)/$(IX_TARGET)



################################################################
# showmacro - prints the value of a variable. For makefile debugging.
.PHONY: showmacro
showmacro:
	@echo $($(MACRO))


.PHONY: script
script:
	$(SCRIPT)

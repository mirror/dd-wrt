# Source files in this directory
CSOURCES	= gtDrvConfig.c gtDrvEvents.c gtHwCntl.c
ASOURCES	=

# Include common variable definitions
ifeq ($(OS_RUN),VXWORKS)
include $(TOOL_DIR)\make.defs
endif
ifeq ($(OS_RUN),WIN32)
include $(TOOL_DIR)\makewce.defs
endif
ifeq ($(OS_RUN),LINUX)
include $(TOOL_DIR)/makelnx.defs
endif

# Add in extra stuffs
EXTRA_INCLUDE	+=
EXTRA_DEFINE	+=
ADDED_CFLAGS	+=

# Include common build rules
ifeq ($(OS_RUN),VXWORKS)
include $(TOOL_DIR)\make.rules
endif
ifeq ($(OS_RUN),WIN32)
include $(TOOL_DIR)\makewce.rules
endif
ifeq ($(OS_RUN),LINUX)
include $(TOOL_DIR)/makelnx.rules
endif

# end of file

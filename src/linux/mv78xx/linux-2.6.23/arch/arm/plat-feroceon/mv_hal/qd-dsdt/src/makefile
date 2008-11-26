# makefile - build main object file
#
# modification history
# --------------------
# 04-15-02,mj	created
#
#######################################################################

TARGET = $(PROJ_NAME)

ifeq ($(OS_RUN),VXWORKS)
WORK_TO_DO = $(PROJ_NAME).o

# Include common variable definitions
include $(TOOL_DIR)\make.defs
endif

ifeq ($(OS_RUN),LINUX)
WORK_TO_DO = $(PROJ_NAME).o

# Include common variable definitions
include $(TOOL_DIR)/makelnx.defs
endif

exe : $(WORK_TO_DO)

.PHONY : exe

vpath %.o    $(OBJDIR)
vpath %.lib  $(OBJDIR)

MAINDRV_PATH = $(SRC_BASE_PATH)

OBJECTS_LIST = $(MAINDRV_PATH)/driver/$(OBJDIR)/driver.o \
               $(MAINDRV_PATH)/msapi/$(OBJDIR)/msapi.o \
               $(MAINDRV_PATH)/platform/$(OBJDIR)/platform.o

OBJECTS = $(OBJECTS_LIST)

OBJDIRS	= $(subst /,\,$(dir $(OBJECTS)))
LIBDIRS	= $(subst /,\,$(LIB_DIR))

OBJDIRS_/ = $(subst \,/,$(OBJDIRS))
LIBDIRS_/ = $(subst \,/,$(LIBDIRS))

ifeq ($(OS_RUN),VXWORKS)
$(PROJ_NAME).o : $(OBJECTS) $(MAD_OBJ)
	@ $(ECHO) '------------'
	@ $(ECHO) 'Building $@'
	$(LD) $(LDFLAGS) -Map $(LIB_DIR)/$(PROJ_NAME).map -o $(LIB_DIR)/$(PROJ_NAME).o $(OBJECTS) $(MAD_OBJ)

$(OBJECTS) : FORCE
	@($(CD) $(subst /,\,$(dir $(@D)))) && $(MAKE)

$(MAD_OBJ) : FORCE
	$(MAKE) -C $(MAD_ROOT)/src

endif

ifeq ($(OS_RUN),LINUX)
$(PROJ_NAME).o : $(OBJECTS) $(MAD_OBJ)
	@ $(ECHO) '------------'
	@ $(ECHO) 'Building $@'
	$(LD) $(LDFLAGS) -Map $(LIB_DIR)/$(PROJ_NAME).map -o $(LIB_DIR)/$(PROJ_NAME).o $(OBJECTS) $(MAD_OBJ)

$(OBJECTS) : FORCE
	@cd $(dir $(@D)); $(MAKE)

$(MAD_OBJ) : FORCE
	$(MAKE) -C $(MAD_ROOT)/src

endif

FORCE :

.PHONY : clean
ifeq ($(OS_RUN),VXWORKS)
clean :
	@for %x in ($(OBJDIRS)) do \
	    @($(CD) %x..) && $(MAKE) clean
	- ($(CD) $(LIBDIRS)) && $(RM) $(PROJ_NAME).o
	- ($(CD) $(LIBDIRS)) && $(RM) $(PROJ_NAME).map
	@if exist $(MAD_ROOT) $(MAKE) -C $(MAD_ROOT)/src clean
endif

ifeq ($(OS_RUN),LINUX)
clean :
	@for i in $(OBJDIRS_/); do	\
		cd $$i..; $(MAKE) clean;	\
	done
	@cd $(LIBDIRS_/); $(RM) $(PROJ_NAME).o
	@cd $(LIBDIRS_/); $(RM) $(PROJ_NAME).map
	@if exist $(MAD_ROOT) $(MAKE) -C $(MAD_ROOT)/src clean
endif

#end of file

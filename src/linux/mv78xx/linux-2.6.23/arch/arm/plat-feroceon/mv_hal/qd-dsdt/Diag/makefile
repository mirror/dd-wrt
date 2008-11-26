# makefile - build main object file
#
# modification history
# --------------------
# 04-15-02,mj	created
#
#######################################################################

DEMONAME = msDiag
WORK_TO_DO = $(DEMONAME).o

exe : $(WORK_TO_DO)

# Include common variable definitions
include $(TOOL_DIR)/make.defs

.PHONY : exe

QDLIB = $(LIB_DIR)/$(PROJ_NAME).o

OBJDIRS	= $(subst /,\,$(dir $(QDLIB)))

#CSOURCES = osSem.c ev96122mii.c testApi.c qdSim.c msApiInit.c utils.c vctTest.c
#CSOURCES = osSem.c testApi.c qdSim.c utils.c vctTest.c
CSOURCES = osSem.c testApi.c qdSim.c utils.c vctTest.c msApiMultiInit.c

COBJECTS  	= $(CSOURCES:.c=.o)
AOBJECTS  	= $(ASOURCES:.s=.o)
ifeq ($(OBJECTS),)
OBJECTS  	= $(COBJECTS) $(AOBJECTS)
endif

ifeq ($(TARGET_CPU),ARM)
EXTRA_DEFINE	+= -DFIREFOX
endif

$(DEMONAME).o : $(OBJECTS) $(QDLIB)
	$(LD) $(LDFLAGS) -Map $(DEMONAME).map -o $(DEMONAME).o $(OBJECTS) $(QDLIB)

$(OBJECTS) : %.o : %.c
.c.o :
	$(CC) $(CFLAGS) $(EXTRA_DEFINE) $(EXTRA_INCLUDE) $(ADDED_CFLAGS) -c $< -o $(notdir $@)

.PHONY : clean
clean :
	$(RM) *.o
	$(RM) *.map

#end of file

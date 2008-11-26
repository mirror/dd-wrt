# makefile - build main object file
#
# modification history
# --------------------
# 04-15-02,mj	created
#
#######################################################################

DEMONAME = sample
WORK_TO_DO = $(DEMONAME).o

exe : $(WORK_TO_DO)

# Include common variable definitions
include $(TOOL_DIR)\make.defs

.PHONY : exe

QDLIB = $(LIB_DIR)/$(PROJ_NAME).o

OBJDIRS	= $(subst /,\,$(dir $(QDLIB)))

CSOURCES = FlowControl/flowCtrl.c Initialization/osSem.c Initialization/ev96122mii.c \
           Initialization/qdSim.c Initialization/msApiInit.c  \
	   MACAddress/macAddr.c QoSSetup/qos.c VlanSetup/hgVlan.c Interrupt/qdInt.c \
		   Trailer/trailer.c RMON/rmon.c 802.1Q/802_1q.c Header/header.c \
		   CableTest/cableTest.c PortMonitor/portMonitor.c MultiDevice/msApiInit.c \
			CrossChipTrunk/crossChipTrunk.c MinimizeCPUTraffic\minimizeCPUTraffic.c	\
			PktGen/phyPktGenSample.c PIRL/pirl.c PIRL/pirl2.c ptp/ptp.c

EXTRA_INCLUDE = -I./Include -I./Initialization
COBJECTS  	= $(CSOURCES:.c=.o)
AOBJECTS  	= $(ASOURCES:.s=.o)
ifeq ($(OBJECTS),)
OBJECTS  	= $(COBJECTS) $(AOBJECTS)
endif

OBJECTS_MS= $(subst /,\,$(OBJECTS))

$(DEMONAME).o : $(OBJECTS) $(QDLIB)
	$(LD) $(LDFLAGS) -Map $(DEMONAME).map -o $(DEMONAME).o $(OBJECTS) $(QDLIB)

$(OBJECTS) : %.o : %.c
.c.o :
	$(CC) $(CFLAGS) $(EXTRA_DEFINE) $(EXTRA_INCLUDE) $(ADDED_CFLAGS) -c $< -o $@

.PHONY : clean
clean :
	@for %x in ($(OBJECTS_MS)) do \
		$(RM) %x
	$(RM) $(DEMONAME).*

#end of file

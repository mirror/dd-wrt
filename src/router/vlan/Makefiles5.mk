
LDLIBS =  

SWAPIDIR = ../../switch/sys
SWLIBDIR = ../../switch/swlib
SWMODDIR = ../../switch/swmod
#CCFLAGS += -g -D_GNU_SOURCE -I$(SWAPIDIR) -I$(SWMODDIR) -I../../linux/linux/include -I../../include
CCFLAGS +=  -I$(SWAPIDIR) -I$(SWMODDIR) -I../../linux/linux/include -I../../include

ifneq ($(wildcard $(TOP)/../switch/swmod/swmod.c),)
LIBNAME = -L$(SWLIBDIR) -lsw -ldl
else
LIBNAME = -L$(SWLIBDIR)/$(PLATFORM) -lsw -ldl
endif
LIBNAME += -L$(TOP)/nvram  -lnvram -L$(TOP)/shared -lshared

LDLIBS += $(LIBNAME)


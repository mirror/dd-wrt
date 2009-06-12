#
# Makefile for the Linux device driver for eCos USB-ethernet
# devices. This makefile has been cloned from the one in
# /usr/src/linux-2.2.16/drivers/usb
#

# This makefile will chain to the Linux makefile if appropriate.
# The toplevel Linux makefile sets the variable KERNELRELEASE
ifeq (,$(KERNELRELEASE))

default:
	make -C /usr/src/linux SUBDIRS=$(shell pwd) modules

modules: default

modules_install:
	@echo Support for installing this module not yet implemented.

else

# A recursive invocation of this makefile from the Linux one.

# Build this driver as a module.
CONFIG_USB_ECOS_USBETH	= m

# Subdirs.

SUB_DIRS	:=
MOD_SUB_DIRS	:= $(SUB_DIRS)
MOD_IN_SUB_DIRS	:= $(SUB_DIRS)
ALL_SUB_DIRS	:= $(SUB_DIRS)

# The target object and module list name.

O_TARGET	:=
M_OBJS		:=
O_OBJS		:=
MOD_LIST_NAME	:= ECOS_USBETH_MODULE

# Objects that export symbols.

export-objs		:=

# Multipart objects.

list-multi		:= 

# Optional parts of multipart objects.

# Object file lists.

obj-y	:=
obj-m	:=
obj-n	:=
obj-	:=

# Object files in subdirectories

# Each configuration option enables a list of files.

obj-$(CONFIG_USB_ECOS_USBETH)	+= ecos_usbeth.o

# Extract lists of the multi-part drivers.
# The 'int-*' lists are the intermediate files used to build the multi's.

multi-y		:= $(filter $(list-multi), $(obj-y))
multi-m		:= $(filter $(list-multi), $(obj-m))
int-y		:= $(sort $(foreach m, $(multi-y), $($(basename $(m))-objs)))
int-m		:= $(sort $(foreach m, $(multi-m), $($(basename $(m))-objs)))

# Files that are both resident and modular: remove from modular.

obj-m		:= $(filter-out $(obj-y), $(obj-m))
int-m		:= $(filter-out $(int-y), $(int-m))

# Take multi-part drivers out of obj-y and put components in.

obj-y		:= $(filter-out $(list-multi), $(obj-y)) $(int-y)

# Translate to Rules.make lists.

O_OBJS		:= $(sort $(filter-out $(export-objs), $(obj-y)))
OX_OBJS		:= $(sort $(filter     $(export-objs), $(obj-y)))
M_OBJS		:= $(sort $(filter-out $(export-objs), $(obj-m)))
MX_OBJS		:= $(sort $(filter     $(export-objs), $(obj-m)))
MI_OBJS		:= $(sort $(filter-out $(export-objs), $(int-m)))
MIX_OBJS	:= $(sort $(filter     $(export-objs), $(int-m)))

# The global Rules.make.

include $(TOPDIR)/Rules.make

# Link rules for multi-part drivers.

endif

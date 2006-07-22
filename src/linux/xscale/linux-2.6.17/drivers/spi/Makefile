#
# Makefile for the kernel spi bus driver.
#

# Init order: core, chardev, bit adapters, pcf adapters

ifeq ($(CONFIG_SPI_DEBUG_CORE),y)
EXTRA_CFLAGS += -DDEBUG
endif
 
obj-$(CONFIG_SPI)         += spi-core.o
objs-spi-core             := spi-core.o helpers.o
obj-$(CONFIG_SPI_CHARDEV) += spi-dev.o
obj-y                     += busses/ chips/ algos/ 


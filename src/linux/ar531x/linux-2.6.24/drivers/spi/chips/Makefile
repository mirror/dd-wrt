#
# Makefile for the kernel hardware sensors chip drivers.
#

EXTRA_CFLAGS	:= -Idrivers/misc

ifeq ($(CONFIG_SPI_DEBUG_CHIP),y)
EXTRA_CFLAGS += -DDEBUG
endif

obj-$(CONFIG_SPI_KS8995M) += ks8995m.o


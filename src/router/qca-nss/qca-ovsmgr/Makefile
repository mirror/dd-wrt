# Makefile for the OVS manager

ifeq ($(SoC),$(filter $(SoC),ipq807x ipq807x_64 ipq60xx ipq60xx_64))
ccflags-y += -DOVSMGR_PPE_SUPPORT
endif

ccflags-y := -I$(obj) -I$(obj)/..
ccflags-y += -DOVSMGR_DEBUG_LEVEL=0
ccflags-y += -Werror -Wall

obj-m += qca-ovsmgr.o
qca-ovsmgr-objs := ovsmgr.o ovsmgr_dp.o ovsmgr_debugfs.o

obj ?= .

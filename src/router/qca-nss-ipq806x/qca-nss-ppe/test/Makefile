# Makefile for PPE test module

ccflags-y += -I$(obj)/../drv/exports
ccflags-y += -DPPE_DRV_BUILD_ID="$(BUILD_ID)"
ccflags-y += -Wall -Werror

obj-m += qca-nss-ppe-mirror-test.o
qca-nss-ppe-mirror-test-objs += ppe_mirror_test.o

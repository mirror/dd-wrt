
TOOL_PATH:=/xfs/toolchains/H-i686-pc-linux-gnulibc2.2/bin
OUTPUT_DIR:=images/



####################################################################################
TARGET:=ixdp425
VERSION:=current
TOPDIR:=${shell pwd}
BUILD_DIR:=$(TOPDIR)/build
ECOS_REPOSITORY:=$(TOPDIR)/packages
ECOSCFG_PATH:=$(TOPDIR)/bin
BYTESWAP:=$(TOPDIR)/tools/byteswap
PLATFORM_DIR:=$(ECOS_REPOSITORY)/hal/arm/xscale/$(TARGET)/current
PATH:=$(PATH):$(strip $(TOOL_PATH))
export TOPDIR BUILD_DIR PATH ECOS_REPOSITORY

32M_SIZE:=0x02000000
64M_SIZE:=0x04000000
128M_SIZE:=0x08000000
256M_SIZE:=0x10000000


all: help



output:
	@if [ ! -d $(OUTPUT_DIR) ]; then \
		mkdir -p $(OUTPUT_DIR); \
	fi;

$(BYTESWAP):
	$(MAKE) -r -C tools

tree_%:
	@if [ ! -d $(BUILD_DIR).$* ]; then \
		mkdir -p $(BUILD_DIR).$*; \
	fi; \
	cd $(BUILD_DIR).$*; \
	$(ECOSCFG_PATH)/ecosconfig new $(TARGET)_npe redboot; \
	$(ECOSCFG_PATH)/ecosconfig import $(ECOS_REPOSITORY)/hal/arm/xscale/$(TARGET)/current/misc/redboot_ROM.ecm.$*; \
	$(ECOSCFG_PATH)/ecosconfig tree; \


setup_%:
	@cd $(PLATFORM_DIR)/include; \
	rm -f ixdp425.h; \
	ln -s ixdp425.h.$* ixdp425.h; \
	cd $(PLATFORM_DIR)/include/pkgconf; \
	rm -f mlt_arm_xscale_ixdp425_rom.h mlt_arm_xscale_ixdp425_rom.ldi mlt_arm_xscale_ixdp425_rom.mlt; \
	ln -s mlt_arm_xscale_ixdp425_rom.h.$* mlt_arm_xscale_ixdp425_rom.h; \
	ln -s mlt_arm_xscale_ixdp425_rom.mlt.$* mlt_arm_xscale_ixdp425_rom.mlt; \
	ln -s mlt_arm_xscale_ixdp425_rom.ldi.$* mlt_arm_xscale_ixdp425_rom.ldi; \


build_%:
	@cd $(BUILD_DIR).$*; \
	export PATH=$(PATH); \
	$(MAKE)


32: setup_32 tree_32 build_32 output $(BYTESWAP)
	$(BYTESWAP) -o $(OUTPUT_DIR)/rb-32.bin 2 $(BUILD_DIR).32/install/bin/redboot.bin

64: setup_64 tree_64 build_64 output $(BYTESWAP)
	$(BYTESWAP) -o $(OUTPUT_DIR)/rb-64.bin 2 $(BUILD_DIR).64/install/bin/redboot.bin

128: setup_128 tree_128 build_128 output $(BYTESWAP)
	$(BYTESWAP) -o $(OUTPUT_DIR)/rb-128.bin 2 $(BUILD_DIR).128/install/bin/redboot.bin

256: setup_256 tree_256 build_256 output $(BYTESWAP)
	$(BYTESWAP) -o $(OUTPUT_DIR)/rb-256.bin 2 $(BUILD_DIR).256/install/bin/redboot.bin





world: 32 64 128 256




clean_%:
	@if [ -d $(BUILD_DIR).$* -a \( "$(BUILD_DIR).$*" != "/" -a "$(BUILD_DIR).$*" != "//" \) ]; then \
		echo "Build directory $(BUILD_DIR).$* does exist and is not /. Removing"; \
		rm -rf $(BUILD_DIR).$*; \
	fi;


clean: clean_32 clean_64 clean_128 clean_256




help:
	@echo  ''
	@echo  ' clean       : remove build directory'
	@echo  ''
	@echo  ' world       : build 32M, 64M, 128M and 256M RedBoot ROM images in one run'
	@echo  ''
	@echo  ' 32          : build 32MByte RedBoot ROM image'
	@echo  ' 64          : build 64MByte RedBoot ROM image'
	@echo  ' 128         : build 128MByte RedBoot ROM image'
	@echo  ' 256         : build 256MByte RedBoot ROM image'
	@echo  ''
	@echo  ''





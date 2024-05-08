include ../.config
export TOOL_PATH=$(TOOLCHAIN)/bin
#export TOOL_PATH=/xfs/toolchains/toolchain-arm_cortex-a15+neon-vfpv4_gcc-6.1.0_musl-1.1.15_eabi/bin
export SYS_PATH=$(LINUXDIR)
export TOOLPREFIX=$(ARCH)-linux-uclibc-
export KVER=$(KERNEL_VERSION)
export CFLAGS=$(COPTS) $(LTO)  -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF -Wno-misleading-identation -Wno-incompatible-pointer-types -Wno-implicit-function-declaration -Wno-return-mismatch 


include ./config

ifndef PRJ_PATH
  PRJ_PATH=$(shell pwd)
endif
export PRJ_PATH

include ./make/config.mk
include ./make/tools.mk
include ./make/$(OS)_opt.mk

SUB_DIR=$(patsubst %/, %, $(dir $(wildcard src/*/Makefile)))
SUB_LIB=$(subst src/, , $(SUB_DIR))

all: $(BIN_DIR) uslib shell
	@echo "---Build [SSDK_SH-$(VERSION)] at $(BUILD_DATE) finished."

kslib:kslib_o
	$(AR) -r $(LTOPLUGIN) $(BIN_DIR)/$(KS_MOD)_$(RUNMODE).a $(wildcard $(BLD_DIR)/KSLIB/*.o)

kslib_o:
	$(foreach i, $(SUB_LIB), $(MAKE) MODULE_TYPE=KSLIB -C src/$i all || exit 1;)

uslib:uslib_o
	$(AR) -r $(LTOPLUGIN) $(BIN_DIR)/$(US_MOD)_$(RUNMODE).a $(wildcard $(BLD_DIR)/USLIB/*.o)

uslib_o:
	$(foreach i, $(SUB_LIB), $(MAKE) MODULE_TYPE=USLIB -C src/$i all || exit 1;)

shell:uslib shell_o
	$(CP) $(BLD_DIR)/SHELL/$(SHELLOBJ) $(BIN_DIR)/$(SHELLOBJ)
#	$(STRIP) $(BIN_DIR)/$(SHELLOBJ)

shell_o:
	$(foreach i, $(SUB_LIB), $(MAKE) MODULE_TYPE=SHELL -C src/$i all || exit 1;)

$(BIN_DIR):
	$(MKDIR) -p $@

release:
	@cd make; ./release.sh $(VER)

clean:
	$(RM) -f $(BLD_DIR)/KSLIB/*
	$(RM) -f $(BLD_DIR)/USLIB/*
	$(RM) -f $(BLD_DIR)/SHELL/*
	$(RM) -f $(BIN_DIR)/*

install:
	mkdir -p $(INSTALLDIR)/usr/sbin
	cp build/bin/ssdk_sh $(INSTALLDIR)/usr/sbin
	cp ./ssdk_sh_id $(INSTALLDIR)/usr/sbin
	cp tools/* $(INSTALLDIR)/usr/sbin
EXTRA_CFLAGS += -g3 -w
ccflags-y := -g3 -w
ccflags-m := -g3 -w

obj-$(CPTCFG_MAC80211) += mac80211.o

mac80211-y := combine.o debugfs.o debugfs_key.o

CFLAGS_trace.o := -I$(src)


$(obj)/regdb.c: $(src)/../wireless/db.txt $(src)/../wireless/genregdb.awk
	@$(AWK) -f $(src)/../wireless/genregdb.awk < $< > $@

clean-files := regdb.c

#EXTRA_LDFLAGS += -flto -fwhole-program -flto=jobserver -fuse-linker-plugin

#ccflags-y += -flto -fwhole-program -fno-fat-lto-objects -flto=jobserver -fuse-linker-plugin

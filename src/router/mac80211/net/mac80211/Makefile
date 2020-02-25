obj-$(CPTCFG_MAC80211) += mac80211.o
obj-$(CPTCFG_MAC80211_COMPRESS) += mac80211_compress.o

# mac80211 objects
mac80211-y := \
	main.o status.o \
	driver-ops.o \
	sta_info.o \
	wep.o \
	wpa.o \
	scan.o offchannel.o \
	ht.o agg-tx.o agg-rx.o \
	vht.o \
	ibss.o \
	iface.o \
	rate.o \
	michael.o \
	tkip.o \
	aes_ccm.o \
	aes_cmac.o \
	aes_gmac.o \
	aead_api.o \
	fils_aead.o \
	cfg.o \
	ethtool.o \
	rx.o \
	spectmgmt.o \
	tx.o \
	key.o \
	util.o \
	wme.o \
	chan.o \
	trace.o mlme.o \
	tdls.o \
	ocb.o \
	airtime.o

mac80211-$(CPTCFG_MAC80211_DEBUGFS) += tdma/debug.o
mac80211-$(CPTCFG_MAC80211_TDMA) += tdma/tdma_utils.o tdma/tdma_rx.o tdma/tdma_tx.o tdma/mesh_utils.o tdma/tdma_originator.o tdma/tdma.o tdma/tdma_main.o
mac80211-$(CPTCFG_MAC80211_TDMA_MESH) += tdma/tdma_mesh.o tdma/mesh.o

mac80211_compress-y += compress/compr.o compress/lzo1x_compress.o compress/lzo1x_decompress_safe.o compress/LzmaEnc.o compress/LzmaDec.o compress/lz4_compress.o compress/lz4_decompress.o



mac80211-$(CPTCFG_MAC80211_LEDS) += led.o
mac80211-$(CPTCFG_MAC80211_DEBUGFS) += \
	debugfs.o \
	debugfs_sta.o \
	debugfs_netdev.o \
	debugfs_key.o

mac80211-$(CPTCFG_MAC80211_MESH) += \
	mesh.o \
	mesh_pathtbl.o \
	mesh_plink.o \
	mesh_hwmp.o \
	mesh_sync.o \
	mesh_ps.o


mac80211-$(CONFIG_PM) += pm.o

CFLAGS_trace.o := -I$(src)

rc80211_minstrel-y := \
	rc80211_minstrel_ht.o

#rc80211_minstrel-$(CPTCFG_MAC80211_DEBUGFS) += \
#	rc80211_minstrel_debugfs.o \
#	rc80211_minstrel_ht_debugfs.o

mac80211-$(CPTCFG_MAC80211_RC_MINSTREL) += $(rc80211_minstrel-y)

#obj-$(CPTCFG_CFG80211) += ../wireless/cfg80211.o
#obj-$(CPTCFG_LIB80211) += ../wireless/lib80211.o
#obj-$(CPTCFG_LIB80211_CRYPT_WEP) += ../wireless/lib80211_crypt_wep.o
#obj-$(CPTCFG_LIB80211_CRYPT_CCMP) += ../wireless/lib80211_crypt_ccmp.o
#obj-$(CPTCFG_LIB80211_CRYPT_TKIP) += ../wireless/lib80211_crypt_tkip.o

obj-$(CONFIG_WEXT_CORE) += ../wireless/wext-core.o
obj-$(CONFIG_WEXT_PROC) += ../wireless/wext-proc.o
obj-$(CONFIG_WEXT_SPY) += ../wireless/wext-spy.o
obj-$(CONFIG_WEXT_PRIV) += ../wireless/wext-priv.o

mac80211-y += ../wireless/core.o ../wireless/sysfs.o ../wireless/radiotap.o ../wireless/util.o ../wireless/reg.o ../wireless/scan.o ../wireless/nl80211.o
mac80211-y += ../wireless/mlme.o ../wireless/ibss.o ../wireless/sme.o ../wireless/chan.o ../wireless/ethtool.o ../wireless/mesh.o ../wireless/ap.o ../wireless/trace.o ../wireless/ocb.o
mac80211-$(CONFIG_OF) += ../wireless/of.o
mac80211-$(CPTCFG_CFG80211_DEBUGFS) += ../wireless/debugfs.o
mac80211-$(CPTCFG_CFG80211_WEXT) += ../wireless/wext-compat.o ../wireless/wext-sme.o
mac80211-$(CPTCFG_CFG80211_INTERNAL_REGDB) += regdb.o
mac80211-$(CPTCFG_MAC80211_TDMA) += ../wireless/cfg80211_tdma.o ../wireless/nl80211_tdma.o

$(obj)/regdb.c: $(src)/../wireless/db.txt $(src)/../wireless/genregdb.awk
	@$(AWK) -f $(src)/../wireless/genregdb.awk < $< > $@

clean-files := regdb.c

EXTRA_CFLAGS += -g


ccflags-y += -g

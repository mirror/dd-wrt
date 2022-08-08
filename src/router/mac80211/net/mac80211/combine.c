#include <net/iw_handler.h>

#if IS_ENABLED(CPTCFG_MAC80211_COMPRESS)
#include "compress/compr.c"
#include "compress/lzo1x_compress.c"
#include "compress/lzo1x_decompress_safe.c"
#include "compress/LzmaEnc.c"
#include "compress/LzmaDec.c"
#include "compress/lz4_compress.c"
#include "compress/lz4_decompress.c"
#endif
#include "cfg.c"
#include "main.c"
#include "tx.c"
#include "status.c"
#include "driver-ops.c"
#include "sta_info.c"
#include "wep.c"
#include "wpa.c"
#include "scan.c"
#include "offchannel.c"
#include "ht.c"
#include "agg-tx.c"
#include "agg-rx.c"
#include "vht.c"
#include "he.c"
#include "s1g.c"
#include "ibss.c"
#include "iface.c"
#include "rate.c"
#include "michael.c"
#include "tkip.c"
#include "aes_cmac.c"
#include "aes_gmac.c"
#include "aead_api.c"
#include "fils_aead.c"
#include "ethtool.c"
#include "rx.c"
#include "spectmgmt.c"
#include "key.c"
#include "util.c"
#include "wme.c"
#include "chan.c"
#include "mlme.c"
#include "tdls.c"
#include "ocb.c"
#include "airtime.c"
#ifdef CPTCFG_MAC80211_DEBUGFS
#include "tdma/debug.c"
#endif
#ifdef CPTCFG_MAC80211_TDMA
#include "tdma/tdma_utils.c"
#include "tdma/tdma_rx.c"
#include "tdma/tdma_tx.c"
#include "tdma/mesh_utils.c"
#include "tdma/tdma_originator.c"
#include "tdma/tdma.c"
#include "tdma/tdma_main.c"
#endif
#ifdef CPTCFG_MAC80211_TDMA_MESH
#include "tdma/tdma_mesh.c"
#include "tdma/mesh.c"
#endif

#ifdef CPTCFG_MAC80211_LEDS
#include "led.c"
#endif

#ifdef CPTCFG_MAC80211_DEBUGFS
#include "debugfs.c"
#include "debugfs_sta.c"
#include "debugfs_netdev.c"
#include "debugfs_key.c"
#endif
#ifdef CPTCFG_MAC80211_MESH
#include "mesh.c"
#include "mesh_pathtbl.c"
#include "mesh_plink.c"
#include "mesh_hwmp.c"
#include "mesh_sync.c"
#include "mesh_ps.c"
#endif

#ifdef CONFIG_PM
#include "pm.c"
#endif
#ifdef CPTCFG_MAC80211_RC_MINSTREL
#include "rc80211_minstrel_ht.c"
#include "rc80211_minstrel_ht_debugfs.c"
#endif

#ifdef CPTCFG_LIBIPW_MODULE
#include "../wireless/lib80211.c"
#include "../wireless/lib80211_crypt_wep.c"
#include "../wireless/lib80211_crypt_ccmp.c"
#include "../wireless/lib80211_crypt_tkip.c"
#endif
#ifdef CONFIG_WEXT_CORE
//#include "../wireless/wext-core.c"
#endif
#ifdef CONFIG_WEXT_PROC
//#include "../wireless/wext-proc.c"
#endif
#ifdef CONFIG_WEXT_PRIV
//#include "../wireless/wext-priv.c"
#endif

#include "../wireless/sysfs.c" 
#include "../wireless/core.c" 
#include "../wireless/radiotap.c" 
#include "../wireless/util.c" 
#include "../wireless/reg.c" 
#include "../wireless/scan.c" 
#include "../wireless/nl80211.c" 
#include "../wireless/pmsr.c"
#include "../wireless/mlme.c" 
#include "../wireless/ibss.c" 
#include "../wireless/sme.c" 
#include "../wireless/chan.c" 
#include "../wireless/ethtool.c" 
#include "../wireless/mesh.c" 
#include "../wireless/ap.c" 
#include "../wireless/ocb.c"
#ifdef CONFIG_OF
#include "../wireless/of.c"
#endif
#ifdef CPTCFG_CFG80211_DEBUGFS
#include "../wireless/debugfs.c"
#endif
#ifdef CPTCFG_CFG80211_WEXT
#include "../wireless/wext-compat.c" 
#include "../wireless/wext-sme.c"
#endif
#ifdef CPTCFG_CFG80211_INTERNAL_REGDB
#include "regdb.c"
#endif
#ifdef CPTCFG_MAC80211_TDMA
#include "../wireless/cfg80211_tdma.c" 
#include "../wireless/nl80211_tdma.c"
#endif

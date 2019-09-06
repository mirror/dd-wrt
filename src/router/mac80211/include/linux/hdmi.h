/* Automatically created during backport process */
#ifndef CPTCFG_BPAUTO_BUILD_HDMI
#include_next <linux/hdmi.h>
#else
#undef hdmi_avi_infoframe_init
#define hdmi_avi_infoframe_init LINUX_BACKPORT(hdmi_avi_infoframe_init)
#undef hdmi_avi_infoframe_pack
#define hdmi_avi_infoframe_pack LINUX_BACKPORT(hdmi_avi_infoframe_pack)
#undef hdmi_spd_infoframe_init
#define hdmi_spd_infoframe_init LINUX_BACKPORT(hdmi_spd_infoframe_init)
#undef hdmi_spd_infoframe_pack
#define hdmi_spd_infoframe_pack LINUX_BACKPORT(hdmi_spd_infoframe_pack)
#undef hdmi_audio_infoframe_init
#define hdmi_audio_infoframe_init LINUX_BACKPORT(hdmi_audio_infoframe_init)
#undef hdmi_audio_infoframe_pack
#define hdmi_audio_infoframe_pack LINUX_BACKPORT(hdmi_audio_infoframe_pack)
#undef hdmi_vendor_infoframe_init
#define hdmi_vendor_infoframe_init LINUX_BACKPORT(hdmi_vendor_infoframe_init)
#undef hdmi_vendor_infoframe_pack
#define hdmi_vendor_infoframe_pack LINUX_BACKPORT(hdmi_vendor_infoframe_pack)
#undef hdmi_infoframe_pack
#define hdmi_infoframe_pack LINUX_BACKPORT(hdmi_infoframe_pack)
#undef hdmi_infoframe_log
#define hdmi_infoframe_log LINUX_BACKPORT(hdmi_infoframe_log)
#undef hdmi_infoframe_unpack
#define hdmi_infoframe_unpack LINUX_BACKPORT(hdmi_infoframe_unpack)
#include <linux/backport-hdmi.h>
#endif /* CPTCFG_BPAUTO_BUILD_HDMI */

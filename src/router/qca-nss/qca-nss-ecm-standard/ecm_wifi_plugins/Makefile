# Makefile for ecm wifi plugin module
ccflags-y += -I$(obj)/../exports -I$(obj)/..
ccflags-y += -DECM_WIFI_PLUGIN_BUILD_ID="$(BUILD_ID)"
ccflags-y += -DECM_WIFI_PLUGIN_DEBUG_LEVEL=2
ccflags-$(ECM_CLASSIFIER_MSCS_SCS_ENABLE) += -DECM_CLASSIFIER_MSCS_SCS_ENABLE
ccflags-$(ECM_WIFI_PLUGIN_OPEN_PROFILE_ENABLE) += -DECM_WIFI_PLUGIN_OPEN_PROFILE_ENABLE
ccflags-y += -Wall -Werror
obj-m += ecm-wifi-plugin.o

ecm-wifi-plugin-objs := ecm_wifi_plugin_init.o
ecm-wifi-plugin-objs += ecm_wifi_plugin_emesh.o

ifneq ($(ECM_WIFI_PLUGIN_OPEN_PROFILE_ENABLE), y)
ecm-wifi-plugin-objs += ecm_wifi_plugin_mscs.o
ecm-wifi-plugin-objs += ecm_wifi_plugin_fse.o
endif

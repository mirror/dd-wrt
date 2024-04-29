# Makefile for ecm wifi plugin module
ccflags-y += -I$(obj)/../exports -I$(obj)/..
ccflags-y += -DECM_WIFI_PLUGIN_BUILD_ID="$(BUILD_ID)"
ccflags-y += -DECM_WIFI_PLUGIN_DEBUG_LEVEL=2
ccflags-$(ECM_CLASSIFIER_MSCS_SCS_ENABLE) += -DECM_CLASSIFIER_MSCS_SCS_ENABLE
ccflags-y += -Wall -Werror
obj-m += ecm-wifi-plugin.o
ecm-wifi-plugin-objs := \
                        ecm_wifi_plugin_emesh.o \
                        ecm_wifi_plugin_mscs.o \
                        ecm_wifi_plugin_init.o


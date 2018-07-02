# drv_dsl_cpe_api: Generated to build Linux 2.6.x kernel object
EXTRA_CFLAGS += -DHAVE_CONFIG_H -DLINUX -D__LINUX__ -D__KERNEL__ -DEXPORT_SYMTAB -Idrivers/net/ethernet/ifxatm/ifxos -Idrivers/net/ethernet/ifxatm/include -Idrivers/net/ethernet/ifxatm
ifeq ($(CONFIG_SOC_VR9),y)
EXTRA_CFLAGS += -DCONFIG_VR9
endif
ifeq ($(CONFIG_AR9),y)
EXTRA_CFLAGS += -DCONFIG_AR9
else
ifeq ($(CONFIG_SOC_XWAY),y)
EXTRA_CFLAGS += -DCONFIG_DANUBE
endif
endif


obj-y += mei/
obj-m += drv_dsl_cpe_api.o
drv_dsl_cpe_api-y := common/drv_dsl_cpe_api.o common/drv_dsl_cpe_fifo.o common/drv_dsl_cpe_autoboot.o common/drv_dsl_cpe_debug.o common/drv_dsl_cpe_os_linux.o common/drv_dsl_cpe_timeout.o g997/drv_dsl_cpe_api_g997.o led/drv_dsl_cpe_led.o pm/drv_dsl_cpe_api_pm.o pm/drv_dsl_cpe_pm_core.o ceoc/drv_dsl_cpe_ceoc.o ceoc/drv_dsl_cpe_intern_ceoc.o device/drv_dsl_cpe_device_danube.o g997/drv_dsl_cpe_api_g997_danube.o pm/drv_dsl_cpe_api_pm_danube.o ceoc/drv_dsl_cpe_danube_ceoc.o



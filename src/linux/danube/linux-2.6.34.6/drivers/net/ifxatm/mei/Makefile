obj-m = lantiq_mei.o lantiq_atm.o

lantiq_atm-objs := ifxmips_atm_core.o ifxmips_atm_danube.o
EXTRA_CFLAGS += -DHAVE_CONFIG_H -DLINUX -D__LINUX__ -D__KERNEL__ -DEXPORT_SYMTAB -Idrivers/net/ifxatm/ifxos -Idrivers/net/ifxatm/include -Idrivers/net/ifxatm

obj-$(CONFIG_ATL1C) += atl1c.o

ifdef CONFIG_PPC
atl1c-objs := atl1c/atl1c_main.o atl1c/atl1c_hw.o atl1c/atl1c_ethtool.o
else
atl1c-objs := atl1c_main.o atl1c_hw.o atl1c_ethtool.o
endif
obj-$(CONFIG_MTD_NAND_RALINK)	+= rt_nand.o
obj-$(CONFIG_MTD_SPI_RALINK)	+= rt_spi.o 

#support NOR/NAND/SPI flash
obj-$(CONFIG_MTD_ANY_RALINK)    += rt_any.o
rt_any-objs = ralink_nand.o gdma.o ralink_spi.o

rt_nand-objs = ralink_nand.o gdma.o
rt_spi-objs = ralink_spi.o



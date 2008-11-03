#ifndef _EUREKA_EP430_H
#define _EUREKA_EP430_H


#include <asm/addrspace.h>		/* for KSEG1ADDR() */
#include <asm/byteorder.h>		/* for cpu_to_le32() */
#include <asm/rt2880/rt_mmap.h>

#define EUREKA_EP430_BASE       			RALINK_PCI_BASE
#define EUREKA_EP430_PCI_CONFIG_ADDR 		    	0x20
#define EUREKA_EP430_PCI_CONFIG_DATA_VIRTUAL_REG   	0x24

/*
 * Because of an error/peculiarity in the Galileo chip, we need to swap the
 * bytes when running bigendian.
 */

#define MV_WRITE(ofs, data)  \
        *(volatile u32 *)(EUREKA_EP430_BASE+(ofs)) = cpu_to_le32(data)
#define MV_READ(ofs, data)   \
        *(data) = le32_to_cpu(*(volatile u32 *)(EUREKA_EP430_BASE+(ofs)))
#define MV_READ_DATA(ofs)    \
        le32_to_cpu(*(volatile u32 *)(EUREKA_EP430_BASE+(ofs)))

#define MV_WRITE_16(ofs, data)  \
        *(volatile u16 *)(EUREKA_EP430_BASE+(ofs)) = cpu_to_le16(data)
#define MV_READ_16(ofs, data)   \
        *(data) = le16_to_cpu(*(volatile u16 *)(EUREKA_EP430_BASE+(ofs)))

#define MV_WRITE_8(ofs, data)  \
        *(volatile u8 *)(EUREKA_EP430_BASE+(ofs)) = data
#define MV_READ_8(ofs, data)   \
        *(data) = *(volatile u8 *)(EUREKA_EP430_BASE+(ofs))

#define MV_SET_REG_BITS(ofs,bits) \
	(*((volatile u32 *)(EUREKA_EP430_BASE+(ofs)))) |= ((u32)cpu_to_le32(bits))
#define MV_RESET_REG_BITS(ofs,bits) \
	(*((volatile u32 *)(EUREKA_EP430_BASE+(ofs)))) &= ~((u32)cpu_to_le32(bits))

#define RALINK_PCI_BAR0SETUP_ADDR *(volatile u32 *)(EUREKA_EP430_BASE + 0x0010)
#define RALINK_PCI_BAR1SETUP_ADDR *(volatile u32 *)(EUREKA_EP430_BASE + 0x0014)

#define RALINK_PCI_IMBASEBAR0_ADDR *(volatile u32 *)(EUREKA_EP430_BASE + 0x0018)
#define RALINK_PCI_IMBASEBAR1_ADDR *(volatile u32 *)(EUREKA_EP430_BASE + 0x001C)
#define RT2880_PCI_MEMBASE (*(volatile unsigned long *)(EUREKA_EP430_BASE + 0x0028))
#define RT2880_PCI_IOBASE (*(volatile unsigned long *)(EUREKA_EP430_BASE + 0x002C))
#define RT2880_PCI_ID (*(volatile unsigned long *)(EUREKA_EP430_BASE + 0x0030))
#define RT2880_PCI_CLASS (*(volatile unsigned long *)(EUREKA_EP430_BASE + 0x0034))
#define RT2880_PCI_SUBID (*(volatile unsigned long *)(EUREKA_EP430_BASE + 0x0038))
#define RT2880_PCI_ARBCTL (*(volatile unsigned long *)(EUREKA_EP430_BASE + 0x0080))

/* EUREKA EP430 PCI INIT reference define  */

#define RALINK_PCI_PCICFG_ADDR *(volatile u32 *)(EUREKA_EP430_BASE + 0x0000)
#define RALINK_PCI_PCIRAW_ADDR *(volatile u32 *)(EUREKA_EP430_BASE + 0x0004)
#define RALINK_PCI_PCIINT_ADDR *(volatile u32 *)(EUREKA_EP430_BASE + 0x0008)
#define RALINK_PCI_PCIMSK_ADDR *(volatile u32 *)(EUREKA_EP430_BASE + 0x000C)

#define RT2880_PCI_MEMWIN(offset) (*(volatile unsigned long *)(EUREKA_EP430_BASE + 0x10000 + offset))
#define RT2880_PCI_IOWIN(offset) (*(volatile unsigned long *)(EUREKA_EP430_BASE + 0x20000 + offset))

#define RT2880_BIT(x)              ((1 << x))

#define RALINK_PCI_PCIRAW_PDVDPERR RT2880_BIT(17)
#define RALINK_PCI_PCIRAW_PDVDSERR RT2880_BIT(16)
#define RALINK_PCI_PCIRAW_DETPERR  RT2880_BIT(13)
#define RALINK_PCI_PCIRAW_SIGSERR  RT2880_BIT(12)
#define RALINK_PCI_PCIRAW_RCVMABRT RT2880_BIT(11)
#define RALINK_PCI_PCIRAW_RCVTABRT RT2880_BIT(10)
#define RALINK_PCI_PCIRAW_SIGTABRT RT2880_BIT(9)
#define RALINK_PCI_PCIRAW_MASDPERR RT2880_BIT(8)

#define RALINK_PCI_PCIRAW_FAIL_STATUS 0x00033F00

#endif

#ifndef _ASM_ARCH_CIR_H
#define _ASM_ARCH_CIR_H
#include <linux/ioctl.h>

#define VCR_KEY_POWER		0x613E609F
#define TV1_KEY_POWER		0x40040100
#define TV1_KEY_POWER_EXT	0xBCBD
#define RC5_KER_POWER		0x0CF3

#define VCC_H_ACT_PER		(16-1)
#define VCC_L_ACT_PER		(8-1)
#define VCC_DATA_LEN		(32-1)
#define TV1_H_ACT_PER		(8-1)
#define TV1_L_ACT_PER		(4-1)
#define TV1_DATA_LEN		(48-1)

#define VCC_BAUD		540
#define TV1_BAUD		430
#ifdef  CONFIG_SL3516_ASIC
#define	EXT_CLK			60
#else
#define	EXT_CLK			20
#endif

#define	NEC_PROTOCOL	0x0
#define	RC5_PROTOCOL	0x1
#define VCC_PROTOCOL	0x0
#define TV1_PROTOCOL	0x01

#ifndef	SL2312_CIR_BASE
#define	SL2312_CIR_BASE		0x4C000000
#endif
#define	CIR_BASE_ADDR		IO_ADDRESS(SL2312_CIR_BASE)
#define STORLINK_CIR_ID		0x00010400

#define	CIR_IP_ID		*(volatile unsigned int *)(CIR_BASE_ADDR + 0x00)
#define	CIR_CTR_REG		*(volatile unsigned int *)(CIR_BASE_ADDR + 0x04)
#define	CIR_STATUS_REG		*(volatile unsigned int *)(CIR_BASE_ADDR + 0x08)
#define	CIR_RX_REG		*(volatile unsigned int *)(CIR_BASE_ADDR + 0x0C)
#define	CIR_RX_EXT_REG		*(volatile unsigned int *)(CIR_BASE_ADDR + 0x10)
#define	CIR_PWR_REG		*(volatile unsigned int *)(CIR_BASE_ADDR + 0x14)
#define	CIR_PWR_EXT_REG		*(volatile unsigned int *)(CIR_BASE_ADDR + 0x18)
#define	CIR_TX_CTR_REG		*(volatile unsigned int *)(CIR_BASE_ADDR + 0x1C)
#define	CIR_TX_FEQ_REG		*(volatile unsigned int *)(CIR_BASE_ADDR + 0x20)
#define	CIR_TX_REG		*(volatile unsigned int *)(CIR_BASE_ADDR + 0x24)
#define	CIR_TX_EXT_REG		*(volatile unsigned int *)(CIR_BASE_ADDR + 0x28)


#ifndef	SL2312_POWER_CTRL_BASE
#define	SL2312_POWER_CTRL_BASE		0x4B000000
#endif

#ifndef PWR_BASE_ADDR
#define	PWR_BASE_ADDR		IO_ADDRESS(SL2312_POWER_CTRL_BASE)
#endif
#define	PWR_CTRL_ID		*(unsigned int*)(PWR_BASE_ADDR+0x00)
#define	PWR_CTRL_REG		*(unsigned int*)(PWR_BASE_ADDR+0x04)
#define	PWR_STATUS_REG		*(unsigned int*)(PWR_BASE_ADDR+0x08)


#define BIT(x)			(1<<x)
#define TX_STATUS		BIT(3)

#define	PWR_STAT_CIR		0x10
#define	PWR_STAT_RTC		0x20
#define	PWR_STAT_PUSH		0x40
#define	PWR_SHUTDOWN		0x01

#define CARR_FREQ		38000

struct cir_ioctl_data {
	__u32 data;
};
struct cir_ioctl_data48 {
	__u32 timeout;
	__u32 length;
	__u8  ret;
	__u32 data;
	__u32 data_ext;
};
#define OLD_DATA			0
#define NEW_RECEIVE			1

#define	CIR_IOCTL_BASE		('I'|'R')
#define CIR_SET_BAUDRATE			_IOW (CIR_IOCTL_BASE,  0, struct cir_ioctl_data)
#define CIR_SET_HIGH_PERIOD			_IOW (CIR_IOCTL_BASE,  1, struct cir_ioctl_data)
#define CIR_SET_LOW_PERIOD			_IOW (CIR_IOCTL_BASE,  2, struct cir_ioctl_data)
#define CIR_SET_PROTOCOL			_IOW (CIR_IOCTL_BASE,  3, struct cir_ioctl_data)
#define CIR_SET_ENABLE_COMPARE		_IOW (CIR_IOCTL_BASE,  4, struct cir_ioctl_data)
#define CIR_SET_ENABLE_DEMOD		_IOW (CIR_IOCTL_BASE,  5, struct cir_ioctl_data)
#define CIR_SET_POWER_KEY			_IOW (CIR_IOCTL_BASE,  6, struct cir_ioctl_data)
#define CIR_GET_BAUDRATE			_IOR (CIR_IOCTL_BASE,  7, struct cir_ioctl_data)
#define CIR_GET_HIGH_PERIOD			_IOR (CIR_IOCTL_BASE,  8 ,struct cir_ioctl_data)
#define CIR_GET_LOW_PERIOD			_IOR (CIR_IOCTL_BASE,  9 ,struct cir_ioctl_data)
#define CIR_GET_PROTOCOL			_IOR (CIR_IOCTL_BASE, 10, struct cir_ioctl_data)
#define CIR_GET_ENABLE_COMPARE		_IOR (CIR_IOCTL_BASE, 11, struct cir_ioctl_data)
#define CIR_GET_ENABLE_DEMOD		_IOR (CIR_IOCTL_BASE, 12, struct cir_ioctl_data)
#define CIR_GET_POWER_KEY			_IOR (CIR_IOCTL_BASE, 13, struct cir_ioctl_data)
#define CIR_GET_DATA				_IOWR (CIR_IOCTL_BASE, 14, struct cir_ioctl_data48)
#define CIR_WAIT_INT_DATA			_IOWR (CIR_IOCTL_BASE, 15, struct cir_ioctl_data48)

#endif //_ASM_ARCH_CIR_H

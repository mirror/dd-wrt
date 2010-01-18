/*
 * include/asm-arm/arch-ks8695/serial.h
 */

#include <asm/arch/regs-mem.h>
#include <asm/arch/regs-irq.h>
#include <linux/vsopenrisc.h>

#define VS_TTYS(n, epld_caps)	\
	{ /* ttyS(n) */ \
		baud_base: 	921600, \
		irq: 		KS8695_INTEPLD_UARTS, \
		flags: 		ASYNC_BOOT_AUTOCONF | ASYNC_VSOPENRISC_EPLD, \
		iomem_base:	(u8 *)(VSOPENRISC_VA_EXTIO0_BASE+0x6020+(n-1)*0x40), \
		iomem_reg_shift: 2, \
		io_type:	SERIAL_IO_MEM, \
		epld_capabilities: epld_caps \
	}

#if defined(CONFIG_ARCH_KS8695_VSOPENRISC)
#define STD_SERIAL_PORT_DEFNS \
	VS_TTYS(1, (CAP_EPLD_PORTOFF|CAP_EPLD_RS_ALL)), \
	VS_TTYS(2, (CAP_EPLD_PORTOFF|CAP_EPLD_RS_ALL)), \
	VS_TTYS(3, CAP_EPLD_RS232), \
	VS_TTYS(4, (CAP_EPLD_PORTOFF|CAP_EPLD_RS232|CAP_EPLD_CAN)), \
	VS_TTYS(5, CAP_EPLD_RS232), \
	VS_TTYS(6, CAP_EPLD_RS232), \
	VS_TTYS(7, CAP_EPLD_RS232), \
	VS_TTYS(8, CAP_EPLD_RS232), \
	VS_TTYS(9, CAP_EPLD_RS232), \
	VS_TTYS(10, CAP_EPLD_RS232), \
	VS_TTYS(11, CAP_EPLD_RS232), \
	VS_TTYS(12, CAP_EPLD_RS232), \
	VS_TTYS(13, CAP_EPLD_RS232), \
	VS_TTYS(14, CAP_EPLD_RS232), \
	VS_TTYS(15, CAP_EPLD_RS232), \
	VS_TTYS(16, CAP_EPLD_RS232)
#else
	#define STD_SERIAL_PORT_DEFNS
#endif /* defined(CONFIG_ARCH_KS8695_VSOPENRISC) */


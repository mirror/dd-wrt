#include <linux/mm.h>
#include <linux/init.h>
#include <linux/ioport.h>

#include <asm/io.h>
#include <asm/system.h>
#include <mach/hardware.h>
#include <mach/regs-mem.h>


#define	KS8695_MEM_WRITE(offset, v)	__raw_writel((v), KS8695_MEM_VA + (offset))
#define	KS8695_MEM_READ(offset)		__raw_readl(KS8695_MEM_VA + (offset))

#define	KS8695_GPIO_WRITE(offset, v)	__raw_writel((v), KS8695_GPIO_VA + (offset))
#define	KS8695_GPIO_READ(offset)	__raw_readl(KS8695_GPIO_VA + (offset))


// FIXME: ?!? ueberschreibt z.B. bit 17 ?!? (EXT3)
#define GPIO_CTRL(_gpio_, _crtl_) \
	(uReg = ( uReg & ~(0xf << ((_gpio_)*4) )) \
	 		| ( ((_crtl_)&0xf) << ((_gpio_)*4) ))

void __init ks8695p_configure_extio_interrupt(void)
{
	u32	uReg;

#ifdef	DEBUG_THIS
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif

	uReg = KS8695_GPIO_READ(KS8695_GPIO_CTRL);
	/* EXT0 is used as PCI bus interrupt source */
				/* level detection (active low) */
	/* EXT1 */
	GPIO_CTRL(1, 0x8);	// level triggered (active low)
	/* EXT2 */
	GPIO_CTRL(2, 0x8);	// level triggered (active low)
	/* EXT3 */
	GPIO_CTRL(3, 0x9);	// level triggered (active high)

	KS8695_GPIO_WRITE(KS8695_GPIO_CTRL, uReg);

#ifdef	DEBUG_THIS
	printk(KERN_INFO "%s: OK\n", __FUNCTION__);
#endif
}

#define EXTIO_SET_ADDR(_start_, _size_) \
	( \
	 	( ((_start_) >> 16) << 12) | \
	 	( (((_start_)+(_size_)-1) >> 16) << 22) \
	)

#define XBOUNDS(a,b,c)	(max(a,min(b,c)))
#define EXTIO_SET_TIMG(tacs,tcos,tact,tcoh) \
	( XBOUNDS(0,(tacs)-0,7)	<< 3	\
	| XBOUNDS(0,(tcos)-1,7) << 0	\
	| XBOUNDS(0,(tact)-1,7)	<< 9	\
	| XBOUNDS(0,(tcoh)-1,7)	<< 6	\
	)

void __init ks8695p_extio_init(void)
{
	u32	uReg;

#ifdef	DEBUG_THIS
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif

	/* EXTIO0 has data width word 
	 */
	uReg = KS8695_MEM_READ(KS8695_ERGCON);
	uReg = (uReg & ~(0x3 << 16) ) | (0x3 << 16);
	KS8695_MEM_WRITE(KS8695_ERGCON, uReg);

	/* Enable EXTIO0 and configure timing
	 */
	uReg = EXTIO_SET_ADDR(VSOPENRISC_PA_EXTIO0_BASE, VSOPENRISC_EXTIO0_SIZE);
	/* tacs=32ns tcos=16ns tact=48ns tcoh=16ns
	 */
	//uReg |= (0x4<<3) | (0x1<<0) | (0x5<<9) | (0x1<<6);
	/*  tacs=4*8ns, tcos=2*8ns, tact=7*8ns, tcoh=2*8ns  */
	uReg |= EXTIO_SET_TIMG(4,2,7,2);
	KS8695_MEM_WRITE(KS8695_EXTACON0, uReg);

//	ks8695p_configure_extio_interrupt();
}


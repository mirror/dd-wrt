/****************************************************************************/

/*
 *  linux/include/asm-h8300/ide.h
 *
 *  Copyright (C) 1994-1996  Linus Torvalds & authors
 *  Copyright (C) 2001       Lineo Inc., davidm@snapgear.com
 *  Copyright (C) 2002       Greg Ungerer (gerg@snapgear.com)
 *  Copyright (C) 2002       Yoshinori Sato (ysato@users.sourceforge.jp)
 */

/****************************************************************************/
#ifndef _H8300_IDE_H
#define _H8300_IDE_H
/****************************************************************************/
#ifdef __KERNEL__
/****************************************************************************/

void ide_print_resource(char *name, hw_regs_t *hw);
static inline int ide_default_irq(ide_ioreg_t base) { return 0; };
static inline ide_ioreg_t ide_default_io_base(int index) { return 0; };

static inline void ide_init_hwif_ports(hw_regs_t *hw,	ide_ioreg_t data_port,
 	                               ide_ioreg_t ctrl_port, int *irq)
{
}


static inline void ide_init_default_hwifs(void)
{
}

#define MAX_HWIFS	1

#define __ide_mm_insw(port,addr,count)  do { } while(0)
#define __ide_mm_insl(port,addr,count)  do { } while(0)
#define __ide_mm_outsw(port,addr,count) do { } while(0)
#define __ide_mm_outsl(port,addr,count) do { } while(0)

/****************************************************************************/
#endif /* __KERNEL__ */
#endif /* _H8300_IDE_H */
/****************************************************************************/

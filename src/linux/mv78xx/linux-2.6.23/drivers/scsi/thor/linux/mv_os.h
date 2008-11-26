#if !defined(LINUX_OS_H)
#define LINUX_OS_H

#ifndef LINUX_VERSION_CODE
#include <linux/version.h>
#endif

#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/time.h>
#include <linux/reboot.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <linux/kdev_t.h>
#include <linux/spinlock.h>
#include <linux/pci.h>
#include <linux/random.h>
#include <linux/nmi.h>
#include <linux/completion.h>
#include <linux/blkdev.h>
#include <linux/vmalloc.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/div64.h>

#if defined(__AC_DBG__) && defined(CONFIG_X86)
#include <linux/timex.h>
#include <asm/msr.h>
#endif /* __AC_DBG__ */

#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_transport.h>

/* OS specific flags */
#define _OS_LINUX
#define _64_BIT_COMPILER

#ifdef CONFIG_64BIT
#ifndef _64_SYS_
#define _64_SYS_
#endif
#endif /* CONFIG_64BIT */

#if defined(__LITTLE_ENDIAN)
#define __MV_LITTLE_ENDIAN__
#elif defined(__BIG_ENDIAN)
#define __MV_BIG_ENDIAN__
#else
#error "screwed by endianness"
#endif 

#if defined(__LITTLE_ENDIAN_BITFIELD)
#define __MV_LITTLE_ENDIAN_BITFIELD__
#elif defined(__BIG_ENDIAN_BITFIELD)
#define __MV_BIG_ENDIAN_BITFIELD__
#else
#error "screwed by endianness"
#endif 

#define CPU_TO_LE_16 cpu_to_le16
#define CPU_TO_LE_32 cpu_to_le32
#define LE_TO_CPU_16 le16_to_cpu
#define LE_TO_CPU_32 le32_to_cpu

#ifndef scsi_to_pci_dma_dir
	#define scsi_to_pci_dma_dir(scsi_dir) ((int)(scsi_dir))
#endif

/*
 *
 * Primary Data Type Definition 
 *
 */
#include "com_define.h" 

/* the use of it should be controlled ... carefully */
#ifdef __COLOR_DEBUG__
#include "color_print.h"
#endif /* __COLOR_DEBUG__ */
#ifndef __COLOR_PRINT_H__
#define RED(x)		x
#define GREEN(x)	x
#define BLUE(x)		x
#define MAGENTA(x)	x
#define CYAN(x)		x
#define WHITE(x)	x
#define YELLOW(x)	x
#endif /* __COLOR_PRINT_H__ */


typedef _MV_U64 BUS_ADDRESS;

#define MV_INLINE __inline
#define CDB_INQUIRY_EVPD 1 //TBD

/*
 *
 *	Exposed function and macro
 *
 */

/* System dependent macro for flushing CPU write cache */
#define MV_CPU_WRITE_BUFFER_FLUSH()

/* System dependent little endian from / to CPU conversions */
/*
MV_U64 MV_CPU_TO_LE64(MV_U64 x) { return cpu_to_le64(x); }
MV_U32 MV_CPU_TO_LE32(MV_U32 x) { return cpu_to_le32(x); }
MV_U16 MV_CPU_TO_LE16(MV_U16 x) { return cpu_to_le16(x); }
MV_U64 MV_LE64_TO_CPU(MV_U64 x) { return le64_to_cpu(x); }
MV_U32 MV_LE32_TO_CPU(MV_U32 x) { return le32_to_cpu(x); }
MV_U16 MV_LE16_TO_CPU(MV_U16 x) { return le16_to_cpu(x); }
*/

#define MV_CPU_TO_LE16(x)   cpu_to_le16(x)	
#define MV_CPU_TO_LE32(x)   cpu_to_le32(x)
#define MV_CPU_TO_LE64(x)   cpu_to_le64(x)

#define MV_LE16_TO_CPU(x)   le16_to_cpu(x)
#define MV_LE32_TO_CPU(x)   le32_to_cpu(x)
#define MV_LE64_TO_CPU(x)   le64_to_cpu(x)

#ifdef __MV_DEBUG__
#define MV_DEBUG
#else
#ifdef MV_DEBUG
#undef MV_DEBUG
#endif /* MV_DEBUG */
#endif /* __MV_DEBUG__ */

#ifndef NULL
#define NULL 0
#endif

/* OS macro definition */
#define MV_MAX_TRANSFER_SECTOR  (MV_MAX_TRANSFER_SIZE/512)

/* register read write: memory io */
#define MV_REG_WRITE_BYTE(base, offset, val)    \
    writeb(val, base + offset)
#define MV_REG_WRITE_WORD(base, offset, val)    \
    writew(val, base + offset)
#define MV_REG_WRITE_DWORD(base, offset, val)    \
    writel(val, base + offset)

#define MV_REG_READ_BYTE(base, offset)			\
	readb(base + offset)
#define MV_REG_READ_WORD(base, offset)			\
	readw(base + offset)
#define MV_REG_READ_DWORD(base, offset)			\
	readl(base + offset)

/* register read write: port io */
#define MV_IO_WRITE_BYTE(base, offset, val)    \
    outb(val, (unsigned)(MV_PTR_INTEGER)(base + offset))
#define MV_IO_WRITE_WORD(base, offset, val)    \
    outw(val, (unsigned)(MV_PTR_INTEGER)(base + offset))
#define MV_IO_WRITE_DWORD(base, offset, val)    \
    outl(val, (unsigned)(MV_PTR_INTEGER)(base + offset))

#define MV_IO_READ_BYTE(base, offset)			\
	inb((unsigned)(MV_PTR_INTEGER)(base + offset))
#define MV_IO_READ_WORD(base, offset)			\
	inw((unsigned)(MV_PTR_INTEGER)(base + offset))
#define MV_IO_READ_DWORD(base, offset)			\
	inl((unsigned)(MV_PTR_INTEGER)(base + offset))

#define MV_PCI_READ_CONFIG_DWORD(hba, offset, reg) \
	pci_read_config_dword(((PHBA_Extension)hba)->pcidev, offset, &reg)

#define MV_PCI_WRITE_CONFIG_DWORD(hba, offset, reg) \
	pci_write_config_dword(((PHBA_Extension)hba)->pcidev, offset, reg)

#define MV_PCI_READ_CONFIG_WORD(hba, offset, reg) \
	pci_read_config_word(((PHBA_Extension)hba)->pcidev, offset, &reg)

#define MV_PCI_WRITE_CONFIG_WORD(hba, offset, reg) \
	pci_write_config_word(((PHBA_Extension)hba)->pcidev, offset, reg)

#define MV_LOCK(plock)       spin_lock(plock)
#define MV_UNLOCK(plock)     spin_unlock(plock)

#define MV_LOCK_IRQ(plock)   do { WARN_ON(irqs_disabled()); \
                                    spin_lock_irq(plock); \
                            } while(0)

#define MV_UNLOCK_IRQ(plock) do { \
                                    spin_unlock_irq(plock); \
                            } while(0)

#define MV_LOCK_IRQSAVE(plock, flag)   spin_lock_irqsave(plock, flag)
#define MV_UNLOCK_IRQRESTORE(plock, flag) spin_unlock_irqrestore(plock, flag)

#define MV_DECLARE_TIMER(x) struct timer_list x


/*Driver Version for Command Line Interface Query.*/
#define VER_MAJOR	1

 /* VER_MINOR 1 for RAID, 0 for non-RAID */
#ifdef RAID_DRIVER
#define VER_MINOR        1       
#define VER_BUILD        3
#else  /* RAID_DRIVER */
#define VER_MINOR        0
#define VER_BUILD        9
#endif /* RAID_DRIVER */

/* OEM Account definition */
#define VER_OEM_GENERIC  0
#define VER_OEM_INTEL    1
#define VER_OEM_ASUS     2

#ifdef __OEM_INTEL__
#define VER_OEM          VER_OEM_INTEL
#elif defined(__OEM__ASUS__)
#define VER_OEM          VER_OEM_ASUS
#else
#define VER_OEM          VER_OEM_GENERIC
#endif /* __OEM_INTEL__ */

#define VER_TEST

#define mv_driver_name   "mv61xx"

/* call VER_VAR_TO_STRING */
#define NUM_TO_STRING(num1, num2, num3, num4) #num1"."#num2"."#num3"."#num4
#define VER_VAR_TO_STRING(major, minor, oem, build) NUM_TO_STRING(major, \
								  minor, \
								  oem,   \
								  build)

#define mv_version_linux   VER_VAR_TO_STRING(VER_MAJOR, VER_MINOR,       \
					     VER_OEM, VER_BUILD) VER_TEST

void HBA_kunmap_sg(void* pReq);

#endif /* LINUX_OS_H */

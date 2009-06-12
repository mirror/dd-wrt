/*
 * Use these macros to read/write the IOC.  All it does is perform the actual
 * read/write.
 */

#ifndef IOC_CONTROL

#ifndef __ASSEMBLY__
#define __IOC(offset)	(IOC_BASE + (offset >> 2))
#else
#define __IOC(offset)	offset
#endif

#define IOC_CONTROL	__IOC(0x00)
#define IOC_KARTTX	__IOC(0x04)
#define IOC_KARTRX	__IOC(0x04)

#define IOC_IRQSTATA	__IOC(0x10)
#define IOC_IRQREQA	__IOC(0x14)
#define IOC_IRQCLRA	__IOC(0x14)
#define IOC_IRQMASKA	__IOC(0x18)

#define IOC_IRQSTATB	__IOC(0x20)
#define IOC_IRQREQB	__IOC(0x24)
#define IOC_IRQMASKB	__IOC(0x28)

#define IOC_FIQSTAT	__IOC(0x30)
#define IOC_FIQREQ	__IOC(0x34)
#define IOC_FIQMASK	__IOC(0x38)

#define IOC_T0CNTL	__IOC(0x40)
#define IOC_T0LTCHL	__IOC(0x40)
#define IOC_T0CNTH	__IOC(0x44)
#define IOC_T0LTCHH	__IOC(0x44)
#define IOC_T0GO	__IOC(0x48)
#define IOC_T0LATCH	__IOC(0x4c)

#define IOC_T1CNTL	__IOC(0x50)
#define IOC_T1LTCHL	__IOC(0x50)
#define IOC_T1CNTH	__IOC(0x54)
#define IOC_T1LTCHH	__IOC(0x54)
#define IOC_T1GO	__IOC(0x58)
#define IOC_T1LATCH	__IOC(0x5c)

#define IOC_T2CNTL	__IOC(0x60)
#define IOC_T2LTCHL	__IOC(0x60)
#define IOC_T2CNTH	__IOC(0x64)
#define IOC_T2LTCHH	__IOC(0x64)
#define IOC_T2GO	__IOC(0x68)
#define IOC_T2LATCH	__IOC(0x6c)

#define IOC_T3CNTL	__IOC(0x70)
#define IOC_T3LTCHL	__IOC(0x70)
#define IOC_T3CNTH	__IOC(0x74)
#define IOC_T3LTCHH	__IOC(0x74)
#define IOC_T3GO	__IOC(0x78)
#define IOC_T3LATCH	__IOC(0x7c)

#endif

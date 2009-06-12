#ifndef _PUB_H_
#define _PUB_H_
#include <asm/board/bf535.h>

/*
 * The PUB board uses the ADI21535 Rev0.2 silicon.
 *
 * According to ADI21535 Anomaly List (Dec3, 2002),
 * the polarity of SIC_MASK reg in Rev0.2 silicon
 * is inverted. Value '1' will disable interrupt
 * while '0' will enable interrupt.
 *
 * HuTao, Jun 20 2003, 5:50PM
 */
#undef SIC_MASK_ALL
#define SIC_MASK_ALL	(~0x80000000)

#endif


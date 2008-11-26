/*
 *
 * a work around for the header hell of Odin driver
 * July 6th, 2006
 * A.C. <ake at marvell dot com>
 */

#ifndef __MV_HBA_HEADER_LINUX__
#define  __MV_HBA_HEADER_LINUX__

struct _HBA_Extension;
typedef struct _HBA_Extension HBA_Extension, *PHBA_Extension;

#include "hba_mod.h"
#ifndef SUPPORT_TIMER
#include "hba_timer.h"
#endif
#include "hba_inter.h"

#endif /* __MV_HBA_HEADER_LINUX__ */

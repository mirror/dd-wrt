#ifndef _ACACIA_ECOS_H_
#define _ACACIA_ECOS_H_
#include <cyg/hal/hal_intr.h>

#define DEBUG_PRINTF diag_printf
/*
 * DEBUG switches to control verbosity.
 * Just modify the value of acacia_debug.
 */
#define ACACIA_DEBUG_ALL         0xffffffff
#define ACACIA_DEBUG_ERROR       0x00000001 /* Unusual conditions and Errors */
#define ACACIA_DEBUG_ARRIVE      0x00000002 /* Arrive into a function */
#define ACACIA_DEBUG_LEAVE       0x00000004 /* Leave a function */
#define ACACIA_DEBUG_RESET       0x00000008 /* Reset */
#define ACACIA_DEBUG_TX          0x00000010 /* Transmit */
#define ACACIA_DEBUG_TX_REAP     0x00000020 /* Transmit Descriptor Reaping */
#define ACACIA_DEBUG_RX          0x00000040 /* Receive */
#define ACACIA_DEBUG_RX_STOP     0x00000080 /* Receive Early Stop */
#define ACACIA_DEBUG_INT         0x00000100 /* Interrupts */
#define ACACIA_DEBUG_LINK_CHANGE 0x00000200 /* PHY Link status changed */
#define ACACIA_DEBUG_PHY         0x00000400 /* PHY Debug */
#define ACACIA_DEBUG_INIT        0x00000400 /* Debug initialization. */

extern int acacia_debug ; 

#define ACACIA_PRINT(FLG, X)                        \
{                                                   \
    if (acacia_debug & (FLG)) {                     \
        DEBUG_PRINTF("%s#%d:%s ",                   \
                     __FILE__,                      \
                     __LINE__,                      \
                     __FUNCTION__);                 \
        DEBUG_PRINTF X;                             \
    }                                               \
}

#define ARRIVE() ACACIA_PRINT(ACACIA_DEBUG_ARRIVE, ("Arrive{\n"))
#define LEAVE() ACACIA_PRINT(ACACIA_DEBUG_LEAVE, ("}Leave\n"))

#define FREE(x)

#define A_DATA_CACHE_INVAL(a,l) HAL_DCACHE_INVALIDATE_ALL()

#define sysMsDelay(ms) HAL_DELAY_US(1000*(ms))

#define ERROR -1
#define OK 0

#endif // _ACACIA_ECOS_H_

#ifndef _AG7100_ECOS_
#define _AG7100_ECOS_

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <stdlib.h>
#include <cyg/hal/ar7240_soc.h>
#include <string.h>

//#define DEBUG 1 /* TBDXXX */

//#define DEBUG 0

#define AG7100_TX_BUF_SIZE 1536

#define AG7100_TX_DESC_CNT      9
#define AG7100_RX_DESC_CNT      9

/*
 * IP needs 16 bit alignment. But RX DMA needs 4 bit alignment. We sacrifice IP
 * Plus Reserve extra head room for wmac
 */
#define AG7100_RX_BUF_SIZE           1536

void ag7100_stop_osdev(void *osa);
void ag7100_start_osdev(void *osa);
bool ag7100_is_linkup(void *osa);
#define A_STOP_OSDEV(_osa) ag7100_stop_osdev(_osa)

#define A_START_OSDEV(_osa) ag7100_start_osdev(_osa)

#define A_WAKE_OSDEV(_osa) ag7100_start_osdev(_osa)

#define A_IS_LINK_UP(_osa)    ag7100_is_linkup(_osa) 
#define A_SET_LINK_UP(_osa)   ag7100_start_osdev(_osa)
#define A_SET_LINK_DOWN(_osa) ag7100_stop_osdev(_osa)

#define A_MALLOC(_size) malloc(_size)
#define A_FREE(_ptr)    free(_ptr)

#define A_DESC_DMA_ALLOC(_size, _dma_addr) (*(_dma_addr) = malloc(_size))

#define A_DESC_DMA_FREE(_size, _vaddr, _dma_addr) free(_vaddr)

#define A_DATA_CACHE_INVAL(a,l) HAL_DCACHE_INVALIDATE(a,l)
#define A_DATA_CACHE_FLUSH(a,l) HAL_DCACHE_FLUSH(a,l)

#define A_MDELAY(_intvl) HAL_DELAY_US(1000*(_intvl))
#define A_UDELAY(_intvl) HAL_DELAY_US(_intvl)

#define A_WR_FLUSH() {*(volatile int *)0xa0002000;}

#if DEBUG
#define DEBUG_PRINTF diag_printf
#else
#define DEBUG_PRINTF
#endif
#define PRINTF diag_printf

#define virt_to_bus(vaddr) CYGARC_PHYSICAL_ADDRESS(vaddr)
#define PHYS_TO_K1(physaddr) CYGARC_UNCACHED_ADDRESS(physaddr)
#define intDisable(old) HAL_DISABLE_INTERRUPTS(old)
#define intEnable(old) HAL_RESTORE_INTERRUPTS(old)
#define sysUDelay(us) HAL_DELAY_US(us)
#define sysMsDelay(ms) HAL_DELAY_US(1000*(ms))
#define LOCAL static

#define ag7100_trc(_trc_word)

typedef void* a_dma_addr_t;

#define assert(_cond)   do           {                          \
    if(!(_cond))       {                                        \
        diag_printf("assert %s %d\n", __FUNCTION__, __LINE__);           \
    }                                                           \
}while(0);

#endif /* _AG7100_ECOS_ */

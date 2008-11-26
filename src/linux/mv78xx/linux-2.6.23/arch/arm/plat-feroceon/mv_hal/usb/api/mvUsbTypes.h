/*******************************************************************************

This software file (the "File") is distributed by Marvell International Ltd. 
or its affiliate(s) under the terms of the GNU General Public License Version 2, 
June 1991 (the "License").  You may use, redistribute and/or modify this File 
in accordance with the terms and conditions of the License, a copy of which 
is available along with the File in the license.txt file or by writing to the 
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 
or on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.

(C) Copyright 2004 - 2007 Marvell Semiconductor Israel Ltd. All Rights Reserved.
(C) Copyright 1999 - 2004 Chipidea Microelectronica, S.A. All Rights Reserved.

*******************************************************************************/

#ifndef __mvUsbTypes_h__
#define __mvUsbTypes_h__

#define _PTR_      *
#define _CODE_PTR_ *

typedef char _PTR_                    char_ptr;    /* signed character       */

typedef signed   char  int_8, _PTR_   int_8_ptr;   /* 8-bit signed integer   */
typedef unsigned char  uint_8, _PTR_   uint_8_ptr;  /* 8-bit signed integer   */

typedef          short int_16, _PTR_   int_16_ptr;  /* 16-bit signed integer  */
typedef unsigned short uint_16, _PTR_  uint_16_ptr; /* 16-bit unsigned integer*/

typedef          int   int_32, _PTR_   int_32_ptr;  /* 32-bit signed integer  */
typedef unsigned int   uint_32, _PTR_  uint_32_ptr; /* 32-bit unsigned integer*/

typedef unsigned long  boolean;  /* Machine representation of a boolean */

typedef void _PTR_     pointer;  /* Machine representation of a pointer */

/*--------------------------------------------------------------------------*/
/*
**                          STANDARD CONSTANTS
**
**  Note that if standard 'C' library files are included after types.h,
**  the defines of TRUE, FALSE and NULL may sometimes conflict, as most
**  standard library files do not check for previous definitions.
*/

#ifndef  FALSE
#   define FALSE ((boolean)0)   
#endif

#ifndef  TRUE
#   define TRUE ((boolean)!FALSE) 
#endif

#ifndef  NULL
#   ifdef __cplusplus
#       define NULL (0)
#   else
#       define NULL ((pointer)0)
#   endif
#endif

#ifndef _ASSERT_
   #define ASSERT(X,Y)
#else
   #define ASSERT(X,Y) if(Y) { USB_printf(X); exit(1);}
#endif

#ifndef  MIN
#   define MIN(a,b)   ((a) < (b) ? (a) : (b))      
#endif

#define USB_MEM_ALIGN(n, align)            ((n) + (-(n) & (align-1)))

/* Macro for aligning the EP queue head to 32 byte boundary */
#define USB_MEM32_ALIGN(n)                  USB_MEM_ALIGN(n, 32)

/* Macro for aligning the EP queue head to 1024 byte boundary */
#define USB_MEM1024_ALIGN(n)                USB_MEM_ALIGN(n, 1024)

/* Macro for aligning the EP queue head to 1024 byte boundary */
#define USB_MEM2048_ALIGN(n)                USB_MEM_ALIGN(n, 2048)

#define PSP_CACHE_LINE_SIZE                 32

#define USB_uint_16_low(x)                  ((x) & 0xFF)
#define USB_uint_16_high(x)                 (((x) >> 8) & 0xFF)

#define USB_CACHE_ALIGN(n)                  USB_MEM_ALIGN(n, PSP_CACHE_LINE_SIZE)       

#ifndef INLINE
#   if defined(MV_VXWORKS)
#       define INLINE   __inline
#   else
#       define INLINE   inline
#   endif /* MV_VXWORKS */
#endif /* INLINE */

/* 16bit byte swap. For example 0x1122 -> 0x2211                            */
static INLINE uint_16 USB_BYTE_SWAP_16BIT(uint_16 value)
{
    return ( ((value & 0x00ff) << 8) | 
             ((value & 0xff00) >> 8) );
}

/* 32bit byte swap. For example 0x11223344 -> 0x44332211                    */
static INLINE uint_32 USB_BYTE_SWAP_32BIT(uint_32 value)
{
    return ( ((value & 0x000000ff) << 24) |                      
             ((value & 0x0000ff00) << 8)  |                    
             ((value & 0x00ff0000) >> 8)  |       
             ((value & 0xff000000) >> 24));
}

    
/* Endianess macros.                                                        */
#if defined(MV_CPU_LE)
#   define USB_16BIT_LE(X)  (X) 
#   define USB_32BIT_LE(X)  (X)
#   define USB_16BIT_BE(X)  USB_BYTE_SWAP_16BIT(X)
#   define USB_32BIT_BE(X)  USB_BYTE_SWAP_32BIT(X)
#elif defined(MV_CPU_BE)
#   define USB_16BIT_LE(X)  USB_BYTE_SWAP_16BIT(X) 
#   define USB_32BIT_LE(X)  USB_BYTE_SWAP_32BIT(X)
#   define USB_16BIT_BE(X)  (X)
#   define USB_32BIT_BE(X)  (X)
#else
    #error "CPU endianess isn't defined!\n"
#endif 

typedef struct
{
    void    (*bspPrintf)          (const char *  fmt, ...);
    int     (*bspSprintf)         (char* buffer, const char *  fmt, ...);
    void*   (*bspUncachedMalloc)  (void* pDev, uint_32 size, uint_32 align,
                                   unsigned long* pPhyAddr);     
    void    (*bspUncachedFree)    (void* pDev, uint_32 size, unsigned long phyAddr, 
                                    void*  pVirtAddr);
    void*   (*bspMalloc)          (unsigned int size);
    void    (*bspFree)            (void* ptr);
    void*   (*bspMemset)          (void* ptr, int val, unsigned int size);
    void*   (*bspMemcpy)          (void* dst, const void* src, unsigned int size);
    unsigned long (*bspCacheFlush)      (void* pDev, void* pVirtAddr, int size);
    unsigned long (*bspCacheInv)        (void* pDev, void* pVirtAddr, int size);
    unsigned long (*bspVirtToPhys)    (void* pDev, void* pVirtAddr);
    int     (*bspLock)            (void);
    void    (*bspUnlock)          (int lockKey);
    uint_32 (*bspGetCapRegAddr)   (int devNo);
    void    (*bspResetComplete)   (int devNo);

} USB_IMPORT_FUNCS;

extern USB_IMPORT_FUNCS*            global_import_funcs;

#define USB_sprintf(frmt, x...)     if( (global_import_funcs != NULL) &&                \
                                         global_import_funcs->bspSprintf != NULL)        \
                                        global_import_funcs->bspSprintf(frmt, ##x)

#define USB_printf(frmt, x...)      if( (global_import_funcs != NULL) &&                \
                                        (global_import_funcs->bspPrintf != NULL) )        \
                                        global_import_funcs->bspPrintf(frmt, ##x)


#define USB_virt_to_phys(pVirt)     (global_import_funcs->bspVirtToPhys == NULL) ?      \
                                        (uint_32)(pVirt) : global_import_funcs->bspVirtToPhys(NULL, pVirt)

#define USB_get_cap_reg_addr(dev)   global_import_funcs->bspGetCapRegAddr(dev)

static INLINE void* USB_uncached_memalloc(uint_32 size, uint_32 align, unsigned long* pPhyAddr) 
{
    /*USB_printf("**** USB_uncached_memalloc: size=%d\n", (size));       */
    return global_import_funcs->bspUncachedMalloc(NULL, size, align, pPhyAddr); 
}

static INLINE void* USB_memalloc(uint_32 size)                                                                              
{
    /*USB_printf("**** USB_memalloc: size=%d\n", (size)); */
    return global_import_funcs->bspMalloc(size);        
}

#define USB_uncached_memfree(pVirt, size, physAddr)                                 \
                /*USB_printf("#### USB_uncached_memfree: pVirt=0x%x\n", (pVirt)); */\
                global_import_funcs->bspUncachedFree(NULL, size, physAddr, pVirt);

#define USB_memfree(ptr)                                                            \
                /*USB_printf("#### USB_memfree: ptr=0x%x\n", (ptr));*/              \
                global_import_funcs->bspFree(ptr);

#define USB_memzero(ptr, n)         global_import_funcs->bspMemset(ptr, 0,  n)
#define USB_memcopy(src, dst, n)    global_import_funcs->bspMemcpy(dst, src, n)

#define USB_dcache_inv(ptr, size)   if(global_import_funcs->bspCacheInv != NULL)  \
                                        global_import_funcs->bspCacheInv(NULL, ptr, size)     

#define USB_dcache_flush(ptr, size) if(global_import_funcs->bspCacheFlush != NULL)  \
                                        global_import_funcs->bspCacheFlush(NULL, ptr, size)     

#define USB_lock()                  (global_import_funcs->bspLock == NULL) ?        \
                                                    0 : global_import_funcs->bspLock()

#define USB_unlock(key)             if(global_import_funcs->bspUnlock != NULL)  \
                                        global_import_funcs->bspUnlock(key)     

#define USB_reset_complete(dev)     if(global_import_funcs->bspResetComplete)       \
                                        global_import_funcs->bspResetComplete(dev)


#if defined(USB_UNDERRUN_WA)

#define USB_SRAM_MAX_PARTS  16  

typedef struct
{
    uint_32 (*bspGetSramAddr) (uint_32* pSize);
    void    (*bspIdmaCopy) (void* dst, void* src, unsigned int size);

} USB_WA_FUNCS;

extern USB_WA_FUNCS*    global_wa_funcs;
extern int              global_wa_sram_parts;
extern int              global_wa_threshold; 

#define USB_get_sram_addr(pSize)        global_wa_funcs->bspGetSramAddr(pSize)

#define USB_idma_copy(dst, src, size)                                   \
            if(global_wa_funcs->bspIdmaCopy != NULL)                    \
                global_wa_funcs->bspIdmaCopy(dst, src, size)

#endif /* USB_UNDERRUN_WA */

#endif /* __mvUsbTypes_h__ */

/* EOF */


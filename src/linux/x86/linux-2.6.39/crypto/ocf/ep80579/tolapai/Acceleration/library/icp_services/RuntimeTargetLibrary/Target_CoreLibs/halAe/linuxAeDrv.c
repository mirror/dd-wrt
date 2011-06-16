/**
 **************************************************************************
 * @file linuxAeDrv.c
 *
 * @description
 *      This file provides implementation of driver framework
 *
 * @par 
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 *
 **************************************************************************/ 
 
#include "halAe_platform.h"
#include "core_io.h"
#include "halAeLinux.hxx"
#include "icp_hal_cfg.h"
#include "icp_accel_handle.h"
#include "IxOsal.h"
#include "halMmap.hxx"

MODULE_DESCRIPTION ("ICP HAL Driver");
MODULE_LICENSE("Dual BSD/GPL");

/* maximum number of PCI devices */
#define MAX_DEVICES    32
/* SINT CSR offset */
#define SINT_OFFSET                0xEC
/* SMIA CSR offset */
#define SMIA_OFFSET                0xE8

/* ICP_DEVICE_CONFIG CSR offset */
#define ICP_DEVICE_CONFIG          0x40
/* SWSKU CSR offset */
#define SWSKU                      0x44

/* bit position for AE attention interrupt */
#define AE_ATTN_INT_BIT            (0x1)
/* bit position for AE thread A interrupt */
#define AE_THREAD_A_INT_BIT        (0x1 << 2)
/* bit position for AE thread B interrupt */
#define AE_THREAD_B_INT_BIT        (0x1 << 3)
/* bit positions for all AE interrupts */
#define AE_INT_BITS                (AE_ATTN_INT_BIT | AE_THREAD_A_INT_BIT | AE_THREAD_B_INT_BIT)

/* AE interrupt types */
#define HALAE_INTR_NUM_TYPES (HALAE_INTR_THD_B+1)

/* function return success */
#define SUCCESS             0
/* function return with error */
#define ERROR               -1

/* data structures to support an interrupt "driver" */
struct REQUEST {
    struct REQUEST  *next;
    IxOsalSemaphore  sem;
    unsigned int     type_mask;
    Hal_IntrMasks_T *masks;
    int              status;
};

extern SPINLOCK_T MEMINFO_LOCK;
extern SPINLOCK_T MEMTABLE_LOCK;
extern HalMemMap_T           HalMmapTable;
extern aeDrv_SysMemInfo_T    HalSysMemInfo;    

extern unsigned int MaxAe;
extern unsigned int MaxAeMask;
icp_accel_dev_t              my_instanceHandle;       

extern unsigned int hal_ae_ncdram_base;
extern unsigned int hal_ae_ncdram_size;
extern unsigned int hal_ae_cdram_base;
extern unsigned int hal_ae_cdram_size;
extern unsigned int hal_ae_sram_base;

extern unsigned int ScratchOffset, SramOffset, NCDramOffset, CDramOffset;

unsigned int pci_ae_irq;
static unsigned int irq_registered = 0; 
pci_device_bar_T pci_device_bar;
struct pci_dev * pci_dev_array[MAX_DEVICES];

extern unsigned int rev_id;

extern unsigned int icp_dev_cfg;
extern unsigned char swsku;

static void intr_cleanup(void);
static int pci_device_probe(icp_accel_dev_t * instanceHandle);
static SPINLOCK_T bk_lock;
static SPINLOCK_T tasklet_lock;
static unsigned int    g_summary_mask;
static unsigned int    g_enabled_mask;
static Hal_IntrMasks_T g_intr_masks;
static struct REQUEST *g_requests;
static struct REQUEST *g_available;

int platform_device_probe(icp_accel_dev_t * instanceHandle);
int platform_driver_init(void);
void platform_driver_cleanup(void);

/***************************************************************************
 *    get_request(unsigned int type_mask, Hal_IntrMasks_T *masks)            
 *
 * Description:
 *
 *    Get a REQUEST structure for the purpose of blocking on an 
 *    ISR event. Use a Structure on the g_available queue if available
 *    else allocate using kmalloc
 *
 *    Initialize a semaphore for a wait state
***************************************************************************/
static struct REQUEST *
get_request(unsigned int type_mask, Hal_IntrMasks_T *masks)
{
    struct REQUEST *request;

    if (g_available) 
    {
        request = g_available;
        g_available = request->next;
    } 
    else 
    {
        if(!(request = (struct REQUEST*) ixOsalMemAlloc(sizeof(struct REQUEST)))) 
        {
            return (NULL);
        }
	
        ixOsalMemSet(request, 0, sizeof(struct REQUEST));

        if(ixOsalSemaphoreInit(&request->sem, 1)) 
        {
            ixOsalMemFree(request);
            return (NULL);
        }
        ixOsalSemaphoreWaitInterruptible(&request->sem, IX_OSAL_WAIT_FOREVER);
    }

    request->type_mask = type_mask;
    request->masks = masks;
    request->status = HALAE_SUCCESS;
    return (request);
}

/***************************************************************************
 *    static void free_request(struct REQUEST* request)            
 *
 * Description:
 *
 *    Free a REQUEST structure to the g_available queue
 *
***************************************************************************/
static void 
free_request(struct REQUEST* request)
{
    request->next = g_available;
    g_available = request;
}

/***************************************************************************
 *    static void cleanup_requests(struct REQUEST* *list)            
 *
 * Description:
 *
 *    Free REQUEST structures on the REQUEST queue, defined by list
 *
***************************************************************************/
static void 
cleanup_requests(struct REQUEST* *list)
{
    struct REQUEST *request, *next;

    for (request = *list; request; request = next) 
    {
        next = request->next;
        ixOsalSemaphoreDestroy(&request->sem);
        ixOsalMemFree(request);
    } /* end for request */
    *list = NULL;
}

/***************************************************************************
 *    static void intr_init()            
 *
 * Description:
 *
 *    Initialize GLOBAL queues and status
 *
***************************************************************************/
static void 
intr_init(void)
{
    g_summary_mask = 0;        /* Summary of INTR_TYPEs with ATTN flags set */ 
    g_enabled_mask = 0;        /*  INTR_TYPEs enabled */
    ixOsalMemSet(&g_intr_masks, 0, sizeof(g_intr_masks));
    g_requests = NULL;        /* Outstanding REQUEST queue */
    g_available = NULL;        /* Free REQUEST structure queue */
}

/***************************************************************************
 *    static void intr_cleanup()            
 *
 * Description:
 *
 *    Free REQUEST structures on the global REQUEST queues
 *
***************************************************************************/
static void 
intr_cleanup()
{
    cleanup_requests(&g_available);
    cleanup_requests(&g_requests);
}

/***************************************************************************
 *    int halAe_IntrPoll(unsigned int  type_mask, Hal_IntrMasks_T  *masks)
 *
 * Description:
 *
 *    Return when ATTN flags of type defined by type_mask are set. Flag
 *    of interest are returned in buffer defined by masks
 *
***************************************************************************/
int 
halAe_IntrPoll(unsigned int type_mask, 
               Hal_IntrMasks_T  *masks)
{
    int lock_level;
    unsigned int type_summary_mask;
    struct REQUEST *request;
    int status;

    if ((type_mask == 0) || (type_mask & ~((1 << HALAE_INTR_NUM_TYPES)-1))) 
    {
        return (HALAE_FAIL);
    }
    SPIN_LOCK_IRQSAVE(bk_lock, lock_level);
    {
        /* This test needs to be inside protection to make sure
           that the interrupt is not disabled after we check it and
           before we int Lock */
        if (0 == (type_mask & g_enabled_mask)) 
        {
            SPIN_UNLOCK_IRQRESTORE(bk_lock, lock_level);

            /* this is really saying that the interrupt is disabled */
            return (HALAE_DISABLED);    
        }

        type_summary_mask = (type_mask & g_summary_mask);
        if (type_summary_mask) 
        {
            ixOsalMemSet(masks, 0, sizeof(Hal_IntrMasks_T));
            if (type_summary_mask & HALAE_INTR_ATTN_BKPT_MASK) 
            {
                masks->attn_bkpt_mask = g_intr_masks.attn_bkpt_mask;
                g_intr_masks.attn_bkpt_mask = 0;
            }
            if (type_summary_mask & HALAE_INTR_ATTN_PARITY_MASK) 
            {
                masks->attn_parity_mask = g_intr_masks.attn_parity_mask;
                g_intr_masks.attn_parity_mask = 0;
            }
            if (type_summary_mask & HALAE_INTR_THD_A_MASK) 
            {
                ixOsalMemCopy(masks->thd_a_mask, g_intr_masks.thd_a_mask,
                              sizeof(g_intr_masks.thd_a_mask));
                ixOsalMemSet(g_intr_masks.thd_a_mask, 0,
                             sizeof(g_intr_masks.thd_a_mask));
            }
            if (type_summary_mask & HALAE_INTR_THD_B_MASK) 
            {
                ixOsalMemCopy(masks->thd_b_mask, g_intr_masks.thd_b_mask,
                              sizeof(g_intr_masks.thd_b_mask));
                ixOsalMemSet(g_intr_masks.thd_b_mask, 0,
                             sizeof(g_intr_masks.thd_b_mask));
            }
            g_summary_mask &= ~type_mask;
            SPIN_UNLOCK_IRQRESTORE(bk_lock, lock_level);
            return (HALAE_SUCCESS);
        } 

        if(!(request = get_request(type_mask, masks)))
        {

#ifdef    _DBG_PRINT

           PRINTF("IntrPoll get_request return NULL\n");
        
#endif

           SPIN_UNLOCK_IRQRESTORE(bk_lock, lock_level);
           return (HALAE_FAIL);
        }
        request->next = g_requests;
        g_requests = request;
    }
    SPIN_UNLOCK_IRQRESTORE(bk_lock, lock_level);
        
    ixOsalSemaphoreWaitInterruptible(&request->sem, IX_OSAL_WAIT_FOREVER);
    status = request->status;

#ifdef    _DBG_PRINT

    PRINTF("IntrPoll API UnBlocked with status = %x\n", status);
        
#endif

    SPIN_LOCK_IRQSAVE(bk_lock, lock_level);
    free_request(request);
    SPIN_UNLOCK_IRQRESTORE(bk_lock, lock_level);

    return (status);
}

/***************************************************************************
 *    void handle_attn_intr_icp(unsigned int *p_attn_bkpt_mask, 
                                unsigned int *p_attn_parity_mask);
 * Description:
 *
 *    Process IRQ_AE_ATTN interrupts(called by ISR)
 *    Interrupts are cleared and attn flags are passed to caller via
 *    p_attn_bkpt_mask, p_attn_parity_mask
 *
***************************************************************************/
static void 
handle_attn_intr_icp(unsigned int *p_attn_bkpt_mask, 
                     unsigned int *p_attn_parity_mask)
{
    int ii, ae;
    unsigned int aeAttnMask, ctxEn;
    unsigned int attn_bkpt_mask, attn_parity_mask;

    aeAttnMask = READ_LWORD(Hal_cap_global_ctl_csr_virtAddr + 
                            CAP_RAW_ATTN_STATUS);

    /* see if nothing needs to be done */
    if (aeAttnMask == 0) 
    {
        return;
    }
    attn_bkpt_mask = attn_parity_mask = 0;

    /* see which AE needs attention */
    for (ii=0; ii<MaxAe; ii++) 
    {
        if (!(aeAttnMask & (1 << ii))) 
        {
            continue;
        }
        ae = AE_NUM(ii);
        ctxEn = GET_AE_CSR(ae, CTX_ENABLES);

        if (ctxEn & CE_BREAKPOINT_BIT) 
        {
            attn_bkpt_mask |= (1 << ae);
        }

        if (ctxEn & CE_CNTL_STORE_PARITY_ERROR_BIT) 
        {
            attn_parity_mask |= (1 << ae);
        }

        /* clear interrupt, interrupt bits are W1C */
        SET_AE_CSR(ae, CTX_ENABLES, ctxEn);
        
    } 

    *p_attn_bkpt_mask = attn_bkpt_mask;
    *p_attn_parity_mask = attn_parity_mask;
}

/***************************************************************************
 *    void process_requests()
 *
 * Description:
 *
 *    Wakeup all REQUEST structures whose REQUEST->type_mask is satisfied
 *    by the the INTR types reflected in the global g_summary_mask. The
 *    actual attention flags corresponding the the satisfied requests are
 *    are copied to the REQUEST->masks[] words. The satisfied INTR types
 *    in g_summary_mask are cleared.
 *
 *    The threads associated with the satifisfied REQUEST structures are
 *    unblocked via REQUEST->sem.
 *
***************************************************************************/
/* The following must be called in an ISR or when interrupts are disabled */
static void 
process_requests(void)
{
    struct REQUEST *request, *prev_req, *next_req;
    unsigned int mask;
    int lock_level;
    
    SPIN_LOCK_IRQSAVE(tasklet_lock, lock_level);
    
    for (prev_req = NULL, request = g_requests; request; 
         prev_req = request, request = next_req) 
    {
        next_req = request->next;
        if (0 == (mask = (request->type_mask & g_summary_mask))) 
        {
            continue;
        }
        /* Remove request from list */
        if (prev_req == NULL) 
        {
            g_requests = next_req;
        }    
        else
        {
            prev_req->next = next_req;
        }
	
        ixOsalMemSet(request->masks, 0, sizeof(Hal_IntrMasks_T));

        if (mask & HALAE_INTR_ATTN_BKPT_MASK) 
        {
            request->masks->attn_bkpt_mask = g_intr_masks.attn_bkpt_mask;
            g_intr_masks.attn_bkpt_mask = 0;
        }
        if (mask & HALAE_INTR_ATTN_PARITY_MASK) 
        {
            request->masks->attn_parity_mask = g_intr_masks.attn_parity_mask;
            g_intr_masks.attn_parity_mask = 0;
        }
        if (mask & HALAE_INTR_THD_A_MASK) 
        {
            ixOsalMemCopy(request->masks->thd_a_mask, g_intr_masks.thd_a_mask,
                          sizeof(g_intr_masks.thd_a_mask));
            ixOsalMemSet(g_intr_masks.thd_a_mask, 0,
                         sizeof(g_intr_masks.thd_a_mask));
        }
        if (mask & HALAE_INTR_THD_B_MASK) 
        {
            ixOsalMemCopy(request->masks->thd_b_mask, g_intr_masks.thd_b_mask,
                          sizeof(g_intr_masks.thd_b_mask));
            ixOsalMemSet(g_intr_masks.thd_b_mask, 0,
                         sizeof(g_intr_masks.thd_b_mask));
        }

        ixOsalSemaphorePostWakeup(&request->sem);
    
        /* On loop, don't change prev_req */
        request = prev_req;

        g_summary_mask &= ~(mask);
        if (g_summary_mask == 0) 
        {
            break;
        }    
    } /* end for request */
    
    SPIN_UNLOCK_IRQRESTORE(tasklet_lock, lock_level);
}

/***************************************************************************
 *    void process_disabled_requests()
 *
 * Description:
 *
 *    Wakeup all REQUEST structures whose REQUEST->type_mask is satisfied
 *    by the the INTR types NOT reflected in the global g_enabled_mask. 
 *
 *    The threads associated with the satifisfied REQUEST structures are
 *    unblocked via REQUEST->sem.
 *
***************************************************************************/
/* The following must be called in an ISR or when interrupts are disabled */
static void 
process_disabled_requests(void)
{
    struct REQUEST *request, *prev_req, *next_req;
    int lock_level;
    
    SPIN_LOCK_IRQSAVE(tasklet_lock, lock_level);

    for (prev_req = NULL, request = g_requests; request; 
         prev_req = request, request = next_req) 
    {
        next_req = request->next;

        /* don't remove requests only if all requested interrupts are still
           enabled */
        if (!(request->type_mask & ~g_enabled_mask)) 
        {
            continue;
        }
        /* Remove request from list */
        if (prev_req == NULL) 
        {
            g_requests = next_req;
        }    
        else
        {
            prev_req->next = next_req;
        }
        request->status = HALAE_DISABLED;

#ifdef    _DBG_PRINT

        PRINTF("Waking Disabled IntrPoll API\n");
        
#endif

        ixOsalSemaphorePostWakeup(&request->sem);
	
        /* On loop, don't change prev_req */
        request = prev_req;

    } /* end for request */
    
    SPIN_UNLOCK_IRQRESTORE(tasklet_lock, lock_level);
}

/***************************************************************************
 *    void halAe_Tasklet(unsigned long unused)
 *
 * Description:
 *
 *    tasklet function to process requests.
 *
***************************************************************************/
void 
halAe_Tasklet(unsigned long unused)
{
    process_requests();            
}

DECLARE_TASKLET(hal_tasklet, halAe_Tasklet, 0);

/***************************************************************************
 *    irqreturn_t halAe_Intr_attn_ISR_ICP(int irq, void *dev_id, 
                                          struct pt_regs *regs)
 *
 * Description:
 *
 *    ISR for IRQ_AE_ATTN & IRQ_AE_THD_AB. Calls handle_intr_icp to clear and
 *    return flags.
 *    Updates g_intr_masks and g_summary_mask as per returned ATTN flags
 *
***************************************************************************/
static 
irqreturn_t halAe_Intr_attn_ISR_ICP(int irq, 
                                    void *dev_id)
{
    unsigned int attn_bkpt_mask, attn_parity_mask;
    unsigned int thd_mask_a, summary_bit_a;
    unsigned int thd_mask_b, summary_bit_b;
    uint64 thd_status_a,thd_status_b;
    unsigned int grp, aeMask = MaxAeMask;
    u8 sInt;

    pci_read_config_byte(dev_id, SINT_OFFSET, &sInt);
    /* this ISR only service attention and thread_ab interrups */
    if((sInt & AE_INT_BITS) == 0) 
    {
        return (IRQ_NONE);
    }    

    attn_bkpt_mask = 0;
    attn_parity_mask = 0;
    /* handle attn interrupt */
    handle_attn_intr_icp(&attn_bkpt_mask, &attn_parity_mask);
   
    g_intr_masks.attn_bkpt_mask |= attn_bkpt_mask;
    g_intr_masks.attn_parity_mask |= attn_parity_mask;
    if (0 != g_intr_masks.attn_bkpt_mask) 
    {
        g_summary_mask |= HALAE_INTR_ATTN_BKPT_MASK;
    }    
    if (0 != g_intr_masks.attn_parity_mask) 
    {
        g_summary_mask |= HALAE_INTR_ATTN_PARITY_MASK;
    }    

    /* handle thd_a interrupt */
    summary_bit_a = 0;
    for(grp = 0, aeMask = MaxAeMask; (aeMask & 0xf); grp ++)
    {
        thd_status_a = (unsigned int)(THD_CSR_ADDR(grp, CAP_THD_INTERRUPT_A_OFFSET));
        /* Get interrupt data */
        thd_mask_a = READ_LWORD(thd_status_a);
        if(thd_mask_a) 
        {
            summary_bit_a = HALAE_INTR_THD_A_MASK;        
            /* Clear interrupt */
            WRITE_LWORD(thd_status_a, thd_mask_a);
            g_intr_masks.thd_a_mask[grp] |= thd_mask_a;
        }
        aeMask = aeMask >> 4;
    }
    
    /* handle thd_b interrupt */
    summary_bit_b = 0;    
    for(grp = 0, aeMask = MaxAeMask; (aeMask & 0xf); grp ++)
    {
        thd_status_b = (unsigned int)(THD_CSR_ADDR(grp, CAP_THD_INTERRUPT_B_OFFSET));
        /* Get interrupt data */
        thd_mask_b = READ_LWORD(thd_status_b);
        if(thd_mask_b) 
        {
            summary_bit_b = HALAE_INTR_THD_B_MASK;        
            /* Clear interrupt */
            WRITE_LWORD(thd_status_b, thd_mask_b);
            g_intr_masks.thd_b_mask[grp] |= thd_mask_b;
        }
        aeMask = aeMask >> 4;
    }
      
    g_summary_mask = g_summary_mask | summary_bit_a | summary_bit_b ;

    tasklet_schedule(&hal_tasklet);   

    return (IRQ_HANDLED);
}

/***************************************************************************
 *    int halAe_IntrEnable(unsigned int   type_mask)
 *
 * Description:
 *
 *    Enables the interrupts defined by type_mask
 *
***************************************************************************/
int 
halAe_IntrEnable(unsigned int   type_mask)
{
    int lock_level;
    unsigned int prev_enabled_mask;
    unsigned int addr;
    unsigned int grp, aeMask = MaxAeMask;
    int ret = HALAE_SUCCESS;

    /* all AE interrupts share a interrupt number, so call request_irq just once
       and ISR dispath interrupts
    */
    SPIN_LOCK_IRQSAVE(bk_lock, lock_level);
    {
       /* Don't enable interrupts that are already enabled */
       type_mask &= ~g_enabled_mask;

       prev_enabled_mask = g_enabled_mask;
       g_enabled_mask |= type_mask;

       /* Clear summary mask */
       g_summary_mask &= ~type_mask;

       /* only register ISR once */
       if(!prev_enabled_mask && g_enabled_mask) 
       {
           if(!irq_registered)
           {
               ret = request_irq(pci_ae_irq, &halAe_Intr_attn_ISR_ICP, IRQF_SHARED, "AeDrv", 
                           pci_dev_array[PCI_DEVICE_AE_CLUSTER_INDEX]);
               if(!ret)
               {
                   /* enable_irq(pci_ae_irq); */
                   irq_registered = 1;
                   ret = HALAE_SUCCESS;
               }
               else
               {
                   ret = HALAE_FAIL;
               }	       
           }
       }    

       if(type_mask & HALAE_INTR_ATTN_BKPT_MASK) 
       {
           g_intr_masks.attn_bkpt_mask = 0;

           addr = (unsigned int)(Hal_cap_global_ctl_csr_virtAddr + CAP_ATTN_MASK_SET);
           WRITE_LWORD(addr, AE_ATTN_ALLBITS);
       }
       if(type_mask & HALAE_INTR_ATTN_PARITY_MASK) 
       {
           g_intr_masks.attn_parity_mask = 0;

           addr = (unsigned int)(Hal_cap_global_ctl_csr_virtAddr + CAP_ATTN_MASK_SET);
           WRITE_LWORD(addr, AE_ATTN_ALLBITS);
       }
       if(type_mask & HALAE_INTR_THD_A_MASK) 
       {   
           ixOsalMemSet(g_intr_masks.thd_a_mask, 0,
                        sizeof(g_intr_masks.thd_a_mask));

           for(grp = 0, aeMask = MaxAeMask; (aeMask & 0xf); grp ++)
           {
               SET_THD_CSR(grp, CAP_THD_ENABLE_SET_A_OFFSET, AE_THREAD_A_ALLBITS);
               aeMask = aeMask >> 4;
           }
       }
       if(type_mask & HALAE_INTR_THD_B_MASK) 
       {  
           ixOsalMemSet(g_intr_masks.thd_b_mask, 0,
                        sizeof(g_intr_masks.thd_b_mask));

           for(grp = 0, aeMask = MaxAeMask; (aeMask & 0xf); grp ++)
           {
               SET_THD_CSR(grp, CAP_THD_ENABLE_SET_B_OFFSET, AE_THREAD_B_ALLBITS);
               aeMask = aeMask >> 4;
           }
       }
    }
    SPIN_UNLOCK_IRQRESTORE(bk_lock, lock_level);

    return (ret);
}

/***************************************************************************
 *    int halAe_IntrDisable(unsigned int     type_mask)
 *
 * Description:
 *
 *    Disables the interrupts defined by type_mask
 *
 *    call process_disabled_requests to:
 *
 *    Wakeup all REQUEST structures whose REQUEST->type_mask is satisfied
 *    by the the INTR types NOT reflected in the global g_enabled_mask.
 *
 *    The threads associated with the satifisfied REQUEST structures are
 *    unblocked via REQUEST->sem.
 *
***************************************************************************/
int 
halAe_IntrDisable(unsigned int type_mask)
{
    int lock_level;
    unsigned int addr;
    unsigned int grp, aeMask = MaxAeMask;

    SPIN_LOCK_IRQSAVE(bk_lock, lock_level);
    {
        /* Don't Disable interrupts that are not enabled */
        type_mask &= g_enabled_mask;

        g_enabled_mask &= ~type_mask;
        g_summary_mask &= ~type_mask;

        /* only free irq when all interrupts disabled */
        if (!g_enabled_mask) 
        {
            if(irq_registered == 1) 
            {
                /* disable_irq(pci_ae_irq); */
                free_irq(pci_ae_irq, pci_dev_array[PCI_DEVICE_AE_CLUSTER_INDEX]);
                irq_registered = 0;
            } 
        }

        if (type_mask & HALAE_INTR_ATTN_BKPT_MASK) 
        {
            g_intr_masks.attn_bkpt_mask = 0;

            addr = (unsigned int)(Hal_cap_global_ctl_csr_virtAddr + 
                                  CAP_ATTN_MASK_CLR);
            WRITE_LWORD(addr, AE_ATTN_ALLBITS);
        }
        if (type_mask & HALAE_INTR_ATTN_PARITY_MASK) 
        {
            g_intr_masks.attn_parity_mask = 0;

            addr = (unsigned int)(Hal_cap_global_ctl_csr_virtAddr + 
                                  CAP_ATTN_MASK_CLR);
            WRITE_LWORD(addr, AE_ATTN_ALLBITS);
        }
        if (type_mask & HALAE_INTR_THD_A_MASK) 
        {
            ixOsalMemSet(g_intr_masks.thd_a_mask, 0,
                         sizeof(g_intr_masks.thd_a_mask));

           for(grp = 0, aeMask = MaxAeMask; (aeMask & 0xf); grp ++)
           {
               SET_THD_CSR(grp, CAP_THD_ENABLE_CLR_A_OFFSET, AE_THREAD_A_ALLBITS);
               aeMask = aeMask >> 4;
           }
        }
        if (type_mask & HALAE_INTR_THD_B_MASK) 
        {
            ixOsalMemSet(g_intr_masks.thd_b_mask, 0,
                         sizeof(g_intr_masks.thd_b_mask));

            for(grp = 0, aeMask = MaxAeMask; (aeMask & 0xf); grp ++)
            {
                SET_THD_CSR(grp, CAP_THD_ENABLE_CLR_B_OFFSET, AE_THREAD_B_ALLBITS);
                aeMask = aeMask >> 4;
            }
        }        

        process_disabled_requests();
    }
    SPIN_UNLOCK_IRQRESTORE(bk_lock, lock_level);

    return (HALAE_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_SetMemoryStartOffset
   Description: set memory start offset
   Returns:     HALAE_SUCCESS or error number
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_SetMemoryStartOffset(unsigned int scratchOffset, 
                           unsigned int sramOffset, 
                           unsigned int ncdramOffset, 
                           unsigned int cdramOffset)
{
   ScratchOffset = scratchOffset;
   SramOffset = sramOffset;
   NCDramOffset = ncdramOffset;
   CDramOffset = cdramOffset;
    
   return (HALAE_SUCCESS);    
}         

/*-----------------------------------------------------------------------------
   Function:    halAe_GetMemoryStartOffset
   Description: get memory start offset
   Returns:     HALAE_SUCCESS or error number
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetMemoryStartOffset(unsigned int *scratchOffset, 
                           unsigned int *sramOffset, 
                           unsigned int *ncdramOffset, 
                           unsigned int *cdramOffset)
{
   *scratchOffset = ScratchOffset;
   *sramOffset = SramOffset;
   *ncdramOffset = NCDramOffset;
   *cdramOffset = CDramOffset;         

   return (HALAE_SUCCESS);       
}    

/*-----------------------------------------------------------------------------
   Function:    halAe_ringPut
   Description: put ring data
   Returns:     HALAE_SUCCESS or error number
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int halAe_ringPut(uint64 baseAddr, unsigned int ringSize, unsigned int tailptr, unsigned int *in_data, unsigned int count)
{
   uint64 baseAddr_v;    
   unsigned int i;
   
   baseAddr_v = (uint64)((unsigned int)phys_to_virt(baseAddr));
   for(i = 0; i < count; i++) 
   {
       WRITE_LWORD(baseAddr_v + ((tailptr+(4*i))%ringSize), in_data[i]);
   }    
   
   return HALAE_SUCCESS;
}

/*-----------------------------------------------------------------------------
   Function:    halAe_ringGet
   Description: get ring data
   Returns:     HALAE_SUCCESS or error number
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int halAe_ringGet(uint64 baseAddr, unsigned int ringSize, unsigned int headptr, unsigned int *out_data, unsigned int count)
{
   uint64 baseAddr_v;    
   unsigned int i;
  
   baseAddr_v = (uint64)((unsigned int)phys_to_virt(baseAddr));
   for(i = 0; i < count; i++) 
   {
       out_data[i] = READ_LWORD(baseAddr_v + ((headptr+(4*i))%ringSize));
   }    
       
   return HALAE_SUCCESS;    
}

/***************************************************************************
 * mapMemIo
 *
 * Description:
 *
 *     Map virtual memory
 *
***************************************************************************/
int 
mapMemIo(HalMmapIo_T *pMemIo)
{
    unsigned int defFlags = _PAGE_PRESENT | _PAGE_DIRTY | _PAGE_USER | _PAGE_RW;
    unsigned int flags;
    uint64 phys, virt;

    flags = defFlags | (pMemIo->cacheable ? 0 : __PAGE_KERNEL_NOCACHE);

#ifdef    _DBG_PRINT

    PRINTF("mapMemIo: phy=0x%x, size=0x%x, flags=0x%x\n", (unsigned int)pMemIo->phys, pMemIo->size, flags); 

#endif

    /* Strip off high order bits */
    phys = pMemIo->phys & ~VA_MASK_ALL;
    
    if(!(virt = (unsigned int)ioremap((PHYS_ADDR_T)phys, pMemIo->size)))
    {

#ifdef    _DBG_PRINT

        PRINTF("init_module: ioremap failed...\n");

#endif

        return (ERROR);
    }

#ifdef    _DBG_PRINT
   
    PRINTF("ioremap(%09llX, %08X) -> %08X\n", phys, pMemIo->size, (unsigned int)virt);

#endif

    /* Copy high order bits from phys to virt */
    pMemIo->virt = virt | (pMemIo->phys & VA_MASK_ALL);

    return (SUCCESS);
}

/***************************************************************************
 * unmapMemIo
 *
 * Description:
 *
 *     Unmap virtual memory
 *
***************************************************************************/
int 
unmapMemIo(HalMmapIo_T *pMemIo)
{
    iounmap((void*)((unsigned int)(pMemIo->virt)));

#ifdef    _DBG_PRINT

    PRINTF("__iounmap(%08X)\n", (unsigned int)pMemIo->virt);
    
#endif

    pMemIo->virt = 0;

    return (SUCCESS);
}

/***************************************************************************
 *    int init_AeDrv_dummy(void)
 *
 * Description:
 *
 *     Dummy device init_module handler
 *
***************************************************************************/
static int __init 
init_AeDrv_dummy(void)
{
    platform_driver_init();
        
    return (SUCCESS);
}

/***************************************************************************
 *    int init_AeDrv_module(void)
 *
 * Description:
 *
 *     Device init_module handler
 *     Does module registration and initializes a global spinlock
 *
***************************************************************************/
static int 
init_AeDrv_module(void)
{
    int                ret;

#ifdef    _DBG_PRINT
   
    void    *TestVtoP1;

    PRINTF("Entering AeDrv Init...\n");

#endif

    SPIN_LOCK_INIT_STRUCT(pci_device_bar);
    /* retrieve PCI information from system driver exposed structure */
    if((ret = pci_device_probe(&my_instanceHandle)))
    {
        PRINTF("icp_hal error: init_AeDrv_module: Probe PCI cofiguration from acceleration system driver failure %x,\n", ret);
	return (ERROR);
    }

    SPIN_LOCK_INIT(bk_lock);
    SPIN_LOCK_INIT(tasklet_lock);

    ixOsalMemSet(&HalSysMemInfo, 0, sizeof(aeDrv_SysMemInfo_T));
      
    /* init hal driver */
    if(halAeDrvInit()) 
    {
       PRINTF("icp_hal error: init_AeDrv_module: Driver initialization failed.\n");
       return (ERROR);
    }   

    return (SUCCESS);
}

/***************************************************************************
 *    int pci_device_probe(icp_accel_dev_t * instanceHandle)
 *
 * Description:
 *
 *     retrieve AE_CLUSTER and RING_CONTROLLER driver info from ASD handle
 *
***************************************************************************/
static int 
pci_device_probe(icp_accel_dev_t * instanceHandle)
{ 
    SPIN_LOCK_STRUCT(pci_device_bar);
    if(ICP_AE_CLUSTER_CAP_BAR >= instanceHandle->aeCluster.numBars)
    {
       SPIN_UNLOCK_STRUCT(pci_device_bar);
       return (ERROR);
    }   
    pci_device_bar.ae_cluster_cap_bridge_reg_region.base_addr = \
         instanceHandle->aeCluster.pciBars[ICP_AE_CLUSTER_CAP_BAR].baseAddr;
    pci_device_bar.ae_cluster_cap_bridge_reg_region.size = \
         instanceHandle->aeCluster.pciBars[ICP_AE_CLUSTER_CAP_BAR].size;
    pci_device_bar.ae_cluster_cap_bridge_reg_region.virt_addr = 0;

    if(ICP_AE_CLUSTER_SP_BAR >= instanceHandle->aeCluster.numBars)
    {
       SPIN_UNLOCK_STRUCT(pci_device_bar);
       return (ERROR);
    }   
    pci_device_bar.ae_cluster_scratch_mem_region.base_addr = \
        instanceHandle->aeCluster.pciBars[ICP_AE_CLUSTER_SP_BAR].baseAddr;
    pci_device_bar.ae_cluster_scratch_mem_region.size = \
        instanceHandle->aeCluster.pciBars[ICP_AE_CLUSTER_SP_BAR].size;
    pci_device_bar.ae_cluster_scratch_mem_region.virt_addr = 0;
                
    if(ICP_AE_CLUSTER_AE_BAR >= instanceHandle->aeCluster.numBars)
    {
       SPIN_UNLOCK_STRUCT(pci_device_bar);
       return (ERROR);
    }   
    pci_device_bar.ae_cluster_ae_tran_reg_region.base_addr = \
       instanceHandle->aeCluster.pciBars[ICP_AE_CLUSTER_AE_BAR].baseAddr;
    pci_device_bar.ae_cluster_ae_tran_reg_region.size = \
       instanceHandle->aeCluster.pciBars[ICP_AE_CLUSTER_AE_BAR].size;
    pci_device_bar.ae_cluster_ae_tran_reg_region.virt_addr = 0;

    if(ICP_AE_CLUSTER_SSU_BAR >= instanceHandle->aeCluster.numBars)
    {
       SPIN_UNLOCK_STRUCT(pci_device_bar);
       return (ERROR);
    }   
    pci_device_bar.ae_cluster_ssu_reg_region.base_addr = \
       instanceHandle->aeCluster.pciBars[ICP_AE_CLUSTER_SSU_BAR].baseAddr;
    pci_device_bar.ae_cluster_ssu_reg_region.size = \
       instanceHandle->aeCluster.pciBars[ICP_AE_CLUSTER_SSU_BAR].size;
    pci_device_bar.ae_cluster_ssu_reg_region.virt_addr = 0;

    rev_id = instanceHandle->aeCluster.revisionId;
    
    pci_dev_array[PCI_DEVICE_AE_CLUSTER_INDEX] = \
       pci_get_device(PCI_VENDOR_ID_INTEL, instanceHandle->aeCluster.deviceId, NULL);
    pci_set_master(pci_dev_array[PCI_DEVICE_AE_CLUSTER_INDEX]);

    /* record the pci irq number for all AE generated interrupts */
    pci_ae_irq = instanceHandle->aeCluster.irq;
    /* SET SMIA */
    pci_write_config_byte(pci_dev_array[PCI_DEVICE_AE_CLUSTER_INDEX], SMIA_OFFSET, AE_INT_BITS);    

#ifdef _DBG_PRINT
   
    PRINTF("AE interrupt vector : 0x%x\n", pci_ae_irq);

#endif

    /* get swsku and icp_dev_cfg to determine AE speed clock */
    pci_read_config_byte(pci_dev_array[PCI_DEVICE_AE_CLUSTER_INDEX], SWSKU, &swsku);
#ifdef    _DBG_PRINT
    PRINTF("swsku=0x%x in ae cluster\n", swsku);
#endif
    pci_read_config_dword(pci_dev_array[PCI_DEVICE_AE_CLUSTER_INDEX], ICP_DEVICE_CONFIG, &icp_dev_cfg);
#ifdef    _DBG_PRINT
    PRINTF("ICP_DEVICE_CONFIG=0x%x\n", icp_dev_cfg);
#endif

    if(ICP_RING_CONTROLLER_CSR_BAR >= instanceHandle->ringCtrlr.numBars)
    {
       SPIN_UNLOCK_STRUCT(pci_device_bar);
       return (ERROR);
    }   
    pci_device_bar.ring_cntl_reg_region.base_addr = \
      instanceHandle->ringCtrlr.pciBars[ICP_RING_CONTROLLER_CSR_BAR].baseAddr;
    pci_device_bar.ring_cntl_reg_region.size = \
      instanceHandle->ringCtrlr.pciBars[ICP_RING_CONTROLLER_CSR_BAR].size;
    pci_device_bar.ring_cntl_reg_region.virt_addr = 0;

    if(ICP_RING_CONTROLLER_RING_BAR >= instanceHandle->ringCtrlr.numBars)
    {
       SPIN_UNLOCK_STRUCT(pci_device_bar);
       return (ERROR);
    }   
    pci_device_bar.ring_cntl_get_put_region.base_addr = \
      instanceHandle->ringCtrlr.pciBars[ICP_RING_CONTROLLER_RING_BAR].baseAddr;
    pci_device_bar.ring_cntl_get_put_region.size = \
      instanceHandle->ringCtrlr.pciBars[ICP_RING_CONTROLLER_RING_BAR].size;
    pci_device_bar.ring_cntl_get_put_region.virt_addr = 0;

    if(ICP_RING_CONTROLLER_SRAM_BAR >= instanceHandle->ringCtrlr.numBars)
    {
       SPIN_UNLOCK_STRUCT(pci_device_bar);
       return (ERROR);
    }   
    pci_device_bar.ring_cntl_sram_region.base_addr = \
      instanceHandle->ringCtrlr.pciBars[ICP_RING_CONTROLLER_SRAM_BAR].baseAddr;
    pci_device_bar.ring_cntl_sram_region.size = \
      instanceHandle->ringCtrlr.pciBars[ICP_RING_CONTROLLER_SRAM_BAR].size;
    pci_device_bar.ring_cntl_sram_region.virt_addr = 0;

    hal_ae_sram_base = pci_device_bar.ring_cntl_sram_region.base_addr;

    pci_dev_array[PCI_DEVICE_RING_CONTROLLER_INDEX] = \
       pci_get_device(PCI_VENDOR_ID_INTEL, instanceHandle->ringCtrlr.deviceId, NULL);
    
    /* Probe platform specific devices */
    if(platform_device_probe(instanceHandle) != SUCCESS)
    {
        SPIN_UNLOCK_STRUCT(pci_device_bar);
        return (ERROR);
    }
    
    SPIN_UNLOCK_STRUCT(pci_device_bar);

    return (SUCCESS);
}

/***************************************************************************
 *    void unmap_memory(void)
 *
 * Description:
 *
 *    unmap memory mapped by the driver
 *
***************************************************************************/
static void 
unmap_memory(void)
{
    HalMmapIo_T    *pMemIo;

    SPIN_LOCK(MEMTABLE_LOCK);
    /* unmap memory that was mapped by this driver */
    for(pMemIo = &HalMmapTable.dram_ch0; pMemIo < &HalMmapTable.the_end; pMemIo++)
    {
        if(pMemIo->size == 0) 
        {
           continue;
        }   
        if (pMemIo->virt == 0) 
        {
           continue;
        }   
        /* PRINTF("Physical = 0x%p Virtual = 0x%p\n",(void*)(pMemIo->phys), (void*)(pMemIo->virt)); */
        iounmap((void*)((unsigned int)(pMemIo->virt)));
        pMemIo->virt = 0;
    }
    SPIN_UNLOCK(MEMTABLE_LOCK);
    
    return;
}

/***************************************************************************
 *    void __exit cleanup_AeDrv_dummy(void)
 *
 * Description:
 *
 *    unregister debug-purpose driver
 *
***************************************************************************/
static void __exit 
cleanup_AeDrv_dummy(void)
{
    platform_driver_cleanup();
    
    return;
}

/***************************************************************************
 *    void cleanup_AeDrv_module(void)
 *
 * Description:
 *
 *    cleanup AE driver
 *
***************************************************************************/
static void 
cleanup_AeDrv_module(void)
{
    /* pci_dev_put Must be called when a user of a device is finished with it. 
     * When the last user of the device calls this function, 
     * the memory of the device is freed. */
    if(pci_dev_array[PCI_DEVICE_AE_CLUSTER_INDEX])
    {
        pci_dev_put(pci_dev_array[PCI_DEVICE_AE_CLUSTER_INDEX]);
    }
    if(pci_dev_array[PCI_DEVICE_RING_CONTROLLER_INDEX])
    {
        pci_dev_put(pci_dev_array[PCI_DEVICE_RING_CONTROLLER_INDEX]);
    }
    unmap_memory();
    SPIN_LOCK_FINI(MEMINFO_LOCK);
    SPIN_LOCK_FINI(MEMTABLE_LOCK);    
    SPIN_LOCK_FINI(bk_lock);
    SPIN_LOCK_FINI(tasklet_lock);    
    SPIN_LOCK_FINI_STRUCT(pci_device_bar);
}

/* control fucntions for ASD */

/***************************************************************************
 *    CpaStatus icp_AsdCfgHalInit(CpaInstanceHandle instanceHandle, 
 *                 icp_asd_cfg_param_get_cb_func_t getCfgParamFunc)
 *
 * Description:
 *
 *    initialize HAL driver. This function invoked by ASD.
 *
***************************************************************************/
CpaStatus 
icp_AsdCfgHalInit(CpaInstanceHandle instanceHandle, 
                  icp_asd_cfg_param_get_cb_func_t getCfgParamFunc)
{
    icp_asd_cfg_value_t tmp;
        
    memcpy((void *)&my_instanceHandle, 
           (void *)(unsigned int)instanceHandle, 
           sizeof(icp_accel_dev_t));
    
    getCfgParamFunc(ICP_ASD_CFG_PARAM_NCDRAM_ADDRESS, &tmp);
    hal_ae_ncdram_base = (unsigned int)tmp;
    getCfgParamFunc(ICP_ASD_CFG_PARAM_NCDRAM_SIZE_BYTES, &tmp);
    hal_ae_ncdram_size = (unsigned int)tmp;
    getCfgParamFunc(ICP_ASD_CFG_PARAM_CDRAM_ADDRESS, &tmp);
    hal_ae_cdram_base = (unsigned int)tmp;
    getCfgParamFunc(ICP_ASD_CFG_PARAM_CDRAM_SIZE_BYTES, &tmp);
    hal_ae_cdram_size = (unsigned int)tmp;
    
    return (init_AeDrv_module());    
}

/***************************************************************************
 *    CpaStatus icp_AsdCfgHalStart(CpaInstanceHandle instanceHandle)
 *
 * Description:
 *
 *    start HAL driver. This function is a fake.
 *
***************************************************************************/
CpaStatus 
icp_AsdCfgHalStart(CpaInstanceHandle instanceHandle)
{
    return (SUCCESS);    
}

/***************************************************************************
 *    CpaStatus icp_AsdCfgHalStop(CpaInstanceHandle instanceHandle)
 *
 * Description:
 *
 *    stop HAL driver. This function is a fake.
 *
***************************************************************************/
CpaStatus 
icp_AsdCfgHalStop(CpaInstanceHandle instanceHandle)
{
    return (SUCCESS);    
}

/***************************************************************************
 *    CpaStatus icp_AsdCfgHalShutdown(CpaInstanceHandle instanceHandle)
 *
 * Description:
 *
 *    shutdown HAL driver. This function invoked by ASD.
 *
***************************************************************************/
CpaStatus 
icp_AsdCfgHalShutdown(CpaInstanceHandle instanceHandle)
{
    cleanup_AeDrv_module();    
    return (SUCCESS);        
}

/* module initialization entry */
module_init(init_AeDrv_dummy);

/* module un-initialization entry */
module_exit(cleanup_AeDrv_dummy);

EXPORT_SYMBOL(Hal_dram_ch0_virtAddr);
EXPORT_SYMBOL(Hal_dram_ch1_virtAddr);
EXPORT_SYMBOL(Hal_sram_ch0_virtAddr);
EXPORT_SYMBOL(Hal_sram_ch0_rd_wr_virtAddr);
EXPORT_SYMBOL(Hal_cap_global_ctl_csr_virtAddr);
EXPORT_SYMBOL(Hal_cap_ae_xfer_csr_virtAddr);
EXPORT_SYMBOL(Hal_cap_ae_local_csr_virtAddr);
EXPORT_SYMBOL(Hal_cap_hash_csr_virtAddr);
EXPORT_SYMBOL(Hal_scratch_rd_wr_swap_virtAddr);
EXPORT_SYMBOL(Hal_ae_fastaccess_csr_virtAddr);
EXPORT_SYMBOL(Hal_ssu_csr_virtAddr);
EXPORT_SYMBOL(Hal_eagletail_ring_csr_virtAddr);
EXPORT_SYMBOL(Hal_eagletail_ring_access_virtAddr);
EXPORT_SYMBOL(Hal_memory_target_csr_virtAddr);
EXPORT_SYMBOL(halAe_IntrDisable);
EXPORT_SYMBOL(halAe_IntrPoll);
EXPORT_SYMBOL(halAe_IntrEnable);
EXPORT_SYMBOL(MEMINFO_LOCK);
EXPORT_SYMBOL(MEMTABLE_LOCK);
EXPORT_SYMBOL(HalSysMemInfo);
EXPORT_SYMBOL(HalMmapTable);
EXPORT_SYMBOL(halAe_GetMemoryStartOffset);
EXPORT_SYMBOL(halAe_SetMemoryStartOffset);
EXPORT_SYMBOL(intr_init);
EXPORT_SYMBOL(intr_cleanup);
EXPORT_SYMBOL(pci_dev_array);
EXPORT_SYMBOL(icp_AsdCfgHalInit);
EXPORT_SYMBOL(icp_AsdCfgHalStart);
EXPORT_SYMBOL(icp_AsdCfgHalStop);
EXPORT_SYMBOL(icp_AsdCfgHalShutdown);
EXPORT_SYMBOL(getVirXaddr);
EXPORT_SYMBOL(getPhyXaddr);
EXPORT_SYMBOL(getMapIo);  
EXPORT_SYMBOL(halAe_ringPut);
EXPORT_SYMBOL(halAe_ringGet);


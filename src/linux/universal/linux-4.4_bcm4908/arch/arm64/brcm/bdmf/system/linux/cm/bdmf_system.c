/*
* <:copyright-BRCM:2013-2015:GPL/GPL:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :>
*/

#include "bdmf_system.h"
#include "bdmf_system_common.h"
/*
 * System buffer abstraction
 */

/** Recycle the data buffer
 * \param[in]   data   Data buffer
 * \param[in]   context -unused,for future use
 */
void bdmf_sysb_databuf_recycle(void *datap, unsigned long context)
{
   return;
}

/** Recycle the system buffer and associated data buffer
 * \param[in]   data   Data buffer
 * \param[in]   context - unused,for future use
 * \param[in]   flags   - indicates what to recyle
 */
void bdmf_sysb_recycle(bdmf_sysb sysb, unsigned long context, uint32_t flags)
{
    return;
}

/*
 * Platform buffer support
 */
static inline void *bdmf_get_tm_ddr_base(void)
{
    return NULL;
}

/** Add data to sysb
 *
 * The function will is similar to skb_put()
 *
 * \param[in]   sysb        System buffer
 * \param[in]   bytes       Bytes to add
 * \returns added block pointer
 */
static inline void *bdmf_sysb_put(const bdmf_sysb sysb, uint32_t bytes)
{
   return NULL;
}

int bdmf_int_connect(int irq, int cpu, int flags,
    int (*isr)(int irq, void *priv), const char *name, void *priv)
{
	/* if needed, put CM code to register for an interrupt */
	return 0;
}

/** Unmask IRQ
 * \param[in]   irq IRQ
 */
void bdmf_int_enable(int irq)
{
    /*
     * in case of Oren the BcmHalMapInterrupt already enables the interrupt.
     */
}

/** Mask IRQ
 * \param[in]   irq IRQ
 */
void bdmf_int_disable(int irq)
{
    /* Supposingly should work for all BCM platforms.
     * If it is not the case - mode ifdefs can be added later.
     */
}

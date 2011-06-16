/**
 **************************************************************************
 * @file asd_isr.c
 *
 * @description
 *      This file contains ASD code for services to QATAL layer
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
#include <linux/version.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include "icp_asd_cfg.h"
#include "asd_drv.h"
#include "asd_isr.h"
#include "asd_cfg.h"
#include "qat_comms.h"

/*
 * Interrupt Registers: Macros
 */
/* Offset of Signal Target Raw Interrupt Register */
#define SINT_OFFSET                0xEC
/* SINT4: Interrupt for rings 63-32 */
#define SINT4_RINGS_63_32           0x10
/* SINT3: Interrupt for rings 31-0 */
#define SINT3_RINGS_31_0            0x8
/* SINT2: Interrupt for CPPM Push/Pull/Misc. Error */
#define SINT2_MISC_ERROR            0x4
/* SINT1: Interrupt for CPPM Correctable Error Error */
#define SINT1_CORRECTABLE_ERROR     0x2
/* SINT0: Interrupt for CPPM UnCorrectable Error Error */
#define SINT0_UNCORRECTABLE_ERROR   0x1

/*
 * The following global variable determines whether to use MSI or INTx
 * interrupts for the ET Ring Controller. It is updated by reading the
 * ICP_ASD_CFG_PARAM_ET_RING_MSI_INTERRUPT_ENABLE parameter from the ASD
 * Configuration table.
 */
static icp_asd_cfg_value_t config_pci_msi=0;

/* These belong to the accelerator */
static struct tasklet_struct *qat_tasklet = NULL;


/**
 * Function to schedule the QAT tasklet 
 */
static int asd_schedule_qat_tasklet(void)
{
        if (qat_tasklet) {
                tasklet_schedule(qat_tasklet);
                return SUCCESS;
        }
        return FAIL;
}


/**
 * Top Half ISR
 *     Calls the appropriate TopHalf handler.
 *     Assumption is that an error return status means interrupt was not handled
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
static irqreturn_t asd_intr(int irq, void *privdata, struct pt_regs *ptx)
#else
static irqreturn_t asd_intr(int irq, void *privdata)
#endif
{
        icp_accel_dev_t *accel_dev=(icp_accel_dev_t *)privdata;
        Cpa8U sInt=0;
        struct pci_dev *pdev=NULL;
        CpaStatus return_value = CPA_STATUS_FAIL;

        /* 
         * Perform top-half processing.
         * Check SINT register to determine what ring the interrupt has fired on
         * If an interrupt occurs on rings 0-31 call the QAT top half handler
         * If an interrupt occurs on rings 32-63 then something went wrong so 
         * log an error message
         * If an interrupt was handled, return IRQ_HANDLED, else return IRQ_NONE
         */ 
        pdev = accel_dev->ringCtrlr.pDev;
        pci_read_config_byte(pdev, SINT_OFFSET, &sInt);

        if (sInt & SINT3_RINGS_31_0)
        {
                return_value = QatComms_intr();
        }

        if (sInt & SINT4_RINGS_63_32)
        {
                /* 
                 * This shouldn't happen!!!
                 * We are in lookaside only mode 
                 */
                ASD_ERROR(KERN_ERR "Error - got interrupt on upper bank in lookaside only mode\n");
                return IRQ_HANDLED;
        }


        if ( return_value == CPA_STATUS_SUCCESS )
        {
                return IRQ_HANDLED;
        }
        else
        {
                return IRQ_NONE;
        }
}

/**
 * Free up the required resources used by QATAL ISR services.
 */

void asd_isr_resource_free(icp_accel_dev_t *accel_dev,
					icp_accel_pci_info_t *pci_info)
{
        int ring_ctlr_irq = 0;

        ring_ctlr_irq = pci_info->irq;

        if (ring_ctlr_irq > 0) {

                free_irq(ring_ctlr_irq, accel_dev);
                pci_info->irq = 0;

                if (config_pci_msi){
                        ASD_DEBUG("Disabling MSI for ring controller \n");
                        pci_disable_msi(pci_info->pDev);
                }
        }

        if (qat_tasklet) {
                tasklet_disable(qat_tasklet);
                tasklet_kill(qat_tasklet);
                kfree(qat_tasklet);
        }
        qat_tasklet = NULL;

        return;
}

/**
 * Allocate the required QATAL ISR services.
 */
int asd_isr_resource_alloc(icp_accel_dev_t *accel_dev,
				icp_accel_pci_info_t *pci_info)
{
        int status = 1;
        int ring_ctlr_irq = 0;
        CpaStatus qat_status = CPA_STATUS_SUCCESS;
        int return_code = FAIL;

        return_code = asd_cfg_param_get(
                                ICP_ASD_CFG_PARAM_ET_RING_MSI_INTERRUPT_ENABLE,
                                &config_pci_msi);

        if (return_code != SUCCESS) {
               ASD_ERROR("failed to read ET_RING_MSI_INTERRUPT_ENABLE\n");
               return FAIL;
        }


        /* create a tasklet for QAT  */
        qat_tasklet = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
        if (!qat_tasklet) {
                ASD_ERROR("Failed to allocate QAT tasklet\n");
                goto err_failure;
        }
        tasklet_init(qat_tasklet,
                     (void (*)(unsigned long))QatComms_bh_handler,
                     (unsigned long)0);

        /*
         * Register the schedule function with QAT to allow it to schedule
         * the bottom-half handler.
         */
        qat_status = QatComms_bh_schedule_register(asd_schedule_qat_tasklet);

        if (qat_status != CPA_STATUS_SUCCESS) {
                ASD_ERROR("failed to register tasklet schedule func with QAT\n");
                goto err_failure;
        }

        /* 
         * ASD must register the master ISR - get IRQ for 
         *  Ring Controller 
         */
        ring_ctlr_irq = pci_info->irq;
        /*
         * Setup the structures in accel_dev
         */
        if (ring_ctlr_irq) {
                if (config_pci_msi){
                        ASD_DEBUG("Enabling MSI for ring controller \n");
                        status = pci_enable_msi(pci_info->pDev);
                        if (status){
                              ASD_ERROR("Unable to allocate MSI interrupt, "
                                          "status: %d\n", status);
                        }else{
                              /*
                               * The irq in the pDev structure may have been
                               * updated in pci_enable_msi() therefore need to
                               * update the irq in accel_dev structure to
                               * ensure it is consistent with the pDev structure
                               */
                              pci_info->irq = pci_info->pDev->irq;
                              ASD_DEBUG("pci_enable_msi success\n");
                        }
                }
                status = request_irq(pci_info->irq, asd_intr,
                                     IRQF_SHARED,
                                     asd_driver_name, (void *)accel_dev);
                
				if (status) {
                        ASD_ERROR("failed request_irq for Ring "
                                  " Controller IRQ %d, status=%d\n",
                                  pci_info->irq, status);
                        goto err_failure;
                }
        }

        return SUCCESS;                /* success */

      err_failure:
        asd_isr_resource_free(accel_dev, pci_info);
        return status;
}

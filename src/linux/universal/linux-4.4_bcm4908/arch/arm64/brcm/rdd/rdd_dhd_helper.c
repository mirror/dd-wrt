/*
   Copyright (c) 2014 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2014:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/

#ifdef CONFIG_DHD_RUNNER
#include "rdd.h"
#include "rdd_runner_defs_auto.h"
#include "rdd_data_structures.h"
#include "rdd_dhd_helper.h"
#include "rdpa_dhd_helper_basic.h"
#if !defined(FIRMWARE_INIT)
#ifndef CM3390
#include <linux/nbuff.h>
#include "bcm_mm.h"
#endif
#include "rdp_mm.h"
#else
#include "rdp_drv_bpm.h"
#include "access_macros.h"
#endif
#ifdef LEGACY_RDP
#include "rdd_legacy_conv.h"
#endif
#include "rdd_ih_defs.h"

#ifdef CM3390
#if !defined(FIRMWARE_INIT)
#include "fpm.h"
#endif
extern uint32_t g_runner_ddr0_base_addr;
#endif
#ifndef G9991
bdmf_boolean is_dhd_enabled[RDPA_MAX_RADIOS] = {0};

rdd_dhd_complete_ring_descriptor_t g_dhd_complete_ring_desc[RDPA_MAX_RADIOS] = {{0, 0, 0, 0},};

extern uint8_t *g_runner_ddr_base_addr;

extern bdmf_fastlock int_lock_irq;
extern rdpa_bpm_buffer_size_t g_bpm_buffer_size;

#if !defined(FIRMWARE_INIT)
#if defined(DHD_RX_POST_HOST)
#include "bdmf_system_common.h"
#include "bcm_intr.h"
#include <linux/kthread.h>

// locally maintained data strcutre to keep RxPost ring info
typedef struct {
    bdmf_boolean is_irq_enabled;
    bdmf_boolean is_initialized;
    bdmf_task rx_post_task;
    wait_queue_head_t thread_wqh;
    int work_avail;
    rdd_dhd_rx_post_ring_t ring[RDPA_MAX_RADIOS];
} rdd_dhd_rx_post_priv_t;
rdd_dhd_rx_post_priv_t g_dhd_rx_post_priv = {0, 0, };
DEFINE_BDMF_FASTLOCK(work_avail_lock);
#else
rdd_dhd_rx_post_ring_t g_dhd_rx_post_ring_priv[RDPA_MAX_RADIOS] = {{0, 0, 0, 0},};
#endif
#endif

#if defined(CM3390)
// FPM Related External References
uint32_t fpm_alloc_token(int size);
void     fpm_free_token(uint32_t);
uint8_t *fpm_token_to_buffer(uint32_t);
#define FPM_TOKEN_VALID_MASK_4RDH          0x80000000
#define FPM_TOKEN_INDEX_MASK_4RDH          0x0001FFFF
#define FPM_TOKEN_INDEX_F_WIDTH_4RDH       17
#define FPM_TOKEN_INDEX_F_OFFSET_4RDH      12
#define DHD_FPM_COUNT_PER_BUFFER_4RDH      8
#endif // defined(CM3390)

#if defined(WL4908)
// FPM Related External References
extern uint32_t fpm_alloc_max_size_token_pool(int pool);
extern void fpm_free_token(uint32_t);
extern uint8_t *fpm_token_to_buffer(uint32_t);
extern uint32_t fpm_convert_fpm_token_to_rdp_token(uint32_t token);
extern uint32_t fpm_convert_rdp_token_to_fpm_token(uint32_t token);
#define FPM_TOKEN_VALID_MASK_4RDH          0x80000000
#define BPM_BUFFER_NUMBER_DDR_OFFSET       17
#define BPM_BUFFER_NUMBER_INDEX_WIDTH      16
#define BPM_BUFFER_NUMBER_INDEX_OFFSET     0
#endif

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#define HEADROOM_SIZE_4RDH ((g_ddr_headroom_size + DRV_RDD_IH_PACKET_HEADER_OFFSET + 3) & (~3))  // Must pad up to 4 byte boundary
#elif defined(CM3390)
#define HEADROOM_SIZE_4RDH DHD_DATA_OFFSET
#else
#define HEADROOM_SIZE_4RDH (g_ddr_headroom_size + DHD_DATA_OFFSET)
#endif

static void rdd_rx_post_descr_init(uint32_t radio_idx, uint8_t *descr_ptr, uint32_t bpm_buffer_number,
    bdmf_boolean valid_bpm)
{
    uint32_t req_id;
    uint16_t len;
    uint8_t* data_buf_ptr_low_virt;
    bdmf_phys_addr_t data_buf_ptr_low_phys;

    RDD_DHD_RX_POST_DESCRIPTOR_MSG_TYPE_WRITE(DHD_MSG_TYPE_RX_POST, descr_ptr);
    RDD_DHD_RX_POST_DESCRIPTOR_IF_ID_WRITE(radio_idx, descr_ptr);
    RDD_DHD_RX_POST_DESCRIPTOR_COMMON_HDR_FLAGS_WRITE(0, descr_ptr);
    RDD_DHD_RX_POST_DESCRIPTOR_EPOCH_WRITE(0, descr_ptr);

#if defined(WL4908) || defined(CM3390)
    req_id = bpm_buffer_number | (DRV_BPM_SP_SPARE_1 << 18);
    len = __swap2bytes(DHD_DATA_LEN);
#elif defined(DSL_63138)
    req_id = bpm_buffer_number | (DRV_BPM_SP_SPARE_1 << 15);
    len = __swap2bytes(DHD_DATA_LEN);
#elif defined(DSL_63148)
    req_id = bpm_buffer_number | (DRV_BPM_SP_SPARE_1 << 14);
    len = __swap2bytes(DHD_DATA_LEN);
#elif defined(CM3390) && !defined(FIRMWARE_INIT)
    req_id = fpm_get_token_index(bpm_buffer_number);
    len = __swap2bytes(DHD_DATA_LEN);
#else
    req_id = bpm_buffer_number | (DRV_BPM_SP_SPARE_1 << 14);
    len = cpu_to_le16(DHD_DATA_LEN);
#endif

#if defined(CM3390)
#if defined(FIRMWARE_INIT)
    data_buf_ptr_low_virt = valid_bpm ? (uint8_t *)g_runner_ddr0_base_addr + bpm_buffer_number * 512 + HEADROOM_SIZE_4RDH : 0;
#else
    data_buf_ptr_low_virt = valid_bpm ? fpm_token_to_buffer_phys(bpm_buffer_number) + HEADROOM_SIZE_4RDH : 0;
    fpm_track_token_tx(bpm_buffer_number);
#endif
#else
#if defined(WL4908) && !defined(FIRMWARE_INIT)
    data_buf_ptr_low_virt = fpm_token_to_buffer(fpm_convert_rdp_token_to_fpm_token(bpm_buffer_number)) + HEADROOM_SIZE_4RDH;
#else
    data_buf_ptr_low_virt = valid_bpm ? g_runner_ddr_base_addr + bpm_buffer_number * g_bpm_buffer_size +
        HEADROOM_SIZE_4RDH : 0;
#endif
#endif

    data_buf_ptr_low_phys = RDD_VIRT_TO_PHYS(data_buf_ptr_low_virt);

#if !defined(FIRMWARE_INIT) && (defined(DSL_63138) || defined(DSL_63148) || defined(WL4908) || defined(CM3390))
    data_buf_ptr_low_phys = __swap4bytes(data_buf_ptr_low_phys);
#else
    data_buf_ptr_low_phys = cpu_to_le32(data_buf_ptr_low_phys);
#endif

    RDD_DHD_RX_POST_DESCRIPTOR_REQUEST_ID_WRITE(req_id, descr_ptr);
    RDD_DHD_RX_POST_DESCRIPTOR_META_BUF_LEN_WRITE(0, descr_ptr);
    RDD_DHD_RX_POST_DESCRIPTOR_DATA_LEN_WRITE(len, descr_ptr);
    RDD_DHD_RX_POST_DESCRIPTOR_METADATA_BUF_ADDR_HI_WRITE(0, descr_ptr);
    RDD_DHD_RX_POST_DESCRIPTOR_METADATA_BUF_ADDR_LOW_WRITE(0, descr_ptr);
    RDD_DHD_RX_POST_DESCRIPTOR_DATA_BUF_ADDR_HI_WRITE(0, descr_ptr);
    RDD_DHD_RX_POST_DESCRIPTOR_DATA_BUF_ADDR_LOW_WRITE(data_buf_ptr_low_phys, descr_ptr);

#if !defined(FIRMWARE_INIT)
#ifdef CM3390
    if (valid_bpm)
        bdmf_dcache_flush(descr_ptr, 32);
#else
    if (valid_bpm)
        cache_flush_len((void *)descr_ptr, 32);
#endif
#endif
}

#if 0
static void rdd_rx_post_descr_dump(uint32_t desc)
{
    uint32_t req_id, buf_addr;
    uint16_t data_len;

    RDD_DHD_RX_POST_DESCRIPTOR_REQUEST_ID_READ(req_id, desc);
    RDD_DHD_RX_POST_DESCRIPTOR_DATA_LEN_READ(data_len, desc);
    RDD_DHD_RX_POST_DESCRIPTOR_DATA_BUF_ADDR_LOW_READ(buf_addr, desc);
}
#endif

int rdd_dhd_rx_post_init(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg)
{
    int rc = DRV_BPM_ERROR_NO_ERROR;
#if !defined(FIRMWARE_INIT)
#if defined(DHD_RX_POST_HOST)
    rdd_dhd_rx_post_ring_t *ring_info = &g_dhd_rx_post_priv.ring[radio_idx];
#else
    rdd_dhd_rx_post_ring_t *ring_info = &g_dhd_rx_post_ring_priv[radio_idx];
#endif
    uint32_t bpm_buffer_number;
    uint8_t *write_ptr;
    uint32_t i;

    for (i = 0, write_ptr = init_cfg->rx_post_flow_ring_base_addr; i < DHD_RX_POST_FLOW_RING_SIZE - 1; i++)
    {
#if defined(CM3390)
        uint32_t token;
        token = fpm_alloc_token(g_bpm_buffer_size * DHD_FPM_COUNT_PER_BUFFER_4RDH);

        if (token & FPM_TOKEN_VALID_MASK_4RDH)
            bpm_buffer_number = (token >> FPM_TOKEN_INDEX_F_OFFSET_4RDH) & FPM_TOKEN_INDEX_MASK_4RDH;
        else
            return DRV_BPM_ERROR_NO_FREE_BUFFER;
#elif defined(WL4908)
        uint32_t token;
        token = fpm_alloc_max_size_token_pool(1);
        if (token & FPM_TOKEN_VALID_MASK_4RDH)
            bpm_buffer_number = fpm_convert_fpm_token_to_rdp_token(token);
        else
            return DRV_BPM_ERROR_NO_FREE_BUFFER;
#else
        rc = fi_bl_drv_bpm_req_buffer(DRV_BPM_SP_SPARE_1, (uint32_t *)&bpm_buffer_number);
        if (rc != DRV_BPM_ERROR_NO_ERROR)
            return rc;
#endif
        rdd_rx_post_descr_init(radio_idx, write_ptr, bpm_buffer_number, 1);
        write_ptr += sizeof(RDD_DHD_RX_POST_DESCRIPTOR_DTS);
    }

    *ring_info->wr_idx_addr = cpu_to_le16(DHD_RX_POST_FLOW_RING_SIZE - 1);
#endif // !defined(FIRMWARE_INIT)
    return rc;
}

int rdd_dhd_rx_post_uninit(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg)
{
#if !defined(FIRMWARE_INIT)
    uint32_t bpm_buffer_number;
    uint8_t *descr_ptr;
    int rc = 0;
    uint16_t start, end;
#if defined(DHD_RX_POST_HOST)
    rdd_dhd_rx_post_ring_t *ring_info = &g_dhd_rx_post_priv.ring[radio_idx];
#else
    rdd_dhd_rx_post_ring_t *ring_info = &g_dhd_rx_post_ring_priv[radio_idx];
#endif
 
    /* the logic behind this is, there should always be
     * (DHD_RX_POST_FLOW_RING_SIZE - 1) buffers in RxPost ring, because Runner
     * allocates 1 back when it receives 1 in RxComplete.  (Wr_idx - 1) should
     * represent the last refilled buffer, and WRAP(wr_idx + 1) should be
     * the oldest refilled buffer in RxPost.  Therefore, we will free by
     * going from wr_idx + 1, toward wr_idx + 2, and on until it wraps
     * around and gets to (wr_idx - 1) */
    end = le16_to_cpu(*ring_info->wr_idx_addr);
    rmb();
    start = (end + 1) & (DHD_RX_POST_FLOW_RING_SIZE - 1);

    do {
        descr_ptr = init_cfg->rx_post_flow_ring_base_addr;
        descr_ptr += (sizeof(RDD_DHD_RX_POST_DESCRIPTOR_DTS) * start);
        RDD_DHD_RX_POST_DESCRIPTOR_REQUEST_ID_READ(bpm_buffer_number, descr_ptr);
#if defined(CM3390)
        bpm_buffer_number = FPM_TOKEN_VALID_MASK_4RDH | (bpm_buffer_number << FPM_TOKEN_INDEX_F_OFFSET_4RDH) | (g_bpm_buffer_size * DHD_FPM_COUNT_PER_BUFFER_4RDH);
        fpm_free_token(bpm_buffer_number);
#elif defined(WL4908)
        bpm_buffer_number &= ~(DRV_BPM_SP_SPARE_1 << 18);
        bpm_buffer_number = fpm_convert_rdp_token_to_fpm_token(bpm_buffer_number);
        fpm_free_token(bpm_buffer_number);
#elif defined(DSL_63138)
        bpm_buffer_number &= ~(DRV_BPM_SP_SPARE_1 << 15);
        rc = fi_bl_drv_bpm_free_buffer(DRV_BPM_SP_SPARE_1, bpm_buffer_number);
#elif defined(DSL_63148)
        bpm_buffer_number &= ~(DRV_BPM_SP_SPARE_1 << 14);
        rc = fi_bl_drv_bpm_free_buffer(DRV_BPM_SP_SPARE_1, bpm_buffer_number);
#else
        bpm_buffer_number &= ~(DRV_BPM_SP_SPARE_1 << 14);
        rc = fi_bl_drv_bpm_free_buffer(DRV_BPM_SP_SPARE_1, bpm_buffer_number);
#endif // defined(DSL_63138)

        if (rc)
            bdmf_trace("Error releasing BPM num %d, rc = %d\n", bpm_buffer_number, rc);

        start++;
        if (unlikely(start == DHD_RX_POST_FLOW_RING_SIZE))
            start = 0;

    } while (start != end);
#endif //  !defined(FIRMWARE_INIT)
   
    return 0;
}

#if defined(DHD_RX_POST_HOST) && !defined(FIRMWARE_INIT)
/* should return the available free space between wr_idx and rd_idx.
 * It needs to handle the case that wr_idx will never overrun rd_idx.
 * Since dongle is fetching 32 RxPost buffer in at once, we will
 * return the avail_cnt will be subtracted by 32, so wr_idx will never
 * overrun rd_idx.
 * Also, maximum # buffer allocation is 32. */
static inline int rdd_dhd_rx_post_get_alloc_cnt(uint16_t wr_idx, uint16_t rd_idx)
{
    int avail_cnt;

    if (rd_idx <= wr_idx)
        avail_cnt = DHD_RX_POST_FLOW_RING_SIZE + rd_idx - wr_idx;
    else
        avail_cnt = rd_idx - wr_idx;
    avail_cnt -= 32;

    if (avail_cnt < 0)
        return 0;
    else
        return avail_cnt;
}

static inline int rdd_dhd_rx_post_check_and_fill(uint32_t radio_idx)
{
    rdd_dhd_rx_post_ring_t *ring_info = &g_dhd_rx_post_priv.ring[radio_idx];
    uint8_t *write_ptr;
    int alloc_cnt = 0, avail_cnt;
    uint32_t bpm_buffer_number, token;

    if (is_dhd_enabled[radio_idx] == 0)
        return 0;

    /* get the updated rd_idx from dongle */
    ring_info->rd_idx = le16_to_cpu(*ring_info->rd_idx_addr);

    avail_cnt = rdd_dhd_rx_post_get_alloc_cnt(ring_info->wr_idx, ring_info->rd_idx);

    /* refill until wr_idx == rd_idx */
    // TODO!! see if we want to do something like allocating max of 32 buffers at once
    while (alloc_cnt < avail_cnt) {
        token = fpm_alloc_max_size_token_pool(1);
        if (token & FPM_TOKEN_VALID_MASK_4RDH)
            bpm_buffer_number = fpm_convert_fpm_token_to_rdp_token(token);
        else
            break;

        write_ptr = ring_info->ring_base +
                    (sizeof(RDD_DHD_RX_POST_DESCRIPTOR_DTS) * ring_info->wr_idx);

        rdd_rx_post_descr_init(radio_idx, write_ptr, bpm_buffer_number, 1);

        alloc_cnt++;
        ring_info->wr_idx++;
        if (unlikely(ring_info->wr_idx >= DHD_RX_POST_FLOW_RING_SIZE))
            ring_info->wr_idx = 0;
    }

    if (unlikely(alloc_cnt == 0))
        return 0;

    /* update wr_idx to shared location */
    *ring_info->wr_idx_addr = cpu_to_le16(ring_info->wr_idx);
    wmb();

    /* raise h2d interrupt? will try if it is needed */
    writel(cpu_to_le32((uint32_t)ring_info->wr_idx), ring_info->mb_int_mapped);

    return alloc_cnt;
}

static int rdd_dhd_rx_post_thread_func(void *priv)
{
    rdd_dhd_rx_post_priv_t *rx_post_info = (rdd_dhd_rx_post_priv_t *)priv;
    int alloc_cnt, i;
    unsigned long flags;

    while (!kthread_should_stop()) {
        if (kthread_should_stop())
            break;
        wait_event_interruptible(rx_post_info->thread_wqh,
                                 rx_post_info->work_avail);

        alloc_cnt = 0;
        for (i = 0; i < RDPA_MAX_RADIOS; i++)
        {
            alloc_cnt += rdd_dhd_rx_post_check_and_fill(i);
        }

        bdmf_fastlock_lock_irq(&work_avail_lock, flags);
        if ((alloc_cnt == 0) && (rx_post_info->work_avail == 1))
        {
            rx_post_info->work_avail = 0;
            bdmf_fastlock_unlock_irq(&work_avail_lock, flags);
            rdd_interrupt_unmask(DHD_RXPOST_IRQ_NUM, 0);
        }
        else
        {
            rx_post_info->work_avail = 1;
            bdmf_fastlock_unlock_irq(&work_avail_lock, flags);
            yield();
        }
    };
    return 0;
}

static void rdd_dhd_rx_post_task_schedule(rdd_dhd_rx_post_priv_t *rx_post_info)
{
    unsigned long flags;

    /* I hate to use  bdmf_fastlock_lock_irq here, but
     * bdmf_fastlock_lock is actually spin_lck_bh() with a BUG_ON message
     * if it is in irq!, so I simply cannot use it.  And we don't have
     * a bdmf_fastlock_lock_[type], that's the simple spin_lock().. */
    bdmf_fastlock_lock_irq(&work_avail_lock, flags);
    if (rx_post_info->work_avail == 0)
    {
        rx_post_info->work_avail = 1;
        wake_up_interruptible(&rx_post_info->thread_wqh);
    }
    else
    {
        rx_post_info->work_avail = 2;
    }
    bdmf_fastlock_unlock_irq(&work_avail_lock, flags);
}

static irqreturn_t rdd_dhd_rx_post_irq_handler(int irq, void *arg)
{

    rdd_interrupt_mask(DHD_RXPOST_IRQ_NUM, 0);
    rdd_interrupt_clear(DHD_RXPOST_IRQ_NUM, 0);

    rdd_dhd_rx_post_task_schedule((rdd_dhd_rx_post_priv_t *)arg);

    bdmf_int_enable(irq);

    return IRQ_HANDLED;
}

static inline void rdd_dhd_rx_post_setup(void)
{
    char devname[16];
    RUNNER_REGS_CFG_INT_MASK runner_interrupt_mask_register;
    int rc, i;
    bdmf_boolean are_all_disabled = 1;

    for (i = 0; i < RDPA_MAX_RADIOS; i++)
    {
        if (is_dhd_enabled[i] != 0)
            are_all_disabled = 0;
    }


    if ((g_dhd_rx_post_priv.is_irq_enabled == 0) && (are_all_disabled == 0))
    {
        /* interrupt was disabled, but now there is a device getting enabled,
         * so we need to enable interrupt */
        RUNNER_REGS_0_CFG_INT_MASK_READ(runner_interrupt_mask_register);
        runner_interrupt_mask_register.int5_mask = 1;
        RUNNER_REGS_0_CFG_INT_MASK_WRITE(runner_interrupt_mask_register);
        RUNNER_REGS_1_CFG_INT_MASK_WRITE(runner_interrupt_mask_register);

        sprintf(devname, "brcm_dhd_rxpost");
        rc = BcmHalMapInterruptEx((FN_HANDLER)rdd_dhd_rx_post_irq_handler,
                                  &g_dhd_rx_post_priv,
                                  INTERRUPT_ID_RUNNER_5, devname,
                                  INTR_REARM_YES, INTR_AFFINITY_DEFAULT);
        if (rc)
        {
            printk("%s:%d:failed to mount interrupt\n", __func__, __LINE__);
            return;
        }
        g_dhd_rx_post_priv.is_irq_enabled = 1;
    } else if ((g_dhd_rx_post_priv.is_irq_enabled != 0) && (are_all_disabled != 0)) {
        /* interrupt was enabled, but now all the devices are disabled,
         * so we need to disable interrupt */
        free_irq(INTERRUPT_ID_RUNNER_5, &g_dhd_rx_post_priv);
        g_dhd_rx_post_priv.is_irq_enabled = 0;
    }
}
#endif

void rdd_dhd_mode_enable(uint32_t radio_idx, bdmf_boolean enable)
{
    is_dhd_enabled[radio_idx] = enable;
#if defined(DHD_RX_POST_HOST) && !defined(FIRMWARE_INIT)
    rdd_dhd_rx_post_setup();
#endif
}

void rdd_dhd_mode_enable_init(void)
{
    uint32_t radio_idx;

    for (radio_idx = 0; radio_idx < RDPA_MAX_RADIOS; radio_idx++)
        is_dhd_enabled[radio_idx] = (bdmf_boolean) 0;

#if defined(DHD_RX_POST_HOST) && !defined(FIRMWARE_INIT)
    g_dhd_rx_post_priv.is_initialized = 0;
#endif
}

static void rdd_dhd_tx_post_descr_init(uint32_t *descr_addr)
{
    RDD_DHD_TX_POST_DESCRIPTOR_DTS *descr = (RDD_DHD_TX_POST_DESCRIPTOR_DTS *)descr_addr;

    RDD_DHD_TX_POST_DESCRIPTOR_MSG_TYPE_WRITE(DHD_MSG_TYPE_TX_POST, descr);
    RDD_DHD_TX_POST_DESCRIPTOR_COMMON_HDR_FLAGS_WRITE(0, descr);
    RDD_DHD_TX_POST_DESCRIPTOR_EPOCH_WRITE(0, descr);
    RDD_DHD_TX_POST_DESCRIPTOR_SEG_CNT_WRITE(1, descr);
    RDD_DHD_TX_POST_DESCRIPTOR_METADATA_BUF_ADDR_HI_WRITE(0, descr);
    RDD_DHD_TX_POST_DESCRIPTOR_METADATA_BUF_ADDR_LOW_WRITE(0, descr);
    RDD_DHD_TX_POST_DESCRIPTOR_META_BUF_LEN_WRITE(0, descr);
    RDD_DHD_TX_POST_DESCRIPTOR_DATA_BUF_ADDR_HI_WRITE(0, descr);
}

static void rdd_dhd_helper_tx_post_buffers_threshold_set(void)
{
#ifndef U_LINUX
#if !defined(WL4908)
    BPM_MODULE_REGS_BPM_GL_TRSH global_configuration;
#endif
    uint16_t *threshold_ptr = (uint16_t *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + DHD_TX_POST_BUFFERS_THRESHOLD_ADDRESS);
    uint16_t threshold;

#if defined(WL4908)
    /* 4908 does not have global threshold setting, so we hardcode threshold to 8192 */
    threshold = 8192;
#else
    BPM_MODULE_REGS_BPM_GL_TRSH_READ(global_configuration);

    /* Get total number of BPM buffers. */
    if (global_configuration.gl_bat <= DRV_BPM_GLOBAL_THRESHOLD_30K)
        threshold = (global_configuration.gl_bat + 1) * 2560;
    else
        threshold = 2560;
#endif

    /* Set tx post buffers threshold to half of the total number of BPM buffers. */
    threshold /= 2;

    /* The tx post buffers threshold is global for all radios. */
    MWRITE_16((uint8_t *)threshold_ptr, threshold);
#endif
}

#if !defined(FIRMWARE_INIT)
#define DHD_VTOP(val) (uint32_t)VIRT_TO_PHYS(val)
#else
#define DHD_VTOP(val) (uint32_t)val
#endif

int rdd_dhd_hlp_cfg(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg, int enable)
{
    RDD_DHD_RADIO_INSTANCE_COMMON_A_DATA_DTS *radio_instance_data_a_ptr;
    RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_DTS *radio_instance_entry_a_ptr;
    RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_DTS *radio_instance_data_ptr;
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_DTS *radio_instance_entry_ptr;
    RDD_DHD_RX_POST_FLOW_RING_BUFFER_DTS *rxp_flring_buffer_ptr;
    RDD_DHD_RX_POST_DESCRIPTOR_DTS *rxp_flring_entry_ptr;
	RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_DTS *ds_lkp_table_ptr;
	RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_DTS *us_lkp_table_ptr;
    uint32_t i, rx_post_last_wr_idx;
#if 0 /* not used */
    uintptr_t rx_post_ptr;
#endif

    radio_instance_data_ptr = (RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_1_OFFSET) +
        DHD_RADIO_INSTANCE_COMMON_B_DATA_ADDRESS - sizeof(RUNNER_COMMON));
    radio_instance_entry_ptr = &radio_instance_data_ptr->entry[radio_idx];

	/* Invalidate lkp entries*/
	for (i = 0; i < RDD_DS_DHD_FLOW_RING_CACHE_LKP_TABLE_SIZE; i++) {
		ds_lkp_table_ptr = (RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_DTS *)RDD_DS_DHD_FLOW_RING_CACHE_LKP_TABLE_PTR() + i;
		us_lkp_table_ptr = (RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_DTS *)RDD_US_DHD_FLOW_RING_CACHE_LKP_TABLE_PTR() + i;
		RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_INVALID_WRITE(1 , ds_lkp_table_ptr);
		RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_INVALID_WRITE(1 , us_lkp_table_ptr);
	}

    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_RX_POST_FR_BASE_PTR_WRITE(DHD_VTOP(init_cfg->rx_post_flow_ring_base_addr), radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_RX_COMPLETE_FR_BASE_PTR_WRITE(DHD_VTOP(init_cfg->rx_complete_flow_ring_base_addr), radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_TX_COMPLETE_FR_BASE_PTR_WRITE(DHD_VTOP(init_cfg->tx_complete_flow_ring_base_addr), radio_instance_entry_ptr);

#if !defined FIRMWARE_INIT && (defined(DSL_63138) || defined(DSL_63148) || defined(CM3390) || defined(WL4908))
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_R2D_WR_FR_DESC_BASE_PTR_WRITE(init_cfg->r2d_wr_arr_base_phys_addr, radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_D2R_RD_FR_DESC_BASE_PTR_WRITE(init_cfg->d2r_rd_arr_base_phys_addr, radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_R2D_RD_FR_DESC_BASE_PTR_WRITE(init_cfg->r2d_rd_arr_base_phys_addr, radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_D2R_WR_FR_DESC_BASE_PTR_WRITE(init_cfg->d2r_wr_arr_base_phys_addr, radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_TX_POST_MGMT_FR_BASE_PTR_WRITE(init_cfg->tx_post_mgmt_arr_base_phys_addr, radio_instance_entry_ptr);

    /* Used by rdd_dhd_helper_shell.c. In order to support both 64 bit and 32 bit host, we have to do this type of casting */
    radio_instance_entry_ptr->tx_post_mgmt_fr_base_ptr_rdd_only_low = (uint32_t)((uint64_t)(uintptr_t)init_cfg->tx_post_mgmt_arr_base_addr & 0xffffffff);
    radio_instance_entry_ptr->tx_post_mgmt_fr_base_ptr_rdd_only_high = (uint32_t)((uint64_t)(uintptr_t)init_cfg->tx_post_mgmt_arr_base_addr >> 32);
#else
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_R2D_WR_FR_DESC_BASE_PTR_WRITE(DHD_VTOP(init_cfg->r2d_wr_arr_base_addr), radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_D2R_RD_FR_DESC_BASE_PTR_WRITE(DHD_VTOP(init_cfg->d2r_rd_arr_base_addr), radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_R2D_RD_FR_DESC_BASE_PTR_WRITE(DHD_VTOP(init_cfg->r2d_rd_arr_base_addr), radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_D2R_WR_FR_DESC_BASE_PTR_WRITE(DHD_VTOP(init_cfg->d2r_wr_arr_base_addr), radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_TX_POST_MGMT_FR_BASE_PTR_WRITE(DHD_VTOP(init_cfg->tx_post_mgmt_arr_base_addr), radio_instance_entry_ptr);
#endif

#if !defined(FIRMWARE_INIT)
#if defined(DHD_RX_POST_HOST)
    g_dhd_rx_post_priv.ring[radio_idx].ring_base = init_cfg->rx_post_flow_ring_base_addr;
    g_dhd_rx_post_priv.ring[radio_idx].wr_idx_addr = (uint16_t *)init_cfg->r2d_wr_arr_base_addr + 1;
    g_dhd_rx_post_priv.ring[radio_idx].rd_idx_addr = (uint16_t *)init_cfg->r2d_rd_arr_base_addr + 1;
#else
    g_dhd_rx_post_ring_priv[radio_idx].wr_idx_addr = (uint16_t *)init_cfg->r2d_wr_arr_base_addr + 1;
#endif
#endif

    rdd_dhd_tx_post_descr_init((uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_DHD_TX_POST_FLOW_RING_BUFFER_ADDRESS));
    rdd_dhd_tx_post_descr_init((uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_DHD_TX_POST_FLOW_RING_BUFFER_ADDRESS));

    rxp_flring_buffer_ptr = (RDD_DHD_RX_POST_FLOW_RING_BUFFER_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + DHD_RX_POST_FLOW_RING_BUFFER_ADDRESS);
    rxp_flring_entry_ptr =  &rxp_flring_buffer_ptr->entry[radio_idx].dhd_rx_post_descriptor;
    rdd_rx_post_descr_init(radio_idx, (uint8_t*)rxp_flring_entry_ptr, 0, 0);

    rdd_dhd_mode_enable(radio_idx, enable);

    /* Update RX_post WR index both in SRAM and DDR. No need doorbell DHD that there are buffers available in RX Post.*/
#if defined(DSL_63138) || defined(DSL_63148) || defined(CM3390) || defined(WL4908)
    /* for all the ARM based, little endian platform, we need to swap from little endian
     * to big endian, then the following "RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_RX_POST_R2D_INDEX_WRITE"
     * call will do another swapping but to little endian and write it... kind of redundant */
    rx_post_last_wr_idx = __swap2bytes(DHD_RX_POST_FLOW_RING_SIZE - 1);
#else
    rx_post_last_wr_idx = cpu_to_le16(DHD_RX_POST_FLOW_RING_SIZE - 1);
#endif
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_RX_POST_R2D_INDEX_WRITE(rx_post_last_wr_idx, radio_instance_entry_ptr);

#if defined(DHD_RX_POST_HOST) && !defined(FIRMWARE_INIT)
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_RX_POST_R2D_INDEX_WRITE(0, radio_instance_entry_ptr);
    if (enable)
    {
        g_dhd_rx_post_priv.ring[radio_idx].wr_idx = DHD_RX_POST_FLOW_RING_SIZE - 1;
        g_dhd_rx_post_priv.ring[radio_idx].mb_int_mapped = (uint32_t *)ioremap(init_cfg->dongle_wakeup_register, 4);
    }
    else
        iounmap((void *)g_dhd_rx_post_priv.ring[radio_idx].mb_int_mapped);
#endif

    radio_instance_data_a_ptr = (RDD_DHD_RADIO_INSTANCE_COMMON_A_DATA_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + DHD_RADIO_INSTANCE_COMMON_A_DATA_ADDRESS);
    radio_instance_entry_a_ptr = &radio_instance_data_a_ptr->entry[radio_idx];

    RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_DS_DHD_DOORBELL_WRITE(init_cfg->dongle_wakeup_register, radio_instance_entry_a_ptr);
    if (!enable)
    {
        /* Reset the rest of the fields */
        RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_DS_RD_FR_R2D_INDEXES_WRITE(0, radio_instance_entry_a_ptr);
        RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_DS_WR_FR_R2D_INDEXES_WRITE(0, radio_instance_entry_a_ptr);
        RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_TX_RD_FR_D2R_INDEXES_WRITE(0, radio_instance_entry_a_ptr);
        RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_TX_WR_FR_D2R_INDEXES_WRITE(0, radio_instance_entry_a_ptr);
        RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_TX_COMPLETE_PACKET_COUNTER_WRITE(0, radio_instance_entry_a_ptr);
        RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_RX_RD_FR_D2R_INDEXES_WRITE(0, radio_instance_entry_ptr);
        RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_RX_WR_FR_D2R_INDEXES_WRITE(0, radio_instance_entry_ptr);
    }
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_US_DHD_DOORBELL_WRITE(init_cfg->dongle_wakeup_register, radio_instance_entry_ptr);
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_ADD_LLCSNAP_HEADER_WRITE(init_cfg->add_llcsnap_header, radio_instance_entry_ptr);

#if 0
    for (i = 0, rx_post_ptr = (uintptr_t)init_cfg->rx_post_flow_ring_base_addr; enable && i < DHD_RX_POST_FLOW_RING_SIZE; i++)
    {
        rdd_rx_post_descr_dump(rx_post_ptr);
        rx_post_ptr += sizeof(RDD_DHD_RX_POST_DESCRIPTOR_DTS);
    }
#endif

    rdd_dhd_helper_tx_post_buffers_threshold_set();

    return 0;
}

void rdd_dhd_helper_flow_ring_flush(uint32_t radio_idx, uint32_t flow_ring_idx)
{
    unsigned long flags;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);
#if defined(CM3390)
    _rdd_cpu_message_send(RDD_CPU_MESSAGE_DHD, RDD_CLUSTER_0, DHD_MSG_TYPE_FLOW_RING_FLUSH, flow_ring_idx | (radio_idx << 14), 0, 1);
#else
    rdd_cpu_tx_send_message(RDD_CPU_TX_MESSAGE_DHD_MESSAGE, PICO_RUNNER_A, RUNNER_PRIVATE_0_OFFSET,
        DHD_MSG_TYPE_FLOW_RING_FLUSH, flow_ring_idx | (radio_idx << 14), 0, 1);
#endif
    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
}

void rdd_dhd_helper_flow_ring_disable(uint32_t radio_idx, uint32_t flow_ring_idx)
{
    unsigned long flags;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);
#if defined(CM3390)
    _rdd_cpu_message_send(RDD_CPU_MESSAGE_DHD, RDD_CLUSTER_0, DHD_MSG_TYPE_FLOW_RING_SET_DISABLED, flow_ring_idx | (radio_idx << 14), 0, 0);
#else
    rdd_cpu_tx_send_message(RDD_CPU_TX_MESSAGE_DHD_MESSAGE, PICO_RUNNER_A, RUNNER_PRIVATE_0_OFFSET,
        DHD_MSG_TYPE_FLOW_RING_SET_DISABLED, flow_ring_idx | (radio_idx << 14), 0, 0);
#endif
    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
}

void rdd_dhd_helper_wakeup_information_get(rdpa_dhd_wakeup_info_t *wakeup_info)
{
    RUNNER_REGS_CFG_CPU_WAKEUP runner_cpu_wakeup_register;

#if defined(USE_SOC_BASE_ADDR)
    wakeup_info->tx_complete_wakeup_register = RUNNER_REGS_0_CFG_CPU_WAKEUP_ADDRESS + RDP_PHYS_BASE;
#else
#if (defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)) && !defined(FIRMWARE_INIT)
    wakeup_info->tx_complete_wakeup_register = (uint32_t)(RUNNER_REGS_0_CFG_CPU_WAKEUP_ADDRESS - RDP_BASE + RDP_PHYS_BASE);
#elif defined(CM3390)
    wakeup_info->tx_complete_wakeup_register = RUNNER_REGS_0_CFG_CPU_WAKEUP_ADDRESS  + RDP_3390_PHYS_BASE;
#else
    wakeup_info->tx_complete_wakeup_register = RUNNER_REGS_0_CFG_CPU_WAKEUP_ADDRESS;
#endif
#endif

#ifdef CM3390
    runner_cpu_wakeup_register.req_trgt = (DHD0_TX_COMPLETE_THREAD_NUMBER + wakeup_info->radio_idx) >> 5;
    runner_cpu_wakeup_register.thread_num = (DHD0_TX_COMPLETE_THREAD_NUMBER + wakeup_info->radio_idx) & 0x1f;
#else
    runner_cpu_wakeup_register.req_trgt = (DHD_TX_COMPLETE_FAST_A_THREAD_NUMBER + wakeup_info->radio_idx) >> 5;
    runner_cpu_wakeup_register.thread_num = (DHD_TX_COMPLETE_FAST_A_THREAD_NUMBER + wakeup_info->radio_idx) & 0x1f;
#endif
    runner_cpu_wakeup_register.urgent_req = 0;
    runner_cpu_wakeup_register.reserved0 = 0;
    MWRITE_32(&wakeup_info->tx_complete_wakeup_value, *(uint32_t *)&runner_cpu_wakeup_register);

#if defined(USE_SOC_BASE_ADDR)
    wakeup_info->rx_complete_wakeup_register = RUNNER_REGS_1_CFG_CPU_WAKEUP_ADDRESS + RDP_PHYS_BASE;
#else
#if (defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)) && !defined(FIRMWARE_INIT)
    wakeup_info->rx_complete_wakeup_register = (uint32_t)(RUNNER_REGS_1_CFG_CPU_WAKEUP_ADDRESS - RDP_BASE + RDP_PHYS_BASE);
#elif defined(CM3390)
    wakeup_info->rx_complete_wakeup_register = RUNNER_REGS_1_CFG_CPU_WAKEUP_ADDRESS  + RDP_3390_PHYS_BASE;
#else
    wakeup_info->rx_complete_wakeup_register = RUNNER_REGS_1_CFG_CPU_WAKEUP_ADDRESS;
#endif
#endif

    runner_cpu_wakeup_register.req_trgt = (DHD_RX_THREAD_NUMBER + wakeup_info->radio_idx) >> 5;
    runner_cpu_wakeup_register.thread_num = (DHD_RX_THREAD_NUMBER + wakeup_info->radio_idx) & 0x1f;
    runner_cpu_wakeup_register.urgent_req = 0;
    runner_cpu_wakeup_register.reserved0 = 0;
    MWRITE_32(&wakeup_info->rx_complete_wakeup_value, *(uint32_t *)&runner_cpu_wakeup_register);

#if defined(DHD_RX_POST_HOST) && !defined(FIRMWARE_INIT)
    /* this is kind of ugly, but this is possibly the only place that rdd_dhd_helper
     * gets triggered in a non-atomic situation in the initialization.  bdmf_task_create
     * which is kthread_run, needs to be called in a non-atomic situation. */
    if (g_dhd_rx_post_priv.is_initialized == 0)
    {
        g_dhd_rx_post_priv.work_avail = 0;
        init_waitqueue_head(&g_dhd_rx_post_priv.thread_wqh);
        if (bdmf_task_create("bcm_dhd_rxpost", BDMFSYS_DEFAULT_TASK_PRIORITY,
                             BDMFSYS_DEFAULT_TASK_STACK, rdd_dhd_rx_post_thread_func,
                             (void *)&g_dhd_rx_post_priv,
                             &g_dhd_rx_post_priv.rx_post_task) != 0)
        {
            printk("%s:%d:failed to create RxPost refill thread\n", __func__, __LINE__);
            return;
        }
        g_dhd_rx_post_priv.is_initialized = 1;
    }
#endif
}

int rdd_dhd_helper_dhd_complete_ring_create(uint32_t radio_idx, uint32_t ring_size)
{
    int rc = 0;
#if !defined(FIRMWARE_INIT)
    RDD_DHD_COMPLETE_RING_DESCRIPTOR_BUFFER_DTS *dhd_complete_desc_buffer_ptr;
    RDD_DHD_COMPLETE_RING_DESCRIPTOR_DTS *dhd_complete_desc_ptr;
    rdd_dhd_complete_ring_descriptor_t *pdesc = &g_dhd_complete_ring_desc[radio_idx];
    uint32_t i;
    bdmf_phys_addr_t phy_addr;
    uint8_t *ring_ptr;

    if (ring_size)
    {
        /* create an array of ring elements */
        if (pdesc->ring_base == 0)
        {
            pdesc->ring_base = rdp_mm_aligned_alloc(sizeof(RDD_DHD_COMPLETE_RING_ENTRY_DTS) * ring_size, &phy_addr);
            if (pdesc->ring_base)
            {
                /* Initialize RDD descriptor */
                pdesc->ring_size = ring_size;
                pdesc->ring_end = pdesc->ring_base + ((ring_size - 1) * sizeof(RDD_DHD_COMPLETE_RING_ENTRY_DTS));
                pdesc->ring_ptr = pdesc->ring_base;

                /* Initialize firmware descriptor */
                dhd_complete_desc_buffer_ptr = RDD_DHD_COMPLETE_RING_DESCRIPTOR_BUFFER_PTR();
                dhd_complete_desc_ptr = &dhd_complete_desc_buffer_ptr->entry[radio_idx];

                RDD_DHD_COMPLETE_RING_DESCRIPTOR_RING_SIZE_WRITE(ring_size, dhd_complete_desc_ptr);
                RDD_DHD_COMPLETE_RING_DESCRIPTOR_RING_BASE_WRITE(phy_addr, dhd_complete_desc_ptr);
                RDD_DHD_COMPLETE_RING_DESCRIPTOR_RING_END_WRITE(phy_addr + ((ring_size - 1) * sizeof(RDD_DHD_COMPLETE_RING_ENTRY_DTS)), dhd_complete_desc_ptr);
                RDD_DHD_COMPLETE_RING_DESCRIPTOR_RING_PTR_WRITE(phy_addr, dhd_complete_desc_ptr);

                /* Initialize the ring elements to be owned by Runner */
                for (i = 0, ring_ptr = pdesc->ring_ptr; i < ring_size; i++, ring_ptr += sizeof(RDD_DHD_COMPLETE_RING_ENTRY_DTS))
                {
                    RDD_DHD_COMPLETE_RING_ENTRY_RING_VALUE_WRITE(0, ring_ptr);
                    RDD_DHD_COMPLETE_RING_ENTRY_OWNERSHIP_WRITE(DHD_COMPLETE_OWNERSHIP_RUNNER, ring_ptr);
                }
            }
            else
                rc = BDMF_ERR_NOMEM;
        }
        else
            rc = BDMF_ERR_ALREADY;
    }
#endif

    return rc;
}

int rdd_dhd_helper_dhd_complete_ring_destroy(uint32_t radio_idx, uint32_t ring_size)
{
    int rc = 0;
#if !defined(FIRMWARE_INIT)
    RDD_DHD_COMPLETE_RING_DESCRIPTOR_BUFFER_DTS *dhd_complete_desc_buffer_ptr;
    RDD_DHD_COMPLETE_RING_DESCRIPTOR_DTS *dhd_complete_desc_ptr;
    rdd_dhd_complete_ring_descriptor_t *pdesc = &g_dhd_complete_ring_desc[radio_idx];

    if (ring_size)
    {
        /* create an array of ring elements */
        if (pdesc->ring_base)
        {
            rdp_mm_aligned_free((void *)pdesc->ring_base, sizeof(RDD_DHD_COMPLETE_RING_ENTRY_DTS) * ring_size);
            pdesc->ring_size = 0;
            pdesc->ring_base = 0;
            pdesc->ring_end = 0;
            pdesc->ring_ptr = 0;

            dhd_complete_desc_buffer_ptr = RDD_DHD_COMPLETE_RING_DESCRIPTOR_BUFFER_PTR();
            dhd_complete_desc_ptr = &dhd_complete_desc_buffer_ptr->entry[radio_idx];

            RDD_DHD_COMPLETE_RING_DESCRIPTOR_RING_SIZE_WRITE(0, dhd_complete_desc_ptr);
            RDD_DHD_COMPLETE_RING_DESCRIPTOR_RING_BASE_WRITE(0, dhd_complete_desc_ptr);
            RDD_DHD_COMPLETE_RING_DESCRIPTOR_RING_END_WRITE(0, dhd_complete_desc_ptr);
            RDD_DHD_COMPLETE_RING_DESCRIPTOR_RING_PTR_WRITE(0, dhd_complete_desc_ptr);
        }
        else
            rc = BDMF_ERR_ALREADY;
    }
#endif

    return rc;
}

uint16_t rdd_dhd_helper_ssid_tx_dropped_packets_get(uint32_t radio_idx, uint32_t ssid)
{
    uint16_t counter;

#if defined DSL_63138 || defined DSL_63148 || defined(WL4908) || defined(CM3390)
    rdd_2_bytes_counter_get(DHD_SSID_DROP_PACKET_GROUP + radio_idx, ssid, &counter);
#else
    rdd_2_bytes_counter_get(DHD_SSID_DROP_PACKET_GROUP + radio_idx, ssid, 0, &counter);
#endif

    return counter;
}
#endif
#endif

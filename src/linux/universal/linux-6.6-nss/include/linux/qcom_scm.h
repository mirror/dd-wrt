/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2010-2015, 2018-2019 The Linux Foundation. All rights reserved.
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
 * Copyright (C) 2015 Linaro Ltd.
 */
#ifndef __QCOM_SCM_H
#define __QCOM_SCM_H

#include <linux/err.h>
#include <linux/types.h>
#include <linux/cpumask.h>

#define QCOM_SCM_VERSION(major, minor)	(((major) << 16) | ((minor) & 0xFF))
#define QCOM_SCM_CPU_PWR_DOWN_L2_ON	0x0
#define QCOM_SCM_CPU_PWR_DOWN_L2_OFF	0x1
#define QCOM_SCM_HDCP_MAX_REQ_CNT	5

#define QTI_SCM_DERIVE_KEY				0xA
#define QTI_SCM_DERIVE_KEY_PARAM_ID		0xD

#define QTI_TZ_DIAG_LOG_ENCR_ID		0x0
#define QTI_TZ_QSEE_LOG_ENCR_ID		0x1
#define QTI_TZ_LOG_NO_UPDATE		-6
#define QTI_SCM_SVC_FUSE		0x8
#define QTI_TRYBIT			BIT(12)

#define QCOM_BREAK_Q6			0x1

#define MAX_FUSE_ADDR_SIZE		0x8
struct fuse_payload {
	uint32_t fuse_addr;
	uint32_t lsb_val;
	uint32_t msb_val;
};

enum qseecom_qceos_cmd_id {
	QSEOS_APP_START_COMMAND = 0x01,
	QSEOS_APP_SHUTDOWN_COMMAND,
	QSEOS_APP_LOOKUP_COMMAND,
	QSEOS_REGISTER_LISTENER,
	QSEOS_DEREGISTER_LISTENER,
	QSEOS_CLIENT_SEND_DATA_COMMAND,
	QSEOS_LISTENER_DATA_RSP_COMMAND,
	QSEOS_LOAD_EXTERNAL_ELF_COMMAND,
	QSEOS_UNLOAD_EXTERNAL_ELF_COMMAND,
	QSEOS_CMD_MAX           = 0xEFFFFFFF,
	QSEE_LOAD_SERV_IMAGE_COMMAND = 0xB,
	QSEE_UNLOAD_SERV_IMAGE_COMMAND = 12,
	QSEE_APP_NOTIFY_COMMAND = 13,
	QSEE_REGISTER_LOG_BUF_COMMAND = 14
};

__packed struct qseecom_load_lib_ireq {
	uint32_t qsee_cmd_id;
	uint32_t mdt_len;		/* Length of the mdt file */
	uint32_t img_len;		/* Length of .bxx and .mdt files */
	phys_addr_t phy_addr;		/* phy addr of the start of image */
};

#define MAX_APP_NAME_SIZE		32
__packed struct qseecom_load_app_ireq {
	struct qseecom_load_lib_ireq load_ireq;
	char app_name[MAX_APP_NAME_SIZE];	/* application name*/
};

union qseecom_load_ireq {
	struct qseecom_load_lib_ireq load_lib_req;
	struct qseecom_load_app_ireq load_app_req;
};

struct qsee_notify_app {
	uint32_t cmd_id;
	phys_addr_t applications_region_addr;
	size_t applications_region_size;
};

__packed struct qseecom_client_send_data_v1_ireq {
	uint32_t qsee_cmd_id;
	uint32_t app_id;
	dma_addr_t req_ptr;
	uint32_t req_len;
	/** First 4 bytes should always be the return status */
	dma_addr_t rsp_ptr;
	uint32_t rsp_len;
};

__packed struct qseecom_client_send_data_v2_ireq {
	struct qseecom_client_send_data_v1_ireq send_data_ireq;
	uint64_t sglistinfo_ptr;
	uint32_t sglistinfo_len;
};

union qseecom_client_send_data_ireq {
	struct qseecom_client_send_data_v1_ireq v1;
	struct qseecom_client_send_data_v2_ireq v2;
};

__packed struct qseecom_unload_ireq {
	uint32_t qsee_cmd_id;
	uint32_t app_id;
};

enum qseecom_command_scm_resp_type {
	QSEOS_APP_ID = 0xEE01,
	QSEOS_LISTENER_ID
};

__packed struct qseecom_command_scm_resp {
	unsigned long result;
	enum qseecom_command_scm_resp_type resp_type;
	unsigned long data;
};

struct qsee_reg_log_buf_req {
	uint32_t qsee_cmd_id;
	phys_addr_t phy_addr;
	uint64_t len;
};

struct tzdbg_log_pos_t {
	uint16_t wrap;
	uint16_t offset;
};

struct qtidbg_log_t {
	struct tzdbg_log_pos_t log_pos;
	uint8_t log_buf[];
};

struct qcom_scm_hdcp_req {
	u32 addr;
	u32 val;
};

struct qcom_scm_vmperm {
	int vmid;
	int perm;
};

struct fuse_blow {
	dma_addr_t address;
	size_t size;
	unsigned long *status;
};

enum qcom_scm_ocmem_client {
	QCOM_SCM_OCMEM_UNUSED_ID = 0x0,
	QCOM_SCM_OCMEM_GRAPHICS_ID,
	QCOM_SCM_OCMEM_VIDEO_ID,
	QCOM_SCM_OCMEM_LP_AUDIO_ID,
	QCOM_SCM_OCMEM_SENSORS_ID,
	QCOM_SCM_OCMEM_OTHER_OS_ID,
	QCOM_SCM_OCMEM_DEBUG_ID,
};

enum qcom_scm_sec_dev_id {
	QCOM_SCM_MDSS_DEV_ID    = 1,
	QCOM_SCM_OCMEM_DEV_ID   = 5,
	QCOM_SCM_PCIE0_DEV_ID   = 11,
	QCOM_SCM_PCIE1_DEV_ID   = 12,
	QCOM_SCM_GFX_DEV_ID     = 18,
	QCOM_SCM_UFS_DEV_ID     = 19,
	QCOM_SCM_ICE_DEV_ID     = 20,
};

enum qcom_scm_ice_cipher {
	QCOM_SCM_ICE_CIPHER_AES_128_XTS = 0,
	QCOM_SCM_ICE_CIPHER_AES_128_CBC = 1,
	QCOM_SCM_ICE_CIPHER_AES_128_ECB = 2,
	QCOM_SCM_ICE_CIPHER_AES_256_XTS = 3,
	QCOM_SCM_ICE_CIPHER_AES_256_CBC = 4,
	QCOM_SCM_ICE_CIPHER_AES_256_ECB = 5,
};

enum ice_cryto_algo_mode {
	ICE_CRYPTO_ALGO_MODE_HW_AES_ECB = 0x0,
	ICE_CRYPTO_ALGO_MODE_HW_AES_XTS = 0x3,
};

enum ice_crpto_key_size {
	ICE_CRYPTO_KEY_SIZE_HW_128 = 0x0,
	ICE_CRYPTO_KEY_SIZE_HW_256 = 0x2,
};

#define QCOM_SCM_VMID_HLOS       0x3
#define QCOM_SCM_VMID_MSS_MSA    0xF
#define QCOM_SCM_VMID_WLAN       0x18
#define QCOM_SCM_VMID_WLAN_CE    0x19
#define QCOM_SCM_PERM_READ       0x4
#define QCOM_SCM_PERM_WRITE      0x2
#define QCOM_SCM_PERM_EXEC       0x1
#define QCOM_SCM_PERM_RW (QCOM_SCM_PERM_READ | QCOM_SCM_PERM_WRITE)
#define QCOM_SCM_PERM_RWX (QCOM_SCM_PERM_RW | QCOM_SCM_PERM_EXEC)

#define QCOM_SCM_OWM_FUSE_CMD_ID   0x22
#define FUSEPROV_SUCCESS           0x0
#define QCOM_SCM_SVC_FUSE          0x8
#define FUSEPROV_INVALID_HASH      0x9
#define FUSEPROV_SECDAT_LOCK_BLOWN 0xB
#define QCOM_KERNEL_AUTH_CMD       0x15
#define TZ_BLOW_FUSE_SECDAT        0x20
#define QCOM_KERNEL_META_AUTH_CMD  0x23

#define QTI_OWNER_QSEE_OS		50
#define QTI_OWNER_TZ_APPS		48
#define QTI_SVC_CRYPTO			10
#define QTI_SVC_APP_MGR			1 /* Application Management */
#define QTI_SVC_APP_ID_PLACEHOLDER	0 /* SVC bits will contain App ID */

#define QTI_CMD_NOTIFY_REGION_ID	0x05
#define QTI_CMD_REGISTER_LOG_BUF	0x06
#define QTI_CMD_LOAD_LIB		0x07
#define QTI_CMD_UNLOAD_LIB		0x08
#define QTI_CMD_LOAD_APP_ID		0x01
#define QTI_CMD_SEND_DATA_ID		0x01
#define QTI_CMD_UNLOAD_APP_ID		0x02
#define QTI_ARMv8_CMD_REMOVE_XPU	0x09

#define QTI_SYSCALL_CREATE_SMC_ID(o, s, f) \
	((uint32_t)((((o & 0x3f) << 24) | (s & 0xff) << 8) | (f & 0xff)))

extern int qcom_fuseipq_scm_call(u32 svc_id, u32 cmd_id,
				 void *cmd_buf, size_t size);
extern int qcom_qfprom_write_version(uint32_t sw_type,
				     uint32_t value,
				     uint32_t qfprom_ret_ptr);
extern int qcom_qfprom_read_version(uint32_t sw_type,
				    uint32_t value,
				    uint32_t qfprom_ret_ptr);
extern int qcom_qfprom_show_authenticate(void);
extern bool qcom_scm_is_available(void);

extern int qcom_scm_set_cold_boot_addr(void *entry);
extern int qcom_scm_set_warm_boot_addr(void *entry);
extern void qcom_scm_cpu_power_down(u32 flags);
extern int qcom_scm_set_remote_state(u32 state, u32 id);
extern int qcom_scm_derive_and_share_key(uint32_t key_len, uint8_t *sw_context,
				u32 sw_context_len, uint8_t *derived_key,
				u32 derived_key_len);

struct qcom_scm_pas_metadata {
	void *ptr;
	dma_addr_t phys;
	ssize_t size;
};

extern int qcom_scm_pas_init_image(u32 peripheral, const void *metadata,
				   size_t size,
				   struct qcom_scm_pas_metadata *ctx);
void qcom_scm_pas_metadata_release(struct qcom_scm_pas_metadata *ctx);
extern int qcom_scm_pas_mem_setup(u32 peripheral, phys_addr_t addr,
				  phys_addr_t size);
extern long qcom_scm_is_feature_available(u32 feature_id);
extern int qcom_scm_break_q6_start(u32 reset_cmd_id);
extern int qcom_scm_pas_auth_and_reset(u32 peripheral);
extern int qcom_scm_pas_shutdown(u32 peripheral);
extern bool qcom_scm_pas_supported(u32 peripheral);
int qti_scm_int_radio_powerup(u32 peripheral);
int qti_scm_int_radio_powerdown(u32 peripheral);
int qti_scm_pdseg_memcpy_v2(u32 peripheral, int phno, dma_addr_t dma, int seg_cnt);

extern int qcom_scm_io_readl(phys_addr_t addr, unsigned int *val);
extern int qcom_scm_io_writel(phys_addr_t addr, unsigned int val);

extern bool qcom_scm_restore_sec_cfg_available(void);
extern int qcom_scm_regsave(void *scm_regsave, u32 buf_size);
extern int qcom_scm_restore_sec_cfg(u32 device_id, u32 spare);
extern int qcom_scm_iommu_secure_ptbl_size(u32 spare, size_t *size);
extern int qcom_scm_iommu_secure_ptbl_init(u64 addr, u32 size, u32 spare);
extern int qcom_scm_iommu_set_cp_pool_size(u32 spare, u32 size);
extern int qcom_scm_mem_protect_video_var(u32 cp_start, u32 cp_size,
					  u32 cp_nonpixel_start,
					  u32 cp_nonpixel_size);
extern int qcom_scm_assign_mem(phys_addr_t mem_addr, size_t mem_sz,
			       unsigned int *src,
			       const struct qcom_scm_vmperm *newvm,
			       unsigned int dest_cnt);

extern bool qcom_scm_ocmem_lock_available(void);
extern int qcom_scm_ocmem_lock(enum qcom_scm_ocmem_client id, u32 offset,
			       u32 size, u32 mode);
extern int qcom_scm_ocmem_unlock(enum qcom_scm_ocmem_client id, u32 offset,
				 u32 size);
extern int qcom_context_ice_sec(u32 type, u8 key_size, u8 algo_mode,
				u8 *data_ctxt, u32 data_ctxt_len, u8 *salt_ctxt, u32 salt_ctxt_len);
extern bool qcom_scm_ice_available(void);
extern bool qcom_scm_ice_hwkey_available(void);
extern int qcom_scm_ice_invalidate_key(u32 index);
extern int qcom_config_sec_ice(void *buf, int size);
extern int qcom_scm_ice_set_key(u32 index, const u8 *key, u32 key_size,
				enum qcom_scm_ice_cipher cipher,
				u32 data_unit_size);

extern bool qcom_scm_hdcp_available(void);
extern int qcom_scm_hdcp_req(struct qcom_scm_hdcp_req *req, u32 req_cnt,
			     u32 *resp);

extern int qcom_scm_iommu_set_pt_format(u32 sec_id, u32 ctx_num, u32 pt_fmt);
extern int qcom_scm_qsmmu500_wait_safe_toggle(bool en);

extern int qcom_scm_lmh_dcvsh(u32 payload_fn, u32 payload_reg, u32 payload_val,
			      u64 limit_node, u32 node_id, u64 version);
extern int qcom_scm_lmh_profile_change(u32 profile_id);
extern bool qcom_scm_lmh_dcvsh_available(void);
extern int qti_seccrypt_clearkey(void);
extern int qti_sec_crypt(void *buf, int size);
extern int qti_set_qcekey_sec(void *buf, int size);
extern int qcom_qcekey_release_xpu_prot(void);

extern int qti_scm_is_tz_log_encrypted(void);
extern int qti_scm_get_encrypted_tz_log(void *ker_buf, u32 buf_len, u32 log_id);
extern int qti_scm_is_tz_log_encryption_supported(void);
extern int qti_scm_tz_log(void *ker_buf, u32 buf_len);
extern int qti_scm_hvc_log(void *ker_buf, u32 buf_len);
extern int qti_qfprom_show_authenticate(void);
extern int qti_scm_get_smmustate(void);
extern int qti_scm_get_ecdsa_blob(u32 svc_id, u32 cmd_id, dma_addr_t nonce_buf,
		u32 nonce_buf_len, dma_addr_t ecdsa_buf, u32 ecdsa_buf_len,
		u32 *ecdsa_consumed_len);

extern int qcom_scm_get_device_attestation_ephimeral_key(void *key_buf,
				u32 key_buf_len, u32 *key_len);
extern int qcom_scm_get_device_attestation_response(void *req_buf,
				u32 req_buf_len, void *extclaim_buf, u32 extclaim_buf_len,
				void *resp_buf, u32 resp_buf_len, u32 *attest_resp_len);
extern int qcom_scm_get_device_provision_response(void *provreq_buf,
				u32 provreq_buf_len, void *provresp_buf, u32 provresp_buf_len,
				u32 *prov_resp_size);

extern bool qcom_scm_sec_auth_available(unsigned int scm_cmd_id);
extern int qcom_scm_get_ipq5332_fuse_list(struct fuse_payload *fuse, size_t size);
extern int qcom_sec_upgrade_auth(unsigned int scm_cmd_id,
				 unsigned int sw_type, unsigned int img_size,
				 unsigned int load_addr);
extern int qcom_sec_upgrade_auth_meta_data(unsigned int scm_cmd_id,unsigned int sw_type,
					   unsigned int img_size,unsigned int load_addr,
					   void* hash_addr,unsigned int hash_size);
extern int qcom_scm_enable_try_mode(void);
extern int qcom_read_dload_reg(void);
extern int qti_scm_qseecom_remove_xpu(void);
extern int qti_scm_qseecom_notify(struct qsee_notify_app *req,
				  size_t req_size,
				  struct qseecom_command_scm_resp *resp,
				  size_t resp_size);
extern int qti_scm_qseecom_load(uint32_t smc_id, uint32_t cmd_id,
				union qseecom_load_ireq *req, size_t req_size,
				struct qseecom_command_scm_resp *resp,
				size_t resp_size);
extern int qti_scm_qseecom_send_data(union qseecom_client_send_data_ireq *req,
				     size_t req_size,
				     struct qseecom_command_scm_resp *resp,
				     size_t resp_size);
extern int qti_scm_qseecom_unload(uint32_t smc_id, uint32_t cmd_id,
				  struct qseecom_unload_ireq *req,
				  size_t req_size,
				  struct qseecom_command_scm_resp *resp,
				  size_t resp_size);
extern int qti_scm_register_log_buf(struct device *dev,
				       struct qsee_reg_log_buf_req *request,
				       size_t req_size,
				       struct qseecom_command_scm_resp
				       *response, size_t resp_size);
extern int qti_scm_tls_hardening(uint32_t req_addr, uint32_t req_size,
				 uint32_t resp_addr, uint32_t resp_size,
				 u32 cmd_id);
extern int qti_scm_aes(uint32_t req_addr, uint32_t req_size, u32 cmd_id);
extern int qti_scm_aes_clear_key_handle(uint32_t key_handle, u32 cmd_id);
extern int qti_scm_tcsr_reg_write(u32 reg_addr, u32 value);

#endif

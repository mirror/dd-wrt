/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */
#ifndef _TMELCOM_IPC_H_
#define _TMELCOM_IPC_H_

#define TMEL_MAX_FUSE_ADDR_SIZE 8
#define SECBOOT_SW_ID_ROOTPD 0xD

#define TME_KDF_SW_CONTEXT_BYTES_MAX 128
#define TME_KDF_SALT_LABEL_BYTES_MAX 64

struct tmel_msg_param_type_buf_in {
	u32 buf;
	u32 buf_len;
};

struct tmel_msg_param_type_buf_out {
	u32 buf;
	u32 buf_len;
	u32 out_buf_len;
};

struct tmel_msg_param_type_buf_in_out {
	u32 buf;
	u32 buf_len;
	u32 out_buf_len;
};

struct tmel_fuse_payload {
	u32 fuse_addr;
	u32 lsb_val;
	u32 msb_val;
} __packed;

struct tmel_fuse_read_multiple_msg {
	u32 status;
	struct tmel_msg_param_type_buf_in_out fuse_read_data;
} __packed;

struct tmel_qwes_init_att_msg {
	u32 status;
	struct tmel_msg_param_type_buf_out rsp;
} __packed;

struct tmel_qwes_device_att_msg {
	u32 status;
	struct tmel_msg_param_type_buf_in req;
	struct tmel_msg_param_type_buf_in ext_claim;
	struct tmel_msg_param_type_buf_out rsp;
} __packed;

struct tmel_qwes_device_prov_msg {
	u32 status;
	struct tmel_msg_param_type_buf_in req;
	struct tmel_msg_param_type_buf_out rsp;
} __packed;

struct tmel_secboot_sec_auth_req {
	u32 sw_id;
	struct tmel_msg_param_type_buf_in elf_buf;
	struct tmel_msg_param_type_buf_in region_list;
	u32 relocate;
} __packed;

struct tmel_secboot_sec_auth_resp {
	u32 first_seg_addr;
	u32 first_seg_len;
	u32 entry_addr;
	u32 extended_error;
	u32 status;
} __packed;

struct tmel_secboot_sec_auth {
	struct tmel_secboot_sec_auth_req req;
	struct tmel_secboot_sec_auth_resp resp;
} __packed;

struct tmel_secboot_sec_auth_req_v2 {
	u32 sw_id;
	struct tmel_msg_param_type_buf_in elf_buf;
	struct tmel_msg_param_type_buf_in region_list;
	u32 relocate;
	u32 ns_integrity_check:1;
	u32 reserved:31;
	struct tmel_msg_param_type_buf_in reserved_buf;
} __packed;

struct tmel_secboot_sec_auth_resp_v2 {
	u32 first_seg_addr;
	u32 first_seg_len;
	u32 entry_addr;
	u32 extended_error;
	u32 status;
	u32 key_handle;
} __packed;

struct tmel_secboot_sec_auth_v2 {
	struct tmel_secboot_sec_auth_req_v2 req;
	struct tmel_secboot_sec_auth_resp_v2 resp;
} __packed;

struct tmel_secboot_teardown_req {
	u32 sw_id;
	u32 secondary_sw_id;
} __packed;

struct tmel_secboot_teardown_resp {
	u32 status;
} __packed;

struct tmel_secboot_teardown {
	struct tmel_secboot_teardown_req req;
	struct tmel_secboot_teardown_resp resp;
} __packed;

struct tmel_licensing_check_msg {
	u32 status;
	struct tmel_msg_param_type_buf_in request;
	struct tmel_msg_param_type_buf_out response;
} __packed;

struct tmel_ttime_get_req_params {
	u32 status;
	struct tmel_msg_param_type_buf_out params;
} __packed;

struct tmel_ttime_set {
	u32 status;
	struct tmel_msg_param_type_buf_in ttime;
} __packed;

struct tmel_qwes_enf_hw_feat_msg {
	u32 status;
	struct tmel_msg_param_type_buf_in_out featid_buf;
	u32 hw_reg_inf_ver;
} __packed;

struct tmel_licensing_install {
	u32 status;
	struct tmel_msg_param_type_buf_in license;
	u32 flags;
	struct tmel_msg_param_type_buf_out identifier;
} __packed;

struct tmel_licensing_ToBeDel_licenses {
	u32 status;
	struct tmel_msg_param_type_buf_out toBeDelLicenses;
} __packed;

struct tmel_log_config {
	u8 component_id;
	u8 log_level;
};

struct tmel_log_get_message {
	u32 status;
	struct tmel_msg_param_type_buf_out log_buf;
} __packed;

struct tmel_log_set_config_message {
	u32 status;
	struct tmel_msg_param_type_buf_in log;
} __packed;

struct tmel_secure_io {
	u32 reg_addr;
	u32 reg_val;
} __packed;

struct tmel_secure_io_read {
	u32 status;
	struct tmel_msg_param_type_buf_in_out read_buf;
} __packed;

struct tmel_secure_io_write {
	u32 status;
	struct tmel_msg_param_type_buf_in write_buf;
} __packed;

struct tmel_get_arb_version_req {
	u32 sw_id;
} __packed;

struct tmel_get_arb_version_rsp {
	u8 oem_version;
	u8 qti_version;
	u8 oem_is_valid;
	u8 qti_is_valid;
	u32 status;
} __packed;

struct tmel_get_arb_version {
	struct tmel_get_arb_version_req req;
	struct tmel_get_arb_version_rsp rsp;
} __packed;

struct tmel_response_cbuffer {
	u32 data;
	u32 len;
	u32 len_used;
} __packed;

struct tmel_km_ecdh_ipkey_req {
	u32 feature_id;
	u32 key_id;
} __packed;

struct tmel_seq_status_rsp {
	u32 tmel_err_status;
	u32 seq_err_status;
	u32 seq_kp_err_status0;
	u32 seq_kp_err_status1;
	u32 seq_rsp_status;
} __packed;

struct tmel_km_ecdh_ipkey_rsp {
	struct tmel_response_cbuffer rsp_buf;
	u32 status;
	struct tmel_seq_status_rsp seq_status;
} __packed;

struct tmel_km_ecdh_ipkey_msg {
	struct tmel_km_ecdh_ipkey_req req;
	struct tmel_km_ecdh_ipkey_rsp rsp;
} __packed;

typedef enum tme_status_e {
	TME_STATUS_SUCCESS,		/* Success */
	TME_STATUS_FAILURE,		/* Generic Failure */
	TME_STATUS_INVALID_INPUT,	/* Invalid Input */
	TME_STATUS_MALFORMED_TOKEN,	/* Token is malformed */
	TME_STATUS_NOT_IMPLEMENTED,	/* Not Supported/Implemented */
	TME_STATUS_INVALID_MEMORY,	/* Invalid Memory Location */
	TME_STATUS_SMALL_OUTPUT_BUFFER,	/* Length of output buffer is smaller than expected */
	TME_STATUS_NOT_READY,		/* FW is not ready or set. */
	TME_STATUS_ME_DATA_UNAVAILABLE,	/* Expected ME Data is not available */
	TME_STATUS_UNKNOWN = 0x7FFFFFFF	/* Unknown */
} tme_status;

struct tmel_cbuffer {
	u32 buf;
	u32 buf_len;
} __packed;

struct tmel_cbuffer_resp {
	u32 buf;
	u32 length;
	u32 length_used;
} __packed;

struct tmel_wrapped_key {
	u32 key;
	u32 length;
} __packed;

struct tmel_plain_text_key {
	u32 buf;
	u32 buf_len;
} __packed;

struct tme_sequencer_status_resp {
	u32 tme_error_status;
	u32 seq_error_status;
	u32 seqkp_error_status0;
	u32 seqkp_error_status1;
	u32 seq_rsp_status;
} __packed;

struct tme_key_policy {
	u32 low;
	u32 high;
} __packed;

struct tme_kdf_spec {
	u32 kdf_algo;
	u32 input_key;
	u32 mix_key;
	u32 l2_key;
	struct tme_key_policy policy;
	u8 sw_context[TME_KDF_SW_CONTEXT_BYTES_MAX];
	u32 sw_context_len;
	u32 security_context;
	u8 salt_label[TME_KDF_SALT_LABEL_BYTES_MAX];
	u32 salt_label_len;
	u32 prf_digest_algo;
} __packed;

struct tmel_aes_derive_key_req {
	u32 key_id;
	struct tmel_cbuffer kdf_info;
	u32 cred_slot;
} __packed;

struct tmel_aes_derive_key_resp {
	u32 key_id;
	tme_status status;
	struct tme_sequencer_status_resp seq_status;
} __packed;

struct tmel_aes_derive_key_msg {
	struct tmel_aes_derive_key_req req;
	struct tmel_aes_derive_key_resp resp;
} __packed;

struct tmel_aes_clear_key_req {
	u32 key_id;
} __packed;

struct tmel_aes_clear_key_resp {
	tme_status status;
	struct tme_sequencer_status_resp seq_status;
} __packed;

struct tmel_aes_clear_key_msg {
	struct tmel_aes_clear_key_req req;
	struct tmel_aes_clear_key_resp resp;
} __packed;

struct tmel_aes_encrypt_req {
	u32 algo;
	u32 key_id;
	struct tmel_cbuffer in_aad;
	struct tmel_cbuffer in_plain_txt;
} __packed;

struct tmel_aes_encrypt_resp {
	struct tmel_cbuffer_resp out_aad;
	struct tmel_cbuffer_resp out_iv;
	struct tmel_cbuffer_resp out_tag;
	struct tmel_cbuffer_resp out_cipher_txt;
	tme_status status;
	struct tme_sequencer_status_resp seq_status;
} __packed;

struct tmel_aes_encrypt_msg {
	struct tmel_aes_encrypt_req req;
	struct tmel_aes_encrypt_resp resp;
} __packed;

struct tmel_aes_decrypt_req {
	u32 algo;
	u32 key_id;
	struct tmel_cbuffer in_aad;
	struct tmel_cbuffer in_iv;
	struct tmel_cbuffer in_tag;
	struct tmel_cbuffer in_cipher_txt;
} __packed;

struct tmel_aes_decrypt_resp {
	struct tmel_cbuffer_resp out_aad;
	struct tmel_cbuffer_resp out_plain_txt;
	tme_status status;
	struct tme_sequencer_status_resp seq_status;
} __packed;

struct tmel_aes_decrypt_msg {
	struct tmel_aes_decrypt_req req;
	struct tmel_aes_decrypt_resp resp;
} __packed;

struct tmel_aes_generate_key_req {
	u32 key_id;
	struct tme_key_policy policy;
	u32 cred_slot;
} __packed;

struct tmel_aes_generate_key_resp {
	u32 key_id;
	tme_status status;
	struct tme_sequencer_status_resp seq_status;
} __packed;

struct tmel_aes_generate_key_msg {
	struct tmel_aes_generate_key_req req;
	struct tmel_aes_generate_key_resp resp;
} __packed;

struct tmel_aes_import_key_req {
	u32 key_id;
	struct tme_key_policy key_policy;
	struct tmel_plain_text_key key_material;
	u32 cred_slot;
} __packed;

struct tmel_aes_import_key_resp {
	u32 key_id;
	tme_status status;
	struct tme_sequencer_status_resp seq_status;
} __packed;

struct tmel_aes_import_key_msg {
	struct tmel_aes_import_key_req req;
	struct tmel_aes_import_key_resp resp;
} __packed;

struct tmel_wrap_key_req {
	u32 key_id; /* ID of the key to be wrapped */
	u32 kw_key_id; /* The ID of the key wrapping key to be used to wrap the target key */
	u32 cred_slot;
} __packed;

struct tmel_wrap_key_resp {
	struct tmel_wrapped_key wrapped_key;
	tme_status status;
	struct tme_sequencer_status_resp seq_status;
} __packed;

struct tmel_wrap_key_msg {
	struct tmel_wrap_key_req req;
	struct tmel_wrap_key_resp resp;
} __packed;

struct tmel_unwrap_key_req {
	u32 key_id;
	u32 kw_key_id; /* The ID of the key to be used to unwrap the key */
	struct tmel_wrapped_key wrapped_key;
	u32 cred_slot;
} __packed;

struct tmel_unwrap_key_resp {
	u32 key_id;
	tme_status status;
	struct tme_sequencer_status_resp seq_status;
} __packed;

struct tmel_unwrap_key_msg {
	struct tmel_unwrap_key_req req;
	struct tmel_unwrap_key_resp resp;
} __packed;

typedef enum {
	TME_CURVE_P_NONE   = 0x00,
	TME_CURVE_P_192    = 0x01,
	TME_CURVE_P_224    = 0x02,
	TME_CURVE_P_256    = 0x03, /**< Eliptical Curve P-256 */
	TME_CURVE_P_384    = 0x04, /**< Eliptical Curve P-384 */
	TME_CURVE_P_521    = 0x05, /**< Eliptical Curve P-521 */
	TME_CURVE_P_256R1  = 0x06,
	TME_CURVE_ED_25519 = 0x07,
	TME_CURVE_ED_448   = 0x08,
	TME_CURVE_E_521    = 0x09,
	TME_CURVE_BP_192   = 0x11,
	TME_CURVE_BP_224   = 0x12,
	TME_CURVE_BP_256   = 0x13,
	TME_CURVE_BP_320   = 0x14,
	TME_CURVE_BP_384   = 0x15,
	TME_CURVE_BP_512   = 0x16,
	TME_CURVE_F_256V1  = 0x17,
	TME_CURVE_RSA_2048 = 0x20,
	TME_CURVE_RSA_3072 = 0x21,
	TME_CURVE_RSA_4096 = 0x22,
	TME_CURVE_MAX      = 0x7FFFFFFF,
} tme_curve_id;

struct tme_ecc_get_pubkey_req {
	tme_curve_id curve_id;
	u32 prv_key_id;
} __packed;

struct tme_ecc_get_pubkey_resp {
	struct tmel_cbuffer_resp pub_key;
	tme_status status;
	struct tme_sequencer_status_resp seq_status;
} __packed;

struct tme_ecc_get_pubkey_msg {
	struct tme_ecc_get_pubkey_req req;
	struct tme_ecc_get_pubkey_resp resp;
} __packed;

typedef enum {
	TME_HA_INVALID = 0x00,       /**< Hash Algorithm: INVALID */
	TME_HA_SHA256  = 0x02,       /**< Hash Algorithm: SHA256  */
	TME_HA_SHA384  = 0x03,       /**< Hash Algorithm: SHA384  */
	TME_HA_SHA512  = 0x05,       /**< Hash Algorithm: SHA512  */
	TME_HA_MAX     = 0x7FFFFFFF, /**< Hash Algorithm: MAX  */
} tme_hash_algo_id;

struct tme_ecc_sign_msg_req {
	tme_curve_id curve_id;
	u32 prv_key_id;
	tme_hash_algo_id hash_algo;
	struct tmel_cbuffer msg;
} __packed;

struct tme_ecc_sign_msg_resp {
	struct tmel_cbuffer_resp sig;
	tme_status status;
	struct tme_sequencer_status_resp seq_status;
} __packed;

struct tme_ecc_sign_msg_msg {
	struct tme_ecc_sign_msg_req req;
	struct tme_ecc_sign_msg_resp resp;
} __packed;

struct tme_ecc_verify_msg_req {
	tme_curve_id curve_id;
	struct tmel_cbuffer pub_key;
	tme_hash_algo_id hash_algo;
	struct tmel_cbuffer msg;
	struct tmel_cbuffer sig;
} __packed;

struct tme_ecc_verify_msg_resp {
	tme_status status;
	struct tme_sequencer_status_resp seq_status;
} __packed;

struct tme_ecc_verify_msg_msg {
	struct tme_ecc_verify_msg_req req;
	struct tme_ecc_verify_msg_resp resp;
} __packed;

struct tme_ecdh_shared_secret_msg_req {
	tme_curve_id curve_id;
	u32 prv_key_id;
	u32 shared_key_id;
	struct tmel_cbuffer pub_key2;
	struct tme_key_policy policy;
} __packed;

struct tme_ecdh_shared_secret_msg_resp {
	u32 shared_key_id;
	tme_status status;
	struct tme_sequencer_status_resp seq_status;
} __packed;

struct tme_ecdh_shared_secret_msg {
	struct tme_ecdh_shared_secret_msg_req req;
	struct tme_ecdh_shared_secret_msg_resp resp;
} __packed;

struct tme_sha_msg_req {
	tme_hash_algo_id hash_algo;
	struct tmel_cbuffer input;
	u32 hmac_key_id;
} __packed;

struct tme_sha_msg_resp {
	struct tmel_cbuffer_resp digest;
	tme_status status;
	struct tme_sequencer_status_resp seq_status;
} __packed;

struct tme_sha_msg {
	struct tme_sha_msg_req req;
	struct tme_sha_msg_resp resp;
} __packed;

struct tmel_update_arb_version_sw_id_list_req {
	struct tmel_cbuffer cbuffer;
} __packed;

struct tmel_update_arb_version_sw_id_list_rsp {
	u32 status;
} __packed;

struct tmel_update_arb_version_sw_id_list {
	struct tmel_update_arb_version_sw_id_list_req req;
	struct tmel_update_arb_version_sw_id_list_rsp rsp;
} __packed;

#ifdef CONFIG_QCOM_TMELCOM
int tmelcom_probed(void);
int tmelcom_init_attestation(u32 *key_buf, u32 key_buf_len, u32 *key_buf_size);
int tmelcom_qwes_getattestation_report(u32 *req_buf, u32 req_buf_len,
				       u32 *extclaim_buf, u32 extclaim_buf_len,
				       u32 *resp_buf, u32 resp_buf_len,
				       u32 *resp_buf_size);
int tmelcom_qwes_device_provision(u32 *req_buf, u32 req_buf_len, u32 *resp_buf,
				  u32 resp_buf_len, u32 *resp_buf_size);
int tmelcom_fuse_list_read(struct tmel_fuse_payload *fuse, size_t size);
int tmelcom_secboot_sec_auth(u32 sw_id, void *metadata, size_t size);
int tmelcom_secboot_sec_auth_v2(u32 sw_id, void *metadata, size_t size);
int tmelcom_secboot_teardown(u32 sw_id, u32 secondary_sw_id);
int tmelcom_licensing_check(void *cbor_req, u32 req_len, void *cbor_resp,
			    u32 resp_len, u32 *used_resp_len);
int tmelcom_ttime_get_req_params(void *params_buf, u32 buf_len, u32 *used_buf_len);
int tmelcom_ttime_set(void *ttime_buf, u32 buf_len);
int tmelcomm_qwes_enforce_hw_features(void *buf, u32 size);
int tmelcom_licensing_install(void *license_buf, u32 license_len, void *ident_buf,
			      u32 ident_len, u32 *ident_used_len, u32 *flags);
int tmelcom_licensing_get_toBeDel_licenses(void *toBeDelLic_buf, u32 toBeDelLic_len,
					   u32 *used_toBeDelLic_len);
int tmelcom_set_tmel_log_config(void *buf, u32 size);
int tmelcom_get_tmel_log(void *buf, u32 max_buf_size, u32 *size);
int tmelcom_secure_io_read(struct tmel_secure_io *buf, size_t size);
int tmelcom_secure_io_write(struct tmel_secure_io *buf, size_t size);
int tmelcomm_secboot_get_arb_version(u32 type, u32 *version);
int tmelcomm_secboot_update_arb_version_list(u32 *sw_id_list, size_t size);
int tmelcomm_get_ecc_public_key(u32 type, void *buf, u32 size, u32 *rsp_len);

int tmelcom_aes_derive_key(u32 key_id, dma_addr_t *dma_kdf_spec, u32 kdf_len,
			   u8 *key_handle);
int tmelcom_aes_clear_key(u32 handle);
int tmelcom_aes_encrypt(struct tmel_aes_encrypt_msg *msg, u32 size);
int tmelcom_aes_decrypt(struct tmel_aes_decrypt_msg *msg, u32 size);
int tmelcom_aes_generate_key(u32 key_id, struct tme_key_policy *policy,
			     u8 *key_handle);
int tmelcom_aes_import_key(u32 key_id, struct tme_key_policy *policy,
			   struct tmel_plain_text_key *key_material,
			   u8 *key_handle);
int tmelcom_wrap_key(u32 key_id, u32 kw_key_id, dma_addr_t *key, u32 len);
int tmelcom_unwrap_key(u32 kw_key_id, dma_addr_t *key, u32 len, u32 *key_id);
int tmelcomm_ecc_get_pubkey(u32 curve_id, u32 prv_key_id, dma_addr_t *dma_pub_key,
			    u32 buf_len, u32 *used_len, u32 ecc_algo);
int tmelcomm_ecc_sign_msg(u32 curve_id, u32 prv_key_id, u32 hash_algo, dma_addr_t *buf,
			  u32 buf_len, dma_addr_t *sig, u32 sig_len, u32 *used_len);
int tmelcomm_ecc_verify_msg(u32 curve_id, dma_addr_t *pub_key, u32 pub_key_len,
			    u32 hash_algo, dma_addr_t *buf, u32 buf_len,
			    dma_addr_t *sig, u32 sig_len);
int tmelcomm_ecdh_shared_secret_msg(u32 curve_id, u32 prv_key_id, u32 *shared_key_id,
				    dma_addr_t *pub_key2, u32 pub_key_len2,
				    struct tme_key_policy *policy);
int tmelcomm_hmac_sha_digest(u32 hash_algo, dma_addr_t *buf_in, u32 buf_in_len,
			     u32 key_id, dma_addr_t *buf_out, u32 buf_out_len,
			     u32 *used_len);
#else
static inline int tmelcom_probed(void)
{
	return -EOPNOTSUPP;
}

static inline int tmelcomm_get_ecc_public_key(u32 type, void *buf, u32 size,
					      u32 *rsp_len)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_fuse_list_read(struct tmel_fuse_payload *fuse,
					 size_t size)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_secboot_sec_auth(u32 sw_id, void *metadata,
					   size_t size)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_secboot_teardown(u32 sw_id, u32 secondary_sw_id)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_init_attestation(u32 *key_buf, u32 key_buf_len,
					   u32 *key_buf_size)
{
	return -EOPNOTSUPP;
}
static inline int tmelcom_qwes_getattestation_report(u32 *req_buf,
						     u32 req_buf_len,
						     u32 *extclaim_buf,
						     u32 extclaim_buf_len,
						     u32 *resp_buf,
						     u32 resp_buf_len,
						     u32 *resp_buf_size)
{
	return -EOPNOTSUPP;
}
static inline int tmelcom_qwes_device_provision(u32 *req_buf, u32 req_buf_len,
						u32 *resp_buf, u32 resp_buf_len,
						u32 *resp_buf_size)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_licensing_check(void *cbor_req, u32 req_len,
					  void *cbor_resp, u32 resp_len,
					  u32 *used_resp_len)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_ttime_get_req_params(void *params_buf, u32 buf_len,
					       u32 *used_buf_len)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_ttime_set(void *ttime_buf, u32 buf_len)
{
	return -EOPNOTSUPP;
}

static inline int tmelcomm_qwes_enforce_hw_features(void *buf, u32 size)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_licensing_install(void *license_buf, u32 license_len,
					    void *ident_buf, u32 ident_len,
					    u32 *ident_used_len, u32 *flags)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_licensing_get_toBeDel_licenses(void *toBeDelLic_buf,
							 u32 toBeDelLic_len,
							 u32 *used_toBeDelLic_len)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_set_tmel_log_config(void *buf, u32 size)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_get_tmel_log(void *buf, u32 max_buf_size,  u32 *size)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_secure_io_read(struct tmel_secure_io *buf, size_t size)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_secure_io_write(struct tmel_secure_io *buf, size_t size)
{
	return -EOPNOTSUPP;
}

static inline int tmelcomm_secboot_get_arb_version(u32 type, u32 *version)
{
	return -EOPNOTSUPP;
}

static inline int tmelcomm_secboot_update_arb_version_list(u32 *sw_id_list,
							   size_t size)
{
	return -EOPNOTSUPP;
}

static inline int tmelcomm_get_public_key(u32 type, void *buf, u32 *rsp_len)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_aes_derive_key(u32 key_id, dma_addr_t *dma_kdf_spec,
					 u32 kdf_len,
					 u8 *key_handle)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_aes_clear_key(u32 handle)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_aes_encrypt(struct tmel_aes_encrypt_msg *msg,
				      u32 size)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_aes_decrypt(struct tmel_aes_decrypt_msg *msg,
				      u32 size)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_aes_generate_key(u32 key_id, struct tme_key_policy *policy,
				    u8 *key_handle)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_aes_import_key(u32 key_id, struct tme_key_policy *policy,
				  struct tmel_plain_text_key *key_material,
				  u8 *key_handle)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_wrap_key(u32 key_id, u32 kw_key_id, dma_addr_t *key,
				   u32 len)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_unwrap_key(u32 kw_key_id, dma_addr_t *key, u32 len,
				     u32 *key_id)
{
	return -EOPNOTSUPP;
}

static inline int tmelcomm_ecc_get_pubkey(u32 curve_id, u32 prv_key_id,
					  dma_addr_t *dma_pub_key, u32 buf_len,
					  u32 *used_len, u32 ecc_algo)
{
	return -EOPNOTSUPP;
}

static inline int tmelcomm_ecc_sign_msg(u32 curve_id, u32 prv_key_id,
					   u32 hash_algo, dma_addr_t *buf,
					   u32 buf_len, dma_addr_t *sig,
					   u32 sig_len, u32 *used_len)
{
	return -EOPNOTSUPP;
}

static inline int tmelcomm_ecc_verify_msg(u32 curve_id, dma_addr_t *pub_key, u32 pub_key_len,
					     u32 hash_algo, dma_addr_t *buf, u32 buf_len,
					     dma_addr_t *sig, u32 sig_len)
{
	return -EOPNOTSUPP;
}

static inline int tmelcomm_ecdh_shared_secret_msg(u32 curve_id, u32 prv_key_id,
						  u32 *shared_key_id,
						  dma_addr_t *pub_key2,
						  u32 pub_key_len2,
						  struct tme_key_policy *policy)
{
	return -EOPNOTSUPP;
}

static inline int tmelcomm_hmac_sha_digest(u32 hash_algo, dma_addr_t *buf_in,
					   u32 buf_in_len, u32 key_id,
					   dma_addr_t *buf_out, u32 buf_out_len,
					   u32 *used_len)
{
	return -EOPNOTSUPP;
}
#endif /* CONFIG_QCOM_TMELCOM */
#endif /* _TMELCOM_IPC_H_ */

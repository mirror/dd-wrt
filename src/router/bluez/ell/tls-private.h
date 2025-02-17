/*
 * Embedded Linux library
 * Copyright (C) 2011-2014  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define TLS_MAX_VERSION	L_TLS_V12
#define TLS_MIN_VERSION	L_TLS_V10

enum tls_cipher_type {
	TLS_CIPHER_STREAM,
	TLS_CIPHER_BLOCK,
	TLS_CIPHER_AEAD,
};

struct tls_bulk_encryption_algorithm {
	enum tls_cipher_type cipher_type;
	union {
		enum l_cipher_type l_id;
		enum l_aead_cipher_type l_aead_id;
	};
	size_t key_length;
	size_t iv_length;
	size_t fixed_iv_length;
	size_t block_length;
	size_t auth_tag_length;
};

/*
 * Support the minimum required set of handshake hash types for the
 * Certificate Verify digital signature and the Finished PRF seed so we
 * don't have to accumulate all of messages full contents until the
 * Finished message.  If we're sent a hash of a different type (in TLS 1.2+)
 * and need to verify we'll give up.
 * SHA1 and MD5 are explicitly required by versions < 1.2 and 1.2 requires
 * that the Finished hash is the same as used for the PRF so we need to
 * keep at least the hashes our supported cipher suites specify for the PRF.
 */
enum handshake_hash_type {
	HANDSHAKE_HASH_SHA384,
	HANDSHAKE_HASH_SHA256,
	HANDSHAKE_HASH_MD5,
	HANDSHAKE_HASH_SHA1,
	__HANDSHAKE_HASH_COUNT,
};
#define HANDSHAKE_HASH_MAX_SIZE	48

struct tls_hash_algorithm {
	uint8_t tls_id;
	enum handshake_hash_type type;
	enum l_checksum_type l_id;
	const char *name;
};

extern const struct tls_hash_algorithm tls_handshake_hash_data[];

typedef bool (*tls_get_hash_t)(struct l_tls *tls,
				enum handshake_hash_type type,
				const uint8_t *data, size_t data_len,
				uint8_t *out, size_t *out_len);

struct tls_signature_algorithm {
	uint8_t id;

	bool (*validate_cert_key_type)(struct l_cert *cert);

	ssize_t (*sign)(struct l_tls *tls, uint8_t *out, size_t out_len,
			tls_get_hash_t get_hash,
			const uint8_t *data, size_t data_len);
	bool (*verify)(struct l_tls *tls, const uint8_t *in, size_t in_len,
			tls_get_hash_t get_hash,
			const uint8_t *data, size_t data_len);
};

struct tls_key_exchange_algorithm {
	bool need_ecc;
	bool need_ffdh;

	bool (*send_server_key_exchange)(struct l_tls *tls);
	void (*handle_server_key_exchange)(struct l_tls *tls,
						const uint8_t *buf, size_t len);

	bool (*send_client_key_exchange)(struct l_tls *tls);
	void (*handle_client_key_exchange)(struct l_tls *tls,
						const uint8_t *buf, size_t len);

	void (*free_params)(struct l_tls *tls);
};

struct tls_mac_algorithm {
	uint8_t id;
	enum l_checksum_type hmac_type;
	size_t mac_length;
};

struct tls_cipher_suite {
	uint8_t id[2];
	const char *name;
	size_t verify_data_length;

	struct tls_bulk_encryption_algorithm *encryption;
	struct tls_signature_algorithm *signature;
	struct tls_key_exchange_algorithm *key_xchg;
	struct tls_mac_algorithm *mac;
	enum l_checksum_type prf_hmac;
};

extern struct tls_cipher_suite *tls_cipher_suite_pref[];

struct tls_compression_method {
	int id;
	const char *name;
};

struct tls_hello_extension {
	const char *name;
	const char *short_name;
	uint16_t id;
	ssize_t (*client_write)(struct l_tls *tls, uint8_t *buf, size_t len);
	/* Handle a Client Hello extension (on server), can't be NULL */
	bool (*client_handle)(struct l_tls *tls,
				const uint8_t *buf, size_t len);
	/* Handle a Client Hello extension's absence (on server) */
	bool (*client_handle_absent)(struct l_tls *tls);
	ssize_t (*server_write)(struct l_tls *tls, uint8_t *buf, size_t len);
	/* Handle a Server Hello extension (on client) */
	bool (*server_handle)(struct l_tls *tls,
				const uint8_t *buf, size_t len);
	/* Handle a Server Hello extension's absence (on client) */
	bool (*server_handle_absent)(struct l_tls *tls);
};

extern const struct tls_hello_extension tls_extensions[];

struct tls_named_group {
	const char *name;
	uint16_t id;
	enum {
		TLS_GROUP_TYPE_EC,
		TLS_GROUP_TYPE_FF,
	} type;
	union {
		struct {
			const uint8_t *prime;
			size_t prime_len;
			unsigned int generator;
		} ff;
	};
};

enum tls_handshake_state {
	TLS_HANDSHAKE_WAIT_START,
	TLS_HANDSHAKE_WAIT_HELLO,
	TLS_HANDSHAKE_WAIT_CERTIFICATE,
	TLS_HANDSHAKE_WAIT_KEY_EXCHANGE,
	TLS_HANDSHAKE_WAIT_HELLO_DONE,
	TLS_HANDSHAKE_WAIT_CERTIFICATE_VERIFY,
	TLS_HANDSHAKE_WAIT_CHANGE_CIPHER_SPEC,
	TLS_HANDSHAKE_WAIT_FINISHED,
	TLS_HANDSHAKE_DONE,
};

enum tls_content_type {
	TLS_CT_CHANGE_CIPHER_SPEC	= 20,
	TLS_CT_ALERT			= 21,
	TLS_CT_HANDSHAKE		= 22,
	TLS_CT_APPLICATION_DATA		= 23,
};

enum tls_handshake_type {
	TLS_HELLO_REQUEST	= 0,
	TLS_CLIENT_HELLO	= 1,
	TLS_SERVER_HELLO	= 2,
	TLS_CERTIFICATE		= 11,
	TLS_SERVER_KEY_EXCHANGE	= 12,
	TLS_CERTIFICATE_REQUEST	= 13,
	TLS_SERVER_HELLO_DONE	= 14,
	TLS_CERTIFICATE_VERIFY	= 15,
	TLS_CLIENT_KEY_EXCHANGE	= 16,
	TLS_FINISHED		= 20,
};

struct l_tls {
	bool server;

	l_tls_write_cb_t tx, rx;
	l_tls_ready_cb_t ready_handle;
	l_tls_disconnect_cb_t disconnected;
	void *user_data;
	l_tls_debug_cb_t debug_handler;
	l_tls_destroy_cb_t debug_destroy;
	void *debug_data;
	char *cert_dump_path;
	enum l_tls_version min_version;
	enum l_tls_version max_version;

	struct l_queue *ca_certs;
	struct l_certchain *cert;
	struct l_key *priv_key;
	size_t priv_key_size;
	char **subject_mask;

	struct tls_cipher_suite **cipher_suite_pref_list;

	struct l_settings *session_settings;
	char *session_prefix;
	uint64_t session_lifetime;
	unsigned int session_count_max;
	l_tls_session_update_cb_t session_update_cb;
	void *session_update_user_data;

	bool in_callback;
	bool pending_destroy;

	/* Record layer */

	uint8_t *record_buf;
	int record_buf_len;
	int record_buf_max_len;
	bool record_flush;

	uint8_t *message_buf;
	int message_buf_len;
	int message_buf_max_len;
	enum tls_content_type message_content_type;

	/* Handshake protocol layer */

	enum tls_handshake_state state;
	struct l_checksum *handshake_hash[__HANDSHAKE_HASH_COUNT];
	uint8_t prev_digest[__HANDSHAKE_HASH_COUNT][HANDSHAKE_HASH_MAX_SIZE];

	enum l_tls_version client_version;
	enum l_tls_version negotiated_version;
	bool cert_requested, cert_sent;
	bool peer_authenticated;
	struct l_cert *peer_cert;
	struct l_key *peer_pubkey;
	size_t peer_pubkey_size;
	enum handshake_hash_type signature_hash;
	const struct tls_hash_algorithm *prf_hmac;
	const struct tls_named_group *negotiated_curve;
	const struct tls_named_group *negotiated_ff_group;

	uint8_t session_id[32];
	size_t session_id_size;
	uint8_t session_id_replaced[32];
	size_t session_id_size_replaced;
	bool session_id_new;
	uint8_t session_cipher_suite_id[2];
	uint8_t session_compression_method_id;
	char *session_peer_identity;
	bool session_resumed;

	struct {
		bool secure_renegotiation;
		/* Max .verify_data_length over supported cipher suites */
		uint8_t client_verify_data[12];
		uint8_t server_verify_data[12];
	} renegotiation_info;

	/* SecurityParameters current and pending */

	struct {
		struct tls_cipher_suite *cipher_suite;
		struct tls_compression_method *compression_method;
		uint8_t master_secret[48];
		uint8_t client_random[32];
		uint8_t server_random[32];
		/*
		 * Max key block size per 6.3 v1.1 is 136 bytes but if we
		 * allow AES_256_CBC_SHA256 with v1.0 we get 128 per section
		 * 6.3 v1.2 + two IVs of 32 bytes.
		 */
		uint8_t key_block[192];
		void *key_xchg_params;
	} pending;

	enum tls_cipher_type cipher_type[2];
	struct tls_cipher_suite *cipher_suite[2];
	union {
		struct l_cipher *cipher[2];
		struct l_aead_cipher *aead_cipher[2];
	};
	struct l_checksum *mac[2];
	size_t mac_length[2];
	size_t block_length[2];
	size_t record_iv_length[2];
	size_t fixed_iv_length[2];
	uint8_t fixed_iv[2][32];
	size_t auth_tag_length[2];
	uint64_t seq_num[2];
	/*
	 * Some of the key and IV parts of the "current" state are kept
	 * inside the cipher and mac states in the kernel so we don't
	 * duplicate them here.
	 */

	bool ready;
};

bool tls10_prf(const void *secret, size_t secret_len,
		const char *label,
		const void *seed, size_t seed_len,
		uint8_t *out, size_t out_len);

bool tls12_prf(enum l_checksum_type type,
		const void *secret, size_t secret_len,
		const char *label,
		const void *seed, size_t seed_len,
		uint8_t *out, size_t out_len);

void tls_disconnect(struct l_tls *tls, enum l_tls_alert_desc desc,
			enum l_tls_alert_desc local_desc);

void tls_tx_record(struct l_tls *tls, enum tls_content_type type,
			const uint8_t *data, size_t len);
bool tls_handle_message(struct l_tls *tls, const uint8_t *message,
			int len, enum tls_content_type type, uint16_t version);

#define TLS_HANDSHAKE_HEADER_SIZE	4

void tls_tx_handshake(struct l_tls *tls, int type, uint8_t *buf, size_t length);

bool tls_cipher_suite_is_compatible(struct l_tls *tls,
					const struct tls_cipher_suite *suite,
					const char **error);

/* Optionally limit allowed cipher suites to a custom set */
bool tls_set_cipher_suites(struct l_tls *tls, const char **suite_list);

void tls_generate_master_secret(struct l_tls *tls,
				const uint8_t *pre_master_secret,
				int pre_master_secret_len);

size_t tls_verify_data_length(struct l_tls *tls, unsigned int index);

const struct tls_named_group *tls_find_group(uint16_t id);
const struct tls_named_group *tls_find_ff_group(const uint8_t *prime,
						size_t prime_len,
						const uint8_t *generator,
						size_t generator_len);

ssize_t tls_write_signature_algorithms(struct l_tls *tls,
					uint8_t *buf, size_t len);
ssize_t tls_parse_signature_algorithms(struct l_tls *tls,
					const uint8_t *buf, size_t len);

int tls_parse_certificate_list(const void *data, size_t len,
				struct l_certchain **out_certchain);

#define TLS_DEBUG(fmt, args...)	\
	l_util_debug(tls->debug_handler, tls->debug_data, "%s:%i " fmt,	\
			__func__, __LINE__, ## args)
#define TLS_SET_STATE(new_state)	\
	do {	\
		TLS_DEBUG("New state %s",	\
				tls_handshake_state_to_str(new_state));	\
		tls->state = new_state;	\
	} while (0)
#define TLS_DISCONNECT(desc, local_desc, fmt, args...)	\
	do {	\
		TLS_DEBUG("Disconnect desc=%s local-desc=%s reason=" fmt,\
				l_tls_alert_to_str(desc),	\
				l_tls_alert_to_str(local_desc), ## args);\
		tls_disconnect(tls, desc, local_desc);	\
	} while (0)

#define TLS_VER_FMT		"1.%i"
#define TLS_VER_ARGS(version)	(((version) & 0xff) - 1)

const char *tls_handshake_state_to_str(enum tls_handshake_state state);

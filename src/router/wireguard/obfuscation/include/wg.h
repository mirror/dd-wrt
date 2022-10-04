#define WG_NOISE_PUBLIC_KEY_LEN 32
#define WG_COOKIE_LEN           16
#define WG_NOISE_TIMESTAMP_LEN  12
#define WG_NOISE_AUTHTAG_LEN    16
#define WG_COOKIE_NONCE_LEN     24

#define wg_noise_encrypted_len(plain_len) ((plain_len) + WG_NOISE_AUTHTAG_LEN)

struct wg_message_header {
	__le32 type;
};

struct wg_message_macs {
	u8 mac1[WG_COOKIE_LEN];
	u8 mac2[WG_COOKIE_LEN];
};

struct wg_message_handshake_initiation {
	struct wg_message_header header;
	__le32 sender_index;
	u8 unencrypted_ephemeral[WG_NOISE_PUBLIC_KEY_LEN];
	u8 encrypted_static[wg_noise_encrypted_len(WG_NOISE_PUBLIC_KEY_LEN)];
	u8 encrypted_timestamp[wg_noise_encrypted_len(WG_NOISE_TIMESTAMP_LEN)];
	struct wg_message_macs macs;
};

struct wg_message_handshake_response {
	struct wg_message_header header;
	__le32 sender_index;
	__le32 receiver_index;
	u8 unencrypted_ephemeral[WG_NOISE_PUBLIC_KEY_LEN];
	u8 encrypted_nothing[wg_noise_encrypted_len(0)];
	struct wg_message_macs macs;
};

struct wg_message_handshake_cookie {
	struct wg_message_header header;
	__le32 receiver_index;
	u8 nonce[WG_COOKIE_NONCE_LEN];
	u8 encrypted_cookie[wg_noise_encrypted_len(WG_COOKIE_LEN)];
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0) && LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0)
#include <linux/completion.h>
#include <linux/random.h>
#include <linux/errno.h>
struct rng_initializer {
	struct completion done;
	struct random_ready_callback cb;
};
static inline void rng_initialized_callback(struct random_ready_callback *cb)
{
	complete(&container_of(cb, struct rng_initializer, cb)->done);
}
static inline int wait_for_random_bytes(void)
{
	static bool rng_is_initialized = false;
	int ret;
	if (unlikely(!rng_is_initialized)) {
		struct rng_initializer rng = {
			.done = COMPLETION_INITIALIZER(rng.done),
			.cb = { .owner = THIS_MODULE, .func = rng_initialized_callback }
		};
		ret = add_random_ready_callback(&rng.cb);
		if (!ret) {
			ret = wait_for_completion_interruptible(&rng.done);
			if (ret) {
				del_random_ready_callback(&rng.cb);
				return ret;
			}
		} else if (ret != -EALREADY)
			return ret;
		rng_is_initialized = true;
	}
	return 0;
}
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0)
/* This is a disaster. Without this API, we really have no way of
 * knowing if it's initialized. We just return that it has and hope
 * for the best... */
static inline int wait_for_random_bytes(void)
{
	return 0;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0) || LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 320))
static inline int get_random_bytes_wait(void *buf, int nbytes)
{
	int ret = wait_for_random_bytes();
	if (unlikely(ret))
		return ret;
	get_random_bytes(buf, nbytes);
	return 0;
}
#endif

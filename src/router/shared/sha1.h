
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

typedef struct sha1_ctx_t {
	uint32_t count[2];
	uint32_t hash[5];
	uint32_t wbuf[16];
} sha1_ctx_t;

void sha1_begin(sha1_ctx_t *ctx);
void sha1_hash(const void *data, unsigned int length, sha1_ctx_t *ctx);

void *sha1_end(void *resbuf, sha1_ctx_t *ctx);

#define SHA1_DIGEST_SIZE 20

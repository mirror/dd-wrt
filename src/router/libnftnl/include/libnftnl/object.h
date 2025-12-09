#ifndef _LIBNFTNL_OBJECT_H_
#define _LIBNFTNL_OBJECT_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#include <libnftnl/common.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	NFTNL_OBJ_TABLE		= 0,
	NFTNL_OBJ_NAME,
	NFTNL_OBJ_TYPE,
	NFTNL_OBJ_FAMILY,
	NFTNL_OBJ_USE,
	NFTNL_OBJ_HANDLE,
	NFTNL_OBJ_USERDATA,
	NFTNL_OBJ_BASE		= 16,
	__NFTNL_OBJ_MAX
};
#define NFTNL_OBJ_MAX (__NFTNL_OBJ_MAX - 1)

enum {
	NFTNL_OBJ_CTR_PKTS	= NFTNL_OBJ_BASE,
	NFTNL_OBJ_CTR_BYTES,
	__NFTNL_OBJ_CTR_MAX,
};

enum {
	NFTNL_OBJ_QUOTA_BYTES	= NFTNL_OBJ_BASE,
	NFTNL_OBJ_QUOTA_CONSUMED,
	NFTNL_OBJ_QUOTA_FLAGS,
	__NFTNL_OBJ_QUOTA_MAX,
};

enum {
	NFTNL_OBJ_CT_HELPER_NAME = NFTNL_OBJ_BASE,
	NFTNL_OBJ_CT_HELPER_L3PROTO,
	NFTNL_OBJ_CT_HELPER_L4PROTO,
	__NFTNL_OBJ_CT_HELPER_MAX,
};

enum nftnl_cttimeout_array_tcp {
	NFTNL_CTTIMEOUT_TCP_SYN_SENT = 0,
	NFTNL_CTTIMEOUT_TCP_SYN_RECV,
	NFTNL_CTTIMEOUT_TCP_ESTABLISHED,
	NFTNL_CTTIMEOUT_TCP_FIN_WAIT,
	NFTNL_CTTIMEOUT_TCP_CLOSE_WAIT,
	NFTNL_CTTIMEOUT_TCP_LAST_ACK,
	NFTNL_CTTIMEOUT_TCP_TIME_WAIT,
	NFTNL_CTTIMEOUT_TCP_CLOSE,
	NFTNL_CTTIMEOUT_TCP_SYN_SENT2,
	NFTNL_CTTIMEOUT_TCP_RETRANS,
	NFTNL_CTTIMEOUT_TCP_UNACK,
	NFTNL_CTTIMEOUT_TCP_MAX
};

enum nftnl_cttimeout_array_udp {
	NFTNL_CTTIMEOUT_UDP_UNREPLIED = 0,
	NFTNL_CTTIMEOUT_UDP_REPLIED,
	NFTNL_CTTIMEOUT_UDP_MAX
};

#define NFTNL_CTTIMEOUT_ARRAY_MAX NFTNL_CTTIMEOUT_TCP_MAX

enum {
	NFTNL_OBJ_CT_TIMEOUT_L3PROTO = NFTNL_OBJ_BASE,
	NFTNL_OBJ_CT_TIMEOUT_L4PROTO,
	NFTNL_OBJ_CT_TIMEOUT_ARRAY,
	__NFTNL_OBJ_CT_TIMEOUT_MAX,
};

enum {
	NFTNL_OBJ_CT_EXPECT_L3PROTO	= NFTNL_OBJ_BASE,
	NFTNL_OBJ_CT_EXPECT_L4PROTO,
	NFTNL_OBJ_CT_EXPECT_DPORT,
	NFTNL_OBJ_CT_EXPECT_TIMEOUT,
	NFTNL_OBJ_CT_EXPECT_SIZE,
	__NFTNL_OBJ_CT_EXPECT_MAX,
};

enum {
	NFTNL_OBJ_LIMIT_RATE	= NFTNL_OBJ_BASE,
	NFTNL_OBJ_LIMIT_UNIT,
	NFTNL_OBJ_LIMIT_BURST,
	NFTNL_OBJ_LIMIT_TYPE,
	NFTNL_OBJ_LIMIT_FLAGS,
	__NFTNL_OBJ_LIMIT_MAX,
};

enum {
	NFTNL_OBJ_SYNPROXY_MSS	= NFTNL_OBJ_BASE,
	NFTNL_OBJ_SYNPROXY_WSCALE,
	NFTNL_OBJ_SYNPROXY_FLAGS,
	__NFTNL_OBJ_SYNPROXY_MAX,
};

enum {
	NFTNL_OBJ_TUNNEL_ID	= NFTNL_OBJ_BASE,
	NFTNL_OBJ_TUNNEL_IPV4_SRC,
	NFTNL_OBJ_TUNNEL_IPV4_DST,
	NFTNL_OBJ_TUNNEL_IPV6_SRC,
	NFTNL_OBJ_TUNNEL_IPV6_DST,
	NFTNL_OBJ_TUNNEL_IPV6_FLOWLABEL,
	NFTNL_OBJ_TUNNEL_SPORT,
	NFTNL_OBJ_TUNNEL_DPORT,
	NFTNL_OBJ_TUNNEL_FLAGS,
	NFTNL_OBJ_TUNNEL_TOS,
	NFTNL_OBJ_TUNNEL_TTL,
	NFTNL_OBJ_TUNNEL_OPTS,
	__NFTNL_OBJ_TUNNEL_MAX,
};

#define NFTNL_TUNNEL_TYPE	0
#define NFTNL_TUNNEL_BASE	4

#define NFTNL_TUNNEL_GENEVE_DATA_MAXLEN		127

enum nftnl_tunnel_type {
	NFTNL_TUNNEL_TYPE_VXLAN,
	NFTNL_TUNNEL_TYPE_ERSPAN,
	NFTNL_TUNNEL_TYPE_GENEVE,
};

enum {
	NFTNL_TUNNEL_VXLAN_GBP		= NFTNL_TUNNEL_BASE,
	__NFTNL_TUNNEL_VXLAN_MAX,
};

enum {
	NFTNL_TUNNEL_ERSPAN_VERSION	= NFTNL_TUNNEL_BASE,
	NFTNL_TUNNEL_ERSPAN_V1_INDEX,
	NFTNL_TUNNEL_ERSPAN_V2_HWID,
	NFTNL_TUNNEL_ERSPAN_V2_DIR,
	__NFTNL_TUNNEL_ERSPAN_MAX,
};

enum {
	NFTNL_TUNNEL_GENEVE_CLASS	= NFTNL_TUNNEL_BASE,
	NFTNL_TUNNEL_GENEVE_TYPE,
	NFTNL_TUNNEL_GENEVE_DATA,
};

struct nftnl_tunnel_opt;
struct nftnl_tunnel_opts;

struct nftnl_tunnel_opts *nftnl_tunnel_opts_alloc(enum nftnl_tunnel_type type);
int nftnl_tunnel_opts_add(struct nftnl_tunnel_opts *opts,
			  struct nftnl_tunnel_opt *opt);
void nftnl_tunnel_opts_free(struct nftnl_tunnel_opts *opts);

struct nftnl_tunnel_opt *nftnl_tunnel_opt_alloc(enum nftnl_tunnel_type type);
int nftnl_tunnel_opt_set(struct nftnl_tunnel_opt *opt, uint16_t type,
			 const void *data, uint32_t data_len);
const void *nftnl_tunnel_opt_get(const struct nftnl_tunnel_opt *ne, uint16_t attr);
const void *nftnl_tunnel_opt_get_data(const struct nftnl_tunnel_opt *ne,
				      uint16_t attr,
				      uint32_t *data_len);
uint8_t nftnl_tunnel_opt_get_u8(const struct nftnl_tunnel_opt *ne, uint16_t attr);
uint16_t nftnl_tunnel_opt_get_u16(const struct nftnl_tunnel_opt *ne, uint16_t attr);
uint32_t nftnl_tunnel_opt_get_u32(const struct nftnl_tunnel_opt *ne, uint16_t attr);
enum nftnl_tunnel_type nftnl_tunnel_opt_get_type(const struct nftnl_tunnel_opt *ne);
uint32_t nftnl_tunnel_opt_get_flags(const struct nftnl_tunnel_opt *ne);

enum {
	NFTNL_OBJ_SECMARK_CTX	= NFTNL_OBJ_BASE,
	__NFTNL_OBJ_SECMARK_MAX,
};

struct nftnl_obj;

struct nftnl_obj *nftnl_obj_alloc(void);
void nftnl_obj_free(const struct nftnl_obj *ne);

bool nftnl_obj_is_set(const struct nftnl_obj *ne, uint16_t attr);
void nftnl_obj_unset(struct nftnl_obj *ne, uint16_t attr);
int nftnl_obj_set_data(struct nftnl_obj *ne, uint16_t attr, const void *data,
		       uint32_t data_len);
void nftnl_obj_set(struct nftnl_obj *ne, uint16_t attr, const void *data) __attribute__((deprecated));
int nftnl_obj_set_u8(struct nftnl_obj *ne, uint16_t attr, uint8_t val);
int nftnl_obj_set_u16(struct nftnl_obj *ne, uint16_t attr, uint16_t val);
int nftnl_obj_set_u32(struct nftnl_obj *ne, uint16_t attr, uint32_t val);
int nftnl_obj_set_u64(struct nftnl_obj *obj, uint16_t attr, uint64_t val);
int nftnl_obj_set_str(struct nftnl_obj *ne, uint16_t attr, const char *str);
const void *nftnl_obj_get_data(const struct nftnl_obj *ne, uint16_t attr,
			       uint32_t *data_len);
const void *nftnl_obj_get(const struct nftnl_obj *ne, uint16_t attr);
uint8_t nftnl_obj_get_u8(const struct nftnl_obj *ne, uint16_t attr);
uint16_t nftnl_obj_get_u16(const struct nftnl_obj *obj, uint16_t attr);
uint32_t nftnl_obj_get_u32(const struct nftnl_obj *ne, uint16_t attr);
uint64_t nftnl_obj_get_u64(const struct nftnl_obj *obj, uint16_t attr);
const char *nftnl_obj_get_str(const struct nftnl_obj *ne, uint16_t attr);
int nftnl_obj_tunnel_opts_foreach(const struct nftnl_obj *ne,
				  int (*cb)(struct nftnl_tunnel_opt *ne, void *data),
				  void *data);

void nftnl_obj_nlmsg_build_payload(struct nlmsghdr *nlh,
				   const struct nftnl_obj *ne);
int nftnl_obj_nlmsg_parse(const struct nlmsghdr *nlh, struct nftnl_obj *ne);
int nftnl_obj_parse(struct nftnl_obj *ne, enum nftnl_parse_type type,
		    const char *data, struct nftnl_parse_err *err);
int nftnl_obj_parse_file(struct nftnl_obj *ne, enum nftnl_parse_type type,
			 FILE *fp, struct nftnl_parse_err *err);
int nftnl_obj_snprintf(char *buf, size_t size, const struct nftnl_obj *ne,
		       uint32_t type, uint32_t flags);
int nftnl_obj_fprintf(FILE *fp, const struct nftnl_obj *ne, uint32_t type,
		      uint32_t flags);

struct nftnl_obj_list;
struct nftnl_obj_list *nftnl_obj_list_alloc(void);
void nftnl_obj_list_free(struct nftnl_obj_list *list);
int nftnl_obj_list_is_empty(struct nftnl_obj_list *list);
void nftnl_obj_list_add(struct nftnl_obj *r, struct nftnl_obj_list *list);
void nftnl_obj_list_add_tail(struct nftnl_obj *r, struct nftnl_obj_list *list);
void nftnl_obj_list_del(struct nftnl_obj *t);
int nftnl_obj_list_foreach(struct nftnl_obj_list *table_list,
			   int (*cb)(struct nftnl_obj *t, void *data),
			   void *data);

struct nftnl_obj_list_iter;
struct nftnl_obj_list_iter *nftnl_obj_list_iter_create(struct nftnl_obj_list *l);
struct nftnl_obj *nftnl_obj_list_iter_next(struct nftnl_obj_list_iter *iter);
void nftnl_obj_list_iter_destroy(struct nftnl_obj_list_iter *iter);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _OBJ_H_ */

#ifndef SRC_COMMON_CONFIG_H_
#define SRC_COMMON_CONFIG_H_

/**
 * @file
 * Elements visible to both the kernel module and the userspace application, and
 * which they use to communicate with each other.
 */

#ifdef __KERNEL__
#include <net/netlink.h>
#else
#include <netlink/attr.h>
#endif
#include "common/types.h"

#define JOOLNL_FAMILY "Jool"
#define JOOLNL_MULTICAST_GRP_NAME "joold"

#define JOOLNL_HDR_MAGIC "jool"
#define JOOLNL_HDR_MAGIC_LEN 4

enum joolnl_operation {
	JNLOP_INSTANCE_FOREACH,
	JNLOP_INSTANCE_ADD,
	JNLOP_INSTANCE_HELLO,
	JNLOP_INSTANCE_RM,
	JNLOP_INSTANCE_FLUSH,

	JNLOP_ADDRESS_QUERY64,
	JNLOP_ADDRESS_QUERY46,

	JNLOP_STATS_FOREACH,

	JNLOP_GLOBAL_FOREACH,
	JNLOP_GLOBAL_UPDATE,

	JNLOP_EAMT_FOREACH,
	JNLOP_EAMT_ADD,
	JNLOP_EAMT_RM,
	JNLOP_EAMT_FLUSH,

	JNLOP_BL4_FOREACH,
	JNLOP_BL4_ADD,
	JNLOP_BL4_RM,
	JNLOP_BL4_FLUSH,

	JNLOP_POOL4_FOREACH,
	JNLOP_POOL4_ADD,
	JNLOP_POOL4_RM,
	JNLOP_POOL4_FLUSH,

	JNLOP_BIB_FOREACH,
	JNLOP_BIB_ADD,
	JNLOP_BIB_RM,

	JNLOP_SESSION_FOREACH,

	JNLOP_FILE_HANDLE,

	JNLOP_JOOLD_ADD,
	JNLOP_JOOLD_ADVERTISE,
	JNLOP_JOOLD_ACK,
};

enum joolnl_attr_root {
	JNLAR_ADDR_QUERY = 1,
	JNLAR_GLOBALS,
	JNLAR_BL4_ENTRIES,
	JNLAR_EAMT_ENTRIES,
	JNLAR_POOL4_ENTRIES,
	JNLAR_BIB_ENTRIES,
	JNLAR_SESSION_ENTRIES,
	JNLAR_OFFSET,
	JNLAR_OFFSET_U8,
	JNLAR_OPERAND,
	JNLAR_PROTO,
	JNLAR_ATOMIC_INIT,
	JNLAR_ATOMIC_END,
	JNLAR_COUNT,
#define JNLAR_MAX (JNLAR_COUNT - 1)
};

enum joolnl_attr_list {
	JNLAL_ENTRY = 1,
	JNLAL_COUNT,
#define JNLAL_MAX (JNLAL_COUNT - 1)
};

extern struct nla_policy joolnl_struct_list_policy[JNLAL_COUNT];
extern struct nla_policy joolnl_plateau_list_policy[JNLAL_COUNT];

#ifdef __KERNEL__
#define JOOLNL_ADDR6_POLICY { \
	.type = NLA_BINARY, \
	.len = sizeof(struct in6_addr), \
}
#define JOOLNL_ADDR4_POLICY { \
	.type = NLA_BINARY, \
	.len = sizeof(struct in_addr), \
}
#else
#define JOOLNL_ADDR6_POLICY { \
	.type = NLA_UNSPEC, \
	.minlen = sizeof(struct in6_addr), \
	.maxlen = sizeof(struct in6_addr), \
}
#define JOOLNL_ADDR4_POLICY { \
	.type = NLA_UNSPEC, \
	.minlen = sizeof(struct in_addr), \
	.maxlen = sizeof(struct in_addr), \
}
#endif

enum joolnl_attr_prefix {
	JNLAP_ADDR = 1,
	JNLAP_LEN,
	JNLAP_COUNT,
#define JNLAP_MAX (JNLAP_COUNT - 1)
};

extern struct nla_policy joolnl_prefix6_policy[JNLAP_COUNT];
extern struct nla_policy joolnl_prefix4_policy[JNLAP_COUNT];

enum joolnl_attr_taddr {
	JNLAT_ADDR = 1,
	JNLAT_PORT,
	JNLAT_COUNT,
#define JNLAT_MAX (JNLAT_COUNT - 1)
};

extern struct nla_policy joolnl_taddr6_policy[JNLAT_COUNT];
extern struct nla_policy joolnl_taddr4_policy[JNLAT_COUNT];

enum joolnl_attr_instance_entry {
	JNLAIE_NS = 1,
	JNLAIE_XF,
	JNLAIE_INAME,
	JNLAIE_COUNT,
#define JNLAIE_MAX (JNLAIE_COUNT - 1)
};

extern struct nla_policy joolnl_instance_entry_policy[JNLAIE_COUNT];

enum joolnl_attr_instance_status {
	JNLAIS_STATUS = 1,
	JNLAIS_COUNT,
#define JNLAIS_MAX (JNLAIS_COUNT - 1)
};

enum joolnl_attr_instance_add {
	JNLAIA_XF = 1,
	JNLAIA_POOL6,
	JNLAIA_COUNT,
#define JNLAIA_MAX (JNLAIA_COUNT - 1)
};

enum joolnl_attr_eam {
	JNLAE_PREFIX6 = 1,
	JNLAE_PREFIX4,
	JNLAE_COUNT,
#define JNLAE_MAX (JNLAE_COUNT - 1)
};

extern struct nla_policy eam_policy[JNLAE_COUNT];

enum joolnl_attr_pool4 {
	JNLAP4_MARK = 1,
	JNLAP4_ITERATIONS,
	JNLAP4_FLAGS,
	JNLAP4_PROTO,
	JNLAP4_PREFIX,
	JNLAP4_PORT_MIN,
	JNLAP4_PORT_MAX,
	JNLAP4_COUNT,
#define JNLAP4_MAX (JNLAP4_COUNT - 1)
};

extern struct nla_policy joolnl_pool4_entry_policy[JNLAP4_COUNT];

enum joolnl_attr_bib {
	JNLAB_SRC6 = 1,
	JNLAB_SRC4,
	JNLAB_PROTO,
	JNLAB_STATIC,
	JNLAB_COUNT,
#define JNLAB_MAX (JNLAB_COUNT - 1)
};

extern struct nla_policy joolnl_bib_entry_policy[JNLAB_COUNT];

enum joolnl_attr_session {
	JNLASE_SRC6 = 1,
	JNLASE_DST6,
	JNLASE_SRC4,
	JNLASE_DST4,
	JNLASE_PROTO,
	JNLASE_STATE,
	JNLASE_TIMER,
	JNLASE_EXPIRATION,
	JNLASE_COUNT,
#define JNLASE_MAX (JNLASE_COUNT - 1)
};

extern struct nla_policy joolnl_session_entry_policy[JNLASE_COUNT];

enum joolnl_attr_address_query {
	JNLAAQ_ADDR6 = 1,
	JNLAAQ_ADDR4,
	JNLAAQ_PREFIX6052,
	JNLAAQ_EAM,
	JNLAAQ_COUNT,
#define JNLAAQ_MAX (JNLAAQ_COUNT - 1)
};

enum joolnl_attr_global {
	/* Common */
	JNLAG_ENABLED = 1,
	JNLAG_POOL6,
	JNLAG_LOWEST_IPV6_MTU,
	JNLAG_DEBUG,
	JNLAG_RESET_TC,
	JNLAG_RESET_TOS,
	JNLAG_TOS,
	JNLAG_PLATEAUS,

	/* SIIT */
	JNLAG_COMPUTE_CSUM_ZERO,
	JNLAG_HAIRPIN_MODE,
	JNLAG_RANDOMIZE_ERROR_ADDR,
	JNLAG_POOL6791V6,
	JNLAG_POOL6791V4,

	/* NAT64 */
	JNLAG_DROP_BY_ADDR,
	JNLAG_DROP_EXTERNAL_TCP,
	JNLAG_DROP_ICMP6_INFO,
	JNLAG_SRC_ICMP6_BETTER,
	JNLAG_F_ARGS,
	JNLAG_HANDLE_RST,
	JNLAG_TTL_TCP_EST,
	JNLAG_TTL_TCP_TRANS,
	JNLAG_TTL_UDP,
	JNLAG_TTL_ICMP,
	JNLAG_BIB_LOGGING,
	JNLAG_SESSION_LOGGING,
	JNLAG_MAX_STORED_PKTS,

	/* joold */
	JNLAG_JOOLD_ENABLED,
	JNLAG_JOOLD_FLUSH_ASAP,
	JNLAG_JOOLD_FLUSH_DEADLINE,
	JNLAG_JOOLD_CAPACITY,
	JNLAG_JOOLD_MAX_PAYLOAD,
	JNLAG_JOOLD_MAX_SESSIONS_PER_PACKET,

	/* Needs to be last */
	JNLAG_COUNT,
#define JNLAG_MAX (JNLAG_COUNT - 1)
};

extern struct nla_policy siit_globals_policy[JNLAG_COUNT];
extern struct nla_policy nat64_globals_policy[JNLAG_COUNT];

enum joolnl_attr_error {
	JNLAERR_CODE = 1,
	JNLAERR_MSG,
	JNLAERR_COUNT,
#define JNLAERR_MAX (JNLAERR_COUNT - 1)
};

/** Is this packet an error report? */
#define JOOLNLHDR_FLAGS_ERROR (1 << 0)
/** Ignore certain validations? */
#define JOOLNLHDR_FLAGS_FORCE (1 << 1)
/** Skip removal of orphaned entries? */
#define JOOLNLHDR_FLAGS_QUICK (1 << 2)
/**
 * "Some data could not be included in this message. Please request it."
 * Named after the IPv6 fragment header flag, though it has nothing to do with
 * IP fragmentation.
 */
#define JOOLNLHDR_FLAGS_M (1 << 3)

typedef __u8 joolnlhdr_flags; /** See JOOLNLHDR_FLAGS_* above. */

/**
 * Prefix to all user-to-kernel messages.
 * Indicates what the rest of the message contains.
 *
 * Mind alignment on this structure.
 *
 * (Name follows kernel conventions: iphdr, ipv6hdr, tcphdr, udphdr, icmphdr,
 * nlmsghdr, genlmsghdr)
 */
struct joolnlhdr {
	/** Always "jool". (Without terminating character.) */
	char magic[JOOLNL_HDR_MAGIC_LEN];
	/** Jool's version. */
	__be32 version;

	/** enum xlator_type (Only relevant in requests from userspace) */
	__u8 xt;

	__u8 flags; /* joolnlhdr_flags */

	__u8 reserved1;
	__u8 reserved2;

	char iname[INAME_MAX_SIZE];
};

#define JOOLNL_HDRLEN NLMSG_ALIGN(sizeof(struct joolnlhdr))

struct config_prefix6 {
	bool set;
	/** Please note that this could be garbage; see above. */
	struct ipv6_prefix prefix;
};

struct config_prefix4 {
	bool set;
	/** Please note that this could be garbage; see above. */
	struct ipv4_prefix prefix;
};

/**
 * Issued during atomic configuration initialization.
 */
struct request_init {
	__u8 xf; /* enum xlator_framework */
};

struct instance_entry_usr {
	/* TODO (fine) find a way to turn this into a u64? */
	__u32 ns;
	__u8 xf; /* enum xlator_framework */
	char iname[INAME_MAX_SIZE];
};

enum instance_hello_status {
	/** Instance exists */
	IHS_ALIVE,
	/** Instance does not exist */
	IHS_DEAD,
};

enum iteration_flags {
	/**
	 * Is the iterations field relevant?
	 * (Irrelevant = "Ignore this; keep the old value.")
	 */
	ITERATIONS_SET = (1 << 0),
	/** Should Jool compute the iterations field automatically? */
	ITERATIONS_AUTO = (1 << 1),
	/** Remove iteration cap? */
	ITERATIONS_INFINITE = (1 << 2),
};

struct pool4_update {
	__u32 mark;
	__u32 iterations;
	__u8 flags;
	__u8 l4_proto;
};

enum address_translation_method {
	AXM_RFC6052,
	AXM_EAMT,
	AXM_RFC6791,
};

struct address_translation_entry {
	enum address_translation_method method;
	union {
		struct ipv6_prefix prefix6052;
		struct eamt_entry eam;
		/* The RFC6791 prefix is unused for now. */
	};
};

struct result_addrxlat64 {
	struct in_addr addr;
	struct address_translation_entry entry;
};

struct result_addrxlat46 {
	struct in6_addr addr;
	struct address_translation_entry entry;
};

enum f_args {
	F_ARGS_SRC_ADDR = (1 << 3),
	F_ARGS_SRC_PORT = (1 << 2),
	F_ARGS_DST_ADDR = (1 << 1),
	F_ARGS_DST_PORT = (1 << 0),
};

struct bib_config {
	/* These values are always measured in milliseconds. */
	struct {
		__u32 tcp_est;
		__u32 tcp_trans;
		__u32 udp;
		__u32 icmp;
	} ttl;

	bool bib_logging;
	bool session_logging;

	/** Use Address-Dependent Filtering? */
	bool drop_by_addr;
	/** Drop externally initiated (IPv4) TCP connections? */
	bool drop_external_tcp;

	__u32 max_stored_pkts;
};

#define JOOLD_MAX_PAYLOAD 2048

struct joold_config {
	/** Is joold enabled on this Jool instance? */
	bool enabled;

	/**
	 * true:  Whenever a session changes, packet it up and send it.
	 *        (Note: In theory, this might be more often than it seems.
	 *        It's not whenever a connection is initiated;
	 *        it's on every translated packet except ICMP errors.
	 *        In practice however, flushes are prohibited until the next
	 *        ACK (otherwise joold quickly saturates the kernel), so
	 *        sessions will end up queuing up even in this mode.)
	 *        This is the preferred method in active scenarios.
	 * false: Wait until we have enough sessions to fill a packet before
	 *        sending them.
	 *        (ACKs are still required, but expected to arrive faster.)
	 *        This is the preferred method in passive scenarios.
	 */
	bool flush_asap;

	/**
	 * The timer forcibly flushes the queue if this hasn't happened after
	 * this amount of milliseconds, regardless of the ACK and @flush_asap.
	 * This helps if an ACK is lost for some reason.
	 */
	__u32 flush_deadline;

	/**
	 * Maximim number of queuable entries.
	 * If this capacity is exceeded, Jool will have to start dropping
	 * sessions.
	 * This exists because it's theoretically possible for joold to not be
	 * able to catch up with the translating traffic, and there's not much
	 * we can do to recover if this happens.
	 */
	__u32 capacity;

	/** Deprecated as of 4.1.11, does nothing. */
	__u32 max_payload;

	/**
	 * Maximum number of sessions joold should send per packet.
	 *
	 * This exists because userspace joold sends sessions via UDP. UDP is
	 * rather packet-oriented, as opposed to stream-oriented, so it doesn't
	 * discover PMTU and instead tends to fragment when we send too many
	 * sessions per packet. Which is bad.
	 *
	 * So the user can tweak this number to prevent fragmentation.
	 *
	 * We should probably handle this ourselves but it sounds like a lot of
	 * code. (I guess I'm missing something.)
	 */
	__u32 max_sessions_per_pkt;
};

/**
 * A copy of the entire running configuration, excluding databases.
 */
struct jool_globals {

	/** Does the user wants this Jool instance to translate packets? */
	bool enabled;
	/** Print debug messages? */
	bool debug;

	/**
	 * BTW: NAT64 Jool can't do anything without pool6, so it validates that
	 * this is this set very early. Most NAT64-exclusive code should just
	 * assume that pool6.set is true.
	 */
	struct config_prefix6 pool6;

	/**
	 * "true" if the Traffic Class field of translated IPv6 headers should
	 * always be zeroized.
	 * Otherwise it will be copied from the IPv4 header's TOS field.
	 */
	bool reset_traffic_class;
	/**
	 * "true" if the Type of Service (TOS) field of translated IPv4 headers
	 * should always be set as "new_tos".
	 * Otherwise it will be copied from the IPv6 header's Traffic Class
	 * field.
	 */
	bool reset_tos;
	/**
	 * If "reset_tos" is "true", this is the value the translator will
	 * always write in the TOS field of translated IPv4 headers.
	 * If "reset_tos" is "false", then this doesn't do anything.
	 */
	__u8 new_tos;

	/**
	 * Smallest reachable IPv6 MTU.
	 *
	 * Because DF does not exist in IPv6, Jool must ensure that that any
	 * DF-disabled IPv4 packet translates into fragments sized this or less.
	 * Otherwise these packets might be black-holed.
	 */
	__u32 lowest_ipv6_mtu;

	/**
	 * If the translator detects the source of the incoming packet does not
	 * implement RFC 1191, these are the plateau values used to determine a
	 * likely path MTU for outgoing ICMPv6 fragmentation needed packets.
	 * The translator is supposed to pick the greatest plateau value that is
	 * less than the incoming packet's Total Length field.
	 */
	struct mtu_plateaus plateaus;

	union {
		struct {
			/**
			 * Amend the UDP checksum of incoming IPv4-UDP packets
			 * when it's zero? Otherwise these packets will be
			 * dropped (because they're illegal in IPv6).
			 */
			bool compute_udp_csum_zero;
			/**
			 * How should hairpinning be handled by EAM-translated
			 * packets.
			 * See @eam_hairpinning_mode.
			 */
			__u8 eam_hairpin_mode;
			/**
			 * Randomize choice of RFC6791 address?
			 * Otherwise it will be set depending on the incoming
			 * packet's Hop Limit.
			 * See https://github.com/NICMx/Jool/issues/130.
			 */
			bool randomize_error_addresses;

			/**
			 * Address used to represent a not translatable source
			 * address of an incoming packet.
			 */
			struct config_prefix6 rfc6791_prefix6;
			/**
			 * Address used to represent a not translatable source
			 * address of an incoming packet.
			 */
			struct config_prefix4 rfc6791_prefix4;

		} siit;
		struct {
			/** Filter ICMPv6 Informational packets? */
			bool drop_icmp6_info;

			/**
			 * True = issue #132 behaviour.
			 * False = RFC 6146 behaviour.
			 */
			bool src_icmp6errs_better;
			/**
			 * Fields of the packet that will be sent to the F() function.
			 * (RFC 6056 algorithm 3.)
			 * See "enum f_args".
			 */
			__u8 f_args;
			/**
			 * Decrease timer when a FIN packet is received during the
			 * `V4 FIN RCV` or `V6 FIN RCV` states?
			 * https://github.com/NICMx/Jool/issues/212
			 */
			bool handle_rst_during_fin_rcv;

			struct bib_config bib;
			struct joold_config joold;
		} nat64;
	};
};

/** From RFC 7757 */
enum eam_hairpinning_mode {
	EHM_OFF = 0,
	EHM_SIMPLE = 1,
	EHM_INTRINSIC = 2,
};

#endif /* SRC_COMMON_CONFIG_H_ */

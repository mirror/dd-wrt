#ifndef NDPI_MAIN_NETFILTER_H
#define NDPI_MAIN_NETFILTER_H

#ifdef NDPI_IPPORT_DEBUG
#undef DP
#define DP(fmt, args...) printk(fmt, __func__, ## args)
#define DBGDATA(a...) a;
#warning  "DEBUG code"
#else
#define DP(fmt, args...)
#define DBGDATA(a...)
#endif

#include "ndpi_flow_info.h" // for sizeof(struct flow_data)

typedef struct ndpi_detection_module_struct ndpi_mod_str_t;

typedef struct ndpi_proto_nf {
	u_int16_t master_protocol /* e.g. HTTP */, 
		  app_protocol /* e.g. FaceBook */;
} ndpi_protocol_nf;


typedef enum write_buf_id {
	W_BUF_IP=0,
	W_BUF_HOST,
	W_BUF_PROTO,
	W_BUF_FLOW,
	W_BUF_DEBUG,
	W_BUF_RISK,
	W_BUF_CFG,
	W_BUF_LAST
} write_buf_id_t;

#define ACC_READ_NORMAL 0
#define ACC_READ_CLOSED 1
#define ACC_READ_MONITOR 2
#define ACC_READ_MASK 0x3
#define ACC_READ_BIN 0x4

struct write_proc_cmd {
	uint32_t  cpos,max;
	char      cmd[0];
};

struct nf_ct_ext_ndpi;

#define NF_STR_OPTLEN (2*255)
#define NF_STR_LBUF (sizeof(struct flow_data) + NF_STR_OPTLEN)

struct ndpi_net {
        struct		timer_list gc;
	struct ndpi_detection_module_struct *ndpi_struct;
	struct proc_dir_entry   *pde,
#ifdef NDPI_DETECTION_SUPPORT_IPV6
				*pe_info6,
#endif
#ifdef BT_ANNOUNCE
				*pe_ann,
#endif
				*pe_flow,
				*pe_info,
				*pe_proto,
				*pe_debug,
				*pe_risk,
				*pe_cfg,
				*pe_hostdef,
				*pe_ipdef;

	hosts_str_t	*hosts;
	hosts_str_t	*hosts_tmp;
	void		*host_ac;
	int		host_error;
	int		host_upd;

	int		n_hash;
	int		gc_count;
	int		gc_index;
	int		gc_index6;
	int		labels_word;

	rwlock_t	ndpi_busy;	// ndpi in use
	atomic_t	ndpi_ready;	// ndpi ready to work
	struct mutex	rem_lock;	/* lock ndpi_delete_acct / ndpi_flow_read */
	struct mutex	host_lock;	/* protect host_ac, hosts, hosts_tmp */

	spinlock_t	ipq_lock;	/* for proto & patricia tree */
	spinlock_t      w_buff_lock;

	struct write_proc_cmd *w_buff[W_BUF_LAST];
	struct nf_ct_ext_ndpi 	*flow_h;	// Head of info list
	struct nf_ct_ext_ndpi	*flow_l;	// save point for next read info
	
	char			*risk_names;
	size_t			risk_names_len;
	ndpi_risk		risk_mask;

	char                    *cfg_dump;
	size_t                  cfg_dump_len;
	
	char			*str_buf;	// buffer for nflow_read
	int			str_buf_len,	// nflow_read data length
				str_buf_offs;	// nflow_read data offset
	int			acc_wait;	// delay for next run ndpi_delete_acct
	atomic_t		acc_work;	// number of active flow info
	atomic_t		acc_rem;	// number of inactive flow info
	unsigned long int	acc_gc;		// next run ndpi_delete_acct (jiffies + X)
	time64_t		acc_open_time;	// time of reading from pos 0
	int			acc_end;	// EOF for read process
	int			acc_limit;	// if acc_work > acc_limit then drop flow info
	int			acc_read_mode;	// ACC_READ_NORMAL - read all connections info,
						// ACC_READ_CLOSED - read closed connections info
						// ACC_READ_MONITOR - read connections info w/o reset counter
						// +4 - binary mode
	int			acc_last_op;	// 0 - open
						// 1 - read
						// 2 - write
	atomic_t		acc_i_packets_lost; // lost traffic from flow info
	atomic_t		acc_o_packets_lost;
	atomic64_t		acc_i_bytes_lost;
	atomic64_t		acc_o_bytes_lost;
	unsigned long int	cnt_view,cnt_del,cnt_out;

	struct ndpi_mark {
		uint32_t	mark,mask;
	} mark[NDPI_NUM_BITS+1];
	atomic64_t	protocols_cnt[NDPI_NUM_BITS+1];
	NDPI_PROTOCOL_BITMASK protocols_bitmask;
	char			ns_name[16];
	u_int8_t debug_level[NDPI_NUM_BITS+1]; /* if defined NDPI_ENABLE_DEBUG_MESSAGES */
};

// 112 bytes with ipv6, 88 bytes without ipv6
struct flow_info {
	/* 0 - in, 1 - out, 2 - last in, 3 - last out */
	uint64_t		b[4]; // 32
	uint32_t		p[4]; // 16
	uint32_t		time_start,time_end; // 8
	uint32_t		ip_snat,ip_dnat; // 8

	union nf_inet_addr	ip_s,ip_d;   // 32/8
	uint16_t		sport,dport,sport_nat,dport_nat; // 8
	uint16_t		ifidx,ofidx; // 4
	uint32_t		connmark;    // 4 bytes
};


#define f_flow_info	0
#define f_for_delete	1
#define f_detect_done	2
#define f_nat_done	3
#define f_flow_yes	4
#define f_ipv6		5
#define f_snat		6
#define f_dnat		7
#define f_userid	8
#define f_tlsdone	9

static inline void clear_bit_simple(int num,uint16_t *sword) {
	*sword &= ~(1u << (num & 15));
}
static inline void set_bit_simple(int num,uint16_t *sword) {
	*sword |= (1u << (num & 15));
}
static inline int test_bit_simple(int num,uint16_t *sword) {
	return *sword & (1u << (num & 15)) ? 1:0;
}

#define test_flow_info(ct_ndpi)	test_bit_simple(f_flow_info,&ct_ndpi->flags)
#define test_for_delete(ct_ndpi)	test_bit_simple(f_for_delete,&ct_ndpi->flags)
#define test_detect_done(ct_ndpi)	test_bit_simple(f_detect_done,&ct_ndpi->flags)
#define test_nat_done(ct_ndpi)	test_bit_simple(f_nat_done,&ct_ndpi->flags)
#define test_flow_yes(ct_ndpi)	test_bit_simple(f_flow_yes,&ct_ndpi->flags)
#define test_ipv6(ct_ndpi)		test_bit_simple(f_ipv6,&ct_ndpi->flags)
#define test_snat(ct_ndpi)		test_bit_simple(f_snat,&ct_ndpi->flags)
#define test_dnat(ct_ndpi)		test_bit_simple(f_dnat,&ct_ndpi->flags)
#define test_userid(ct_ndpi)		test_bit_simple(f_userid,&ct_ndpi->flags)
#define test_tlsdone(ct_ndpi)		test_bit_simple(f_tlsdone,&ct_ndpi->flags)

#define set_flow_info(ct_ndpi)	set_bit_simple(f_flow_info,&ct_ndpi->flags)
#define set_for_delete(ct_ndpi)	set_bit_simple(f_for_delete,&ct_ndpi->flags)
#define set_detect_done(ct_ndpi)	set_bit_simple(f_detect_done,&ct_ndpi->flags)
#define set_nat_done(ct_ndpi)	set_bit_simple(f_nat_done,&ct_ndpi->flags)
#define set_flow_yes(ct_ndpi)	set_bit_simple(f_flow_yes,&ct_ndpi->flags)
#define set_ipv6(ct_ndpi)	set_bit_simple(f_ipv6,&ct_ndpi->flags)
#define set_snat(ct_ndpi)	set_bit_simple(f_snat,&ct_ndpi->flags)
#define set_dnat(ct_ndpi)	set_bit_simple(f_dnat,&ct_ndpi->flags)
#define set_userid(ct_ndpi)	set_bit_simple(f_userid,&ct_ndpi->flags)
#define set_tlsdone(ct_ndpi)	set_bit_simple(f_tlsdone,&ct_ndpi->flags)

#define clear_flow_info(ct_ndpi)	clear_bit_simple(f_flow_info,&ct_ndpi->flags)
#define clear_for_delete(ct_ndpi) clear_bit_simple(f_for_delete,&ct_ndpi->flags)
#define clear_detect_done(ct_ndpi) clear_bit_simple(f_detect_done,&ct_ndpi->flags)
#define clear_nat_done(ct_ndpi)	clear_bit_simple(f_nat_done,&ct_ndpi->flags)
#define clear_flow_yes(ct_ndpi)	clear_bit_simple(f_flow_yes,&ct_ndpi->flags)
#define clear_ipv6(ct_ndpi)	clear_bit_simple(f_ipv6,&ct_ndpi->flags)
#define clear_snat(ct_ndpi)	clear_bit_simple(f_snat,&ct_ndpi->flags)
#define clear_dnat(ct_ndpi)	clear_bit_simple(f_dnat,&ct_ndpi->flags)
#define clear_userid(ct_ndpi)	clear_bit_simple(f_userid,&ct_ndpi->flags)
#define clear_tlsdone(ct_ndpi)	clear_bit_simple(f_tlsdone,&ct_ndpi->flags)


struct nf_ct_ext_ndpi {
	struct nf_ct_ext_ndpi	*next;		// 4/8
	struct ndpi_flow_struct	*flow;		// 4/8
	char			*host;		// 4/8 bytes
	char			*flow_opt;	// 4/8 bytes
	ndpi_protocol_nf	proto;		// 4 bytes
	spinlock_t		lock;		// 2/4 bytes
						// ?/56 bytes with debug spinlock
#if __SIZEOF_LONG__ == 8
	uint16_t		flags;		// 2 bytes
	uint8_t			l4_proto;	// 1
	uint8_t			confidence;	// 1
	struct flow_info	flinfo;		// 112 bytes
#else
	struct flow_info	flinfo;		// 112 bytes
	uint16_t		flags;		// 2 bytes
	uint8_t			l4_proto;	// 1
	uint8_t			confidence;	// 1
#endif
	uint64_t		risk;		// 8 risk bitmap
	uint16_t		ja3s,ja3c,ja4c,tlsv,tlsfp;
						// offset+1 in flow_opt

} __attribute__((__aligned__(__SIZEOF_LONG__ * 2)));

//extern unsigned long ndpi_log_debug;

#include "../lib/third_party/include/ahocorasick.h"

NDPI_STATIC const char *acerr2txt(AC_ERROR_t r);
NDPI_STATIC int str_coll_to_automata(struct ndpi_detection_module_struct *ndpi_str,
		void *host_ac,hosts_str_t *hosts);

NDPI_STATIC void set_debug_trace( struct ndpi_net *n);

static inline int get_acc_mode(struct ndpi_net *n) {
	return n->acc_read_mode & ACC_READ_MASK;
}


#endif
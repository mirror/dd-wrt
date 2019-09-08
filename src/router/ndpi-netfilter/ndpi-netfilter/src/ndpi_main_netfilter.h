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

typedef struct ndpi_detection_module_struct ndpi_mod_str_t;

typedef enum write_buf_id {
	W_BUF_IP=0,
	W_BUF_HOST,
	W_BUF_PROTO,
	W_BUF_FLOW,
	W_BUF_LAST
} write_buf_id_t;

struct write_proc_cmd {
	uint32_t  cpos,max;
	char      cmd[0];
};

struct nf_ct_ext_ndpi;

struct ndpi_net {
	struct ndpi_detection_module_struct *ndpi_struct;
	struct rb_root osdpi_id_root;
	NDPI_PROTOCOL_BITMASK protocols_bitmask;
	atomic_t	protocols_cnt[NDPI_NUM_BITS+1];
	spinlock_t	id_lock;
	spinlock_t	ipq_lock; // for proto & patricia tree
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
				*pe_hostdef,
				*pe_ipdef;
	int		n_hash;
	int		gc_count;
	int		gc_index;
	int		gc_index6;
	int		labels_word;
        struct		timer_list gc;

	spinlock_t	host_lock; /* protect host_ac, hosts, hosts_tmp */
	hosts_str_t	*hosts;
	
	hosts_str_t	*hosts_tmp;
	void		*host_ac;
	int		host_error;

	spinlock_t	       w_buff_lock;
	struct write_proc_cmd *w_buff[W_BUF_LAST];

	struct ndpi_mark {
		uint32_t	mark,mask;
	} mark[NDPI_NUM_BITS+1];
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
	u_int8_t debug_level[NDPI_NUM_BITS+1];
#endif
	spinlock_t		rem_lock;	// lock ndpi_delete_acct
	struct nf_ct_ext_ndpi 	*flow_h;	// Head of info list
	struct nf_ct_ext_ndpi	*flow_l;	// save point for next read info
	atomic_t		init_done;	// ndpi_net_init() complete
	atomic_t		acc_open;	// flow is open
	atomic_t		acc_work;	// number of active flow info
	atomic_t		acc_rem;	// number of inactive flow info
	atomic_t		shutdown;	// stop netns
	unsigned long int	acc_gc;		// next run ndpi_delete_acct (jiffies + X)
	unsigned long int	acc_open_time;	// time of reading from pos 0
	int			acc_wait;	// delay for next run ndpi_delete_acct
	int			acc_end;	// EOF for read process
	int			acc_limit;	// if acc_work > acc_limit then drop flow info
	int			acc_read_mode;	// 0 - read all connections info,
						// 1 - read closed connections info
						// 2 - read connections info w/o reset counter
						// +4 - binary mode
	atomic_t		acc_i_packets_lost; // lost traffic from flow info
	atomic_t		acc_o_packets_lost;
	atomic64_t		acc_i_bytes_lost;
	atomic64_t		acc_o_bytes_lost;
	unsigned long int	cnt_view,cnt_del,cnt_out;
};

struct flow_info {
	/* 0 - in, 1 - out, 2 - last in, 3 - last out */
	uint64_t		b[4]; // 32
	uint32_t		p[4]; // 16
	uint32_t		time_start,time_end; // 8

	union nf_inet_addr	ip_s,ip_d; // 32
	uint32_t		ip_snat,ip_dnat; // 8
	uint16_t		sport,dport,sport_nat,dport_nat; // 8
	uint16_t		ifidx,ofidx; // 4
}  __attribute ((packed)); // 108 bytes


#define f_flow_info	0
#define f_for_delete	1
#define f_detect_done	2
#define f_nat_done	3
#define f_flow_yes	4
#define f_ipv6		5
#define f_snat		6
#define f_dnat		7
#define f_userid	8

#define test_flow_info(ct_ndpi)	test_bit(f_flow_info,&ct_ndpi->flags)
#define test_for_delete(ct_ndpi)	test_bit(f_for_delete,&ct_ndpi->flags)
#define test_detect_done(ct_ndpi)	test_bit(f_detect_done,&ct_ndpi->flags)
#define test_nat_done(ct_ndpi)	test_bit(f_nat_done,&ct_ndpi->flags)
#define test_flow_yes(ct_ndpi)	test_bit(f_flow_yes,&ct_ndpi->flags)
#define test_ipv6(ct_ndpi)		test_bit(f_ipv6,&ct_ndpi->flags)
#define test_snat(ct_ndpi)		test_bit(f_snat,&ct_ndpi->flags)
#define test_dnat(ct_ndpi)		test_bit(f_dnat,&ct_ndpi->flags)
#define test_userid(ct_ndpi)		test_bit(f_userid,&ct_ndpi->flags)

#define set_flow_info(ct_ndpi)	set_bit(f_flow_info,&ct_ndpi->flags)
#define set_for_delete(ct_ndpi)	set_bit(f_for_delete,&ct_ndpi->flags)
#define set_detect_done(ct_ndpi)	set_bit(f_detect_done,&ct_ndpi->flags)
#define set_nat_done(ct_ndpi)	set_bit(f_nat_done,&ct_ndpi->flags)
#define set_flow_yes(ct_ndpi)	set_bit(f_flow_yes,&ct_ndpi->flags)
#define set_ipv6(ct_ndpi)	set_bit(f_ipv6,&ct_ndpi->flags)
#define set_snat(ct_ndpi)	set_bit(f_snat,&ct_ndpi->flags)
#define set_dnat(ct_ndpi)	set_bit(f_dnat,&ct_ndpi->flags)
#define set_userid(ct_ndpi)	set_bit(f_userid,&ct_ndpi->flags)

#define clear_flow_info(ct_ndpi)	clear_bit(f_flow_info,&ct_ndpi->flags)
#define clear_for_delete(ct_ndpi) clear_bit(f_for_delete,&ct_ndpi->flags)
#define clear_detect_done(ct_ndpi) clear_bit(f_detect_done,&ct_ndpi->flags)
#define clear_nat_done(ct_ndpi)	clear_bit(f_nat_done,&ct_ndpi->flags)
#define clear_flow_yes(ct_ndpi)	clear_bit(f_flow_yes,&ct_ndpi->flags)
#define clear_ipv6(ct_ndpi)	clear_bit(f_ipv6,&ct_ndpi->flags)
#define clear_snat(ct_ndpi)	clear_bit(f_snat,&ct_ndpi->flags)
#define clear_dnat(ct_ndpi)	clear_bit(f_dnat,&ct_ndpi->flags)
#define clear_userid(ct_ndpi)	clear_bit(f_userid,&ct_ndpi->flags)


struct nf_ct_ext_ndpi {
	struct nf_ct_ext_ndpi	*next;		// 4/8
	struct ndpi_flow_struct	*flow;		// 4/8
	struct ndpi_id_struct   *src,*dst;	// 8/16
	char			*host;		// 4/8 bytes
	char			*ssl;		// 4/8 bytes
	struct flow_info	flinfo;		// 108 bytes
	ndpi_protocol		proto;		// 8 bytes
	long unsigned int	flags;		// 4/8 bytes
	uint32_t		connmark;	// 4 bytes
	spinlock_t		lock;		// 2/4 bytes
						// ?/56 bytes with debug spinlock

	uint8_t			l4_proto;	// 1
/* 
 * 32bit - 148 bytes, 64bit - 233+7 bytes;
 */
} __attribute ((packed));

//static unsigned long ndpi_log_debug;

#include "../lib/third_party/include/ahocorasick.h"
static const char *acerr2txt(AC_ERROR_t r);

static void set_debug_trace( struct ndpi_net *n);
#endif
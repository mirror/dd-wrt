/*-----------------------------------------------------------------------------------
*	sl351x_hash_cfg.h
*
*	Description:
*	
*	History:
*
*	9/14/2005	Gary Chen	Create
*
*-------------------------------------------------------------------------------------*/
#ifndef _SL351x_HASH_CFG_H_
#define _SL351x_HASH_CFG_H_	1

// #define NAT_DEBUG_MSG	1
// #define DEBUG_NAT_MIXED_HW_SW_TX	1
#ifdef DEBUG_NAT_MIXED_HW_SW_TX
	// #define NAT_DEBUG_LAN_HASH_TIMEOUT	1
	// #define NAT_DEBUG_WAN_HASH_TIMEOUT	1
#endif

#define IPIV(a,b,c,d)		((a<<24)+(b<<16)+(c<<8)+d)
#define	IPIV1(a)			((a>>24)&0xff)
#define	IPIV2(a)			((a>>16)&0xff)
#define IPIV3(a)			((a>>8)&0xff)
#define IPIV4(a)			((a)&0xff)

#define HASH_MAX_BYTES			64	// 128
#define HASH_ACTION_DWORDS		9
#define HASH_MAX_DWORDS			(HASH_MAX_BYTES / sizeof(u32))
#define HASH_MAX_KEY_DWORD		(HASH_MAX_DWORDS - HASH_ACTION_DWORDS)
#define HASH_INIT_KEY			0x534C4F52
#define HASH_BITS				12	// 12 : Normal, 7: Simulation
#define HASH_TOTAL_ENTRIES		(1 << HASH_BITS)
#define HASH_MAX_ENTRIES		(1 << 12)
#define HASH_TOE_ENTRIES		(HASH_TOTAL_ENTRIES >> 5)
#define HASH_BITS_MASK			((1 << HASH_BITS) - 1)

#define hash_lock(lock)			// spin_lock_bh(lock)
#define hash_unlock(lock)		// spin_unlock_bh(lock)

/*----------------------------------------------------------------------
 *  special macro
 ----------------------------------------------------------------------*/
#define HASH_PUSH_WORD(cp, data)	{*cp++ = (((u16)(data))     ) & 0xff; 	\
							 		*cp++ = (((u16)(data)) >> 8) & 0xff;} 
#define HASH_PUSH_DWORD(cp, data)	{*cp++ = (u8)(((u32)(data))      ) & 0xff;	\
							  		*cp++ = (u8)(((u32)(data)) >>  8) & 0xff;	\
							  		*cp++ = (u8)(((u32)(data)) >> 16) & 0xff;	\
							  		*cp++ = (u8)(((u32)(data)) >> 24) & 0xff;}
#define HASH_PUSH_BYTE(cp, data)	{*cp++ = ((u8)(data)) & 0xff;}

/*----------------------------------------------------------------------
 *  key
 ----------------------------------------------------------------------*/
typedef struct {
	u8		port;
	u16		Ethertype;
	u8		da[6];
	u8		sa[6];
	u16		pppoe_sid;	
	u16		vlan_id;	
	u8		ipv4_hdrlen;	
	u8		ip_tos;	
	u8		ip_protocol;	
	u32		ipv6_flow_label;
	u8		sip[16];
	u8		dip[16];
	//__u32			sip[4];
	//__u32			dip[4];
	u8		l4_bytes[24];
	u8		l7_bytes[24];
	u8		ipv6;	// 1: IPv6, 0: IPV4
} ENTRY_KEY_T;

/*----------------------------------------------------------------------
 *  key for NAT
 *	Note: packed
 ----------------------------------------------------------------------*/
typedef struct {
	u16		Ethertype;		// not used
	u8		port_id;
	u8		rule_id;
	u8		ip_protocol;
	u8		reserved1;		// ip_tos, not used
	u16		reserved2;		// not used
	u32		sip;
	u32		dip;
	u16		sport;
	u16		dport;
} NAT_KEY_T;

#define NAT_KEY_DWORD_SIZE	(sizeof(NAT_KEY_T)/sizeof(u32))
#define NAT_KEY_SIZE		(sizeof(NAT_KEY_T))

/*----------------------------------------------------------------------
 *  key for NAT
 *	Note: packed
 ----------------------------------------------------------------------*/
typedef struct {
	u16		Ethertype;		// not used
	u8		port_id;
	u8		rule_id;
	u8		ip_protocol;
	u8		reserved1;		// ip_tos, not used
	u16		reserved2;		// not used
	u32		sip;
	u32		dip;
	u16		reserved3;
	u16		protocol;
	u16		reserved4;
	u16		call_id;
} GRE_KEY_T;

#define GRE_KEY_DWORD_SIZE	(sizeof(GRE_KEY_T)/sizeof(u32))
#define GRE_KEY_SIZE		(sizeof(GRE_KEY_T))
/*----------------------------------------------------------------------
 *  key present or not
 ----------------------------------------------------------------------*/
typedef struct {
	u32		port			: 1;
	u32		Ethertype		: 1;
	u32		da				: 1;
	u32		sa				: 1;
	u32		pppoe_sid		: 1;	
	u32		vlan_id			: 1;	
	u32		ipv4_hdrlen		: 1;	
	u32		ip_tos			: 1;
	u32		ip_protocol		: 1;	
	u32		ipv6_flow_label	: 1;
	u32		sip				: 1;
	u32		dip				: 1;
	u32		l4_bytes_0_3	: 1;
	u32		l4_bytes_4_7	: 1;
	u32		l4_bytes_8_11	: 1;
	u32		l4_bytes_12_15	: 1;
	u32		l4_bytes_16_19	: 1;
	u32		l4_bytes_20_23	: 1;
	u32		l7_bytes_0_3	: 1;
	u32		l7_bytes_4_7	: 1;
	u32		l7_bytes_8_11	: 1;
	u32		l7_bytes_12_15	: 1;
	u32		l7_bytes_16_19	: 1;
	u32		l7_bytes_20_23	: 1;
	u32		reserved		: 8;
} KEY_FIELD_T;

/*----------------------------------------------------------------------
 *  action
 ----------------------------------------------------------------------*/
typedef struct {
	u32		reserved0	: 5;	// bit 0:4
	u32		pppoe		: 2;	// bit 5:6
	u32		vlan		: 2;	// bit 7:8
	u32		sa			: 1;	// bit 9
	u32		da			: 1;	// bit 10
	u32		Dport		: 1;	// bit 11
	u32		Sport		: 1;	// bit 12
	u32		Dip			: 1;	// bit 13
	u32		Sip			: 1;	// bit 14
	u32		sw_id		: 1;	// bit 15
	u32		frag		: 1;	// bit 16
	u32		option		: 1;	// bit 17
	u32		ttl_0		: 1;	// bit 18
	u32		ttl_1		: 1;	// bit 19
	u32		mtu			: 1;	// bit 20
	u32		exception	: 1;	// bit 21
	u32		srce_qid	: 1;	// bit 22
	u32		discard		: 1;	// bit 23
	u32		dest_qid	: 8;	// bit 24:31
} ENTRY_ACTION_T;

#define ACTION_DISCARD_BIT		BIT(23)
#define ACTION_SRCE_QID_BIT		BIT(22)
#define ACTION_EXCEPTION_BIT	BIT(21)
#define ACTION_MTU_BIT			BIT(20)
#define ACTION_TTL_1_BIT		BIT(19)
#define ACTION_TTL_0_BIT		BIT(18)
#define ACTION_IP_OPTION		BIT(17)
#define ACTION_FRAG_BIT			BIT(16)
#define ACTION_SWID_BIT			BIT(15)
#define ACTION_SIP_BIT			BIT(14)
#define ACTION_DIP_BIT			BIT(13)
#define ACTION_SPORT_BIT		BIT(12)
#define ACTION_DPORT_BIT		BIT(11)
#define ACTION_DA_BIT			BIT(10)
#define ACTION_SA_BIT			BIT(9)
#define ACTION_VLAN_DEL_BIT		BIT(8)
#define ACTION_VLAN_INS_BIT		BIT(7)
#define ACTION_PPPOE_DEL_BIT	BIT(6)
#define ACTION_PPPOE_INS_BIT	BIT(5)
#define ACTION_L4_THIRD_BIT		BIT(4)
#define ACTION_L4_FOURTH_BIT	BIT(3)

#define NAT_ACTION_BITS			(ACTION_SRCE_QID_BIT  | ACTION_EXCEPTION_BIT |	\
								ACTION_TTL_1_BIT | ACTION_TTL_0_BIT | 			\
								ACTION_IP_OPTION | ACTION_FRAG_BIT |			\
								ACTION_DA_BIT | ACTION_SA_BIT)
#define NAT_LAN2WAN_ACTIONS		(NAT_ACTION_BITS | ACTION_SIP_BIT | ACTION_SPORT_BIT)
#define NAT_WAN2LAN_ACTIONS		(NAT_ACTION_BITS | ACTION_DIP_BIT | ACTION_DPORT_BIT)
#define NAT_PPPOE_LAN2WAN_ACTIONS	(NAT_LAN2WAN_ACTIONS | ACTION_PPPOE_INS_BIT)
#define NAT_PPPOE_WAN2LAN_ACTIONS	(NAT_WAN2LAN_ACTIONS | ACTION_PPPOE_DEL_BIT)
#define NAT_PPTP_LAN2WAN_ACTIONS	(NAT_ACTION_BITS | ACTION_SIP_BIT | ACTION_L4_FOURTH_BIT)
#define NAT_PPTP_WAN2LAN_ACTIONS	(NAT_ACTION_BITS | ACTION_DIP_BIT | ACTION_L4_FOURTH_BIT)
#define NAT_PPPOE_PPTP_LAN2WAN_ACTIONS	(NAT_PPTP_LAN2WAN_ACTIONS | ACTION_PPPOE_INS_BIT)
#define NAT_PPPOE_PPTP_WAN2LAN_ACTIONS	(NAT_PPTP_WAN2LAN_ACTIONS | ACTION_PPPOE_DEL_BIT)
								
/*----------------------------------------------------------------------
 *  parameter
 ----------------------------------------------------------------------*/
typedef struct {
	u8		da[6];
	u8		sa[6];
	u16		vlan;	
	u16  	pppoe;	
	u32		Sip;
	u32		Dip;
	u16  	Sport;	
	u16  	Dport;	
	u16  	sw_id;	
	u16  	mtu;	
} ENTRY_PARAM_T;

/*----------------------------------------------------------------------
 *  Hash Entry
 ----------------------------------------------------------------------*/
typedef struct {
	char			rule;
	ENTRY_KEY_T		key;
	KEY_FIELD_T		key_present;
	ENTRY_ACTION_T	action;
	ENTRY_PARAM_T	param;
	int				index;
	int				total_dwords;
} HASH_ENTRY_T;

/*----------------------------------------------------------------------
 *  NAT Hash Entry
 ----------------------------------------------------------------------*/
typedef struct {
	short	counter;
	short	interval;
} HASH_TIMEOUT_T;

/*----------------------------------------------------------------------
 *  NAT Hash Entry for TCP/UDP protocol
 ----------------------------------------------------------------------*/
typedef struct {
	NAT_KEY_T			key;
	union {
		u32				dword;
		ENTRY_ACTION_T	bits;
	} action;
	ENTRY_PARAM_T		param;
	HASH_TIMEOUT_T		tmo;	// used by software only, to use memory space efficiently
} NAT_HASH_ENTRY_T;

#define NAT_HASH_ENTRY_SIZE		(sizeof(NAT_HASH_ENTRY_T))

/*----------------------------------------------------------------------
 *  GRE Hash Entry for PPTP/GRE protocol
 ----------------------------------------------------------------------*/
typedef struct {
	GRE_KEY_T			key;
	union {
		u32				dword;
		ENTRY_ACTION_T	bits;
	} action;
	ENTRY_PARAM_T		param;
	HASH_TIMEOUT_T		tmo;	// used by software only, to use memory space efficiently
} GRE_HASH_ENTRY_T;

#define GRE_HASH_ENTRY_SIZE		(sizeof(GRE_HASH_ENTRY_T))

/*----------------------------------------------------------------------
 *  External Variables
 ----------------------------------------------------------------------*/
extern char				hash_tables[HASH_TOTAL_ENTRIES][HASH_MAX_BYTES] __attribute__ ((aligned(16)));
extern u32				hash_nat_owner_bits[HASH_TOTAL_ENTRIES/32];
/*----------------------------------------------------------------------
* hash_get_valid_flag
*----------------------------------------------------------------------*/
static inline int hash_get_valid_flag(int index)
{
	volatile u32 *hash_valid_bits_ptr = (volatile u32 *)TOE_V_BIT_BASE;

#ifdef SL351x_GMAC_WORKAROUND
	if (index >= (0x80 * 8) && index < (0x8c * 8))
		return 1;
#endif	
	return (hash_valid_bits_ptr[index/32] & (1 << (index %32)));
}

/*----------------------------------------------------------------------
* hash_get_nat_owner_flag
*----------------------------------------------------------------------*/
static inline int hash_get_nat_owner_flag(int index)
{
	return (hash_nat_owner_bits[index/32] & (1 << (index %32)));
}

/*----------------------------------------------------------------------
* hash_validate_entry
*----------------------------------------------------------------------*/
static inline void hash_validate_entry(int index)
{
	volatile u32	*hash_valid_bits_ptr = (volatile u32 *)TOE_V_BIT_BASE;
	register int	ptr = index/32, bits = 1 << (index %32);
	
	hash_valid_bits_ptr[ptr] |= bits;
}

/*----------------------------------------------------------------------
* hash_invalidate_entry
*----------------------------------------------------------------------*/
static inline void hash_invalidate_entry(int index)
{
	volatile u32 *hash_valid_bits_ptr = (volatile u32 *)TOE_V_BIT_BASE;
	register int	ptr = index/32, bits = 1 << (index %32);
	
	hash_valid_bits_ptr[ptr] &= ~(bits);
}

/*----------------------------------------------------------------------
* hash_nat_enable_owner
*----------------------------------------------------------------------*/
static inline void hash_nat_enable_owner(int index)
{
	hash_nat_owner_bits[index/32] |= (1 << (index % 32));
}

/*----------------------------------------------------------------------
* hash_nat_disable_owner
*----------------------------------------------------------------------*/
static inline void hash_nat_disable_owner(int index)
{
	hash_nat_owner_bits[index/32] &= ~(1 << (index % 32));
}

/*----------------------------------------------------------------------
* hash_get_entry
*----------------------------------------------------------------------*/
static inline void *hash_get_entry(int index)
{
	return (void*) &hash_tables[index][0];
}

/*----------------------------------------------------------------------
* Functions
*----------------------------------------------------------------------*/
extern int hash_add_entry(HASH_ENTRY_T *entry);
extern void sl351x_hash_init(void);
extern void hash_set_valid_flag(int index, int valid);
extern void hash_set_nat_owner_flag(int index, int valid);
extern void *hash_get_entry(int index);
extern int hash_build_keys(u32 *destp, HASH_ENTRY_T *entry);
extern void hash_build_nat_keys(u32 *destp, HASH_ENTRY_T *entry);
extern int hash_write_entry(HASH_ENTRY_T *entry, u8 *key);
extern int hash_add_entry(HASH_ENTRY_T *entry);
extern	u16 hash_crc16(u16 crc, u8 *datap, u32 len);
extern	u16 hash_gen_crc16(u8 *datap, u32 len);

#endif // _SL351x_HASH_CFG_H_




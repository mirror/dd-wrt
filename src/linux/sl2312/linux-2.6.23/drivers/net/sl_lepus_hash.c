/**************************************************************************
* Copyright 2006 StorLink Semiconductors, Inc.  All rights reserved.
*--------------------------------------------------------------------------
* Name			: sl_lepus_hash.c
* Description	:
*		Handle Storlink Lepus Hash Functions
*
* History
*
*	Date		Writer		Description
*----------------------------------------------------------------------------
*	03/13/2006	Gary Chen	Create and implement
*
****************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/completion.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/semaphore.h>
#include <asm/arch/irqs.h>
#include <asm/arch/it8712.h>
#include <linux/mtd/kvctl.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/list.h>
#define	 MIDWAY
#define	 SL_LEPUS

#include <asm/arch/sl2312.h>
#include <asm/arch/sl_lepus_gmac.h>
#include <asm/arch/sl_hash_cfg.h>

#ifndef RXTOE_DEBUG
#define RXTOE_DEBUG
#endif
#undef RXTOE_DEBUG

/*----------------------------------------------------------------------
* Definition
*----------------------------------------------------------------------*/
#define	hash_printf				printk

#define HASH_TIMER_PERIOD		(60*HZ)	// seconds
#define HASH_ILLEGAL_INDEX		0xffff

/*----------------------------------------------------------------------
* Variables
*----------------------------------------------------------------------*/
u32					hash_activate_bits[HASH_TOTAL_ENTRIES/32];
u32					hash_nat_owner_bits[HASH_TOTAL_ENTRIES/32];
char 				hash_tables[HASH_TOTAL_ENTRIES][HASH_MAX_BYTES] __attribute__ ((aligned(16)));
static struct timer_list hash_timer_obj;
LIST_HEAD(hash_timeout_list);

/*----------------------------------------------------------------------
* Functions
*----------------------------------------------------------------------*/
void dm_long(u32 location, int length);
static void hash_timer_func(u32 data);

/*----------------------------------------------------------------------
* hash_init
*----------------------------------------------------------------------*/
void hash_init(void)
{
	int i;
	volatile u32 *dp1, *dp2, dword;

	dp1 = (volatile u32 *) TOE_V_BIT_BASE;
	dp2 = (volatile u32 *) TOE_A_BIT_BASE;

	for (i=0; i<HASH_TOTAL_ENTRIES/32; i++)
	{
		*dp1++ = 0;
		dword = *dp2++;	// read-clear
	}
	memset((void *)&hash_nat_owner_bits, 0, sizeof(hash_nat_owner_bits));
	memset((void *)&hash_tables, 0, sizeof(hash_tables));

	init_timer(&hash_timer_obj);
	hash_timer_obj.expires = jiffies + HASH_TIMER_PERIOD;
	hash_timer_obj.data = (unsigned long)&hash_timer_obj;
	hash_timer_obj.function = (void *)&hash_timer_func;
	add_timer(&hash_timer_obj);

#if (HASH_MAX_BYTES == 128)
	writel((unsigned long)__pa(&hash_tables) | 3,	// 32 words
			TOE_GLOBAL_BASE + GLOBAL_HASH_TABLE_BASE_REG);
#elif (HASH_MAX_BYTES == 64)
	writel((unsigned long)__pa(&hash_tables) | 2,	// 16 words
			TOE_GLOBAL_BASE + GLOBAL_HASH_TABLE_BASE_REG);
#else
	#error Incorrect setting for HASH_MAX_BYTES
#endif

}
/*----------------------------------------------------------------------
* hash_add_entry
*----------------------------------------------------------------------*/
int hash_add_entry(HASH_ENTRY_T *entry)
{
	int	rc;
	u32	key[HASH_MAX_DWORDS];
	rc = hash_build_keys((u32 *)&key, entry);
	if (rc < 0)
		return -1;
	hash_write_entry(entry, (unsigned char*) &key[0]);
//	hash_set_valid_flag(entry->index, 1);
//	printk("Dump hash key!\n");
//	dump_hash_key(entry);
	return entry->index;
}

/*----------------------------------------------------------------------
* hash_set_valid_flag
*----------------------------------------------------------------------*/
void hash_set_valid_flag(int index, int valid)
{
	register u32 reg32;

	reg32 = TOE_V_BIT_BASE + (index/32) * 4;

	if (valid)
	{
		writel(readl(reg32) | (1 << (index%32)), reg32);
	}
	else
	{
		writel(readl(reg32) & ~(1 << (index%32)), reg32);
	}
}

/*----------------------------------------------------------------------
* hash_set_nat_owner_flag
*----------------------------------------------------------------------*/
void hash_set_nat_owner_flag(int index, int valid)
{
	if (valid)
	{
		hash_nat_owner_bits[index/32] |= (1 << (index % 32));
	}
	else
	{
		hash_nat_owner_bits[index/32] &= ~(1 << (index % 32));
	}
}


/*----------------------------------------------------------------------
* hash_build_keys
*----------------------------------------------------------------------*/
int hash_build_keys(u32 *destp, HASH_ENTRY_T *entry)
{
	u32 	data;
	unsigned char 	*cp;
	int				i, j;
	unsigned short 	index;
	int 			total;

	memset((void *)destp, 0, HASH_MAX_BYTES);
	cp = (unsigned char *)destp;

	if (entry->key_present.port || entry->key_present.Ethertype)
	{
		HASH_PUSH_WORD(cp, entry->key.Ethertype);		// word 0
		HASH_PUSH_BYTE(cp, entry->key.port);			// Byte 2
		HASH_PUSH_BYTE(cp, 0);							// Byte 3
	}
	else
	{
		HASH_PUSH_DWORD(cp, 0);
	}

	if (entry->key_present.da || entry->key_present.sa)
	{
		unsigned char mac[4];
		if (entry->key_present.da)
		{
			for (i=0; i<4; i++)
				HASH_PUSH_BYTE(cp, entry->key.da[i]);
		}
		mac[0] = (entry->key_present.da) ? entry->key.da[4] : 0;
		mac[1] = (entry->key_present.da) ? entry->key.da[5] : 0;
		mac[2] = (entry->key_present.sa) ? entry->key.sa[0] : 0;
		mac[3] = (entry->key_present.sa) ? entry->key.sa[1] : 0;
		data = mac[0] + (mac[1]<<8) + (mac[2]<<16) + (mac[3]<<24);
		HASH_PUSH_DWORD(cp, data);
		if (entry->key_present.sa)
		{
			for (i=2; i<6; i++)
				HASH_PUSH_BYTE(cp, entry->key.sa[i]);
		}
	}

	if (entry->key_present.pppoe_sid || entry->key_present.vlan_id)
	{
		HASH_PUSH_WORD(cp, entry->key.vlan_id);		// low word
		HASH_PUSH_WORD(cp, entry->key.pppoe_sid);	// high word
	}
	if (entry->key_present.ipv4_hdrlen || entry->key_present.ip_tos || entry->key_present.ip_protocol)
	{
		HASH_PUSH_BYTE(cp, entry->key.ip_protocol);		// Byte 0
		HASH_PUSH_BYTE(cp, entry->key.ip_tos);			// Byte 1
		HASH_PUSH_BYTE(cp, entry->key.ipv4_hdrlen);		// Byte 2
		HASH_PUSH_BYTE(cp, 0);							// Byte 3
	}

	if (entry->key_present.ipv6_flow_label)
	{
		HASH_PUSH_DWORD(cp, entry->key.ipv6_flow_label);	// low word
	}
	if (entry->key_present.sip)
	{
		// input (entry->key.sip[i]) is network-oriented
		// output (hash key) is host-oriented
		for (i=3; i>=0; i--)
			HASH_PUSH_BYTE(cp, entry->key.sip[i]);
		if (entry->key.ipv6)
		{
			for (i=4; i<16; i+=4)
			{
				for (j=i+3; j>=i; j--)
					HASH_PUSH_BYTE(cp, entry->key.sip[j]);
			}
		}
	}
	if (entry->key_present.dip)
	{
		// input (entry->key.sip[i]) is network-oriented
		// output (hash key) is host-oriented
		for (i=3; i>=0; i--)
			HASH_PUSH_BYTE(cp, entry->key.dip[i]);
		if (entry->key.ipv6)
		{
			for (i=4; i<16; i+=4)
			{
				for (j=i+3; j>=i; j--)
					HASH_PUSH_BYTE(cp, entry->key.dip[j]);
			}
		}
	}

	if (entry->key_present.l4_bytes_0_3)
	{
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[0]);
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[1]);
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[2]);
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[3]);
	}
	if (entry->key_present.l4_bytes_4_7)
	{
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[4]);
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[5]);
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[6]);
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[7]);
	}
	if (entry->key_present.l4_bytes_8_11)
	{
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[8]);
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[9]);
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[10]);
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[11]);
	}
	if (entry->key_present.l4_bytes_12_15)
	{
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[12]);
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[13]);
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[14]);
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[15]);
	}
	if (entry->key_present.l4_bytes_16_19)
	{
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[16]);
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[17]);
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[18]);
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[19]);
	}
	if (entry->key_present.l4_bytes_20_23)
	{
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[20]);
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[21]);
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[22]);
		HASH_PUSH_BYTE(cp, entry->key.l4_bytes[23]);
	}
	if (entry->key_present.l7_bytes_0_3)
	{
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[0]);
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[1]);
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[2]);
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[3]);
	}
	if (entry->key_present.l7_bytes_4_7)
	{
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[4]);
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[5]);
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[6]);
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[7]);
	}
	if (entry->key_present.l7_bytes_8_11)
	{
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[8]);
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[9]);
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[10]);
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[11]);
	}
	if (entry->key_present.l7_bytes_12_15)
	{
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[12]);
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[13]);
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[14]);
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[15]);
	}
	if (entry->key_present.l7_bytes_16_19)
	{
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[16]);
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[17]);
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[18]);
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[19]);
	}
	if (entry->key_present.l7_bytes_20_23)
	{
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[20]);
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[21]);
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[22]);
		HASH_PUSH_BYTE(cp, entry->key.l7_bytes[23]);
	}

	// get hash index
	total = (u32)((u32)cp - (u32)destp) / (sizeof(u32));

	if (total > HASH_MAX_KEY_DWORD)
	{
		//hash_printf("Total key words (%d) is too large (> %d)!\n",
		//				total, HASH_MAX_KEY_DWORD);
		return -1;
	}

	if (entry->key_present.port || entry->key_present.Ethertype)
		index = hash_gen_crc16((unsigned char *)destp, total * 4);
	else
	{
		if (total == 1)
		{
			hash_printf("No key is assigned!\n");
			return -1;
		}

		index = hash_gen_crc16((unsigned char *)(destp+1), (total-1) * 4);
	}

	entry->index = index & HASH_BITS_MASK;

	//hash_printf("Total key words = %d, Hash Index= %d\n",
	//				total, entry->index);

	cp = (unsigned char *)destp;
	cp+=3;
	HASH_PUSH_BYTE(cp, entry->rule);	// rule

	entry->total_dwords = total;

	return total;
}

/*----------------------------------------------------------------------
* hash_build_nat_keys
*----------------------------------------------------------------------*/
void hash_build_nat_keys(u32 *destp, HASH_ENTRY_T *entry)
{
	unsigned char 	*cp;
	int				i;
	unsigned short 	index;
	int 			total;

	memset((void *)destp, 0, HASH_MAX_BYTES);

	cp = (unsigned char *)destp + 2;
	HASH_PUSH_BYTE(cp, entry->key.port);
	cp++;

	if (entry->key_present.pppoe_sid || entry->key_present.vlan_id)
	{
		HASH_PUSH_WORD(cp, entry->key.vlan_id);		// low word
		HASH_PUSH_WORD(cp, entry->key.pppoe_sid);	// high word
	}

	HASH_PUSH_BYTE(cp, entry->key.ip_protocol);
	cp+=3;

	// input (entry->key.sip[i]) is network-oriented
	// output (hash key) is host-oriented
	for (i=3; i>=0; i--)
		HASH_PUSH_BYTE(cp, entry->key.sip[i]);

	// input (entry->key.sip[i]) is network-oriented
	// output (hash key) is host-oriented
	for (i=3; i>=0; i--)
		HASH_PUSH_BYTE(cp, entry->key.dip[i]);

	HASH_PUSH_BYTE(cp, entry->key.l4_bytes[0]);
	HASH_PUSH_BYTE(cp, entry->key.l4_bytes[1]);
	HASH_PUSH_BYTE(cp, entry->key.l4_bytes[2]);
	HASH_PUSH_BYTE(cp, entry->key.l4_bytes[3]);

	// get hash index
	total = (u32)((u32)cp - (u32)destp) / (sizeof(u32));

	index = hash_gen_crc16((unsigned char *)destp, total * 4);
	entry->index = index & ((1 << HASH_BITS) - 1);

	cp = (unsigned char *)destp;
	cp+=3;
	HASH_PUSH_BYTE(cp, entry->rule);	// rule

	entry->total_dwords = total;
}


/*----------------------------------------------------------------------
* hash_write_entry
*----------------------------------------------------------------------*/
int hash_write_entry(HASH_ENTRY_T *entry, unsigned char *key)
{
	int		i;
	u32		*srcep, *destp, *destp2;

	srcep = (u32 *)key;
	destp2 = destp = (u32 *)&hash_tables[entry->index][0];

	for (i=0; i<(entry->total_dwords); i++, srcep++, destp++)
		*destp = *srcep;

	srcep = (u32 *)&entry->action;
	*destp++ = *srcep;

	srcep = (u32 *)&entry->param;
	for (i=0; i<(sizeof(ENTRY_PARAM_T)/sizeof(*destp)); i++, srcep++, destp++)
		*destp = *srcep;

	memset(destp, 0, (HASH_MAX_DWORDS-entry->total_dwords-HASH_ACTION_DWORDS) * sizeof(u32));

	consistent_sync(destp2, (entry->total_dwords+HASH_ACTION_DWORDS) * 4, PCI_DMA_TODEVICE);
	return 0;
}

/*----------------------------------------------------------------------
* hash_timer_func
*----------------------------------------------------------------------*/
static void hash_timer_func(u32 data)
{
	int				i, j;
	volatile u32	*active_p, *own_p, *valid_p;
	u32				a_bits, own_bits;

	valid_p = (volatile u32 *)TOE_V_BIT_BASE;
	active_p = (volatile u32 *)hash_activate_bits;
	own_p = (volatile u32 *)hash_nat_owner_bits;
	for (i=0; i<(HASH_TOTAL_ENTRIES/32); i++, own_p++, active_p++, valid_p++)
	{
		*active_p |= readl(TOE_A_BIT_BASE + (i*4));
		a_bits = *active_p;
		own_bits = *own_p;
		if (own_bits)
		{
#ifndef DEBUG_NAT_MIXED_HW_SW_TX
			a_bits = own_bits & ~a_bits;
#else
			a_bits = own_bits & a_bits;
#endif
			for (j=0; a_bits && j<32; j++)
			{
				if (a_bits & 1)
				{
					*valid_p &= ~(1 << j);		// invalidate it
#if !(defined(NAT_DEBUG_LAN_HASH_TIMEOUT) || defined(NAT_DEBUG_WAN_HASH_TIMEOUT))
					*own_p &= ~(1 << j);		// release ownership for NAT
#endif
// #ifdef DEBUG_NAT_MIXED_HW_SW_TX
#if 0
					hash_printf("%lu %s: Clear hash index: %d\n", jiffies/HZ, __func__, i*32+j);
#endif
				}
				a_bits >>= 1;
			}
			*active_p &= ~own_bits;		// deactivate it for next polling
		}
	}

	hash_timer_obj.expires = jiffies + HASH_TIMER_PERIOD;
	add_timer((struct timer_list *)data);
}

/*----------------------------------------------------------------------
* dm_long
*----------------------------------------------------------------------*/
void dm_long(u32 location, int length)
{
	u32		*start_p, *curr_p, *end_p;
	u32		*datap, data;
	int		i;

	//if (length > 1024)
	//	length = 1024;

	start_p = (u32 *)location;
	end_p = (u32 *)location + length;
	curr_p = (u32 *)((u32)location & 0xfffffff0);
	datap = (u32 *)location;
	while (curr_p < end_p)
	{
		hash_printf("0x%08x: ",(u32)curr_p & 0xfffffff0);
		for (i=0; i<4; i++)
		{
			if (curr_p < start_p || curr_p >= end_p)
               hash_printf("         ");
			else
			{
				data = *datap;
				hash_printf("%08X ", data);
			}
			if (i==1)
              hash_printf("- ");

			curr_p++;
			datap++;
		}
        hash_printf("\n");
	}
}

/*----------------------------------------------------------------------
* hash_dump_entry
*----------------------------------------------------------------------*/
void hash_dump_entry(int index)
{
	hash_printf("Hash Index %d:\n", index);
	dm_long((u32)&hash_tables[index][0], HASH_MAX_DWORDS);
}



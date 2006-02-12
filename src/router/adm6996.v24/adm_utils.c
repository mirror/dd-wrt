#include "adm6996.h"
#include "adm_regs.h"

#include <asm/string.h>

#define BIT(x) (1<<x)

void adm_get_ports_stats (int port, struct adm_port_stat aps[])
{
	uint32 val;
  extern adm_info_t* einfo;

	memset (aps, 0, sizeof (struct adm_port_stat)*6);
	
	adm_rreg (einfo, 1, 1, &val);

	aps[0].link = val&0x1; 
	aps[0].speed = (val&0x2) ? 1 : 0;
	aps[0].duplex = (val&0x4)? 1 : 0;
	aps[0].flow = (val&0x8)? 1 : 0;
	
	aps[1].link = (val&0x100)? 1 : 0; 
	aps[1].speed = (val&0x200) ? 1 : 0;
	aps[1].duplex = (val&0x400)? 1 : 0;
	aps[1].flow = (val&0x800)? 1 : 0;

	aps[2].link = (val & BIT(16)) ? 1 : 0; 
	aps[2].speed = (val & BIT(17)) ? 1 : 0;
	aps[2].duplex = (val & BIT(18))? 1 : 0;
	aps[2].flow = (val & BIT(19))? 1 : 0;
	
	aps[3].link = (val&0x1000000)? 1 : 0; 
	aps[3].speed = (val&0x2000000) ? 1 : 0;
	aps[3].duplex = (val&0x4000000)? 1 : 0;
	aps[3].flow = (val&0x8000000)? 1 : 0;

	aps[4].link = (val& BIT(28))? 1 : 0; 
	aps[4].speed = (val & BIT(29)) ? 1 : 0;
	aps[4].duplex = (val & BIT(30))? 1 : 0;
	aps[4].flow = (val & BIT(31))? 1 : 0;

	adm_rreg (einfo, 1, 2, &val);

	aps[5].link = (val&0x1); 
	aps[5].speed = (val&0x6)>>1;
	aps[5].duplex = (val&0x8)? 1 : 0;
	aps[5].flow = (val&0x10)? 1 : 0;


	adm_rreg (einfo, 1, 3, &val);

	aps[0].cable_len = (val & 0x3);
	aps[0].cable_broken = (val & 0x4)? 1 : 0;
	
	aps[1].cable_len = (val & 0xc0) >> 6;
	aps[1].cable_broken = (val & 0x100)? 1 : 0;

	aps[2].cable_len = (val & 0x3000) >> 12;
	aps[2].cable_broken = (val & 0x4000)? 1 : 0;

	aps[3].cable_len = (val & 0xc0000) >> 18;
	aps[3].cable_broken = (val & 0x100000)? 1 : 0;

	aps[4].cable_len = (val & 0x600000) >> 21;
	aps[4].cable_broken = (val & 0x800000)? 1 : 0;

	adm_rreg (einfo, 1, 0x04, &aps[0].rx_packets);
	adm_rreg (einfo, 1, 0x06, &aps[1].rx_packets);
	adm_rreg (einfo, 1, 0x08, &aps[2].rx_packets);
	adm_rreg (einfo, 1, 0x0a, &aps[3].rx_packets);
	adm_rreg (einfo, 1, 0x0b, &aps[4].rx_packets);
	adm_rreg (einfo, 1, 0x0c, &aps[5].rx_packets);

	adm_rreg (einfo, 1, 0x16, &aps[0].tx_packets);
	adm_rreg (einfo, 1, 0x18, &aps[1].tx_packets);
	adm_rreg (einfo, 1, 0x1a, &aps[2].tx_packets);
	adm_rreg (einfo, 1, 0x1c, &aps[3].tx_packets);
	adm_rreg (einfo, 1, 0x1d, &aps[4].tx_packets);
	adm_rreg (einfo, 1, 0x1e, &aps[5].tx_packets);

	adm_rreg (einfo, 1, 0x0d, &aps[0].rx_bytes);
	adm_rreg (einfo, 1, 0x0f, &aps[1].rx_bytes);
	adm_rreg (einfo, 1, 0x11, &aps[2].rx_bytes);
	adm_rreg (einfo, 1, 0x13, &aps[3].rx_bytes);
	adm_rreg (einfo, 1, 0x14, &aps[4].rx_bytes);
	adm_rreg (einfo, 1, 0x15, &aps[5].rx_bytes);

	adm_rreg (einfo, 1, 0x1f, &aps[0].tx_bytes);
	adm_rreg (einfo, 1, 0x21, &aps[1].tx_bytes);
	adm_rreg (einfo, 1, 0x23, &aps[2].tx_bytes);
	adm_rreg (einfo, 1, 0x25, &aps[3].tx_bytes);
	adm_rreg (einfo, 1, 0x26, &aps[4].tx_bytes);
	adm_rreg (einfo, 1, 0x27, &aps[5].tx_bytes);

	adm_rreg (einfo, 1, 0x28, &aps[0].collisions);
	adm_rreg (einfo, 1, 0x2a, &aps[1].collisions);
	adm_rreg (einfo, 1, 0x2c, &aps[2].collisions);
	adm_rreg (einfo, 1, 0x2e, &aps[3].collisions);
	adm_rreg (einfo, 1, 0x2f, &aps[4].collisions);
	adm_rreg (einfo, 1, 0x30, &aps[5].collisions);

	adm_rreg (einfo, 1, 0x31, &aps[0].errors);
	adm_rreg (einfo, 1, 0x33, &aps[1].errors);
	adm_rreg (einfo, 1, 0x35, &aps[2].errors);
	adm_rreg (einfo, 1, 0x37, &aps[3].errors);
	adm_rreg (einfo, 1, 0x38, &aps[4].errors);
	adm_rreg (einfo, 1, 0x39, &aps[5].errors);
	
	adm_rreg (einfo, 1, 0x3a, &val);

	aps[0].rxp_overflow = val&1;
	aps[1].rxp_overflow = (val>>2)&1;
	aps[2].rxp_overflow = (val>>4)&1;
	aps[3].rxp_overflow = (val>>6)&1;
	aps[4].rxp_overflow = (val>>7)&1;
	aps[5].rxp_overflow = (val>>8)&1;

	aps[0].rxb_overflow = (val>>9)&1;
	aps[1].rxb_overflow = (val>>11)&1;
	aps[2].rxb_overflow = (val>>13)&1;
	aps[3].rxb_overflow = (val>>15)&1;
	aps[4].rxb_overflow = (val>>16)&1;
	aps[5].rxb_overflow = (val>>17)&1;

	adm_rreg (einfo, 1, 0x3b, &val);

	aps[0].txp_overflow = val&1;
	aps[1].txp_overflow = (val>>2)&1;
	aps[2].txp_overflow = (val>>4)&1;
	aps[3].txp_overflow = (val>>6)&1;
	aps[4].txp_overflow = (val>>7)&1;
	aps[5].txp_overflow = (val>>8)&1;

	aps[0].txb_overflow = (val>>9)&1;
	aps[1].txb_overflow = (val>>11)&1;
	aps[2].txb_overflow = (val>>13)&1;
	aps[3].txb_overflow = (val>>15)&1;
	aps[4].txb_overflow = (val>>16)&1;
	aps[5].txb_overflow = (val>>17)&1;

	adm_rreg (einfo, 1, 0x3c, &val);

	aps[0].col_overflow = val&1;
	aps[1].col_overflow = (val>>2)&1;
	aps[2].col_overflow = (val>>4)&1;
	aps[3].col_overflow = (val>>6)&1;
	aps[4].col_overflow = (val>>7)&1;
	aps[5].col_overflow = (val>>8)&1;

	aps[0].err_overflow = (val>>9)&1;
	aps[1].err_overflow = (val>>11)&1;
	aps[2].err_overflow = (val>>13)&1;
	aps[3].err_overflow = (val>>15)&1;
	aps[4].err_overflow = (val>>16)&1;
	aps[5].err_overflow = (val>>17)&1;
}

void adm_get_gen_config (struct adm_gen_config* conf)
{
	int vlan;
	uint32 val;
  extern adm_info_t* einfo;
	union {
		struct adm_port_config c;
		uint16 v;
	} u;

	memset (conf, 0, sizeof *conf);
	
	adm_rreg (einfo, 0, 1, &val);
	u.v = val >> 16;
	conf->port[0] = u.c;
	
	adm_rreg (einfo, 0, 3, &val);
	u.v = val >> 16;
	conf->port[1] = u.c;
	
	adm_rreg (einfo, 0, 5, &val);
	u.v = val >> 16;
	conf->port[2] = u.c;

	adm_rreg (einfo, 0, 7, &val);
	u.v = val >> 16;
	conf->port[3] = u.c;
	
	adm_rreg (einfo, 0, 8, &val);
	u.v = val & 0xFFFF;
	conf->port[4] = u.c;
	
	u.v = val >> 16;
	conf->port[5] = u.c;

	/* other pvid bits from registers 28-2c */
	adm_rreg (einfo, 0, 0x28, &val);
	conf->port[0].pvid_4_11 = (val&0xff);

	val >>= 16;
	conf->port[1].pvid_4_11 = (val&0xff);
	
	adm_rreg (einfo, 0, 0x2a, &val);
	conf->port[2].pvid_4_11 = (val&0xff);

	val >>= 16;
	conf->port[3].pvid_4_11 = (val&0xff);
	conf->port[4].pvid_4_11 = (val&0xff00)>>8;
	
	adm_rreg (einfo, 0, 0x2c, &val);
	conf->port[5].pvid_4_11 = (val&0xff);
	
	conf->tag_shift = (val&0x700)>>8;
	conf->reg_2c_bit_11 = (val&BIT(11)) ? 1 : 0;
	conf->fwd_management_mac1 = (val&BIT(12)) ? 1 : 0;
	conf->fwd_management_mac2 = (val&BIT(13)) ? 1 : 0;
	conf->fwd_management_mac3 = (val&BIT(14)) ? 1 : 0;
	conf->fwd_management_mac4 = (val&BIT(15)) ? 1 : 0;

	adm_rreg (einfo, 0, 0x0e, &val);
	conf->vlan_prio = val&0xffff;
	conf->tos_prio = (val>>16)&0xffff;

	adm_rreg (einfo, 0, 0x11, &val);
	val >>= 16;
	conf->mac_clone_11 = (val & BIT(4)) ? 1 : 0;
	conf->vlan_mode = (val & BIT(5)) ? 1 : 0;
	conf->smart_squelch = (val & BIT(10)) ? 1 : 0;
	
	adm_rreg (einfo, 0, 0x12, &val);
	conf->port[0].mac_lock = (val&BIT(0)) ? 1 : 0;
	conf->port[1].mac_lock = (val&BIT(2)) ? 1 : 0;
	conf->port[2].mac_lock = (val&BIT(4)) ? 1 : 0;
	conf->port[3].mac_lock = (val&BIT(6)) ? 1 : 0;
	conf->port[4].mac_lock = (val&BIT(7)) ? 1 : 0;
	conf->port[5].mac_lock = (val&BIT(8)) ? 1 : 0;
	
	conf->drop_on_collisions = (val&BIT(15)) ? 1 : 0;

	
	for (vlan = 0; vlan < 16; ++vlan)
	{
		int i;
		static const int bit_map[] = { BIT(0), BIT(2), BIT(4), 
			                             BIT(6), BIT(7), BIT(8) };
		val >>= 16;
		for (i=0; i<6; ++i)
			if (bit_map[i] & val)
				conf->vlan_groups[vlan] |= BIT(i);
		++vlan;
		if (vlan >= 16) break;
		adm_rreg (einfo, 0, 0x13+vlan, &val);
		for (i=0; i<6; ++i)
			if (bit_map[i] & val)
				conf->vlan_groups[vlan] |= BIT(i);
	}


	adm_rreg (einfo, 0, 0x30, &val);
	conf->mac_clone_30 = (val & BIT(5)) ? 1 : 0;
	conf->mii_speed = (val & BIT(6)) ? 1 : 0;
	conf->speed_led = (val & BIT(9)) ? 1 : 0;
	conf->port_led = (val & BIT(12)) ? 1 : 0;


	/* 0x31 reg */
	val >>= 16;

	conf->port[0].threshold = (val>>0) & 0x07;
	conf->port[1].threshold = (val>>4) & 0x07;
	conf->port[2].threshold = (val>>8) & 0x07;
	conf->port[3].threshold = (val>>12) & 0x07;

	conf->port[0].count_recv = (val & BIT(3)) ? 1 : 0;
	conf->port[1].count_recv = (val & BIT(7)) ? 1 : 0;
	conf->port[2].count_recv = (val & BIT(11)) ? 1 : 0;
	conf->port[3].count_recv = (val & BIT(15)) ? 1 : 0;
	
	adm_rreg (einfo, 0, 0x32, &val);

	conf->port[4].threshold = (val>>0) & 0x07;
	conf->port[5].threshold = (val>>4) & 0x07;

	conf->port[4].count_recv = (val & BIT(3)) ? 1 : 0;
	conf->port[5].count_recv = (val & BIT(7)) ? 1 : 0;

	val >>= 16;
	conf->port[0].thrs_ena = (val & BIT(0)) ? 1 : 0;
	conf->port[1].thrs_ena = (val & BIT(2)) ? 1 : 0;
	conf->port[2].thrs_ena = (val & BIT(4)) ? 1 : 0;
	conf->port[3].thrs_ena = (val & BIT(6)) ? 1 : 0;
	conf->port[4].thrs_ena = (val & BIT(7)) ? 1 : 0;
	conf->port[5].thrs_ena = (val & BIT(8)) ? 1 : 0;
	
	adm_rreg (einfo, 0, 0x0a, &val);
	conf->replace_vid_pvid = (val & BIT(9)) ? 1 : 0;
	val >>= 16;
	conf->ipg92 = (val & BIT(6)) ? 1 : 0;
	conf->trunk = (val & BIT(7)) ? 1 : 0;
	conf->far_end_fault = (val & BIT(15)) ? 0 : 1;

	adm_rreg (einfo, 0, 0x10, &val);
	conf->storm_threshold = (val & 0x3);
	conf->storming = (val & BIT(2)) ? 1 : 0;
	conf->xcrc = (val & BIT(4)) ? 0 : 1;
	conf->aging = (val & BIT(7)) ? 0 : 1;
	conf->polarity_error = (val & BIT(5)) ? 1 : 0;
	conf->reg_10_bit_3 = (val & BIT(3)) ? 1 : 0;

	conf->drop_q0 = (val >> 8) & 0x3;
	conf->drop_q1 = (val >> 10) & 0x3;
	conf->drop_q2 = (val >> 12) & 0x3;
	conf->drop_q3 = (val >> 14) & 0x3;

}

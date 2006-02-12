#ifndef _ADM_REGS_H
#define _ADM_REGS_H

/********* EPROM registers **********/
/* port configuration register */
union adm_reg_01 {
	struct {
		unsigned int	flow_control:1,						/* 1= enable */
									auto_negotioation:1,			/* 1= enable */
									speed:1,									/* 1= 100 Mmit/s */
									duplex:1,									/* 1= full */
								  output_packet_tagging:1,	/* 1= tag */
									disable:1,								/* 1= disable */
									tos_over_vlan_prio:1,			/* 1=check tos first */
									enable_port_prio:1,				/* 1=port based priority */
									port_priority:2,
									pvid:4,										/* low 4 bits of port vlan ID */
									fx_mode:1,								/* 1= FX mode, 0= TP mode */
									auto_mdix:1								/* 1= crossover auto mdix */
															;
		} bits;
	unsigned short val;
};

union adm_reg_0a {
	struct {
		unsigned int	ro0:9,
									replaced_vid_by_pvid:1,
									ro1:6;
	} bits;
	unsigned short val;
};

union adm_reg_0b {
	struct {
		unsigned int	ro0:6,
									ipg96:1,									/* 0= 96 IPG, 1= 92 IPG */
									trunk:1,									/* 1= enable port 3,4 as trunk */
									ro1:7,
									far_end_fault:1;					/* 0= enable FEF detection */
	} bits;
	unsigned short val;
};

union adm_reg_0e {
	struct {
		unsigned int	vlan0_prio:2,
									vlan1_prio:2,
									vlan2_prio:2,
									vlan3_prio:2,
									vlan4_prio:2,
									vlan5_prio:2,
									vlan6_prio:2,
									vlan7_prio:2;
	} bits;
	unsigned short val;
};

union adm_reg_0f {
	struct {
		unsigned int	tos0_prio:2,
									tos1_prio:2,
									tos2_prio:2,
									tos3_prio:2,
									tos4_prio:2,
									tos5_prio:2,
									tos6_prio:2,
									tos7_prio:2;
	} bits;
	unsigned short val;
};

union adm_reg_10 {
	struct {
		unsigned int	storming_threshold:2,				/* boadcast storming threshold */
									storming_enable:1,
									reg_10_bit_3:1,							/* reserved */
									crc_disable:1,							/* 0= enable CRC chacks */
									/* may be... */
									polarity_error:1,						/* read only */
									ro1:1,
									aging_disable:1,						/* 0= enabling aging */
									drop_q0:2,									/* drop scheme for Q0 */
									drop_q1:2,									/* drop scheme for Q1 */
									drop_q2:2,									/* drop scheme for Q2 */
									drop_q3:2;									/* drop scheme for Q3 */
	} bits;
	unsigned short val;
};

/* Warning: RO reserved bits seems to be RW for this register */ 
union adm_reg_11 {
	struct {
		unsigned int	ro0:4,
									mac_clone:1,								/* 1= mac clone enable */
									vlan_mode:1,								/* 0=by pass 1=802.1q */
									ro1:2,
									ro2:2,
									/* may be... */
									back_pressure:1,						/* 1= enable */
									ro3:5;
	} bits;
	unsigned short val;
};

union adm_reg_12 {
	struct {
		unsigned int	port0_mac_lock:1,						/* 1= lock first seen mac addr */
									rw0:1,
									port1_mac_lock:1,
									rw1:1,
									port2_mac_lock:1,
									rw2:1,
									port3_mac_lock:1,
									port4_mac_lock:1,
									port5_mac_lock:1,
									rw3:2,
									rw4:1,
									rw5:2,
									rw6:1,
									drop_on_collision:1;				/* 1= drop pck on excessive 
																							    collisions
																						   */
	} bits;
	unsigned short val;
};

union adm_reg_13 { /* also 14-22 */
	struct {
		unsigned int	vlan_map:9,
									ro0:7;
	} bits1;
	struct {
		unsigned int	vlan_map_port0:1,
									rw0:1,
									vlan_map_port1:1,
									rw1:1,
									vlan_map_port2:1,
									rw2:1,
									vlan_map_port3:1,
									vlan_map_port4:1,
									vlan_map_port5:1,
									ro3:7;
	} bits2;
	unsigned short val;
};

union adm_reg_28 { /* also 29, 2a, 2b */
	struct {
		unsigned int	pvid_lo:8,
									pvid_hi:8;
	} bits;
	unsigned short val;
};

union adm_reg_2c {
	struct {
		unsigned int	pvid:8,
									tag_shift:3,
									/* may be... */
									vlan_grouping_control:1,

									fwd_mac_0:1,	/* 1= forward 0180C200000-FF */
									fwd_mac_1:1,	/* 1= forward 0180C200002-0F */
									fwd_mac_2:1,	/* 1= forward 0180C200001 */
									fwd_mac_3:1;	/* 1= forward 0180C200000 */
	} bits;
	unsigned short val;
};

union adm_reg_30 {
	struct {
		unsigned int	rw0:1,
		              rw1:1,
									rw2:1,
									rw3:2,
									mac_clone:1,							/* unknown */
									mii_speed_double:1,				/* 1=50MHZ 0=25MHZ */
									rw4:1,
									rw5:1,
									speed_led:1,
									rw6:1,
									rw7:1,
									port_led:1,
									rw8:3;
	} bits;
	unsigned short val;
};

union adm_reg_31 {
	struct {
		unsigned int	port0_threshold:3,
									port0_recv_counted:1,
									port1_threshold:3,
									port1_recv_counted:1,
									port2_threshold:3,
									port2_recv_counted:1,
									port3_threshold:3,
									port3_recv_counted:1;
	} bits;
	unsigned short val;
};

union adm_reg_32 {
	struct {
		unsigned int	port4_threshold:3,
									port4_recv_counted:1,
									port5_threshold:3,
									port5_recv_counted:1,
									ro0:8;
	} bits;
	unsigned short val;
};

union adm_reg_33 {
	struct {
		unsigned int	port0_bandwidth:1,							/* 1= enable */
									rw0:1,
									port1_bandwidth:1,
									rw1:1,
									port2_bandwidth:1,
									rw2:1,
									port3_bandwidth:1,
									port4_bandwidth:1,
									port5_bandwidth:1,
									ro3:7;
	} bits;
	unsigned short val;
};

/********* Serial registers **********/

/* ports status */
union adm_ser_01 {
	struct {
		unsigned int	port0_link:1,
									port0_speed:1,
									port0_duplex:1,
									port0_flow:1,
									r0:1, r1:1, r2:1, r3:1,
										 
									port1_link:1,
									port1_speed:1,
									port1_duplex:1,
									port1_flow:1,
									r4:1, r5:1, r6:1, r7:1,
										 
									port2_link:1,
									port2_speed:1,
									port2_duplex:1,
									port2_flow:1,
									r8:1, r9:1, r10:1, r11:1,

									port3_link:1,
									port3_speed:1,
									port3_duplex:1,
									port3_flow:1,

									port4_link:1,
									port4_speed:1,
									port4_duplex:1,
									port4_flow:1;
	} bits;
	unsigned int val;
};

union adm_ser_02 {
	struct {
		unsigned int	port5_link:1,
									port5_speed:2,
									port5_duplex:1,
									port5_flow:1,
									r:27;
	} bits;
	unsigned int val;
};

/* cable register */
union adm_ser_03 {
	struct {
		unsigned int	port0_cable_len:2,
									port0_cable_broken:1,
									r0:2, r1:1,
									port1_cable_len:2,
									port1_cable_broken:1,
									r2:2, r3:1,
									port2_cable_len:2,
									port2_cable_broken:1,
									r4:2, r5:1,
									port3_cable_len:2,
									port3_cable_broken:1,
									port4_cable_len:2,
									port4_cable_broken:1,
									r6:8;
	} bits;
	unsigned int val;
};

union adm_ser_3a {
	struct {
		unsigned int	port0_rxp_overflow:1,
									r0:1,
									port1_rxp_overflow:1,
									r1:1,
									port2_rxp_overflow:1,
									r2:1,
									port3_rxp_overflow:1,
									port4_rxp_overflow:1,
									port5_rxp_overflow:1,

									port0_rxb_overflow:1,
									r3:1,
									port1_rxb_overflow:1,
									r4:1,
									port2_rxb_overflow:1,
									r5:1,
									port3_rxb_overflow:1,
									port4_rxb_overflow:1,
									port5_rxb_overflow:1,
									r6:14;
	} bits;
	unsigned int val;
};

union adm_ser_3b {
	struct {
		unsigned int	port0_txp_overflow:1,
									r0:1,
									port1_txp_overflow:1,
									r1:1,
									port2_txp_overflow:1,
									r2:1,
									port3_txp_overflow:1,
									port4_txp_overflow:1,
									port5_txp_overflow:1,

									port0_txb_overflow:1,
									r3:1,
									port1_txb_overflow:1,
									r4:1,
									port2_txb_overflow:1,
									r5:1,
									port3_txb_overflow:1,
									port4_txb_overflow:1,
									port5_txb_overflow:1,
									r6:14;
	} bits;
	unsigned int val;
};

union adm_ser_3c {
	struct {
		unsigned int	port0_col_overflow:1,
									r0:1,
									port1_col_overflow:1,
									r1:1,
									port2_col_overflow:1,
									r2:1,
									port3_col_overflow:1,
									port4_col_overflow:1,
									port5_col_overflow:1,

									port0_err_overflow:1,
									r3:1,
									port1_err_overflow:1,
									r4:1,
									port2_err_overflow:1,
									r5:1,
									port3_err_overflow:1,
									port4_err_overflow:1,
									port5_err_overflow:1,
									r6:14;
	} bits;
	unsigned int val;
};

#endif

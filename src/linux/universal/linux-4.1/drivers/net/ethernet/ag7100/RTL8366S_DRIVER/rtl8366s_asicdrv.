#ifndef _RTL8366S_ASICDRV_H_
#define _RTL8366S_ASICDRV_H_

#include "rtl8366s_types.h"




#define RTL8366S_REGBITLENGTH					16
#define RTL8366S_ACLRULENO						32
#define RTL8366S_ACLINDEXMAX					(RTL8366S_ACLRULENO-1)
#define RTL8366S_RMAUSERDEFMAX				4
#define RTL8366S_VLANMCIDXMAX					15
#define RTL8366S_FIDMAX						7
#define RTL8366S_VIDMAX						0xFFF
#define RTL8366S_PRIORITYMAX					7
#define RTL8366S_PORTMASK						0x3F
#define RTL8366S_SPTSTSMASK					0x03
#define RTL8366S_L2ENTRYMAX					3
#define RTL8366S_CAMENTRYMAX					7
#define RTL8366S_SVLANIDXMAX					3
#define RTL8366S_PPBIDXMAX						3
#define RTL8366S_LAHASHVALMAX					7
#define RTL8366S_LAHASHSELMAX					2
#define RTL8366S_LAPORTSMAX					4
#define RTL8366S_DSCPMAX						63
#define RTL8366S_DSCPMIN						0

#define RTL8366S_SCPERIODMAX					3
#define RTL8366S_SCCOUNTMAX					3

#define RTL8366S_ACLIDXMAX						15
#define RTL8366S_ACLDATALENGTH				6
#define RTL8366S_ACLDATA0WRITE 				0x0901
#define RTL8366S_ACLDATA1WRITE 				0x0B01
#define RTL8366S_ACLDATA2WRITE 				0x0D01
#define RTL8366S_ACLDATA0READ 				0x0801
#define RTL8366S_ACLDATA1READ 				0x0A01
#define RTL8366S_ACLDATA2READ 				0x0C01

#define RTL8366S_PHY_NO_MAX					4
#define RTL8366S_PHY_PAGE_MAX					7
#define RTL8366S_PHY_ADDR_MAX					31

#define RTL8366S_QOS_BWCTL_DISABLE			0x3FFF

#define RTL8366S_LED_GROUP_MAX				4



/* switch global control */
#define RTL8366S_SWITCH_GLOBAL_CTRL_REG		0x0000
#define RTL8366S_MAX_LENGHT_MSK				0x0030
#define RTL8366S_MAX_LENGHT_BIT				4
#define RTL8366S_CAM_TBL_BIT					6
#define RTL8366S_JAM_MODE_BIT					9
#define RTL8366S_MAX_PAUSE_CNT_BIT			11
#define RTL8366S_EN_VLAN_BIT					13
#define RTL8366S_EN_QOS_BIT					14

#define RTL8366S_PORT_ENABLE_CTRL_REG		0x0001
#define RTL8366S_PORT_DISABLE_MSK				0x003F

#define RTL8366S_PORT_LEARNDIS_CTRL_REG		0x0002
#define RTL8366S_PORT_LEARNDIS_MSK			0x003F

#define RTL8366S_DISABLE_AGE_CTRL_REG		0x0003
#define RTL8366S_DISABLE_AGE_MSK				0x003F

#define RTL8366S_SECURITY_CTRL2_REG			0x0004
#define RTL8366S_DROP_UNDA_MSK				0x0001
#define RTL8366S_DROP_UNDA_BIT				0
#define RTL8366S_DROP_UNSA_MSK				0x0002
#define RTL8366S_DROP_UNSA_BIT				1
#define RTL8366S_DROP_UNMATCHSA_MSK			0x0004
#define RTL8366S_DROP_UNMATCHSA_BIT			2
#define RTL8366S_FAST_AGE_MSK					0x0010
#define RTL8366S_FAST_AGE_BIT					4
#define RTL8366S_DISABLE_LEARN_MSK			0x0040
#define RTL8366S_DISABLE_LEARN_BIT			6

/* Port mirror configuration */
#define RTL8366S_PORT_MIRROR_REG				0x0007
#define RTL8366S_PORT_MIRROR_SOURCE_BIT		0
#define RTL8366S_PORT_MIRROR_SOURCE_MSK		0x0007
#define RTL8366S_PORT_MIRROR_MINITOR_BIT		4
#define RTL8366S_PORT_MIRROR_MINITOR_MSK		0x0070
#define RTL8366S_PORT_MIRROR_RX_BIT			8
#define RTL8366S_PORT_MIRROR_RX_MSK			0x0100
#define RTL8366S_PORT_MIRROR_TX_BIT			9
#define RTL8366S_PORT_MIRROR_TX_MSK			0x0200
#define RTL8366S_PORT_MIRROR_ISO_BIT			11
#define RTL8366S_PORT_MIRROR_ISO_MSK			0x0800


#define RTL8366S_EGRESS_KEEP_FORMAT_REG		0x0008
#define RTL8366S_EGRESS_KEEP_PORT_BIT			8


#define RTL8366S_GREEN_FEATURE_REG			0x000A
#define RTL8366S_GREEN_FEATURE_TX_BIT		3
#define RTL8366S_GREEN_FEATURE_TX_MSK		0x0008
#define RTL8366S_GREEN_FEATURE_RX_BIT		4
#define RTL8366S_GREEN_FEATURE_RX_MSK		0x0010

/* Port ability */
#define RTL8366S_PORT_ABILITY_BASE			0x0011

/* V-LAN member configuration */
#define RTL8366S_VLAN_MEMCONF_BASE			0x0016
#define RTL8366S_VLAN_TB_CTRL_REG				0x010F
#define RTL8366S_VLAN_TB_BIT					0
#define RTL8366S_VLAN_TB_MSK					0x0001
#define RTL8366S_SPT_STATE_BASE				0x003A

/* cpu port control reg */
#define RTL8366S_CPU_CTRL_REG					0x004F
#define RTL8366S_CPU_DRP_BIT					14
#define RTL8366S_CPU_DRP_MSK					0x4000
#define RTL8366S_CPU_INSTAG_BIT				15
#define RTL8366S_CPU_INSTAG_MSK				0x8000

/* port vlan control register */
#define RTL8366S_PORT_VLAN_CTRL_BASE			0x0058

/*port linking status*/
#define RTL8366S_PORT_LINK_STATUS_BASE		0x0060
#define RTL8366S_PORT_STATUS_SPEED_BIT		0
#define RTL8366S_PORT_STATUS_SPEED_MSK		0x0003
#define RTL8366S_PORT_STATUS_DUPLEX_BIT		2
#define RTL8366S_PORT_STATUS_DUPLEX_MSK		0x0004
#define RTL8366S_PORT_STATUS_LINK_BIT			4
#define RTL8366S_PORT_STATUS_LINK_MSK		0x0010
#define RTL8366S_PORT_STATUS_TXPAUSE_BIT		5
#define RTL8366S_PORT_STATUS_TXPAUSE_MSK	0x0020
#define RTL8366S_PORT_STATUS_RXPAUSE_BIT		6
#define RTL8366S_PORT_STATUS_RXPAUSE_MSK	0x0040
#define RTL8366S_PORT_STATUS_AN_BIT			7
#define RTL8366S_PORT_STATUS_AN_MSK			0x0080

/*internal control*/
#define RTL8366S_RESET_CONTROL_REG			0x0100
#define RTL8366S_RESET_QUEUE_BIT				2

#define RTL8366S_CHIP_ID_REG					0x0105

/* Table Acess Control */
#define RTL8366S_TABLE_ACCESS_CTRL_REG		0x0180
#define RTL8366S_TABLE_WRITE_BASE				0x0182
#define RTL8366S_TABLE_READ_BASE				0x0188
#define RTL8366S_VLAN_TABLE_WRITE_BASE		0x0185
#define RTL8366S_VLAN_TABLE_READ_BASE		0x018b

#define RTL8366S_TABLE_VLAN_WRITE_CTRL		0x0F01	
#define RTL8366S_TABLE_VLAN_READ_CTRL		0x0E01	
#define RTL8366S_TABLE_L2TB_WRITE_CTRL		0x0101	
#define RTL8366S_TABLE_L2TB_READ_CTRL			0x0001	
#define RTL8366S_TABLE_CAMTB_WRITE_CTRL		0x0301
#define RTL8366S_TABLE_CAMTB_READ_CTRL		0x0201

/* storm filtering control */
#define RTL8366S_STORM_FILTERING_1_REG		0x02E2	
#define RTL8366S_STORM_FILTERING_PERIOD_BIT	0
#define RTL8366S_STORM_FILTERING_PERIOD_MSK	0x0003
#define RTL8366S_STORM_FILTERING_COUNT_BIT	2
#define RTL8366S_STORM_FILTERING_COUNT_MSK	0x000C
#define RTL8366S_STORM_FILTERING_BC_BIT		8

#define RTL8366S_STORM_FILTERING_2_REG		0x02E3	
#define RTL8366S_STORM_FILTERING_MC_BIT		0
#define RTL8366S_STORM_FILTERING_UNDA_BIT	8

/* ACL registers address */
#define RTL8366S_ACL_CONTROL_REG				0x0300
#define RTL8366S_ACL_INC_ING_BIT				12
#define RTL8366S_ACL_RANGE_REG_BASE			0x0301

#define RTL8366S_ACL_METER_BASE				0x0305
#define RTL8366S_ACL_METER_RATE_MSK			0x3FFF
#define RTL8366S_ACL_METER_TOLE_MSK			0x003F

/* Protocol and port based 802.1q */
#define RTL8366S_PROTOCOL_GDATA_BASE			0x0350
#define RTL8366S_PROTOCOL_VLAN_CTRL_BASE		0x0358

/* SVLAN registers address */
#define RTL8366S_SVLAN_CTRL_REG				0x0370
#define RTL8366S_SVLAN_EN_BIT					0
#define RTL8366S_SVLAN_EN_MSK					0x0001
#define RTL8366S_SVLAN_PRIO_BIT				1
#define RTL8366S_SVLAN_PRIO_MSK				0x0002
#define RTL8366S_SVLAN_STAG_BIT				2
#define RTL8366S_SVLAN_STAG_MSK				0x0004
#define RTL8366S_SVLAN_PORT_OFF				3
#define RTL8366S_SVLAN_PORT_MSK				0x001F8

#define RTL8366S_SVLAN_PROTOCOL_TYPE			0x0371
#define RTL8366S_SVLAN_STAG_CTRL_BASE		0x0372

#define RTL8366S_SVLAN_PORT_CTRL_REG			0x0376
#define RTL8366S_SVLAN_VSIDX_BIT				2
#define RTL8366S_SVLAN_VSIDX_MSK				0x0003

#define RTL8366S_VLAN_TAGINGRESS_REG			0x0378

#define RTL8366S_VLAN_MEMBERINGRESS_REG		0x0379



/* 802.3 ad Link aggragation register */
#define RTL8366S_LINK_AGGREGATION_CTRL_REG			0x0380
#define RTL8366S_LINK_AGGREGATION_MODE_BIT			0
#define RTL8366S_LINK_AGGREGATION_MODE_MSK			0x0001
#define RTL8366S_LINK_AGGREGATION_HASHSEL_BIT		1
#define RTL8366S_LINK_AGGREGATION_HASHSEL_MSK		0x0006
#define RTL8366S_LINK_AGGREGATION_PORTMASK_BIT		3
#define RTL8366S_LINK_AGGREGATION_PORTMASK_MSK		0x01F8

#define RTL8366S_LINK_AGGREGATION_MAPPING_BASE		0x0381
#define RTL8366S_LINK_AGGREGATION_MAPPING_BIT		3
#define RTL8366S_LINK_AGGREGATION_MAPPING_MSK		0x0007

#define RTL8366S_LINK_AGGREGATION_FC_CTRL_REG		0x0383
#define RTL8366S_LINK_AGGREGATION_FC_MSK			0x003F

#define RTL8366S_LINK_AGGREGATION_QEMPTY_REG		0x0384
#define RTL8366S_LINK_AGGREGATION_QEMPTY_MSK		0x003F

/* 802.1x access control */
#define PB1XCR0											(0x0 + 0x340)  /* 802.1X port base control register 0 */
#define PB1XCR1											(0x1 + 0x340)  /* 802.1X port base control register 1 */
#define MBXCR											(0x2 + 0x340)  /* 802.1X mac base control register*/
#define GVCR												(0x3 + 0x340)  /* Guest VLAN register*/

#define RTL8366S_PBPORTOFFSET							0
#define RTL8366S_PBAUTHOFFSET							6
#define RTL8366S_PBOPDIROFFSET						12
#define RTL8366S_MBPORTOFFSET							0
#define RTL8366S_MBOPDIROFFSET						7
#define RTL8366s_1XPROCOFFSET							0
#define RTL8366S_GVIDXOFFSET							2
#define RTL8366S_GVTALKOFFSET							6

#define RTL8366S_PBPORTMASK							0x3f
#define RTL8366S_PBAUTHMASK 							0xfc0
#define RTL8366S_PBOPDIRMASK							0x3f000
#define RTL8366S_MBPORTMASK							0x3f
#define RTL8366S_MBOPDIRMASK							0x80
#define RTL8366s_1XPROCMASK							0x03
#define RTL8366S_GVIDXMASK								0x3C
#define RTL8366S_GVTALKMASK							0x40

#define RTL8366S_MBOPDIRBIT							1
#define RTL8366S_1XPROCMAX								0x2
#define RTL8366S_GVIDXMAX								16
#define RTL8366S_GVTALKBIT								1

/* RMA register address */
#define RTL8366S_LUT_CONTROL_REG						0x0390
#define RTL8366S_EN_IPMULTICAST_LUT_BIT				0
#define RTL8366S_EN_IPMULTICAST_LUT_MSK				0x0001
#define RTL8366S_EN_IPMULTICAST_VLAN_LEAKY_BIT		1
#define RTL8366S_EN_IPMULTICAST_VLAN_LEAKY_MSK		0x007E

#define RTL8366S_RMA_CONTROL_REG						0x0391
#define RTL8366S_RMA_USER_DEFINED_BASE				0x0392

/* Cable Testing register address */
#define RTL8366S_CABLE_TESTING_BASE					0x0402
#define RTL8366S_CABLE_TESTING_START_BIT				0
#define RTL8366S_CABLE_TESTING_START_MSK				0x0000
#define RTL8366S_CABLE_TESTING_ACT_BIT				1
#define RTL8366S_CABLE_TESTING_ACT_MSK				0x0002
#define RTL8366S_CABLE_TESTING_FINISH_BIT				2
#define RTL8366S_CABLE_TESTING_FINISH_MSK			0x0004
#define RTL8366S_CABLE_TESTING_CH_MIS_BIT			4
#define RTL8366S_CABLE_TESTING_CH_MIS_MSK			0x0010
#define RTL8366S_CABLE_TESTING_CH_OPEN_BIT			8
#define RTL8366S_CABLE_TESTING_CH_OPEN_MSK			0x0100
#define RTL8366S_CABLE_TESTING_CH_SHORT_BIT			12
#define RTL8366S_CABLE_TESTING_CH_SHORT_MSK			0x1000
#define RTL8366S_CABLE_TESTING_CH_LENGTH_BASE		0x0403


/* Interrupt control address */
#define RTL8366S_INTERRUPT_CONTROL_REG				0x0440
#define RTL8366S_INTERRUPT_POLARITY_BIT				0
#define RTL8366S_INTERRUPT_POLARITY_MSK				0x0001

#define RTL8366S_INTERRUPT_MASK_REG					0x0441
#define RTL8366S_INTERRUPT_MASK_MSK					0x0FFF
#define RTL8366S_INTERRUPT_LINKDOWN_BIT				6



#define RTL8366S_INTERRUPT_STATUS_REG				0x0442
#define RTL8366S_INTERRUPT_STATUS_MSK				0x0FFF

/* MIBs control */
#define RTL8366S_MIB_COUTER_BASE				0x1000
#define RTL8366S_MIB_COUTER_PORT_OFFSET		0x0040
#define RTL8366S_MIB_COUTER_2_BASE			0x1180
#define RTL8366S_MIB_COUTER2_PORT_OFFSET		0x0008
#define RTL8366S_MIB_DOT1DTPLEARNDISCARD	0x11B0

#define RTL8366S_MIB_CTRL_REG					0x11F0

#define RTL8366S_MIB_CTRL_USER_MSK			0x01FF
#define RTL8366S_MIB_CTRL_BUSY_MSK			0x0001
#define RTL8366S_MIB_CTRL_RESET_MSK			0x0002

#define RTL8366S_MIB_CTRL_GLOBAL_RESET_MSK	0x0004
#define RTL8366S_MIB_CTRL_PORT_RESET_BIT		0x0003
#define RTL8366S_MIB_CTRL_PORT_RESET_MSK		0x01FC

/*MAC control*/
#define RTL8366S_MAC_FORCE_CTRL0_REG			0x0F04
#define RTL8366S_MAC_FORCE_CTRL1_REG			0x0F05


/* PHY registers control */
#define RTL8366S_PHY_ACCESS_CTRL_REG			0x8028
#define RTL8366S_PHY_ACCESS_DATA_REG			0x8029

#define RTL8366S_PHY_CTRL_READ				1
#define RTL8366S_PHY_CTRL_WRITE				0

#define RTL8366S_PHY_REG_MASK					0x1F
#define RTL8366S_PHY_PAGE_OFFSET				5
#define RTL8366S_PHY_PAGE_MASK				(0x7<<5)
#define RTL8366S_PHY_NO_OFFSET				9
#define RTL8366S_PHY_NO_MASK					(0x1F<<9)




/* Packet Scheduling Control Register */
#define RTL8366S_SCH_TGCR_REG					0x0270
#define RTL8366S_SCH_TICK_OFFSET				(0)                    
#define RTL8366S_SCH_TICK_MSK					(0xff<<0)              
#define RTL8366S_SCH_TOKEN_OFFSET				(8)                    
#define RTL8366S_SCH_TOKEN_MSK				(0xff<<8)              

#define RTL8366S_SCH_T2LBCR_REG				0x02D8
#define RTL8366S_SCH_TYPE2LBTH_OFFSET		(0)                    
#define RTL8366S_SCH_TYPE2LBTH_MSK			(0xffff<<0)              


/* Scheduler Control Register */
#define RTL8366S_SCR_P4_P0_REG				0x0271
#define RTL8366S_SCR_P5_REG					0x0272
#define RTL8366S_SCR_APR_OFFSET				(0)
#define RTL8366S_SCR_APR_MSK					(0x1<<0)
#define RTL8366S_SCR_PPR_OFFSET				(1)
#define RTL8366S_SCR_PPR_MSK					(0x1<<1)
#define RTL8366S_SCR_WFQ_OFFSET				(2)
#define RTL8366S_SCR_WFQ_MSK					(0x1<<2)
#define RTL8366S_SCR_P0_SCHDIS_OFFSET		(0)
#define RTL8366S_SCR_P0_SCHDIS_MSK			(0x7<<0)
#define RTL8366S_SCR_P1_SCHDIS_OFFSET		(3)
#define RTL8366S_SCR_P1_SCHDIS_MSK			(0x7<<3)
#define RTL8366S_SCR_P2_SCHDIS_OFFSET		(6)
#define RTL8366S_SCR_P2_SCHDIS_MSK			(0x7<<6)
#define RTL8366S_SCR_P3_SCHDIS_OFFSET		(9)
#define RTL8366S_SCR_P3_SCHDIS_MSK			(0x7<<9)
#define RTL8366S_SCR_P4_SCHDIS_OFFSET		(12)
#define RTL8366S_SCR_P4_SCHDIS_MSK			(0x7<<12)
#define RTL8366S_SCR_P5_SCHDIS_OFFSET		(0)
#define RTL8366S_SCR_P5_SCHDIS_MSK			(0x7<<0)

#define RTL8366S_EBC_INC_IFG_REG				0x0272
#define RTL8366S_EBC_INC_IFG_OFFSET			(3)
#define RTL8366S_EBC_INC_IFG_MSK				(0x1<<3)

/* Leaky Bucket Parameters Register */
#define RTL8366S_LBR_APR_BASE					0x0273
#define RTL8366S_LBR_PPR_BASE					0x0274
#define RTL8366S_LBR_APR_REG(port, queue)		(RTL8366S_LBR_APR_BASE+(0xC*port)+(0x2*queue))
#define RTL8366S_LBR_PPR_REG(port, queue)		(RTL8366S_LBR_PPR_BASE+(0xC*port)+(0x2*queue))
#define RTL8366S_LBR_APR_OFFSET				(0)
#define RTL8366S_LBR_APR_MSK					(0x3fff<<0)
#define RTL8366S_LBR_SIZE_OFFSET				(0)
#define RTL8366S_LBR_SIZE_MSK					(0x3f<<0)
#define RTL8366S_LBR_PPR_OFFSET				(6)
#define RTL8366S_LBR_PPR_MSK					(0x7<<6)

/* Weighted Fair Queue Parameters Register */
#define RTL8366S_WFQ_BASE						0x02C1
#define RTL8366S_WFQ_EN_REG(port)				(RTL8366S_WFQ_BASE+(0x4*port))
#define RTL8366S_WFQ_EN_OFFSET(queue)		(queue)
#define RTL8366S_WFQ_EN_MSK(queue)			(0x1<<queue)


#define RTL8366S_WFQ_EN_Q0_OFFSET			(0)
#define RTL8366S_WFQ_EN_Q0_MSK				(0x1<<0)
#define RTL8366S_WFQ_EN_Q1_OFFSET			(1)
#define RTL8366S_WFQ_EN_Q1_MSK				(0x1<<1)
#define RTL8366S_WFQ_EN_Q2_OFFSET			(2)
#define RTL8366S_WFQ_EN_Q2_MSK				(0x1<<2)
#define RTL8366S_WFQ_EN_Q3_OFFSET			(3)
#define RTL8366S_WFQ_EN_Q3_MSK				(0x1<<3)
#define RTL8366S_WFQ_EN_Q4_OFFSET			(4)
#define RTL8366S_WFQ_EN_Q4_MSK				(0x1<<4)
#define RTL8366S_WFQ_EN_Q5_OFFSET			(5)
#define RTL8366S_WFQ_EN_Q5_MSK				(0x1<<5)


#define RTL8366S_WFQ_Q0_REG(port)				(RTL8366S_WFQ_BASE+(0x4*port))
#define RTL8366S_WFQ_Q0_WEIGHT_OFFSET		(6)
#define RTL8366S_WFQ_Q0_WEIGHT_MSK			(0x7f<<6)
#define RTL8366S_WFQ_Q1_2_0_REG(port)			(RTL8366S_WFQ_BASE+(0x4*port))
#define RTL8366S_WFQ_Q1_2_0_WEIGHT_OFFSET	(13)
#define RTL8366S_WFQ_Q1_2_0_WEIGHT_MSK		(0x7<<13)
#define RTL8366S_WFQ_Q1_2_0_BIT_OFFSET		(0)
#define RTL8366S_WFQ_Q1_2_0_BIT_MSK			(0x7<<0)
#define RTL8366S_WFQ_Q1_6_3_REG(port)			(RTL8366S_WFQ_BASE+(0x4*port)+1)
#define RTL8366S_WFQ_Q1_6_3_WEIGHT_OFFSET	(0)
#define RTL8366S_WFQ_Q1_6_3_WEIGHT_MSK		(0xf<<0)
#define RTL8366S_WFQ_Q1_6_3_BIT_OFFSET		(3)
#define RTL8366S_WFQ_Q1_6_3_BIT_MSK			(0xf<<3)
#define RTL8366S_WFQ_Q2_REG(port)				(RTL8366S_WFQ_BASE+(0x4*port)+1)
#define RTL8366S_WFQ_Q2_WEIGHT_OFFSET		(4)
#define RTL8366S_WFQ_Q2_WEIGHT_MSK			(0x7f<<4)
#define RTL8366S_WFQ_Q3_4_0_REG(port)			(RTL8366S_WFQ_BASE+(0x4*port)+1)
#define RTL8366S_WFQ_Q3_4_0_WEIGHT_OFFSET	(11)
#define RTL8366S_WFQ_Q3_4_0_WEIGHT_MSK		(0x1f<<11)
#define RTL8366S_WFQ_Q3_4_0_BIT_OFFSET		(0)
#define RTL8366S_WFQ_Q3_4_0_BIT_MSK			(0x1f<<0)
#define RTL8366S_WFQ_Q3_6_5_REG(port)			(RTL8366S_WFQ_BASE+(0x4*port)+2)
#define RTL8366S_WFQ_Q3_6_5_WEIGHT_OFFSET	(0)
#define RTL8366S_WFQ_Q3_6_5_WEIGHT_MSK		(0x3<<0)
#define RTL8366S_WFQ_Q3_6_5_BIT_OFFSET		(5)
#define RTL8366S_WFQ_Q3_6_5_BIT_MSK			(0x3<<5)
#define RTL8366S_WFQ_Q4_REG(port)				(RTL8366S_WFQ_BASE+(0x4*port)+2)
#define RTL8366S_WFQ_Q4_WEIGHT_OFFSET		(2)
#define RTL8366S_WFQ_Q4_WEIGHT_MSK			(0x7f<<2)
#define RTL8366S_WFQ_Q5_REG(port)				(RTL8366S_WFQ_BASE+(0x4*port)+2)
#define RTL8366S_WFQ_Q5_WEIGHT_OFFSET		(9)
#define RTL8366S_WFQ_Q5_WEIGHT_MSK			(0x7f<<9)
#define RTL8366S_WFQ_Q_WEIGHT_OFFSET		(0)
#define RTL8366S_WFQ_Q_WEIGHT_MSK			(0x7f<<0)

/* Bandwdith Control Register */
#define RTL8366S_IB_BASE						0x0200
#define RTL8366S_IB_REG(port)					(RTL8366S_IB_BASE+port)
#define RTL8366S_IB_BDTH_OFFSET		  		(0)
#define RTL8366S_IB_BDTH_MSK					(0x3fff<<0)
#define RTL8366S_IB_PREIFP_OFFSET				(14)
#define RTL8366S_IB_PREIFP_MSK					(0x1<<14)

#define RTL8366S_EB_BASE						0x02C0
#define RTL8366S_EB_REG(port)					(RTL8366S_EB_BASE+(0x4*port))
#define RTL8366S_EB_BDTH_OFFSET				(0)
#define RTL8366S_EB_BDTH_MSK					(0x3fff<<0)
 

/* Remarking Control Register */
#define RTL8366S_REM_REG						0x0229
#define RTL8366S_REM_1Q_OFFSET				(0)
#define RTL8366S_REM_1Q_MSK					(0x1<<0)
#define RTL8366S_REM_DSCP_OFFSET				(1)
#define RTL8366S_REM_DSCP_MSK					(0x1<<1)

#define RTL8366S_D1Q_BASE						0x022A
#define RTL8366S_D1Q_REG(pri)					(RTL8366S_D1Q_BASE+ ((pri>3)*0x1))
#define RTL8366S_D1Q_OFFSET(pri)				(pri*3-((pri>3)*12))
#define RTL8366S_D1Q_MSK(pri)					(0x7<<RTL8366S_D1Q_OFFSET(pri))

#define RTL8366S_DSCP_BASE						0x022C
#define RTL8366S_DSCP_REG(pri)					(RTL8366S_DSCP_BASE+ ((pri>>1)*0x1))
#define RTL8366S_DSCP_OFFSET(pri)				((pri & 0x1)*6)
#define RTL8366S_DSCP_MSK(pri)					(0x3f<<RTL8366S_DSCP_OFFSET(pri))


/* Priority Decision Control Register */
#define RTL8366S_PDCR_REG						0x021C
#define RTL8366S_PDCR_ACL_OFFSET				(0)
#define RTL8366S_PDCR_ACL_MSK					(0xf<<0)
#define RTL8366S_PDCR_DSCP_OFFSET				(4)
#define RTL8366S_PDCR_DSCP_MSK				(0xf<<4)
#define RTL8366S_PDCR_1Q_OFFSET				(8)
#define RTL8366S_PDCR_1Q_MSK					(0xf<<8)
#define RTL8366S_PDCR_PBP_OFFSET				(12)
#define RTL8366S_PDCR_PBP_MSK					(0xf<<12)


#define RTL8366S_PBP_P4_P0_REG				0x020C
#define RTL8366S_PBP_P5_PRE_REG				0x020C
#define RTL8366S_PBP_P5_END_REG				0x020D
#define RTL8366S_PBP_P0_OFFSET				(0)
#define RTL8366S_PBP_P0_MSK					(0x7<<0)
#define RTL8366S_PBP_P1_OFFSET				(3)
#define RTL8366S_PBP_P1_MSK					(0x7<<3)
#define RTL8366S_PBP_P2_OFFSET				(6)
#define RTL8366S_PBP_P2_MSK					(0x7<<6)
#define RTL8366S_PBP_P3_OFFSET				(9)
#define RTL8366S_PBP_P3_MSK					(0x7<<9)
#define RTL8366S_PBP_P4_OFFSET				(12)
#define RTL8366S_PBP_P4_MSK					(0x7<<12)
#define RTL8366S_PBP_P5_PRE_OFFSET			(15)
#define RTL8366S_PBP_P5_PRE_MSK				(0x1<<15)
#define RTL8366S_PBP_P5_END_OFFSET			(0)
#define RTL8366S_PBP_P5_END_MSK				(0X3<<0)
#define RTL8366S_PBP_P5_0_OFFSET				(0)
#define RTL8366S_PBP_P5_0_MSK					(0x1<<0)
#define RTL8366S_PBP_P5_2_1_OFFSET			(1)
#define RTL8366S_PBP_P5_2_1_MSK				(0X3<<1)


#define RTL8366S_1QMCR_BASE					0x021A
#define RTL8366S_1QMCR_REG(pri)					(RTL8366S_1QMCR_BASE + ((pri>3)*0x1))
#define RTL8366S_1QMCR_OFFSET(pri)				(pri*3-((pri>3)*12))
#define RTL8366S_1QMCR_MSK(pri)				(0x7<<RTL8366S_1QMCR_OFFSET(pri))

#define RTL8366S_DSCPPR_BASE					0x020E
#define RTL8366S_DSCPPR_0_4_REG(dscp)			(RTL8366S_DSCPPR_BASE + ((dscp>>4)*0x3))
#define RTL8366S_DSCPPR_5_PRE_REG(dscp)		(RTL8366S_DSCPPR_BASE + ((dscp>>4)*0x3))
#define RTL8366S_DSCPPR_5_END_REG(dscp)		(RTL8366S_DSCPPR_BASE + ((dscp>>4)*0x3) + 1)
#define RTL8366S_DSCPPR_6_9_REG(dscp)			(RTL8366S_DSCPPR_BASE + ((dscp>>4)*0x3) + 1)
#define RTL8366S_DSCPPR_10_PRE_REG(dscp)		(RTL8366S_DSCPPR_BASE + ((dscp>>4)*0x3) + 1)
#define RTL8366S_DSCPPR_10_END_REG(dscp)		(RTL8366S_DSCPPR_BASE + ((dscp>>4)*0x3) + 2)
#define RTL8366S_DSCPPR_11_15_REG(dscp)		(RTL8366S_DSCPPR_BASE + ((dscp>>4)*0x3) + 2)
#define RTL8366S_DSCPPR_0_4_OFFSET(dscp)		((dscp%16)*0x3)
#define RTL8366S_DSCPPR_0_4_MSK(dscp)			(0x7<<RTL8366S_DSCPPR_0_4_OFFSET(dscp))
#define RTL8366S_DSCPPR_5_PRE_OFFSET			(15)
#define RTL8366S_DSCPPR_5_PRE_MSK				(0x1<<15)
#define RTL8366S_DSCPPR_5_END_OFFSET			(0)
#define RTL8366S_DSCPPR_5_END_MSK			(0x3<<0)
#define RTL8366S_DSCPPR_5_PRE_BIT_OFFSET		(0)
#define RTL8366S_DSCPPR_5_PRE_BIT_MSK		(0x1<<0)
#define RTL8366S_DSCPPR_5_END_BIT_OFFSET		(1)
#define RTL8366S_DSCPPR_5_END_BIT_MSK		(0x3<<1)
#define RTL8366S_DSCPPR_6_9_OFFSET(dscp)		((dscp%16)*0x3 - 16)
#define RTL8366S_DSCPPR_6_9_MSK(dscp)			(0x7<<RTL8366S_DSCPPR_6_9_OFFSET(dscp))
#define RTL8366S_DSCPPR_10_PRE_OFFSET(dscp)	((dscp%16)*0x3 - 16)
#define RTL8366S_DSCPPR_10_PRE_MSK(dscp)		(0x3<<RTL8366S_DSCPPR_10_PRE_OFFSET(dscp))
#define RTL8366S_DSCPPR_10_END_OFFSET(dscp)	((dscp%16)*0x0)
#define RTL8366S_DSCPPR_10_END_MSK(dscp)		(0x1<<RTL8366S_DSCPPR_10_END_OFFSET(dscp))
#define RTL8366S_DSCPPR_10_PRE_BIT_OFFSET 	(0)
#define RTL8366S_DSCPPR_10_PRE_BIT_MSK		(0x3<<0)
#define RTL8366S_DSCPPR_10_END_BIT_OFFSET	(2)
#define RTL8366S_DSCPPR_10_END_BIT_MSK		(0x1<<2)
#define RTL8366S_DSCPPR_11_15_OFFSET(dscp)	((dscp%16)*0x3 - 32)
#define RTL8366S_DSCPPR_11_15_MSK(dscp)		(0x7<<RTL8366S_DSCPPR_11_15_OFFSET(dscp))


/* Output Queue Number Control Register */
#define RTL8366S_OQN_REG						0x021D
#define RTL8366S_OQN_OFFSET(port)				(port*3-((port>4)*15))                    
#define RTL8366S_OQN_MSK(port)					(0x7<<RTL8366S_OQN_OFFSET(port))              

#define RTL8366S_OQN_P5_0_REG					0x021D
#define RTL8366S_OQN_P5_2_1_REG				0x021E
#define RTL8366S_OQN_P5_0_OFFSET				(15)
#define RTL8366S_OQN_P5_0_MSK					(0x1<<15)
#define RTL8366S_OQN_P5_0_BIT_OFFSET			(0)
#define RTL8366S_OQN_P5_0_BIT_MSK				(0x1<<0)
#define RTL8366S_OQN_P5_2_1_OFFSET			(0)
#define RTL8366S_OQN_P5_2_1_MSK				(0x3<<0)
#define RTL8366S_OQN_P5_2_1_BIT_OFFSET		(1)
#define RTL8366S_OQN_P5_2_1_BIT_MSK			(0x3<<1)


/* Queue Number and Priority Mapping to Queue ID Control Register */
#define RTL8366S_QNPQID_BASE					0x0220
#define RTL8366S_QNPQID_QN1_PRE_REG(qnum)	(RTL8366S_QNPQID_BASE + qnum - (qnum<3) + (qnum>3))
#define RTL8366S_QNPQID_QN1_END_REG(qnum)	(RTL8366S_QNPQID_BASE + qnum - (qnum<3) + (qnum>3) + 1)
#define RTL8366S_QNPQID_QN2_PRE_REG(qnum)	(RTL8366S_QNPQID_BASE + qnum - (qnum<4) + (qnum>4))
#define RTL8366S_QNPQID_QN2_END_REG(qnum)	(RTL8366S_QNPQID_BASE + qnum - (qnum<4) + (qnum>4) + 1)


#define RTL8366S_QNPQID_QN1PQID0_OFFSET		(0)                    
#define RTL8366S_QNPQID_QN1PQID0_MSK			(0x7<<0)                    
#define RTL8366S_QNPQID_QN1PQID1_OFFSET		(3)                    
#define RTL8366S_QNPQID_QN1PQID1_MSK			(0x7<<3)                    
#define RTL8366S_QNPQID_QN1PQID2_OFFSET		(6)                    
#define RTL8366S_QNPQID_QN1PQID2_MSK			(0x7<<6)                    
#define RTL8366S_QNPQID_QN1PQID3_OFFSET		(9)                    
#define RTL8366S_QNPQID_QN1PQID3_MSK			(0x7<<9)                    
#define RTL8366S_QNPQID_QN1PQID4_OFFSET		(12)                    
#define RTL8366S_QNPQID_QN1PQID4_MSK			(0x7<<12)                    

#define RTL8366S_QNPQID_QN1PQID5_PRE_OFFSET		(15)                    
#define RTL8366S_QNPQID_QN1PQID5_PRE_MSK		(0x1<<15)                    
#define RTL8366S_QNPQID_QN1PQID5_END_OFFSET		(0)                    
#define RTL8366S_QNPQID_QN1PQID5_END_MSK		(0x3<<0)   
	
#define RTL8366S_QNPQID_QN1PQID5_0_OFFSET		(0)                    
#define RTL8366S_QNPQID_QN1PQID5_0_MSK			(0x1<<0)                    
#define RTL8366S_QNPQID_QN1PQID5_2_1_OFFSET		(1)                    
#define RTL8366S_QNPQID_QN1PQID5_2_1_MSK		(0x3<<1)   

#define RTL8366S_QNPQID_QN1PQID6_OFFSET			(2)                    
#define RTL8366S_QNPQID_QN1PQID6_MSK				(0x7<<2)                    
#define RTL8366S_QNPQID_QN1PQID7_OFFSET			(5)                    
#define RTL8366S_QNPQID_QN1PQID7_MSK				(0x7<<5)                    

#define RTL8366S_QNPQID_QN2PQID0_OFFSET			(8)                    
#define RTL8366S_QNPQID_QN2PQID0_MSK				(0x7<<8)                    
#define RTL8366S_QNPQID_QN2PQID1_OFFSET			(11)                    
#define RTL8366S_QNPQID_QN2PQID1_MSK				(0x7<<11)                    

#define RTL8366S_QNPQID_QN2PQID2_PRE_OFFSET		(14)                    
#define RTL8366S_QNPQID_QN2PQID2_PRE_MSK		(0x3<<14)                    
#define RTL8366S_QNPQID_QN2PQID2_END_OFFSET		(0)                    
#define RTL8366S_QNPQID_QN2PQID2_END_MSK		(0x1<<0)                    
#define RTL8366S_QNPQID_QN2PQID2_1_0_OFFSET		(0)                    
#define RTL8366S_QNPQID_QN2PQID2_1_0_MSK		(0x3<<0)                    
#define RTL8366S_QNPQID_QN2PQID2_2_OFFSET		(2)                    
#define RTL8366S_QNPQID_QN2PQID2_2_MSK			(0x1<<2)                    

#define RTL8366S_QNPQID_QN2PQID3_OFFSET			(1)                    
#define RTL8366S_QNPQID_QN2PQID3_MSK				(0x7<<1)                    
#define RTL8366S_QNPQID_QN2PQID4_OFFSET			(4)                    
#define RTL8366S_QNPQID_QN2PQID4_MSK				(0x7<<4)                    
#define RTL8366S_QNPQID_QN2PQID5_OFFSET			(7)                    
#define RTL8366S_QNPQID_QN2PQID5_MSK				(0x7<<7)                    
#define RTL8366S_QNPQID_QN2PQID6_OFFSET			(10)                    
#define RTL8366S_QNPQID_QN2PQID6_MSK				(0x7<<10)                    
#define RTL8366S_QNPQID_QN2PQID7_OFFSET			(13)                    
#define RTL8366S_QNPQID_QN2PQID7_MSK				(0x7<<13)                    


/* Flow Control Register */

/* Queue-descriptor-based */
#define RTL8366S_QDBFC_BASE						0x0230
#define RTL8366S_QDBFC_QG0_REG(port)				(RTL8366S_QDBFC_BASE+(port*0x3))
#define RTL8366S_QDBFC_QG1_REG(port)				(RTL8366S_QDBFC_BASE+(port*0x3)+1)
#define RTL8366S_QDBFC_QG2_REG(port)				(RTL8366S_QDBFC_BASE+(port*0x3)+2)
#define RTL8366S_QDBFC_FCON_OFFSET				0
#define RTL8366S_QDBFC_FCON_MSK					(0x7f<<0)
#define RTL8366S_QDBFC_FCOFF_OFFSET				7
#define RTL8366S_QDBFC_FCOFF_MSK					(0x1f<<7)

/* Queue-packet-based */
#define RTL8366S_QPBFC_BASE						0x0242
#define RTL8366S_QPBFC_QG0_REG(port)				(RTL8366S_QPBFC_BASE+(port*0x3))
#define RTL8366S_QPBFC_QG1_REG(port)				(RTL8366S_QPBFC_BASE+(port*0x3)+1)
#define RTL8366S_QPBFC_QG2_REG(port)				(RTL8366S_QPBFC_BASE+(port*0x3)+2)
#define RTL8366S_QPBFC_FCON_OFFSET				0
#define RTL8366S_QPBFC_FCON_MSK					(0x3f<<0)
#define RTL8366S_QPBFC_FCOFF_OFFSET				6
#define RTL8366S_QPBFC_FCOFF_MSK					(0xf<<6)

/* Port-based */
#define RTL8366S_PBFC_BASE							0x0254
#define RTL8366S_PBFC_ENQUE_REG(port)				(RTL8366S_PBFC_BASE+(port*0x2))
#define RTL8366S_PBFC_ENQUE_OFFSET(queue)		(queue)
#define RTL8366S_PBFC_ENQUE_MSK(queue)			(0x1<<queue)
#define RTL8366S_PBFC_FCOFF_REG(port)				(RTL8366S_PBFC_BASE+(port*0x2))
#define RTL8366S_PBFC_FCOFF_OFFSET				6
#define RTL8366S_PBFC_FCOFF_MSK					(0x1ff<<6)
#define RTL8366S_PBFC_FCON_REG(port)				(RTL8366S_PBFC_BASE+(port*0x2)+1)
#define RTL8366S_PBFC_FCON_OFFSET					0
#define RTL8366S_PBFC_FCON_MSK					(0x1ff<<0)

/* System-based */
#define RTL8366S_SBFC_FCOFF_REG					0x0260
#define RTL8366S_SBFC_FCOFF_OFFSET				0
#define RTL8366S_SBFC_FCOFF_MSK					(0x3ff<<0)
#define RTL8366S_SBFC_FCON_REG					0x0261
#define RTL8366S_SBFC_FCON_OFFSET					0
#define RTL8366S_SBFC_FCON_MSK					(0x3ff<<0)	
#define RTL8366S_SBFC_RUNOUT_REG					0x0262
#define RTL8366S_SBFC_RUNOUT_OFFSET				0
#define RTL8366S_SBFC_RUNOUT_MSK					(0x3ff<<0)

/* Queue Gap */
#define RTL8366S_PQSOCR_GAP_REG					0x0263
#define RTL8366S_PQSOCR_GAP_OFFSET				0
#define RTL8366S_PQSOCR_GAP_MSK					(0x1ff<<0)

/* Packet-Used-Pages Flow control */
#define RTL8366S_PUPFCR_REG						0x0264
#define RTL8366S_PUPFCR_FCON_OFFSET				0
#define RTL8366S_PUPFCR_FCON_MSK					(0x1ff<<0)
#define RTL8366S_PUPFCR_EN_OFFSET					15
#define RTL8366S_PUPFCR_EN_MSK					(0x1<<15)


/* LED registers*/
#define RTL8366S_LED_BLINK_REG						0x420
#define RTL8366S_LED_BLINKRATE_BIT				0
#define RTL8366S_LED_BLINKRATE_MSK				0x0007
#define RTL8366S_LED_INDICATED_CONF_REG			0x421
#define RTL8366S_LED_0_1_FORCE_REG				0x422
#define RTL8366S_LED_2_3_FORCE_REG				0x423

#define RTL8366S_LINK_AMPLITUDE_0_REG				0x8014
#define RTL8366S_LINK_AMPLITUDE_1_REG				0x8015

/* enum for port ID */
enum PORTID
{
	PORT0 =  0,
	PORT1,
	PORT2,
	PORT3,
	PORT4,
	PORT5,
	PORT_MAX
};

/* enum for port ability speed */
enum PORTABILITYSPEED
{
	SPD_10M_H = 1,
	SPD_10M_F,
	SPD_100M_H,
	SPD_100M_F,
	SPD_1000M_F
};

/* enum for port current link speed */
enum PORTLINKSPEED
{
	SPD_10M = 0,
	SPD_100M,
	SPD_1000M
};

/* enum for mac link mode */
enum MACLINKMODE
{
	MAC_NORMAL = 0,
	MAC_FORCE,
};

/* enum for port current link duplex mode */
enum PORTLINKDUPLEXMODE
{
	HALF_DUPLEX = 0,
	FULL_DUPLEX
};

/* spanning tree state */
enum SPTSTATE
{
	DISABLED = 0,
	BLOCKING,
	LEARNING,
	FORWARDING
};

enum ACLFORMAT
{
	ACL_MAC = 0,
	ACL_IPV4,
	ACL_IPV4_ICMP,
	ACL_IPV4_IGMP,
	ACL_IPV4_TCP,
	ACL_IPV4_UDP,
	ACL_IPV6_SIP,	
	ACL_IPV6_DIP,	
	ACL_IPV6_EXT,	
	ACL_IPV6_TCP,	
	ACL_IPV6_UDP,	
	ACL_IPV6_ICMP,	
	ACL_DUMB = 15,
};

/* enum for output queue number */
enum QUEUENUM
{
	QNUM1 = 1,
	QNUM2,
	QNUM3,
	QNUM4,
	QNUM5,
	QNUM6,
};

/* enum for queue ID */
enum QUEUEID
{
	QUEUE0 = 0,
	QUEUE1,
	QUEUE2,
	QUEUE3,
	QUEUE4,
	QUEUE5,
};

/* enum for queue type */
enum QUEUETYPE
{
	STR_PRIO = 0,
	WFQ_PRIO,
};

/* enum for priority value type */
enum PRIORITYVALUE
{
	PRI0 = 0,
	PRI1,
	PRI2,
	PRI3,
	PRI4,
	PRI5,
	PRI6,
	PRI7,
};

/* enum for RMA trapping frame type */
enum RMATRAPFRAME
{
	RMA_BRG_GROUP = 0,
	RMA_FD_PAUSE,
	RMA_SP_MCAST,
	RMA_1X_PAE,
	RMA_BRG_MNGEMENT,
	RMA_GMRP,
	RMA_GVRP,
	RMA_UNDEF_BRG,
	RMA_UNDEF_GARP,
	RMA_IGMP_MLD_PPPOE,
	RMA_IGMP,
	RMA_MLD,
	RMA_USER_DEF1,	
	RMA_USER_DEF2,	
	RMA_USER_DEF3,	
	RMA_USER_DEF4,		
};

enum CHANNELID
{
	CH_A =  0,
	CH_B,
	CH_C,
	CH_D,
	CH_MAX
};


typedef struct   smi_ether_addr_s{

#ifdef _LITTLE_ENDIAN
	uint16	mac0:8;
	uint16	mac1:8;
	uint16	mac2:8;
	uint16	mac3:8;
	uint16	mac4:8;
	uint16	mac5:8;
#else
	uint16	mac1:8;
	uint16	mac0:8;
	uint16	mac3:8;
	uint16	mac2:8;
	uint16	mac5:8;
	uint16	mac4:8;
#endif	
}smi_ether_addr_t;



typedef struct  VLANCONFIG{

#ifdef _LITTLE_ENDIAN
	uint16 	vid:12;
 	uint16 	priority:3;
 	uint16 	reserved1:1;

	uint16 	member:6;
 	uint16 	untag:6;
 	uint16 	fid:3;
	uint16 	reserved2:1;

#else
  	uint16 	reserved2:1;
	uint16 	priority:3;
	uint16 	vid:12;


	uint16 	reserved1:1;
 	uint16 	fid:3;
 	uint16 	untag:6;
 	uint16 	member:6;

#endif
}rtl8366s_vlanconfig;


typedef struct  VLANCONVIDPRIO{

#ifdef _LITTLE_ENDIAN
	uint16 	vid:12;
 	uint16 	priority:3;
	uint16 	dummy:1;

#else
	uint16 	dummy:1;
 	uint16 	priority:3;
	uint16 	vid:12;

#endif
}rtl8366s_vlanconvidprio;

typedef struct  VLANCONMUF{

#ifdef _LITTLE_ENDIAN
 	uint16 	member:6;
 	uint16 	untag:6;
 	uint16 	fid:3;
	uint16 	dummy:1;
#else
	uint16 	dummy:1;
 	uint16 	fid:3;
 	uint16 	untag:6;
 	uint16 	member:6;
#endif
}rtl8366s_vlanconmuf;



typedef struct  VLANTABLE{

#ifdef _LITTLE_ENDIAN
	uint16 	vid:12;
 	uint16 	reserved1:4;
	
 	uint16 	member:6;
 	uint16 	untag:6;
 	uint16 	fid:3;
	uint16 	reserved2:1;

#else
 	uint16 	reserved1:4;
	uint16 	vid:12;

	uint16 	reserved2:1;
 	uint16 	fid:3;
 	uint16 	untag:6;
 	uint16 	member:6;
	
#endif
}rtl8366s_vlan4kentry;




struct smi_l2tb_ipmulticast_st{
	
#ifdef _LITTLE_ENDIAN
	uint16 	sip0:8;
	uint16 	sip1:8;
	uint16 	sip2:8;
	uint16 	sip3:8;
	uint16 	dip0:8;
	uint16 	dip1:8;
	uint16 	dip2:8;
	uint16 	dip3:8;

	uint16 	 mbr:6;
	uint16 	 reserved1:9;
	uint16 	 ipmulti:1;

	uint16 	 reserved2;

#else
	uint16 	sip2:8;
	uint16 	sip3:8;
	uint16 	sip0:8;
	uint16 	sip1:8;
	uint16 	dip2:8;
	uint16 	dip3:8;
	uint16 	dip0:8;
	uint16 	dip1:8;

	uint16 	 ipmulti:1;
	uint16 	 reserved1:9;
	uint16 	 mbr:6;

	uint16 	 reserved2;
#endif
	
};


struct smi_l2tb_macstatic_st{

#ifdef _LITTLE_ENDIAN

	uint16	mac0:8;
	uint16	mac1:8;
	uint16	mac2:8;
	uint16	mac3:8;
	uint16	mac4:8;
	uint16	mac5:8;

	uint16 	fid:3;
	uint16 	reserved1:13;

	uint16 	mbr:6;
	uint16 	reserved2:7;
	uint16 	auth:1;
	uint16 	swst:1;
	uint16 	ipmulti:1;

	uint16 	reserved3;
#else
	uint16	mac1:8;
	uint16	mac0:8;
	uint16	mac3:8;
	uint16	mac2:8;
	uint16	mac5:8;
	uint16	mac4:8;

	uint16 	reserved1:13;
	uint16 	fid:3;

	uint16 	ipmulti:1;
	uint16 	swst:1;
	uint16 	auth:1;
	uint16 	reserved2:7;
	uint16 	mbr:6;

	uint16 	reserved3;
#endif	
};

struct smi_l2tb_maclearn_st{

#ifdef _LITTLE_ENDIAN

	ether_addr_t 	mac;

	uint16 	fid:3;
	uint16 	reserved1:13;

	uint16 	spa:3;
	uint16 	age:2;
	uint16 	reserved2:8;
	uint16 	auth:1;
	uint16 	swst:1;
	uint16 	ipmulti:1;
	
	uint16 	reserved3;
#else
	ether_addr_t 	mac;

	uint16 	reserved1:13;
	uint16 	fid:3;
	
	uint16 	ipmulti:1;
	uint16 	swst:1;
	uint16 	auth:1;
	uint16 	reserved2:8;
	uint16 	age:2;
	uint16 	spa:3;

	
	uint16 	reserved3;


#endif
};

struct l2tb_ipmulticast_st{
	
#ifdef _LITTLE_ENDIAN
	ipaddr_t sip;
	ipaddr_t dip;

	uint16 	 mbr:6;
	uint16 	 reserved1:9;
	uint16 	 ipmulti:1;

	uint16 	 reserved2;

#else
	ipaddr_t sip;
	ipaddr_t dip;

	uint16 	 ipmulti:1;
	uint16 	 reserved1:9;
	uint16 	 mbr:6;

	uint16 	 reserved2;
#endif
	
};

struct l2tb_macstatic_st{

#ifdef _LITTLE_ENDIAN

	ether_addr_t 	mac;

	uint16 	fid:3;
	uint16 	reserved1:13;

	uint16 	mbr:6;
	uint16 	reserved2:7;
	uint16 	auth:1;
	uint16 	swst:1;
	uint16 	ipmulti:1;

	uint16 	reserved3;
#else
	ether_addr_t 	mac;

	uint16 	reserved1:13;
	uint16 	fid:3;

	uint16 	ipmulti:1;
	uint16 	swst:1;
	uint16 	auth:1;
	uint16 	reserved2:7;
	uint16 	mbr:6;

	uint16 	reserved3;
#endif	
};

struct l2tb_maclearn_st{

#ifdef _LITTLE_ENDIAN

	ether_addr_t 	mac;

	uint16 	fid:3;
	uint16 	reserved1:13;

	uint16 	spa:3;
	uint16 	age:2;
	uint16 	reserved2:8;
	uint16 	auth:1;
	uint16 	swst:1;
	uint16 	ipmulti:1;
	
	uint16 	reserved3;
#else
	ether_addr_t 	mac;

	uint16 	reserved1:13;
	uint16 	fid:3;
	
	uint16 	ipmulti:1;
	uint16 	swst:1;
	uint16 	auth:1;
	uint16 	reserved2:8;
	uint16 	age:2;
	uint16 	spa:3;

	
	uint16 	reserved3;


#endif
};

typedef 	union L2TABLE{

		struct l2tb_ipmulticast_st	ipmul;
		struct l2tb_macstatic_st	swstatic;
		struct l2tb_maclearn_st	autolearn;	

		struct smi_l2tb_ipmulticast_st	smi_ipmul;
		struct smi_l2tb_macstatic_st	smi_swstatic;
		struct smi_l2tb_macstatic_st	smi_autolearn;
		
}rtl8366s_l2table;


struct camtb_macstatic_st{

#ifdef _LITTLE_ENDIAN
	ether_addr_t 	mac;

	uint16 	fid:3;
	uint16 	reserved1:13;

	uint16 	mbr:6;
	uint16 	reserved2:7;
	uint16 	auth:1;
	uint16 	swst:1;
	uint16 	reserved3:1;

	uint16 	reserved4;
#else
	ether_addr_t 	mac;

	uint16 	reserved1:13;
	uint16 	fid:3;

	uint16 	reserved3:1;
	uint16 	swst:1;
	uint16 	auth:1;
	uint16 	reserved2:7;
	uint16 	mbr:6;

	uint16 	reserved4;
#endif	
};

struct camtb_maclearn_st{

#ifdef _LITTLE_ENDIAN
	ether_addr_t 	mac;

	uint16 	fid:3;
	uint16 	reserved1:13;

	uint16 	spa:3;
	uint16 	age:2;
	uint16 	reserved2:8;
	uint16 	auth:1;
	uint16 	swst:1;
	uint16 	reserved3:1;
	
	uint16 	reserved4;
#else
	ether_addr_t 	mac;

	uint16 	reserved1:13;
	uint16 	fid:3;
	
	uint16 	reserved3:1;
	uint16 	swst:1;
	uint16 	auth:1;
	uint16 	reserved2:8;
	uint16 	age:2;
	uint16 	spa:3;
	
	uint16 	reserved4;;
#endif
};



struct smi_camtb_macstatic_st{

#ifdef _LITTLE_ENDIAN
	uint16	mac0:8;
	uint16	mac1:8;
	uint16	mac2:8;
	uint16	mac3:8;
	uint16	mac4:8;
	uint16	mac5:8;
	
	uint16 	fid:3;
	uint16 	reserved1:13;

	uint16 	mbr:6;
	uint16 	reserved2:7;
	uint16 	auth:1;
	uint16 	swst:1;
	uint16 	reserved3:1;

	uint16 	reserved4;
#else
	uint16	mac1:8;
	uint16	mac0:8;
	uint16	mac3:8;
	uint16	mac2:8;
	uint16	mac5:8;
	uint16	mac4:8;

	uint16 	reserved1:13;
	uint16 	fid:3;

	uint16 	reserved3:1;
	uint16 	swst:1;
	uint16 	auth:1;
	uint16 	reserved2:7;
	uint16 	mbr:6;

	uint16 	reserved4;
#endif	
};

typedef 	union CAMTABLE{

		struct camtb_macstatic_st	swstatic;
		struct camtb_maclearn_st	autolearn;			

		struct smi_camtb_macstatic_st	smi_swstatic;

}rtl8366s_camtable;


struct smi_mac_st{

#ifdef _LITTLE_ENDIAN

	uint16	dmp0:8;
	uint16	dmp1:8;
	uint16	dmp2:8;
	uint16	dmp3:8;
	uint16	dmp4:8;
	uint16	dmp5:8;

	uint16	dmm0:8;
	uint16	dmm1:8;
	uint16	dmm2:8;
	uint16	dmm3:8;
	uint16	dmm4:8;
	uint16	dmm5:8;

	uint16	smp0:8;
	uint16	smp1:8;
	uint16	smp2:8;
	uint16	smp3:8;
	uint16	smp4:8;
	uint16	smp5:8;

	uint16	smm0:8;
	uint16	smm1:8;
	uint16	smm2:8;
	uint16	smm3:8;
	uint16	smm4:8;
	uint16	smm5:8;

	uint16 	tlu;
	uint16 	tll;

	uint16	vtd:1;
	uint16	vtm:1;
	uint16	pu:3;
	uint16	pl:3;
	uint16	vidd_1:8;
	
	uint16	vidd_2:4;
	uint16	vidm:12;
#else
	uint16	dmp1:8;
	uint16	dmp0:8;
	uint16	dmp3:8;
	uint16	dmp2:8;
	uint16	dmp5:8;
	uint16	dmp4:8;

	uint16	dmm1:8;
	uint16	dmm0:8;
	uint16	dmm3:8;
	uint16	dmm2:8;
	uint16	dmm5:8;
	uint16	dmm4:8;

	uint16	smp1:8;
	uint16	smp0:8;
	uint16	smp3:8;
	uint16	smp2:8;
	uint16	smp5:8;
	uint16	smp4:8;

	uint16	smm1:8;
	uint16	smm0:8;
	uint16	smm3:8;
	uint16	smm2:8;
	uint16	smm5:8;
	uint16	smm4:8;
	
	uint16 	tlu;
	uint16 	tll;

	uint16	vidd_1:8;
	uint16	pl:3;
	uint16	pu:3;
	uint16	vtm:1;
	uint16	vtd:1;
	
	uint16	vidm:12;
	uint16	vidd_2:4;

#endif	
};	

struct mac_st{

	ether_addr_t 	dmp;
	ether_addr_t 	dmm;
	ether_addr_t 	smp;
	ether_addr_t 	smm;
	uint16 	tlu;
	uint16 	tll;
	uint16	vtd:1;
	uint16	vtm:1;
	uint16	pu:3;
	uint16	pl:3;
	uint16	vidd:12;
	uint16	vidm:12;

};	

struct smi_ipv4_st{
#ifdef _LITTLE_ENDIAN
	uint16 	sipU0:8;
	uint16 	sipU1:8;
	uint16 	sipU2:8;
	uint16 	sipU3:8;

	uint16 	sipL0:8;
	uint16 	sipL1:8;
	uint16 	sipL2:8;
	uint16 	sipL3:8;

	uint16 	dipU0:8;
	uint16 	dipU1:8;
	uint16 	dipU2:8;
	uint16 	dipU3:8;

	uint16 	dipL0:8;
	uint16 	dipL1:8;
	uint16 	dipL2:8;
	uint16 	dipL3:8;

	uint16	tosD:8;
	uint16	tosM:8;
	
	uint16	protoV:4;
	uint16	proto1:8;
	uint16	proto2_1:4;
	
	uint16	proto2_2:4;
	uint16	proto3:8;
	uint16	proto4_1:4;
	
	uint16	proto4_2:4;
	uint16	flagD:3;
	uint16	flagM:3;
	uint16	offU_1:6;

	uint16	offU_2:7;
	uint16	offL_1:9;
	
	uint16	offL_2:4;
	uint16	reserved:12;
#else
	uint16 	sipU2:8;
	uint16 	sipU3:8;
	uint16 	sipU0:8;
	uint16 	sipU1:8;

	uint16 	sipL2:8;
	uint16 	sipL3:8;
	uint16 	sipL0:8;
	uint16 	sipL1:8;

	uint16 	dipU2:8;
	uint16 	dipU3:8;
	uint16 	dipU0:8;
	uint16 	dipU1:8;

	uint16 	dipL2:8;
	uint16 	dipL3:8;
	uint16 	dipL0:8;
	uint16 	dipL1:8;

	uint16	tosM:8;
	uint16	tosD:8;
	
	uint16	proto2_1:4;
	uint16	proto1:8;
	uint16	protoV:4;
	
	uint16	proto4_1:4;
	uint16	proto3:8;
	uint16	proto2_2:4;
	
	uint16	offU_1:6;
	uint16	flagM:3;
	uint16	flagD:3;
	uint16	proto4_2:4;

	uint16	offL_1:9;
	uint16	offU_2:7;
	
	uint16	reserved:12;
	uint16	offL_2:4;
#endif
};

struct ipv4_st{

	ipaddr_t 	sipU;
	ipaddr_t 	sipL;
	ipaddr_t 	dipU;
	ipaddr_t 	dipL;
	uint16	tosD:8;
	uint16	tosM:8;
	uint16	protoV:4;
	uint16	proto1:8;
	uint16	proto2:8;
	uint16	proto3:8;
	uint16	proto4:8;
	uint16	flagD:3;
	uint16	flagM:3;
	uint16	offU:13;
	uint16	offL:13;
};


struct smi_ipv4_icmp_st{

#ifdef _LITTLE_ENDIAN
	uint16 	sipU0:8;
	uint16 	sipU1:8;
	uint16 	sipU2:8;
	uint16 	sipU3:8;

	uint16 	sipL0:8;
	uint16 	sipL1:8;
	uint16 	sipL2:8;
	uint16 	sipL3:8;

	uint16 	dipU0:8;
	uint16 	dipU1:8;
	uint16 	dipU2:8;
	uint16 	dipU3:8;

	uint16 	dipL0:8;
	uint16 	dipL1:8;
	uint16 	dipL2:8;
	uint16 	dipL3:8;

	uint16	tosD:8;
	uint16	tosM:8;
	
	uint16	typeV:4;
	uint16	type1:8;
	uint16	type2_1:4;
	
	uint16	type2_2:4;
	uint16	type3:8;
	uint16	type4_1:4;
	
	uint16	type4_2:4;
	uint16	codeV:4;
	uint16	code1:8;

	uint16	code2:8;
	uint16	code3:8;

	uint16	code4:8;
	uint16	reserved:8;
#else
	uint16 	sipU2:8;
	uint16 	sipU3:8;
	uint16 	sipU0:8;
	uint16 	sipU1:8;

	uint16 	sipL2:8;
	uint16 	sipL3:8;
	uint16 	sipL0:8;
	uint16 	sipL1:8;

	uint16 	dipU2:8;
	uint16 	dipU3:8;
	uint16 	dipU0:8;
	uint16 	dipU1:8;

	uint16 	dipL2:8;
	uint16 	dipL3:8;
	uint16 	dipL0:8;
	uint16 	dipL1:8;

	uint16	tosM:8;
	uint16	tosD:8;

	uint16	type2_1:4;	
	uint16	type1:8;
	uint16	typeV:4;
	
	uint16	type4_1:4;
	uint16	type3:8;
	uint16	type2_2:4;
	
	uint16	code1:8;
	uint16	codeV:4;
	uint16	type4_2:4;

	uint16	code3:8;
	uint16	code2:8;

	uint16	reserved:8;
	uint16	code4:8;
#endif
};

struct ipv4_icmp_st{

	ipaddr_t 	sipU;
	ipaddr_t 	sipL;
	ipaddr_t 	dipU;
	ipaddr_t 	dipL;
	uint16	tosD:8;
	uint16	tosM:8;
	uint16	typeV:4;
	uint16	type1:8;
	uint16	type2:8;
	uint16	type3:8;
	uint16	type4:8;
	uint16	codeV:4;
	uint16	code1:8;
	uint16	code2:8;
	uint16	code3:8;
	uint16	code4:8;
};



struct smi_ipv4_igmp_st{

#ifdef _LITTLE_ENDIAN
	uint16 	sipU0:8;
	uint16 	sipU1:8;
	uint16 	sipU2:8;
	uint16 	sipU3:8;

	uint16 	sipL0:8;
	uint16 	sipL1:8;
	uint16 	sipL2:8;
	uint16 	sipL3:8;

	uint16 	dipU0:8;
	uint16 	dipU1:8;
	uint16 	dipU2:8;
	uint16 	dipU3:8;

	uint16 	dipL0:8;
	uint16 	dipL1:8;
	uint16 	dipL2:8;
	uint16 	dipL3:8;

	uint16	tosD:8;
	uint16	tosM:8;
	
	uint16	typeV:4;
	uint16	type1:8;
	uint16	type2_1:4;
	
	uint16	type2_2:4;
	uint16	type3:8;
	uint16	type4_1:4;

	uint16	type4_2:4;
	uint16	reserved:12;
#else
	uint16 	sipU2:8;
	uint16 	sipU3:8;
	uint16 	sipU0:8;
	uint16 	sipU1:8;

	uint16 	sipL2:8;
	uint16 	sipL3:8;
	uint16 	sipL0:8;
	uint16 	sipL1:8;

	uint16 	dipU2:8;
	uint16 	dipU3:8;
	uint16 	dipU0:8;
	uint16 	dipU1:8;

	uint16 	dipL2:8;
	uint16 	dipL3:8;
	uint16 	dipL0:8;
	uint16 	dipL1:8;

	uint16	tosM:8;
	uint16	tosD:8;
	
	uint16	type2_1:4;
	uint16	type1:8;
	uint16	typeV:4;
	
	uint16	type4_1:4;
	uint16	type3:8;
	uint16	type2_2:4;

	uint16	reserved:12;
	uint16	type4_2:4;

#endif
};


struct ipv4_igmp_st{

	ipaddr_t 	sipU;
	ipaddr_t 	sipL;
	ipaddr_t 	dipU;
	ipaddr_t 	dipL;
	uint16	tosD:8;
	uint16	tosM:8;
	uint16	typeV:4;
	uint16	type1:8;
	uint16	type2:8;
	uint16	type3:8;
	uint16	type4:8;
};



struct smi_ipv4_tcp_st{

#ifdef _LITTLE_ENDIAN
	uint16 	sipU0:8;
	uint16 	sipU1:8;
	uint16 	sipU2:8;
	uint16 	sipU3:8;

	uint16 	sipL0:8;
	uint16 	sipL1:8;
	uint16 	sipL2:8;
	uint16 	sipL3:8;

	uint16 	dipU0:8;
	uint16 	dipU1:8;
	uint16 	dipU2:8;
	uint16 	dipU3:8;

	uint16 	dipL0:8;
	uint16 	dipL1:8;
	uint16 	dipL2:8;
	uint16 	dipL3:8;

	uint16	tosD:8;
	uint16	tosM:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
	
	uint16	flagD:6;
	uint16	flagM:6;
	uint16	reserved:4;
#else
	uint16 	sipU2:8;
	uint16 	sipU3:8;
	uint16 	sipU0:8;
	uint16 	sipU1:8;

	uint16 	sipL2:8;
	uint16 	sipL3:8;
	uint16 	sipL0:8;
	uint16 	sipL1:8;

	uint16 	dipU2:8;
	uint16 	dipU3:8;
	uint16 	dipU0:8;
	uint16 	dipU1:8;

	uint16 	dipL2:8;
	uint16 	dipL3:8;
	uint16 	dipL0:8;
	uint16 	dipL1:8;

	uint16	tosM:8;
	uint16	tosD:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
	
	uint16	reserved:4;
	uint16	flagM:6;
	uint16	flagD:6;
#endif
};

struct ipv4_tcp_st{

	ipaddr_t 	sipU;
	ipaddr_t 	sipL;
	ipaddr_t 	dipU;
	ipaddr_t 	dipL;
	uint16	tosD:8;
	uint16	tosM:8;
	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
	uint16	flagD:6;
	uint16	flagM:6;
};



struct smi_ipv4_udp_st{

#ifdef _LITTLE_ENDIAN
	uint16 	sipU0:8;
	uint16 	sipU1:8;
	uint16 	sipU2:8;
	uint16 	sipU3:8;

	uint16 	sipL0:8;
	uint16 	sipL1:8;
	uint16 	sipL2:8;
	uint16 	sipL3:8;

	uint16 	dipU0:8;
	uint16 	dipU1:8;
	uint16 	dipU2:8;
	uint16 	dipU3:8;

	uint16 	dipL0:8;
	uint16 	dipL1:8;
	uint16 	dipL2:8;
	uint16 	dipL3:8;

	uint16	tosD:8;
	uint16	tosM:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
#else
	uint16 	sipU2:8;
	uint16 	sipU3:8;
	uint16 	sipU0:8;
	uint16 	sipU1:8;

	uint16 	sipL2:8;
	uint16 	sipL3:8;
	uint16 	sipL0:8;
	uint16 	sipL1:8;

	uint16 	dipU2:8;
	uint16 	dipU3:8;
	uint16 	dipU0:8;
	uint16 	dipU1:8;

	uint16 	dipL2:8;
	uint16 	dipL3:8;
	uint16 	dipL0:8;
	uint16 	dipL1:8;

	uint16	tosM:8;
	uint16	tosD:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;

#endif
};

struct ipv4_udp_st{

	ipaddr_t 	sipU;
	ipaddr_t 	sipL;
	ipaddr_t 	dipU;
	ipaddr_t 	dipL;

	uint16	tosD:8;
	uint16	tosM:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
};



struct smi_ipv6_sip_st{

	uint16 	sipU[8];
	uint16 	sipL[8];
};
struct ipv6_sip_st{

	uint16 	sipU[8];
	uint16 	sipL[8];
};

struct smi_ipv6_dip_st{

	uint16 	dipU[8];
	uint16 	dipL[8];
};
struct ipv6_dip_st{

	uint16 	dipU[8];
	uint16 	dipL[8];
};

struct smi_ipv6_ext_st{

#ifdef _LITTLE_ENDIAN
	uint16	tcD:8;
	uint16	tcM:8;
	
	uint16	nhV:4;
	uint16	nhp1:8;
	uint16	nhp2_1:4;
	
	uint16	nhp2_2:4;
	uint16	nhp3:8;
	uint16	nhp4_1:4;

	uint16	nhp4_2:4;
	uint16	reserved:12;
#else
	uint16	tcM:8;
	uint16	tcD:8;
	
	uint16	nhp2_1:4;
	uint16	nhp1:8;
	uint16	nhV:4;
	
	uint16	nhp4_1:4;
	uint16	nhp3:8;
	uint16	nhp2_2:4;

	uint16	reserved:12;
	uint16	nhp4_2:4;
	
#endif
	
};

struct ipv6_ext_st{

	uint16	tcD:8;
	uint16	tcM:8;
	
	uint16	nhV:4;
	uint16	nhp1:8;
	uint16	nhp2:8;
	uint16	nhp3:8;
	uint16	nhp4:8;
	uint16	reserved:12;
	
};



struct smi_ipv6_tcp_st{

#ifdef _LITTLE_ENDIAN
	uint16	tcD:8;
	uint16	tcM:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
	
	uint16	flagD:6;
	uint16	flagM:6;
	uint16	reserved:4;

#else
	uint16	tcM:8;
	uint16	tcD:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
	
	uint16	reserved:4;
	uint16	flagM:6;
	uint16	flagD:6;

#endif
};

struct ipv6_tcp_st{

	uint16	tcD:8;
	uint16	tcM:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
	
	uint16	flagD:6;
	uint16	flagM:6;
};
struct smi_ipv6_udp_st{

#ifdef _LITTLE_ENDIAN
	uint16	tcD:8;
	uint16	tcM:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
#else
	uint16	tcM:8;
	uint16	tcD:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;


#endif	
};
struct ipv6_udp_st{

	uint16	tcD:8;
	uint16	tcM:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
};

struct smi_ipv6_icmp_st{

#ifdef _LITTLE_ENDIAN
	uint16	tcD:8;
	uint16	tcM:8;
	
	uint16	typeV:4;
	uint16	type1:8;
	uint16	type2_1:4;

	uint16	type2_2:4;
	uint16	type3:8;
	uint16	type4_1:4;

	uint16	type4_2:4;
	uint16	codeV:4;
	uint16	code1:8;

	uint16	code2:8;
	uint16	code3:8;
	
	uint16	code4:8;
	uint16	reserved:8;
#else
	uint16	tcM:8;
	uint16	tcD:8;
	
	uint16	type2_1:4;
	uint16	type1:8;
	uint16	typeV:4;

	uint16	type4_1:4;
	uint16	type3:8;
	uint16	type2_2:4;

	uint16	code1:8;
	uint16	codeV:4;
	uint16	type4_2:4;

	uint16	code3:8;
	uint16	code2:8;
	
	uint16	reserved:8;
	uint16	code4:8;
#endif
};


struct ipv6_icmp_st{

	uint16	tcD:8;
	uint16	tcM:8;
	
	uint16	typeV:4;
	uint16	type1:8;
	uint16	type2:8;
	uint16	type3:8;
	uint16	type4:8;
	uint16	codeV:4;
	uint16	code1:8;
	uint16	code2:8;
	uint16	code3:8;
	uint16	code4:8;
};



typedef union{

	struct smi_mac_st			smi_mac;	
	struct smi_ipv4_st			smi_ipv4;
	struct smi_ipv4_icmp_st	smi_ipv4icmp;
	struct smi_ipv4_igmp_st	smi_ipv4igmp;
	struct smi_ipv4_tcp_st		smi_ipv4tcp;
	struct smi_ipv4_udp_st		smi_ipv4udp;

	struct smi_ipv6_sip_st		smi_ipv6sip;
	struct smi_ipv6_dip_st		smi_ipv6dip;
	struct smi_ipv6_ext_st		smi_ipv6ext;
	struct smi_ipv6_tcp_st		smi_ipv6tcp;
	struct smi_ipv6_udp_st		smi_ipv6udp;
	struct smi_ipv6_icmp_st	smi_ipv6icmp;


	
}rtl8366s_smiaclrule;	




typedef struct  SMI_ACLTABLE{
	
#ifdef _LITTLE_ENDIAN

	rtl8366s_smiaclrule rule;

	uint16 ac_meteridx:4;
	uint16 ac_policing:1;
	uint16 ac_priority:3;
	uint16 ac_spri:1;
	uint16 ac_mirpmsk:6;
	uint16 ac_mir:1;

	uint16 op_term:1;
	uint16 op_exec:1;
	uint16 op_and:1;
	uint16 op_not:1;
	uint16 op_init:1;
	uint16 format:4;
	uint16 reserved:7;

#else
	rtl8366s_smiaclrule rule;

	uint16 ac_mir:1;
	uint16 ac_mirpmsk:6;
	uint16 ac_spri:1;
	uint16 ac_priority:3;
	uint16 ac_policing:1;
	uint16 ac_meteridx:4;

	uint16 reserved:7;
	uint16 format:4;
	uint16 op_init:1;
	uint16 op_not:1;
	uint16 op_and:1;
	uint16 op_exec:1;
	uint16 op_term:1;

#endif

}rtl8366s_smiacltable;



typedef union{

	struct mac_st				mac;	
	struct ipv4_st				ipv4;
	struct ipv4_icmp_st		ipv4icmp;
	struct ipv4_igmp_st		ipv4igmp;
	struct ipv4_tcp_st			ipv4tcp;
	struct ipv4_udp_st			ipv4udp;

	struct ipv6_sip_st			ipv6sip;
	struct ipv6_dip_st			ipv6dip;
	struct ipv6_ext_st			ipv6ext;
	struct ipv6_tcp_st			ipv6tcp;
	struct ipv6_udp_st			ipv6udp;
	struct ipv6_icmp_st		ipv6icmp;

}rtl8366s_aclrule;	


typedef struct  ACLTABLE{
	
	rtl8366s_aclrule rule;

	uint16 ac_meteridx:4;
	uint16 ac_policing:1;
	uint16 ac_priority:3;
	uint16 ac_spri:1;
	uint16 ac_mirpmsk:6;
	uint16 ac_mir:1;

	uint16 op_term:1;
	uint16 op_exec:1;
	uint16 op_and:1;
	uint16 op_not:1;
	uint16 op_init:1;
	uint16 format:4;
	uint16 reserved:7;

}rtl8366s_acltable;

struct svlan_vid_prio_st{
#ifdef _LITTLE_ENDIAN
	uint16 svid:12;
	uint16 priority:3;
	uint16 reserved:1;
#else
	uint16 reserved:1;
	uint16 priority:3;
	uint16 svid:12;
#endif
};	
	

typedef struct  SVLANCONFIG{

#ifdef _LITTLE_ENDIAN
	uint16 vs_en:1;
	uint16 vs_sprisel:1;
	uint16 vs_stag:1;
	uint16 vs_port:6;
	uint16 reserved1:7;

	uint16 vs_type;
	struct svlan_vid_prio_st vs_idp[4];

	uint16 vs_idx0:2;
	uint16 vs_idx1:2;
	uint16 vs_idx2:2;
	uint16 vs_idx3:2;
	uint16 vs_idx4:2;
	uint16 vs_idx5:2;
	uint16 reserved2:4;


#else
	uint16 reserved1:7;
	uint16 vs_port:6;
	uint16 vs_stag:1;
	uint16 vs_sprisel:1;
	uint16 vs_en:1;

	uint16 vs_type;
	struct svlan_vid_prio_st vs_idp[4];


	uint16 reserved2:4;
	uint16 vs_idx5:2;
	uint16 vs_idx4:2;
	uint16 vs_idx3:2;
	uint16 vs_idx2:2;
	uint16 vs_idx1:2;
	uint16 vs_idx0:2;
#endif

}rtl8366s_svlanconfig;

typedef struct  PROTOCOLGDATACONFIG{

#ifdef _LITTLE_ENDIAN
	uint16 frameType:2;
	uint16 valid:1;
	uint16 reserved:13;
#else
	uint16 reserved:13;
	uint16 valid:1;
	uint16 frameType:2;
#endif
	uint16 value;

}rtl8366s_protocolgdatacfg;


typedef struct  PROTOCLVLANCONFIG{

#ifdef _LITTLE_ENDIAN
	uint16 vidx:4;
	uint16 prio:3;
	uint16 valid:1;
	uint16 reserved:8;
#else
	uint16 reserved:8;
	uint16 valid:1;
	uint16 prio:3;
	uint16 vidx:4;
#endif
	
}rtl8366s_protocolvlancfg;

typedef struct  PORTABILITYCONFIG{
#ifdef _LITTLE_ENDIAN
	uint8 AN:1;
	uint8 H10:1;
	uint8 F10:1;
	uint8 H100:1;
	uint8 F100:1;
	uint8 F1000:1;
	uint8 FC:1;
	uint8 AsyFC:1;
#else
	uint8 AsyFC:1;
	uint8 FC:1;
	uint8 F1000:1;
	uint8 F100:1;
	uint8 H100:1;
	uint8 F10:1;
	uint8 H10:1;
	uint8 AN:1;
#endif
}rtl8366s_portabilitycfg;


enum RTL8366S_MIBCOUNTER{

	IfInOctets = 0,
	EtherStatsOctets,
	EtherStatsUnderSizePkts,
	EtherFregament,
	EtherStatsPkts64Octets,
	EtherStatsPkts65to127Octets,
	EtherStatsPkts128to255Octets,
	EtherStatsPkts256to511Octets,
	EtherStatsPkts512to1023Octets,
	EtherStatsPkts1024to1518Octets,
	EtherOversizeStats,
	EtherStatsJabbers,
	IfInUcastPkts,
	EtherStatsMulticastPkts,
	EtherStatsBroadcastPkts,
	EtherStatsDropEvents,
	Dot3StatsFCSErrors,
	Dot3StatsSymbolErrors,
	Dot3InPauseFrames,
	Dot3ControlInUnknownOpcodes,
	IfOutOctets,
	Dot3StatsSingleCollisionFrames,
	Dot3StatMultipleCollisionFrames,
	Dot3sDeferredTransmissions,
	Dot3StatsLateCollisions,
	EtherStatsCollisions,
	Dot3StatsExcessiveCollisions,
	Dot3OutPauseFrames,
	Dot1dBasePortDelayExceededDiscards,
	Dot1dTpPortInDiscards,
	IfOutUcastPkts,
	IfOutMulticastPkts,
	IfOutBroadcastPkts,
	/*Device only */	
	Dot1dTpLearnEntryDiscardFlag,
	RTL8366S_MIBS_NUMBER,
	
};	

enum RTL8366S_LEDCONF{

	LEDCONF_LEDOFF=0, 		
	LEDCONF_DUPCOL,		
	LEDCONF_LINK_ACT,		
	LEDCONF_SPD1000,		
	LEDCONF_SPD100,		
	LEDCONF_SPD10,			
	LEDCONF_SPD1000ACT,	
	LEDCONF_SPD100ACT,	
	LEDCONF_SPD10ACT,		
	LEDCONF_SPD10010ACT,  
	LEDCONF_FIBER,			
	LEDCONF_FAULT,			
	LEDCONF_LINKRX,		
	LEDCONF_LINKTX,		
	LEDCONF_MASTER,		
	LEDCONF_LEDFORCE,		
};


enum RTL8366S_LEDBLINKRATE{

	LEDBLINKRATE_43MS=0, 		
	LEDBLINKRATE_84MS,		
	LEDBLINKRATE_120MS,
	LEDBLINKRATE_170MS,
	LEDBLINKRATE_340MS,
	LEDBLINKRATE_670MS,
	LEDBLINKRATE_MAX,

};

enum RTL8366S_LINKAMP{

	LINKAMP_NORMAL=0, // Modifyed by shatakeyama @ NTT-DATA-ITEC (2008/01/18)
	LINKAMP_25MV,     // Modifyed by shatakeyama @ NTT-DATA-ITEC (2008/01/18)
	LINKAMP_MAX,      // Modifyed by shatakeyama @ NTT-DATA-ITEC (2008/01/18)
	                  // rename [LINK_***] ==> [LINKAMP_***]
};


int32 rtl8366s_setAsicRegBit(uint32 reg, uint32 bit, uint32 value);
int32 rtl8366s_setAsicRegBits(uint32 reg, uint32 bits, uint32 value);

int32 rtl8366s_getAsicReg(uint32 reg, uint32 *val);
int32 rtl8366s_setAsicReg(uint32 reg, uint32 value);

int32 rtl8366s_setAsicAcl(enum PORTID port, uint32 enabled);
int32 rtl8366s_getAsicAcl(enum PORTID port, uint32* enabled);
int32 rtl8366s_setAsicAclUnmatchedPermit(enum PORTID port, uint32 enabled);
int32 rtl8366s_getAsicAclUnmatchedPermit(enum PORTID port, uint32* enabled);
int32 rtl8366s_setAsicAclMeterInterFrameGap(uint32 enabled);
int32 rtl8366s_getAsicAclMeterInterFrameGap(uint32* enabled);

int32 rtl8366s_setAsicAclMeter(uint32 meterIdx,uint32 aclmRate, uint32 aclmTole);
int32 rtl8366s_getAsicAclMeter(uint32 meterIdx,uint32* aclmRate, uint32* aclmTole);


int32 rtl8366s_setAsicAclStartEnd(enum PORTID port, uint32 aclStart, uint32 aclEnd);
int32 rtl8366s_getAsicAclStartEnd(enum PORTID port, uint32 *aclStart, uint32 *aclEnd);
int32 rtl8366s_setAsicAclRule( uint32 index, rtl8366s_acltable *aclTable);
int32 rtl8366s_getAsicAclRule( uint32 index, rtl8366s_acltable *aclTable);
int32 rtl8366s_setAsicSmiAclTable( uint32 index, rtl8366s_smiacltable *smiaclTable);

int32 rtl8366s_setAsicRma(uint32 bit, uint32 enabled);
int32 rtl8366s_getAsicRma(uint32 bit, uint32* enabled);
int32 rtl8366s_setAsicRmaUserDefinedAddress(uint32 index, ether_addr_t mac, uint32 mask);
int32 rtl8366s_getAsicRmaUserDefinedAddress(uint32 index, ether_addr_t *mac, uint32 *mask);


int32 rtl8366s_setAsicVlan4kTbUsage(uint32 enabled);
int32 rtl8366s_getAsicVlan4kTbUsage(uint32* enabled);

int32 rtl8366s_setAsicVlanMemberConfig(uint32 index,rtl8366s_vlanconfig *vlanmconf );
int32 rtl8366s_getAsicVlanMemberConfig(uint32 index,rtl8366s_vlanconfig *vlanmconf );
int32 rtl8366s_setAsicVlan(uint32 enabled);
int32 rtl8366s_getAsicVlan(uint32* enabled);
int32 rtl8366s_setAsicVlan4kEntry(rtl8366s_vlan4kentry vlan4kEntry);
int32 rtl8366s_getAsicVlan4kEntry(rtl8366s_vlan4kentry *vlan4kEntry);
int32 rtl8366s_setAsicSpanningTreeStatus(enum PORTID port, uint32 fid, enum SPTSTATE state);
int32 rtl8366s_getAsicSpanningTreeStatus(enum PORTID port, uint32 fid, enum SPTSTATE* state);
int32 rtl8366s_setAsicVlanAcceptTaggedOnly(enum PORTID port, uint32 enabled);
int32 rtl8366s_getAsicVlanAcceptTaggedOnly(enum PORTID port, uint32* enabled);
int32 rtl8366s_setAsicVlanDropTaggedPackets(enum PORTID port, uint32 enabled);
int32 rtl8366s_getAsicVlanDropTaggedPackets(enum PORTID port, uint32* enabled);
int32 rtl8366s_setAsicVlanIngressFiltering(enum PORTID port, uint32 enabled);
int32 rtl8366s_getAsicVlanIngressFiltering(enum PORTID port, uint32* enabled);
int32 rtl8366s_setAsicVlanPortBasedVID(enum PORTID port, uint32 index);
int32 rtl8366s_getAsicVlanPortBasedVID(enum PORTID port, uint32* index);
int32 rtl8366s_setAsicVlanProtocolBasedGroupData(uint32 index, rtl8366s_protocolgdatacfg pbcfg);
int32 rtl8366s_getAsicVlanProtocolBasedGroupData(uint32 index, rtl8366s_protocolgdatacfg* pbcfg);
int32 rtl8366s_setAsicVlanProtocolAndPortBasedCfg(enum PORTID port, uint32 index, rtl8366s_protocolvlancfg ppbcfg);
int32 rtl8366s_getAsicVlanProtocolAndPortBasedCfg(enum PORTID port, uint32 index, rtl8366s_protocolvlancfg* ppbcfg);


int32 rtl8366s_setAsicL2LookupTb(uint32 entry, rtl8366s_l2table *l2Table, uint8 bIPMulti);
int32 rtl8366s_getAsicL2LookupTb(uint32 entry, rtl8366s_l2table *l2Table, uint8 bIPMulti);
int32 rtl8366s_setAsicL2CamTb(uint32 entry, rtl8366s_camtable *camTable);
int32 rtl8366s_getAsicL2CamTb(uint32 entry, rtl8366s_camtable *camTable);
int32 rtl8366s_setAsicL2IpMulticastLookup(uint32 enabled);
int32 rtl8366s_getAsicL2IpMulticastLookup(uint32* enabled);
int32 rtl8366s_setAsicL2CamTbUsage(uint32 disabled);
int32 rtl8366s_getAsicL2CamTbUsage(uint32 *disabled);


int32 rtl8366s_setAsicSvlanProtocolType(uint32 protocolType);
int32 rtl8366s_getAsicSvlanProtocolType(uint32* protocolType);
int32 rtl8366s_setAsicSvlanUplinkPortMask(uint32 portMask);
int32 rtl8366s_getAsicSvlanUplinkPortMask(uint32* portMask);
int32 rtl8366s_setAsicSvlanTag(uint32 enabled);
int32 rtl8366s_getAsicSvlanTag(uint32* enabled);
int32 rtl8366s_setAsicSvlan(uint32 enabled);
int32 rtl8366s_getAsicSvlan(uint32* enabled);
int32 rtl8366s_setAsicSvlanPrioDecision(uint32 vsPrio);
int32 rtl8366s_getAsicSvlanPrioDecision(uint32* vsPrio);
int32 rtl8366s_setAsicSvlanVidPriority(uint32 index,struct svlan_vid_prio_st stag);
int32 rtl8366s_getAsicSvlanVidPriority(uint32 index,struct svlan_vid_prio_st* stag);
int32 rtl8366s_setAsicSvlanVsidx(enum PORTID port, uint32 vsidx);
int32 rtl8366s_getAsicSvlanVsidx(enum PORTID port, uint32* vsidx);


int32 rtl8366s_setAsicCpuPortMask(enum PORTID port, uint32 enabled);
int32 rtl8366s_getAsicCpuPortMask(enum PORTID port, uint32* enabled);
int32 rtl8366s_setAsicCpuDropUnda(uint32 enable);
int32 rtl8366s_getAsicCpuDropUnda(uint32* enable);
int32 rtl8366s_setAsicCpuDisableInsTag(uint32 enable);
int32 rtl8366s_getAsicCpuDisableInsTag(uint32* enable);


int32 rtl8366s_setAsicLinkAggregationMode(uint32 mode);
int32 rtl8366s_getAsicLinkAggregationMode(uint32 *mode);
int32 rtl8366s_setAsicLinkAggregationHashAlgorithm(uint32 algorithm);
int32 rtl8366s_getAsicLinkAggregationHashAlgorithm(uint32 *algorithm);
int32 rtl8366s_setAsicLinkAggregationPortMask(uint32 portmask);
int32 rtl8366s_getAsicLinkAggregationPortMask(uint32* portmask);
int32 rtl8366s_setAsicLinkAggregationHashTable(uint32 hashval,enum PORTID port);
int32 rtl8366s_getAsicLinkAggregationHashTable(uint32 hashval,enum PORTID* port);
int32 rtl8366s_setAsicLinkAggregationFlowControl(uint32 fcport);
int32 rtl8366s_getAsicLinkAggregationFlowControl(uint32* fcport);
int32 rtl8366s_getAsicLinkAggregationQueueEmpty(uint32* qeport);

int32 rtl8366s_setAsicStormFiltering(enum PORTID port, uint32 bcstorm, uint32 mcstorm, uint32 undastorm);
int32 rtl8366s_getAsicStormFiltering(enum PORTID port, uint32* bcstorm, uint32* mcstorm, uint32* undastorm);
int32 rtl8366s_setAsicStormFilteringMeter(uint32 period, uint32 counter );
int32 rtl8366s_getAsicStormFilteringMeter(uint32* period, uint32* counter );

int32 rtl8366s_setAsicPortMirroring(enum PORTID mirrored, enum PORTID monitor);
int32 rtl8366s_getAsicPortMirroring(uint32 *mirrored, uint32 *monitor);
int32 rtl8366s_setAsicMirroredPortRxMirror(uint32 enabled);
int32 rtl8366s_getAsicMirroredPortRxMirror(uint32* enabled);
int32 rtl8366s_setAsicMirroredPortTxMirror(uint32 enabled);
int32 rtl8366s_getAsicMirroredPortTxMirror(uint32* enabled);
int32 rtl8366s_setAsicMinitorPortIsolation(uint32 enabled);
int32 rtl8366s_getAsicMinitorPortIsolation(uint32* enabled);

int32 rtl8366s_setAsicPortDisable(uint32 mask);
int32 rtl8366s_getAsicPortDisable(uint32* mask);
int32 rtl8366s_setAsicPortAbility(enum PORTID port,uint32 ability);
int32 rtl8366s_getAsicPortAbility(enum PORTID port,uint32* ability);
int32 rtl8366s_setAsicPortJamMode(uint32 mode);
int32 rtl8366s_getAsicPortJamMode(uint32* mode);
int32 rtl8366s_setAsicMaxLengthInRx(uint32 maxLength);
int32 rtl8366s_getAsicMaxLengthInRx(uint32* maxLength);
int32 rtl8366s_setAsicPortLearnDisable(uint32 mask);
int32 rtl8366s_getAsicPortLearnDisable(uint32* mask);
int32 rtl8366s_setAsicDisableLearn(uint32 enable);
int32 rtl8366s_getAsicDisableLearn(uint32* enable);
int32 rtl8366s_setAsicPortAge(uint32 mask);
int32 rtl8366s_getAsicPortAge(uint32* mask);
int32 rtl8366s_setAsicFastAge(uint32 enable);
int32 rtl8366s_getAsicFastAge(uint32* enable);
int32 rtl8366s_setAsicDropUnknownDa(uint32 enable);
int32 rtl8366s_getAsicDropUnknownDa(uint32* enable);
int32 rtl8366s_setAsicDropUnknownSa(uint32 enable);
int32 rtl8366s_getAsicDropUnknownSa(uint32 *enable);
int32 rtl8366s_setAsicDropUnmatchedSa(uint32 enable);
int32 rtl8366s_getAsicDropUnmatchedSa(uint32* enable);

int32 rtl8366s_setAsicInterruptPolarity(uint32 polarity);
int32 rtl8366s_getAsicInterruptPolarity(uint32* polarity);
int32 rtl8366s_setAsicInterruptMask(uint32 mask);
int32 rtl8366s_getAsicInterruptMask(uint32* mask);
int32 rtl8366s_getAsicInterruptStatus(uint32* mask);

int32 rtl8366s_setAsicMIBsCounterReset(uint32 mask);
int32 rtl8366s_getAsicMIBsCounter(enum PORTID port,enum RTL8366S_MIBCOUNTER mibIdx,uint64* counter);
int32 rtl8366s_getAsicMIBsControl(uint32* mask);

int32 rtl8366s_setAsicPHYRegs( uint32 phyNo, uint32 page, uint32 addr, uint32 data);
int32 rtl8366s_getAsicPHYRegs( uint32 phyNo, uint32 page, uint32 addr, uint32 *data);

int32 rtl8366s_setAsicQosEnable( uint32 enabled);
int32 rtl8366s_getAsicQosEnable( uint32* enabled);

int32 rtl8366s_setAsicLBParameter( uint32 token, uint32 tick, uint32 hiThreshold );
int32 rtl8366s_getAsicLBParameter( uint32* pToken, uint32* pTick, uint32* pHiThreshold );
int32 rtl8366s_setAsicQueueRate( enum PORTID port, enum QUEUEID queueid, uint32 pprTime, uint32 aprBurstSize, uint32 apr );
int32 rtl8366s_getAsicQueueRate( enum PORTID port, enum QUEUEID queueid, uint32* pPprTime, uint32* pAprBurstSize, uint32* pApr );

int32 rtl8366s_setAsicDisableSchedulerAbility( enum PORTID port, uint32 aprDisable, uint32 pprDisable, uint32 wfqDisable );
int32 rtl8366s_getAsicDisableSchedulerAbility( enum PORTID port, uint32* pAprDisable, uint32* pPprDisable, uint32* pWfqDisable);
int32 rtl8366s_setAsicPortIngressBandwidth( enum PORTID port, uint32 bandwidth, uint32 preifg);
int32 rtl8366s_getAsicPortIngressBandwidth( enum PORTID port, uint32* pBandwidth, uint32* pPreifg );
int32 rtl8366s_setAsicPortEgressBandwidth( enum PORTID port, uint32 bandwidth, uint32 preifg );
int32 rtl8366s_getAsicPortEgressBandwidth( enum PORTID port, uint32* pBandwidth, uint32* preifg );
int32 rtl8366s_setAsicQueueWeight( enum PORTID port, enum QUEUEID queueid, enum QUEUETYPE queueType, uint32 weight );
int32 rtl8366s_getAsicQueueWeight( enum PORTID port, enum QUEUEID queueid, enum QUEUETYPE *pQueueType, uint32 *pWeight );
int32 rtl8366s_setAsicOutputQueueNumber( enum PORTID port, enum QUEUENUM qnum );
int32 rtl8366s_getAsicOutputQueueNumber( enum PORTID port, enum QUEUENUM *qnum );

int32 rtl8366s_setAsicDot1pRemarkingAbility(uint32 isEnable);
int32 rtl8366s_getAsicDot1pRemarkingAbility(uint32* isEnable);
int32 rtl8366s_setAsicDot1pRemarkingParameter( enum PRIORITYVALUE priority, enum PRIORITYVALUE newpriority );
int32 rtl8366s_getAsicDot1pRemarkingParameter( enum PRIORITYVALUE priority, enum PRIORITYVALUE *pNewpriority );
int32 rtl8366s_setAsicDscpRemarkingAbility(uint32 isEnable);
int32 rtl8366s_getAsicDscpRemarkingAbility( uint32* isEnable);
int32 rtl8366s_setAsicDscpRemarkingParameter( enum PRIORITYVALUE priority, uint32 newdscp );
int32 rtl8366s_getAsicDscpRemarkingParameter( enum PRIORITYVALUE priority, uint32* pNewdscp );

int32 rtl8366s_setAsicPriorityDecision( uint32 portpri, uint32 dot1qpri, uint32 dscppri, uint32 aclpri);
int32 rtl8366s_getAsicPriorityDecision( uint32* pPortpri, uint32* pDot1qpri, uint32* pDscppri, uint32* pAclpri);
int32 rtl8366s_setAsicPortPriority( enum PORTID port, enum PRIORITYVALUE priority );
int32 rtl8366s_getAsicPortPriority( enum PORTID port, enum PRIORITYVALUE *pPriority );
int32 rtl8366s_setAsicDot1qAbsolutelyPriority( enum PRIORITYVALUE srcpriority, enum PRIORITYVALUE priority );
int32 rtl8366s_getAsicDot1qAbsolutelyPriority( enum PRIORITYVALUE srcpriority, enum PRIORITYVALUE *pPriority );
int32 rtl8366s_setAsicDscpPriority( uint32 dscp, enum PRIORITYVALUE priority );
int32 rtl8366s_getAsicDscpPriority( uint32 dscp, enum PRIORITYVALUE *pPriority );
int32 rtl8366s_setAsicPriorityToQIDMappingTable( enum QUEUENUM qnum, enum PRIORITYVALUE priority, enum QUEUEID qid );
int32 rtl8366s_getAsicPriorityToQIDMappingTable( enum QUEUENUM qnum, enum PRIORITYVALUE priority, enum QUEUEID* pQid );

int32 rtl8366s_setAsicSystemBasedFlowControlRegister(uint32 sharedON, uint32 sharedOFF, uint32 drop);
int32 rtl8366s_getAsicSystemBasedFlowControlRegister(uint32 *sharedON, uint32 *sharedOFF, uint32 *drop);
int32 rtl8366s_setAsicPortBasedFlowControlRegister(enum PORTID port, uint32 fcON, uint32 fcOFF);
int32 rtl8366s_getAsicPortBasedFlowControlRegister(enum PORTID port, uint32 *fcON, uint32 *fcOFF);
int32 rtl8366s_setAsicQueueDescriptorBasedFlowControlRegister(enum PORTID port, enum QUEUEID queue, uint32 fcON, uint32 fcOFF);
int32 rtl8366s_getAsicQueueDescriptorBasedFlowControlRegister(enum PORTID port, enum QUEUEID queue, uint32 *fcON, uint32 *fcOFF);
int32 rtl8366s_setAsicQueuePacketBasedFlowControlRegister(enum PORTID port, enum QUEUEID queue, uint32 fcON, uint32 fcOFF);
int32 rtl8366s_getAsicQueuePacketBasedFlowControlRegister(enum PORTID port, enum QUEUEID queue, uint32 *fcON, uint32 *fcOFF);
int32 rtl8366s_setAsicPerQueuePhysicalLengthGapRegister(uint32 gap);
int32 rtl8366s_getAsicPerQueuePhysicalLengthGapRegister(uint32 *gap);
int32 rtl8366s_setAsicQueueFlowControlConfigureRegister(enum PORTID port, enum QUEUEID queue, uint32 isEnable);
int32 rtl8366s_getAsicQueueFlowControlConfigureRegister(enum PORTID port, enum QUEUEID queue, uint32 *isEnable);
int32 rtl8366s_setAsicPacketUsedPagesFlowControlRegister(uint32 fcON, uint32 isEnable);
int32 rtl8366s_getAsicPacketUsedPagesFlowControlRegister(uint32 *fcON, uint32 *isEnable);

int32 rtl8366s_setAsic1xPBEnConfig(uint32 data);
int32 rtl8366s_getAsic1xPBEnConfig(uint32 *data);
int32 rtl8366s_setAsic1xPBAuthConfig(uint32 data);
int32 rtl8366s_getAsic1xPBAuthConfig(uint32 *data);
int32 rtl8366s_setAsic1xPBOpdirConfig(uint32 data);
int32 rtl8366s_getAsic1xPBOpdirConfig(uint32 *data);
int32 rtl8366s_setAsic1xMBEnConfig(uint32 data);
int32 rtl8366s_getAsic1xMBEnConfig(uint32 *data);
int32 rtl8366s_setAsic1xMBOpdirConfig(uint32 data);
int32 rtl8366s_getAsic1xMBOpdirConfig(uint32 *data);
int32 rtl8366s_setAsic1xProcConfig(uint32 data);
int32 rtl8366s_getAsic1xProcConfig(uint32 *data);
int32 rtl8366s_setAsicGVIndexConfig(uint32 data);
int32 rtl8366s_getAsicGVIndexConfig(uint32 *data);
int32 rtl8366s_setAsicGVTalkConfig(uint32 data);
int32 rtl8366s_getAsicGVTALKConfig(uint32 *data);

int32 rtl8366s_setAsicLedIndicateInfoConfig(uint32 ledNo, enum RTL8366S_LEDCONF config);
int32 rtl8366s_getAsicLedIndicateInfoConfig(uint32 ledNo, enum RTL8366S_LEDCONF* config);
int32 rtl8366s_setAsicForceLeds(uint32 ledG0Msk, uint32 ledG1Msk, uint32 ledG2Msk, uint32 ledG3Msk);
int32 rtl8366s_setAsicLedBlinkRate(enum RTL8366S_LEDBLINKRATE blinkRate);
int32 rtl8366s_getAsicLedBlinkRate(enum RTL8366S_LEDBLINKRATE* blinkRate);


int32 rtl8366s_setAsicMacForceLink(enum PORTID port,enum MACLINKMODE force,enum PORTLINKSPEED speed,enum PORTLINKDUPLEXMODE duplex,uint32 link,uint32 txPause,uint32 rxPause);
int32 rtl8366s_getAsicPortLinkState(enum PORTID port, enum PORTLINKSPEED *speed, enum PORTLINKDUPLEXMODE *duplex,uint32 *link,uint32 *txPause,uint32 *rxPause, uint32 *nWay);


int32 rtl8366s_setAsicIpMulticastVlanLeaky(enum PORTID port, uint32 enabled );
int32 rtl8366s_getAsicIpMulticastVlanLeaky(enum PORTID port, uint32* enabled );

int32 rtl8366s_setAsicKeepCtagFormat(enum PORTID port, uint32 enabled);
int32 rtl8366s_getAsicKeepCtagFormat(enum PORTID port, uint32* enabled);


 
#endif /*#ifndef _RTL8366S_ASICDRV_H_*/


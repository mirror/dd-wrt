#ifndef _RTL8306SDM_H_
#define _RTL8306SDM_H_


#define RTL8306S(x)		(((x) == 0 || (x) == 2)?TRUE:FALSE)
#define RTL8306SD(x)		(((x) == 1)?TRUE:FALSE)
#define RTL8306SDM(x)	(((x) == 3)?TRUE:FALSE)

#define RTL8306_S 0
#define RTL8306_SD 1
#define RTL8306_SDM 2
#define RTL8306_CHIPID 0X5988
#define RTL8306_VERNUM 0X0


#define RTL8306_PHY_NUMBER	7
#define RTL8306_REGSPERPAGE  32
#define RTL8306_REGSPERPHY	68
#define RTL8306_REG_NUMBER  ((RTL8306_REGSPERPHY)*(RTL8306_PHY_NUMBER))
#define RTL8306_PAGE_NUMBER 4
#define RTL8306_REGPAGE0		0x0
#define RTL8306_REGPAGE1		0x1
#define RTL8306_REGPAGE2		0x2
#define RTL8306_REGPAGE3		0x3
#define RTL8306_PORT_NUMBER 6
#define RTL8306_PORT0 		0x0
#define RTL8306_PORT1 		0x1
#define RTL8306_PORT2 		0x2
#define RTL8306_PORT3 		0x3
#define RTL8306_PORT4 		0x4
#define RTL8306_PORT5 		0x5
#define RTL8306_NOCPUPORT 7

#define RTL8306_ETHER_AUTO_100FULL	0x01
#define RTL8306_ETHER_AUTO_100HALF	0x02
#define RTL8306_ETHER_AUTO_10FULL	0x03
#define RTL8306_ETHER_AUTO_10HALF	0x04
#define RTL8306_IDLE_TIMEOUT			10
#define RTL8306_ETHER_SPEED_100 100
#define RTL8306_ETHER_SPEED_10 10



/*ACL Packet processing method*/
#define RTL8306_ACT_DROP 	0x0      /*drop the packet		*/
#define RTL8306_ACT_PERMIT	0x1	    /*permit the packet		*/
#define RTL8306_ACT_TRAPCPU	0x2	    /*trap the packet to cpu*/		
#define RTL8306_ACT_MIRROR	0x3      /*mirror the packet   	*/
#define RTL8306_ACT_FLOOD	0x4	    /*flood the packet	*/	

/* PHY control registers 
*/
#define RTL8306_PHY0_CONTROL							0x0000               		 
#define RTL8306_PHY0_STATUS         						0x0001                  	 
#define RTL8306_PHY0_IDENTIFIER_1						0x0002                 
#define RTL8306_PHY0_IDENTIFIER_2      					0x0003               
#define RTL8306_PHY0_AUTONEGO_ADVERTISEMENT			0x0004
#define RTL8306_PHY0_AUTONEGO_LINK_PARTNER_ABILITY	0x0005
#define RTL8306_PHY0_AUTONEGO_EXPANSION				0x0006
#define RTL8306_PHY0_INTERNAL_TEST						0x0007

#define RTL8306_PHY1_CONTROL							0x0100                		 
#define RTL8306_PHY1_STATUS               					0x0101            	 
#define RTL8306_PHY1_IDENTIFIER_1      					0x0102               
#define RTL8306_PHY1_IDENTIFIER_2      					0x0103               
#define RTL8306_PHY1_AUTONEGO_ADVERTISEMENT			0x0104
#define RTL8306_PHY1_AUTONEGO_LINK_PARTNER_ABILITY	0x0105
#define RTL8306_PHY1_AUTONEGO_EXPANSION				0x0106
#define RTL8306_PHY1_INTERNAL_TEST						0x0107

#define RTL8306_PHY2_CONTROL							0x0200                		 
#define RTL8306_PHY2_STATUS                           	 			0x0201
#define RTL8306_PHY2_IDENTIFIER_1                     			0x0202
#define RTL8306_PHY2_IDENTIFIER_2                     			0x0203
#define RTL8306_PHY2_AUTONEGO_ADVERTISEMENT			0x0204
#define RTL8306_PHY2_AUTONEGO_LINK_PARTNER_ABILITY	0x0205
#define RTL8306_PHY2_AUTONEGO_EXPANSION				0x0206
#define RTL8306_PHY2_INTERNAL_TEST						0x0207

#define RTL8306_PHY3_CONTROL                		 			0x0300
#define RTL8306_PHY3_STATUS                           	 			0x0301
#define RTL8306_PHY3_IDENTIFIER_1                     			0x0302
#define RTL8306_PHY3_IDENTIFIER_2                     			0x0303
#define RTL8306_PHY3_AUTONEGO_ADVERTISEMENT			0x0304
#define RTL8306_PHY3_AUTONEGO_LINK_PARTNER_ABILITY	0x0305
#define RTL8306_PHY3_AUTONEGO_EXPANSION				0x0306
#define RTL8306_PHY3_INTERNAL_TEST						0x0307

#define RTL8306_PHY4_CONTROL                		 			0x0400
#define RTL8306_PHY4_STATUS                           	 			0x0401
#define RTL8306_PHY4_IDENTIFIER_1                     			0x0402
#define RTL8306_PHY4_IDENTIFIER_2                     			0x0403
#define RTL8306_PHY4_AUTONEGO_ADVERTISEMENT			0x0404
#define RTL8306_PHY4_AUTONEGO_LINK_PARTNER_ABILITY	0x0405
#define RTL8306_PHY4_AUTONEGO_EXPANSION				0x0406
#define RTL8306_PHY4_INTERNAL_TEST						0x0407

#define RTL8306_PHY5_CONTROL                		 			0x0500
#define RTL8306_PHY5_STATUS                           	 			0x0501
#define RTL8306_PHY5_IDENTIFIER_1                     			0x0502
#define RTL8306_PHY5_IDENTIFIER_2                     			0x0503
#define RTL8306_PHY5_AUTONEGO_ADVERTISEMENT			0x0504
#define RTL8306_PHY5_AUTONEGO_LINK_PARTNER_ABILITY	0x0505

#define RTL8306_PHY6_CONTROL                		 			0x0600
#define RTL8306_PHY6_STATUS                           	 			0x0601
#define RTL8306_PHY6_IDENTIFIER_1                     			0x0602
#define RTL8306_PHY6_IDENTIFIER_2                     			0x0603
#define RTL8306_PHY6_AUTONEGO_ADVERTISEMENT			0x0604
#define RTL8306_PHY6_AUTONEGO_LINK_PARTNER_ABILITY	0x0605

/* PHY control register field definitions 
*/
#define RTL8306_PHY_RESET					(1 << 15)
#define RTL8306_PHY_ENABLE_LOOPBACK		(1 << 14)
#define RTL8306_SPEED_SELECT_100M			(1 << 13)
#define RTL8306_SPEED_SELECT_10M                   0
#define RTL8306_ENABLE_AUTONEGO			(1 << 12)
#define RTL8306_POWER_DOWN					(1 << 11)
#define RTL8306_ISOLATE_PHY					(1 << 10)
#define RTL8306_RESTART_AUTONEGO			(1 << 9)
#define RTL8306_SELECT_FULL_DUPLEX			(1 << 8)
#define RTL8306_SELECT_HALF_DUPLEX			0
/* PHY status register field definitions 
*/
#define RTL8306_STS_CAPABLE_100BASE_T4 				(1 << 15)
#define RTL8306_STS_CAPABLE_100BASE_TX_FD			(1 << 14)
#define RTL8306_STS_CAPABLE_100BASE_TX_HD			(1 << 13)
#define RTL8306_STS_CAPABLE_100BASE_T_FD			(1 << 12)
#define RTL8306_STS_CAPABLE_100BASE_T_HD			(1 << 11)
#define RTL8306_STS_MF_PREAMBLE_SUPPRESSION		(1 << 6)
#define RTL8306_STS_AUTONEGO_COMPLETE				(1 << 5)
#define RTL8306_STS_REMOTE_FAULT                            (1 << 4)
#define RTL8306_STS_CAPABLE_NWAY_AUTONEGO                   (1 << 3)
#define RTL8306_STS_LINK_ESTABLISHED                        (1 << 2)
#define RTL8306_STS_JABBER_DETECTED                         (1 << 1)
#define RTL8306_STS_CAPABLE_EXTENDED                        (1 << 0)
/* PHY identifier 1 
*/
#define RTL8306_OUT_3_18_MASK                               (0xFFFF << 16)
#define RTL8306_OUT_3_18_OFFSET                             16
#define RTL8306_OUT_19_24_MASK                              (0x3F << 10)
#define RTL8306_OUT_19_24_OFFSET                            10
#define RTL8306_MODEL_NUMBER_MASK                           (0x3F << 4)
#define RTL8306_MODEL_NUMBER_OFFSET                         4
#define RTL8306_REVISION_NUMBER_MASK                        0x0F
#define RTL8306_REVISION_NUMBER_OFFSET                      0
/* PHY auto-negotiation advertisement and 
link partner ability registers field definitions
*/
#define RTL8306_NEXT_PAGE_ENABLED                           (1 << 15)
#define RTL8306_ACKNOWLEDGE                                 (1 << 14)
#define RTL8306_REMOTE_FAULT                                (1 << 13)
#define RTL8306_CAPABLE_PAUSE                               (1 << 10)
#define RTL8306_CAPABLE_100BASE_T4                          (1 << 9)
#define RTL8306_CAPABLE_100BASE_TX_FD                       (1 << 8)
#define RTL8306_CAPABLE_100BASE_TX_HD                       (1 << 7)
#define RTL8306_CAPABLE_10BASE_TX_FD                        (1 << 6)
#define RTL8306_CAPABLE_10BASE_TX_HD                        (1 << 5)
#define RTL8306_SELECTOR_MASK                               0x1F
#define RTL8306_SELECTOR_OFFSET                             0


#define RTL8306_IGMP 0
#define RTL8306_MLD  1
#define RTL8306_PORT_RX  0
#define RTL8306_PORT_TX  1
#define RTL8306_QUEUE0	0
#define RTL8306_QUEUE1	1
#define RTL8306_QUEUE2	2
#define RTL8306_QUEUE3	3
#define RTL8306_ACL_PRIO 0
#define RTL8306_DSCP_PRIO 1
#define RTL8306_1QBP_PRIO 2
#define RTL8306_PBP_PRIO 3
#define RTL8306_CPUTAG_PRIO 4
/*
#define RTL8306_TXQ0	0
#define RTL8306_TXQ1	1
#define RTL8306_TXQ2	2
#define RTL8306_TXQ3  3
*/
#define RTL8306_VLAN_ENTRYS  16   /*Vlan entry number*/ 
#define RTL8306_VLAN_IRTAG	0	/*The switch will remove VLAN tags and add new tags */
#define RTL8306_VLAN_RTAG	1	/*The switch will remove VLAN tags */
#define RTL8306_VLAN_ITAG	2	/*The switch will  add new VLANtag */
#define RTL8306_VLAN_UNDOTAG    3  /*Do not insert or remove  VLAN tag */

#define RTL8306_1QTAG_PRIO0		0
#define RTL8306_1QTAG_PRIO1		1
#define RTL8306_1QTAG_PRIO2		2
#define RTL8306_1QTAG_PRIO3		3
#define RTL8306_1QTAG_PRIO4		4
#define RTL8306_1QTAG_PRIO5		5
#define RTL8306_1QTAG_PRIO6 		6
#define RTL8306_1QTAG_PRIO7		7
#define RTL8306_PRIO0		0
#define RTL8306_PRIO1		1
#define RTL8306_PRIO2		2
#define RTL8306_PRIO3		3


#define RTL8306_QOS_SET0			0
#define RTL8306_QOS_SET1 		1
#define RTL8306_DSCP_EF			0
#define RTL8306_DSCP_AFL1		1
#define RTL8306_DSCP_AFM1		2
#define RTL8306_DSCP_AFH1		3
#define RTL8306_DSCP_AFL2		4	
#define RTL8306_DSCP_AFM2		5		
#define RTL8306_DSCP_AFH2		6		
#define RTL8306_DSCP_AFL3		7			
#define RTL8306_DSCP_AFM3		8
#define RTL8306_DSCP_AFH3		9
#define RTL8306_DSCP_AFL4		10	
#define RTL8306_DSCP_AFM4		11		
#define RTL8306_DSCP_AFH4		12
#define RTL8306_DSCP_NC			13
#define RTL8306_DSCP_REG_PRI		14			
#define RTL8306_DSCP_BF			15

#define RTL8306_DSCP_USERA		0
#define RTL8306_DSCP_USERB		1
#define RTL8306_IPADD_A	0
#define RTL8306_IPADD_B	1

#define RTL8306_FCO_SET0			0x0
#define RTL8306_FCO_SET1			0x1
#define RTL8306_FCOFF			0x0
#define RTL8306_FCON				0x1
#define RTL8306_FCO_DSC			0x0
#define RTL8306_FCO_QLEN	 		0x1
#define RTL8306_FCO_FULLTHR		0x0
#define RTL8306_FCO_OVERTHR 		0x1

#define RTL8306_ACL_ENTRYNUM	16
#define RTL8306_ACL_INVALID		0x6
#define RTL8306_ACL_ANYPORT  	0x7
#define RTL8306_ACL_ETHER		0x0
#define RTL8306_ACL_TCP			0x1
#define RTL8306_ACL_UDP			0x2
#define RTL8306_ACL_TCPUDP		0x3


#define RTL8306_MIB_CNT1			0
#define RTL8306_MIB_CNT2			1
#define RTL8306_MIB_CNT3			2
#define RTL8306_MIB_CNT4			3
#define RTL8306_MIB_CNT5			4
#define RTL8306_MIB_RESET		0
#define RTL8306_MIB_START		1
#define RTL8306_MIB_BYTE			0
#define RTL8306_MIB_PKT			1

#define RTL8306_MIR_INVALID		0x6

#define RTL8306_LUT_ENTRY0		0
#define RTL8306_LUT_ENTRY1		1
#define RTL8306_LUT_ENTRY2		2
#define RTL8306_LUT_ENTRY3		3
#define RTL8306_LUT_FULL               -2  /*Four way of the same entry are all written by cpu*/
#define RTL8306_LUT_NOTEXIST     -3
#define RTL8306_LUT_AGEOUT 		0
#define RTL8306_LUT_AGE100		100
#define RTL8306_LUT_AGE200		200
#define RTL8306_LUT_AGE300		300
#define RTL8306_LUT_DYNAMIC		0
#define RTL8306_LUT_STATIC		1
#define RTL8306_LUT_UNAUTH		0
#define RTL8306_LUT_AUTH			1

#define RTL8306_SPAN_DISABLE		0
#define RTL8306_SPAN_BLOCK		1
#define RTL8306_SPAN_LEARN		2
#define RTL8306_SPAN_FORWARD	3

#define RTL8306_PORT_UNAUTH		0
#define RTL8306_PORT_AUTH		1
#define RTL8306_PORT_BOTHDIR		0
#define RTL8306_PORT_INDIR		1
#define RTL8306_MAC_BOTHDIR		0
#define RTL8306_MAC_INDIR			1

#define RTL8306_RESADDRXX		0	/*reserved address 01-80-c2-00-00-xx (exclude 00, 01, 02, 03, 10, 20, 21) */
#define RTL8306_RESADDR21		1	/*reserved address 01-80-c2-00-00-21*/
#define RTL8306_RESADDR20		2	/*reserved address 01-80-c2-00-00-20*/
#define RTL8306_RESADDR10		3	/*reserved address 01-80-c2-00-00-10*/
#define RTL8306_RESADDR03		4	/*reserved address 01-80-c2-00-00-03*/
#define RTL8306_RESADDR02		5	/*reserved address 01-80-c2-00-00-02*/
#define RTL8306_RESADDR00		6     /*reserved address 01-80-c2-00-00-00*/
#define RTL8306_RESADDR01		7	/*reserved address 01-80-c2-00-00-01*/	

#define RTL8306_LED_GROUPA		0
#define RTL8306_LED_GROUPB		1
#define RTL8306_LED_GROUPC		2
#define RTL8306_LED_GROUPD		3
#define RTL8306_LED_OFF			0
#define RTL8306_LED_ON			1
#define RTL8306_LED_ASICCTL		0     /*LED controlled by ASIC*/
#define RTL8306_LED_CPUCTL		1	/*LED controlled by CPU*/
/*
#define RTL8306_BROAD_PKT			0     //Broadcast packet
#define RTL8306_MULTI_PKT			1     //Multicast packet
#define RTL8306_UDA_PKT			2    //Unkown DA Storm packet	
*/
#define RTL8306_INPUTDROP			0   /*Input Drop for all types of packet*/
#define RTL8306_OUTPUTDROP		1   /*Output Drop for unicast packet, but not include unknown DA unicast packet*/	
#define RTL8306_BRO_INPUTDROP		2   /*Broadcast packet is input drop, but unicast packet is also output drop*/		
#define RTL8306_BRO_OUTDROP		3   /*Broadcast packet is output drop, and unicastl packet is also output drop*/	
#define RTL8306_MUL_INPUTDROP		4   /*Multicast  packet is input drop, but unicast packet is also output drop*/		  
#define RTL8306_MUL_OUTDROP		5   /*Multicast packet is output drop, and unicast packet is also output drop*/	
#define RTL8306_UDA_INPUTDROP		6   /*Unkown DA  unicast packet is input drop, but unicast packet is also output drop*/		  
#define RTL8306_UDA_OUTPUTDROP	7   /*Unkown DA  uinicast packet is output drop, and unicastl packet is also output drop*/
#define RTL8306_UDA_DISABLEDROP 	8  /* Disable Unkown DA unicast packet Drop*/

#define RTL8306_UNICASTPKT		0   /*Unicast packet, but not include unknown DA unicast packet*/
#define RTL8306_BROADCASTPKT      1   /*Broadcast packet*/
#define RTL8306_MULTICASTPKT		2   /*Multicast packet*/
#define RTL8306_UDAPKT			3   /*Unknown DA unicast packet*/

#define RTL8306_STM_FILNUM64      0  /*continuous 64 pkts will trigger storm fileter*/
#define RTL8306_STM_FILNUM32      1  /*continuous 32 pkts will trigger storm fileter*/
#define RTL8306_STM_FILNUM16      2  /*continuous 16 pkts will trigger storm fileter*/
#define RTL8306_STM_FILNUM8       3  /*continuous 8 pkts will trigger storm fileter*/
#define RTL8306_STM_FIL800MS      0  /*filter 800ms after trigger storm filter*/
#define RTL8306_STM_FIL400MS      1  /*filter 400ms after trigger storm filter*/
#define RTL8306_STM_FIL200MS      2  /*filter 200ms after trigger storm filter*/
#define RTL8306_STM_FIL100MS      3  /*filter 100ms after trigger storm filter*/

				
#define RTL8306_GET_REG_ADDR(x, page, phy, reg) 	do { (page) = ((x) & 0xFF0000) >> 16; (phy) = ((x) & 0x00FF00) >> 8; (reg) = ((x) & 0x0000FF); } while(0)
/*compute look up table index of a mac addrees, LUT index : MAC[13:15] + MAC[0:5]*/
#define RTL8306_MAC_INDEX(mac, index)			do { index = ((mac[4] & 0x7) << 6) |((mac[5] & 0xFC) >>2) ;} while(0)

typedef struct asicVersionPara_s
{
    uint16 chipid;
    uint8 vernum;
    uint8 series;
    uint8 revision;
} asicVersionPara_t;


int32 rtl8306_setAsicPhyReg(uint32 phyad, uint32 regad, uint32 npage, uint32 value);
int32 rtl8306_getAsicPhyReg(uint32 phyad, uint32 regad, uint32 npage, uint32 *pvalue); 
int32 rtl8306_setAsicPhyRegBit(uint32 phyad, uint32 regad, uint32 bit, uint32 npage,  uint32 value) ;
int32 rtl8306_getAsicPhyRegBit(uint32 phyad, uint32 regad, uint32 bit, uint32 npage,  uint32 * pvalue) ;
int32 rtl8306_setAsicEthernetPHY(uint32 phy, uint32 autoNegotiation, uint32 advCapability, uint32 speed, uint32 fullDuplex) ;
int32 rtl8306_getAsicEthernetPHY(uint32 phy, uint32 *autoNegotiation, uint32 *advCapability, uint32 *speed, uint32 *fullDuplex);
int32 rtl8306_setAsicPort5LinkStatus(uint32 enabled);
int32 rtl8306_getAsicPort5LinkStatus(uint32 *enabled);
int32 rtl8306_setAsicTurboMIIEnable(uint32 enabled);
int32 rtl8306_getAsicPHYLinkStatus(uint32 phy, uint32 *linkUp);
int32 rtl8306_getAsicPHYAutoNegotiationDone(uint32 phy, uint32 *done); 
int32 rtl8306_setAsicPHYLoopback(uint32 phy, uint32 enabled);
int32 rtl8306_getAsicPHYLoopback(uint32 phy, uint32 *enabled);
int32 rtl8306_setAsic25MClockOutput(uint32 enabled);
int32 rtl8306_setAsicVlanEnable(uint32 enabled);
int32 rtl8306_getAsicVlanEnable(uint32 *enabled);
int32 rtl8306_setAsicVlanTagAware(uint32 enabled) ;
int32 rtl8306_getAsicVlanTagAware(uint32 *enabled);
int32 rtl8306_setAsicVlanIngressFilter(uint32 enabled);
int32 rtl8306_getAsicVlanIngressFilter(uint32 *enabled);
int32 rtl8306_setAsicVlanTaggedOnly(uint32 enabled);
int32 rtl8306_getAsicVlanTaggedOnly(uint32 *enabled);
int32 rtl8306_setAsicVlan(uint32 vlanIndex, uint32 vid, uint32 memberPortMask);
int32 rtl8306_getAsicVlan(uint32 vlanIndex, uint32 *vid, uint32 *memberPortMask);
int32 rtl8306_setAsicPortVlanIndex(uint32 port, uint32 vlanIndex) ;
int32 rtl8306_getAsicPortVlanIndex(uint32 port, uint32 *vlanIndex);
int32 rtl8306_setAsicLeakyVlan(uint32 enabled);
int32 rtl8306_getAsicLeakyVlan(uint32 *enabled);
int32 rtl8306_setAsicArpVlan(uint32 enabled);
int32 rtl8306_getAsicArpVlan(uint32 *enabled);
int32 rtl8306_setAsicMulticastVlan(uint32 enabled);
int32 rtl8306_getAsicMulticastVlan(uint32 * enabled);
int32 rtl8306_setAsicMirrorVlan(uint32 enabled);
int32 rtl8306_getAsicMirrorVlan(uint32 * enabled);
int32 rtl8306_setAsicNullVidReplaceVlan(uint32 port, uint32 enabled);
int32 rtl8306_getAsicNullVidReplaceVlan(uint32 port, uint32* enabled);
int32 rtl8306_setAsicVlanTagInsertRemove(uint32 port, uint32 option);
int32 rtl8306_getAsicVlanTagInsertRemove(uint32 port, uint32 *option);
int32 rtl8306_setAsicVlanTrapToCPU(uint32 enabled);
int32 rtl8306_getAsicVlanTrapToCPU(uint32 *enabled);
int32 rtl8306_setAsic1pRemarkingVlan(uint32 port, uint32 enabled);
int32 rtl8306_getAsic1pRemarkingVlan(uint32 port, uint32 *enabled);
int32 rtl8306_setAsic1pRemarkingPriority(uint32 priority, uint32 priority1p);
int32 rtl8306_getAsic1pRemarkingPriority(uint32 priority, uint32 *priority1p);
int32 rtl8306_setAsicCPUPort(uint32 port, uint32 enTag);
int32 rtl8306_getAsicCPUPort(uint32 *port, uint32 *enTag);
int32 rtl8306_setAsicCPUTaggedPktCRCCheck(uint32 enabled);
int32 rtl8306_getAsicCPUTaggedPktCRCCheck(uint32 *enabled);
int32 rtl8306_setAsicIGMPMLDSnooping(uint32 protocol, uint32 enabled);
int32 rtl8306_getAsicIGMPMLDSnooping(uint32 protocol, uint32 *enabled);
int32 rtl8306_setAsicTrapPPPoEPkt(uint32 enabled);
int32 rtl8306_getAsicTrapPPPoEPkt(uint32 *enabled);
int32 rtl8306_setAsicQosPortQueueNum(uint32 num);
int32 rtl8306_getAsicQosPortQueueNum(uint32 *num);
int32 rtl8306_setAsicQosTxQueueWeight(uint32 queue, uint32 weight, uint32 set );
int32 rtl8306_getAsicQosTxQueueWeight(uint32 queue, uint32 *weight, uint32 set);
int32 rtl8306_setAsicQosTxQueueStrictPriority(uint32 queue, uint32 set, uint32 enabled) ;
int32 rtl8306_getAsicQosTxQueueStrictPriority(uint32 queue, uint32 set, uint32 *enabled);
int32 rtl8306_setAsicQosTxQueueLeakyBucket(uint32 queue, uint32 set, uint32 burstsize, uint32 rate) ;
int32 rtl8306_getAsicQosTxQueueLeakyBucket(uint32 queue, uint32 set, uint32 *burstsize, uint32 *rate) ;
int32 rtl8306_setAsicQosPortScheduleMode(uint32 port, uint32 set, uint32 quemask);
int32 rtl8306_getAsicQosPortScheduleMode(uint32 port, uint32 *set, uint32 *quemask);
int32 rtl8306_setAsicQosPortRate(uint32 port, uint32 rate, uint32 direction, uint32 enabled);
int32 rtl8306_getAsicQosPortRate(uint32 port, uint32 *rate, uint32 direction, uint32 *enabled);
int32 rtl8306_setAsicQosRxRateGlobalControl(uint32 hisize, uint32 losize, uint32 preamble);
int32 rtl8306_getAsicQosRxRateGlobalControl(uint32 *hisize, uint32 *losize, uint32 *preamble);
int32 rtl8306_setAsicQosPktPriorityAssign(uint32 type, uint32 level);
int32 rtl8306_getAsicQosPktPriorityAssign(uint32 type, uint32 *level);
int32 rtl8306_setAsicQosPrioritytoQIDMapping(uint32 priority, uint32 qid);
int32 rtl8306_getAsicQosPrioritytoQIDMapping(uint32 priority, uint32 *qid);
int32 rtl8306_setAsicQosPortBasedPriority(uint32 port, uint32 priority);
int32 rtl8306_getAsicQosPortBasedPriority(uint32 port, uint32 *priority);
int32 rtl8306_setAsicQos1QBasedPriority(uint32 port, uint32 priority);
int32 rtl8306_getAsicQos1QBasedPriority(uint32 port, uint32 *priority);
int32 rtl8306_setAsicQos1QtagPriorityto2bitPriority(uint32 tagprio, uint32 prio);
int32 rtl8306_getAsicQos1QtagPriorityto2bitPriority(uint32 tagprio, uint32 *prio);
int32 rtl8306_setAsicQosDSCPBasedPriority(uint32 type, uint32 priority);
int32 rtl8306_getAsicQosDSCPBasedPriority(uint32 type, uint32 *priority);
int32 rtl8306_setAsicQosDSCPUserAssignPriority(uint32 entry, uint32 priority, uint32 enabled);
int32 rtl8306_getAsicQosDSCPUserAssignPriority(uint32 entry, uint32 *priority, uint32 *enabled);
int32 rtl8306_setAsicQosIPAddressPriority(uint32 priority);
int32 rtl8306_getAsicQosIPAddressPriority(uint32 *priority);
int32 rtl8306_setAsicQosIPAddress(uint32 entry, uint32 ip, uint32 mask, uint32 enabled);
int32 rtl8306_getAsicQosIPAddress(uint32 entry, uint32 *ip, uint32 *mask , uint32 *enabled) ;
int32 rtl8306_setAsicQosPriorityEnable(uint32 port, uint32 type, uint32 enabled);
int32 rtl8306_getAsicQosPriorityEnable(uint32 port, uint32 type, uint32 *enabled);
int32 rtl8306_setAsicQosSystemRxFlowControl(uint32 enabled);
int32 rtl8306_getAsicQosSystemRxFlowControl(uint32 *enabled);
int32 rtl8306_setAsicAclEntry(uint32 entryadd, uint32 phyport, uint32 action, uint32 protocol, uint32 data, uint32 priority) ;
int32 rtl8306_getAsicAclEntry(uint32 entryadd, uint32 *phyport, uint32 *action, uint32 *protocol, uint32  *data, uint32 *priority) ;
int32 rtl8306_getAsicMibCounter(uint32 port, uint32 counter, uint32 *value);
int32 rtl8306_setAsicMibCounterReset(uint32 port, uint32 operation);
int32 rtl8306_setAsicMibCounterUnit(uint32 port, uint32 counter, uint32 unit) ;
int32 rtl8306_getAsicMibCounterUnit(uint32 port, uint32 counter, uint32 *unit);
int32 rtl8306_setAsicMirrorPort(uint32 mirport, uint32 rxport, uint32 txport, uint32 enFilter) ;
int32 rtl8306_getAsicMirrorPort(uint32 *mirport, uint32 *rxport, uint32 *txport, uint32 *enFilter) ;
int32 rtl8306_setAsicMirrorMacAddress(uint8 *macAddr, uint32 enabled);
int32 rtl8306_getAsicMirrorMacAddress(uint8 *macAddr, uint32 *enabled);
int32 rtl8306_setAsicLUTUnicastEntry(uint8 *macAddress, uint32 entry, uint32 age, uint32 isStatic, uint32 isAuth, uint32 port) ;
int32 rtl8306_getAsicLUTUnicastEntry(uint8 *macAddress, uint32 entryAddr, uint32 *age, uint32 *isStatic, uint32 *isAuth, uint32 *port) ;
int32 rtl8306_setAsicLUTMulticastEntry(uint8 *macAddress, uint32 entry, uint32 isAuth, uint32 portMask) ;
int32 rtl8306_getAsicLUTMulticastEntry(uint8 *macAddress, uint32 entryAddr, uint32 *isAuth, uint32 *portMask) ;
int32 rtl8306_setAsicLUTLRU(uint32 enabled);
int32 rtl8306_getAsicLUTLRU(uint32 *enabled);
#ifndef RTL_MULTICAST_SNOOPING_TEST
int32  rtl8306_addMuticastMacAddress(uint8 *macAddr,uint32 isAuth, uint32 portMask, uint32 *entryaddr);
int32 rtl8306_deleteMacAddress(uint8 *macAddr, uint32 *entryaddr) ;
#endif

int32 rtl8306_setAsic1dPortState(uint32 port, uint32 state);
int32 rtl8306_getAsic1dPortState(uint32 port, uint32 *state);
int32 rtl8306_setAsicReservedAddressForward(uint32 addr, uint32 action);
int32 rtl8306_getAsicReservedAddressForward(uint32 addr, uint32 *action);
int32 rtl8306_setAsicLedStatus(uint32 port, uint32 group, uint32 status, uint32 enCPUCtl);
int32 rtl8306_getAsicLedStatus(uint32 port, uint32 group, uint32 *status, uint32 *enCPUCtl) ;
int32 rtl8306_setAsicBicolorLedStatus(uint32 port, uint32 greenStatus, uint32 yellowStatus, uint32 enCPUCtl);
int32 rtl8306_getAsicBicolorLedStatus(uint32 port, uint32 *greenStatus, uint32 *yellowStatus, uint32 *enCPUCtl) ;
int32 rtl8306_setAsic1xPortBased(uint32 port, uint32 enabled, uint32 isAuth, uint32 direction) ;
int32 rtl8306_getAsic1xPortBased(uint32 port, uint32 *enabled, uint32 *isAuth, uint32 *direction) ;
int32 rtl8306_setAsic1xMacBased(uint32 port, uint32 enabled, uint32 direction);
int32 rtl8306_getAsic1xMacBased(uint32 port, uint32 *enabled, uint32 *direction) ;
int32 rtl8306_setAsic1xUnauthPktAct(uint32 action) ;
int32 rtl8306_getAsic1xUnauthPktAct(uint32 *action);
int32 rtl8306_setAsicStormFilterEnable(uint32 type, uint32 enabled);
int32 rtl8306_getAsicStormFilterEnable(uint32 type, uint32 *enabled);
int32 rtl8306_setAsicStormFilter(uint32 trigNum, uint32 filTime, uint32 enOtherClr, uint32 enStmInt);
int32 rtl8306_getAsicStormFilter(uint32* trigNum, uint32* filTime, uint32* enOtherClr, uint32* enStmInt);
int32 rtl8306_setAsicInputOutputDrop(uint32 type) ;
int32 rtl8306_getAsicInputOutputDrop(uint32 pkttype, uint32 *drptype) ;
int32 rtl8306_setAsicInterrupt(uint32 enInt, uint32 intmask);
int32 rtl8306_getAsicInterrupt(uint32 *enInt, uint32 *intmask);
int32 rtl8306_getAsicInterruptFlag(uint32 *intmask);
int32 rtl8306_asicSoftReset(void);
int32 rtl8306_setAsicQosQueueFlowControlThr(uint32 queue, uint32 type, uint32 onoff, uint32 set, uint32 value, uint32 enabled);
int32 rtl8306_getAsicQosQueueFlowControlThr(uint32 queue, uint32 type, uint32 onoff, uint32 set, uint32* value, uint32* enabled) ;
int32 rtl8306_setAsicQosPortFlowControlThr(uint32 port, uint32 onthr, uint32 offthr, uint32 direction );
int32 rtl8306_getAsicQosPortFLowControlThr(uint32 port, uint32 *onthr, uint32 *offthr, uint32 direction);
int32 rtl8306_setAsicQosPortFlowControlMode(uint32 port, uint32 set);
int32 rtl8306_getAsicQosPortFlowControlMode(uint32 port , uint32 *set);
int32 rtl8306_getVendorID(uint32 *id);
int32 rtl8306_getAsicVersionInfo(asicVersionPara_t *pAsicVer);
int32 rtl8306_setAsicPortLearningAbility(uint32 port, uint32 enabled) ;

#endif /*#ifndef _RTL8306SDM_H_*/

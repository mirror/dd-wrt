#ifndef _RTL8306_DRIVER_1_H_
#define _RTL8306_DRIVER_1_H_

/*
if you need backup asic info in cpu memroy in order to 
accellerate CPU process, please define this macro. If 
support IGMP snooping, this macro is required
*/

#define   RTL8306_TBLBAK  


#define RTL8306_QOS_NOQ          0X0
#define RTL8306_QOS_ONLYQ2     0x4
#define RTL8306_QOS_ONLYQ3     0x8
#define RTL8306_QOS_Q3Q2         0xC


/*Vlan*/	
#define RTL8306_VLAN_FULL			-10		/*Vlan number is full*/
#define RTL8306_VLAN_VIDEXISTED	-11		
#define RTL8306_VLAN_VIDNOTEXISTS	-12
#ifdef RTL8306_TBLBAK
    typedef struct rtl8306_vlanConfigBakPara_s {
          uint8 enVlan;
          uint8 enArpVlan;
          uint8 enLeakVlan;
          uint8 enVlanTagOnly;
          uint8 enIngress;
          uint8 enTagAware;
          uint8 enIPMleaky;
          uint8 enMirLeaky;
    } rtl8306_vlanConfigBakPara_t;

    typedef struct rtl8306_vlanConfigPerPortBakPara_s {
         uint8 vlantagInserRm;
         uint8 en1PRemark;
         uint8 enNulPvidRep;         
    } rtl8306_vlanConfigPerPortBakPara_t;

    typedef struct  rtl8306_vlanTblBakPara_s {
        uint16 vid;
        uint8 memberPortMask;        
    } rtl8306_vlanTblBakPara_t;

    typedef struct rtl8306_aclTblBakPara_s {
        uint8 phy_port;
        uint8 proto;
        uint16 data;
        uint8 action;
        uint8 pri;        

    } rtl8306_aclTblBakPara_t;

    typedef struct rtl8306_mirConfigBakPara_s {
        uint8 mirPort;
        uint8 mirRxPortMask;
        uint8 mirTxPortMask;
        uint8 enMirself;
        uint8 enMirMac;
        uint8 mir_mac[6];
    } rtl8306_mirConfigBakPara_t;
    

    typedef struct rtl8306_ConfigBakPara_s  {

        rtl8306_vlanConfigBakPara_t vlanConfig;                    /*VLAN global configuration*/
        rtl8306_vlanConfigPerPortBakPara_t vlanConfig_perport[6];   /*VLAN per-port configuration*/
        rtl8306_vlanTblBakPara_t vlanTable[16]; /*It backups VLAN table in cpu memory*/
        uint8 vlanPvidIdx[6];   /*per-port PVID index*/                  
        uint8 En1PremarkPortMask; /*Enable/disable 802.1P remarking  port mask */
        uint8 dot1PremarkCtl[4]; /*802.1p remarking table*/
        uint8 dot1DportCtl[6]; /*Spanning tree port state*/
        rtl8306_aclTblBakPara_t aclTbl[16];         /*ACL table*/
        rtl8306_mirConfigBakPara_t mir; /*mirror configuration*/                                                                         
     } rtl8306_ConfigBakPara_t;

extern rtl8306_ConfigBakPara_t rtl8306_TblBak;    /*switch setting*/

#endif

int32 rtl8306_setEthernetPHY(uint32 phy, uint32 autoNegotiation, uint32 advCapability, uint32 speed, uint32 fullDuplex);
int32 rtl8306_getEthernetPHY(uint32 phy, uint32 *autoNegotiation, uint32 *advCapability, uint32 *speed, uint32 *fullDuplex);
int32 rtl8306_getPHYLinkStatus(uint32 phy, uint32 *linkUp);
int32 rtl8306_setPort5LinkStatus(uint32 enabled);
int32 rtl8306_initVlan(void);
int32 rtl8306_addVlan(uint32 vid);
int32 rtl8306_delVlan(uint32 vid);
int32 rtl8306_addVlanPortMember(uint32 vid, uint32 port);
int32 rtl8306_delVlanPortMember(uint32 vid, uint32 port);
int32 rtl8306_getVlanPortMember(uint32 vid, uint32 *portmask);
int32 rtl8306_setPvid(uint32 port, uint32 vid);
int32 rtl8306_getPvid(uint32 port, uint32 *vid);
int32 rtl8306_setIngressFilter(uint32 enabled);
int32 rtl8306_setVlanTagOnly(uint32 enabled);
int32 rtl8306_setVlanTagAware(uint32 enabled);
int32 rtl8306_init(void);
int32 rtl8306_initQos(uint32 qnum);   


#endif

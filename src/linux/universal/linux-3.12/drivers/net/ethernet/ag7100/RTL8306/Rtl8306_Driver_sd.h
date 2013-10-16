#ifndef _RTL8306_DRIVER_2_H_
#define _RTL8306_DRIVER_2_H_




typedef struct rtl8306_qosPriorityArbitPara_s {
uint32 aclbasedLevel;
uint32 dscpbasedLevel;
uint32 dot1qbasedLevel;
uint32 portbasedLevel;
} rtl8306_qosPriorityArbitPara_t;


typedef struct rtl8306_qosQueueRatePara_s {
    uint32   q3_n64Kbps;
    uint32   q3_burstsize;
    uint32   q2_n64Kbps;
    uint32   q2_burstsize;       
} rtl8306_qosQueueRatePara_t ;

typedef struct rtl8306_qosIPadrressPara_s {
    uint32 ip1;
    uint32 ip1mask;
    uint32 ip2;
    uint32 ip2mask;
    uint32 prio;
} rtl8306_qosIPadrressPara_t;

typedef struct rtl8306_qosPriorityEnablePara_s {
    uint32 en_dscp;
    uint32 en_1qBP;
    uint32 en_pbp;
    uint32 en_cputag;
} rtl8306_qosPriorityEnablePara_t;

typedef struct rtl8306_mirrorPara_s {
    uint32 mirport;
    uint32 rxport;
    uint32 txport;
    uint8 macAddr[6];
    uint32 enMirMac;
} rtl8306_mirrorPara_t;


typedef struct rtl8306_stormPara_s {
    uint32 enBroadStmfil;
    uint32 enUDAStmfil;
} rtl8306_stormPara_t;

#define RTL8306_PRI_PORTBASE     0
#define RTL8306_PRI_1QDEFAULT   1 
#define RTL8306_PRI_1QTAG          2
#define RTL8306_PRI_DSCP            3
#define RTL8306_PRI_IP                 4

int32 rtl8306_addLUTUnicastMacAddress(uint8 *macAddress, uint32 age, uint32 isStatic, uint32 isAuth, uint32 port);
int32 rtl8306_delLUTMacAddress(uint8 *macAddress);
int32 rtl8306_addLUTMulticastMacAddress(uint8 *macAddress, uint32 portMask);
int32 rtl8306_setCPUPort(uint32 port, uint32 enTag, rtl8306_qosQueueRatePara_t* querate);
int32 rtl8306_setPrioritytoQIDMapping(uint32 priority, uint32 qid);
int32 rtl8306_getPrioritytoQIDMapping(uint32 priority, uint32 *qid);
int32 rtl8306_setPrioritySourceArbit(rtl8306_qosPriorityArbitPara_t priArbit);
int32 rtl8306_getPrioritySourceArbit(rtl8306_qosPriorityArbitPara_t *priArbit);
int32 rtl8306_setPrioritySourceEnable(uint32 port, rtl8306_qosPriorityEnablePara_t enPri );
int32 rtl8306_getPrioritySourceEnable(uint32 port, rtl8306_qosPriorityEnablePara_t* enPri );
int32 rtl8306_setPrioritySourcePriority(uint32 pri_type, uint32 pri_obj, uint32 pri_val) ;
int32 rtl8306_getPrioritySourcePriority(uint32 pri_type, uint32 pri_obj, uint32 *pri_val) ;
int32  rtl8306_setIPAddressPriority(rtl8306_qosIPadrressPara_t ippri);
int32 rtl8306_setPortRate(uint32 port, uint32 isOutPut, uint32 n64Kbps,  uint32 enabled);
int32 rtl8306_getPortRate(uint32 port, uint32 isOutPut, uint32 *n64Kbps, uint32 *enabled);
int32 rtl8306_setSpanningTreePortState(uint32 port, uint32 state);
int32 rtl8306_getSpanningTreePortState(uint32 port, uint32 * state);
int32 rtl8306_setStormFilter(rtl8306_stormPara_t storm) ;
int32 rtl8306_setDot1xPortBased(uint32 port, uint32 enabled, uint32 isAuth, uint32 direction);
int32 rtl8306_setDot1xMacBased(uint32 port, uint32 enabled, uint32 direction);
int32 rtl8306_enableInterrupt(uint32 enInt, uint32 intmask) ;
int32 rtl8306_getInterruptEvent(uint32 *intmask);
int32 rtl8306_setLedStatus(uint32 port, uint32 group, uint32 status, uint32 enCPUCtl);
int32 rtl8306_getLedStatus(uint32 port, uint32 group, uint32 *status, uint32 *enCPUCtl);


#endif


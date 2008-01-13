#include <linux/mii.h>

#define DEVELOPE_ENV

#ifdef DEVELOPE_ENV
#define	ET_5325E(args)	printk args	
#else
#define	ET_5325E(args)
#endif

#define BCM5397
extern char *nvram_get(const char *name);
#define nvram_safe_get(name) (nvram_get(name)?:"0")

struct port_qos_t{
	short page;
	short addr;
	uint16 content_mask;
	int len;
	uint16 *content_set; 
	uint32 content32_mask;
	uint32 *content32_set;
};


#ifdef BCM5397
/*=====================================Power=================================================================*/
#if 0
#define PAGE_PORT0_PHY 0x10
#define PAGE_PORT1_PHY 0x11
#define PAGE_PORT2_PHY 0x12
#define PAGE_PORT3_PHY 0x13
#define PAGE_PORT4_PHY 0x14
#define PAGE_PORT5_PHY 0x15  /*below for 8port like bcm5398*/
#define PAGE_PORT6_PHY 0x16
#define PAGE_PORT7_PHY 0x17
#define REG_POWER_LED  0x30 /*SPI=30 MII=18h*/
#define POWER_VAL      0xC042
#define POWER_VAL_MASK 0x0000
uint16 power_content[] = {0x0,POWER_VAL};
static struct port_qos_t power_port0_en[] = {
        { PAGE_PORT0_PHY, REG_POWER_LED, POWER_VAL_MASK, 2, power_content, 0, 0},
        { -1}
};
static struct port_qos_t power_port1_en[] = {
        { PAGE_PORT1_PHY, REG_POWER_LED, POWER_VAL_MASK, 2, power_content, 0, 0},
        { -1}
};
static struct port_qos_t power_port2_en[] = {
        { PAGE_PORT2_PHY, REG_POWER_LED, POWER_VAL_MASK, 2, power_content, 0, 0},
        { -1}
};
static struct port_qos_t power_port3_en[] = {
        { PAGE_PORT3_PHY, REG_POWER_LED, POWER_VAL_MASK, 2, power_content, 0, 0},
        { -1}
};
static struct port_qos_t power_port4_en[] = {
        { PAGE_PORT4_PHY, REG_POWER_LED, POWER_VAL_MASK, 2, power_content, 0, 0},
        { -1}
};
#endif

/*======================Per-Port priority support four leave ================================================*/
/*When port-base QoS is Enable. Ingress frames are assigned a priority ID value
  based on the "PORT_QOS_PRI" bits in the "Default IEEE 802.1Q Tag Register(Page 34h) Addr(10h) on page.193"
  Below only for PortBase Setting
*/
#define PAGE_802_1Q		0x34 
#define REG_802_1Q_PORT0        0x10 /*PortBase Priority setting (Port0 is WanPort)*/
#define REG_802_1Q_PORT1	0x12
#define REG_802_1Q_PORT2        0x14
#define REG_802_1Q_PORT3        0x16
#define REG_802_1Q_PORT4        0x18
//#define REG_802_1Q_IMPPORT	0x20
#define QOS_802_1Q_HIGH_PRIOR   0xE001/*classification priority*/
#define QOS_802_1Q_MEDIUM_PRIOR 0xA001
#define QOS_802_1Q_NORMAL_PRIOR 0x6001
#define QOS_802_1Q_LOW_PRIOR 	0x1001
#define QOS_802_1Q_PRIOR_MASK   0xFFFF

uint16 qos_portbase_prior_content[]=
	{0x0001, QOS_802_1Q_HIGH_PRIOR, QOS_802_1Q_MEDIUM_PRIOR, QOS_802_1Q_NORMAL_PRIOR, QOS_802_1Q_LOW_PRIOR};

//static struct port_qos_t priority_0[] = {
//        { PAGE_802_1Q, REG_802_1Q_PORT0, QOS_802_1Q_PRIOR_MASK, 2, qos_portbase_prior_content, 0, 0},
//        {-1}
//};

static struct port_qos_t priority_1[] = {
        { PAGE_802_1Q, REG_802_1Q_PORT1, QOS_802_1Q_PRIOR_MASK, 2, qos_portbase_prior_content, 0, 0},
        {-1}
};

static struct port_qos_t priority_2[] = {
        { PAGE_802_1Q, REG_802_1Q_PORT2, QOS_802_1Q_PRIOR_MASK, 2, qos_portbase_prior_content, 0, 0},
        {-1}
};

static struct port_qos_t priority_3[] = {
        { PAGE_802_1Q, REG_802_1Q_PORT3, QOS_802_1Q_PRIOR_MASK, 2, qos_portbase_prior_content, 0, 0},
        {-1}
};

static struct port_qos_t priority_4[] = {
        { PAGE_802_1Q, REG_802_1Q_PORT4, QOS_802_1Q_PRIOR_MASK, 2, qos_portbase_prior_content, 0, 0},
        {-1}
};

/*=======================================================================================================*/
#define PAGE_QOS                0x30

//#define REG_QOS_TCR             0x01 /*QoS Threshold control Register (01h-02h) 16bit*/
//#define REG_QOS_PRIOR_802_1P     0x10 /*IEEE 802.1P Priority Map Register (10h-13h) 32bit*/

/*Common Variate [Priority ID is mapped to one of the egress transmit queue base on]*/
#define REG_QOS_TX_QC           0x80 /*Tx Queue Control Register 80h 8bit*/
#define REG_QOS_TX_QW_0		0x81 /*Tx Queue Weight Register 81h 8bit*/
#define REG_QOS_TX_QW_1         0x82 /*Tx Queue Weight Register 82h 8bit*/
#define REG_QOS_TX_QW_2         0x83 /*Tx Queue Weight Register 83h 8bit*/
#define REG_QOS_TX_QW_3         0x84 /*Tx Queue Weight Register 84h 8bit*/

/*=============================(PortBase Variate)=============================================*/
/*QoS Global Control Register 00h 8bit  Default PortBase Enable*/
#define REG_QOS_PORTBASE_EN     0x00
#if 1 //Enable port_base function
//#define QOS_PORTBASE_EN_CONTENT	 (0x4C)
#define QOS_PORTBASE_EN_CONTENT  (0x48)
#define QOS_PORTBASE_EN_MASK     (0xff^QOS_PORTBASE_EN_CONTENT)
#else  //disable portbase function
#define QOS_PORTBASE_EN_CONTENT  0x08   
#define QOS_PORTBASE_EN_MASK     0xff^QOS_PORTBASE_EN_CONTENT
#endif
static uint16 qos_portbase_en_content[] = {0x0, QOS_PORTBASE_EN_CONTENT}; //0x0 is disable QoS  
static struct port_qos_t qos_portbase_en[] = {
        { PAGE_QOS, REG_QOS_PORTBASE_EN, QOS_PORTBASE_EN_MASK, 1, qos_portbase_en_content, 0, 0},
        { -1}
};

/*=============QoS (802.1P Variate)========================================================*/
#if 0
#define REG_QOS_802_1P_EN        0x04 /*IEEE 802.1P Enable Register (04h-05h) 16bit*/
#define QOS_802_1P_EN            0x009e /*Enable 0-4 Port/IMP_Port Num*/
#define QOS_802_1P_EN_MASK	 0xffff^QOS_802_1P_EN
static uint16 qos_802_1p_en_content[] ={0x0, QOS_802_1P_EN};
static struct port_qos_t qos_802_1p_en[] = {
        { PAGE_QOS, REG_QOS_802_1P_EN, QOS_802_1P_EN_MASK, 2, qos_802_1p_en_content, 0, 0},
        {-1}
};

#define REG_QOS_802_1P_VAL 	0x10
#define QOS_802_1P_VAL		0x00FAC688
#define QOS_802_1P_VAL_MASK	0xFFFFFFFF^QOS_802_1P_VAL
static uint32 qos_802_1p_val_content[]={0x0, QOS_802_1P_VAL};
static struct port_qos_t qos_802_1p_val[] = {
        { PAGE_QOS, REG_QOS_802_1P_VAL, 0, 4, 0, QOS_802_1P_VAL_MASK, qos_802_1p_val_content},
        {-1}
};
#endif
/*========================QoS (DiffServ DSCP Variate)======================================*/
#if 0
#define REG_QOS_DSCP_EN      0x06 /*DiffServ Enable Register (06h-07h) 16bit*/
#define QOS_DSCP_EN	     0x009e /*Enable 0-4 PortNum*/
#define QOS_DSCP_EN_MASK     0xffff^QOS_DSCP_EN
static uint16 qos_dscp_en_content[] = {0x0, QOS_DSCP_EN};
static struct port_qos_t qos_dscp_en[] = {
        { PAGE_QOS, REG_QOS_DSCP_EN, QOS_DSCP_EN_MASK, 1, qos_dscp_en_content, 0, 0},
        { -1}
};

/*==========QoS DSCP Variate Map0  every time 16bit=======================================*/
#define REG_QOS_DSCP_MAP_0   0x30 /*DiffServ Priority Map 0 Register (30h-35h) 48bit*/
#define QOS_DSCP_VAL_MASK    0xFF
#define QOS_DSCP_LV1_BIT     30   /*AF1 (30h) Medium */
#define QOS_DSCP_LV1_VAL     0x6
#define QOS_DSCP_LV5_BIT     0    /*BE  (30h) Low    */
#define QOS_DSCP_LV5_VAL     0x0
static uint16 qos_dscp_map0_content[4]={0x0,
					(uint16)0x0, 
					(uint16)(QOS_DSCP_LV1_VAL << (QOS_DSCP_LV1_BIT -16)),
					(uint16)0x0
};
static struct port_qos_t qos_dscp_map0_val[] = {
        { PAGE_QOS, REG_QOS_DSCP_MAP_0, QOS_DSCP_VAL_MASK, 6, qos_dscp_map0_content, 0, 0},
        {-1}
};
/*============================Diff-Ser Map1============================================*/
#define REG_QOS_DSCP_MAP_1   0x36 /*DiffServ Priority Map 1 Register (36h-3Bh) 48bit*/
#define QOS_DSCP_LV2_BIT     6    /*AF2 (36h)*/
#define QOS_DSCP_LV2_VAL     0x4
#define QOS_DSCP_LV3_BIT     30   /*AF3 (36h) Normal */
#define QOS_DSCP_LV3_VAL     0x3
static uint16 qos_dscp_map1_content[4]={0x0,
					(uint16)0x0,
                                        (uint16)(QOS_DSCP_LV3_VAL << (QOS_DSCP_LV3_BIT - 16)),
                                        (uint16)(QOS_DSCP_LV2_VAL << QOS_DSCP_LV2_BIT)
};
static struct port_qos_t qos_dscp_map1_val[] = {
        { PAGE_QOS, REG_QOS_DSCP_MAP_1, QOS_DSCP_VAL_MASK, 6, qos_dscp_map1_content, 0, 0},
        {-1}
};
/*==========================Diff-Ser Map2 ===============================================*/
#define REG_QOS_DSCP_MAP_2      0x3C /*DiffServ Priority Map 2 Register (3Ch-31h) 48bit*/
#define QOS_DSCP_LV0_BIT     42   /*EF  (3Ch) High   */
#define QOS_DSCP_LV0_VAL     0x7  /*here is reference Ingress Port Priority ID Map Register*/
#define QOS_DSCP_LV4_BIT     6    /*AF4 (3Ch)        */
#define QOS_DSCP_LV4_VAL     0x2
//#define QOS_DSCP_MAP2_CONTENT ( QOS_DSCP_LV0_VAL << QOS_DSCP_LV0_BIT ) | ( QOS_DSCP_LV4_VAL << QOS_DSCP_LV4_BIT )
static uint16 qos_dscp_map2_content[4]={0x0,
					(uint16)(QOS_DSCP_LV0_VAL << (QOS_DSCP_LV0_BIT - 32)),
					(uint16)0x0,
					(uint16)(QOS_DSCP_LV4_VAL << QOS_DSCP_LV4_BIT) 
};
static struct port_qos_t qos_dscp_map2_val[] = {
        { PAGE_QOS, REG_QOS_DSCP_MAP_2, QOS_DSCP_VAL_MASK, 6, qos_dscp_map2_content, 0, 0},
        {-1}
};
/*===========================Diff-Ser Map3 ================================================*/
#define REG_QOS_DSCP_MAP_3      0x42 /*DiffServ Priority Map 3 Register (42h-47h) 48bit*/
static uint16 qos_dscp_map3_content[4]={0x0,0x0,0x0,0x0}; //Not Map
static struct port_qos_t qos_dscp_map3_val[] = {
        { PAGE_QOS, REG_QOS_DSCP_MAP_3, QOS_DSCP_VAL_MASK, 6, qos_dscp_map3_content, 0, 0},
        {-1}
};
#endif
/*=================== Ingress Port Priority ID Map(Enable per-port support queue num)=================*/
#define REG_QOS_PORT0_PRIOID   0x50 /*Ingress Port Priority ID Map Register (50h-51h) 16bit*/
#define REG_QOS_PORT1_PRIOID   0x52 /*Ingress Port Priority ID Map Register (52h-53h) 16bit*/
#define REG_QOS_PORT2_PRIOID   0x54 /*Ingress Port Priority ID Map Register (54h-55h) 16bit*/
#define REG_QOS_PORT3_PRIOID   0x56 /*Ingress Port Priority ID Map Register (56h-57h) 16bit*/
#define REG_QOS_PORT4_PRIOID   0x58 /*Ingress Port Priority ID Map Register (58h-59h) 16bit*/
#define REG_QOS_IMPPORT_PRIOID 0x60 /*Ingress Port Priority ID MAP Register (60h-61h) 16bit*/
#define QOS_PORT_PRIOID        0xFA50 /*Is Support ALL Priority Queue ID (50h-51h 16bit)*/ 
#define QOS_PORT_PRIOID_MASK   (0x0) /*Because it is 16 bit*/ 
static uint16 qos_prioid_content[]={0x0,QOS_PORT_PRIOID};

//static struct port_qos_t ingress_port0[] = {
//        { PAGE_QOS, REG_QOS_PORT0_PRIOID, QOS_PORT_PRIOID_MASK, 2, qos_prioid_content, 0, 0},
//        {-1}
//};


static struct port_qos_t ingress_port1[] = {
        { PAGE_QOS, REG_QOS_PORT1_PRIOID, QOS_PORT_PRIOID_MASK, 2, qos_prioid_content, 0, 0},
        {-1}
};

static struct port_qos_t ingress_port2[] = {
        { PAGE_QOS, REG_QOS_PORT2_PRIOID, QOS_PORT_PRIOID_MASK, 2, qos_prioid_content, 0, 0},
        {-1}
};

static struct port_qos_t ingress_port3[] = {
        { PAGE_QOS, REG_QOS_PORT3_PRIOID, QOS_PORT_PRIOID_MASK, 2, qos_prioid_content, 0, 0},
        {-1}
};

static struct port_qos_t ingress_port4[] = {
        { PAGE_QOS, REG_QOS_PORT4_PRIOID, QOS_PORT_PRIOID_MASK, 2, qos_prioid_content, 0, 0},
        {-1}
};

static struct port_qos_t ingress_impport[] = {
        { PAGE_QOS, REG_QOS_IMPPORT_PRIOID, QOS_PORT_PRIOID_MASK, 2, qos_prioid_content, 0, 0},
        {-1}
};


/*=============================Port Control Register Page.165============================*/
//#define QOS_PORT0_CTL_VAL    0x0/*bit8 DuplexMode/bit9 RE_Auto-negotiation/bit12 Auto-negotiation_EN/bit13 Speed*/
//#define QOS_PORT1_CTL_VAL    0x0
//#define QOS_PORT2_CTL_VAL    0x0
//#define QOS_PORT3_CTL_VAL    0x0
//#define QOS_PORT4_CTL_VAL    0x0


/*==================================Tx Queue Control ==================================*/
#if 0
/*Common Variate [Priority ID is mapped to one of the egress transmit queue base on]*/
#define REG_QOS_TX_QC           0x80 /*Tx Queue Control Register 80h 8bit*/
//#define REG_QOS_TX_QW_0         0x81 /*Tx Queue Weight Register 81h 8bit*/
//#define REG_QOS_TX_QW_1         0x82 /*Tx Queue Weight Register 82h 8bit*/
//#define REG_QOS_TX_QW_2         0x83 /*Tx Queue Weight Register 83h 8bit*/
//#define REG_QOS_TX_QW_3         0x84 /*Tx Queue Weight Register 84h 8bit*/
#define QOS_TXQ_BIT          2
#define QOS_TXQ_VAL	     0x3 /*four-queue mode*/
#define QOS_TX_QUEUE_CONTENT (QOS_TXQ_VAL << QOS_TXQ_BIT)
#define QOS_TX_QUEUE_MASK    0xff
static uint16 qos_tx_queue_content[] = {0x0, QOS_TX_QUEUE_CONTENT };
static struct port_qos_t qos_tx_queue[] = {
        { PAGE_QOS, REG_QOS_TX_QC, QOS_TX_QUEUE_MASK, 1, qos_tx_queue_content, 0, 0},
        { -1}
};
#endif
/*========================Traffic priority remap control register==========================*/
#if 0
#define REG_QOS_TRAFFIC_PRIO  0xA0 /*Enable Traffic Priority Remap Control Register (A0h-A1h)16bit*/
#define QOS_TRAFFIC_PRIO_VAL  0x1F  /*Enable 0~4 port*/
#define QOS_TRAFFIC_PRIO_MASK 0xff
static uint16 qos_traffic_prio_content[] = {0x0, QOS_TRAFFIC_PRIO_VAL};
static struct port_qos_t qos_traffic_prio[] = {
        { PAGE_QOS, REG_QOS_TRAFFIC_PRIO, QOS_TRAFFIC_PRIO_MASK, 1, qos_traffic_prio_content, 0, 0},
        { -1}
};
#endif
/*==============================================================================================*/

#if 0
static struct port_qos_t flow_control_0[] = {{ -1}};
static struct port_qos_t rate_limit_0[] = {{ -1}};
static struct port_qos_t flow_control_1[] = {{ -1}};
static struct port_qos_t rate_limit_1[] = {{ -1}};
static struct port_qos_t flow_control_2[] = {{ -1}};
static struct port_qos_t rate_limit_2[] = {{ -1}};
static struct port_qos_t flow_control_3[] = {{ -1}};
static struct port_qos_t rate_limit_3[] = {{ -1}};
static struct port_qos_t flow_control_4[] = {{ -1}};
static struct port_qos_t rate_limit_4[] = {{ -1}};

static struct port_qos_t wan_speed[] = {{-1}};
#endif 

#endif

static struct port_qos_t *port_mii_addr[] = {
#if 1
	//priority_0,             //Port0 is Wan_Port
	//flow_control_0,
	//rate_limit_0,
        priority_1,
        //flow_control_1,
        //rate_limit_1,
	priority_2,
	//flow_control_2,
	//rate_limit_2,
	priority_3,
	//flow_control_3,
	//rate_limit_3,
	priority_4,
	//flow_control_4,
	//rate_limit_4,
	//wan_speed,
	//qos_tx_queue,
	//qos_traffic_prio,
	qos_portbase_en,
	//qos_802_1p_en,
	//qos_802_1p_val,
	//qos_dscp_en,
	//qos_dscp_map0_val,
	//qos_dscp_map1_val,
	//qos_dscp_map2_val,
	//qos_dscp_map3_val,
	//ingress_port0,
	ingress_port1,
	ingress_port2,
	ingress_port3,
	ingress_port4,
	ingress_impport,
        //power_port0_en,
	//power_port1_en,
	//power_port2_en,
	//power_port3_en,
	//power_port4_en,
#endif
	NULL
};


static char *port_option_name[] = {
#if 1
 	//"port_priority_0",     //port0 is wan
	//"port_flow_control_0",
	//"port_rate_limit_0",
        "port_priority_1",
        //"port_flow_control_1",
        //"port_rate_limit_1",
	"port_priority_2",
	//"port_flow_control_2",
	//"port_rate_limit_2",
	"port_priority_3",
	//"port_flow_control_3",
	//"port_rate_limit_3",
	"port_priority_4",
	//"port_flow_control_4",
	//"port_rate_limit_4",
	//"wan_speed",
	//"QoS", /*Tx_Queue (Enable four Queue)*/
	//"QoS", /*Enable Traffic Priority Remap control*/
        "QoS", /*PortBase enable*/
	//"QoS", /*802.1P Enable*/
	//"QoS", /*802.1P Val setup*/
        //"QoS", /*dscp en*/
        //"QoS", /*dscp value to map 0~3 (Everytime=6byte)*/
        //"QoS",
	//"QoS",
	//"QoS",
	"QoS", /*ingress port for five portnum + imp_port number*/
	"QoS",
	"QoS",
	"QoS",
	"QoS",
	//"QoS", /*for priority imp port*/
	//"QoS", /*test power led function 0~ 4*/
	//"QoS",
	//"QoS",
	//"QoS",
	//"QoS",
#endif
	NULL
};

void ReadDataFromRegister_(robo_info_t *robo, unsigned short reg_idx, unsigned short RegNumber, unsigned short *hidata, unsigned short *lodata)
{
	
        uint16 val16 = 0;
	
	ET_5325E(( "1:%x(0x%02x)\t value=0x%04x\n", reg_idx, RegNumber, *lodata));
	robo->ops->read_reg(robo, reg_idx, RegNumber, &val16, 2);
	ET_5325E(( "1:%x(0x%02x)\t val16=0x%04x\n", reg_idx, RegNumber, val16));
	*lodata = val16;
	ET_5325E(( "1:%x(0x%02x)\t value=0x%04x\n", reg_idx, RegNumber, *lodata));

}

/*reg_idx=phyid /conternt_idx=val_in*/
void WriteDataToRegister_(robo_info_t *robo, unsigned short reg_idx, unsigned short content_idx)
{
    uint8 PageNumber = 0, RegNumber = 0;
    uint8 val8 = 0;
    uint16 val16 = 0, val48[3];
    uint32 val32 = 0, val64[2];
    memset(val48,0,6);
    memset(val64,0,8);
    int i, len;
    char *value=NULL;
    int int_port = 0;
    struct port_qos_t *port_qos = port_mii_addr[reg_idx];
   
    for (i=0; port_qos[i].page != -1; i++)
    {
    	PageNumber = port_qos[i].page;
    	RegNumber = port_qos[i].addr;
    	len =  port_qos[i].len;
	
	ET_5325E(("@_@ %s i[%d]Page[%x]RegNum[%x]len[%d] QoS.\n",__FUNCTION__,i,PageNumber,RegNumber,len));
	
	switch (len){
	case 1:
		robo->ops->read_reg(robo, PageNumber, RegNumber, &val8, len);
		ET_5325E(( "Orig data[0x%02x]len[%d].\n",val8, len));
        	val8 = val8 & (uint8)port_qos[i].content_mask | ((uint8)port_qos[i].content_set[content_idx]);
        	ET_5325E(( "Writer 8bit data[0x%02x]\n", val8));
        	robo->ops->write_reg(robo, PageNumber, RegNumber, &val8, len);
        	robo->ops->read_reg(robo, PageNumber, RegNumber, &val8, len);
        	ET_5325E(( "Reload(Page[0x%02x]Reg[0x%02x])data[0x%02x].\n", PageNumber, RegNumber, val8));
		break;
	case 4:
		robo->ops->read_reg(robo, PageNumber, RegNumber, &val32, len);
		ET_5325E(( "Orig data[0x%08x] len[%d]\n", val32, len));
        	val32 = port_qos[i].content32_set[1];
        	ET_5325E(( "Write 32bit data[0x%08x].\n", val32));
        	robo->ops->write_reg(robo, PageNumber, RegNumber, &val32, len);
        	robo->ops->read_reg(robo, PageNumber, RegNumber, &val32, len);
        	ET_5325E(( "Reload(Page[0x%x]Reg[0x%x])data[0x%08x]\n", PageNumber, RegNumber, val32));
		break;
        case 6:
                robo->ops->read_reg(robo, PageNumber, RegNumber, &val48, len);
                ET_5325E(( "Orig data[0x%04x-0x%04x-0x%04x]len[%d]\n", val48[0], val48[1], val48[2], len));
                memcpy(&val48[0],&(port_qos[i].content_set[1]),2);//copy 2byte
                memcpy(&val48[1],&(port_qos[i].content_set[2]),2); //copy 2byte
                memcpy(&val48[2],&(port_qos[i].content_set[3]),2); //copy 2byte
		
                ET_5325E(("Writer 48bit data[0x%04x-0x%04x-0x%04x].\n", val48[0], val48[1], val48[2]));
                robo->ops->write_reg(robo, PageNumber, RegNumber, &val48, len);
                robo->ops->read_reg(robo, PageNumber, RegNumber, &val48, len);
                printk( "Reload(Page[0x%02x]:Reg[0x%02x])data[0x%04x-0x%04x-0x%04x].\n\n", 
			PageNumber, RegNumber,  val48[0], val48[1], val48[2]);
                break;

        case 8:
                robo->ops->read_reg(robo, PageNumber, RegNumber, &val64, len);
                ET_5325E(( "Orig value[0x%08x-0x%08x] len[%d]\n", val64[0],val64[1], len));
		memcpy(&val64[0], &(port_qos[i].content32_set[1]),4);
		memcpy(&val64[1], &(port_qos[i].content32_set[2]),4);
		ET_5325E(("Writer 64bit data[0x%08x-0x%08x].\n", val64[0], val64[1]));
               	robo->ops->write_reg(robo, PageNumber, RegNumber, &val64, len);
               	robo->ops->read_reg(robo, PageNumber, RegNumber, &val64, len);
        	printk( "Reload(Page[0x%02x]:Reg[0x%02x])value[0x%08x-0x%08x].\n\n", PageNumber, RegNumber, val64[0],val64[1]);
		break;

	case 2:
		//(uint16 *)data = &val16;
		if(!strcmp(port_option_name[reg_idx], "wan_speed"))
		{// by tallest for wan port speed 
			value = nvram_safe_get("vlan1ports");
			if(!value)
				value = "0 5";
			sscanf(value, "%d",&int_port);
			PageNumber = PageNumber + int_port;
			ET_5325E(( "Wan Port Speed: %dth port, PageNO=0x%x\n",int_port, PageNumber));
		}
	default :
		robo->ops->read_reg(robo, PageNumber, RegNumber, &val16, len);
		ET_5325E(( "Qrig value[0x%04x] len[%d]\n", val16, len));
        	val16 = (val16 & port_qos[i].content_mask) | (port_qos[i].content_set[content_idx]);
        	ET_5325E(( "(Write data[0x%04x]\n", val16));
        	robo->ops->write_reg(robo, PageNumber, RegNumber, &val16, len);
        	robo->ops->read_reg(robo, PageNumber, RegNumber, &val16, len);
        	ET_5325E(( "(Reload Page[0x%02x]Reg[0x%02x])value[0x%04x].\n\n", 
				PageNumber, RegNumber, val16));
		break;
	}
	
    }
}



/*Initial QoS Setting*/
void
robo_config_qos(robo_info_t *robo)
{
	unsigned short i;
	char *value = NULL,*tmp;
	tmp = nvram_safe_get("QoS");
	if (bcm_atoi(tmp))
	{
		for (i = 0; port_option_name[i]; i++)
		{
			if((value = nvram_safe_get(port_option_name[i])))
				WriteDataToRegister_(robo, i, bcm_atoi(value)); 
		}
	}
	else
	{
                for (i = 0; port_option_name[i]; i++)
                {
                        if((value = nvram_safe_get(port_option_name[i])))
                                WriteDataToRegister_(robo, i, 0);
                }
	}
	return;
}



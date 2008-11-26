#include <Copyright.h>

/********************************************************************************
* msApiPrototype.h
*
* DESCRIPTION:
*       API Prototypes for QuarterDeck Device
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#ifndef __msApiInternal_h
#define __msApiInternal_h

#ifdef __cplusplus
extern "C" {
#endif

#ifdef GT_USE_SIMPLE_PORT_MAPPING
#define GT_LPORT_2_PORT(_lport)      (GT_U8)((_lport) & 0xff)
#define GT_PORT_2_LPORT(_port)       (GT_32)((_port) & 0xff)
#define GT_LPORTVEC_2_PORTVEC(_lvec)      (GT_U8)((_lvec) & 0xffff)
#define GT_PORTVEC_2_LPORTVEC(_pvec)       (GT_32)((_pvec) & 0xffff)
#else
#define GT_LPORT_2_PORT(_lport)      lport2port(dev->validPortVec, _lport)
#define GT_PORT_2_LPORT(_port)       port2lport(dev->validPortVec, _port)
#define GT_LPORTVEC_2_PORTVEC(_lvec)	lportvec2portvec(dev->validPortVec, _lvec)
#define GT_PORTVEC_2_LPORTVEC(_pvec)	portvec2lportvec(dev->validPortVec, _pvec)
#endif

#define GT_IS_PORT_SET(_portVec, _port)	\
			((_portVec) & (0x1 << (_port)))

#define GT_IS_IRLUNIT_VALID(_dev,_unit)		\
		(((_dev)->deviceId == GT_88E6065)?(_unit < 12):	\
		(((_dev)->deviceId == GT_88E6055)?(_unit < 12):	\
		(((_dev)->deviceId == GT_88E6061)?(_unit < 6):	\
		(((_dev)->deviceId == GT_88E6035)?(_unit < 6):	\
										 (_unit < 3)))))


/* The following macro converts a binary    */
/* value (of 1 bit) to a boolean one.       */
/* 0 --> GT_FALSE                           */
/* 1 --> GT_TRUE                            */
#define BIT_2_BOOL(binVal,boolVal)                                  \
            (boolVal) = (((binVal) == 0) ? GT_FALSE : GT_TRUE)

/* The following macro converts a boolean   */
/* value to a binary one (of 1 bit).        */
/* GT_FALSE --> 0                           */
/* GT_TRUE --> 1                            */
#define BOOL_2_BIT(boolVal,binVal)                                  \
            (binVal) = (((boolVal) == GT_TRUE) ? 1 : 0)

/* device name - devName */
#define DEV_88E6051                      0x0001    /* quarterdeck 6051      */
#define DEV_88E6052                      0x0002    /* quarterdeck 6052      */
#define DEV_88E6021                      0x0004    /* fullsail              */
#define DEV_88E6060                      0x0008    /* Gondola               */
#define DEV_88E6063                      0x0010    /* clippership 6063      */
#define DEV_FF_EG                        0x0020    /* FireFox-EG            */
#define DEV_FF_HG                        0x0040    /* FireFox-HG            */
#define DEV_FH_VPN                       0x0080    /* FireHawk-VPN          */
#define DEV_88E6083                      0x0100    /* Octane 6083           */
#define DEV_88E6181                      0x0200    /* Sapphire 88E6181      */
#define DEV_88E6183                      0x0400    /* Sapphire 88E6153,88E6183 */
#define DEV_88E6093	 	                 0x0800   /* 88E6093                  */
#define DEV_88E6092	 	                 0x1000   /* 88E6092                  */
#define DEV_88E6095	 	                 0x2000   /* 88E6095                  */
#define DEV_88E6182                      0x4000   /* Jade 88E6152, 88E6182 */
#define DEV_88E6185                      0x8000   /* Jade 88E6155, 88E6185 */
#define DEV_88E6108                      0x10000   /* 88E6108 */
#define DEV_88E6061                      0x20000   /* 88E6031, 88E6061 */
#define DEV_88E6065                      0x40000   /* 88E6035, 88E6055, 88E6065 */

#define DEV_88E6095_FAMILY	( DEV_88E6092 | DEV_88E6095 )
#define DEV_88E6185_FAMILY	( DEV_88E6182 | DEV_88E6185 | DEV_88E6108)

#define DEV_88E6065_FAMILY	( DEV_88E6061 | DEV_88E6065 )

#define DEV_NEW_FEATURE_IN_REV (DEV_88E6095_FAMILY | DEV_88E6182 | DEV_88E6185)

#define DEV_BURST_RATE		( DEV_88E6108 )
#define DEV_DROP_BCAST		( DEV_88E6108 )
#define DEV_ARP_PRI	        ( DEV_88E6108 )
#define DEV_SNOOP_PRI	    ( DEV_88E6108 )
#define DEV_SERDES_CORE	    ( DEV_88E6108 )

#define DEV_AGE_INTERRUPT	( DEV_88E6108 )

#define DEV_PORT_BASED_AGE_INT	( DEV_88E6065_FAMILY )

/* DEV_8PORT_SWITCH is used to access the given device's Register Map */
#define DEV_8PORT_SWITCH	( DEV_88E6083 | DEV_88E6181 | DEV_88E6183 | 	\
							  DEV_88E6093 | 								\
							  DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )

#define DEV_PORT_SECURITY	( DEV_88E6083 | DEV_88E6183 | DEV_88E6093 | 	\
							  DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )

#define DEV_BROADCAST_INVALID	( DEV_88E6051 | DEV_88E6052 | DEV_FF_EG | DEV_FF_HG)

/* Configurable ATU Size */
#define DEV_ATU_256_2048 		( DEV_88E6021 | DEV_88E6060 | DEV_88E6065_FAMILY )
#define DEV_ATU_562_2048 		\
						( DEV_88E6052 | DEV_88E6063 | DEV_FF_HG | 	\
						  DEV_FH_VPN | DEV_88E6083 )

#define DEV_ATU_SIZE_FIXED	DEV_GIGABIT_SWITCH
#define DEV_ATU_1024	(DEV_88E6108)
#define DEV_ATU_8192	(DEV_88E6095_FAMILY | DEV_88E6182 | DEV_88E6185)

#define DEV_DBNUM_FULL 	\
						( DEV_88E6021 | DEV_88E6060 | DEV_88E6063 | 	\
						  DEV_FH_VPN |  DEV_88E6083 |					\
						  DEV_88E6183 | DEV_88E6093 | DEV_88E6061 )

#define DEV_DBNUM_64 	( DEV_88E6065 )

#define DEV_DBNUM_256 	( DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )

#define DEV_STATIC_ADDR	\
						( DEV_88E6021 | DEV_FF_EG | DEV_FF_HG |			\
						  DEV_88E6052 | DEV_88E6063 | DEV_FH_VPN |		\
						  DEV_88E6083 |	DEV_88E6183 | DEV_88E6093 | 	\
						  DEV_88E6095_FAMILY | DEV_88E6185_FAMILY |		\
						  DEV_88E6065_FAMILY )

#define DEV_TRAILER		\
						( DEV_88E6021 | DEV_FF_HG | DEV_88E6052 |  		\
						  DEV_88E6063 | DEV_FH_VPN | DEV_88E6083 )

#define DEV_TRAILER_P5		( DEV_FF_EG )
#define DEV_TRAILER_P4P5	( DEV_88E6060 )

#define DEV_HEADER		\
						( DEV_FF_HG | DEV_88E6063 | DEV_FH_VPN |	\
						  DEV_88E6083 |	DEV_88E6183 | DEV_88E6093 | \
						  DEV_88E6095_FAMILY | DEV_88E6185_FAMILY |	\
						  DEV_88E6065_FAMILY )

#define DEV_HEADER_P5		( DEV_FF_EG )
#define DEV_HEADER_P4P5		( DEV_88E6060 )

/* DEV_QoS : Devices with multiple Queues for QoS Priority Support */
#define DEV_QoS			\
						( DEV_88E6021 | DEV_FF_HG |	DEV_88E6051 | 		\
						  DEV_88E6052 | DEV_88E6063 | DEV_FH_VPN |		\
						  DEV_88E6083 | DEV_88E6181 | DEV_88E6183 | 	\
						  DEV_88E6093 | DEV_88E6095_FAMILY |			\
						  DEV_88E6185_FAMILY | DEV_88E6065_FAMILY )

#define DEV_QoS_PRI		\
						( DEV_QoS & ~(DEV_88E6065_FAMILY) )

#define DEV_QoS_FPRI_QPRI	( DEV_88E6065_FAMILY )

#define DEV_TAGGING			DEV_QoS

#define DEV_EGRESS_DOUBLE_TAGGING	\
						( DEV_QoS & ~(DEV_88E6051 | DEV_88E6092 | DEV_88E6182 | DEV_88E6061 ) )

#define DEV_INGRESS_DOUBLE_TAGGING	\
						( DEV_88E6181 | DEV_88E6183 | DEV_88E6093 | 	\
						  DEV_88E6095 | DEV_88E6185 | DEV_88E6108 )

#define DEV_PRIORITY_REMAPPING		\
						( DEV_88E6181 | DEV_88E6183 | DEV_88E6093 | 	\
						  DEV_88E6095 | DEV_88E6185 | DEV_88E6108 |		\
						  DEV_88E6065 )


#define DEV_802_1Q		( DEV_88E6021 | DEV_88E6063 | DEV_FH_VPN | 			\
						  DEV_88E6083 | DEV_88E6183 | DEV_88E6093 | 		\
						  DEV_88E6095 | DEV_88E6092 | DEV_88E6185_FAMILY |	\
						  DEV_88E6065_FAMILY )

#define DEV_802_1S		( DEV_88E6095 | DEV_88E6185 | DEV_88E6108 | DEV_88E6065 )

#define DEV_802_1W		( DEV_88E6183 | DEV_88E6093 | DEV_88E6095 | 	\
						  DEV_88E6185 | DEV_88E6108 | DEV_88E6065 )

#define DEV_ATU_15SEC_AGING	( DEV_GIGABIT_SWITCH | DEV_88E6065_FAMILY )
#define DEV_ATU_RM_PORTS	( DEV_88E6093_FAMILY | DEV_88E6065 )
#define DEV_ATU_EXT_PRI		( DEV_88E6065_FAMILY )

#define DEV_VTU_EXT_INFO	( DEV_88E6065_FAMILY )

#define DEV_RMON		( DEV_88E6021 | DEV_88E6063 | DEV_FH_VPN | 			\
						  DEV_88E6083 | DEV_88E6183 | DEV_88E6093 | 		\
						  DEV_88E6092 | DEV_88E6095 | DEV_88E6185_FAMILY |	\
						  DEV_88E6065 ) 

#define DEV_RMON_TYPE_1	( DEV_88E6021 | DEV_88E6063 | DEV_FH_VPN | DEV_88E6083 ) 
#define DEV_RMON_TYPE_2 ( DEV_88E6183 )
#define DEV_RMON_TYPE_3 ( DEV_88E6093 | DEV_88E6095 | DEV_88E6092 | 	\
						  DEV_88E6185_FAMILY | DEV_88E6065 )
#define DEV_RMON_REALTIME_SUPPORT	( DEV_88E6065 )

#define DEV_IGMP_SNOOPING	\
						( DEV_88E6021 | DEV_88E6063 | DEV_FH_VPN |			\
						  DEV_88E6083 | DEV_88E6183 | DEV_88E6093 | 		\
						  DEV_88E6095_FAMILY | DEV_88E6185_FAMILY |			\
						  DEV_88E6065_FAMILY )

#define DEV_PORT_MONITORING	\
						( DEV_88E6060 | DEV_88E6063 | DEV_FH_VPN |			\
						  DEV_88E6083 | DEV_88E6183 | DEV_88E6093 | 		\
						  DEV_88E6095_FAMILY | DEV_88E6185_FAMILY |			\
						  DEV_88E6065_FAMILY )

#define DEV_ENABLE_MONITORING	\
						( DEV_88E6060 | DEV_88E6063 | DEV_FH_VPN |			\
						  DEV_88E6083 | DEV_88E6183 | DEV_88E6093 |			\
						  DEV_88E6065_FAMILY )

#define DEV_MC_RATE_PERCENT	\
						( DEV_88E6021 | DEV_88E6051 | DEV_88E6052 )

#define DEV_MC_RATE_KBPS	\
						( DEV_FF_HG | DEV_88E6063 | DEV_FH_VPN |			\
						  DEV_88E6083 )

#define DEV_INGRESS_RATE_KBPS	\
						( DEV_FF_HG | DEV_88E6063 | DEV_FH_VPN |			\
						  DEV_88E6083 | DEV_88E6181 | DEV_88E6183 | 		\
						  DEV_88E6093 | DEV_88E6095_FAMILY | DEV_88E6185_FAMILY)

#define DEV_EGRESS_RATE_KBPS	\
						( DEV_FF_HG | DEV_88E6063 | DEV_FH_VPN |			\
						  DEV_88E6083 | DEV_88E6181 | DEV_88E6183 | 		\
						  DEV_88E6093 | DEV_88E6095 | DEV_88E6185 | 		\
						  DEV_88E6108 | DEV_88E6065_FAMILY )

#define DEV_PIRL_RESOURCE	\
						( DEV_88E6065_FAMILY )

#define DEV_RESTRICTED_PIRL_RESOURCE	\
						( DEV_88E6061 )

#define DEV_NONE_RATE_LIMIT		\
						( DEV_88E6065 )

#define DEV_MII_DUPLEX_CONFIG	\
						( DEV_88E6021 | DEV_88E6063 | DEV_FH_VPN |			\
						  DEV_88E6083 )

#define DEV_QD_PLUS 	\
					( DEV_88E6021 | DEV_FF_EG | DEV_FF_HG |					\
					  DEV_88E6060 | DEV_88E6063 | DEV_FH_VPN |				\
					  DEV_88E6083 | DEV_88E6181 | DEV_88E6183 | 			\
					  DEV_88E6093 | 										\
					  DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )

#define DEV_FASTETH_SWITCH	\
					( DEV_88E6051 | DEV_88E6052 | DEV_88E6021 | 		\
					  DEV_FF_EG | DEV_FF_HG | DEV_88E6060 | 			\
					  DEV_88E6063 | DEV_FH_VPN | DEV_88E6083 |			\
					  DEV_88E6065_FAMILY )

#define DEV_ENHANCED_FE_SWITCH		( DEV_88E6065_FAMILY )

#define DEV_EXTERNAL_PHY	\
					( DEV_88E6181 | DEV_88E6183 | DEV_88E6093 |			\
					  DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )

#define DEV_EXTERNAL_PHY_ONLY	( DEV_88E6181 | DEV_88E6183 | DEV_88E6182 | DEV_88E6185 )

#define DEV_INTERNAL_GPHY   ( DEV_88E6108 )

#define DEV_FC_WITH_VALUE			\
					( DEV_88E6093 | DEV_88E6095_FAMILY | DEV_88E6185_FAMILY |	\
					  DEV_88E6065_FAMILY)
#define DEV_FC_STATUS				\
					( DEV_88E6181 | DEV_88E6183 | DEV_88E6093 | 	\
					  DEV_88E6095_FAMILY | DEV_88E6185_FAMILY | DEV_88E6065_FAMILY)
#define DEV_FC_DIS_STATUS	( DEV_88E6065_FAMILY )

#define DEV_CORE_TAG		( DEV_88E6093 | DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )

#define DEV_PCS_LINK		( DEV_88E6093 | DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )

#define DEV_MGMII_STATUS	( DEV_88E6093 | DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )

#define DEV_88E6183_FAMILY		( DEV_88E6183 | DEV_88E6185_FAMILY )
#define DEV_88E6093_FAMILY		( DEV_88E6093 | DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )

#define DEV_UNMANAGED_SWITCH	( DEV_88E6181 )

#define DEV_GIGABIT_SWITCH		\
					( DEV_88E6181 | DEV_88E6183 | DEV_88E6093 | 	\
					  DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )

#define DEV_GIGABIT_MANAGED_SWITCH	\
					( DEV_88E6183 | DEV_88E6093 | \
					  DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )

#define DEV_CROSS_CHIP_VLAN		\
					( DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )

#define DEV_TRUNK	( DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )

#define DEV_REDUCED_TRUNK	( DEV_88E6065_FAMILY )

#define DEV_FRAME_SIZE_1632		\
					( DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )

#define DEV_FLOW_CTRL_DELAY		\
					( DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )

/* port based CPU Port */
#define DEV_ENHANCED_CPU_PORT	\
					( DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )

#define DEV_CPU_PORT	( DEV_88E6065_FAMILY )
#define DEV_MULTICAST	( DEV_88E6065_FAMILY )

/* supports Reserved Multicast, etc */
#define DEV_ENHANCED_MULTICAST	( DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )
#define DEV_ARP_SUPPORT		( DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )
#define DEV_MARVELL_TAG_FLOW_CTRL	( DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )
#define DEV_USE_DOUBLE_TAG_DATA		( DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )
#define DEV_MARVELL_TAG_LOOP_BLOCK	( DEV_88E6095_FAMILY | DEV_88E6185_FAMILY )

#define DEV_PRIORITY_OVERRIDE	\
					( DEV_88E6183 | DEV_88E6093 | DEV_88E6095 | 	\
					  DEV_88E6185 | DEV_88E6108 )

#define DEV_STACKING	\
					( DEV_88E6095 | DEV_88E6185 | DEV_88E6108 )

#define DEV_8_TRUNKING	( DEV_88E6092 | DEV_88E6182 )

#define DEV_FQPRI_OVERRIDE		( DEV_88E6065 )

#define DEV_Px_MODE 	( DEV_88E6065_FAMILY )

#define DEV_SA_FILTERING	( DEV_88E6065 )

#define DEV_ARP_TO_CPU		( DEV_88E6065_FAMILY )

#define DEV_EGRESS_FLOOD	( DEV_88E6065_FAMILY )

#define DEV_FORCE_MAP		( DEV_88E6065_FAMILY )

#define DEV_PORT_SCHEDULE	( DEV_88E6065 )

#define DEV_OUT_Q_SIZE		( DEV_88E6065_FAMILY )

#define DEV_PROVIDER_TAG	( DEV_88E6065_FAMILY )

#define DEV_OLD_HEADER		( DEV_88E6065_FAMILY )

#define DEV_RECURSIVE_TAG_STRIP		( DEV_88E6065_FAMILY )

#define DEV_FORCE_WITH_VALUE			\
					( DEV_88E6181 | DEV_88E6183 | DEV_88E6093 | 	\
					  DEV_88E6095_FAMILY | DEV_88E6185_FAMILY |		\
					  DEV_88E6065_FAMILY )

/* Grouping ATU Entry State for Unicast */

#define DEV_UC_7_DYNAMIC		\
				( DEV_88E6065_FAMILY | DEV_88E6095_FAMILY |	DEV_88E6185_FAMILY |	\
				  DEV_88E6183 | DEV_88E6093 )

#define DEV_UC_NO_PRI_TO_CPU_STATIC_NRL		(DEV_88E6065_FAMILY)
#define DEV_UC_TO_CPU_STATIC_NRL			(DEV_88E6065_FAMILY)
#define DEV_UC_NO_PRI_STATIC_NRL			(DEV_88E6065_FAMILY)
#define DEV_UC_STATIC_NRL					(DEV_88E6065_FAMILY)

#define DEV_UC_NO_PRI_TO_CPU_STATIC			\
				( DEV_88E6065_FAMILY | DEV_88E6095_FAMILY |	DEV_88E6185_FAMILY )
#define DEV_UC_TO_CPU_STATIC			\
				( DEV_88E6065_FAMILY | DEV_88E6095_FAMILY |	DEV_88E6185_FAMILY )

#define DEV_UC_NO_PRI_STATIC			\
				( DEV_88E6065_FAMILY | DEV_88E6095_FAMILY |	DEV_88E6185_FAMILY |	\
				  DEV_88E6183 | DEV_88E6093 )

#define DEV_UC_STATIC	( DEV_STATIC_ADDR )


/* Grouping ATU Entry State for Multicast */

#define DEV_MC_MGM_STATIC_UNLIMITED_RATE		(DEV_88E6065_FAMILY)
#define DEV_MC_PRIO_MGM_STATIC_UNLIMITED_RATE	(DEV_88E6065_FAMILY)

#define DEV_MC_STATIC_UNLIMITED_RATE	( DEV_STATIC_ADDR & ~DEV_88E6052 )

#define DEV_MC_MGM_STATIC		( DEV_STATIC_ADDR & ~DEV_88E6083 )

#define DEV_MC_STATIC				( DEV_STATIC_ADDR )
#define DEV_MC_PRIO_MGM_STATIC		( DEV_STATIC_ADDR )

#define DEV_MC_PRIO_STATIC_UNLIMITED_RATE ( DEV_STATIC_ADDR & ~ (DEV_88E6083|DEV_88E6052) )

#define DEV_MC_PRIO_STATIC		( DEV_STATIC_ADDR & ~DEV_88E6083 )



/* Macros to utilize Device Group */

#define IS_IN_DEV_GROUP(dev,_group) (dev->devName & (_group))

/* need to check port number(_hwPort) later */
#define IS_VALID_API_CALL(dev,_hwPort, _devName)	    	\
	(!(dev->devName & (_devName)) ? GT_NOT_SUPPORTED : GT_OK)

#define DOES_DEVPORT_SUPPORT_PCS(dev, _hwPort)			\
	(!(dev->devName & DEV_GIGABIT_SWITCH) || \
	 (dev->devName & DEV_INTERNAL_GPHY) ||   \
	(!(dev->devName & DEV_EXTERNAL_PHY_ONLY) && (((_hwPort) < 8) || ((_hwPort) > 10)))	\
	? 0 : 1)

#define IS_CONFIGURABLE_PHY(dev,_hwPort)	driverIsPhyAttached(dev,_hwPort)

#define GT_GET_SERDES_PORT(dev,_hwPort)		driverGetSerdesPort(dev,_hwPort)

#define GT_GIG_PHY_INT_MASK(dev,_portVct)    ((_portVct) = (_portVct) & 0xF7)

#define RECOMMENDED_ESB_LIMIT(dev)			16777200
#define RECOMMENDED_CBS_LIMIT(dev)			393216
#define RECOMMENDED_BUCKET_INCREMENT(dev)	174

typedef struct _EXTRA_OP_DATA
{
	GT_U32 moveFrom;
	GT_U32 moveTo;
	GT_U32 intCause;
	GT_U32 reserved;
} GT_EXTRA_OP_DATA;

/*******************************************************************************
* gvtuGetViolation
*
* DESCRIPTION:
*       Get VTU Violation data
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       vtuIntStatus - interrupt cause, source portID, and vid.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORT  - if current device does not support this feature.
*
* COMMENTS:
*		This is an internal function. No user should call this function.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuGetViolation
(
    IN GT_QD_DEV*       dev,
    OUT GT_VTU_INT_STATUS *vtuIntStatus
);

/*******************************************************************************
* gvtuGetViolation2
*
* DESCRIPTION:
*       Get VTU Violation data (for Gigabit Device)
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       vtuIntStatus - interrupt cause, source portID, and vid.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORT  - if current device does not support this feature.
*
* COMMENTS:
*		This is an internal function. No user should call this function.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuGetViolation2
(
    IN GT_QD_DEV*       dev,
    OUT GT_VTU_INT_STATUS *vtuIntStatus
);

/*******************************************************************************
* gvtuGetViolation3
*
* DESCRIPTION:
*       Get VTU Violation data (for Spinnaker family Device)
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       vtuIntStatus - interrupt cause, source portID, and vid.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORT  - if current device does not support this feature.
*
* COMMENTS:
*		This is an internal function. No user should call this function.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuGetViolation3
(
    IN GT_QD_DEV*       dev,
    OUT GT_VTU_INT_STATUS *vtuIntStatus
);

/*******************************************************************************
* gatuGetViolation
*
* DESCRIPTION:
*       Get ATU Violation data
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       atuIntStatus - interrupt cause, source portID, and vid.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORT  - if current device does not support this feature.
*
* COMMENTS:
*		This is an internal function. No user should call this function.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gatuGetViolation
(
    IN  GT_QD_DEV         *dev,
    OUT GT_ATU_INT_STATUS *atuIntStatus
);

/*******************************************************************************
* gsysSetRetransmitMode
*
* DESCRIPTION:
*       This routine set the Retransmit Mode.
*
* INPUTS:
*       en - GT_TRUE Retransimt Mode is enabled, GT_FALSE otherwise.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetRetransmitMode
(
    IN GT_QD_DEV*       dev,
    IN GT_BOOL en
);

/*******************************************************************************
* gsysGetRetransmitMode
*
* DESCRIPTION:
*       This routine get the Retransmit Mode.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE Retransmit Mode is enabled, GT_FALSE otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetRetransmitMode
(
    IN GT_QD_DEV*       dev,
    IN GT_BOOL *en
);

/*******************************************************************************
* gsysSetLimitBackoff
*
* DESCRIPTION:
*       This routine set the Limit Backoff bit.
*
* INPUTS:
*       en - GT_TRUE:  uses QoS half duplex backoff operation  
*            GT_FALSE: uses normal half duplex backoff operation
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetLimitBackoff
(
    IN GT_QD_DEV*       dev,
    IN GT_BOOL en
);

/*******************************************************************************
* gsysGetLimitBackoff
*
* DESCRIPTION:
*       This routine set the Limit Backoff bit.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE:  uses QoS half duplex backoff operation  
*            GT_FALSE: uses normal half duplex backoff operation
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetLimitBackoff
(
    IN GT_QD_DEV*       dev,
    IN GT_BOOL *en
);

/*******************************************************************************
* gsysSetRsvRegPri
*
* DESCRIPTION:
*       This routine set the Reserved Queue's Requesting Priority 
*
* INPUTS:
*       en - GT_TRUE: use the last received frome's priority
*            GT_FALSE:use the last switched frame's priority 
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetRsvReqPri
(
    IN GT_QD_DEV*       dev,
    IN GT_BOOL en
);

/*******************************************************************************
* gsysGetRsvReqPri
*
* DESCRIPTION:
*       This routine get the Reserved Queue's Requesting Priority 
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE: use the last received frome's priority
*            GT_FALSE:use the last switched frame's priority 
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetRsvReqPri
(
    IN GT_QD_DEV*       dev,
    IN GT_BOOL *en
);

/*******************************************************************************
* gsysGetPtrCollision
*
* DESCRIPTION:
*       This routine get the QC Pointer Collision.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE Discard is enabled, GT_FALSE otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
* 		This feature is for both clippership and fullsail
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetPtrCollision
(
    IN GT_QD_DEV*       dev,
    IN GT_BOOL *mode
);

/*******************************************************************************
* gsysGetDpvCorrupt
*
* DESCRIPTION:
*       This routine get the DpvCorrupt bit. This bit is set to a one when the 
*       QC detects a destination vector error
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE: destination vector corrupt, GT_FALSE otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
* 	This feature is on clippership, but not on fullsail
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetDpvCorrupt
(
    IN GT_BOOL *mode
);

/*******************************************************************************
* gsysGetMissingPointers
*
* DESCRIPTION:
*       This routine get the Missing Pointer bit. This bit is set to a one when  
*       the Register File detects less than 64 pointers in the Link List. 
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE: Missing Pointers error, GT_FALSE otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
* 	This feature is on clippership, but not on fullsail
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetMissingPointers
(
    IN GT_QD_DEV*       dev,
    IN GT_BOOL *mode
);

/*******************************************************************************
* gtDbgPrint
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*
* COMMENTS:
*       None
*
*******************************************************************************/
void gtDbgPrint(char* format, ...);


/*******************************************************************************
* gtSemRegister
*
* DESCRIPTION:
*       Assign QuarterDeck Semaphore functions to the given semaphore set.
*		QuarterDeck maintains its own memory for the structure.
*
* INPUTS:
*		semFunctions - point to the GT_SEM_ROUTINES
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gtSemRegister
(
    IN GT_QD_DEV*       dev,
    IN  GT_SEM_ROUTINES* semRoutines
);


/*******************************************************************************
* gpirlInitialize
*
* DESCRIPTION:
*       This routine initializes PIRL Resources.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpirlInitialize
(
    IN  GT_QD_DEV  			*dev
);


/*******************************************************************************
* lport2port
*
* DESCRIPTION:
*       This function converts logical port number to physical port number
*
* INPUTS:
*		portVec - physical port list in vector
*		port    - logical port number
* OUTPUTS:
*		None.
* RETURNS:
*       physical port number
*
* COMMENTS:
*
*******************************************************************************/
GT_U8 lport2port
(
    IN GT_U16    portVec,
	IN GT_LPORT	 port
);

/*******************************************************************************
* port2lport
*
* DESCRIPTION:
*       This function converts physical port number to logical port number
*
* INPUTS:
*		portVec - physical port list in vector
*		port    - logical port number
* OUTPUTS:
*		None.
* RETURNS:
*       physical port number
*
* COMMENTS:
*
*******************************************************************************/
GT_LPORT port2lport
(
    IN GT_U16    portVec,
	IN GT_U8	 hwPort
);

/*******************************************************************************
* lportvec2portvec
*
* DESCRIPTION:
*       This function converts logical port vector to physical port vector
*
* INPUTS:
*		portVec - physical port list in vector
*		lVec 	- logical port vector
* OUTPUTS:
*		None.
* RETURNS:
*       physical port vector
*
* COMMENTS:
*
*******************************************************************************/
GT_U32 lportvec2portvec
(
    IN GT_U16    portVec,
	IN GT_U32	 lVec
);

/*******************************************************************************
* portvec2lportvec
*
* DESCRIPTION:
*       This function converts physical port vector to logical port vector
*
* INPUTS:
*		portVec - physical port list in vector
*		pVec 	- physical port vector
* OUTPUTS:
*		None.
* RETURNS:
*       logical port vector
*
* COMMENTS:
*
*******************************************************************************/
GT_U32 portvec2lportvec
(
    IN GT_U16    portVec,
	IN GT_U32	 pVec
);

#ifdef __cplusplus
}
#endif

#endif /* __msApiInternal_h */

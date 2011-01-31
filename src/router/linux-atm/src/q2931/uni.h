/* uni.h - Various Q.2931/Q.2971/Q.2963.1/UNI 3.x/UNI 4.0 constants */

/* Written 1995-1998 by Werner Almesberger, EPFL-LRC/ICA */

/*
 * Note: some values don't appear in UNI 3.0 or 3.1 but are taken from Q.2931
 *       and related ITU documents.
 */

#ifndef UNI_H
#define UNI_H

/* Maximum message size */

#define MAX_Q_MSG 1000

/* Protocol discriminator */

#define Q2931_PROTO_DSC		9 /* Q.2931 user-network call/connection
				     control message */

/* Coding Standards */

#define Q2931_CS_ITU		0	/* ITU-T standardized */
#define Q2931_CS_NET		3	/* Standard defined for the network */

/* Message types */

#define ATM_MSG_NATIONAL	0x00	/* National specific message escape */
#define ATM_MSG_SETUP		0x05	/* SETUP */
#define ATM_MSG_ALERTING	0x01	/* ALERTING */
#define ATM_MSG_CALL_PROC	0x02	/* CALL PROCEEDING */
#define ATM_MSG_CONNECT		0x07	/* CONNECT */
#define ATM_MSG_CONN_ACK	0x0f	/* CONNECT ACKNOWLEDGE */
#define ATM_MSG_RESTART		0x46	/* RESTART */
#define ATM_MSG_RELEASE		0x4d	/* RELEASE */
#define ATM_MSG_REST_ACK	0x4e	/* RESTART ACKNOWLEDGE */
#define ATM_MSG_REL_COMP	0x5a	/* RELEASE COMPLETE */
#define ATM_MSG_NOTIFY		0x6e	/* NOTIFY */
#define ATM_MSG_STATUS_ENQ	0x75	/* STATUS ENQUIRY */
#define ATM_MSG_STATUS		0x7d	/* STATUS */
#define ATM_MSG_ADD_PARTY	0x80	/* ADD PARTY */
#define ATM_MSG_ADD_PARTY_ACK	0x81	/* ADD PARTY ACKNOWLEDGE */
#define ATM_MSG_ADD_PARTY_REJ	0x82	/* ADD PART REJECT */
#define ATM_MSG_PARTY_ALERT	0x85	/* PARTY ALERTING */
#define ATM_MSG_DROP_PARTY	0x83	/* DROP PARTY */
#define ATM_MSG_DROP_PARTY_ACK	0x84	/* DROP PARTY ACKNOWLEDGE */
#define ATM_MSG_MODIFY_REQ	0x88	/* MODIFY REQUEST */
#define ATM_MSG_MODIFY_ACK	0x89	/* MODIFY ACKNOWLEDGE */
#define ATM_MSG_MODIFY_REJ	0x8a	/* MODIFY REJECT */
#define ATM_MSG_CONN_AVAIL	0x8b	/* CONNECTION AVAILABLE */
#define ATM_MSG_LEAF_FAILURE	0x90	/* LEAF SETUP FAILURE */
#define ATM_MSG_LEAF_REQUEST	0x91	/* LEAF SETUP REQUEST */
#define ATM_MSG_RESERVED	0xff	/* reserved for EVEN MORE msg types */

/* Information element identifiers */

#define ATM_IE_CAUSE		0x08 /* Cause */
#define ATM_IE_CALL_STATE	0x14 /* Call state */
#define ATM_IE_NOTIFY		0x27 /* Notification indicator */
#define ATM_IE_E2E_TDL		0x42 /* End-to-end transit delay */
#define ATM_IE_EPR		0x54 /* Endpoint reference */
#define ATM_IE_EP_STATE		0x55 /* Endpoint state */
#define ATM_IE_AAL		0x58 /* ATM adaption layer parameters */
#define ATM_IE_TD		0x59 /* ATM traffic descriptor */
#define ATM_IE_CONN_ID		0x5a /* Connection identifier */
#define ATM_IE_OAM_TD		0x5b /* OAM traffic descriptor */
#define ATM_IE_QOS		0x5c /* Quality of service parameter */
#define ATM_IE_BHLI		0x5d /* Broadband high layer information */
#define ATM_IE_BBCAP		0x5e /* Broadband bearer capability */
#define ATM_IE_BLLI		0x5f /* Broadband low-layer information */
#define ATM_IE_BBS_COMP		0x62 /* Broadband sending complete */
#define ATM_IE_BBREP		0x63 /* Broadband repeat indicator */
#define ATM_IE_CGPN		0x6c /* Calling party number */
#define ATM_IE_CGPS		0x6d /* Calling party subaddress */
#define ATM_IE_CDPN		0x70 /* Called party number */
#define ATM_IE_CDPS		0x71 /* Called party subaddress */
#define ATM_IE_TNS		0x78 /* Transit network selection */
#define ATM_IE_RESTART		0x79 /* Restart indicator */
#define ATM_IE_GIT		0x7f /* Generic identifier transport */
#define ATM_IE_ALT_TD		0x81 /* Alternate ATM traffic descriptor */
#define ATM_IE_MIN_TD		0x80 /* Minimum acceptable ATM traffic desc. */
#define ATM_IE_ABR_SET_PRM	0x84 /* ABR setup parameters */
#define ATM_IE_BBRT		0x89 /* Broadband report type */
#define ATM_IE_ABR_ADD_PRM	0xe4 /* ABR additional parameters */
#define ATM_IE_LIJ_ID		0xe8 /* Leaf initiated join call identifer */
#define ATM_IE_LIJ_PRM		0xe9 /* Leaf initiated join parameters */
#define ATM_IE_LEAF_SN		0xea /* Leaf sequence number */
#define ATM_IE_SCOPE_SEL	0xeb /* Connection Scope Selection */
#define ATM_IE_EQOS		0xec /* Extended QOS parameters */

/* Cause: Location */

#define ATM_LOC_USER		  0 /* user */
#define ATM_LOC_PRV_LOC		  1 /* private network serving the local user */
#define ATM_LOC_PUB_LOC		  2 /* public network serving the local user */
#define ATM_LOC_TRANS_NET	  3 /* transit network */
#define ATM_LOC_PRV_RMT		  4 /* public network serving the remote user */
#define ATM_LOC_PUB_RMT		  5 /* private network serving the remote
				       user */
#define ATM_LOC_INT_NET		  7 /* international network */
#define ATM_LOC_BEYOND_IWP	 10 /* network beyond interworking point */

/* Cause: Cause values */

/* ----------------------------------- normal event */
#define ATM_CV_UNALLOC		  1 /* unallocated (unassigned) number */
#define ATM_CV_NO_ROUTE_TNS	  2 /* no route to specified transit network */
#define ATM_CV_NO_ROUTE_DEST	  3 /* no route to destination */
#if defined(UNI30) || defined(DYNAMIC_UNI)
#define ATM_CV_CI_UNACC		 10 /* VPCI/VCI unacceptable */
#endif
#if defined(UNI31) || defined(UNI40) || defined(DYNAMIC_UNI)
#define ATM_CV_NORMAL_CLEAR	 16 /* normal call clearing */
#endif
#define ATM_CV_USER_BUSY	 17 /* user busy */
#define ATM_CV_NO_USER_RESP	 18 /* no user responding */
#define ATM_CV_CALL_REJ		 21 /* call rejected */
#define ATM_CV_NUM_CHANGED	 22 /* number changed */
#define ATM_CV_REJ_CLIR		 23 /* user rejects all calls with calling
				       line identification restriction (CLIR)*/
#define ATM_CV_DEST_OOO		 27 /* destination out of order */
#define ATM_CV_INV_NUM_FMT	 28 /* invalid number format (address
				        incomplete) */
#define ATM_CV_RESP_STAT_ENQ	 30 /* response to STATUS ENQUIRY */
#define ATM_CV_NORMAL_UNSPEC	 31 /* normal, unspecified */
/* ----------------------------------- resource unavailable */
#define ATM_CV_CI_UNAVAIL	 35 /* requested VPCI/VCI unavailable */
#if defined(UNI31) || defined(UNI40) || defined(DYNAMIC_UNI)
#define ATM_CV_CI_FAIL		 36 /* VPCI/VCI assignment failure */
#define ATM_CV_UCR_UNAVAIL_NEW	 37 /* user cell rate not available (>3.0)*/
#endif
#define ATM_CV_NET_OOO		 38 /* network out of order - unused */
#define ATM_CV_TEMP_FAIL	 41 /* temporary failure */
#define ATM_CV_ACC_INF_DISC	 43 /* access information discarded */
#define ATM_CV_NO_CI		 45 /* no VPCI/VCI available */
#define ATM_CV_RES_UNAVAIL	 47 /* resource unavailable, unspecified */
/* ----------------------------------- service or option not available */
#define ATM_CV_QOS_UNAVAIL	 49 /* Quality of Service unavailable */
#if defined(UNI30) || defined(ALLOW_UNI30) || defined(DYNAMIC_UNI)
#define ATM_CV_UCR_UNAVAIL_OLD	 51 /* user cell rate not available (3.0) */
#endif
#define ATM_CV_BBCAP_NOT_AUTH	 57 /* bearer capability not authorized */
#define ATM_CV_BBCAP_UNAVAIL	 58 /* bearer capability not presently
				       available */
#define ATM_CV_UNAVAILABLE	 63 /* service or option not available,
				       unspecified */
/* ----------------------------------- service or option not implemented */
#define ATM_CV_BBCAP_NOT_IMPL	 65 /* bearer capability not implemented */
#define ATM_CV_UNSUPP_TRAF_PRM	 73 /* unsupported combination of traffic
				       parameters */
#if defined(UNI31) || defined(UNI40) || defined(DYNAMIC_UNI)
#define ATM_CV_AAL_UNSUPP_NEW	 78 /* AAL param. cannot be supported (>3.0) */
#endif
/* ----------------------------------- invalid message */
#define ATM_CV_INV_CR		 81 /* invalid call reference value */
#define ATM_CV_NO_SUCH_CHAN	 82 /* identified channel does not exist */
#define ATM_CV_INCOMP_DEST	 88 /* incompatible destination */
#define ATM_CV_INV_EPR		 89 /* invalid endpoint reference */
#define ATM_CV_INV_TNS		 91 /* invalid transit network selection */
#define ATM_CV_TOO_MANY_APR	 92 /* too many pending add party requests */
#if defined(UNI30) || defined(DYNAMIC_UNI)
#define ATM_CV_AAL_UNSUPP_OLD	 93 /* AAL param. cannot be supported (3.0) */
#endif
/* ----------------------------------- protocol error */
#define ATM_CV_MAND_IE_MISSING	 96 /* mandatory information element is
				       missing */
#define ATM_CV_UNKNOWN_MSG_TYPE  97 /* message type non-existent or not
				       implemented */
#define ATM_CV_UNKNOWN_IE	 99 /* information element non-existent or not
				       implemented */
#define ATM_CV_INVALID_IE	100 /* invalid information element contents */
#define ATM_CV_INCOMP_MSG	101 /* message not compatible with call state*/
#define ATM_CV_TIMER_EXP	102 /* recovery on timer expiry */
#define ATM_CV_BAD_MSG_LEN	104 /* incorrect message length */
#define ATM_CV_PROTOCOL_ERROR	111 /* protocol error, unspecified */

/* Cause: P-U values */

#define ATM_PU_PROVIDER		0   /* Network service - Provider */
#define ATM_PU_USER		1   /* Network service - User */

/* Cause: N-A values */

#define ATM_NA_NORMAL		0   /* Normal */
#define ATM_NA_ABNORMAL		1   /* Abnormal */

/* Cause: Condition */

#define ATM_COND_UNKNOWN	0 /* Unknown */
#define ATM_COND_PERMANENT	1 /* Permanent */
#define ATM_COND_TRANSIENT	2 /* Transient */

/* Cause: Reject reason */

#define ATM_RSN_USER		0 /* User specific */
#define ATM_RSN_IE_MISS		1 /* Information element missing */
#define ATM_RSN_IE_INSUFF	2 /* Information element contents are not
				     sufficient */
/* Restart Indicator class values */

#define ATM_RST_IND_VC		0 /* Indicated virtual channel */
#define ATM_RST_ALL_VC		2 /* All virtual channels controlled by the
				     Layer 3 entity which sends the RESTART
				     message */

/* Action Indicator for messages */

#define ATM_AI_MSG_CLEAR	0 /* clear call */
#define ATM_AI_MSG_DSC_IGN	1 /* discard and ignore */
#define ATM_AI_MSG_DSC_STAT	2 /* discard and report status */
#define ATM_AI_MSG_RSV		3 /* reserved */

/* Action Indicator for IEs */

#define ATM_AI_IE_CLEAR		0 /* clear call */
#define ATM_AI_IE_DSCIE_PRC	1 /* discard IE and proceed */
#define ATM_AI_IE_DSCIE_STAT	2 /* discard IE, procees, and report status */
#define ATM_AI_IE_DSCMSG_IGN	5 /* discard message, and ignore */
#define ATM_AI_IE_DSCMSG_STAT	6 /* discard message, and report status */

/* Type of number */

#define ATM_TON_UNKNOWN		0 /* unknown */
#define ATM_TON_INTRNTNL	1 /* international number */
#define ATM_TON_NATIONAL	2 /* national number */
#define ATM_TON_NETWORK		3 /* network specific number */
#define ATM_TON_SUBSCRIBER	4 /* subscriber number */
#define ATM_TON_ABBRV		6 /* abbreviated number */

/* Numbering/addressing plan */

#define ATM_NP_UNKNOWN		0 /* unknown */
#define ATM_NP_E164		1 /* ISDN numbering plan (E.164) */
#define ATM_NP_AEA		2 /* ATM endsystem address */
#define ATM_NP_PRIVATE		9 /* private numbering plan */

/* Type of sub-address */

#define ATM_SAT_NSAP		0 /* NSAP (Rec. X.213 ISO/IEC 8348) */
#define ATM_SAT_AEA		1 /* ATM endsystem address */
#define ATM_SAT_USER		2 /* user-specified */

/* Presentation indicator */

#define ATM_PRS_ALLOW		0 /* presentation allowed */
#define ATM_PRS_RESTRICT	1 /* presentation restricted */
#define ATM_PRS_NOTAVL		2 /* number not available */

/* Screening indicator */

#define ATM_SCRN_UP_NS		0 /* user-provided, not screened */
#define ATM_SCRN_UP_VP		1 /* user-provided, verified and passed */
#define ATM_SCRN_UP_VF		2 /* user-provided, verified and failed */
#define ATM_SCRN_NP		3 /* network provided */

/* VP-associated signalling */

#define ATM_VPA_VPA		0 /* VP-associated signalling */
#define ATM_VPA_EXPL		1 /* explicit indication of VPCI */

/* Preferred/exclusive */

#define ATM_POE_EXC_EXC		0 /* exclusive VPCI; exclusive VCI */
#define ATM_POE_EXC_ANY		1 /* exclusive VPCI; any VCI */
#if defined(UNI40) || defined(DYNAMIC_UNI)
#define ATM_POE_EXC_NO		2 /* exclusive VPCI; no VCI (used for VPCs) */
#endif

/* Traffic descriptor tags */

#define ATM_TD_FW_PCR_0		0x82 /* Forward peak cell rate (CLP=0) */
#define ATM_TD_BW_PCR_0		0x83 /* Backward peak cell rate (CLP=0) */
#define ATM_TD_FW_PCR_01	0x84 /* Forward peak cell rate (CLP=0+1) */
#define ATM_TD_BW_PCR_01	0x85 /* Backward peak cell rate (CLP=0+1) */
#define ATM_TD_FW_SCR_0		0x88 /* Forward sustained cell rate (CLP=0) */
#define ATM_TD_BW_SCR_0		0x89 /* Backward sustained cell rate (CLP=0) */
#define ATM_TD_FW_SCR_01	0x90 /* Forward sustained cell rate (CLP=0+1) */
#define ATM_TD_BW_SCR_01	0x91 /* Backward sustained cell rate (CLP=0+1)*/
#define ATM_TD_FW_MCR_01	0x92 /* Forward ABR min. cell rate (CLP=0+1) */
#define ATM_TD_BW_MCR_01	0x93 /* Backward ABR min. cell rate (CLP=0+1) */
#define ATM_TD_FW_MBS_0		0xa0 /* Forward maximum burst size (CLP=0) */
#define ATM_TD_BW_MBS_0		0xa1 /* Backward maximum  burst size (CLP=0) */
#define ATM_TD_FW_MBS_01	0xb0 /* Forward maximum burst size (CLP=0+1) */
#define ATM_TD_BW_MBS_01	0xb1 /* Backward maximum burst size (CLP=0+1) */
#define ATM_TD_BEST_EFFORT	0xbe /* Best effort indicator */
#define ATM_TD_TM_OPT		0xbf /* Traffic management options */

/* Frame discard forward/backward */

#define ATM_FD_NO		0 /* No Frame discard allowed */
#define ATM_FD_YES		1 /* Frame discard allowed */

/* Tagging forward/backward */

#define ATM_TAG_NO		0 /* Tagging not requested */
#define ATM_TAG_YES		1 /* Tagging requested */

/* Bearer class */

#define ATM_BC_BCOB_A		1 /* BCOB-A */
#define ATM_BC_BCOB_C		3 /* BCOB-C */
#define ATM_BC_BCOB_X		16 /* BCOB-X */
#define ATM_BC_TVP		24 /* Transparent VP service */

/* ATM Transfer Capability */

#define ATM_TC_CBR		0x05 /* CBR */
#define ATM_TC_CBR_CLR		0x07 /* CBR with CLR commitment on CLP=0+1 */
#define ATM_TC_VBR_RT		0x09 /* Real time VBR */
#define ATM_TC_VBR_RT_CLR	0x13 /* Real time VBR w/ CLR comm. on CLP=0+1 */
#define ATM_TC_VBR_NRT		0x0a /* Non-real time VBR */
#define ATM_TC_VBR_NRT_CLR	0x0b /* Non-real time VBR w/ CLR com. CLP=0+1 */
#define ATM_TC_ABR		0x0c /* ABR */

#define ATM_TC_VBR_NRT_R00	0x00 /* Non-real time VBR (reception only) */
#define ATM_TC_VBR_RT_R01	0x01 /* Real time VBR (reception only) */
#define ATM_TC_VBR_NRT_R02	0x02 /* Non-real time VBR (reception only) */
#define ATM_TC_CBR_R04		0x04 /* CBR (reception only) */
#define ATM_TC_CBR_R06		0x06 /* CBR (reception only) */
#define ATM_TC_VBR_NRT_R08	0x08 /* Non-real time VBR (reception only) */

#define ATM_TC_RSV_20		0x20 /* Reserved for backward compatibility */
#define ATM_TC_RSV_21		0x21 /* Reserved for backward compatibility */
#define ATM_TC_RSV_22		0x22 /* Reserved for backward compatibility */
#define ATM_TC_RSV_24		0x24 /* Reserved for backward compatibility */
#define ATM_TC_RSV_25		0x25 /* Reserved for backward compatibility */
#define ATM_TC_RSV_26		0x26 /* Reserved for backward compatibility */
#define ATM_TC_RSV_28		0x28 /* Reserved for backward compatibility */
#define ATM_TC_RSV_29		0x29 /* Reserved for backward compatibility */
#define ATM_TC_RSV_2A		0x2a /* Reserved for backward compatibility */

#define ATM_TC_RSV_40		0x40 /* Reserved for backward compatibility */
#define ATM_TC_RSV_41		0x41 /* Reserved for backward compatibility */
#define ATM_TC_RSV_42		0x42 /* Reserved for backward compatibility */
#define ATM_TC_RSV_44		0x44 /* Reserved for backward compatibility */
#define ATM_TC_RSV_45		0x45 /* Reserved for backward compatibility */
#define ATM_TC_RSV_46		0x46 /* Reserved for backward compatibility */
#define ATM_TC_RSV_48		0x48 /* Reserved for backward compatibility */
#define ATM_TC_RSV_49		0x49 /* Reserved for backward compatibility */
#define ATM_TC_RSV_4A		0x4a /* Reserved for backward compatibility */

#define ATM_TC_RSV_60		0x60 /* Reserved for backward compatibility */
#define ATM_TC_RSV_61		0x61 /* Reserved for backward compatibility */
#define ATM_TC_RSV_62		0x62 /* Reserved for backward compatibility */
#define ATM_TC_RSV_64		0x64 /* Reserved for backward compatibility */
#define ATM_TC_RSV_65		0x65 /* Reserved for backward compatibility */
#define ATM_TC_RSV_66		0x66 /* Reserved for backward compatibility */
#define ATM_TC_RSV_68		0x68 /* Reserved for backward compatibility */
#define ATM_TC_RSV_69		0x69 /* Reserved for backward compatibility */
#define ATM_TC_RSV_6A		0x6a /* Reserved for backward compatibility */


#ifdef OBSOLETE_DEFINITIONS_FOLLOW

/* Traffic type */

	ATM_TT_NO_IND		0 /* no indication */
	ATM_TT_CBR		1 /* constant bit rate */
	ATM_TT_VBR		2 /* variable bit rate */

/* Timing requirements */

	ATM_TR_NO_IND		0 /* no indication */
	ATM_TR_E2E_REQ		1 /* end-to-end timing required */
	ATM_TR_E2E_NRQ		2 /* end-to-end timing not required */

#endif


/* Susceptibility to clipping */

#define ATM_STC_NO		0 /* not susceptible to clipping */
#define ATM_STC_YES		1 /* susceptible to clipping */

/* User-plane connection configuration */

#define ATM_UPCC_P2P		0 /* point-to-point */
#define ATM_UPCC_P2M		1 /* point-to-multipoint */

/* Instruction field flags */

#define ATM_FLAG_NO		0 /* instruction field not significant */
#define ATM_FLAG_YES		1 /* follow explicit instructions */

/* AAL parameter tags */

#define ATM_AALP_FW_MAX_SDU	0x8c /* Forward maximum CPCS-SDU size */
#define ATM_AALP_BW_MAX_SDU	0x81 /* Backward maximum CPCS-SDU size */
#define ATM_AALP_AAL_MODE	0x83 /* AAL mode (UNI 3.0 only) */
#define ATM_AALP_SSCS		0x84 /* SSCS type */

/* Transit delay identifiers */

#define ATM_TDL_CUM		0x01 /* Cumulative transit delay value */
#define ATM_TDL_E2EMAX		0x03 /* Maximum end-to-end transit delay value*/
#define ATM_TDL_NGI		0x0a /* Network generated indicator */

/* Transit network identification */

#define ATM_TNI_USER		0x00 /* User-specified */
#define ATM_TNI_NNI		0x02 /* National network identification */
#define ATM_TNI_INI		0x04 /* International network identification */

/* Network identification plan */

#define ATM_NIP_UNKNOWN		0x00 /* Unknown */
#define ATM_NIP_CARRIER		0x01 /* Carrier identification code */
#define ATM_NIP_DATA		0x03 /* Data network id. code (X.121) */

/* Shaping indicator */

#define ATM_SHI_NONE		0x00 /* No user specified requirement */
#define ATM_SHI_NOAGG		0x01 /* Aggr. shaping of user & OAM not all. */

/* Compliance indicator */

#define ATM_OCI_OPT		0x00 /* Use of e2e OAM F5 flow is optional */
#define ATM_OCI_MAND		0x01 /* Use of e2e OAM F5 flow is mandatory */

/* User-network fault management indicator */

#define ATM_UNFM_NONE		0x00 /* No user-orig. fault mg. indications */
#define ATM_UNFM_1CPS		0x01 /* Use of u-o fm. ind. w/ rate 1 cps */

/* End-to-end OAM F5 flow indicator */

#define ATM_OFI_0_0		0x00 /* 0% of cell rate (CLP=0+1) in ATM TD */
#define ATM_OFI_0_1		0x01 /* 0.1% of cell rate (CLP=0+1) in ATM TD */
#define ATM_OFI_1_0		0x04 /* 1% of cell rate (CLP=0+1) in ATM TD */

/* Identifier related standard/application */

#define ATM_IRS_DSMCC		0x01 /* DSM-CC ISO/IEC 13818-6 */
#define ATM_IRS_H245		0x02 /* Recommendation H.245 */

/* Identifier type */

#define ATM_IT_SESSION		0x01 /* Session */
#define ATM_IT_RESOURCE		0x02 /* Resource */

/* Leaf call identifier type */

#define ATM_LIT_ROOT		0x00 /* Root assigned */

/* (LIJ) screening indication */

#define ATM_LSI_NJ_NR		0x00 /* Network Join Without Root Notif. */

/* Type of connection scope */

#define ATM_TCS_ORGANIZATIONAL	0x00 /* Organizational */

/* Connection scope selection */

#define ATM_CSS_RSV_00		0x00 /* Reserved */
#define ATM_CSS_LOCAL		0x01 /* Local network */
#define ATM_CSS_LOCAL_P1	0x02 /* Local network plus one */
#define ATM_CSS_LOCAL_P2	0x03 /* Local network plus two */
#define ATM_CSS_SITE_M1		0x04 /* Site minus one */
#define ATM_CSS_SITE		0x05 /* Intra-site */
#define ATM_CSS_SITE_P1		0x06 /* Site plus one */
#define ATM_CSS_ORG_M1		0x07 /* Organization minus one */
#define ATM_CSS_ORG		0x08 /* Intra-organization */
#define ATM_CSS_ORG_P1		0x09 /* Organization plus one */
#define ATM_CSS_COM_M1		0x0a /* Community minus one */
#define ATM_CSS_COM		0x0b /* Intra-community */
#define ATM_CSS_COM_P1		0x0c /* Community plus one */
#define ATM_CSS_REG		0x0d /* Regional */
#define ATM_CSS_INTER		0x0e /* Inter-regional */
#define ATM_CSS_GLOBAL		0x0f /* Global */

/* Origin (of extended QOS) */

#define ATM_EQO_USER		0x00 /* Originating user */
#define ATM_EQO_NET		0x01 /* Intermediate network */

/* Extended QOS parameters */

#define ATM_EQP_ACC_FW_CDV	0x94 /* Acceptable fwd peak-to-peak CDV */
#define ATM_EQP_ACC_BW_CDV	0x95 /* Acceptable bwd peak-to-peak CDV */
#define ATM_EQP_CUM_FW_CDV	0x96 /* Cumulative fwd peak-to-peak CDV */
#define ATM_EQP_CUM_BW_CDV	0x97 /* Cumulative bwd peak-to-peak CDV */
#define ATM_EQP_ACC_FW_CLR	0xa2 /* Acceptable fwd cell loss ratio */
#define ATM_EQP_ACC_BW_CLR	0xa3 /* Acceptable bwd cell loss ratio */

/* ABR additional parameters */

#define ATM_AAP_FW_REC		0xc2 /* Forward additional parameters record */
#define ATM_AAP_BW_REC		0xc3 /* Backward additional parameters record */

/* ABR setup parameters */

#define ATM_ASP_FW_ICR		0xc2 /* Forward ABR initial cell rate, CLP01 */
#define ATM_ASP_BW_ICR		0xc3 /* Backward ABR initial cell rate, CLP01 */
#define ATM_ASP_FW_TBE		0xc4 /* Fwd ABR transient buffer exposure */
#define ATM_ASP_BW_TBE		0xc5 /* Bwd ABR transient buffer exposure */
#define ATM_ASP_CRF_RTT		0xc6 /* Cumulative RM fixed round trip time */
#define ATM_ASP_FW_RIF		0xc8 /* Forward rate increase factor */
#define ATM_ASP_BW_RIF		0xc9 /* Backward rate increase factor */
#define ATM_ASP_FW_RDF		0xca /* Forward rate decrease factor */
#define ATM_ASP_BW_RDF		0xcb /* Backward rate decrease factor */

/* Type of report (Q.2963.1) */

#define ATM_TOR_MOD_CONF	0x01 /* Modification confirmation */

/* The following constants tag message parser errors. */

#define RECOV_IND_IE		1	/* IE problem */

/* The following constants tag application-specific errors. */

#define RECOV_ASE_UNKNOWN_IE	1	/* unknown IE */

#endif

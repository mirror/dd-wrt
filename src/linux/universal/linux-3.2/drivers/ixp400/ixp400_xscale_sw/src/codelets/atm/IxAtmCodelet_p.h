/**
 * @file    IxAtmCodelet_p.h
 *
 * @date    31-May-2002
 *
 * @brief   Private header file for the IXP400 Atm codelet
 * 
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

#ifndef IXATMCODELET_P_H
#define IXATMCODELET_P_H

/*
 * System defined includes
 */

/*
 * User defined includes
 */
#include "IxOsal.h"
#include "IxAtmTypes.h"
#include "IxAtmdAcc.h"
#include "IxAtmdAcc.h"
#include "IxAtmm.h"
#include "IxNpeDl.h"

/*
 * #defines and macros used in this file.
 */

/**
 * @brief Mbuf cluster block size 
 */
#define IX_ATMCODELET_MBUF_SIZE               (2000)  

/**
 * @brief Notification trigger threshold value 
 */
#define IX_ATMCODELET_RX_FREE_Q_LOW_THRESHOLD (8) 

/**
 * @brief Default Cell Loss Priority value for ATM cell 
 */
#define IX_ATMCODELET_DEFAULT_CLP             (0)

/**
 * @brief ATM Codelet log
 */
#define IX_ATMCODELET_LOG printf

/**
 * @brief VCI values 
 */
#define IX_OAM_ITU610_F4_SEG_VCI     3
#define IX_OAM_ITU610_F4_ETE_VCI     4

/**
 * @brief PTI values 
 */
#define IX_OAM_ITU610_F5_SEG_PTI                    4
#define IX_OAM_ITU610_F5_ETE_PTI                    5

/**
 * @brief  Length of correlation tag bytes in LB cell 
 */
#define IX_OAM_ITU610_LB_CORRELATION_TAG_LEN 4

/**
 * @brief Size of all location ID attributes in bytes. 
 */
#define IX_OAM_ITU610_LOCATION_ID_LEN 16

/**
 * @brief Reserved bytes in OAM cells 
 */
#define IX_OAM_ITU610_RESERVED_BYTE_VALUE         0x6a
#define IX_OAM_ITU610_GENERIC_PAYLOAD_RESERVED_BYTES_LEN   45
#define IX_OAM_ITU610_LB_RESERVED_BYTES_LEN                 8

/**
 * @brief Reserved and CRC10 length 
 */
#define IX_OAM_ITU610_RESERVED_AND_CRC10_LEN 2

/** 
 * @brief Loopback timeout 
 */
#define IX_OAM_ITU610_LB_TIMEOUT_PERIOD_MSECS   5000

/**
 * @brief OAM Function Types
 */
#define IX_OAM_ITU610_TYPE_FAULT_MAN_LB     0x18

/**
 * @brief Values for Loopback Indication field 
 */
#define IX_OAM_ITU610_LB_INDICATION_PARENT  1
#define IX_OAM_ITU610_LB_INDICATION_CHILD   0

#define IX_ATMCODELET_8VCS    8
#define IX_ATMCODELET_16VCS   16

/*
 * Typedefs
 */

/**
 * @brief atm cell header
 */
typedef UINT32 atmCellHeader;

/**
 * @brief Codelet statistics
 */
typedef struct
{
    UINT32 txPdus;
    UINT32 txBytes;
    UINT32 rxPdus;
    UINT32 rxBytes;
    UINT32 txDonePdus;
    UINT32 rxFreeBuffers;
    UINT32 txPdusSubmitFail;
    UINT32 txPdusSubmitBusy;
    UINT32 rxPdusInvalid;
} IxAtmCodeletStats;

/**
 * @brief Mode of operation of the Atmd Access Codelet.
 */
typedef enum
{
    /**< All cells sent out are looped back in by the UTOPIA hardware.
     * Useful for demonstrating traffic on a board with no ADSL PHY or
     * external test equipment. Setup 32 UBR VCs. */
    IX_ATMCODELET_UTOPIA_LOOPBACK = 0, 

    /**< All cells sent out are looped back in by the UTOPIA hardware. Setup only
     * 8VBR, 8CBR and 16UBR VCs */
    IX_ATMCODELET_UTOPIA_LOOPBACK_8VBR_8CBR_16UBR, 

    /**< All cells received are looped back out again by the software.
     * Setup 32 UBR VCs*/
    IX_ATMCODELET_SOFTWARE_LOOPBACK,   

    /**< All cells received are looped back out again by the software.
     * Setup 8VBR, 8 CBR and 16 UBR VCs*/
    IX_ATMCODELET_SOFTWARE_LOOPBACK_8VBR_8CBR_16UBR,   
   
    /**< All cells sent are anticipated to  be loopbed back by external
     * equipment. Setup 32 UBRs */
    IX_ATMCODELET_REMOTE_LOOPBACK, 

    /**< All cells sent are anticipated to  be loopbed back by external
     * equipment. Setup 8VBR, 8CBR, 16UBR VCs */
    IX_ATMCODELET_REMOTE_LOOPBACK_8VBR_8CBR_16UBR, 

    /**< Mode type for OAM Utopia Loopback ping */
    IX_ATMCODELET_UTOPIA_LOOPBACK_PERFORM_OAM_PING_F4F5,
				
    /**< Mode type for OAM Software Loopback Ping */
    IX_ATMCODELET_SOFTWARE_LOOPBACK_PERFORM_OAM_PING_F4F5, 
    
    /**< Invalid mode */
    IX_ATMCODELET_INVALID_MODE
} IxAtmCodeletMode;

/**
 * @brief The type of AAL that the user is allowed to use
 */
typedef enum
{
    ixAtmCodeletAalTypeInvalid = 0, /**< Invalid AAL*/
    ixAtmCodeletAalType5,           /**< AAL 5*/
    ixAtmCodeletAalType0_48,        /**< AAL 0 48bytes */
    ixAtmCodeletAalType0_52,        /**< AAL0 52bytes */
    ixAtmCodeletAalTypeMax          /**< Max AAL type that is available here*/ 
} IxAtmCodeletAalType; 

/**
 * @brief Oam cells payload typedefs
 */
typedef struct
{
    UINT8 oamTypeAndFunction;
    UINT8 loopbackIndication;
    UINT8 correlationTag[IX_OAM_ITU610_LB_CORRELATION_TAG_LEN];
    UINT8 llid[IX_OAM_ITU610_LOCATION_ID_LEN];
    UINT8 sourceId[IX_OAM_ITU610_LOCATION_ID_LEN];
    UINT8 reserved[IX_OAM_ITU610_LB_RESERVED_BYTES_LEN];
    UINT8 reservedAndCrc10[IX_OAM_ITU610_RESERVED_AND_CRC10_LEN];
} IxOamITU610LbPayload;

/**
 * @brief Generic payload isn't a real payload but is used
 * for checking which payload type a received OAM cell is
 */
typedef struct
{
    UINT8 oamTypeAndFunction;    
    UINT8 reserved[IX_OAM_ITU610_GENERIC_PAYLOAD_RESERVED_BYTES_LEN];
    UINT8 reservedAndCrc10[IX_OAM_ITU610_RESERVED_AND_CRC10_LEN];
} IxOamITU610GenericPayload;

/**
 * @brief OAM ITU610 Payload
 */
typedef union
{
    IxOamITU610LbPayload lbPayload;
    IxOamITU610GenericPayload genericPayload;
    /* Other payload types could be defined here, e.g. Fault Management etc.*/
} IxOamITU610Payload;

/**
 * @brief OAM ITU610 Cell
 */
typedef struct
{
    atmCellHeader header;
    IxOamITU610Payload payload;
} IxOamITU610Cell;

/*
 * Function declarations
 */

/** 
 * @brief This function is used to initialize the lower level system component
 *         used by the IxAtmCodelet. It should be called before any other
 *         IxAtmCodelet API function UNLESS the system initialisation is done
 *         elsewhere. 
 * 
 * This function initializes QMgr using ixQMgrInit() function in order to 
 * provide interfaces for configuring and accessing the IXP4XX AQM hardware 
 * queues to facilitate communication of data between the NPEs and the XScale.
 *
 * It also sets up how the QMgr is being called i.e. interrupt or poll mode. 
 * In addition, this function initializes the NPE Message Handler 
 * (ixNpeMhInitialize()), download NPE image (ixAtmUtilsAtmImageDownload()), 
 * and starts the NPE (ixNpeDlNpeExecutionStart()).
 *
 * @param "UINT32 numPorts" The number of ATM ports to configure.
 *         Valid values range from 1 to 12.
 *
 * @param "IxAtmCodeletMode mode" This indicates the codelet mode.
 *
 * @return
 * IX_SUCCESS Indicates that the Atm Codelet system init successfull.<br>
 * IX_FAIL Some internal error has prevented the Atm Codelet
 *          sstem components from initialising.
 */
PUBLIC IX_STATUS
ixAtmCodeletSystemInit (UINT32 numPorts,
			IxAtmCodeletMode mode);

/** 
 * @brief This function is used to initialize the IxAtmCodelet. It
 *         should be called before any other IxAtmCodelet API function.
 *         This function can be called from the VxWorks* windSh.
 *
 * This function does the following sequence:<br>
 * a) Enabled up either UTOPIA, Software or Remote loopback mode
 *
 * b) Sets up the UTOPIA Phy port addresss for each port. These addresses 
 *    need to be defined as per the Physical interface used.
 *
 * c) Initialize the following component:
 *    - ATM Scheduler - ixAtmSchInit()
 *    - ATM Driver Access component - ixAtmdAccInit()
 *    - ATM Mgmt for UTOPIA - ixAtmmUtopiaInit()
 *    - MBuf Pool through the IxAtmUtils subcomponent- ixAtmUtilsMbufPoolInit()
 *    - The ports (ixAtmmPortInitialize)
 *
 * d) Enable the ports using ixAtmmPortEnable()
 *
 * e) Depending on the Loopback mode, this function will initialize
 *    - IxAtmCodeletRxTx subcomponent by using ixAtmTxRxInit() function <br>
 *        IF UTOPIA_LOOPBACK or REMOTE_LOOPBACK mode is set
 *    - IxAtmCodeletSwLoopback subcomponent by using ixAtmSwLbInit() function <br>
 *	  IF SOFTWARE_LOOPBACK mode is set
 * f) Initialize OAM Codelet using ixOamCodeletInit()
 *
 * g) Sets the initialized flag - i.e. ixAtmCodeletInitialized = TRUE
 *
 * @param "UINT32 numPorts" The number of ATM ports to configure.
 *         Valid values range from 1 to 12.
 *
 * @param "UINT32 rxToTxRatio" Ratio of PDUs received to attempted PDU sends
 *        when softwareLoopbackEnabled := TRUE. e.g. if rxToTxRatio is 6 then 
 *        1 PDU will be sent for every 6 PDUs received.
 *
 * @return
 * IX_SUCCESS Indicates tha the Atm Codelet has been successfully 
 *          initialized. <br>
 * IX_FAIL Some internal error has prevented the Atm Codelet
 *          from initialising.
 */
PUBLIC IX_STATUS
ixAtmCodeletInit (UINT32 numPorts, 
		  UINT32 rxToTxRatio);

/**
 * @brief This function is used to provision a number of bidirectional
 *         Atm UBR channels on the specified number of ports.
 *         This function may be called a number of times for different Aal
 *         types until all of the available 32 channels are used.
 *
 * It is responsible to create the specified number of ports and channels. 
 * The number of ports provisioned must be <= the number of ports specified 
 * to ixAtmCodeletInit(). There are two functions to be called depending 
 * on the Loopback mode:
 *	i)  ixAtmSwLbChannelsProvision (in ixAtmCodeletSwLoopback.c).
 *		This is for Software Loopback mode (See Figure 1) <br>
 *	ii) ixAtmRxTxChannelsProvision (in ixAtmCodeletRxTx.c).
 *		This is for Utopia and Remote Loopback mode (See Figure 1)
 *
 * <pre>
 *             ixAtmSwLbChannelsProvision() or ixAtmRxTxChannelsProvision()
 *			       	             |
 *		       		             v	      	        IxAtmSwLoopback
 *	---------------------------------------------------------------------------
 *			       	             |                    IxAtmUtilsAtm
 *		       		             v
 *		         PUBLIC ixAtmUtilsAtmVcRegisterConnect() 
 *		             /				 \
 *		            /  	       	 	          \
 *	      	           /		       	  	   \
 *	------------------------------	   ----------------------------------------
 *     		          |     IxAtmm		            |		 IxAtmd
 *    		          v				    v
 *	1. PUBLIC ixAtmmVcRegister(), 	   2. PUBLIC ixAtmdAccTxVcConnect()
 *	(To setup Tx and Rx VC)  	      (Connect Tx to the VC)
 *
 *				       	   3. PUBLIC ixAtmdAccRxVcConnect() 
 *				      	      (Connect Rx to the VC)
 *
 *				           4. PUBLIC ixAtmdAccRxVcFreeEntriesQuery()
 *					      (Get the number of entries in rx free queue)
 *	
 *					   5. PUBLIC ixAtmdAccRxVcFreeLowCallbackRegister()
 *					      (Replenish Rx buffers if callback is not NULL)
 *														
 *					   6. PUBLIC ixAtmdAccRxVcEnable()
 *					      (Start the Rx service on a VC)
 *
 *                              <center><b> Figure 1 </b></center>
 * </pre>
 *
 * @param "UINT32 numPorts" The number of ATM ports to configure
 *        channels on. This must be less than or equal to the number of
 *        ports configured in the call to ixAtmCodeletInit().
 *
 * @param "UINT32 numChannels" The number of bidirectional Atm
 *        channels to provision. Channels are provisioned accross numPorts
 *        in the following manner, for 2 ports, 4 VCs,
 *        Channel 0 on port 0, channel 1 on port 1, channel 2 on port 0,
 *        channel 3 on port 1.
 *        Valid values range from 1 to 32.
 *        Subsequent calls will assign channel and port numbers in ascending order.
 *
 * @param "IxAtmdAccAalType aalType" The aalType for the channels to
 *        provision, i.e. Aal5, Aal0 48 byte mode or Aal0 52 byte cell mode.
 *
 * @return
 * IX_SUCCESS Indicates tha the Atm Codelet has successfully provisioned the requested
 *          number of channels.<br>
 * IX_FAIL Some internal error has prevented the ATM codelet from
 *          from provisioning the channels.
 */
PUBLIC IX_STATUS
ixAtmCodeletUbrChannelsProvision (UINT32 numPorts,
				  UINT32 numChannels,
				  IxAtmdAccAalType aalType);

/**
 * @brief This function is provided to remove all channels provisioned earlier
 *         in a call to ixAtmCodeletUbrChannelsProvision().
 *
 * It is responsible to remove all VCs registered by 
 * ixAtmCodeletUbrChannelsProvision. 
 * There are two functions to be called depending on the Loopback mode:
 * 	i)  ixAtmSwLbChannelsRemove (in ixAtmCodeletSwLoopback.c).
 * 		This is for Software Loopback mode (See Figure 2) <br>
 * 	ii) ixAtmRxTxChannelsRemove (in ixAtmCodeletRxTx.c).
 * 		This is for Utopia and Remote Loopback mode. (See Figure 2)
 *
 *	Both ChannelsRemove function calls in the following manner.
 *
 * <pre>
 *     	 ixAtmRxTxChannelsRemove() or ixAtmSwLbChannelsRemove()
 *    	       			   |
 *	      			   |		 IxAtmCodeletsRxTx
 *     	----------------------------------------------------------
 *     				   |	             IxAtmUtilsAtm
 *    			       	   v
 *              PUBLIC ixAtmUtilsAtmAllVcsDisconnect()
 *	  	       		   |
 *	       	       		   v
 *     	       	PRIVATE ixAtmUtilsAtmVcUnregisterDisconnect()
 *    		      		   |
 *	      			   |
 *    	----------------------------------------------------------
 *		       		   |                        IxAtmm
 *	      			   v		
 *		PUBLIC ixAtmmVcDeregister() for Tx and Rx
 *
 *                      <center><b> Figure 2 </b></center>
 * </pre>
 *
 * @param none
 *
 * @return
 * IX_SUCCESS Indicates that the Atm Codelet has successfully removed
 *          all provisioned channels. <br>
 * IX_FAIL Some internal error has prevented the Atm Codelet
 *          from removing provisioned channels.
 */
PUBLIC IX_STATUS
ixAtmCodeletUbrChannelsRemove (void);


/** 
 * @brief This function is used to send Aal0 packets,
 *         with the number of cells per packet specified. The packets are multiplexed
 *         across all provisioned Aal0 48/52 channels.
 *         This function will block until the MBUF pool is nearly depleted.
 *
 * It is responsible to transmit a number of Aal0 packets each containing 
 * cellsPerPacket cells.
 *
 * This function will call ixAtmRxTxAal0PacketsSend (in ixAtmCodeletRxTx.c). 
 * (See Figure 3)
 *
 * <pre>
 *     	(AAL0 Packets)					(AAL5 Packets)
 *      ixAtmRxTxAal0PacketsSend()	 	        ixAtmRxTxAal5CpcsSdusSend()
 *     		|							|
 *     		v							v
 *     	PRIVATE ixAtmRxTxAal0Tx()			PRIVATE ixAtmRxTxAal5CpcsTx() 
 *     		|							|
 *     		|							|
 *     		+--------->PRIVATE ixAtmRxTxWithRetiesSend()<-----------+
 *     				        |
 *     				        |			       IxAtmCodeletRxTx
 *	   ----------------------------------------------------------------------------
 *			       		|			              IxAtmdAcc
 *		       			v
 *		       	    PUBLIC ixAtmdAccTxVcPduSubmit()
 *
 *                               <center><b> Figure 3 </b></center>
 * </pre>
 *
 * 
 * @param "UINT32 cellsPerPacket" The number of cells per packet to send.
 *
 *
 * @return
 * IX_SUCCESS Indicates tha the Atm Codelet has successfully sent
 *          all of the packets.<br>
 * IX_FAIL Some internal error has prevented the Atm Codelet
 *          from sending the packets successfully.
 */
PUBLIC IX_STATUS
ixAtmCodeletAal0PacketsSend (UINT32 cellsPerPacket);

/** 
 * @brief This function is used to send the Aal5 CPCS SDUs
 *         of specified SDU size multiplexed accross all Aal5 provisioned channels.
 *         This function will block until the MBUF pool is nearly depleted.
 *
 * It is responsible to transmit a number of Aal5 CPCS SDUs of length sduLength. 
 * This function will call ixAtmRxTxAal5CpcsSdusSend (in ixAtmCodeletRxTx.c).
 * (See Figure 3 for ixAtmRxTxAal5CpcsSdusSend)
 *
 * @param "UINT32 sduSize" The SDU size of the sdus to send in bytes. The Atm
 *         trailer + padding will be appended to this.
 *
 *
 * @return
 * IX_SUCCESS Indicates tha the Atm Codelet has successfully sent
 *          all of the sdus.<br>
 * IX_FAIL Some internal error has prevented the Atm Codelet
 *          from sending the sdus successfully.
 */
PUBLIC IX_STATUS
ixAtmCodeletAal5CpcsSdusSend (UINT32 sduSize);

/**
 * @brief This function is called to modify the portRate on a
 *         previously initialized port.
 *
 * @param "IxAtmLogicalPort port" Specifies the ATM port which is to be
 *          modified.
 *
 * It is used to modify the transmit port rate. The transmit port rate is 
 * done by calling ixAtmSchPortRateModify() function (IxAtmSch component) to
 * perform traffic shaping.
 *
 * @return
 * IX_SUCCESS The port rate has been successfully modified.<br>
 * IX_FAIL The port rate could not be modified, either
 *      because the input data was invalid, or the new port rate is
 *      insufficient to support established ATM VC contracts on this
 *      port.
 */
PUBLIC IX_STATUS
ixAtmCodeletPortRateModify (IxAtmLogicalPort port,
			    UINT32 portRate);

/** 
 * @brief Display some statistics about each port configured
 *
 * @param none
 *
 * @return void
 */
PUBLIC void
ixAtmCodeletPortQuery (void);

/** 
 * @brief This function is used to dump VC configuration and sdu send and
 *         receive counters.
 *
 * It displays some statistics about the transmit and receive streams
 *
 * @param none
 *
 * @return void
 */
PUBLIC void
ixAtmCodeletShow (void);

/**
 * @brief This function initializes Atm codelet transport subcomponent
 */
IX_STATUS
ixAtmRxTxInit (IxAtmCodeletStats *stats);



/**
 * @brief This function is used to provision a number of Atm transport channels
 */
IX_STATUS
ixAtmRxTxChannelsProvision (UINT32 numPorts,
			    UINT32 numChannels,
			    IxAtmdAccAalType aalType);

/**
 * @brief This function is used to remove all provsioned channels
 */
IX_STATUS
ixAtmRxTxChannelsRemove ( void );


/**
 * @brief Get transport stats
 */
void
ixAtmRxTxStatsGet (IxAtmCodeletStats *stats);

/** 
 * @brief Replenish buffers to the RxFree queue. Buffers are taken from the mbuf pool
 */
void
ixAtmRxTxRxFreeLowReplenish (IxAtmdAccUserId userId);

/**
 * @brief Send Aal5 CPCS SDUs
 *
 * @param "UINT32 sduSize" The SDU size in bytes to be transmitted
 */
IX_STATUS
ixAtmRxTxAal5CpcsSdusSend (UINT32 sduSize);

/**
 * @brief Send Aal0 packets
 *
 * @param "UINT32 cellsPerPacket" The number of cells per packet to send.
 */
IX_STATUS
ixAtmRxTxAal0PacketsSend (UINT32 cellsPerPacket);

/**
 * @brief This function initializes Atm codelet transport subcomponent
 */
IX_STATUS
ixAtmSwLbInit (IxAtmCodeletStats *stats, UINT32 rxToTxRatio);

/**
 * @briefThis function is used to provision a number of SW loopback Atm transport channels
 */
IX_STATUS
ixAtmSwLbChannelsProvision (UINT32 numPorts,
			    UINT32 numChannels,
			    IxAtmdAccAalType aalType);

/**
 * @brief This function is used to remove all provsioned channels
 */
IX_STATUS
ixAtmSwLbChannelsRemove ( void );


/*
 * Replenish buffers to the RxFree queue. Buffers are taken from
 *  the software queue.
 */
void
ixAtmSwLbRxFreeLowReplenish (IxAtmdAccUserId userId);

/*
 * @brief Get transport stats
 */
void
ixAtmSwLbStatsGet (IxAtmCodeletStats *stats);

/** 
 * @brief This function is used to configure OAM accross a number of previously configured ports.
 *         N.B. IxQmgr, IxNpeMh, IxAtmSch, IxAtmdAcc, IxAtmm must be initialized
 *         before calling this API, e.g. see AtmCodelet:ixAtmCodeletSystemInit()
 *
 * @param "UINT32 numPorts" The number of ATM ports to configure
 *        on. This must be less than or equal to the number of
 *        ports configured in e.g. the call to ixAtmCodeletInit().
 *
 * @return
 * - IX_SUCCESS : Indicates that the Atm Codelet has successfully provisioned the OAM Tx
 *          Port channels and the OAM Rx channel.
 * - IX_FAIL : Some internal error has prevented the ATM codelet from
 *          from provisioning the channels.
 */
PUBLIC IX_STATUS
ixOamCodeletInit (UINT32 numPorts);

/** 
 * @brief This function is used to initiate an OAM F5 ETE loopback, i.e. OAM ping,
 *         on the specified Port, VPI and VCI for a number of cells.
 *         ixOamCodeletInit must be called before calling this function.
 * 
 *         OAM F5 ETE Parent Loopback cells are sent to the F5 connection 
 *         end point (LLID = all one's).
 *
 *         If an OAM F5 ETE Child Loopback cell has been received in response to a Parent
 *         cell sent, a message will be displayed at the WindSh prompt.
 *
 *         The loopback is complete and a subsequent loopback is allowed only when all
 *         the loopback parent cells have been sent and either A) a child loopback cell has been 
 *         received in response to the last cell OR B) 5 seconds have passed since the last cell 
 *         was sent.
 *         N.B. the function blocks until the loopback has completed.
 *
 * @param "IxAtmLogicalPort" The port to send the OAM loopback cells on. This port must have been
 *        configured previously.
 *
 * @param "UINT32 VPI" The VPI of the channel to send the OAM loopback cells on. An Aal0/Aal5 
 *        channel need not be configured previously on the same VPI/VCI.
 *
 * @param "UINT32 VCI" The VCI of the channel to send the OAM loopback cells on. An Aal0/Aal5 
 *        channel need not be configured previously on the same VPI/VCI.
 *
 * @param "UINT32 numCells" The number of parent loopback cells to send. Not checked against
 *        some max value, only limited by size of type.
 *
 * @return
 * - IX_SUCCESS : Indicates that the Oam Codelet has successfully performed the OAM ping.
 * - IX_FAIL : Some internal error has prevented the Oam Codelet performing the OAM ping.
 */
PUBLIC IX_STATUS
ixOamCodeletOamF5EtePing (IxAtmLogicalPort port,
			  UINT32 vpi,
			  UINT32 vci,
			  UINT32 numCells);

/** 
 * @brief This function is used to initiate an OAM F5 Segment loopback, i.e. OAM ping,
 *         on the specified Port, VPI and VCI for a number of cells.
 *         ixOamCodeletInit must be called before calling this function.
 * 
 *         OAM F5 Segment Parent Loopback cells are sent to the F5 connection 
 *         end point (LLID = all one's).
 *
 *         If an OAM F5 Segment Child Loopback cell has been received in response to a Parent
 *         cell sent, a message will be displayed at the WindSh prompt.
 *
 *         The loopback is complete and a subsequent loopback is allowed only when all
 *         the loopback parent cells have been sent and either A) a child loopback cell has been 
 *         received in response to the last cell OR B) 5 seconds have passed since the last cell 
 *         was sent.
 *         N.B. the function blocks until the loopback has completed.
 *
 * @param "IxAtmLogicalPort" The port to send the OAM loopback cells on. This port must have been
 *        configured previously.
 *
 * @param "UINT32 VPI" The VPI of the channel to send the OAM loopback cells on. An Aal0/Aal5 
 *        channel need not be configured previously on the same VPI/VCI.
 *
 * @param "UINT32 VCI" The VCI of the channel to send the OAM loopback cells on. An Aal0/Aal5 
 *        channel need not be configured previously on the same VPI/VCI.
 *
 * @param "UINT32 numCells" The number of parent loopback cells to send. Not checked against
 *        some max value, only limited by size of type.
 *
 * @return
 * - IX_SUCCESS : Indicates that the Oam Codelet has successfully performed the OAM ping.
 * - IX_FAIL : Some internal error has prevented the Oam Codelet performing the OAM ping.
 */
PUBLIC IX_STATUS
ixOamCodeletOamF5SegPing (IxAtmLogicalPort port,
			  UINT32 vpi,
			  UINT32 vci,
			  UINT32 numCells);

/** 
 * @brief This function is used to initiate an OAM F4 ETE loopback, i.e. OAM ping,
 *         on the specified Port, VPI for a number of cells. F4 flows are VPC
 *         based and as such don't require a VCI.
 *         ixOamCodeletInit must be called before calling this function.
 * 
 *         OAM F4 ETE Parent Loopback cells are sent to the F4 connection 
 *         end point (LLID = all one's).
 *
 *         If an OAM F4 ETE Child Loopback cell has been received in response to a Parent
 *         cell sent, a message will be displayed at the WindSh prompt.
 *
 *         The loopback is complete and a subsequent loopback is allowed only when all
 *         the loopback parent cells have been sent and either A) a child loopback cell has been 
 *         received in response to the last cell OR B) 5 seconds have passed since the last cell 
 *         was sent.
 *         N.B. the function blocks until the loopback has completed.
 *
 * @param "IxAtmLogicalPort" The port to send the OAM loopback cells on. This port must have been
 *        configured previously.
 *
 * @param "UINT32 VPI" The VPI of the channel to send the OAM loopback cells on. An Aal0/Aal5 
 *        channel need not be configured previously on the same VPI/VCI.
 *
 * @param "UINT32 numCells" The number of parent loopback cells to send. Not checked against
 *        some max value, only limited by size of type.
 *
 * @return
 * - IX_SUCCESS : Indicates that the Oam Codelet has successfully performed the OAM ping.
 * - IX_FAIL : Some internal error has prevented the Oam Codelet performing the OAM ping.
 */
PUBLIC IX_STATUS
ixOamCodeletOamF4EtePing (IxAtmLogicalPort port,
			  UINT32 vpi,
			  UINT32 numCells);

/** 
 * @brief This function is used to initiate an OAM F4 Segment loopback, i.e. OAM ping,
 *         on the specified Port, VPI for a number of cells. F4 flows are VPC
 *         based and as such don't require a VCI.
 *         ixOamCodeletInit must be called before calling this function.
 * 
 *         OAM F4 Segment Parent Loopback cells are sent to the F4 connection 
 *         end point (LLID = all one's).
 *
 *         If an OAM F4 Segment Child Loopback cell has been received in response to a Parent
 *         cell sent, a message will be displayed at the WindSh prompt.
 *
 *         The loopback is complete and a subsequent loopback is allowed only when all
 *         the loopback parent cells have been sent and either A) a child loopback cell has been 
 *         received in response to the last cell OR B) 5 seconds have passed since the last cell 
 *         was sent.
 *         N.B. the function blocks until the loopback has completed.
 *
 * @param "IxAtmLogicalPort" The port to send the OAM loopback cells on. This port must have been
 *        configured previously.
 *
 * @param "UINT32 VPI" The VPI of the channel to send the OAM loopback cells on. An Aal0/Aal5 
 *        channel need not be configured previously on the same VPI/VCI.
 *
 * @param "UINT32 numCells" The number of parent loopback cells to send. Not checked against
 *        some max value, only limited by size of type.
 *
 * @return
 * - IX_SUCCESS : indicates that the Oam Codelet has successfully performed the OAM ping.
 * - IX_FAIL : Some internal error has prevented the Oam Codelet performing the OAM ping.
 */
PUBLIC IX_STATUS
ixOamCodeletOamF4SegPing (IxAtmLogicalPort port,
			  UINT32 vpi,
			  UINT32 numCells);

/** 
 * @brief This function is used to dump OAM statistics, i.e. parent and child cell receive
 * and transmit counts etc.
 *
 * @param none
 *
 * @return void
 */
void
ixOamCodeletShow (void);

/**
 * @brief Register and connect an Aal Pdu receive and transmit VC for a
 *        particular port/vpi/vci.
 *
 * This function allows a user to connect to an Aal5 Pdu receive and transmitservice
 * for a particular port/vpi/vci. It registers the callback and allocates
 * internal resources and a Connection Id to be used in further API calls
 * related to this VCC.
 *
 * @param port (in) VC identification : logical PHY port [0..7]
 * @param vpi (in) VC identification : ATM Vpi [0..255]
 * @param vci (in) VC identification : ATM Vci [0..65535]
 * @param atmService (in) type of service AAL5 @b only in ixp425 1.0
 * @param rxQueueId (in) this identifieds which of two Qs the VC
 *     should use.when icoming traffic is processed
 * @param rxCallback (in) function called when mbufs are received.
 *     This parameter cannot be a null pointer.
 * @param minimumReplenishCount (in) number of free mbufs to be used with
 *     this channel. Use a high number when the expected traffic rate on
 *     this channel is high, or when the user's mbufs are small, or when
 *     the RxVcFreeLow Notification has to be invoked less often.
 * @param bufferFreeCallback (in) function to be called to return
 *     ownership of buffers to IxAtmAtmUtils user.
 * @param userId (in) user Id to use in callback communications
 * @param rxConnId (out) Rx Conn Id passed back from IxAtmdAcc
 * @param txConnId (out) Tx Conn Id passed back from IxAtmdAcc
 *
 * @return
 * IX_SUCCESS successful call to ixAtmUtilsAtmVcRegisterConnect <br>
 * IX_FAIL parameter error, VC already in use or port is
 * not initialised or some other error occurs during processing.
 *
 */
PUBLIC IX_STATUS
ixAtmUtilsAtmVcRegisterConnect (IxAtmLogicalPort port,
				 unsigned vpi,
				 unsigned vci,
				 IxAtmdAccAalType aalType,
				 IxAtmServiceCategory atmService,
				 IxAtmRxQueueId rxQueueId,
				 IxAtmdAccRxVcRxCallback rxCallback,
				 UINT32 minimumReplenishCount,
				 IxAtmdAccTxVcBufferReturnCallback bufferFreeCallback,
				 IxAtmdAccRxVcFreeLowCallback rxFreeLowCallback,
				 IxAtmdAccUserId userId,
				 IxAtmConnId *rxConnId,
				 IxAtmConnId *txConnId);

/**
 * @brief Disconnect and unregister an Aal transmit and receive VC.
 *
 * @param rxConnId (in) conn Id of the Rx VC to disconnect and unregister
 * @param txConnId (in) conn Id of the Tx VC to disconnect and unregister
 *
 * @return
 * IX_SUCCESS successful call to ixAtmUtilsAtmVcUnregisterDisconnect<br>
 * IX_FAIL parameter error, failed to disconnect,
 * not initialised or some other error occurs during processing.
 *
 */
PUBLIC IX_STATUS
ixAtmUtilsAtmVcUnregisterDisconnect (IxAtmConnId rxConnId, IxAtmConnId txConnId);


/**
 * @brief Disconnect and unregister all registered VCs.
 *
 * @param none
 *
 * @return
 * IX_SUCCESS successful call to ixAtmUtilsAtmAllVcsDisconnect <br>
 * IX_FAIL parameter error, failed to disconnect,
 * not initialised or some other error occurs during processing.
 *
 */
PUBLIC IX_STATUS
ixAtmUtilsAtmAllVcsDisconnect (void);

/**
 * @brief Download an ATM image to the NPEA
 *
 * @param numports (in) Number of ports in the system. A value > 1 requires
 *        a MPHY_MULTIPORT to NPEA image to be downloaded. A value of 1 can use
 *        a SPHY or MPHY NPEA image.
 *
 * @param imagePtr (in) Pointer to image file that can contain multiple images.
 *
 * @param phyMode (out) This parameter will be set depending on which images is
 *        available in imagePtr. See description for numports above.
 *
 *
 * @return
 * IX_SUCCESS successful call to ixAtmUtilsAtmNpeImagedownload<br>
 * IX_FAIL parameter error, failed to disconnect,
 * not initialised or some other error occurs during processing.
 *
 */
PUBLIC IX_STATUS
ixAtmUtilsAtmImageDownload (unsigned numPorts,
			     IxAtmmPhyMode *phyMode);

/**
 * @brief Configure test test and stimulus FPGA clocks for IXP4XX as ATM master
 *
 * @param none
 *
 * @return
 * IX_SUCCESS successful call to ixAtmUtilsUtopiaFpgaStimulusAsMasterSet<br>
 * IX_FAIL parameter error, failed to configure stimulus FPGA,
 * not initialised or some other error occurs during processing.
 *
 */
PUBLIC BOOL
ixAtmUtilsUtopiaFpgaStimulusAsMasterSet(void);

/**
 * @brief Initialize the utilities buffer pool.
 *
 * @param none
 *
 * @return
 * IX_SUCCESS successful call to ixAtmUtilsMbufPoolInit <br>
 * IX_FAIL parameter error, failed initialize buffer pool
 * or some other error occurs during processing.
 *
 */
IX_STATUS
ixAtmUtilsMbufPoolInit (void);



/**
 * @brief Get an unchained buffer
 *
 * @param bufSize (in) Buffer capacity.
 *
 * @param buf (out) Pointer to buffer; will be set the new buffer
 *
 *
 * @return
 * IX_SUCCESS successful call to ixAtmUtilsMbufGet <br>
 * IX_FAIL parameter error, failed to get a buffer,
 * or some other error occurs during processing.
 *
 */
void
ixAtmUtilsMbufGet (UINT32 bufSize, IX_OSAL_MBUF **buf);

/**
 * @brief Free a buffer chain
 *
 * @param buf (out) Pointer to buffer to be freed.
 *
 *
 * @return
 * IX_SUCCESS successful call to ixAtmUtilsMbufGet<br>
 * IX_FAIL parameter error, failed to get a buffer,
 * or some other error occurs during processing.
 *
 */
void
ixAtmUtilsMbufFree (IX_OSAL_MBUF *buf);

/**
 * @brief Show statistics for the buffer pool; free, used buffer counts etc.
 *
 * @return none
 *
 */
void
ixAtmUtilsMbufShow (void);


/**
 * @brief Get the total number of MBUF in the pool
 *
 * @return none
 *
 */
void
ixAtmUtilsMbufPoolSizeGet (UINT32 bufSize, UINT32 *total);


/**
 * @brief Get the total number of unused MBUF in the pool
 *
 * @return none
 *
 */
void
ixAtmUtilsMbufPoolFreeGet (UINT32 bufSize, UINT32 *avail);

/**
 * @brief Setup the ATM codelet to use 8VBR, 8CBR, and 16UBR VCs
 * 
 * @return none 
 *
 */ 
PUBLIC void
ixAtmUtilsAtmRtVcsSet(void);

/**
 * @brief This function will return TRUE or FALSE status, to indicate whether
 *  the ATM codelet will or will not setup 8VBR, 8CBR, and 16UBR VCs 
 * 
 * @return 
 * TRUE when real-time VCs are used
 * FALSE when real-time VCs are not used
 *
 */ 
PUBLIC BOOL
ixAtmUtilsAtmRtVcsGet(void);

/** 
 * @brief This function is responsible to unset the global variable to FALSE
 * so that the ATM codelet will not setup real-time VCs 
 * 
 * @return none
 * 
 */
PUBLIC void
ixAtmUtilsAtmRtVcsUnset(void);

#endif
/* IXATMCODELET_P_H */

/*===========================================================================
FILE: 
   GobiConnectionMgmtAPIStructs.h

DESCRIPTION:
   Declaration of the Gobi API structures

Copyright (c) 2012, Code Aurora Forum. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Code Aurora Forum nor
      the names of its contributors may be used to endorse or promote
      products derived from this software without specific prior written
      permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
==========================================================================*/

#pragma once

// Include the enumerations
#include "GobiConnectionMgmtAPIEnums.h"

#pragma pack( push, 1 )

// Structure to represent a QMUX header
struct sQMUXHeader
{
   // mIF is always 1
   BYTE mIF;

   // mLength is full size of QMUX message NOT including mIF
   WORD mLength;

   // Flags are always 0 for clients
   BYTE mFlags;

   eQMIService mServiceType;
   BYTE mClientID;
};

// Structure to represent a QMI Control Raw Transaction Header
struct sQMIControlRawTransactionHeader
{
   BYTE mResponse   :1;
   BYTE mIndication :1;
   BYTE mReserved   :6; 
   BYTE mTransactionID;
};

// Structure to represent a QMI Service Raw Transaction Header
struct sQMIServiceRawTransactionHeader
{
   BYTE mCompound   :1;
   BYTE mResponse   :1;
   BYTE mIndication :1;
   BYTE mReserved   :5; 
   WORD mTransactionID;
};

// Structure to represetn a QMI Raw Message Header
struct sQMIRawMessageHeader
{
   WORD mMessageID;
   WORD mLength;

   // This array must be the size specified by mLength
   // BYTE mTLV[1];
};

// Structure to represent a QMUX message
struct sQMUXMessage
{
   sQMUXHeader mQMUXHeader;

   // Either a sQMUXControlRawTransactionHeader or
   // sQMIServiceRawTransactionHeader should go here
   
   sQMIRawMessageHeader mQMIRawMessageHeader;
};

// Structure to represent a QMI (control/service) content 
struct sQMIRawContentHeader
{
   BYTE mTypeID;
   WORD mLength;

   // This array must be the size specified by mLength
   // BYTE mValue[1];
};

// Structure to describe request TLV 0x01 for QMI CTL Get Client ID
struct sQMICTLGetClientID_ServiceType
{
   eQMIService mQMIService;
};

// Structure to describe response TLV 0x01 for QMI CTL Get Client ID
struct sQMICTLGetClientID_AssignedClientID
{
   eQMIService mQMIService;
   BYTE mClientID;
};

// Structure to describe TLV 0x02, the Result Code
// It is common for all Responses
struct sResultCode
{
   eQMIResults mQMIResult;
   eQMIErrors mQMIError;
};

// Structure to describe TLV 0x01 for QMI LOC, the status
// It is common for QMI LOC indications above 0x0032
struct sLOCIndication_Status
{
   eQMILOCStatus mStatus;
};

// Structure to describe request TLV 0x10 for WDSSetEventReport()
struct sWDSSetEventReportRequest_ChannelRateIndicator
{
   INT8 mReportChannelRate;
};

// Structure to describe request TLV 0x11 for WDSSetEventReport()
struct sWDSSetEventReportRequest_TransferStatisticsIndicator
{
   UINT8 mTransferStatisticsIntervalSeconds;
   bool mReportTXPacketSuccesses:1;
   bool mReportRXPacketSuccesses:1;
   bool mReportTXPacketErrors:1;
   bool mReportRXPacketErrors:1;
   bool mReportTXOverflows:1;
   bool mReportRXOverflows:1;
   bool mTXByteTotal:1;
   bool mRXByteTotal:1;
   bool mTXPacketsDropped:1;
   bool mRXPacketsDropped:1;

   // Padding out 22 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[2];
};

// Structure to describe request TLV 0x12 for WDSSetEventReport()
struct sWDSSetEventReportRequest_DataBearerTechnologyIndicator
{
   INT8 mReportDataBearerTechnology;
};

// Structure to describe request TLV 0x13 for WDSSetEventReport()
struct sWDSSetEventReportRequest_DormancyStatusIndicator
{
   INT8 mReportDormancyStatus;
};

// Structure to describe request TLV 0x14 for WDSSetEventReport()
struct sWDSSetEventReportRequest_MIPStatusIndicator
{
   INT8 mReportMIPStatus;
};

// Structure to describe request TLV 0x15 for WDSSetEventReport()
struct sWDSSetEventReportRequest_CurrentDataBearerTechnologyIndicator
{
   INT8 mReportDataBearerTechnology;
};

// Structure to describe request TLV 0x17 for WDSSetEventReport()
struct sWDSSetEventReportRequest_DataCallStatusIndicator
{
   INT8 mReportDataCallStatus;
};

// Structure to describe request TLV 0x18 for WDSSetEventReport()
struct sWDSSetEventReportRequest_PreferredDataSystemIndicator
{
   INT8 mReportPreferredDataSystem;
};

// Structure to describe request TLV 0x19 for WDSSetEventReport()
struct sWDSSetEventReportRequest_EVDOPMChangeIndicator
{
   INT8 mReportEVDOPageMonitorPeriodChange;
};

// Structure to describe request TLV 0x1A for WDSSetEventReport()
struct sWDSSetEventReportRequest_DataSystemsIndicator
{
   INT8 mReportDataSystems;
};

// Structure to describe request TLV 0x1B for WDSSetEventReport()
struct sWDSSetEventReportRequest_UplinkFlowControlIndicator
{
   INT8 mReportUplinkFlowControl;
};

// Structure to describe indication TLV 0x10 for WDS EventReport
struct sWDSEventReportIndication_TXPacketSuccesses
{
   UINT32 mTXPacketSuccesses;
};

// Structure to describe indication TLV 0x11 for WDS EventReport
struct sWDSEventReportIndication_RXPacketSuccesses
{
   UINT32 mRXPacketSuccesses;
};

// Structure to describe indication TLV 0x12 for WDS EventReport
struct sWDSEventReportIndication_TXPacketErrors
{
   UINT32 mTXPacketErrors;
};

// Structure to describe indication TLV 0x13 for WDS EventReport
struct sWDSEventReportIndication_RXPacketErrors
{
   UINT32 mRXPacketErrors;
};

// Structure to describe indication TLV 0x14 for WDS EventReport
struct sWDSEventReportIndication_TXOverflows
{
   UINT32 mTXOverflows;
};

// Structure to describe indication TLV 0x15 for WDS EventReport
struct sWDSEventReportIndication_RXOverflows
{
   UINT32 mRXOverflows;
};

// Structure to describe indication TLV 0x16 for WDS EventReport
struct sWDSEventReportIndication_ChannelRates
{
   UINT32 mChannelTXRatebps;
   UINT32 mChannelRXRatebps;
};

// Structure to describe indication TLV 0x17 for WDS EventReport
struct sWDSEventReportIndication_DataBearerTechnology
{
   eQMIDataBearerTechnologies mDataBearerTechnology;
};

// Structure to describe indication TLV 0x18 for WDS EventReport
struct sWDSEventReportIndication_DormancyStatus
{
   eQMIDormancyStatus mDormancyStatus;
};

// Structure to describe indication TLV 0x19 for WDS EventReport
struct sWDSEventReportIndication_TXBytes
{
   UINT64 mTXByteTotal;
};

// Structure to describe indication TLV 0x1A for WDS EventReport
struct sWDSEventReportIndication_RXBytes
{
   UINT64 mRXByteTotal;
};

// Structure to describe indication TLV 0x1B for WDS EventReport
struct sWDSEventReportIndication_MIPStatus
{
   UINT8 mMIPStatus;
};

// Structure to describe indication TLV 0x1D for WDS EventReport
struct sWDSEventReportIndication_CurrentDataBearerTechnology
{
   eQMIWDSNetworkTypes mNetworkType;

   // The following union is based on the value of mNetworkType
   union uValOfNetworkType
   {
      // If the value of mNetworkType == 1
      struct sNetworkTypeIs1
      {
         bool mCDMA1x:1;
         bool mCDMA1xEvDORev0:1;
         bool mCDMA1xEvDORevA:1;
         bool mCDMA1xEvDORevB:1;
         bool mCDMAEHRPD:1;
         bool mCDMAFMC:1;
      
         // Padding out 25 bits
         UINT8 mReserved1:2;
         UINT8 mReserved2[2];
         UINT8 mReserved3:7;
      
         bool mNullBearer:1;
      
         // The following union is for handing both mCDMA1x and mCDMA1xEvDORev(0, A, B)
         union uValOfCDMA1x_or_CDMA1xEvDORevX
         {
            // If the value of mCDMA1x == 1
            struct sCDMA1xIs1
            {
               bool mCDMA1xIS95:1;
               bool mCDMA1xIS2000:1;
               bool mCDMA1xIS2000RelA:1;
            
               // Padding out 29 bits
               UINT8 mReserved4:5;
               UINT8 mReserved5[3];
            };
      
            sCDMA1xIs1 mCDMA1xIs1;
      
            // If the value of mCDMA1xEvDORev0 == 1
            struct sCDMA1xEvDORev0Is1
            {
               bool mCDMA1xEvDORev0DPA:1;
            
               // Padding out 31 bits
               UINT8 mReserved6:7;
               UINT8 mReserved7[3];
            };
      
            sCDMA1xEvDORev0Is1 mCDMA1xEvDORev0Is1;
      
            // If the value of mCDMA1xEvDORevA == 1
            struct sCDMA1xEvDORevAIs1
            {
               bool mCDMA1xEvDORevADPA:1;
               bool mCDMA1xEvDORevAMFPA:1;
               bool mCDMA1xEvDORevAEMPA:1;
               bool mCDMA1xEvDORevAEMPAEHRPD:1;
            
               // Padding out 28 bits
               UINT8 mReserved8:4;
               UINT8 mReserved9[3];
            };
      
            sCDMA1xEvDORevAIs1 mCDMA1xEvDORevAIs1;
      
            // If the value of mCDMA1xEvDORevB == 1
            struct sCDMA1xEvDORevBIs1
            {
               bool mCDMA1xEvDORevBDPA:1;
               bool mCDMA1xEvDORevBMFPA:1;
               bool mCDMA1xEvDORevBEMPA:1;
               bool mCDMA1xEvDORevBEMPAEHRPD:1;
               bool mCDMA1xEvDORevBMMPA:1;
               bool mCDMA1xEvDORevBMMPAEHRPD:1;
            
               // Padding out 26 bits
               UINT8 mReserved10:2;
               UINT8 mReserved11[3];
            };
      
            sCDMA1xEvDORevBIs1 mCDMA1xEvDORevBIs1;
      
            // Padding out 32 bits
            UINT8 mReserved12[4];
         };
      
         uValOfCDMA1x_or_CDMA1xEvDORevX mValOfCDMA1x_or_CDMA1xEvDORevX;
      };

      sNetworkTypeIs1 mNetworkTypeIs1;

      // If the value of mNetworkType == 2
      struct sNetworkTypeIs2
      {
         bool mWCDMA:1;
         bool mGPRS:1;
         bool mHSDPA:1;
         bool mHSUPA:1;
         bool mEDGE:1;
         bool mLTE:1;
         bool mHSDPAPlus:1;
         bool mDualCellHSDPAPlus:1;
         bool m64QAM:1;
         bool mTDSCDMA:1;
      
         // Padding out 21 bits
         UINT8 mReserved13:6;
         UINT8 mReserved14;
         UINT8 mReserved15:7;
      
         bool mNullBearer:1;
      };

      sNetworkTypeIs2 mNetworkTypeIs2;

      // Padding out 64 bits
      UINT8 mReserved16[8];
   };

   uValOfNetworkType mValOfNetworkType;
};

// Structure to describe indication TLV 0x1F for WDS EventReport
struct sWDSEventReportIndication_DataCallStatus
{
   eQMIWDSDataCallStatus mDataCallStatus;
};

// Structure to describe indication TLV 0x20 for WDS EventReport
struct sWDSEventReportIndication_PreferredDataSystem
{
   eQMIWDSDataSystems mPreferredDataSystem;
};

// Structure to describe indication TLV 0x22 for WDS EventReport
struct sWDSEventReportIndication_DataCallType
{
   eQMIWDSDataCallTypes mDataCallType;
   eQMIWDSTetheredCallTypes mTetheredCallType;
};

// Structure to describe indication TLV 0x23 for WDS EventReport
struct sWDSEventReportIndication_EVDOPageMonitorPeriodChange
{
   UINT8 mEVDOPageMonitorPeriodChange;
   INT8 mEVDOForceLongSleep;
};

// Structure to describe indication TLV 0x24 for WDS EventReport
struct sWDSEventReportIndication_DataSystems
{
   eQMIWDSDataSystemNetworkTypes mPreferredNetworkType;
   UINT8 mNetworkCount;

   struct sNetwork
   {
      eQMIWDSDataSystemNetworkTypes mNetworkType;
   
      // The following union is based on the value of mNetworkType
      union uValOfNetworkType
      {
         // If the value of mNetworkType == 0
         struct sNetworkTypeIs0
         {
            bool mWCDMA:1;
            bool mGPRS:1;
            bool mHSDPA:1;
            bool mHSUPA:1;
            bool mEDGE:1;
            bool mLTE:1;
            bool mHSDPAPlus:1;
            bool mDualCellHSDPAPlus:1;
            bool m64QAM:1;
            bool mTDSCDMA:1;
         
            // Padding out 21 bits
            UINT8 mReserved1:6;
            UINT8 mReserved2;
            UINT8 mReserved3:7;
         
            bool mNULLBearer:1;
         };
   
         sNetworkTypeIs0 mNetworkTypeIs0;
   
         // If the value of mNetworkType == 1
         struct sNetworkTypeIs1
         {
            bool mCDMA1x:1;
            bool mCDMA1xEvDORev0:1;
            bool mCDMA1xEvDORevA:1;
            bool mCDMA1xEvDORevB:1;
            bool mCDMAEHRPD:1;
            bool mCDMAFMC:1;
         
            // Padding out 25 bits
            UINT8 mReserved4:2;
            UINT8 mReserved5[2];
            UINT8 mReserved6:7;
         
            bool mNULLBearer:1;
         
            // The following union is for handing all mCDMA1x types
            union uValOfCDMA1xTypes
            {
               // If the value of mCDMA1x == 1
               struct sCDMA1xIs1
               {
                  bool mCDMA1xIS95:1;
                  bool mCDMA1xIS2000:1;
                  bool mCDMA1xIS2000RelA:1;
               
                  // Padding out 29 bits
                  UINT8 mReserved7:5;
                  UINT8 mReserved8[3];
               };
         
               sCDMA1xIs1 mCDMA1xIs1;
         
               // If the value of mCDMA1xEvDORev0 == 1
               struct sCDMA1xEvDORev0Is1
               {
                  bool mCDMA1xEvDORev0DPA:1;
               
                  // Padding out 31 bits
                  UINT8 mReserved9:7;
                  UINT8 mReserved10[3];
               };
         
               sCDMA1xEvDORev0Is1 mCDMA1xEvDORev0Is1;
         
               // If the value of mCDMA1xEvDORevA == 1
               struct sCDMA1xEvDORevAIs1
               {
                  bool mCDMA1xEvDORevADPA:1;
                  bool mCDMA1xEvDORevAMFPA:1;
                  bool mCDMA1xEvDORevAEMPA:1;
                  bool mCDMA1xEvDORevAEMPAEHRPD:1;
               
                  // Padding out 28 bits
                  UINT8 mReserved11:4;
                  UINT8 mReserved12[3];
               };
         
               sCDMA1xEvDORevAIs1 mCDMA1xEvDORevAIs1;
         
               // If the value of mCDMA1xEvDORevB == 1
               struct sCDMA1xEvDORevBIs1
               {
                  bool mCDMA1xEvDORevBDPA:1;
                  bool mCDMA1xEvDORevBMFPA:1;
                  bool mCDMA1xEvDORevBEMPA:1;
                  bool mCDMA1xEvDORevBEMPAEHRPD:1;
                  bool mCDMA1xEvDORevBMMPA:1;
                  bool mCDMA1xEvDORevBMMPAEHRPD:1;
               
                  // Padding out 26 bits
                  UINT8 mReserved13:2;
                  UINT8 mReserved14[3];
               };
         
               sCDMA1xEvDORevBIs1 mCDMA1xEvDORevBIs1;
         
               // Padding out 32 bits
               UINT8 mReserved15[4];
            };
         
            uValOfCDMA1xTypes mValOfCDMA1xTypes;
         };
   
         sNetworkTypeIs1 mNetworkTypeIs1;
   
         // Padding out 64 bits
         UINT8 mReserved16[8];
      };
   
      uValOfNetworkType mValOfNetworkType;
   };

   // This array must be the size specified by mNetworkCount
   // sNetwork mNetworks[1];
};

// Structure to describe indication TLV 0x25 for WDS EventReport
struct sWDSEventReportIndication_TXPacketsDropped
{
   UINT32 mTXPacketsDropped;
};

// Structure to describe indication TLV 0x26 for WDS EventReport
struct sWDSEventReportIndication_RXPacketsDropped
{
   UINT32 mRXPacketsDropped;
};

// Structure to describe indication TLV 0x27 for WDS EventReport
struct sWDSEventReportIndication_UplinkFlowControl
{
   INT8 mUplinkFlowControlEnabled;
};

// Structure to describe request TLV 0x01 for WDSAbort()
struct sWDSAbortRequest_TransactionID
{
   UINT16 mTransactionID;
};

// Structure to describe request TLV 0x10 for WDSSetIndication()
struct sWDSSetIndicationRequest_TMGIList
{
   INT8 mReportEMBMSTMGIList;
};

// Structure to describe request TLV 0x10 for WDSStartNetworkInterface()
struct sWDSStartNetworkInterfaceRequest_PrimaryDNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x11 for WDSStartNetworkInterface()
struct sWDSStartNetworkInterfaceRequest_SecondaryDNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x12 for WDSStartNetworkInterface()
struct sWDSStartNetworkInterfaceRequest_PrimaryNBNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x13 for WDSStartNetworkInterface()
struct sWDSStartNetworkInterfaceRequest_SecondaryNBNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x14 for WDSStartNetworkInterface()
struct sWDSStartNetworkInterfaceRequest_ContextAPNName
{
   // String is variable length, but must be size of the container
   // char mAPNName[1];
};

// Structure to describe request TLV 0x15 for WDSStartNetworkInterface()
struct sWDSStartNetworkInterfaceRequest_IPAddress
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x16 for WDSStartNetworkInterface()
struct sWDSStartNetworkInterfaceRequest_Authentication
{
   bool mEnablePAP:1;
   bool mEnableCHAP:1;

   // Padding out 6 bits
   UINT8 mReserved1:6;
};

// Structure to describe request TLV 0x17 for WDSStartNetworkInterface()
struct sWDSStartNetworkInterfaceRequest_Username
{
   // String is variable length, but must be size of the container
   // char mUsername[1];
};

// Structure to describe request TLV 0x18 for WDSStartNetworkInterface()
struct sWDSStartNetworkInterfaceRequest_Password
{
   // String is variable length, but must be size of the container
   // char mPassword[1];
};

// Structure to describe request TLV 0x19 for WDSStartNetworkInterface()
struct sWDSStartNetworkInterfaceRequest_IPFamily
{
   eQMIWDSIPFamilies mIPFamily;
};

// Structure to describe request TLV 0x30 for WDSStartNetworkInterface()
struct sWDSStartNetworkInterfaceRequest_TechnologyPreference
{
   bool mEnable3GPP:1;
   bool mEnable3GPP2:1;

   // Padding out 6 bits
   UINT8 mReserved1:6;
};

// Structure to describe request TLV 0x31 for WDSStartNetworkInterface()
struct sWDSStartNetworkInterfaceRequest_3GPPProfileIdentifier
{
   UINT8 mProfileIndex;
};

// Structure to describe request TLV 0x32 for WDSStartNetworkInterface()
struct sWDSStartNetworkInterfaceRequest_3GPP2ProfileIdentifier
{
   UINT8 mProfileIndex;
};

// Structure to describe request TLV 0x33 for WDSStartNetworkInterface()
struct sWDSStartNetworkInterfaceRequest_Autoconnect
{
   eQMIWDSAutoconnectSettings mAutoconnectSetting;
};

// Structure to describe request TLV 0x34 for WDSStartNetworkInterface()
struct sWDSStartNetworkInterfaceRequest_ExtendedTechnologyPreference
{
   eQMIWDSExtendedTechPrefs mExtendedTechnologyPreference;
};

// Structure to describe request TLV 0x35 for WDSStartNetworkInterface()
struct sWDSStartNetworkInterfaceRequest_CallType
{
   eQMIWDSCallTypes mCallType;
};

// Structure to describe response TLV 0x01 for WDSStartNetworkInterface()
struct sWDSStartNetworkInterfaceResponse_PacketDataHandle
{
   UINT32 mPacketDataHandle;
};

// Structure to describe response TLV 0x10 for WDSStartNetworkInterface()
struct sWDSStartNetworkInterfaceResponse_CallEndReason
{
   eQMICallEndReasons mCallEnd;
};

// Structure to describe response TLV 0x11 for WDSStartNetworkInterface()
struct sWDSStartNetworkInterfaceResponse_VerboseCallEndReason
{
   eQMIWDSCallEndReasonTypes mCallEndReasonType;

   // The following union is based on the value of mCallEndReasonType
   union uValOfCallEndReasonType
   {
      // Always present
      UINT16 mCallEndReasonValue;

      // If the value of mCallEndReasonType == 1
      struct sCallEndReasonTypeIs1
      {
         eQMIWDSMobileIPCallEndReasons mMobileIPCallEndReason;
      };

      sCallEndReasonTypeIs1 mCallEndReasonTypeIs1;

      // If the value of mCallEndReasonType == 2
      struct sCallEndReasonTypeIs2
      {
         eQMIWDSInternalCallEndReasons mInternalCallEndReason;
      };

      sCallEndReasonTypeIs2 mCallEndReasonTypeIs2;

      // If the value of mCallEndReasonType == 3
      struct sCallEndReasonTypeIs3
      {
         eQMIWDSCallManagerCallEndReasons mCallManagerCallEndReason;
      };

      sCallEndReasonTypeIs3 mCallEndReasonTypeIs3;

      // If the value of mCallEndReasonType == 6
      struct sCallEndReasonTypeIs6
      {
         eQMIWDS3GPPCallEndReasons m3GPPCallEndReason;
      };

      sCallEndReasonTypeIs6 mCallEndReasonTypeIs6;

      // If the value of mCallEndReasonType == 7
      struct sCallEndReasonTypeIs7
      {
         eQMIWDSPPPCallEndReason mPPPCallEndReason;
      };

      sCallEndReasonTypeIs7 mCallEndReasonTypeIs7;

      // If the value of mCallEndReasonType == 8
      struct sCallEndReasonTypeIs8
      {
         eQMIWDSEHRPDCallEndReason mEHRPDCallEndReason;
      };

      sCallEndReasonTypeIs8 mCallEndReasonTypeIs8;

      // If the value of mCallEndReasonType == 9
      struct sCallEndReasonTypeIs9
      {
         eQMIWDSIPv6CallEndReason mIPv6CallEndReason;
      };

      sCallEndReasonTypeIs9 mCallEndReasonTypeIs9;

      // Padding out 16 bits
      UINT8 mReserved1[2];
   };

   uValOfCallEndReasonType mValOfCallEndReasonType;
};

// Structure to describe request TLV 0x01 for WDSStopNetworkInterface()
struct sWDSStopNetworkInterfaceRequest_PacketDataHandle
{
   UINT32 mPacketDataHandle;
};

// Structure to describe request TLV 0x10 for WDSStopNetworkInterface()
struct sWDSStopNetworkInterfaceRequest_Autoconnect
{
   INT8 mAutoconnectOff;
};

// Structure to describe response TLV 0x01 for WDSGetPacketServiceStatus()
struct sWDSGetPacketServiceStatusResponse_Status
{
   eQMIConnectionStatus mConnectionStatus;
};

// Structure to describe indication TLV 0x01 for WDS PacketServiceStatusReport
struct sWDSPacketServiceStatusReportIndication_Status
{
   eQMIConnectionStatus mConnectionStatus;
   INT8 mReconfigureRequired;
};

// Structure to describe indication TLV 0x10 for WDS PacketServiceStatusReport
struct sWDSPacketServiceStatusReportIndication_CallEndReason
{
   eQMICallEndReasons mCallEnd;
};

// Structure to describe indication TLV 0x11 for WDS PacketServiceStatusReport
struct sWDSPacketServiceStatusReportIndication_VerboseCallEndReason
{
   eQMIWDSCallEndReasonTypes mCallEndReasonType;

   // The following union is based on the value of mCallEndReasonType
   union uValOfCallEndReasonType
   {
      // Always present
      UINT16 mCallEndReasonValue;

      // If the value of mCallEndReasonType == 1
      struct sCallEndReasonTypeIs1
      {
         eQMIWDSMobileIPCallEndReasons mMobileIPCallEndReason;
      };

      sCallEndReasonTypeIs1 mCallEndReasonTypeIs1;

      // If the value of mCallEndReasonType == 2
      struct sCallEndReasonTypeIs2
      {
         eQMIWDSInternalCallEndReasons mInternalCallEndReason;
      };

      sCallEndReasonTypeIs2 mCallEndReasonTypeIs2;

      // If the value of mCallEndReasonType == 3
      struct sCallEndReasonTypeIs3
      {
         eQMIWDSCallManagerCallEndReasons mCallManagerCallEndReason;
      };

      sCallEndReasonTypeIs3 mCallEndReasonTypeIs3;

      // If the value of mCallEndReasonType == 6
      struct sCallEndReasonTypeIs6
      {
         eQMIWDS3GPPCallEndReasons m3GPPCallEndReason;
      };

      sCallEndReasonTypeIs6 mCallEndReasonTypeIs6;

      // If the value of mCallEndReasonType == 7
      struct sCallEndReasonTypeIs7
      {
         eQMIWDSPPPCallEndReason mPPPCallEndReason;
      };

      sCallEndReasonTypeIs7 mCallEndReasonTypeIs7;

      // If the value of mCallEndReasonType == 8
      struct sCallEndReasonTypeIs8
      {
         eQMIWDSEHRPDCallEndReason mEHRPDCallEndReason;
      };

      sCallEndReasonTypeIs8 mCallEndReasonTypeIs8;

      // If the value of mCallEndReasonType == 9
      struct sCallEndReasonTypeIs9
      {
         eQMIWDSIPv6CallEndReason mIPv6CallEndReason;
      };

      sCallEndReasonTypeIs9 mCallEndReasonTypeIs9;

      // Padding out 16 bits
      UINT8 mReserved1[2];
   };

   uValOfCallEndReasonType mValOfCallEndReasonType;
};

// Structure to describe indication TLV 0x12 for WDS PacketServiceStatusReport
struct sWDSPacketServiceStatusReportIndication_IPFamily
{
   eQMIWDSIPFamilies mIPFamily;
};

// Structure to describe indication TLV 0x13 for WDS PacketServiceStatusReport
struct sWDSPacketServiceStatusReportIndication_ExtendedTechnology
{
   eQMIWDSExtendedTechPrefs mExtendedTechnology;
};

// Structure to describe response TLV 0x01 for WDSGetChannelRates()
struct sWDSGetChannelRatesResponse_ChannelRates
{
   UINT32 mChannelTXRatebps;
   UINT32 mChannelRXRatebps;
   UINT32 mMaxChannelTXRatebps;
   UINT32 mMaxChannelRXRatebps;
};

// Structure to describe request TLV 0x01 for WDSGetPacketStatistics()
struct sWDSGetPacketStatisticsRequest_PacketStatsMask
{
   bool mReportTXPacketSuccesses:1;
   bool mReportRXPacketSuccesses:1;
   bool mReportTXPacketErrors:1;
   bool mReportRXPacketErrors:1;
   bool mReportTXOverflows:1;
   bool mReportRXOverflows:1;
   bool mTXByteTotal:1;
   bool mRXByteTotal:1;
   bool mTXPacketsDropped:1;
   bool mRXPacketsDropped:1;

   // Padding out 22 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[2];
};

// Structure to describe response TLV 0x10 for WDSGetPacketStatistics()
struct sWDSGetPacketStatisticsResponse_TXPacketSuccesses
{
   UINT32 mTXPacketSuccesses;
};

// Structure to describe response TLV 0x11 for WDSGetPacketStatistics()
struct sWDSGetPacketStatisticsResponse_RXPacketSuccesses
{
   UINT32 mRXPacketSuccesses;
};

// Structure to describe response TLV 0x12 for WDSGetPacketStatistics()
struct sWDSGetPacketStatisticsResponse_TXPacketErrors
{
   UINT32 mTXPacketErrors;
};

// Structure to describe response TLV 0x13 for WDSGetPacketStatistics()
struct sWDSGetPacketStatisticsResponse_RXPacketErrors
{
   UINT32 mRXPacketErrors;
};

// Structure to describe response TLV 0x14 for WDSGetPacketStatistics()
struct sWDSGetPacketStatisticsResponse_TXOverflows
{
   UINT32 mTXOverflows;
};

// Structure to describe response TLV 0x15 for WDSGetPacketStatistics()
struct sWDSGetPacketStatisticsResponse_RXOverflows
{
   UINT32 mRXOverflows;
};

// Structure to describe response TLV 0x19 for WDSGetPacketStatistics()
struct sWDSGetPacketStatisticsResponse_TXBytes
{
   UINT64 mTXByteTotal;
};

// Structure to describe response TLV 0x1A for WDSGetPacketStatistics()
struct sWDSGetPacketStatisticsResponse_RXBytes
{
   UINT64 mRXByteTotal;
};

// Structure to describe response TLV 0x1B for WDSGetPacketStatistics()
struct sWDSGetPacketStatisticsResponse_PreviousTXBytes
{
   UINT64 mPreviousCallTXByteTotal;
};

// Structure to describe response TLV 0x1C for WDSGetPacketStatistics()
struct sWDSGetPacketStatisticsResponse_PreviousRXBytes
{
   UINT64 mPreviousCallRXByteTotal;
};

// Structure to describe response TLV 0x1D for WDSGetPacketStatistics()
struct sWDSGetPacketStatisticsResponse_TXPacketsDropped
{
   UINT32 mTXPacketsDropped;
};

// Structure to describe response TLV 0x1E for WDSGetPacketStatistics()
struct sWDSGetPacketStatisticsResponse_RXPacketsDropped
{
   UINT32 mRXPacketsDropped;
};

// Structure to describe request TLV 0x01 for WDSCreateProfile()
struct sWDSCreateProfileRequest_ProfileType
{
   eQMIProfileTypes mProfileType;
};

// Structure to describe request TLV 0x10 for WDSCreateProfile()
struct sWDSCreateProfileRequest_ProfileName
{
   // String is variable length, but must be size of the container
   // char mProfileName[1];
};

// Structure to describe request TLV 0x11 for WDSCreateProfile()
struct sWDSCreateProfileRequest_PDPType
{
   eQMIPDPTypes mPDPType;
};

// Structure to describe request TLV 0x12 for WDSCreateProfile()
struct sWDSCreateProfileRequest_PDPHeaderCompressionType
{
   eQMIWDSPDPHeaderCompressionType mPDPHeaderCompressionType;
};

// Structure to describe request TLV 0x13 for WDSCreateProfile()
struct sWDSCreateProfileRequest_PDPDataCompressionType
{
   eQMIWDSPDPDataCompressionType mPDPDataCompressionType;
};

// Structure to describe request TLV 0x14 for WDSCreateProfile()
struct sWDSCreateProfileRequest_APNName
{
   // String is variable length, but must be size of the container
   // char mAPNName[1];
};

// Structure to describe request TLV 0x15 for WDSCreateProfile()
struct sWDSCreateProfileRequest_PrimaryDNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x16 for WDSCreateProfile()
struct sWDSCreateProfileRequest_SecondaryDNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x17 for WDSCreateProfile()
struct sWDSCreateProfileRequest_UMTSRequestedQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
};

// Structure to describe request TLV 0x18 for WDSCreateProfile()
struct sWDSCreateProfileRequest_UMTSMinimumQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
};

// Structure to describe request TLV 0x19 for WDSCreateProfile()
struct sWDSCreateProfileRequest_GPRSRequestedQoS
{
   UINT32 mPrecedenceClass;
   UINT32 mDelayClass;
   UINT32 mReliabilityClass;
   UINT32 mPeakThroughputClass;
   UINT32 mMeanThroughputClass;
};

// Structure to describe request TLV 0x1A for WDSCreateProfile()
struct sWDSCreateProfileRequest_GPRSMinimumQoS
{
   UINT32 mPrecedenceClass;
   UINT32 mDelayClass;
   UINT32 mReliabilityClass;
   UINT32 mPeakThroughputClass;
   UINT32 mMeanThroughputClass;
};

// Structure to describe request TLV 0x1B for WDSCreateProfile()
struct sWDSCreateProfileRequest_Username
{
   // String is variable length, but must be size of the container
   // char mUsername[1];
};

// Structure to describe request TLV 0x1C for WDSCreateProfile()
struct sWDSCreateProfileRequest_Password
{
   // String is variable length, but must be size of the container
   // char mPassword[1];
};

// Structure to describe request TLV 0x1D for WDSCreateProfile()
struct sWDSCreateProfileRequest_Authentication
{
   bool mEnablePAP:1;
   bool mEnableCHAP:1;

   // Padding out 6 bits
   UINT8 mReserved1:6;
};

// Structure to describe request TLV 0x1E for WDSCreateProfile()
struct sWDSCreateProfileRequest_IPAddress
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x1F for WDSCreateProfile()
struct sWDSCreateProfileRequest_PCSCF
{
   INT8 mPCSCFAddressUsingPCO;
};

// Structure to describe request TLV 0x20 for WDSCreateProfile()
struct sWDSCreateProfileRequest_PDPAccessControlFlag
{
   eQMIWDSPDPAccessControlFlag mPDPAccessControlFlag;
};

// Structure to describe request TLV 0x21 for WDSCreateProfile()
struct sWDSCreateProfileRequest_PCSCFAddressUsingDHCP
{
   INT8 mPCSCFAddressUsingDHCP;
};

// Structure to describe request TLV 0x22 for WDSCreateProfile()
struct sWDSCreateProfileRequest_IMCNFlag
{
   INT8 mIMCN;
};

// Structure to describe request TLV 0x23 for WDSCreateProfile()
struct sWDSCreateProfileRequest_TrafficFlowTemplateID1Parameters
{
   UINT8 mFilterID;
   UINT8 mEvaluationID;
   eQMIWDSIPVersion mIPVersion;

   // The following union is based on the value of mIPVersion
   union uValOfIPVersion
   {
      // If the value of mIPVersion == 4
      struct sIPVersionIs4
      {
         UINT8 mIPV4Address[4];
      };

      sIPVersionIs4 mIPVersionIs4;

      // If the value of mIPVersion == 6
      struct sIPVersionIs6
      {
         UINT16 mIPv6Address[8];
      };

      sIPVersionIs6 mIPVersionIs6;

      // Padding out 128 bits
      UINT8 mReserved1[16];
   };

   uValOfIPVersion mValOfIPVersion;

   UINT8 mSourceIPMask;
   UINT8 mNextHeader;
   UINT16 mDestinationPortRangeStart;
   UINT16 mDestinationPortRangeEnd;
   UINT16 mSourcePortRangeStart;
   UINT16 mSourcePortRangeEnd;
   UINT32 mIPSECSecurityParameterIndex;
   UINT16 mTOSMask;
   UINT32 mFlowLabel;
};

// Structure to describe request TLV 0x24 for WDSCreateProfile()
struct sWDSCreateProfileRequest_TrafficFlowTemplateID2Parameters
{
   UINT8 mFilterID;
   UINT8 mEvaluationID;
   eQMIWDSIPVersion mIPVersion;

   // The following union is based on the value of mIPVersion
   union uValOfIPVersion
   {
      // If the value of mIPVersion == 4
      struct sIPVersionIs4
      {
         UINT8 mIPV4Address[4];
      };

      sIPVersionIs4 mIPVersionIs4;

      // If the value of mIPVersion == 6
      struct sIPVersionIs6
      {
         UINT16 mIPv6Address[8];
      };

      sIPVersionIs6 mIPVersionIs6;

      // Padding out 128 bits
      UINT8 mReserved1[16];
   };

   uValOfIPVersion mValOfIPVersion;

   UINT8 mSourceIPMask;
   UINT8 mNextHeader;
   UINT16 mDestinationPortRangeStart;
   UINT16 mDestinationPortRangeEnd;
   UINT16 mSourcePortRangeStart;
   UINT16 mSourcePortRangeEnd;
   UINT32 mIPSECSecurityParameterIndex;
   UINT16 mTOSMask;
   UINT32 mFlowLabel;
};

// Structure to describe request TLV 0x25 for WDSCreateProfile()
struct sWDSCreateProfileRequest_PDPContextNumber
{
   UINT8 mPDPContextNumber;
};

// Structure to describe request TLV 0x26 for WDSCreateProfile()
struct sWDSCreateProfileRequest_PDPContextSecondaryFlag
{
   INT8 mPDPContextSecondaryFlag;
};

// Structure to describe request TLV 0x27 for WDSCreateProfile()
struct sWDSCreateProfileRequest_PDPContextPrimaryID
{
   UINT8 mPDPPrimaryID;
};

// Structure to describe request TLV 0x28 for WDSCreateProfile()
struct sWDSCreateProfileRequest_IPv6Address
{
   UINT16 mIPv6Address[8];
};

// Structure to describe request TLV 0x29 for WDSCreateProfile()
struct sWDSCreateProfileRequest_RequestedQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
   INT8 mSignalingIndication;
};

// Structure to describe request TLV 0x2A for WDSCreateProfile()
struct sWDSCreateProfileRequest_MinimumQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
   INT8 mSignalingIndication;
};

// Structure to describe request TLV 0x2B for WDSCreateProfile()
struct sWDSCreateProfileRequest_PrimaryIPv6
{
   UINT16 mIPv6Address[8];
};

// Structure to describe request TLV 0x2C for WDSCreateProfile()
struct sWDSCreateProfileRequest_SecondaryIPv6
{
   UINT16 mIPv6Address[8];
};

// Structure to describe request TLV 0x2D for WDSCreateProfile()
struct sWDSCreateProfileRequest_AddressPreference
{
   eQMIWDSAddressAllocationPreference mAddressAllocationPreference;
};

// Structure to describe request TLV 0x2E for WDSCreateProfile()
struct sWDSCreateProfileRequest_LTEQoSParameters
{
   eQMIWDSQoSClassIdentifier mQoSClassIdentifier;
   UINT32 mGuaranteedDownlinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mMaxUplinkBitrate;
};

// Structure to describe request TLV 0x2F for WDSCreateProfile()
struct sWDSCreateProfileRequest_APNDisabled
{
   INT8 mAPNDisabled;
};

// Structure to describe request TLV 0x30 for WDSCreateProfile()
struct sWDSCreateProfileRequest_PDNInactivityTimer
{
   UINT32 mPDNInactivityTimerSeconds;
};

// Structure to describe request TLV 0x31 for WDSCreateProfile()
struct sWDSCreateProfileRequest_APNClass
{
   UINT8 mAPNClass;
};

// Structure to describe request TLV 0x35 for WDSCreateProfile()
struct sWDSCreateProfileRequest_APNBearer
{
   bool mGSM:1;
   bool mWCDMA:1;
   bool mLTE:1;

   // Padding out 60 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2[6];
   UINT8 mReserved3:7;

   bool mAny:1;
};

// Structure to describe request TLV 0x8F for WDSCreateProfile()
struct sWDSCreateProfileRequest_ProfilePersistent
{
   INT8 mProfilePersistent;
};

// Structure to describe request TLV 0x90 for WDSCreateProfile()
struct sWDSCreateProfileRequest_NegotiateDNSServerPreference
{
   INT8 mNegotiateDNSServerPreference;
};

// Structure to describe request TLV 0x91 for WDSCreateProfile()
struct sWDSCreateProfileRequest_PPPSessionCloseTimerDO
{
   UINT32 mPPPSessionCloseTimerDOSeconds;
};

// Structure to describe request TLV 0x92 for WDSCreateProfile()
struct sWDSCreateProfileRequest_PPPSessionCloseTimer1X
{
   UINT32 mPPPSessionCloseTimer1XSeconds;
};

// Structure to describe request TLV 0x93 for WDSCreateProfile()
struct sWDSCreateProfileRequest_AllowLinger
{
   INT8 mAllowLinger;
};

// Structure to describe request TLV 0x94 for WDSCreateProfile()
struct sWDSCreateProfileRequest_LCPACKTimeout
{
   UINT16 mTimeoutMilliseconds;
};

// Structure to describe request TLV 0x95 for WDSCreateProfile()
struct sWDSCreateProfileRequest_IPCPACKTimeout
{
   UINT16 mTimeoutMilliseconds;
};

// Structure to describe request TLV 0x96 for WDSCreateProfile()
struct sWDSCreateProfileRequest_AuthenticationTimeout
{
   UINT16 mTimeoutMilliseconds;
};

// Structure to describe request TLV 0x97 for WDSCreateProfile()
struct sWDSCreateProfileRequest_LCPConfigRetryCount
{
   UINT8 mRetryCount;
};

// Structure to describe request TLV 0x98 for WDSCreateProfile()
struct sWDSCreateProfileRequest_IPCPConfigRetryCount
{
   UINT8 mRetryCount;
};

// Structure to describe request TLV 0x99 for WDSCreateProfile()
struct sWDSCreateProfileRequest_AuthenticationRetry
{
   UINT8 mRetryCount;
};

// Structure to describe request TLV 0x9A for WDSCreateProfile()
struct sWDSCreateProfileRequest_AuthenticationProtocol
{
   eQMIWDSAuthenticationProtocol mAuthenticationProtocol;
};

// Structure to describe request TLV 0x9B for WDSCreateProfile()
struct sWDSCreateProfileRequest_UserID
{
   // String is variable length, but must be size of the container
   // char mUsername[1];
};

// Structure to describe request TLV 0x9C for WDSCreateProfile()
struct sWDSCreateProfileRequest_AuthenticationPassword
{
   // String is variable length, but must be size of the container
   // char mPassword[1];
};

// Structure to describe request TLV 0x9D for WDSCreateProfile()
struct sWDSCreateProfileRequest_DataRate
{
   eQMIWDSDataRate mDataRate;
};

// Structure to describe request TLV 0x9E for WDSCreateProfile()
struct sWDSCreateProfileRequest_ApplicationType
{
   eQMIWDSApplicationType mApplicationType;
};

// Structure to describe request TLV 0x9F for WDSCreateProfile()
struct sWDSCreateProfileRequest_DataMode
{
   eQMIWDSDataMode mDataMode;
};

// Structure to describe request TLV 0xA0 for WDSCreateProfile()
struct sWDSCreateProfileRequest_ApplicationPriority
{
   UINT8 mApplicationPriority;
};

// Structure to describe request TLV 0xA1 for WDSCreateProfile()
struct sWDSCreateProfileRequest_APNString
{
   // String is variable length, but must be size of the container
   // char mAPNName[1];
};

// Structure to describe request TLV 0xA2 for WDSCreateProfile()
struct sWDSCreateProfileRequest_PDNType
{
   eQMIWDSPDNType mPDNType;
};

// Structure to describe request TLV 0xA3 for WDSCreateProfile()
struct sWDSCreateProfileRequest_PCSCFAddressNeeded
{
   INT8 mPCSCFAddressNeeded;
};

// Structure to describe request TLV 0xA4 for WDSCreateProfile()
struct sWDSCreateProfileRequest_PrimaryIPv4Address
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0xA5 for WDSCreateProfile()
struct sWDSCreateProfileRequest_SecondaryIPv4Address
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0xA6 for WDSCreateProfile()
struct sWDSCreateProfileRequest_PrimaryIPv6Address
{
   UINT16 mIPv6Address[8];
};

// Structure to describe request TLV 0xA7 for WDSCreateProfile()
struct sWDSCreateProfileRequest_SecondaryIPv6Address
{
   UINT16 mIPv6Address[8];
};

// Structure to describe request TLV 0xA8 for WDSCreateProfile()
struct sWDSCreateProfileRequest_RATType
{
   eQMIWDS3GPP2RATTypes mRATType;
};

// Structure to describe request TLV 0xA9 for WDSCreateProfile()
struct sWDSCreateProfileRequest_3GPP2APNEnabled
{
   INT8 mAPNEnabled;
};

// Structure to describe request TLV 0xAA for WDSCreateProfile()
struct sWDSCreateProfileRequest_3GPP2PDNInactivityTimer
{
   UINT32 mPDNInactivityTimerMinutes;
};

// Structure to describe request TLV 0xAB for WDSCreateProfile()
struct sWDSCreateProfileRequest_3GPP2APNClass
{
   UINT8 mAPNClass;
};

// Structure to describe request TLV 0xAD for WDSCreateProfile()
struct sWDSCreateProfileRequest_PDNAuthenticationProtocol
{
   eQMIWDSAuthenticationProtocol mAuthenticationProtocol;
};

// Structure to describe request TLV 0xAE for WDSCreateProfile()
struct sWDSCreateProfileRequest_PDNUserID
{
   // String is variable length, but must be size of the container
   // char mUserID[1];
};

// Structure to describe request TLV 0xAF for WDSCreateProfile()
struct sWDSCreateProfileRequest_PDNPassword
{
   // String is variable length, but must be size of the container
   // char mPassword[1];
};

// Structure to describe request TLV 0xB0 for WDSCreateProfile()
struct sWDSCreateProfileRequest_PDNLabel
{
   // String is variable length, but must be size of the container
   // char mLabel[1];
};

// Structure to describe response TLV 0x01 for WDSCreateProfile()
struct sWDSCreateProfileResponse_ProfileIdentifier
{
   eQMIProfileTypes mProfileType;
   UINT8 mProfileIndex;
};

// Structure to describe response TLV 0xE0 for WDSCreateProfile()
struct sWDSCreateProfileResponse_ExtendedErrorCode
{
   eQMIWDSExtendedErrorCode mExtendedErrorCode;
};

// Structure to describe request TLV 0x01 for WDSModifyProfile()
struct sWDSModifyProfileRequest_ProfileIdentifier
{
   eQMIProfileTypes mProfileType;
   UINT8 mProfileIndex;
};

// Structure to describe request TLV 0x10 for WDSModifyProfile()
struct sWDSModifyProfileRequest_ProfileName
{
   // String is variable length, but must be size of the container
   // char mProfileName[1];
};

// Structure to describe request TLV 0x11 for WDSModifyProfile()
struct sWDSModifyProfileRequest_PDPType
{
   eQMIPDPTypes mPDPType;
};

// Structure to describe request TLV 0x12 for WDSModifyProfile()
struct sWDSModifyProfileRequest_PDPHeaderCompressionType
{
   eQMIWDSPDPHeaderCompressionType mPDPHeaderCompressionType;
};

// Structure to describe request TLV 0x13 for WDSModifyProfile()
struct sWDSModifyProfileRequest_PDPDataCompressionType
{
   eQMIWDSPDPDataCompressionType mPDPDataCompressionType;
};

// Structure to describe request TLV 0x14 for WDSModifyProfile()
struct sWDSModifyProfileRequest_APNName
{
   // String is variable length, but must be size of the container
   // char mAPNName[1];
};

// Structure to describe request TLV 0x15 for WDSModifyProfile()
struct sWDSModifyProfileRequest_PrimaryDNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x16 for WDSModifyProfile()
struct sWDSModifyProfileRequest_SecondaryDNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x17 for WDSModifyProfile()
struct sWDSModifyProfileRequest_UMTSRequestedQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
};

// Structure to describe request TLV 0x18 for WDSModifyProfile()
struct sWDSModifyProfileRequest_UMTSMinimumQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
};

// Structure to describe request TLV 0x19 for WDSModifyProfile()
struct sWDSModifyProfileRequest_GPRSRequestedQoS
{
   UINT32 mPrecedenceClass;
   UINT32 mDelayClass;
   UINT32 mReliabilityClass;
   UINT32 mPeakThroughputClass;
   UINT32 mMeanThroughputClass;
};

// Structure to describe request TLV 0x1A for WDSModifyProfile()
struct sWDSModifyProfileRequest_GPRSMinimumQoS
{
   UINT32 mPrecedenceClass;
   UINT32 mDelayClass;
   UINT32 mReliabilityClass;
   UINT32 mPeakThroughputClass;
   UINT32 mMeanThroughputClass;
};

// Structure to describe request TLV 0x1B for WDSModifyProfile()
struct sWDSModifyProfileRequest_Username
{
   // String is variable length, but must be size of the container
   // char mUsername[1];
};

// Structure to describe request TLV 0x1C for WDSModifyProfile()
struct sWDSModifyProfileRequest_Password
{
   // String is variable length, but must be size of the container
   // char mPassword[1];
};

// Structure to describe request TLV 0x1D for WDSModifyProfile()
struct sWDSModifyProfileRequest_Authentication
{
   bool mEnablePAP:1;
   bool mEnableCHAP:1;

   // Padding out 6 bits
   UINT8 mReserved1:6;
};

// Structure to describe request TLV 0x1E for WDSModifyProfile()
struct sWDSModifyProfileRequest_IPAddress
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x1F for WDSModifyProfile()
struct sWDSModifyProfileRequest_PCSCF
{
   INT8 mPCSCFAddressUsingPCO;
};

// Structure to describe request TLV 0x20 for WDSModifyProfile()
struct sWDSModifyProfileRequest_PDPAccessControlFlag
{
   eQMIWDSPDPAccessControlFlag mPDPAccessControlFlag;
};

// Structure to describe request TLV 0x21 for WDSModifyProfile()
struct sWDSModifyProfileRequest_PCSCFAddressUsingDHCP
{
   INT8 mPCSCFAddressUsingDHCP;
};

// Structure to describe request TLV 0x22 for WDSModifyProfile()
struct sWDSModifyProfileRequest_IMCNFlag
{
   INT8 mIMCN;
};

// Structure to describe request TLV 0x23 for WDSModifyProfile()
struct sWDSModifyProfileRequest_TrafficFlowTemplateID1Parameters
{
   UINT8 mFilterID;
   UINT8 mEvaluationID;
   eQMIWDSIPVersion mIPVersion;

   // The following union is based on the value of mIPVersion
   union uValOfIPVersion
   {
      // If the value of mIPVersion == 4
      struct sIPVersionIs4
      {
         UINT8 mIPV4Address[4];
      };

      sIPVersionIs4 mIPVersionIs4;

      // If the value of mIPVersion == 6
      struct sIPVersionIs6
      {
         UINT16 mIPv6Address[8];
      };

      sIPVersionIs6 mIPVersionIs6;

      // Padding out 128 bits
      UINT8 mReserved1[16];
   };

   uValOfIPVersion mValOfIPVersion;

   UINT8 mSourceIPMask;
   UINT8 mNextHeader;
   UINT16 mDestinationPortRangeStart;
   UINT16 mDestinationPortRangeEnd;
   UINT16 mSourcePortRangeStart;
   UINT16 mSourcePortRangeEnd;
   UINT32 mIPSECSecurityParameterIndex;
   UINT16 mTOSMask;
   UINT32 mFlowLabel;
};

// Structure to describe request TLV 0x24 for WDSModifyProfile()
struct sWDSModifyProfileRequest_TrafficFlowTemplateID2Parameters
{
   UINT8 mFilterID;
   UINT8 mEvaluationID;
   eQMIWDSIPVersion mIPVersion;

   // The following union is based on the value of mIPVersion
   union uValOfIPVersion
   {
      // If the value of mIPVersion == 4
      struct sIPVersionIs4
      {
         UINT8 mIPV4Address[4];
      };

      sIPVersionIs4 mIPVersionIs4;

      // If the value of mIPVersion == 6
      struct sIPVersionIs6
      {
         UINT16 mIPv6Address[8];
      };

      sIPVersionIs6 mIPVersionIs6;

      // Padding out 128 bits
      UINT8 mReserved1[16];
   };

   uValOfIPVersion mValOfIPVersion;

   UINT8 mSourceIPMask;
   UINT8 mNextHeader;
   UINT16 mDestinationPortRangeStart;
   UINT16 mDestinationPortRangeEnd;
   UINT16 mSourcePortRangeStart;
   UINT16 mSourcePortRangeEnd;
   UINT32 mIPSECSecurityParameterIndex;
   UINT16 mTOSMask;
   UINT32 mFlowLabel;
};

// Structure to describe request TLV 0x25 for WDSModifyProfile()
struct sWDSModifyProfileRequest_PDPContextNumber
{
   UINT8 mPDPContextNumber;
};

// Structure to describe request TLV 0x26 for WDSModifyProfile()
struct sWDSModifyProfileRequest_PDPContextSecondaryFlag
{
   INT8 mPDPContextSecondaryFlag;
};

// Structure to describe request TLV 0x27 for WDSModifyProfile()
struct sWDSModifyProfileRequest_PDPContextPrimaryID
{
   UINT8 mPDPPrimaryID;
};

// Structure to describe request TLV 0x28 for WDSModifyProfile()
struct sWDSModifyProfileRequest_IPv6Address
{
   UINT16 mIPv6Address[8];
};

// Structure to describe request TLV 0x29 for WDSModifyProfile()
struct sWDSModifyProfileRequest_RequestedQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
   INT8 mSignalingIndication;
};

// Structure to describe request TLV 0x2A for WDSModifyProfile()
struct sWDSModifyProfileRequest_MinimumQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
   INT8 mSignalingIndication;
};

// Structure to describe request TLV 0x2B for WDSModifyProfile()
struct sWDSModifyProfileRequest_PrimaryIPv6
{
   UINT16 mIPv6Address[8];
};

// Structure to describe request TLV 0x2C for WDSModifyProfile()
struct sWDSModifyProfileRequest_SecondaryIPv6
{
   UINT16 mIPv6Address[8];
};

// Structure to describe request TLV 0x2D for WDSModifyProfile()
struct sWDSModifyProfileRequest_AddressPreference
{
   eQMIWDSAddressAllocationPreference mAddressAllocationPreference;
};

// Structure to describe request TLV 0x2E for WDSModifyProfile()
struct sWDSModifyProfileRequest_LTEQoSParameters
{
   eQMIWDSQoSClassIdentifier mQoSClassIdentifier;
   UINT32 mGuaranteedDownlinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mMaxUplinkBitrate;
};

// Structure to describe request TLV 0x2F for WDSModifyProfile()
struct sWDSModifyProfileRequest_APNDisabled
{
   INT8 mAPNDisabled;
};

// Structure to describe request TLV 0x30 for WDSModifyProfile()
struct sWDSModifyProfileRequest_PDNInactivityTimer
{
   UINT32 mPDNInactivityTimerSeconds;
};

// Structure to describe request TLV 0x31 for WDSModifyProfile()
struct sWDSModifyProfileRequest_APNClass
{
   UINT8 mAPNClass;
};

// Structure to describe request TLV 0x35 for WDSModifyProfile()
struct sWDSModifyProfileRequest_APNBearer
{
   bool mGSM:1;
   bool mWCDMA:1;
   bool mLTE:1;

   // Padding out 60 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2[6];
   UINT8 mReserved3:7;

   bool mAny:1;
};

// Structure to describe request TLV 0x90 for WDSModifyProfile()
struct sWDSModifyProfileRequest_NegotiateDNSServerPrefrence
{
   INT8 mNegotiateDNSServerPreference;
};

// Structure to describe request TLV 0x91 for WDSModifyProfile()
struct sWDSModifyProfileRequest_PPPSessionCloseTimerDO
{
   UINT32 mPPPSessionCloseTimerDOSeconds;
};

// Structure to describe request TLV 0x92 for WDSModifyProfile()
struct sWDSModifyProfileRequest_PPPSessionCloseTimer1X
{
   UINT32 mPPPSessionCloseTimer1XSeconds;
};

// Structure to describe request TLV 0x93 for WDSModifyProfile()
struct sWDSModifyProfileRequest_AllowLinger
{
   INT8 mAllowLinger;
};

// Structure to describe request TLV 0x94 for WDSModifyProfile()
struct sWDSModifyProfileRequest_LCPACKTimeout
{
   UINT16 mTimeoutMilliseconds;
};

// Structure to describe request TLV 0x95 for WDSModifyProfile()
struct sWDSModifyProfileRequest_IPCPACKTimeout
{
   UINT16 mTimeoutMilliseconds;
};

// Structure to describe request TLV 0x96 for WDSModifyProfile()
struct sWDSModifyProfileRequest_AuthenticationTimeout
{
   UINT16 mTimeoutMilliseconds;
};

// Structure to describe request TLV 0x97 for WDSModifyProfile()
struct sWDSModifyProfileRequest_LCPConfigRetryCount
{
   UINT8 mRetryCount;
};

// Structure to describe request TLV 0x98 for WDSModifyProfile()
struct sWDSModifyProfileRequest_IPCPConfigRetryCount
{
   UINT8 mRetryCount;
};

// Structure to describe request TLV 0x99 for WDSModifyProfile()
struct sWDSModifyProfileRequest_AuthenticationRetry
{
   UINT8 mRetryCount;
};

// Structure to describe request TLV 0x9A for WDSModifyProfile()
struct sWDSModifyProfileRequest_AuthenticationProtocol
{
   eQMIWDSAuthenticationProtocol mAuthenticationProtocol;
};

// Structure to describe request TLV 0x9B for WDSModifyProfile()
struct sWDSModifyProfileRequest_UserID
{
   // String is variable length, but must be size of the container
   // char mUsername[1];
};

// Structure to describe request TLV 0x9C for WDSModifyProfile()
struct sWDSModifyProfileRequest_AuthenticationPassword
{
   // String is variable length, but must be size of the container
   // char mPassword[1];
};

// Structure to describe request TLV 0x9D for WDSModifyProfile()
struct sWDSModifyProfileRequest_DataRate
{
   eQMIWDSDataRate mDataRate;
};

// Structure to describe request TLV 0x9E for WDSModifyProfile()
struct sWDSModifyProfileRequest_ApplicationType
{
   eQMIWDSApplicationType mApplicationType;
};

// Structure to describe request TLV 0x9F for WDSModifyProfile()
struct sWDSModifyProfileRequest_DataMode
{
   eQMIWDSDataMode mDataMode;
};

// Structure to describe request TLV 0xA0 for WDSModifyProfile()
struct sWDSModifyProfileRequest_ApplicationPriority
{
   UINT8 mApplicationPriority;
};

// Structure to describe request TLV 0xA1 for WDSModifyProfile()
struct sWDSModifyProfileRequest_APNString
{
   // String is variable length, but must be size of the container
   // char mAPNName[1];
};

// Structure to describe request TLV 0xA2 for WDSModifyProfile()
struct sWDSModifyProfileRequest_PDNType
{
   eQMIWDSPDNType mPDNType;
};

// Structure to describe request TLV 0xA3 for WDSModifyProfile()
struct sWDSModifyProfileRequest_PCSCFAddressNeeded
{
   INT8 mPCSCFAddressNeeded;
};

// Structure to describe request TLV 0xA4 for WDSModifyProfile()
struct sWDSModifyProfileRequest_PrimaryIPv4Address
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0xA5 for WDSModifyProfile()
struct sWDSModifyProfileRequest_SecondaryIPv4Address
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0xA6 for WDSModifyProfile()
struct sWDSModifyProfileRequest_PrimaryIPv6Address
{
   UINT16 mIPv6Address[8];
};

// Structure to describe request TLV 0xA7 for WDSModifyProfile()
struct sWDSModifyProfileRequest_SecondaryIPv6Address
{
   UINT16 mIPv6Address[8];
};

// Structure to describe request TLV 0xA8 for WDSModifyProfile()
struct sWDSModifyProfileRequest_RATType
{
   eQMIWDS3GPP2RATTypes mRATType;
};

// Structure to describe request TLV 0xA9 for WDSModifyProfile()
struct sWDSModifyProfileRequest_3GPP2APNEnabled
{
   INT8 mAPNEnabled;
};

// Structure to describe request TLV 0xAA for WDSModifyProfile()
struct sWDSModifyProfileRequest_3GPP2PDNInactivityTimer
{
   UINT32 mPDNInactivityTimerMinutes;
};

// Structure to describe request TLV 0xAB for WDSModifyProfile()
struct sWDSModifyProfileRequest_3GPP2APNClass
{
   UINT8 mAPNClass;
};

// Structure to describe request TLV 0xAD for WDSModifyProfile()
struct sWDSModifyProfileRequest_PDNAuthenticationProtocol
{
   eQMIWDSAuthenticationProtocol mAuthenticationProtocol;
};

// Structure to describe request TLV 0xAE for WDSModifyProfile()
struct sWDSModifyProfileRequest_PDNUserID
{
   // String is variable length, but must be size of the container
   // char mUserID[1];
};

// Structure to describe request TLV 0xAF for WDSModifyProfile()
struct sWDSModifyProfileRequest_PDNPassword
{
   // String is variable length, but must be size of the container
   // char mPassword[1];
};

// Structure to describe request TLV 0xB0 for WDSModifyProfile()
struct sWDSModifyProfileRequest_PDNLabel
{
   // String is variable length, but must be size of the container
   // char mLabel[1];
};

// Structure to describe response TLV 0xE0 for WDSModifyProfile()
struct sWDSModifyProfileResponse_ExtendedErrorCode
{
   eQMIWDSExtendedErrorCode mExtendedErrorCode;
};

// Structure to describe request TLV 0x01 for WDSDeleteProfile()
struct sWDSDeleteProfileRequest_ProfileIdentifier
{
   eQMIProfileTypes mProfileType;
   UINT8 mProfileIndex;
};

// Structure to describe response TLV 0xE0 for WDSDeleteProfile()
struct sWDSDeleteProfileResponse_ExtendedErrorCode
{
   eQMIWDSExtendedErrorCode mExtendedErrorCode;
};

// Structure to describe request TLV 0x10 for WDSGetProfileList()
struct sWDSGetProfileListRequest_ProfileName
{
   // String is variable length, but must be size of the container
   // char mProfileName[1];
};

// Structure to describe request TLV 0x11 for WDSGetProfileList()
struct sWDSGetProfileListRequest_PDPType
{
   eQMIPDPTypes mPDPType;
};

// Structure to describe request TLV 0x12 for WDSGetProfileList()
struct sWDSGetProfileListRequest_PDPHeaderCompressionType
{
   eQMIWDSPDPHeaderCompressionType mPDPHeaderCompressionType;
};

// Structure to describe request TLV 0x13 for WDSGetProfileList()
struct sWDSGetProfileListRequest_PDPDataCompressionType
{
   eQMIWDSPDPDataCompressionType mPDPDataCompressionType;
};

// Structure to describe request TLV 0x14 for WDSGetProfileList()
struct sWDSGetProfileListRequest_APNName
{
   // String is variable length, but must be size of the container
   // char mAPNName[1];
};

// Structure to describe request TLV 0x15 for WDSGetProfileList()
struct sWDSGetProfileListRequest_PrimaryDNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x16 for WDSGetProfileList()
struct sWDSGetProfileListRequest_SecondaryDNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x17 for WDSGetProfileList()
struct sWDSGetProfileListRequest_UMTSRequestedQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
};

// Structure to describe request TLV 0x18 for WDSGetProfileList()
struct sWDSGetProfileListRequest_UMTSMinimumQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
};

// Structure to describe request TLV 0x19 for WDSGetProfileList()
struct sWDSGetProfileListRequest_GPRSRequestedQoS
{
   UINT32 mPrecedenceClass;
   UINT32 mDelayClass;
   UINT32 mReliabilityClass;
   UINT32 mPeakThroughputClass;
   UINT32 mMeanThroughputClass;
};

// Structure to describe request TLV 0x1A for WDSGetProfileList()
struct sWDSGetProfileListRequest_GPRSMinimumQoS
{
   UINT32 mPrecedenceClass;
   UINT32 mDelayClass;
   UINT32 mReliabilityClass;
   UINT32 mPeakThroughputClass;
   UINT32 mMeanThroughputClass;
};

// Structure to describe request TLV 0x1B for WDSGetProfileList()
struct sWDSGetProfileListRequest_Username
{
   // String is variable length, but must be size of the container
   // char mUsername[1];
};

// Structure to describe request TLV 0x1C for WDSGetProfileList()
struct sWDSGetProfileListRequest_Password
{
   // String is variable length, but must be size of the container
   // char mPassword[1];
};

// Structure to describe request TLV 0x1D for WDSGetProfileList()
struct sWDSGetProfileListRequest_Authentication
{
   bool mEnablePAP:1;
   bool mEnableCHAP:1;

   // Padding out 6 bits
   UINT8 mReserved1:6;
};

// Structure to describe request TLV 0x1E for WDSGetProfileList()
struct sWDSGetProfileListRequest_IPAddress
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x1F for WDSGetProfileList()
struct sWDSGetProfileListRequest_PCSCF
{
   INT8 mPCSCFAddressUsingPCO;
};

// Structure to describe request TLV 0x20 for WDSGetProfileList()
struct sWDSGetProfileListRequest_PDPAccessControlFlag
{
   eQMIWDSPDPAccessControlFlag mPDPAccessControlFlag;
};

// Structure to describe request TLV 0x21 for WDSGetProfileList()
struct sWDSGetProfileListRequest_PCSCFAddressUsingDHCP
{
   INT8 mPCSCFAddressUsingDHCP;
};

// Structure to describe request TLV 0x22 for WDSGetProfileList()
struct sWDSGetProfileListRequest_IMCNFlag
{
   INT8 mIMCN;
};

// Structure to describe request TLV 0x23 for WDSGetProfileList()
struct sWDSGetProfileListRequest_TrafficFlowTemplateID1Parameters
{
   UINT8 mFilterID;
   UINT8 mEvaluationID;
   eQMIWDSIPVersion mIPVersion;

   // The following union is based on the value of mIPVersion
   union uValOfIPVersion
   {
      // If the value of mIPVersion == 4
      struct sIPVersionIs4
      {
         UINT8 mIPV4Address[4];
      };

      sIPVersionIs4 mIPVersionIs4;

      // If the value of mIPVersion == 6
      struct sIPVersionIs6
      {
         UINT16 mIPv6Address[8];
      };

      sIPVersionIs6 mIPVersionIs6;

      // Padding out 128 bits
      UINT8 mReserved1[16];
   };

   uValOfIPVersion mValOfIPVersion;

   UINT8 mSourceIPMask;
   UINT8 mNextHeader;
   UINT16 mDestinationPortRangeStart;
   UINT16 mDestinationPortRangeEnd;
   UINT16 mSourcePortRangeStart;
   UINT16 mSourcePortRangeEnd;
   UINT32 mIPSECSecurityParameterIndex;
   UINT16 mTOSMask;
   UINT32 mFlowLabel;
};

// Structure to describe request TLV 0x24 for WDSGetProfileList()
struct sWDSGetProfileListRequest_TrafficFlowTemplateID2Parameters
{
   UINT8 mFilterID;
   UINT8 mEvaluationID;
   eQMIWDSIPVersion mIPVersion;

   // The following union is based on the value of mIPVersion
   union uValOfIPVersion
   {
      // If the value of mIPVersion == 4
      struct sIPVersionIs4
      {
         UINT8 mIPV4Address[4];
      };

      sIPVersionIs4 mIPVersionIs4;

      // If the value of mIPVersion == 6
      struct sIPVersionIs6
      {
         UINT16 mIPv6Address[8];
      };

      sIPVersionIs6 mIPVersionIs6;

      // Padding out 128 bits
      UINT8 mReserved1[16];
   };

   uValOfIPVersion mValOfIPVersion;

   UINT8 mSourceIPMask;
   UINT8 mNextHeader;
   UINT16 mDestinationPortRangeStart;
   UINT16 mDestinationPortRangeEnd;
   UINT16 mSourcePortRangeStart;
   UINT16 mSourcePortRangeEnd;
   UINT32 mIPSECSecurityParameterIndex;
   UINT16 mTOSMask;
   UINT32 mFlowLabel;
};

// Structure to describe request TLV 0x25 for WDSGetProfileList()
struct sWDSGetProfileListRequest_PDPContextNumber
{
   UINT8 mPDPContextNumber;
};

// Structure to describe request TLV 0x26 for WDSGetProfileList()
struct sWDSGetProfileListRequest_PDPContextSecondaryFlag
{
   INT8 mPDPContextSecondaryFlag;
};

// Structure to describe request TLV 0x27 for WDSGetProfileList()
struct sWDSGetProfileListRequest_PDPContextPrimaryID
{
   UINT8 mPDPPrimaryID;
};

// Structure to describe request TLV 0x28 for WDSGetProfileList()
struct sWDSGetProfileListRequest_IPv6Address
{
   UINT16 mIPv6Address[8];
};

// Structure to describe request TLV 0x29 for WDSGetProfileList()
struct sWDSGetProfileListRequest_RequestedQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
   INT8 mSignalingIndication;
};

// Structure to describe request TLV 0x2A for WDSGetProfileList()
struct sWDSGetProfileListRequest_MinimumQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
   INT8 mSignalingIndication;
};

// Structure to describe request TLV 0x2B for WDSGetProfileList()
struct sWDSGetProfileListRequest_PrimaryIPv6
{
   UINT16 mIPv6Address[8];
};

// Structure to describe request TLV 0x2C for WDSGetProfileList()
struct sWDSGetProfileListRequest_SecondaryIPv6
{
   UINT16 mIPv6Address[8];
};

// Structure to describe request TLV 0x2D for WDSGetProfileList()
struct sWDSGetProfileListRequest_AddressPreference
{
   eQMIWDSAddressAllocationPreference mAddressAllocationPreference;
};

// Structure to describe request TLV 0x2E for WDSGetProfileList()
struct sWDSGetProfileListRequest_LTEQoSParameters
{
   eQMIWDSQoSClassIdentifier mQoSClassIdentifier;
   UINT32 mGuaranteedDownlinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mMaxUplinkBitrate;
};

// Structure to describe request TLV 0x90 for WDSGetProfileList()
struct sWDSGetProfileListRequest_NegotiateDNSServerPreferences
{
   INT8 mNegotiateDNSServerPreference;
};

// Structure to describe request TLV 0x91 for WDSGetProfileList()
struct sWDSGetProfileListRequest_PPPSessionCloseTimerDO
{
   UINT32 mPPPSessionCloseTimerDOSeconds;
};

// Structure to describe request TLV 0x92 for WDSGetProfileList()
struct sWDSGetProfileListRequest_PPPSessionCloseTimer1X
{
   UINT32 mPPPSessionCloseTimer1XSeconds;
};

// Structure to describe request TLV 0x93 for WDSGetProfileList()
struct sWDSGetProfileListRequest_AllowLinger
{
   INT8 mAllowLinger;
};

// Structure to describe request TLV 0x94 for WDSGetProfileList()
struct sWDSGetProfileListRequest_LCPACKTimeout
{
   UINT16 mTimeoutMilliseconds;
};

// Structure to describe request TLV 0x95 for WDSGetProfileList()
struct sWDSGetProfileListRequest_IPCPACKTimeout
{
   UINT16 mTimeoutMilliseconds;
};

// Structure to describe request TLV 0x96 for WDSGetProfileList()
struct sWDSGetProfileListRequest_AuthenticationTimeout
{
   UINT16 mTimeoutMilliseconds;
};

// Structure to describe request TLV 0x97 for WDSGetProfileList()
struct sWDSGetProfileListRequest_LCPConfigRetryCount
{
   UINT8 mRetryCount;
};

// Structure to describe request TLV 0x98 for WDSGetProfileList()
struct sWDSGetProfileListRequest_IPCPConfigRetryCount
{
   UINT8 mRetryCount;
};

// Structure to describe request TLV 0x99 for WDSGetProfileList()
struct sWDSGetProfileListRequest_AuthenticationRetry
{
   UINT8 mRetryCount;
};

// Structure to describe request TLV 0x9A for WDSGetProfileList()
struct sWDSGetProfileListRequest_AuthenticationProtocol
{
   eQMIWDSAuthenticationProtocol mAuthenticationProtocol;
};

// Structure to describe request TLV 0x9B for WDSGetProfileList()
struct sWDSGetProfileListRequest_UserID
{
   // String is variable length, but must be size of the container
   // char mUsername[1];
};

// Structure to describe request TLV 0x9C for WDSGetProfileList()
struct sWDSGetProfileListRequest_AuthenticationPassword
{
   // String is variable length, but must be size of the container
   // char mPassword[1];
};

// Structure to describe request TLV 0x9D for WDSGetProfileList()
struct sWDSGetProfileListRequest_DataRate
{
   eQMIWDSDataRate mDataRate;
};

// Structure to describe request TLV 0x9E for WDSGetProfileList()
struct sWDSGetProfileListRequest_ApplicationType
{
   eQMIWDSApplicationType mApplicationType;
};

// Structure to describe request TLV 0x9F for WDSGetProfileList()
struct sWDSGetProfileListRequest_DataMode
{
   eQMIWDSDataMode mDataMode;
};

// Structure to describe request TLV 0xA0 for WDSGetProfileList()
struct sWDSGetProfileListRequest_ApplicationPriority
{
   UINT8 mApplicationPriority;
};

// Structure to describe request TLV 0xA1 for WDSGetProfileList()
struct sWDSGetProfileListRequest_APNString
{
   // String is variable length, but must be size of the container
   // char mAPNName[1];
};

// Structure to describe request TLV 0xA2 for WDSGetProfileList()
struct sWDSGetProfileListRequest_PDNType
{
   eQMIWDSPDNType mPDNType;
};

// Structure to describe request TLV 0xA3 for WDSGetProfileList()
struct sWDSGetProfileListRequest_PCSCFAddressNeeded
{
   INT8 mPCSCFAddressNeeded;
};

// Structure to describe request TLV 0xA4 for WDSGetProfileList()
struct sWDSGetProfileListRequest_PrimaryIPv4Address
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0xA5 for WDSGetProfileList()
struct sWDSGetProfileListRequest_SecondaryIPv4Address
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0xA6 for WDSGetProfileList()
struct sWDSGetProfileListRequest_PrimaryIPv6Address
{
   UINT16 mIPv6Address[8];
};

// Structure to describe request TLV 0xA7 for WDSGetProfileList()
struct sWDSGetProfileListRequest_SecondaryIPv6Address
{
   UINT16 mIPv6Address[8];
};

// Structure to describe response TLV 0x01 for WDSGetProfileList()
struct sWDSGetProfileListResponse_ProfileList
{
   UINT8 mNumberOfProfiles;

   struct sProfile
   {
      eQMIProfileTypes mProfileType;
      UINT8 mProfileIndex;
      UINT8 mProfileNameLength;
   
      // This array must be the size specified by mProfileNameLength
      // char mProfileName[1];
   };

   // This array must be the size specified by mNumberOfProfiles
   // sProfile mProfiles[1];
};

// Structure to describe response TLV 0xE0 for WDSGetProfileList()
struct sWDSGetProfileListResponse_ExtendedErrorCode
{
   eQMIWDSExtendedErrorCode mExtendedErrorCode;
};

// Structure to describe request TLV 0x01 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsRequest_ProfileIdentifier
{
   eQMIProfileTypes mProfileType;
   UINT8 mProfileIndex;
};

// Structure to describe response TLV 0x10 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_ProfileName
{
   // String is variable length, but must be size of the container
   // char mProfileName[1];
};

// Structure to describe response TLV 0x11 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PDPType
{
   eQMIPDPTypes mPDPType;
};

// Structure to describe response TLV 0x12 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PDPHeaderCompressionType
{
   eQMIWDSPDPHeaderCompressionType mPDPHeaderCompressionType;
};

// Structure to describe response TLV 0x13 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PDPDataCompressionType
{
   eQMIWDSPDPDataCompressionType mPDPDataCompressionType;
};

// Structure to describe response TLV 0x14 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_APNName
{
   // String is variable length, but must be size of the container
   // char mAPNName[1];
};

// Structure to describe response TLV 0x15 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PrimaryDNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0x16 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_SecondaryDNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0x17 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_UMTSRequestedQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
};

// Structure to describe response TLV 0x18 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_UMTSMinimumQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
};

// Structure to describe response TLV 0x19 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_GPRSRequestedQoS
{
   UINT32 mPrecedenceClass;
   UINT32 mDelayClass;
   UINT32 mReliabilityClass;
   UINT32 mPeakThroughputClass;
   UINT32 mMeanThroughputClass;
};

// Structure to describe response TLV 0x1A for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_GPRSMinimumQoS
{
   UINT32 mPrecedenceClass;
   UINT32 mDelayClass;
   UINT32 mReliabilityClass;
   UINT32 mPeakThroughputClass;
   UINT32 mMeanThroughputClass;
};

// Structure to describe response TLV 0x1B for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_Username
{
   // String is variable length, but must be size of the container
   // char mUsername[1];
};

// Structure to describe response TLV 0x1D for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_Authentication
{
   bool mEnablePAP:1;
   bool mEnableCHAP:1;

   // Padding out 6 bits
   UINT8 mReserved1:6;
};

// Structure to describe response TLV 0x1E for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_IPAddress
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0x1F for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PCSCF
{
   INT8 mPCSCFAddressUsingPCO;
};

// Structure to describe response TLV 0x20 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PDPAccessControlFlag
{
   eQMIWDSPDPAccessControlFlag mPDPAccessControlFlag;
};

// Structure to describe response TLV 0x21 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PCSCFAddressUsingDHCP
{
   INT8 mPCSCFAddressUsingDHCP;
};

// Structure to describe response TLV 0x22 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_IMCMFlag
{
   INT8 mIMCN;
};

// Structure to describe response TLV 0x23 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_TrafficFlowTemplateID1Parameters
{
   UINT8 mFilterID;
   UINT8 mEvaluationID;
   eQMIWDSIPVersion mIPVersion;

   // The following union is based on the value of mIPVersion
   union uValOfIPVersion
   {
      // If the value of mIPVersion == 4
      struct sIPVersionIs4
      {
         UINT8 mIPV4Address[4];
      };

      sIPVersionIs4 mIPVersionIs4;

      // If the value of mIPVersion == 6
      struct sIPVersionIs6
      {
         UINT16 mIPv6Address[8];
      };

      sIPVersionIs6 mIPVersionIs6;

      // Padding out 128 bits
      UINT8 mReserved1[16];
   };

   uValOfIPVersion mValOfIPVersion;

   UINT8 mSourceIPMask;
   UINT8 mNextHeader;
   UINT16 mDestinationPortRangeStart;
   UINT16 mDestinationPortRangeEnd;
   UINT16 mSourcePortRangeStart;
   UINT16 mSourcePortRangeEnd;
   UINT32 mIPSECSecurityParameterIndex;
   UINT16 mTOSMask;
   UINT32 mFlowLabel;
};

// Structure to describe response TLV 0x24 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_TrafficFlowTemplateID2Parameters
{
   UINT8 mFilterID;
   UINT8 mEvaluationID;
   eQMIWDSIPVersion mIPVersion;

   // The following union is based on the value of mIPVersion
   union uValOfIPVersion
   {
      // If the value of mIPVersion == 4
      struct sIPVersionIs4
      {
         UINT8 mIPV4Address[4];
      };

      sIPVersionIs4 mIPVersionIs4;

      // If the value of mIPVersion == 6
      struct sIPVersionIs6
      {
         UINT16 mIPv6Address[8];
      };

      sIPVersionIs6 mIPVersionIs6;

      // Padding out 128 bits
      UINT8 mReserved1[16];
   };

   uValOfIPVersion mValOfIPVersion;

   UINT8 mSourceIPMask;
   UINT8 mNextHeader;
   UINT16 mDestinationPortRangeStart;
   UINT16 mDestinationPortRangeEnd;
   UINT16 mSourcePortRangeStart;
   UINT16 mSourcePortRangeEnd;
   UINT32 mIPSECSecurityParameterIndex;
   UINT16 mTOSMask;
   UINT32 mFlowLabel;
};

// Structure to describe response TLV 0x25 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PDPContextNumber
{
   UINT8 mPDPContextNumber;
};

// Structure to describe response TLV 0x26 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PDPContextSecondaryFlag
{
   INT8 mPDPContextSecondaryFlag;
};

// Structure to describe response TLV 0x27 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PDPContextPrimaryID
{
   UINT8 mPDPPrimaryID;
};

// Structure to describe response TLV 0x28 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_IPv6Address
{
   UINT16 mIPv6Address[8];
};

// Structure to describe response TLV 0x29 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_RequestedQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
   INT8 mSignalingIndication;
};

// Structure to describe response TLV 0x2A for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_MinimumQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
   INT8 mSignalingIndication;
};

// Structure to describe response TLV 0x2B for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PrimaryIPv6
{
   UINT16 mIPv6Address[8];
};

// Structure to describe response TLV 0x2C for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_SecondaryIPv6
{
   UINT16 mIPv6Address[8];
};

// Structure to describe response TLV 0x2D for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_AddressPreference
{
   eQMIWDSAddressAllocationPreference mAddressAllocationPreference;
};

// Structure to describe response TLV 0x2E for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_LTEQoSParameters
{
   eQMIWDSQoSClassIdentifier mQoSClassIdentifier;
   UINT32 mGuaranteedDownlinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mMaxUplinkBitrate;
};

// Structure to describe response TLV 0x2F for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_APNDisabled
{
   INT8 mAPNDisabled;
};

// Structure to describe response TLV 0x30 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PDNInactivityTimer
{
   UINT32 mPDNInactivityTimerSeconds;
};

// Structure to describe response TLV 0x31 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_APNClass
{
   UINT8 mAPNClass;
};

// Structure to describe response TLV 0x35 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_APNBearer
{
   bool mGSM:1;
   bool mWCDMA:1;
   bool mLTE:1;

   // Padding out 60 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2[6];
   UINT8 mReserved3:7;

   bool mAny:1;
};

// Structure to describe response TLV 0x90 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_NegotiateDNSServerPreference
{
   INT8 mNegotiateDNSServerPreference;
};

// Structure to describe response TLV 0x91 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PPPSessionCloseTimerDO
{
   UINT32 mPPPSessionCloseTimerDOSeconds;
};

// Structure to describe response TLV 0x92 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PPPSessionCloseTimer1X
{
   UINT32 mPPPSessionCloseTimer1XSeconds;
};

// Structure to describe response TLV 0x93 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_AllowLinger
{
   INT8 mAllowLinger;
};

// Structure to describe response TLV 0x94 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_LCPACKTimeout
{
   UINT16 mTimeoutMilliseconds;
};

// Structure to describe response TLV 0x95 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_IPCPACKTimeout
{
   UINT16 mTimeoutMilliseconds;
};

// Structure to describe response TLV 0x96 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_AuthenticationTimeout
{
   UINT16 mTimeoutMilliseconds;
};

// Structure to describe response TLV 0x97 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_LCPConfigRetryCount
{
   UINT8 mRetryCount;
};

// Structure to describe response TLV 0x98 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_IPCPConfigRetryCount
{
   UINT8 mRetryCount;
};

// Structure to describe response TLV 0x99 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_AuthenticationRetry
{
   UINT8 mRetryCount;
};

// Structure to describe response TLV 0x9A for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_AuthenticationProtocol
{
   eQMIWDSAuthenticationProtocol mAuthenticationProtocol;
};

// Structure to describe response TLV 0x9B for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_UserID
{
   // String is variable length, but must be size of the container
   // char mUsername[1];
};

// Structure to describe response TLV 0x9C for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_AuthenticationPassword
{
   // String is variable length, but must be size of the container
   // char mPassword[1];
};

// Structure to describe response TLV 0x9D for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_DataRate
{
   eQMIWDSDataRate mDataRate;
};

// Structure to describe response TLV 0x9E for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_ApplicationType
{
   eQMIWDSApplicationType mApplicationType;
};

// Structure to describe response TLV 0x9F for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_DataMode
{
   eQMIWDSDataMode mDataMode;
};

// Structure to describe response TLV 0xA0 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_ApplicationPriority
{
   UINT8 mApplicationPriority;
};

// Structure to describe response TLV 0xA1 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_APNString
{
   // String is variable length, but must be size of the container
   // char mAPNName[1];
};

// Structure to describe response TLV 0xA2 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PDNType
{
   eQMIWDSPDNType mPDNType;
};

// Structure to describe response TLV 0xA3 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PCSCFAddressNeeded
{
   INT8 mPCSCFAddressNeeded;
};

// Structure to describe response TLV 0xA4 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PrimaryIPv4Address
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0xA5 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_SecondaryIPv4Address
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0xA6 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PrimaryIPv6Address
{
   UINT16 mIPv6Address[8];
};

// Structure to describe response TLV 0xA7 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_SecondaryIPv6Address
{
   UINT16 mIPv6Address[8];
};

// Structure to describe response TLV 0xA8 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_RATType
{
   eQMIWDS3GPP2RATTypes mRATType;
};

// Structure to describe response TLV 0xA9 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_3GPP2APNEnabled
{
   INT8 mAPNEnabled;
};

// Structure to describe response TLV 0xAA for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_3GPP2PDNInactivityTimer
{
   UINT32 mPDNInactivityTimerMinutes;
};

// Structure to describe response TLV 0xAB for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_3GPP2APNClass
{
   UINT8 mAPNClass;
};

// Structure to describe response TLV 0xAD for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PDNAuthenticationProtocol
{
   eQMIWDSAuthenticationProtocol mAuthenticationProtocol;
};

// Structure to describe response TLV 0xAE for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PDNUserID
{
   // String is variable length, but must be size of the container
   // char mUserID[1];
};

// Structure to describe response TLV 0xAF for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PDNPassword
{
   // String is variable length, but must be size of the container
   // char mPassword[1];
};

// Structure to describe response TLV 0xB0 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_PDNLabel
{
   // String is variable length, but must be size of the container
   // char mLabel[1];
};

// Structure to describe response TLV 0xE0 for WDSGetProfileSettings()
struct sWDSGetProfileSettingsResponse_ExtendedErrorCode
{
   eQMIWDSExtendedErrorCode mExtendedErrorCode;
};

// Structure to describe request TLV 0x01 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsRequest_ProfileType
{
   eQMIProfileTypes mProfileType;
};

// Structure to describe response TLV 0x10 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_ProfileName
{
   // String is variable length, but must be size of the container
   // char mProfileName[1];
};

// Structure to describe response TLV 0x11 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PDPType
{
   eQMIPDPTypes mPDPType;
};

// Structure to describe response TLV 0x12 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PDPHeaderCompressionType
{
   eQMIWDSPDPHeaderCompressionType mPDPHeaderCompressionType;
};

// Structure to describe response TLV 0x13 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PDPDataCompressionType
{
   eQMIWDSPDPDataCompressionType mPDPDataCompressionType;
};

// Structure to describe response TLV 0x14 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_APNName
{
   // String is variable length, but must be size of the container
   // char mAPNName[1];
};

// Structure to describe response TLV 0x15 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PrimaryDNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0x16 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_SecondaryDNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0x17 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_UMTSRequestedQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
};

// Structure to describe response TLV 0x18 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_UMTSMinimumQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
};

// Structure to describe response TLV 0x19 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_GPRSRequestedQoS
{
   UINT32 mPrecedenceClass;
   UINT32 mDelayClass;
   UINT32 mReliabilityClass;
   UINT32 mPeakThroughputClass;
   UINT32 mMeanThroughputClass;
};

// Structure to describe response TLV 0x1A for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_GPRSMinimumQoS
{
   UINT32 mPrecedenceClass;
   UINT32 mDelayClass;
   UINT32 mReliabilityClass;
   UINT32 mPeakThroughputClass;
   UINT32 mMeanThroughputClass;
};

// Structure to describe response TLV 0x1B for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_Username
{
   // String is variable length, but must be size of the container
   // char mUsername[1];
};

// Structure to describe response TLV 0x1C for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_Password
{
   // String is variable length, but must be size of the container
   // char mPassword[1];
};

// Structure to describe response TLV 0x1D for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_Authentication
{
   bool mEnablePAP:1;
   bool mEnableCHAP:1;

   // Padding out 6 bits
   UINT8 mReserved1:6;
};

// Structure to describe response TLV 0x1E for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_IPAddress
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0x1F for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PCSCF
{
   INT8 mPCSCFAddressUsingPCO;
};

// Structure to describe response TLV 0x20 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PDPAccessControlFlag
{
   eQMIWDSPDPAccessControlFlag mPDPAccessControlFlag;
};

// Structure to describe response TLV 0x21 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PCSCFAddressUsingDHCP
{
   INT8 mPCSCFAddressUsingDHCP;
};

// Structure to describe response TLV 0x22 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_IMCNFlag
{
   INT8 mIMCN;
};

// Structure to describe response TLV 0x23 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_TrafficFlowTemplateID1Parameters
{
   UINT8 mFilterID;
   UINT8 mEvaluationID;
   eQMIWDSIPVersion mIPVersion;

   // The following union is based on the value of mIPVersion
   union uValOfIPVersion
   {
      // If the value of mIPVersion == 4
      struct sIPVersionIs4
      {
         UINT8 mIPV4Address[4];
      };

      sIPVersionIs4 mIPVersionIs4;

      // If the value of mIPVersion == 6
      struct sIPVersionIs6
      {
         UINT16 mIPv6Address[8];
      };

      sIPVersionIs6 mIPVersionIs6;

      // Padding out 128 bits
      UINT8 mReserved1[16];
   };

   uValOfIPVersion mValOfIPVersion;

   UINT8 mSourceIPMask;
   UINT8 mNextHeader;
   UINT16 mDestinationPortRangeStart;
   UINT16 mDestinationPortRangeEnd;
   UINT16 mSourcePortRangeStart;
   UINT16 mSourcePortRangeEnd;
   UINT32 mIPSECSecurityParameterIndex;
   UINT16 mTOSMask;
   UINT32 mFlowLabel;
};

// Structure to describe response TLV 0x24 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_TrafficFlowTemplateID2Parameters
{
   UINT8 mFilterID;
   UINT8 mEvaluationID;
   eQMIWDSIPVersion mIPVersion;

   // The following union is based on the value of mIPVersion
   union uValOfIPVersion
   {
      // If the value of mIPVersion == 4
      struct sIPVersionIs4
      {
         UINT8 mIPV4Address[4];
      };

      sIPVersionIs4 mIPVersionIs4;

      // If the value of mIPVersion == 6
      struct sIPVersionIs6
      {
         UINT16 mIPv6Address[8];
      };

      sIPVersionIs6 mIPVersionIs6;

      // Padding out 128 bits
      UINT8 mReserved1[16];
   };

   uValOfIPVersion mValOfIPVersion;

   UINT8 mSourceIPMask;
   UINT8 mNextHeader;
   UINT16 mDestinationPortRangeStart;
   UINT16 mDestinationPortRangeEnd;
   UINT16 mSourcePortRangeStart;
   UINT16 mSourcePortRangeEnd;
   UINT32 mIPSECSecurityParameterIndex;
   UINT16 mTOSMask;
   UINT32 mFlowLabel;
};

// Structure to describe response TLV 0x25 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PDPContextNumber
{
   UINT8 mPDPContextNumber;
};

// Structure to describe response TLV 0x26 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PDPContextSecondaryFlag
{
   INT8 mPDPContextSecondaryFlag;
};

// Structure to describe response TLV 0x27 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PDPContextPrimaryID
{
   UINT8 mPDPPrimaryID;
};

// Structure to describe response TLV 0x28 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_IPv6Address
{
   UINT16 mIPv6Address[8];
};

// Structure to describe response TLV 0x29 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_RequestedQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
   INT8 mSignalingIndication;
};

// Structure to describe response TLV 0x2A for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_MinimumQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
   INT8 mSignalingIndication;
};

// Structure to describe response TLV 0x2B for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PrimaryIPv6
{
   UINT16 mIPv6Address[8];
};

// Structure to describe response TLV 0x2C for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_SecondaryIPv6
{
   UINT16 mIPv6Address[8];
};

// Structure to describe response TLV 0x2D for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_AddressPreference
{
   eQMIWDSAddressAllocationPreference mAddressAllocationPreference;
};

// Structure to describe response TLV 0x2E for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_LTEQoSParameters
{
   eQMIWDSQoSClassIdentifier mQoSClassIdentifier;
   UINT32 mGuaranteedDownlinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mMaxUplinkBitrate;
};

// Structure to describe response TLV 0x2F for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_APNDisabled
{
   INT8 mAPNDisabled;
};

// Structure to describe response TLV 0x30 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PDNInactivityTimer
{
   UINT32 mPDNInactivityTimerSeconds;
};

// Structure to describe response TLV 0x31 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_APNClass
{
   UINT8 mAPNClass;
};

// Structure to describe response TLV 0x35 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_APNBearer
{
   bool mGSM:1;
   bool mWCDMA:1;
   bool mLTE:1;

   // Padding out 60 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2[6];
   UINT8 mReserved3:7;

   bool mAny:1;
};

// Structure to describe response TLV 0x90 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_NegotiateDNSServerPreferences
{
   INT8 mNegotiateDNSServerPreference;
};

// Structure to describe response TLV 0x91 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PPPSessionCloseTimerDO
{
   UINT32 mPPPSessionCloseTimerDOSeconds;
};

// Structure to describe response TLV 0x92 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PPPSessionCloseTimer1X
{
   UINT32 mPPPSessionCloseTimer1XSeconds;
};

// Structure to describe response TLV 0x93 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_AllowLinger
{
   INT8 mAllowLinger;
};

// Structure to describe response TLV 0x94 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_LCPACKTimeout
{
   UINT16 mTimeoutMilliseconds;
};

// Structure to describe response TLV 0x95 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_IPCPACKTimeout
{
   UINT16 mTimeoutMilliseconds;
};

// Structure to describe response TLV 0x96 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_AuthenticationTimeout
{
   UINT16 mTimeoutMilliseconds;
};

// Structure to describe response TLV 0x97 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_LCPConfigRetryCount
{
   UINT8 mRetryCount;
};

// Structure to describe response TLV 0x98 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_IPCPConfigRetryCount
{
   UINT8 mRetryCount;
};

// Structure to describe response TLV 0x99 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_AuthenticationRetry
{
   UINT8 mRetryCount;
};

// Structure to describe response TLV 0x9A for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_AuthenticationProtocol
{
   eQMIWDSAuthenticationProtocol mAuthenticationProtocol;
};

// Structure to describe response TLV 0x9B for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_UserID
{
   // String is variable length, but must be size of the container
   // char mUsername[1];
};

// Structure to describe response TLV 0x9C for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_AuthenticationPassword
{
   // String is variable length, but must be size of the container
   // char mPassword[1];
};

// Structure to describe response TLV 0x9D for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_DataRate
{
   eQMIWDSDataRate mDataRate;
};

// Structure to describe response TLV 0x9E for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_ApplicationType
{
   eQMIWDSApplicationType mApplicationType;
};

// Structure to describe response TLV 0x9F for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_DataMode
{
   eQMIWDSDataMode mDataMode;
};

// Structure to describe response TLV 0xA0 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_ApplicationPriority
{
   UINT8 mApplicationPriority;
};

// Structure to describe response TLV 0xA1 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_APNString
{
   // String is variable length, but must be size of the container
   // char mAPNName[1];
};

// Structure to describe response TLV 0xA2 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PDNType
{
   eQMIWDSPDNType mPDNType;
};

// Structure to describe response TLV 0xA3 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PCSCFAddressNeeded
{
   INT8 mPCSCFAddressNeeded;
};

// Structure to describe response TLV 0xA4 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PrimaryIPv4Address
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0xA5 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_SecondaryIPv4Address
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0xA6 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PrimaryIPv6Address
{
   UINT16 mIPv6Address[8];
};

// Structure to describe response TLV 0xA7 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_SecondaryIPv6Address
{
   UINT16 mIPv6Address[8];
};

// Structure to describe response TLV 0xA8 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_RATType
{
   eQMIWDS3GPP2RATTypes mRATType;
};

// Structure to describe response TLV 0xA9 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_3GPP2APNEnabled
{
   INT8 mAPNEnabled;
};

// Structure to describe response TLV 0xAA for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_3GPP2PDNInactivityTimer
{
   UINT32 mPDNInactivityTimerMinutes;
};

// Structure to describe response TLV 0xAB for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_3GPP2APNClass
{
   UINT8 mAPNClass;
};

// Structure to describe response TLV 0xAD for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PDNAuthenticationProtocol
{
   eQMIWDSAuthenticationProtocol mAuthenticationProtocol;
};

// Structure to describe response TLV 0xAE for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PDNUserID
{
   // String is variable length, but must be size of the container
   // char mUserID[1];
};

// Structure to describe response TLV 0xAF for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PDNPassword
{
   // String is variable length, but must be size of the container
   // char mPassword[1];
};

// Structure to describe response TLV 0xB0 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_PDNLabel
{
   // String is variable length, but must be size of the container
   // char mLabel[1];
};

// Structure to describe response TLV 0xE0 for WDSGetDefaultSettings()
struct sWDSGetDefaultSettingsResponse_ExtendedErrorCode
{
   eQMIWDSExtendedErrorCode mExtendedErrorCode;
};

// Structure to describe request TLV 0x10 for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsRequest_RequestedSettings
{
   bool mProfileID:1;
   bool mProfileName:1;
   bool mPDPType:1;
   bool mAPNName:1;
   bool mDNSAddress:1;
   bool mGrantedQoS:1;
   bool mUsername:1;
   bool mAuthenticationProtocol:1;
   bool mIPAddress:1;
   bool mGatewayInfo:1;
   bool mPCSCFAddress:1;
   bool mPCSCFServerAddressList:1;
   bool mPCSCFDomainNameList:1;
   bool mMTU:1;
   bool mDomainNameList:1;
   bool mIPFamily:1;
   bool mIMCNFlag:1;
   bool mExtendedTechnology:1;

   // Padding out 14 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2;
};

// Structure to describe response TLV 0x10 for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_ProfileName
{
   // String is variable length, but must be size of the container
   // char mProfileName[1];
};

// Structure to describe response TLV 0x11 for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_PDPType
{
   eQMIPDPTypes mPDPType;
};

// Structure to describe response TLV 0x14 for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_APNName
{
   // String is variable length, but must be size of the container
   // char mAPNName[1];
};

// Structure to describe response TLV 0x15 for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_PrimaryDNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0x16 for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_SecondaryDNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0x17 for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_UMTSGrantedQoS
{
   eQMITrafficClasses mTrafficClass;
   UINT32 mMaxUplinkBitrate;
   UINT32 mMaxDownlinkBitrate;
   UINT32 mGuaranteedUplinkBitrate;
   UINT32 mGuaranteedDownlinkBitrate;
   eQMIQoSDeliveryOrders mQoSDeliveryOrder;
   UINT32 mMaxSDUSize;
   eQMISDUErrorRatios mSDUErrorRatio;
   eQMISDUResidualBitErrorRatios mSDUResidualBitErrorRatio;
   eQMIErroneousSDUDeliveries mErroneousSDUDelivery;
   UINT32 mTransferDelay;
   UINT32 mTrafficHandlingPriority;
};

// Structure to describe response TLV 0x19 for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_GPRSGrantedQoS
{
   UINT32 mPrecedenceClass;
   UINT32 mDelayClass;
   UINT32 mReliabilityClass;
   UINT32 mPeakThroughputClass;
   UINT32 mMeanThroughputClass;
};

// Structure to describe response TLV 0x1B for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_Username
{
   // String is variable length, but must be size of the container
   // char mUsername[1];
};

// Structure to describe response TLV 0x1D for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_Authentication
{
   bool mEnablePAP:1;
   bool mEnableCHAP:1;

   // Padding out 6 bits
   UINT8 mReserved1:6;
};

// Structure to describe response TLV 0x1E for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_IPAddress
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0x1F for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_ProfileID
{
   eQMIProfileTypes mProfileType;
   UINT8 mProfileIndex;
};

// Structure to describe response TLV 0x20 for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_GatewayAddress
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0x21 for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_GatewaySubnetMask
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0x22 for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_PCSCF
{
   INT8 mPCSCFAddressUsingPCO;
};

// Structure to describe response TLV 0x23 for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_PCSCFServerAddressList
{
   UINT8 mNumberOfInstances;

   struct sInstance
   {
      UINT8 mIPV4Address[4];
   };

   // This array must be the size specified by mNumberOfInstances
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x24 for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_PCSCFDomainNameList
{
   UINT8 mNumberOfInstances;

   struct sInstance
   {
      UINT16 mFQDNLength;
   
      // This array must be the size specified by mFQDNLength
      // char mFQDN[1];
   };

   // This array must be the size specified by mNumberOfInstances
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x25 for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_IPv6Address
{
   UINT16 mIPv6Address[8];
   UINT8 mIPPrefixLength;
};

// Structure to describe response TLV 0x26 for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_IPv6GatewayAddress
{
   UINT16 mIPv6Address[8];
   UINT8 mIPPrefixLength;
};

// Structure to describe response TLV 0x27 for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_PrimaryIPv6DNS
{
   UINT16 mIPv6Address[8];
};

// Structure to describe response TLV 0x28 for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_SecondaryIPv6DNS
{
   UINT16 mIPv6Address[8];
};

// Structure to describe response TLV 0x29 for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_MTU
{
   UINT32 mMTU;
};

// Structure to describe response TLV 0x2A for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_DomainNameList
{
   UINT8 mNumberOfInstances;

   struct sInstance
   {
      UINT16 mDomainNameLength;
   
      // This array must be the size specified by mDomainNameLength
      // char mDomainName[1];
   };

   // This array must be the size specified by mNumberOfInstances
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x2B for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_IPFamily
{
   eQMIWDSIPFamilies mIPFamily;
};

// Structure to describe response TLV 0x2C for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_IMCNFlag
{
   INT8 mIMCN;
};

// Structure to describe response TLV 0x2D for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_ExtendedTechnology
{
   eQMIWDSExtendedTechPrefs mExtendedTechnologyPreference;
};

// Structure to describe response TLV 0x2E for WDSGetCurrentSettings()
struct sWDSGetCurrentSettingsResponse_PCSCFIPv6AddressList
{
   UINT8 mNumberOfAddresses;

   struct sAddress
   {
      UINT16 mIPv6Address[8];
   };

   // This array must be the size specified by mNumberOfAddresses
   // sAddress mAddresses[1];
};

// Structure to describe request TLV 0x01 for WDSSetMIPMode()
struct sWDSSetMIPModeRequest_MobileIPMode
{
   eQMIMobileIPModes mMIPMode;
};

// Structure to describe response TLV 0x01 for WDSGetMIPMode()
struct sWDSGetMIPModeResponse_MobileIPMode
{
   eQMIMobileIPModes mMIPMode;
};

// Structure to describe response TLV 0x01 for WDSGetDormancy()
struct sWDSGetDormancyResponse_DormancyStatus
{
   eQMIDormancyStatus mDormancyStatus;
};

// Structure to describe response TLV 0x01 for WDSGetAutoconnectSetting()
struct sWDSGetAutoconnectSettingResponse_Autoconnect
{
   eQMIWDSAutoconnectSettings mAutoconnectSetting;
};

// Structure to describe response TLV 0x10 for WDSGetAutoconnectSetting()
struct sWDSGetAutoconnectSettingResponse_Roam
{
   eQMIWDSAutoconnectRoamSettings mAutoconnectRoamSetting;
};

// Structure to describe response TLV 0x01 for WDSGetDataSessionDuration()
struct sWDSGetDataSessionDurationResponse_Duration
{
   UINT64 mDataSessionDuration;
};

// Structure to describe response TLV 0x10 for WDSGetDataSessionDuration()
struct sWDSGetDataSessionDurationResponse_PreviousDuration
{
   UINT64 mPreviousDataSessionDuration;
};

// Structure to describe response TLV 0x11 for WDSGetDataSessionDuration()
struct sWDSGetDataSessionDurationResponse_ActiveDuration
{
   UINT64 mDataSessionActiveDuration;
};

// Structure to describe response TLV 0x12 for WDSGetDataSessionDuration()
struct sWDSGetDataSessionDurationResponse_PreviousActiveDuration
{
   UINT64 mPreviousDataSessionActiveDuration;
};

// Structure to describe response TLV 0x01 for WDSGetModemStatus()
struct sWDSGetModemStatusResponse_Status
{
   eQMIConnectionStatus mConnectionStatus;
   UINT64 mDataSessionDuration;
};

// Structure to describe response TLV 0x10 for WDSGetModemStatus()
struct sWDSGetModemStatusResponse_CallEndReason
{
   eQMICallEndReasons mCallEnd;
};

// Structure to describe indication TLV 0x01 for WDS ModemStatusReport
struct sWDSModemStatusReportIndication_Status
{
   eQMIConnectionStatus mConnectionStatus;
};

// Structure to describe indication TLV 0x10 for WDS ModemStatusReport
struct sWDSModemStatusReportIndication_CallEndReason
{
   eQMICallEndReasons mCallEnd;
};

// Structure to describe response TLV 0x01 for WDSGetDataBearerTechnology()
struct sWDSGetDataBearerTechnologyResponse_Technology
{
   eQMIDataBearerTechnologies mDataBearerTechnology;
};

// Structure to describe response TLV 0x10 for WDSGetDataBearerTechnology()
struct sWDSGetDataBearerTechnologyResponse_LastCallTechnology
{
   eQMIDataBearerTechnologies mDataBearerTechnology;
};

// Structure to describe request TLV 0x01 for WDSGetModemInfo()
struct sWDSGetModemInfoRequest_RequestedStatus
{
   bool mConnectionStatus:1;
   bool mLastCallEndReason:1;
   bool mRXTXByteTotals:1;
   bool mDormancyStatus:1;
   bool mDataBearerTechnology:1;
   bool mChannelRates:1;
   bool mDuration:1;

   // Padding out 25 bits
   UINT8 mReserved1:1;
   UINT8 mReserved2[3];
};

// Structure to describe request TLV 0x10 for WDSGetModemInfo()
struct sWDSGetModemInfoRequest_ConnectionStatusIndicator
{
   INT8 mReportConnectionStatus;
};

// Structure to describe request TLV 0x11 for WDSGetModemInfo()
struct sWDSGetModemInfoRequest_TransferStatisticsIndicator
{
   UINT8 mTransferStatisticsIntervalSeconds;

   // Padding out 6 bits
   UINT8 mReserved1:6;

   bool mTXByteTotal:1;
   bool mRXByteTotal:1;

   // Padding out 24 bits
   UINT8 mReserved2[3];
};

// Structure to describe request TLV 0x12 for WDSGetModemInfo()
struct sWDSGetModemInfoRequest_DormancyStatusIndicator
{
   INT8 mReportDormancyStatus;
};

// Structure to describe request TLV 0x13 for WDSGetModemInfo()
struct sWDSGetModemInfoRequest_DataBearerTechnologyIndicator
{
   INT8 mReportDataBearerTechnology;
};

// Structure to describe request TLV 0x14 for WDSGetModemInfo()
struct sWDSGetModemInfoRequest_ChannelRateIndicator
{
   INT8 mReportChannelRate;
};

// Structure to describe response TLV 0x10 for WDSGetModemInfo()
struct sWDSGetModemInfoResponse_Status
{
   eQMIConnectionStatus mConnectionStatus;
   UINT64 mDataSessionDuration;
};

// Structure to describe response TLV 0x11 for WDSGetModemInfo()
struct sWDSGetModemInfoResponse_CallEndReason
{
   eQMICallEndReasons mCallEnd;
};

// Structure to describe response TLV 0x12 for WDSGetModemInfo()
struct sWDSGetModemInfoResponse_TXBytes
{
   UINT64 mTXByteTotal;
};

// Structure to describe response TLV 0x13 for WDSGetModemInfo()
struct sWDSGetModemInfoResponse_RXBytes
{
   UINT64 mRXByteTotal;
};

// Structure to describe response TLV 0x14 for WDSGetModemInfo()
struct sWDSGetModemInfoResponse_DormancyStatus
{
   eQMIDormancyStatus mDormancyStatus;
};

// Structure to describe response TLV 0x15 for WDSGetModemInfo()
struct sWDSGetModemInfoResponse_Technology
{
   eQMIDataBearerTechnologies mDataBearerTechnology;
};

// Structure to describe response TLV 0x16 for WDSGetModemInfo()
struct sWDSGetModemInfoResponse_Rates
{
   UINT32 mChannelTXRatebps;
   UINT32 mChannelRXRatebps;
   UINT32 mMaxChannelTXRatebps;
   UINT32 mMaxChannelRXRatebps;
};

// Structure to describe response TLV 0x17 for WDSGetModemInfo()
struct sWDSGetModemInfoResponse_PreviousTXBytes
{
   UINT64 mPreviousCallTXByteTotal;
};

// Structure to describe response TLV 0x18 for WDSGetModemInfo()
struct sWDSGetModemInfoResponse_PreviousRXBytes
{
   UINT64 mPreviousCallRXByteTotal;
};

// Structure to describe response TLV 0x19 for WDSGetModemInfo()
struct sWDSGetModemInfoResponse_ActiveDuration
{
   UINT64 mDataSessionActiveDuration;
};

// Structure to describe response TLV 0x20 for WDSGetModemInfo()
struct sWDSGetModemInfoResponse_LastCallTechnology
{
   eQMIDataBearerTechnologies mDataBearerTechnology;
};

// Structure to describe indication TLV 0x10 for WDS ModemInfoReport
struct sWDSModemInfoReportIndication_Status
{
   eQMIConnectionStatus mConnectionStatus;
};

// Structure to describe indication TLV 0x11 for WDS ModemInfoReport
struct sWDSModemInfoReportIndication_CallEndReason
{
   eQMICallEndReasons mCallEnd;
};

// Structure to describe indication TLV 0x12 for WDS ModemInfoReport
struct sWDSModemInfoReportIndication_TXBytes
{
   UINT64 mTXByteTotal;
};

// Structure to describe indication TLV 0x13 for WDS ModemInfoReport
struct sWDSModemInfoReportIndication_RXBytes
{
   UINT64 mRXByteTotal;
};

// Structure to describe indication TLV 0x14 for WDS ModemInfoReport
struct sWDSModemInfoReportIndication_DormancyStatus
{
   eQMIDormancyStatus mDormancyStatus;
};

// Structure to describe indication TLV 0x15 for WDS ModemInfoReport
struct sWDSModemInfoReportIndication_Technology
{
   eQMIDataBearerTechnologies mDataBearerTechnology;
};

// Structure to describe indication TLV 0x16 for WDS ModemInfoReport
struct sWDSModemInfoReportIndication_Rates
{
   UINT32 mChannelTXRatebps;
   UINT32 mChannelRXRatebps;
};

// Structure to describe response TLV 0x01 for WDSGetActiveMIPProfile()
struct sWDSGetActiveMIPProfileResponse_Index
{
   UINT8 mProfileIndex;
};

// Structure to describe request TLV 0x01 for WDSSetActiveMIPProfile()
struct sWDSSetActiveMIPProfileRequest_Index
{
   char mSPC[6];
   UINT8 mProfileIndex;
};

// Structure to describe request TLV 0x01 for WDSGetMIPProfile()
struct sWDSGetMIPProfileRequest_Index
{
   UINT8 mProfileIndex;
};

// Structure to describe response TLV 0x10 for WDSGetMIPProfile()
struct sWDSGetMIPProfileResponse_State
{
   INT8 mEnabled;
};

// Structure to describe response TLV 0x11 for WDSGetMIPProfile()
struct sWDSGetMIPProfileResponse_HomeAddress
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0x12 for WDSGetMIPProfile()
struct sWDSGetMIPProfileResponse_PrimaryHomeAgentAddress
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0x13 for WDSGetMIPProfile()
struct sWDSGetMIPProfileResponse_SecondaryHomeAgentAddress
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0x14 for WDSGetMIPProfile()
struct sWDSGetMIPProfileResponse_ReverseTunneling
{
   INT8 mReverseTunneling;
};

// Structure to describe response TLV 0x15 for WDSGetMIPProfile()
struct sWDSGetMIPProfileResponse_NAI
{
   // String is variable length, but must be size of the container
   // char mNAI[1];
};

// Structure to describe response TLV 0x16 for WDSGetMIPProfile()
struct sWDSGetMIPProfileResponse_HASPI
{
   UINT32 mHASPI;
};

// Structure to describe response TLV 0x17 for WDSGetMIPProfile()
struct sWDSGetMIPProfileResponse_AAASPI
{
   UINT32 mAAASPI;
};

// Structure to describe response TLV 0x1A for WDSGetMIPProfile()
struct sWDSGetMIPProfileResponse_HAState
{
   eQMIHAAAAKeyStates mKeyState;
};

// Structure to describe response TLV 0x1B for WDSGetMIPProfile()
struct sWDSGetMIPProfileResponse_AAAState
{
   eQMIHAAAAKeyStates mKeyState;
};

// Structure to describe request TLV 0x01 for WDSSetMIPProfile()
struct sWDSSetMIPProfileRequest_Index
{
   char mSPC[6];
   UINT8 mProfileIndex;
};

// Structure to describe request TLV 0x10 for WDSSetMIPProfile()
struct sWDSSetMIPProfileRequest_State
{
   INT8 mEnabled;
};

// Structure to describe request TLV 0x11 for WDSSetMIPProfile()
struct sWDSSetMIPProfileRequest_HomeAddress
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x12 for WDSSetMIPProfile()
struct sWDSSetMIPProfileRequest_PrimaryHomeAgentAddress
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x13 for WDSSetMIPProfile()
struct sWDSSetMIPProfileRequest_SecondaryHomeAgentAddress
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x14 for WDSSetMIPProfile()
struct sWDSSetMIPProfileRequest_ReverseTunneling
{
   INT8 mReverseTunneling;
};

// Structure to describe request TLV 0x15 for WDSSetMIPProfile()
struct sWDSSetMIPProfileRequest_NAI
{
   // String is variable length, but must be size of the container
   // char mNAI[1];
};

// Structure to describe request TLV 0x16 for WDSSetMIPProfile()
struct sWDSSetMIPProfileRequest_HASPI
{
   UINT32 mHASPI;
};

// Structure to describe request TLV 0x17 for WDSSetMIPProfile()
struct sWDSSetMIPProfileRequeste_AAASPI
{
   UINT32 mAAASPI;
};

// Structure to describe request TLV 0x18 for WDSSetMIPProfile()
struct sWDSSetMIPProfileRequest_MNHA
{
   // String is variable length, but must be size of the container
   // char mMNHAKey[1];
};

// Structure to describe request TLV 0x19 for WDSSetMIPProfile()
struct sWDSSetMIPProfileRequest_MNAAA
{
   // String is variable length, but must be size of the container
   // char mMNAAAKey[1];
};

// Structure to describe response TLV 0x10 for WDSGetMIPParameters()
struct sWDSGetMIPParametersResponse_MobileIPMode
{
   eQMIMobileIPModes mMIPMode;
};

// Structure to describe response TLV 0x11 for WDSGetMIPParameters()
struct sWDSGetMIPParametersResponse_RetryAttemptLimit
{
   UINT8 mRetryAttemptLimit;
};

// Structure to describe response TLV 0x12 for WDSGetMIPParameters()
struct sWDSGetMIPParametersResponse_RetryAttemptInterval
{
   UINT8 mRetryAttemptInterval;
};

// Structure to describe response TLV 0x13 for WDSGetMIPParameters()
struct sWDSGetMIPParametersResponse_ReRegistrationPeriod
{
   UINT8 mReRegistrationPeriod;
};

// Structure to describe response TLV 0x14 for WDSGetMIPParameters()
struct sWDSGetMIPParametersResponse_ReRegistrationOnlyWithTraffic
{
   INT8 mReRegistrationOnlyWithTraffic;
};

// Structure to describe response TLV 0x15 for WDSGetMIPParameters()
struct sWDSGetMIPParametersResponse_MNHAAuthenticatorCalculator
{
   INT8 mMNHAAuthenticatorCalculator;
};

// Structure to describe response TLV 0x16 for WDSGetMIPParameters()
struct sWDSGetMIPParametersResponse_MNHARFC2002BISAuthentication
{
   INT8 mMNHARFC2002BISAuthentication;
};

// Structure to describe request TLV 0x01 for WDSSetMIPParameters()
struct sWDSSetMIPParametersRequest_SPC
{
   char mSPC[6];
};

// Structure to describe request TLV 0x10 for WDSSetMIPParameters()
struct sWDSSetMIPParametersRequest_MobileIPMode
{
   eQMIMobileIPModes mMIPMode;
};

// Structure to describe request TLV 0x11 for WDSSetMIPParameters()
struct sWDSSetMIPParametersRequest_RetryAttemptLimit
{
   UINT8 mRetryAttemptLimit;
};

// Structure to describe request TLV 0x12 for WDSSetMIPParameters()
struct sWDSSetMIPParametersRequest_RetryAttemptInterval
{
   UINT8 mRetryAttemptInterval;
};

// Structure to describe request TLV 0x13 for WDSSetMIPParameters()
struct sWDSSetMIPParametersRequest_ReRegistrationPeriod
{
   UINT8 mReRegistrationPeriod;
};

// Structure to describe request TLV 0x14 for WDSSetMIPParameters()
struct sWDSSetMIPParametersRequest_ReRegistrationOnlyWithTraffic
{
   INT8 mReRegistrationOnlyWithTraffic;
};

// Structure to describe request TLV 0x15 for WDSSetMIPParameters()
struct sWDSSetMIPParametersRequest_MNHAAuthenticatorCalculator
{
   INT8 mMNHAAuthenticatorCalculator;
};

// Structure to describe request TLV 0x16 for WDSSetMIPParameters()
struct sWDSSetMIPParametersRequest_MNHARFC2002BISAuthentication
{
   INT8 mMNHARFC2002BISAuthentication;
};

// Structure to describe response TLV 0x01 for WDSGetLastMIPStatus()
struct sWDSGetLastMIPStatusResponse_Status
{
   UINT8 mLastMIPStatus;
};

// Structure to describe response TLV 0x01 for WDSGetANAAAAuthenticationStatus()
struct sWDSGetANAAAAuthenticationStatusResponse_Status
{
   INT8 mANAAAAuthenticated;
};

// Structure to describe response TLV 0x01 for WDSGetCurrentDataBearerTechnology()
struct sWDSGetCurrentDataBearerTechnologyResponse_Technology
{
   eQMIWDSNetworkTypes mNetworkType;

   // The following union is based on the value of mNetworkType
   union uValOfNetworkType
   {
      // If the value of mNetworkType == 1
      struct sNetworkTypeIs1
      {
         bool mCDMA1x:1;
         bool mCDMA1xEvDORev0:1;
         bool mCDMA1xEvDORevA:1;
         bool mCDMA1xEvDORevB:1;
         bool mCDMAEHRPD:1;
         bool mCDMAFMC:1;
      
         // Padding out 25 bits
         UINT8 mReserved1:2;
         UINT8 mReserved2[2];
         UINT8 mReserved3:7;
      
         bool mNullBearer:1;
      
         // The following union is for handing both mCDMA1x and mCDMA1xEvDORev(0, A, B)
         union uValOfCDMA1x_or_CDMA1xEvDORevX
         {
            // If the value of mCDMA1x == 1
            struct sCDMA1xIs1
            {
               bool mCDMA1xIS95:1;
               bool mCDMA1xIS2000:1;
               bool mCDMA1xIS2000RelA:1;
            
               // Padding out 29 bits
               UINT8 mReserved4:5;
               UINT8 mReserved5[3];
            };
      
            sCDMA1xIs1 mCDMA1xIs1;
      
            // If the value of mCDMA1xEvDORev0 == 1
            struct sCDMA1xEvDORev0Is1
            {
               bool mCDMA1xEvDORev0DPA:1;
            
               // Padding out 31 bits
               UINT8 mReserved6:7;
               UINT8 mReserved7[3];
            };
      
            sCDMA1xEvDORev0Is1 mCDMA1xEvDORev0Is1;
      
            // If the value of mCDMA1xEvDORevA == 1
            struct sCDMA1xEvDORevAIs1
            {
               bool mCDMA1xEvDORevADPA:1;
               bool mCDMA1xEvDORevAMFPA:1;
               bool mCDMA1xEvDORevAEMPA:1;
               bool mCDMA1xEvDORevAEMPAEHRPD:1;
            
               // Padding out 28 bits
               UINT8 mReserved8:4;
               UINT8 mReserved9[3];
            };
      
            sCDMA1xEvDORevAIs1 mCDMA1xEvDORevAIs1;
      
            // If the value of mCDMA1xEvDORevB == 1
            struct sCDMA1xEvDORevBIs1
            {
               bool mCDMA1xEvDORevBDPA:1;
               bool mCDMA1xEvDORevBMFPA:1;
               bool mCDMA1xEvDORevBEMPA:1;
               bool mCDMA1xEvDORevBEMPAEHRPD:1;
               bool mCDMA1xEvDORevBMMPA:1;
               bool mCDMA1xEvDORevBMMPAEHRPD:1;
            
               // Padding out 26 bits
               UINT8 mReserved10:2;
               UINT8 mReserved11[3];
            };
      
            sCDMA1xEvDORevBIs1 mCDMA1xEvDORevBIs1;
      
            // Padding out 32 bits
            UINT8 mReserved12[4];
         };
      
         uValOfCDMA1x_or_CDMA1xEvDORevX mValOfCDMA1x_or_CDMA1xEvDORevX;
      };

      sNetworkTypeIs1 mNetworkTypeIs1;

      // If the value of mNetworkType == 2
      struct sNetworkTypeIs2
      {
         bool mWCDMA:1;
         bool mGPRS:1;
         bool mHSDPA:1;
         bool mHSUPA:1;
         bool mEDGE:1;
         bool mLTE:1;
         bool mHSDPAPlus:1;
         bool mDualCellHSDPAPlus:1;
         bool m64QAM:1;
         bool mTDSCDMA:1;
      
         // Padding out 21 bits
         UINT8 mReserved13:6;
         UINT8 mReserved14;
         UINT8 mReserved15:7;
      
         bool mNullBearer:1;
      };

      sNetworkTypeIs2 mNetworkTypeIs2;

      // Padding out 64 bits
      UINT8 mReserved16[8];
   };

   uValOfNetworkType mValOfNetworkType;
};

// Structure to describe response TLV 0x10 for WDSGetCurrentDataBearerTechnology()
struct sWDSGetCurrentDataBearerTechnologyResponse_LastCallTechnology
{
   eQMIWDSNetworkTypes mNetworkType;

   // The following union is based on the value of mNetworkType
   union uValOfNetworkType
   {
      // If the value of mNetworkType == 1
      struct sNetworkTypeIs1
      {
         bool mCDMA1x:1;
         bool mCDMA1xEvDORev0:1;
         bool mCDMA1xEvDORevA:1;
         bool mCDMA1xEvDORevB:1;
         bool mCDMAEHRPD:1;
         bool mCDMAFMC:1;
      
         // Padding out 25 bits
         UINT8 mReserved1:2;
         UINT8 mReserved2[2];
         UINT8 mReserved3:7;
      
         bool mNullBearer:1;
      
         // The following union is for handing both mCDMA1x and mCDMA1xEvDORev(0, A, B)
         union uValOfCDMA1x_or_CDMA1xEvDORevX
         {
            // If the value of mCDMA1x == 1
            struct sCDMA1xIs1
            {
               bool mCDMA1xIS95:1;
               bool mCDMA1xIS2000:1;
               bool mCDMA1xIS2000RelA:1;
            
               // Padding out 29 bits
               UINT8 mReserved4:5;
               UINT8 mReserved5[3];
            };
      
            sCDMA1xIs1 mCDMA1xIs1;
      
            // If the value of mCDMA1xEvDORev0 == 1
            struct sCDMA1xEvDORev0Is1
            {
               bool mCDMA1xEvDORev0DPA:1;
            
               // Padding out 31 bits
               UINT8 mReserved6:7;
               UINT8 mReserved7[3];
            };
      
            sCDMA1xEvDORev0Is1 mCDMA1xEvDORev0Is1;
      
            // If the value of mCDMA1xEvDORevA == 1
            struct sCDMA1xEvDORevAIs1
            {
               bool mCDMA1xEvDORevADPA:1;
               bool mCDMA1xEvDORevAMFPA:1;
               bool mCDMA1xEvDORevAEMPA:1;
               bool mCDMA1xEvDORevAEMPAEHRPD:1;
            
               // Padding out 28 bits
               UINT8 mReserved8:4;
               UINT8 mReserved9[3];
            };
      
            sCDMA1xEvDORevAIs1 mCDMA1xEvDORevAIs1;
      
            // If the value of mCDMA1xEvDORevB == 1
            struct sCDMA1xEvDORevBIs1
            {
               bool mCDMA1xEvDORevBDPA:1;
               bool mCDMA1xEvDORevBMFPA:1;
               bool mCDMA1xEvDORevBEMPA:1;
               bool mCDMA1xEvDORevBEMPAEHRPD:1;
               bool mCDMA1xEvDORevBMMPA:1;
               bool mCDMA1xEvDORevBMMPAEHRPD:1;
            
               // Padding out 26 bits
               UINT8 mReserved10:2;
               UINT8 mReserved11[3];
            };
      
            sCDMA1xEvDORevBIs1 mCDMA1xEvDORevBIs1;
      
            // Padding out 32 bits
            UINT8 mReserved12[4];
         };
      
         uValOfCDMA1x_or_CDMA1xEvDORevX mValOfCDMA1x_or_CDMA1xEvDORevX;
      };

      sNetworkTypeIs1 mNetworkTypeIs1;

      // If the value of mNetworkType == 2
      struct sNetworkTypeIs2
      {
         bool mWCDMA:1;
         bool mGPRS:1;
         bool mHSDPA:1;
         bool mHSUPA:1;
         bool mEDGE:1;
         bool mLTE:1;
         bool mHSDPAPlus:1;
         bool mDualCellHSDPAPlus:1;
         bool m64QAM:1;
         bool mTDSCDMA:1;
      
         // Padding out 21 bits
         UINT8 mReserved13:6;
         UINT8 mReserved14;
         UINT8 mReserved15:7;
      
         bool mNullBearer:1;
      };

      sNetworkTypeIs2 mNetworkTypeIs2;

      // Padding out 64 bits
      UINT8 mReserved16[8];
   };

   uValOfNetworkType mValOfNetworkType;
};

// Structure to describe request TLV 0x10 for WDSGetCallList()
struct sWDSGetCallListRequest_ListType
{
   eQMICallHistoryTypes mCallListType;
};

// Structure to describe response TLV 0x10 for WDSGetCallList()
struct sWDSGetCallListResponse_FullList
{
   UINT16 mCallRecords;

   struct sRecord
   {
      UINT16 mID;
      eQMICallTypes mType;
      eQMIDataBearerTechnologies mDataBearer;
      UINT64 mTimestamp;
      UINT8 mIPV4Address[4];
      UINT64 mTotalDuration;
      UINT64 mActiveDuration;
      UINT64 mRXByteTotal;
      UINT64 mTXByteTotal;
      eQMICallEndReasons mCallEnd;
      UINT8 mPhoneNumberLength;
   
      // This array must be the size specified by mPhoneNumberLength
      // char mPhoneNumber[1];
   };

   // This array must be the size specified by mCallRecords
   // sRecord mRecords[1];
};

// Structure to describe response TLV 0x11 for WDSGetCallList()
struct sWDSGetCallListResponse_IDList
{
   UINT16 mCallRecords;

   struct sRecord
   {
      UINT16 mID;
   };

   // This array must be the size specified by mCallRecords
   // sRecord mRecords[1];
};

// Structure to describe request TLV 0x01 for WDSGetCallRecord()
struct sWDSGetCallRecordRequest_RecordID
{
   UINT16 mID;
};

// Structure to describe response TLV 0x01 for WDSGetCallRecord()
struct sWDSGetCallRecordResponse_Record
{
   UINT16 mID;
   eQMICallTypes mType;
   eQMIDataBearerTechnologies mDataBearer;
   UINT64 mTimestamp;
   UINT8 mIPV4Address[4];
   UINT64 mTotalDuration;
   UINT64 mActiveDuration;
   UINT64 mRXByteTotal;
   UINT64 mTXByteTotal;
   eQMICallEndReasons mCallEnd;
   UINT8 mPhoneNumberLength;

   // This array must be the size specified by mPhoneNumberLength
   // char mPhoneNumber[1];
};

// Structure to describe response TLV 0x01 for WDSGetCallListMaxSize()
struct sWDSGetCallListMaxSizeResponse_Maximum
{
   UINT16 mCallListMaxSize;
};

// Structure to describe request TLV 0x01 for WDSGetDefaultProfileNumber()
struct sWDSGetDefaultProfileNumberRequest_ProfileType
{
   eQMIProfileTypes mProfileType;
   eQMIWDSProfileFamily mProfileFamily;
};

// Structure to describe response TLV 0x01 for WDSGetDefaultProfileNumber()
struct sWDSGetDefaultProfileNumberResponse_ProfileNumber
{
   UINT8 mProfileIndex;
};

// Structure to describe response TLV 0xE0 for WDSGetDefaultProfileNumber()
struct sWDSGetDefaultProfileNumberResponse_ExtendedErrorCode
{
   eQMIWDSExtendedErrorCode mExtendedErrorCode;
};

// Structure to describe request TLV 0x01 for WDSSetDefaultProfileNumber()
struct sWDSSetDefaultProfileNumberRequest_ProfileIdentifier
{
   eQMIProfileTypes mProfileType;
   eQMIWDSProfileFamily mProfileFamily;
   UINT8 mProfileIndex;
};

// Structure to describe response TLV 0xE0 for WDSSetDefaultProfileNumber()
struct sWDSSetDefaultProfileNumberResponse_ExtendedErrorCode
{
   eQMIWDSExtendedErrorCode mExtendedErrorCode;
};

// Structure to describe request TLV 0x01 for WDSResetProfile()
struct sWDSResetProfileRequest_ProfileIdentifier
{
   eQMIProfileTypes mProfileType;
   UINT8 mProfileIndex;
};

// Structure to describe response TLV 0xE0 for WDSResetProfile()
struct sWDSResetProfileResponse_ExtendedErrorCode
{
   eQMIWDSExtendedErrorCode mExtendedErrorCode;
};

// Structure to describe request TLV 0x01 for WDSResetProfileParamToInvalid()
struct sWDSResetProfileParamToInvalidRequest_ProfileParam
{
   eQMIProfileTypes mProfileType;
   UINT8 mProfileIndex;
   eQMIWDSProfileParamID mProfileParamID;
};

// Structure to describe response TLV 0xE0 for WDSResetProfileParamToInvalid()
struct sWDSResetProfileParamToInvalidResponse_ExtendedErrorCode
{
   eQMIWDSExtendedErrorCode mExtendedErrorCode;
};

// Structure to describe request TLV 0x01 for WDSSetIPFamilyPreference()
struct sWDSSetIPFamilyPreferenceRequest_IPFamilyPreference
{
   eQMIWDSIPFamilies mIPFamily;
};

// Structure to describe request TLV 0x01 for WDSSetFMCTunnelParameters()
struct sWDSSetFMCTunnelParametersRequest_Parameters
{
   UINT32 mStreamID;
   INT8 mNATIsPresent;
   UINT16 mPortID;
};

// Structure to describe request TLV 0x10 for WDSSetFMCTunnelParameters()
struct sWDSSetFMCTunnelParametersRequest_IPv4Address
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x11 for WDSSetFMCTunnelParameters()
struct sWDSSetFMCTunnelParametersRequest_IPv6Address
{
   UINT16 mIPv6Address[8];
};

// Structure to describe response TLV 0x10 for WDSGetFMCTunnelParameters()
struct sWDSGetFMCTunnelParametersResponse_Parameters
{
   UINT32 mStreamID;
   INT8 mNATIsPresent;
   UINT16 mPortID;
};

// Structure to describe response TLV 0x11 for WDSGetFMCTunnelParameters()
struct sWDSGetFMCTunnelParametersResponse_IPv4Address
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0x12 for WDSGetFMCTunnelParameters()
struct sWDSGetFMCTunnelParametersResponse_IPv6Address
{
   UINT16 mIPv6Address[8];
};

// Structure to describe request TLV 0x01 for WDSSetAutoconnectSetting()
struct sWDSSetAutoconnectSettingRequest_Autoconnect
{
   eQMIWDSAutoconnectSettings mAutoconnectSetting;
};

// Structure to describe request TLV 0x10 for WDSSetAutoconnectSetting()
struct sWDSSetAutoconnectSettingRequest_Roam
{
   eQMIWDSAutoconnectRoamSettings mAutoconnectRoamSetting;
};

// Structure to describe response TLV 0x10 for WDSGetDNSSetting()
struct sWDSGetDNSSettingResponse_PrimaryDNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0x11 for WDSGetDNSSetting()
struct sWDSGetDNSSettingResponse_SecondaryDNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe response TLV 0x12 for WDSGetDNSSetting()
struct sWDSGetDNSSettingResponse_PrimaryIPv6DNS
{
   UINT16 mIPv6Address[8];
};

// Structure to describe response TLV 0x13 for WDSGetDNSSetting()
struct sWDSGetDNSSettingResponse_SecondaryIPv6DNS
{
   UINT16 mIPv6Address[8];
};

// Structure to describe request TLV 0x10 for WDSSetDNSSetting()
struct sWDSSetDNSSettingRequest_PrimaryDNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x11 for WDSSetDNSSetting()
struct sWDSSetDNSSettingRequest_SecondaryDNS
{
   UINT8 mIPV4Address[4];
};

// Structure to describe request TLV 0x12 for WDSSetDNSSetting()
struct sWDSSetDNSSettingRequest_PrimaryIPv6DNS
{
   UINT16 mIPv6Address[8];
};

// Structure to describe request TLV 0x13 for WDSSetDNSSetting()
struct sWDSSetDNSSettingRequest_SecondaryIPv6DNS
{
   UINT16 mIPv6Address[8];
};

// Structure to describe response TLV 0x01 for WDSGetCDMAPreDormancySettings()
struct sWDSGetCDMAPreDormancySettingsResponse_Settings
{
   eQMIWDSCDMAServiceOptions mServiceOption;
   eQMIWDSCDMANetworks mDataSessionNetwork;
};

// Structure to describe request TLV 0x01 for WDSSetCAMTimer()
struct sWDSSetCAMTimerRequest_Timer
{
   UINT32 mCAMTimerSeconds;
};

// Structure to describe response TLV 0x01 for WDSGetCAMTimer()
struct sWDSGetCAMTimerResponse_Timer
{
   UINT32 mCAMTimerSeconds;
};

// Structure to describe request TLV 0x01 for WDSSetSCRM()
struct sWDSSetSCRMRequest_SCRM
{
   INT8 mSCRMEnabled;
};

// Structure to describe response TLV 0x01 for WDSGetSCRM()
struct sWDSGetSCRMResponse_SCRM
{
   INT8 mSCRMEnabled;
};

// Structure to describe request TLV 0x01 for WDSSetRDUD()
struct sWDSSetRDUDRequest_RDUD
{
   INT8 mRDUDEnabled;
};

// Structure to describe response TLV 0x01 for WDSGetRDUD()
struct sWDSGetRDUDResponse_RDUD
{
   INT8 mRDUDEnabled;
};

// Structure to describe response TLV 0x01 for WDSGetSIPMIPCallType()
struct sWDSGetSIPMIPCallTypeResponse_CallType
{
   eQMIWDSSIPMIPCallTypes mCallType;
};

// Structure to describe request TLV 0x01 for WDSSetEVDOPageMonitorPeriod()
struct sWDSSetEVDOPageMonitorPeriodRequest_Period
{
   UINT8 mPageMonitorPeriod;
};

// Structure to describe indication TLV 0x01 for WDS EVDOPageMonitorPeriodIndication
struct sWDSEVDOPageMonitorPeriodIndication_Result
{
   eQMIWDSSlotCycleSetResults mSlotCycleSetResult;
};

// Structure to describe request TLV 0x01 for WDSSetEVDOLongSleep()
struct sWDSSetEVDOLongSleepRequest_Setting
{
   INT8 mForceLongSleep;
};

// Structure to describe response TLV 0x01 for WDSGetEVDOPageMonitorPeriod()
struct sWDSGetEVDOPageMonitorPeriodResponse_Details
{
   UINT8 mPageMonitorPeriod;
   INT8 mForceLongSleep;
};

// Structure to describe response TLV 0x01 for WDSGetCallThrottleInfo()
struct sWDSGetCallThrottleInfoResponse_Details
{
   UINT32 mEVDOThrottledDelaySeconds;
   UINT32 mCDMAThrottledDelaySeconds;
};

// Structure to describe request TLV 0x01 for WDSGetNSAPI()
struct sWDSGetNSAPIRequest_APN
{
   // String is variable length, but must be size of the container
   // char mAPNName[1];
};

// Structure to describe response TLV 0x01 for WDSGetNSAPI()
struct sWDSGetNSAPIResponse_NSAPI
{
   UINT8 mNSAPICount;

   // This array must be the size specified by mNSAPICount
   // UINT8 mNSAPI[1];
};

// Structure to describe request TLV 0x01 for WDSSetDUNCallControlPreference()
struct sWDSSetDUNCallControlPreferenceRequest_Preference
{
   eQMIWDSDUNControlPreferences mDUNControl;
};

// Structure to describe request TLV 0x10 for WDSSetDUNCallControlPreference()
struct sWDSSetDUNCallControlPreferenceRequest_AllowDUN
{
   INT8 mAllowDUNCalls;
};

// Structure to describe response TLV 0x01 for WDSGetDUNCallControlInfo()
struct sWDSGetDUNCallControlInfoResponse_Status
{
   INT8 mDUNControlEnabled;
};

// Structure to describe response TLV 0x10 for WDSGetDUNCallControlInfo()
struct sWDSGetDUNCallControlInfoResponse_AllowDUN
{
   INT8 mAllowDUNCalls;
};

// Structure to describe response TLV 0x11 for WDSGetDUNCallControlInfo()
struct sWDSGetDUNCallControlInfoResponse_CurrentClient
{
   INT8 mSetByCurrentClient;
};

// Structure to describe response TLV 0x12 for WDSGetDUNCallControlInfo()
struct sWDSGetDUNCallControlInfoResponse_ReportMask
{
   bool mSendDUNCallNotifications:1;
   bool mSendEntitlementNotifications:1;
   bool mSendSilentRedailNotifications:1;

   // Padding out 5 bits
   UINT8 mReserved1:5;
};

// Structure to describe request TLV 0x01 for WDSSetDUNCallControlEventReport()
struct sWDSSetDUNCallControlEventReportRequest_CallNotifications
{
   INT8 mEnableDUNCallNotifications;
};

// Structure to describe request TLV 0x10 for WDSSetDUNCallControlEventReport()
struct sWDSSetDUNCallControlEventReportRequest_EntitlementNotifications
{
   INT8 mEnableEntitlementNotifications;
};

// Structure to describe request TLV 0x11 for WDSSetDUNCallControlEventReport()
struct sWDSSetDUNCallControlEventReportRequest_RedialNotifications
{
   INT8 mEnableSilentRedailNotifications;
};

// Structure to describe response TLV 0x01 for WDSSetDUNCallControlEventReport()
struct sWDSSetDUNCallControlEventReportResponse_ReportMask
{
   bool mSendDUNCallNotifications:1;
   bool mSendEntitlementNotifications:1;
   bool mSendSilentRedailNotifications:1;

   // Padding out 5 bits
   UINT8 mReserved1:5;
};

// Structure to describe indication TLV 0x01 for WDS DUNCallControlEventReport
struct sWDSDUNCallControlEventReportIndication_Event
{
   eQMIWDSDUNControlEvents mDUNControlEvent;
};

// Structure to describe indication TLV 0x10 for WDS DUNCallControlEventReport
struct sWDSDUNCallControlEventReportIndication_CallNotification
{
   INT8 mDUNCallAllowed;
};

// Structure to describe indication TLV 0x11 for WDS DUNCallControlEventReport
struct sWDSDUNCallControlEventReportIndication_CallID
{
   UINT8 mDUNCallID;
};

// Structure to describe indication TLV 0x12 for WDS DUNCallControlEventReport
struct sWDSDUNCallControlEventReportIndication_PreviousFailureReason
{
   eQMIWDSCallEndReasonTypes mCallEndReasonType;

   // The following union is based on the value of mCallEndReasonType
   union uValOfCallEndReasonType
   {
      // Always present
      UINT16 mCallEndReasonValue;

      // If the value of mCallEndReasonType == 1
      struct sCallEndReasonTypeIs1
      {
         eQMIWDSMobileIPCallEndReasons mMobileIPCallEndReason;
      };

      sCallEndReasonTypeIs1 mCallEndReasonTypeIs1;

      // If the value of mCallEndReasonType == 2
      struct sCallEndReasonTypeIs2
      {
         eQMIWDSInternalCallEndReasons mInternalCallEndReason;
      };

      sCallEndReasonTypeIs2 mCallEndReasonTypeIs2;

      // If the value of mCallEndReasonType == 3
      struct sCallEndReasonTypeIs3
      {
         eQMIWDSCallManagerCallEndReasons mCallManagerCallEndReason;
      };

      sCallEndReasonTypeIs3 mCallEndReasonTypeIs3;

      // If the value of mCallEndReasonType == 6
      struct sCallEndReasonTypeIs6
      {
         eQMIWDS3GPPCallEndReasons m3GPPCallEndReason;
      };

      sCallEndReasonTypeIs6 mCallEndReasonTypeIs6;

      // If the value of mCallEndReasonType == 7
      struct sCallEndReasonTypeIs7
      {
         eQMIWDSPPPCallEndReason mPPPCallEndReason;
      };

      sCallEndReasonTypeIs7 mCallEndReasonTypeIs7;

      // If the value of mCallEndReasonType == 8
      struct sCallEndReasonTypeIs8
      {
         eQMIWDSEHRPDCallEndReason mEHRPDCallEndReason;
      };

      sCallEndReasonTypeIs8 mCallEndReasonTypeIs8;

      // If the value of mCallEndReasonType == 9
      struct sCallEndReasonTypeIs9
      {
         eQMIWDSIPv6CallEndReason mIPv6CallEndReason;
      };

      sCallEndReasonTypeIs9 mCallEndReasonTypeIs9;

      // Padding out 16 bits
      UINT8 mReserved1[2];
   };

   uValOfCallEndReasonType mValOfCallEndReasonType;
};

// Structure to describe request TLV 0x01 for WDSPendingDUNCallControl()
struct sWDSPendingDUNCallControlRequest_Action
{
   INT8 mDUNCallAllowed;
};

// Structure to describe request TLV 0x01 for WDSEMBMSTMGIActivate()
struct sWDSEMBMSTMGIActivateRequest_TMGI
{
   UINT8 mTMGI[6];
   INT8 mSessionIDValid;
   UINT8 mSessionID;
};

// Structure to describe response TLV 0x10 for WDSEMBMSTMGIActivate()
struct sWDSEMBMSTMGIActivateResponse_ExtendedError
{
   eQMIWDSEMBMSErrorCodes mExtendedEMBMSErrorCode;
};

// Structure to describe indication TLV 0x01 for WDS EMBMSTMGIActivateIndication
struct sWDSEMBMSTMGIActivateIndication_Status
{
   eQMIWDSEMBMSOperationStatus mTMGIOperationStatus;
};

// Structure to describe request TLV 0x01 for WDSEMBMSTMGIDeactivate()
struct sWDSEMBMSTMGIDeactivateRequest_TMGI
{
   UINT8 mTMGI[6];
   INT8 mSessionIDValid;
   UINT8 mSessionID;
};

// Structure to describe response TLV 0x10 for WDSEMBMSTMGIDeactivate()
struct sWDSEMBMSTMGIDeactivateResponse_ExtendedError
{
   eQMIWDSEMBMSErrorCodes mExtendedEMBMSErrorCode;
};

// Structure to describe indication TLV 0x01 for WDS EMBMSTMGIDectivateIndication
struct sWDSEMBMSTMGIDectivateIndication_Status
{
   eQMIWDSEMBMSOperationStatus mTMGIOperationStatus;
};

// Structure to describe request TLV 0x01 for WDSEMBMSTMGIListQuery()
struct sWDSEMBMSTMGIListQueryRequest_Type
{
   eQMIWDSEMBMSListTypes mTMGIListType;
};

// Structure to describe response TLV 0x10 for WDSEMBMSTMGIListQuery()
struct sWDSEMBMSTMGIListQueryResponse_List
{
   eQMIWDSEMBMSListTypes mTMGIListType;
   UINT8 mTMGIListCount;

   struct sEntry
   {
      UINT8 mTMGI[6];
      INT8 mSessionIDValid;
      UINT8 mSessionID;
   };

   // This array must be the size specified by mTMGIListCount
   // sEntry mEntrys[1];
};

// Structure to describe response TLV 0x11 for WDSEMBMSTMGIListQuery()
struct sWDSEMBMSTMGIListQueryResponse_OOS
{
   eQMIWDSOOSWarningReasons mOOSWarningReason;
};

// Structure to describe indication TLV 0x10 for WDS EMBMSTMGIListIndication
struct sWDSEMBMSTMGIListIndication_List
{
   eQMIWDSEMBMSListTypes mTMGIListType;
   UINT8 mTMGIListCount;

   struct sEntry
   {
      UINT8 mTMGI[6];
      INT8 mSessionIDValid;
      UINT8 mSessionID;
   };

   // This array must be the size specified by mTMGIListCount
   // sEntry mEntrys[1];
};

// Structure to describe indication TLV 0x11 for WDS EMBMSTMGIListIndication
struct sWDSEMBMSTMGIListIndication_OOS
{
   eQMIWDSOOSWarningReasons mOOSWarningReason;
};

// Structure to describe response TLV 0x10 for WDSGetPreferredDataSystem()
struct sWDSGetPreferredDataSystemResponse_PreferredDataSystem
{
   eQMIWDSDataSystems mPreferredDataSystem;
};

// Structure to describe response TLV 0x10 for WDSGetLastDataCallStatus()
struct sWDSGetLastDataCallStatusResponse_DataCallStatus
{
   eQMIWDSDataCallStatus mDataCallStatus;
};

// Structure to describe response TLV 0x11 for WDSGetLastDataCallStatus()
struct sWDSGetLastDataCallStatusResponse_DataCallType
{
   eQMIWDSDataCallTypes mDataCallType;
   eQMIWDSTetheredCallTypes mTetheredCallType;
};

// Structure to describe response TLV 0x10 for WDSGetCurrentDataSystems()
struct sWDSGetCurrentDataSystemsResponse_Systems
{
   eQMIWDSDataSystemNetworkTypes mPreferredNetworkType;
   UINT8 mNetworkCount;

   struct sNetwork
   {
      eQMIWDSDataSystemNetworkTypes mNetworkType;
   
      // The following union is based on the value of mNetworkType
      union uValOfNetworkType
      {
         // If the value of mNetworkType == 0
         struct sNetworkTypeIs0
         {
            bool mWCDMA:1;
            bool mGPRS:1;
            bool mHSDPA:1;
            bool mHSUPA:1;
            bool mEDGE:1;
            bool mLTE:1;
            bool mHSDPAPlus:1;
            bool mDualCellHSDPAPlus:1;
            bool m64QAM:1;
            bool mTDSCDMA:1;
         
            // Padding out 21 bits
            UINT8 mReserved1:6;
            UINT8 mReserved2;
            UINT8 mReserved3:7;
         
            bool mNULLBearer:1;
         };
   
         sNetworkTypeIs0 mNetworkTypeIs0;
   
         // If the value of mNetworkType == 1
         struct sNetworkTypeIs1
         {
            bool mCDMA1x:1;
            bool mCDMA1xEvDORev0:1;
            bool mCDMA1xEvDORevA:1;
            bool mCDMA1xEvDORevB:1;
            bool mCDMAEHRPD:1;
            bool mCDMAFMC:1;
         
            // Padding out 25 bits
            UINT8 mReserved4:2;
            UINT8 mReserved5[2];
            UINT8 mReserved6:7;
         
            bool mNULLBearer:1;
         
            // The following union is for handing all mCDMA1x types
            union uValOfCDMA1xTypes
            {
               // If the value of mCDMA1x == 1
               struct sCDMA1xIs1
               {
                  bool mCDMA1xIS95:1;
                  bool mCDMA1xIS2000:1;
                  bool mCDMA1xIS2000RelA:1;
               
                  // Padding out 29 bits
                  UINT8 mReserved7:5;
                  UINT8 mReserved8[3];
               };
         
               sCDMA1xIs1 mCDMA1xIs1;
         
               // If the value of mCDMA1xEvDORev0 == 1
               struct sCDMA1xEvDORev0Is1
               {
                  bool mCDMA1xEvDORev0DPA:1;
               
                  // Padding out 31 bits
                  UINT8 mReserved9:7;
                  UINT8 mReserved10[3];
               };
         
               sCDMA1xEvDORev0Is1 mCDMA1xEvDORev0Is1;
         
               // If the value of mCDMA1xEvDORevA == 1
               struct sCDMA1xEvDORevAIs1
               {
                  bool mCDMA1xEvDORevADPA:1;
                  bool mCDMA1xEvDORevAMFPA:1;
                  bool mCDMA1xEvDORevAEMPA:1;
                  bool mCDMA1xEvDORevAEMPAEHRPD:1;
               
                  // Padding out 28 bits
                  UINT8 mReserved11:4;
                  UINT8 mReserved12[3];
               };
         
               sCDMA1xEvDORevAIs1 mCDMA1xEvDORevAIs1;
         
               // If the value of mCDMA1xEvDORevB == 1
               struct sCDMA1xEvDORevBIs1
               {
                  bool mCDMA1xEvDORevBDPA:1;
                  bool mCDMA1xEvDORevBMFPA:1;
                  bool mCDMA1xEvDORevBEMPA:1;
                  bool mCDMA1xEvDORevBEMPAEHRPD:1;
                  bool mCDMA1xEvDORevBMMPA:1;
                  bool mCDMA1xEvDORevBMMPAEHRPD:1;
               
                  // Padding out 26 bits
                  UINT8 mReserved13:2;
                  UINT8 mReserved14[3];
               };
         
               sCDMA1xEvDORevBIs1 mCDMA1xEvDORevBIs1;
         
               // Padding out 32 bits
               UINT8 mReserved15[4];
            };
         
            uValOfCDMA1xTypes mValOfCDMA1xTypes;
         };
   
         sNetworkTypeIs1 mNetworkTypeIs1;
   
         // Padding out 64 bits
         UINT8 mReserved16[8];
      };
   
      uValOfNetworkType mValOfNetworkType;
   };

   // This array must be the size specified by mNetworkCount
   // sNetwork mNetworks[1];
};

// Structure to describe request TLV 0x01 for WDSGetPDNThrottleInfo()
struct sWDSGetPDNThrottleInfoRequest_Type
{
   eQMIWDSDataSystemNetworkTypes mTechnologyType;
};

// Structure to describe response TLV 0x10 for WDSGetPDNThrottleInfo()
struct sWDSGetPDNThrottleInfoResponse_Info
{
   UINT8 mThrottleInfoCount;

   struct sInfo
   {
      INT8 mIPv4Throttled;
      INT8 mIPv6Throttled;
      UINT32 mIPv4ThrottleTimeLeftInMilliseconds;
      UINT32 mIPv6ThrottleTimeLeftInMilliseconds;
      UINT8 mAPNNameLength;
   
      // This array must be the size specified by mAPNNameLength
      // char mAPNName[1];
   };

   // This array must be the size specified by mThrottleInfoCount
   // sInfo mInfos[1];
};

// Structure to describe response TLV 0x10 for WDSGetLTEAttachParameters()
struct sWDSGetLTEAttachParametersResponse_APNString
{
   // String is variable length, but must be size of the container
   // char mAPNName[1];
};

// Structure to describe response TLV 0x11 for WDSGetLTEAttachParameters()
struct sWDSGetLTEAttachParametersResponse_IPSupport
{
   eQMIWDSLTEIPTypes mIPType;
};

// Structure to describe response TLV 0x12 for WDSGetLTEAttachParameters()
struct sWDSGetLTEAttachParametersResponse_OTAAttach
{
   INT8 mOTAAttachPerformed;
};

// Structure to describe response TLV 0x10 for WDSGetFlowControlStatus()
struct sWDSGetFlowControlStatusResponse_UplinkFlowControl
{
   INT8 mUplinkFlowControlEnabled;
};

// Structure to describe request TLV 0x10 for DMSSetEventReport()
struct sDMSSetEventReportRequest_PowerState
{
   INT8 mReportPowerState;
};

// Structure to describe request TLV 0x11 for DMSSetEventReport()
struct sDMSSetEventReportRequest_BatteryLevel
{
   UINT8 mBatteryLevelLowerLimit;
   UINT8 mBatteryLevelUpperLimit;
};

// Structure to describe request TLV 0x12 for DMSSetEventReport()
struct sDMSSetEventReportRequest_PINStatus
{
   INT8 mReportPINStatus;
};

// Structure to describe request TLV 0x13 for DMSSetEventReport()
struct sDMSSetEventReportRequest_ActivationState
{
   INT8 mReportActivationState;
};

// Structure to describe request TLV 0x14 for DMSSetEventReport()
struct sDMSSetEventReportRequest_OperatingMode
{
   INT8 mReportOperatingMode;
};

// Structure to describe request TLV 0x15 for DMSSetEventReport()
struct sDMSSetEventReportRequest_UIMState
{
   INT8 mReportUIMState;
};

// Structure to describe request TLV 0x16 for DMSSetEventReport()
struct sDMSSetEventReportRequest_WirelessDisableState
{
   INT8 mReportWirelessDisableState;
};

// Structure to describe request TLV 0x17 for DMSSetEventReport()
struct sDMSSetEventReportRequest_PRLInit
{
   INT8 mReportPRLInitialization;
};

// Structure to describe indication TLV 0x10 for DMS EventReport
struct sDMSEventReportIndication_PowerState
{
   eQMIDMSPowerSources mPowerSource:1;
   bool mBatteryConnected:1;
   bool mBatteryCharging:1;
   bool mPowerFault:1;

   // Padding out 4 bits
   UINT8 mReserved1:4;

   UINT8 mBatteryLevel;
};

// Structure to describe indication TLV 0x11 for DMS EventReport
struct sDMSEventReportIndication_PIN1State
{
   eQMIDMSPINStatus mPINStatus;
   UINT8 mRemainingVerifyRetries;
   UINT8 mRemainingUnblockRetries;
};

// Structure to describe indication TLV 0x12 for DMS EventReport
struct sDMSEventReportIndication_PIN2State
{
   eQMIDMSPINStatus mPINStatus;
   UINT8 mRemainingVerifyRetries;
   UINT8 mRemainingUnblockRetries;
};

// Structure to describe indication TLV 0x13 for DMS EventReport
struct sDMSEventReportIndication_ActivationState
{
   eQMIDMSActivationStates mActivationState;
};

// Structure to describe indication TLV 0x14 for DMS EventReport
struct sDMSEventReportIndication_OperatingMode
{
   eQMIDMSOperatingModes mOperatingMode;
};

// Structure to describe indication TLV 0x15 for DMS EventReport
struct sDMSEventReportIndication_UIMState
{
   eQMIDMSUIMStates mUIMState;
};

// Structure to describe indication TLV 0x16 for DMS EventReport
struct sDMSEventReportIndication_WirelessDisableState
{
   INT8 mWirelessDisableOn;
};

// Structure to describe indication TLV 0x17 for DMS EventReport
struct sDMSEventReportIndication_PRLInit
{
   INT8 mPRLLoaded;
};

// Structure to describe response TLV 0x01 for DMSGetDeviceCapabilities()
struct sDMSGetDeviceCapabilitiesResponse_Capabilities
{
   UINT32 mMaxTXRatebps;
   UINT32 mMaxRXRatebps;
   eQMIDMSDataServiceCapabilities1 mDataServiceCapability;
   INT8 mSIMSupported;
   UINT8 mRadioInterfaceCount;

   // This array must be the size specified by mRadioInterfaceCount
   // eQMIDMSRadioInterfaces mRadioInterface[1];
};

// Structure to describe response TLV 0x10 for DMSGetDeviceCapabilities()
struct sDMSGetDeviceCapabilitiesResponse_ServiceCapability
{
   eQMIDMSServiceCapabilities mServiceCapability;
};

// Structure to describe response TLV 0x11 for DMSGetDeviceCapabilities()
struct sDMSGetDeviceCapabilitiesResponse_VoiceCapability
{
   bool mGWCSFBCapable:1;
   bool m1xCSFBCapable:1;
   bool mVoLTECapable:1;

   // Padding out 61 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2[7];
};

// Structure to describe response TLV 0x01 for DMSGetDeviceManfacturer()
struct sDMSGetDeviceManfacturerResponse_Manfacturer
{
   // String is variable length, but must be size of the container
   // char mDeviceManfacturer[1];
};

// Structure to describe response TLV 0x01 for DMSGetDeviceModel()
struct sDMSGetDeviceModelResponse_Model
{
   // String is variable length, but must be size of the container
   // char mDeviceModelID[1];
};

// Structure to describe response TLV 0x01 for DMSGetDeviceRevision()
struct sDMSGetDeviceRevisionResponse_Revision
{
   // String is variable length, but must be size of the container
   // char mDeviceRevisionID[1];
};

// Structure to describe response TLV 0x10 for DMSGetDeviceRevision()
struct sDMSGetDeviceRevisionResponse_BootCodeRevision
{
   // String is variable length, but must be size of the container
   // char mBootCodeRevisionID[1];
};

// Structure to describe response TLV 0x11 for DMSGetDeviceRevision()
struct sDMSGetDeviceRevisionResponse_UQCNRevision
{
   // String is variable length, but must be size of the container
   // char mBootCodeRevisionID[1];
};

// Structure to describe response TLV 0x01 for DMSGetDeviceVoiceNumber()
struct sDMSGetDeviceVoiceNumberResponse_VoiceNumber
{
   // String is variable length, but must be size of the container
   // char mDeviceVoiceNumber[1];
};

// Structure to describe response TLV 0x10 for DMSGetDeviceVoiceNumber()
struct sDMSGetDeviceVoiceNumberResponse_MobileIDNumber
{
   // String is variable length, but must be size of the container
   // char mDeviceMobileIDNumber[1];
};

// Structure to describe response TLV 0x11 for DMSGetDeviceVoiceNumber()
struct sDMSGetDeviceVoiceNumberResponse_IMSI
{
   // String is variable length, but must be size of the container
   // char mIMSI[1];
};

// Structure to describe response TLV 0x10 for DMSGetDeviceSerialNumbers()
struct sDMSGetDeviceSerialNumbersResponse_ESN
{
   // String is variable length, but must be size of the container
   // char mESN[1];
};

// Structure to describe response TLV 0x11 for DMSGetDeviceSerialNumbers()
struct sDMSGetDeviceSerialNumbersResponse_IMEI
{
   // String is variable length, but must be size of the container
   // char mIMEI[1];
};

// Structure to describe response TLV 0x12 for DMSGetDeviceSerialNumbers()
struct sDMSGetDeviceSerialNumbersResponse_MEID
{
   // String is variable length, but must be size of the container
   // char mMEID[1];
};

// Structure to describe response TLV 0x01 for DMSGetPowerState()
struct sDMSGetPowerStateResponse_PowerState
{
   eQMIDMSPowerSources mPowerSource:1;
   bool mBatteryConnected:1;
   bool mBatteryCharging:1;
   bool mPowerFault:1;

   // Padding out 4 bits
   UINT8 mReserved1:4;

   UINT8 mBatteryLevel;
};

// Structure to describe request TLV 0x01 for DMSUIMSetPINProtection()
struct sDMSUIMSetPINProtectionRequest_Info
{
   UINT8 mPINID;
   UINT8 mPINEnabled;
   UINT8 mPINLength;

   // This array must be the size specified by mPINLength
   // char mPINValue[1];
};

// Structure to describe response TLV 0x10 for DMSUIMSetPINProtection()
struct sDMSUIMSetPINProtectionResponse_RetryInfo
{
   UINT8 mRemainingVerifyRetries;
   UINT8 mRemainingUnblockRetries;
};

// Structure to describe request TLV 0x01 for DMSUIMVerifyPIN()
struct sDMSUIMVerifyPINRequest_Info
{
   UINT8 mPINID;
   UINT8 mPINLength;

   // This array must be the size specified by mPINLength
   // char mPINValue[1];
};

// Structure to describe response TLV 0x10 for DMSUIMVerifyPIN()
struct sDMSUIMVerifyPINResponse_RetryInfo
{
   UINT8 mRemainingVerifyRetries;
   UINT8 mRemainingUnblockRetries;
};

// Structure to describe request TLV 0x01 for DMSUIMUnblockPIN()
struct sDMSUIMUnblockPINRequest_Info1
{
   UINT8 mPINID;
   UINT8 mPUKLength;

   // This array must be the size specified by mPUKLength
   // char mPUKValue[1];
};

struct sDMSUIMUnblockPINRequest_Info2
{
   UINT8 mNewPINLength;

   // This array must be the size specified by mNewPINLength
   // char mNewPINValue[1];
};

struct sDMSUIMUnblockPINRequest_Info
{
   sDMSUIMUnblockPINRequest_Info1 mDMSUIMUnblockPINRequest_Info1;
   sDMSUIMUnblockPINRequest_Info2 mDMSUIMUnblockPINRequest_Info2;
};

// Structure to describe response TLV 0x10 for DMSUIMUnblockPIN()
struct sDMSUIMUnblockPINResponse_RetryInfo
{
   UINT8 mRemainingVerifyRetries;
   UINT8 mRemainingUnblockRetries;
};

// Structure to describe request TLV 0x01 for DMSUIMChangePIN()
struct sDMSUIMChangePINRequest_Info1
{
   UINT8 mPINID;
   UINT8 mOldPINLength;

   // This array must be the size specified by mOldPINLength
   // char mOldPINValue[1];
};

struct sDMSUIMChangePINRequest_Info2
{
   UINT8 mNewPINLength;

   // This array must be the size specified by mNewPINLength
   // char mNewPINValue[1];
};

struct sDMSUIMChangePINRequest_Info
{
   sDMSUIMChangePINRequest_Info1 mDMSUIMChangePINRequest_Info1;
   sDMSUIMChangePINRequest_Info2 mDMSUIMChangePINRequest_Info2;
};

// Structure to describe response TLV 0x10 for DMSUIMChangePIN()
struct sDMSUIMChangePINResponse_RetryInfo
{
   UINT8 mRemainingVerifyRetries;
   UINT8 mRemainingUnblockRetries;
};

// Structure to describe response TLV 0x11 for DMSUIMGetPINStatus()
struct sDMSUIMGetPINStatusResponse_PIN1Status
{
   eQMIDMSPINStatus mPINStatus;
   UINT8 mRemainingVerifyRetries;
   UINT8 mRemainingUnblockRetries;
};

// Structure to describe response TLV 0x12 for DMSUIMGetPINStatus()
struct sDMSUIMGetPINStatusResponse_PIN2Status
{
   eQMIDMSPINStatus mPINStatus;
   UINT8 mRemainingVerifyRetries;
   UINT8 mRemainingUnblockRetries;
};

// Structure to describe response TLV 0x01 for DMSGetHardwareRevision()
struct sDMSGetHardwareRevisionResponse_HardwareRevision
{
   // String is variable length, but must be size of the container
   // char mDeviceHardwareRevision[1];
};

// Structure to describe response TLV 0x01 for DMSGetOperatingMode()
struct sDMSGetOperatingModeResponse_OperatingMode
{
   eQMIDMSOperatingModes mOperatingMode;
};

// Structure to describe response TLV 0x10 for DMSGetOperatingMode()
struct sDMSGetOperatingModeResponse_OfflineReason
{
   UINT8 mHostImageMismatch:1;
   UINT8 mUQCNImageMismatch:1;
   UINT8 mIncompatibleUQCN:1;
   UINT8 mUQCNCopyIssue:1;

   // Padding out 12 bits
   UINT8 mReserved1:4;
   UINT8 mReserved2;
};

// Structure to describe response TLV 0x11 for DMSGetOperatingMode()
struct sDMSGetOperatingModeResponse_PlatformRestricted
{
   INT8 mPlatformRestricted;
};

// Structure to describe request TLV 0x01 for DMSSetOperatingMode()
struct sDMSSetOperatingModeRequest_OperatingMode
{
   eQMIDMSOperatingModes mOperatingMode;
};

// Structure to describe response TLV 0x01 for DMSGetTimestamp()
struct sDMSGetTimestampResponse_Timestamp
{
   UINT64 mTimestamp:48;

   // mSource is of type eQMIDMSTimestampSources
   UINT64 mSource:16;
};

// Structure to describe response TLV 0x01 for DMSGetPRLVersion()
struct sDMSGetPRLVersionResponse_PRLVersion
{
   UINT16 mPRLVersion;
};

// Structure to describe response TLV 0x01 for DMSGetActivationState()
struct sDMSGetActivationStateResponse_ActivationState
{
   eQMIDMSActivationStates mActivationState;
};

// Structure to describe request TLV 0x01 for DMSActivateAutomatic()
struct sDMSActivateAutomaticRequest_ActivationCode
{
   UINT8 mCodeLength;

   // This array must be the size specified by mCodeLength
   // char mCode[1];
};

// Structure to describe request TLV 0x01 for DMSActivateManual()
struct sDMSActivateManualRequest_ActivationData1
{
   char mSPC[6];
   UINT16 mSID;
   UINT8 mMDNLength;

   // This array must be the size specified by mMDNLength
   // char mMDN[1];
};

struct sDMSActivateManualRequest_ActivationData2
{
   UINT8 mMINLength;

   // This array must be the size specified by mMINLength
   // char mMIN[1];
};

struct sDMSActivateManualRequest_ActivationData
{
   sDMSActivateManualRequest_ActivationData1 mDMSActivateManualRequest_ActivationData1;
   sDMSActivateManualRequest_ActivationData2 mDMSActivateManualRequest_ActivationData2;
};

// Structure to describe request TLV 0x10 for DMSActivateManual()
struct sDMSActivateManualRequest_PRLObsolete
{
   UINT16 mPRLLength;

   // This array must be the size specified by mPRLLength
   // UINT8 mPRL[1];
};

// Structure to describe request TLV 0x11 for DMSActivateManual()
struct sDMSActivateManualRequest_MNHAKey
{
   UINT8 mMNHALength;

   // This array must be the size specified by mMNHALength
   // char mMNHA[1];
};

// Structure to describe request TLV 0x12 for DMSActivateManual()
struct sDMSActivateManualRequest_MNAAAKey
{
   UINT8 mMNAAALength;

   // This array must be the size specified by mMNAAALength
   // char mMNAAA[1];
};

// Structure to describe request TLV 0x13 for DMSActivateManual()
struct sDMSActivateManualRequest_PRL
{
   UINT16 mPRLTotalLength;
   UINT16 mPRLSegmentLength;
   UINT8 mPRLSegmentID;

   // This array must be the size specified by mPRLSegmentLength
   // UINT8 mPRL[1];
};

// Structure to describe response TLV 0x01 for DMSGetLockState()
struct sDMSGetLockStateResponse_LockState
{
   eQMIDMSLockStates mLockState;
};

// Structure to describe request TLV 0x01 for DMSSetLockState()
struct sDMSSetLockStateRequest_LockState
{
   eQMIDMSLockStates mLockState;
   char mLockCode[4];
};

// Structure to describe request TLV 0x01 for DMSSetLockCode()
struct sDMSSetLockCodeRequest_LockCode
{
   char mCurrentLockCode[4];
   char mNewLockCode[4];
};

// Structure to describe response TLV 0x01 for DMSReadUserData()
struct sDMSReadUserDataResponse_UserData
{
   UINT16 mDataLength;

   // This array must be the size specified by mDataLength
   // UINT8 mData[1];
};

// Structure to describe request TLV 0x01 for DMSWriteUserData()
struct sDMSWriteUserDataRequest_UserData
{
   UINT16 mDataLength;

   // This array must be the size specified by mDataLength
   // UINT8 mData[1];
};

// Structure to describe response TLV 0x01 for DMSReadERIData()
struct sDMSReadERIDataResponse_UserData
{
   UINT16 mDataLength;

   // This array must be the size specified by mDataLength
   // UINT8 mData[1];
};

// Structure to describe request TLV 0x01 for DMSResetFactoryDefaults()
struct sDMSResetFactoryDefaultsRequest_SPC
{
   char mSPC[6];
};

// Structure to describe request TLV 0x01 for DMSValidateSPC()
struct sDMSValidateSPCRequest_SPC
{
   char mSPC[6];
};

// Structure to describe response TLV 0x01 for DMSUIMGetICCID()
struct sDMSUIMGetICCIDResponse_ICCID
{
   // String is variable length, but must be size of the container
   // char mICCID[1];
};

// Structure to describe response TLV 0x01 for DMSUIMGetHostLockID()
struct sDMSUIMGetHostLockIDResponse_ID
{
   UINT32 mHostLockCode;
};

// Structure to describe request TLV 0x01 for DMSUIMGetControlKeyStatus()
struct sDMSUIMGetControlKeyStatusRequest_Facility
{
   eQMIDMSUIMFacility mFacility;
};

// Structure to describe response TLV 0x01 for DMSUIMGetControlKeyStatus()
struct sDMSUIMGetControlKeyStatusResponse_Status
{
   eQMIDMSUIMFacilityStates mFacilityState;
   UINT8 mRemainingVerifyRetries;
   UINT8 mRemainingUnblockRetries;
};

// Structure to describe response TLV 0x10 for DMSUIMGetControlKeyStatus()
struct sDMSUIMGetControlKeyStatusResponse_Blocking
{
   INT8 mOperationBlocking;
};

// Structure to describe request TLV 0x01 for DMSUIMSetControlKeyProtection()
struct sDMSUIMSetControlKeyProtectionRequest_Facility
{
   eQMIDMSUIMFacility mFacility;
   eQMIDMSUIMFacilityStates mFacilityState;
   UINT8 mControlKeyLength;

   // This array must be the size specified by mControlKeyLength
   // char mControlKey[1];
};

// Structure to describe response TLV 0x10 for DMSUIMSetControlKeyProtection()
struct sDMSUIMSetControlKeyProtectionResponse_Status
{
   UINT8 mRemainingVerifyRetries;
};

// Structure to describe request TLV 0x01 for DMSUIMUnblockControlKey()
struct sDMSUIMUnblockControlKeyRequest_Facility
{
   eQMIDMSUIMFacility mFacility;
   UINT8 mControlKeyLength;

   // This array must be the size specified by mControlKeyLength
   // char mControlKey[1];
};

// Structure to describe response TLV 0x10 for DMSUIMUnblockControlKey()
struct sDMSUIMUnblockControlKeyResponse_Status
{
   UINT8 mRemainingUnblockRetries;
};

// Structure to describe response TLV 0x01 for DMSGetIMSI()
struct sDMSGetIMSIResponse_IMSI
{
   // String is variable length, but must be size of the container
   // char mIMSI[1];
};

// Structure to describe response TLV 0x01 for DMSGetUIMState()
struct sDMSGetUIMStateResponse_State
{
   eQMIDMSUIMStates mUIMState;
};

// Structure to describe response TLV 0x01 for DMSGetBandCapabilities()
struct sDMSGetBandCapabilitiesResponse_Bands
{
   bool mBandClass0ASystem:1;
   bool mBandClass0BSystem:1;
   bool mBandClass1:1;
   bool mBandClass2:1;
   bool mBandClass3ASystem:1;
   bool mBandClass4:1;
   bool mBandClass5:1;
   bool mGSMDCS:1;
   bool mGSMPrimary:1;
   bool mGSMExtended:1;
   bool mBandClass6:1;
   bool mBandClass7:1;
   bool mBandClass8:1;
   bool mBandClass9:1;
   bool mBandClass10:1;
   bool mBandClass11:1;
   bool mGSM450:1;
   bool mGSM480:1;
   bool mGSM750:1;
   bool mGSM850:1;
   bool mGSMRailways:1;
   bool mGSMPCS:1;
   bool mWCDMA2100I:1;
   bool mWCDMAPCS1900:1;
   bool mWCDMADCS1800:1;
   bool mWCDMA1700US:1;
   bool mWCDMA850:1;
   bool mWCDMA800:1;
   bool mBandClass12:1;
   bool mBandClass14:1;

   // Padding out 1 bits
   UINT8 mReserved1:1;

   bool mBandClass15:1;

   // Padding out 16 bits
   UINT8 mReserved2[2];

   bool mWCDMA2600:1;
   bool mWCDMA900:1;
   bool mWCDMA1700Japan:1;

   // Padding out 5 bits
   UINT8 mReserved3:5;

   bool mBandClass16:1;
   bool mBandClass17:1;
   bool mBandClass18:1;
   bool mBandClass19:1;

   // Padding out 4 bits
   UINT8 mReserved4:4;
};

// Structure to describe response TLV 0x10 for DMSGetBandCapabilities()
struct sDMSGetBandCapabilitiesResponse_LTEBands
{
   bool mEUTRANBand1:1;
   bool mEUTRANBand2:1;
   bool mEUTRANBand3:1;
   bool mEUTRANBand4:1;
   bool mEUTRANBand5:1;
   bool mEUTRANBand6:1;
   bool mEUTRANBand7:1;
   bool mEUTRANBand8:1;
   bool mEUTRANBand9:1;
   bool mEUTRANBand10:1;
   bool mEUTRANBand11:1;
   bool mEUTRANBand12:1;
   bool mEUTRANBand13:1;
   bool mEUTRANBand14:1;

   // Padding out 2 bits
   UINT8 mReserved1:2;

   bool mEUTRANBand17:1;
   bool mEUTRANBand18:1;
   bool mEUTRANBand19:1;
   bool mEUTRANBand20:1;
   bool mEUTRANBand21:1;

   // Padding out 2 bits
   UINT8 mReserved2:2;

   bool mEUTRANBand24:1;
   bool mEUTRANBand25:1;

   // Padding out 7 bits
   UINT8 mReserved3:7;

   bool mEUTRANBand33:1;
   bool mEUTRANBand34:1;
   bool mEUTRANBand35:1;
   bool mEUTRANBand36:1;
   bool mEUTRANBand37:1;
   bool mEUTRANBand38:1;
   bool mEUTRANBand39:1;
   bool mEUTRANBand40:1;
   bool mEUTRANBand41:1;

   // Padding out 23 bits
   UINT8 mReserved4:7;
   UINT8 mReserved5[2];
};

// Structure to describe response TLV 0x11 for DMSGetBandCapabilities()
struct sDMSGetBandCapabilitiesResponse_TDSBands
{
   bool mTDSBandA:1;
   bool mTDSBandB:1;
   bool mTDSBandC:1;
   bool mTDSBandD:1;
   bool mTDSBandE:1;
   bool mTDSBandF:1;

   // Padding out 58 bits
   UINT8 mReserved1:2;
   UINT8 mReserved2[7];
};

// Structure to describe response TLV 0x01 for DMSGetFactorySerialNumber()
struct sDMSGetFactorySerialNumberResponse_ID
{
   // String is variable length, but must be size of the container
   // char mFactorySerialNumber[1];
};

// Structure to describe request TLV 0x01 for DMSSetDeviceTime()
struct sDMSSetDeviceTimeRequest_Time
{
   UINT64 mTimeInMilliseconds;
};

// Structure to describe request TLV 0x10 for DMSSetDeviceTime()
struct sDMSSetDeviceTimeRequest_Type
{
   eQMIDMSTimeReferences mTimeReference;
};

// Structure to describe response TLV 0x01 for DMSGetSoftwareVersion()
struct sDMSGetSoftwareVersionResponse_Version
{
   // String is variable length, but must be size of the container
   // char mSoftwareVersion[1];
};

// Structure to describe request TLV 0x01 for DMSSetSPC()
struct sDMSSetSPCRequest_CurrentSPC
{
   char mCurrentSPC[6];
};

// Structure to describe response TLV 0x10 for DMSGetCurrentPRLInfo()
struct sDMSGetCurrentPRLInfoResponse_Version
{
   UINT16 mPRLVersion;
};

// Structure to describe response TLV 0x11 for DMSGetCurrentPRLInfo()
struct sDMSGetCurrentPRLInfoResponse_Preference
{
   INT8 mPRLOnlyPreferenceSet;
};

// Structure to describe request TLV 0x01 for NASAbort()
struct sNASAbortRequest_TransactionID
{
   UINT16 mTransactionID;
};

// Structure to describe request TLV 0x10 for NASSetEventReport()
struct sNASSetEventReportRequest_SignalIndicator
{
   INT8 mReportSignalStrength;
   UINT8 mNumberOfThresholds;

   // This array must be the size specified by mNumberOfThresholds
   // INT8 mSignalStrengthThresholddBm[1];
};

// Structure to describe request TLV 0x11 for NASSetEventReport()
struct sNASSetEventReportRequest_RFIndicator
{
   INT8 mReportRFInfo;
};

// Structure to describe request TLV 0x12 for NASSetEventReport()
struct sNASSetEventReportRequest_RegistrationRejectIndicator
{
   INT8 mReportLUReject;
};

// Structure to describe request TLV 0x13 for NASSetEventReport()
struct sNASSetEventReportRequest_RSSIIndicator
{
   INT8 mReportRSSI;
   UINT8 mRSSIDelta;
};

// Structure to describe request TLV 0x14 for NASSetEventReport()
struct sNASSetEventReportRequest_ECIOIndicator
{
   INT8 mReportECIO;
   UINT8 mECIODelta;
};

// Structure to describe request TLV 0x15 for NASSetEventReport()
struct sNASSetEventReportRequest_IOIndicator
{
   INT8 mReportIO;
   UINT8 mIODelta;
};

// Structure to describe request TLV 0x16 for NASSetEventReport()
struct sNASSetEventReportRequest_SINRIndicator
{
   INT8 mReportSINR;
   UINT8 mSINRDelta;
};

// Structure to describe request TLV 0x17 for NASSetEventReport()
struct sNASSetEventReportRequest_ErrorRateIndicator
{
   INT8 mReportErrorRate;
};

// Structure to describe request TLV 0x18 for NASSetEventReport()
struct sNASSetEventReportRequest_RSRQIndicator
{
   INT8 mReportRSRQ;
   UINT8 mRSRQDelta;
};

// Structure to describe request TLV 0x19 for NASSetEventReport()
struct sNASSetEventReportRequest_ECIOThreshold
{
   INT8 mReportECIO;
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT16 mThreshold[1];
};

// Structure to describe request TLV 0x1A for NASSetEventReport()
struct sNASSetEventReportRequest_SINRThreshold
{
   INT8 mReportSINR;
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // UINT8 mThreshold[1];
};

// Structure to describe request TLV 0x1B for NASSetEventReport()
struct sNASSetEventReportRequest_LTESNRDelta
{
   INT8 mReportLTESNR;
   UINT16 mLTESNRDelta;
};

// Structure to describe request TLV 0x1C for NASSetEventReport()
struct sNASSetEventReportRequest_LTERSPSDelta
{
   INT8 mReportLTERSRP;
   UINT8 mLTERSRPDelta;
};

// Structure to describe indication TLV 0x10 for NAS EventReport
struct sNASEventReportIndication_SignalStrength
{
   INT8 mSignalStrengthdBm;
   eQMINASRadioInterfaces mRadioInterface;
};

// Structure to describe indication TLV 0x11 for NAS EventReport
struct sNASEventReportIndication_RFInfo
{
   UINT8 mNumberOfInstances;

   struct sInstance
   {
      eQMINASRadioInterfaces mRadioInterface;
      eQMINASBandClasses mActiveBandClass;
      UINT16 mActiveChannel;
   };

   // This array must be the size specified by mNumberOfInstances
   // sInstance mInstances[1];
};

// Structure to describe indication TLV 0x12 for NAS EventReport
struct sNASEventReportIndication_RegistrationReject
{
   eQMINASServiceDomains mServiceDomain;
   UINT16 mRejectCause;
};

// Structure to describe indication TLV 0x13 for NAS EventReport
struct sNASEventReportIndication_RSSI
{
   UINT8 mRSSIDelta;
   eQMINASRadioInterfaces mRadioInterface;
};

// Structure to describe indication TLV 0x14 for NAS EventReport
struct sNASEventReportIndication_ECIO
{
   UINT8 mECIO;
   eQMINASRadioInterfaces mRadioInterface;
};

// Structure to describe indication TLV 0x15 for NAS EventReport
struct sNASEventReportIndication_IO
{
   UINT32 mIO;
};

// Structure to describe indication TLV 0x16 for NAS EventReport
struct sNASEventReportIndication_SINR
{
   eQMINASSINRLevels mSINR;
};

// Structure to describe indication TLV 0x17 for NAS EventReport
struct sNASEventReportIndication_ErrorRate
{
   UINT16 mErrorRate;
   eQMINASRadioInterfaces mRadioInterface;
};

// Structure to describe indication TLV 0x18 for NAS EventReport
struct sNASEventReportIndication_RSRQ
{
   INT8 mRSRQ;
   eQMINASRadioInterfaces mRadioInterface;
};

// Structure to describe indication TLV 0x19 for NAS EventReport
struct sNASEventReportIndication_LTESNR
{
   INT16 mLTESNR;
};

// Structure to describe indication TLV 0x1A for NAS EventReport
struct sNASEventReportIndication_LTERSRP
{
   INT16 mLTERSRP;
};

// Structure to describe request TLV 0x10 for NASSetRegistrationEventReport()
struct sNASSetRegistrationEventReportRequest_SystemSelectIndicator
{
   INT8 mReportSystemSelect;
};

// Structure to describe request TLV 0x12 for NASSetRegistrationEventReport()
struct sNASSetRegistrationEventReportRequest_DDTMIndicator
{
   INT8 mReportDDTM;
};

// Structure to describe request TLV 0x13 for NASSetRegistrationEventReport()
struct sNASSetRegistrationEventReportRequest_ServingSystemIndicator
{
   INT8 mReportServingSystem;
};

// Structure to describe request TLV 0x14 for NASSetRegistrationEventReport()
struct sNASSetRegistrationEventReportRequest_DualStandbyIndicator
{
   INT8 mReportDualStandby;
};

// Structure to describe request TLV 0x15 for NASSetRegistrationEventReport()
struct sNASSetRegistrationEventReportRequest_SubscriptionInformationIndicator
{
   INT8 mReportSubscriptionInformation;
};

// Structure to describe request TLV 0x17 for NASSetRegistrationEventReport()
struct sNASSetRegistrationEventReportRequest_NetworkTimeIndicator
{
   INT8 mReportNetworkTime;
};

// Structure to describe request TLV 0x18 for NASSetRegistrationEventReport()
struct sNASSetRegistrationEventReportRequest_SystemInformationIndicator
{
   INT8 mReportSystemInformation;
};

// Structure to describe request TLV 0x19 for NASSetRegistrationEventReport()
struct sNASSetRegistrationEventReportRequest_SignalStrengthIndicator
{
   INT8 mReportSignalStrength;
};

// Structure to describe request TLV 0x1A for NASSetRegistrationEventReport()
struct sNASSetRegistrationEventReportRequest_ErrorRateIndicator
{
   INT8 mReportErrorRate;
};

// Structure to describe request TLV 0x1B for NASSetRegistrationEventReport()
struct sNASSetRegistrationEventReportRequest_NewEVDOUATIIndicator
{
   INT8 mReportNewEVDOUATI;
};

// Structure to describe request TLV 0x1C for NASSetRegistrationEventReport()
struct sNASSetRegistrationEventReportRequest_EVDOSessionIndicator
{
   INT8 mReportEVDOSessionClose;
};

// Structure to describe request TLV 0x1D for NASSetRegistrationEventReport()
struct sNASSetRegistrationEventReportRequest_ManagedRoamingIndicator
{
   INT8 mReportManagedRoaming;
};

// Structure to describe request TLV 0x1E for NASSetRegistrationEventReport()
struct sNASSetRegistrationEventReportRequest_CurrentPLMNName
{
   INT8 mReportCurrentPLMNName;
};

// Structure to describe request TLV 0x1F for NASSetRegistrationEventReport()
struct sNASSetRegistrationEventReportRequest_EMBMSStatus
{
   INT8 mReportEMBMSStatus;
};

// Structure to describe request TLV 0x20 for NASSetRegistrationEventReport()
struct sNASSetRegistrationEventReportRequest_RFBandInfo
{
   INT8 mReportRFBandInfo;
};

// Structure to describe request TLV 0x21 for NASSetRegistrationEventReport()
struct sNASSetRegistrationEventReportRequest_NetworkReject
{
   INT8 mNetworkRejectEnabled;
   INT8 mSupressSytemInfoEnabled;
};

// Structure to describe request TLV 0x22 for NASSetRegistrationEventReport()
struct sNASSetRegistrationEventReportRequest_OperatorNameData
{
   INT8 mOperatorNameDataEnabled;
};

// Structure to describe request TLV 0x23 for NASSetRegistrationEventReport()
struct sNASSetRegistrationEventReportRequest_CSPPLMNModeBit
{
   INT8 mCSPPLMNModeBitEnabled;
};

// Structure to describe request TLV 0x24 for NASSetRegistrationEventReport()
struct sNASSetRegistrationEventReportRequest_RTREConfiguration
{
   INT8 mRTREConfigurationEnabled;
};

// Structure to describe request TLV 0x10 for NASGetSignalStrength()
struct sNASGetSignalStrengthRequest_RequestMask
{
   bool mQueryRSSI:1;
   bool mQueryECIO:1;
   bool mQueryIO:1;
   bool mQuerySINR:1;
   bool mQueryErrorRate:1;
   bool mQueryRSRQ:1;
   bool mQueryLTESNR:1;
   bool mQueryLTERSRQ:1;

   // Padding out 8 bits
   UINT8 mReserved1;
};

// Structure to describe response TLV 0x01 for NASGetSignalStrength()
struct sNASGetSignalStrengthResponse_SignalStrength
{
   INT8 mSignalStrengthdBm;
   eQMINASRadioInterfaces mRadioInterface;
};

// Structure to describe response TLV 0x10 for NASGetSignalStrength()
struct sNASGetSignalStrengthResponse_SignalStrengthList
{
   UINT16 mNumberOfInfoInstances;

   struct sInfo
   {
      INT8 mSignalStrengthdBm;
      eQMINASRadioInterfaces mRadioInterface;
   };

   // This array must be the size specified by mNumberOfInfoInstances
   // sInfo mInfos[1];
};

// Structure to describe response TLV 0x11 for NASGetSignalStrength()
struct sNASGetSignalStrengthResponse_RSSIList
{
   UINT16 mNumberOfMeasurements;

   struct sMeasurement
   {
      UINT8 mRSSIDelta;
      eQMINASRadioInterfaces mRadioInterface;
   };

   // This array must be the size specified by mNumberOfMeasurements
   // sMeasurement mMeasurements[1];
};

// Structure to describe response TLV 0x12 for NASGetSignalStrength()
struct sNASGetSignalStrengthResponse_ECIOList
{
   UINT16 mNumberOfMeasurements;

   struct sMeasurement
   {
      UINT8 mECIO;
      eQMINASRadioInterfaces mRadioInterface;
   };

   // This array must be the size specified by mNumberOfMeasurements
   // sMeasurement mMeasurements[1];
};

// Structure to describe response TLV 0x13 for NASGetSignalStrength()
struct sNASGetSignalStrengthResponse_IO
{
   UINT32 mIO;
};

// Structure to describe response TLV 0x14 for NASGetSignalStrength()
struct sNASGetSignalStrengthResponse_SINR
{
   eQMINASSINRLevels mSINR;
};

// Structure to describe response TLV 0x15 for NASGetSignalStrength()
struct sNASGetSignalStrengthResponse_ErrorRateList
{
   UINT16 mNumberOfMeasurements;

   struct sMeasurement
   {
      UINT16 mErrorRate;
      eQMINASRadioInterfaces mRadioInterface;
   };

   // This array must be the size specified by mNumberOfMeasurements
   // sMeasurement mMeasurements[1];
};

// Structure to describe response TLV 0x16 for NASGetSignalStrength()
struct sNASGetSignalStrengthResponse_RSRQ
{
   INT8 mRSRQ;
   eQMINASRadioInterfaces mRadioInterface;
};

// Structure to describe response TLV 0x17 for NASGetSignalStrength()
struct sNASGetSignalStrengthResponse_LTESNR
{
   INT16 mLTESNR;
};

// Structure to describe response TLV 0x18 for NASGetSignalStrength()
struct sNASGetSignalStrengthResponse_LTERSRQ
{
   INT16 mLTERSRP;
};

// Structure to describe request TLV 0x10 for NASPerformNetworkScan()
struct sNASPerformNetworkScanRequest_NetworkMask
{
   bool mGSM:1;
   bool mUMTS:1;
   bool mLTE:1;
   bool mTDSCDMA:1;

   // Padding out 4 bits
   UINT8 mReserved1:4;
};

// Structure to describe response TLV 0x10 for NASPerformNetworkScan()
struct sNASPerformNetworkScanResponse_NetworkInfo
{
   UINT16 mNumberOfInfoInstances;

   struct sNetworkInfo
   {
      UINT16 mMobileCountryCode;
      UINT16 mMobileNetworkCode;
      eQMINASInUseStates mInUseStatus:2;
      eQMINASRoamingStates mRoamingStatus:2;
      eQMINASForbiddenStates mForbiddenStatus:2;
      eQMINASPreferredStates mPreferredStatus:2;
      UINT8 mDescriptionLength;
   
      // This array must be the size specified by mDescriptionLength
      // char mDescription[1];
   };

   // This array must be the size specified by mNumberOfInfoInstances
   // sNetworkInfo mNetworkInfos[1];
};

// Structure to describe response TLV 0x11 for NASPerformNetworkScan()
struct sNASPerformNetworkScanResponse_NetworkRAT
{
   UINT16 mNumberOfInfoInstances;

   struct sInfo
   {
      UINT16 mMobileCountryCode;
      UINT16 mMobileNetworkCode;
      eQMINASRadioAccessTechnologies mRadioAccessTechnology;
   };

   // This array must be the size specified by mNumberOfInfoInstances
   // sInfo mInfos[1];
};

// Structure to describe response TLV 0x12 for NASPerformNetworkScan()
struct sNASPerformNetworkScanResponse_PCSInfo
{
   UINT16 mPCSInfoCount;

   struct sPCSInfo
   {
      UINT16 mMobileCountryCode;
      UINT16 mMobileNetworkCode;
      INT8 mMNCIncludesPCSDigit;
   };

   // This array must be the size specified by mPCSInfoCount
   // sPCSInfo mPCSInfos[1];
};

// Structure to describe response TLV 0x13 for NASPerformNetworkScan()
struct sNASPerformNetworkScanResponse_NetworkScanResult
{
   eQMINASNetworkScanResult mNetworkScanResult;
};

// Structure to describe request TLV 0x01 for NASInitiateNetworkRegister()
struct sNASInitiateNetworkRegisterRequest_Action
{
   eQMINASRegisterActions mRegisterAction;
};

// Structure to describe request TLV 0x10 for NASInitiateNetworkRegister()
struct sNASInitiateNetworkRegisterRequest_ManualInfo
{
   UINT16 mMobileCountryCode;
   UINT16 mMobileNetworkCode;
   eQMINASRadioAccessTechnologies mRadioAccessTechnology;
};

// Structure to describe request TLV 0x11 for NASInitiateNetworkRegister()
struct sNASInitiateNetworkRegisterRequest_ChangeDuration
{
   eQMINASChangeDuration mChangeDuration;
};

// Structure to describe request TLV 0x12 for NASInitiateNetworkRegister()
struct sNASInitiateNetworkRegisterRequest_PCSInfo
{
   INT8 mMNCIncludesPCSDigit;
};

// Structure to describe request TLV 0x10 for NASInitiateAttach()
struct sNASInitiateAttachRequest_Action
{
   eQMINASPSAttachActions mPSAttachAction;
};

// Structure to describe response TLV 0x01 for NASGetServingSystem()
struct sNASGetServingSystemResponse_ServingSystem
{
   eQMINASRegistrationStates mRegistrationState;
   eQMINASCSPSAttachStates mCSAttachState;
   eQMINASCSPSAttachStates mPSAttachState;
   eQMINASRegisteredNetworks mRegisteredNetwork;
   UINT8 mNumberOfRadioInterfacesInUse;

   // This array must be the size specified by mNumberOfRadioInterfacesInUse
   // eQMINASRadioInterfaces mRadioInterface[1];
};

// Structure to describe response TLV 0x10 for NASGetServingSystem()
struct sNASGetServingSystemResponse_RoamingIndicator
{
   eQMINASRoamingIndicators mRoamingIndicator;
};

// Structure to describe response TLV 0x11 for NASGetServingSystem()
struct sNASGetServingSystemResponse_DataServices
{
   UINT8 mNumberOfDataCapabilities;

   // This array must be the size specified by mNumberOfDataCapabilities
   // eQMINASDataServiceCapabilities2 mDataCapability[1];
};

// Structure to describe response TLV 0x12 for NASGetServingSystem()
struct sNASGetServingSystemResponse_CurrentPLMN
{
   UINT16 mMobileCountryCode;
   UINT16 mMobileNetworkCode;
   UINT8 mDescriptionLength;

   // This array must be the size specified by mDescriptionLength
   // char mDescription[1];
};

// Structure to describe response TLV 0x13 for NASGetServingSystem()
struct sNASGetServingSystemResponse_SystemID
{
   UINT16 mSystemID;
   UINT16 mNetworkID;
};

// Structure to describe response TLV 0x14 for NASGetServingSystem()
struct sNASGetServingSystemResponse_BaseStation
{
   UINT16 mBaseStationID;
   INT32 mLatitude;
   INT32 mLongitude;
};

// Structure to describe response TLV 0x15 for NASGetServingSystem()
struct sNASGetServingSystemResponse_RoamingList
{
   UINT8 mNumberOfInstances;

   struct sInstance
   {
      eQMINASRadioInterfaces mRadioInterface;
      eQMINASRoamingIndicators mRoamingIndicator;
   };

   // This array must be the size specified by mNumberOfInstances
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x16 for NASGetServingSystem()
struct sNASGetServingSystemResponse_DefaultRoaming
{
   eQMINASRoamingIndicators mRoamingIndicator;
};

// Structure to describe response TLV 0x17 for NASGetServingSystem()
struct sNASGetServingSystemResponse_3GPP2TimeZone
{
   UINT8 mLeapSeconds;
   INT8 mLocalTimeOffset;
   INT8 mDaylightSavingsInEffect;
};

// Structure to describe response TLV 0x18 for NASGetServingSystem()
struct sNASGetServingSystemResponse_ProtocolRevision
{
   eQMINASRevision mProtocolRevision;
};

// Structure to describe response TLV 0x1A for NASGetServingSystem()
struct sNASGetServingSystemResponse_3GPPTimeZone
{
   INT8 m3GPPTimeZone;
};

// Structure to describe response TLV 0x1B for NASGetServingSystem()
struct sNASGetServingSystemResponse_3GPPDaylightSavingsAdjustment
{
   eQMINASDaylightSavingsAdjustment mDaylightSavingsAdjustment;
};

// Structure to describe response TLV 0x1C for NASGetServingSystem()
struct sNASGetServingSystemResponse_3GPPLocationAreaCode
{
   UINT16 mLocationAreaCode;
};

// Structure to describe response TLV 0x1D for NASGetServingSystem()
struct sNASGetServingSystemResponse_3GPPCellID
{
   UINT32 mCellID;
};

// Structure to describe response TLV 0x1E for NASGetServingSystem()
struct sNASGetServingSystemResponse_3GPP2ConcurrentService
{
   eQMINASConcurrentService mConcurrentService;
};

// Structure to describe response TLV 0x1F for NASGetServingSystem()
struct sNASGetServingSystemResponse_3GPP2PRLIndicator
{
   eQMINASPRLIndicator mPRLIndicator;
};

// Structure to describe response TLV 0x20 for NASGetServingSystem()
struct sNASGetServingSystemResponse_DualTransferModeIndication
{
   eQMINASDualTransferMode mDualTransferMode;
};

// Structure to describe response TLV 0x21 for NASGetServingSystem()
struct sNASGetServingSystemResponse_DetailedServiceInformation
{
   eQMINASServiceStatus mServiceStatus;
   eQMINASSystemServiceCapabilities mSystemServiceCapabilities;
   eQMINASServiceStatus mCDMA1xEVDOServiceStatus;
   eQMINASCDMA1xEVDOHybridInformation mCDMA1xEVDOHybridInformation;
   eQMINASSystemForbidden mSystemForbidden;
};

// Structure to describe response TLV 0x22 for NASGetServingSystem()
struct sNASGetServingSystemResponse_CDMASystemInformation
{
   UINT16 mMobileCountryCode;
   UINT8 mIMSI_11_12;
};

// Structure to describe response TLV 0x23 for NASGetServingSystem()
struct sNASGetServingSystemResponse_CDMA1xEVDOPersonality
{
   eQMINASCDMA1xEVDOPersonality mCDMA1xEVDOPersonality;
};

// Structure to describe response TLV 0x24 for NASGetServingSystem()
struct sNASGetServingSystemResponse_TrackingAreaCode
{
   UINT16 mTrackingAreaCode;
};

// Structure to describe response TLV 0x25 for NASGetServingSystem()
struct sNASGetServingSystemResponse_CallBarring
{
   eQMINASCallBarringStatus mCSCallBarringStatus;
   eQMINASCallBarringStatus mPSCallBarringStatus;
};

// Structure to describe response TLV 0x26 for NASGetServingSystem()
struct sNASGetServingSystemResponse_UMTSPSC
{
   UINT16 mPrimaryScramblingCode;
};

// Structure to describe response TLV 0x27 for NASGetServingSystem()
struct sNASGetServingSystemResponse_PCSInfo
{
   UINT16 mMobileCountryCode;
   UINT16 mMobileNetworkCode;
   INT8 mMNCIncludesPCSDigit;
};

// Structure to describe response TLV 0x28 for NASGetServingSystem()
struct sNASGetServingSystemResponse_HSCallStatus
{
   eQMINASHighSpeedCallStatus mHighSpeedCallStatus;
};

// Structure to describe indication TLV 0x01 for NAS ServingSystemIndication
struct sNASServingSystemIndication_ServingSystem
{
   eQMINASRegistrationStates mRegistrationState;
   eQMINASCSPSAttachStates mCSAttachState;
   eQMINASCSPSAttachStates mPSAttachState;
   eQMINASRegisteredNetworks mRegisteredNetwork;
   UINT8 mNumberOfRadioInterfacesInUse;

   // This array must be the size specified by mNumberOfRadioInterfacesInUse
   // eQMINASRadioInterfaces mRadioInterface[1];
};

// Structure to describe indication TLV 0x10 for NAS ServingSystemIndication
struct sNASServingSystemIndication_RoamingIndicator
{
   eQMINASRoamingIndicators mRoamingIndicator;
};

// Structure to describe indication TLV 0x11 for NAS ServingSystemIndication
struct sNASServingSystemIndication_DataServices
{
   UINT8 mNumberOfDataCapabilities;

   // This array must be the size specified by mNumberOfDataCapabilities
   // eQMINASDataServiceCapabilities2 mDataCapability[1];
};

// Structure to describe indication TLV 0x12 for NAS ServingSystemIndication
struct sNASServingSystemIndication_CurrentPLMN
{
   UINT16 mMobileCountryCode;
   UINT16 mMobileNetworkCode;
   UINT8 mDescriptionLength;

   // This array must be the size specified by mDescriptionLength
   // char mDescription[1];
};

// Structure to describe indication TLV 0x13 for NAS ServingSystemIndication
struct sNASServingSystemIndication_SystemID
{
   UINT16 mSystemID;
   UINT16 mNetworkID;
};

// Structure to describe indication TLV 0x14 for NAS ServingSystemIndication
struct sNASServingSystemIndication_BaseStation
{
   UINT16 mBaseStationID;
   INT32 mLatitude;
   INT32 mLongitude;
};

// Structure to describe indication TLV 0x15 for NAS ServingSystemIndication
struct sNASServingSystemIndication_RoamingList
{
   UINT8 mNumberOfInstances;

   struct sInstance
   {
      eQMINASRadioInterfaces mRadioInterface;
      eQMINASRoamingIndicators mRoamingIndicator;
   };

   // This array must be the size specified by mNumberOfInstances
   // sInstance mInstances[1];
};

// Structure to describe indication TLV 0x16 for NAS ServingSystemIndication
struct sNASServingSystemIndication_DefaultRoaming
{
   eQMINASRoamingIndicators mRoamingIndicator;
};

// Structure to describe indication TLV 0x17 for NAS ServingSystemIndication
struct sNASServingSystemIndication_TimeZone
{
   UINT8 mLeapSeconds;
   INT8 mLocalTimeOffset;
   INT8 mDaylightSavingsInEffect;
};

// Structure to describe indication TLV 0x18 for NAS ServingSystemIndication
struct sNASServingSystemIndication_ProtocolRevision
{
   eQMINASRevision mProtocolRevision;
};

// Structure to describe indication TLV 0x19 for NAS ServingSystemIndication
struct sNASServingSystemIndication_PLMNChange
{
   INT8 mPLMNChanged;
};

// Structure to describe indication TLV 0x1A for NAS ServingSystemIndication
struct sNASServingSystemIndication_3GPPTimeZone
{
   INT8 m3GPPTimeZone;
};

// Structure to describe indication TLV 0x1B for NAS ServingSystemIndication
struct sNASServingSystemIndication_3GPPDaylightSavingAdjustment
{
   eQMINASDaylightSavingsAdjustment mDaylightSavingsAdjustment;
};

// Structure to describe indication TLV 0x1C for NAS ServingSystemIndication
struct sNASServingSystemIndication_3GPPUniversalTimeAndZone
{
   UINT16 mYear;
   UINT8 mMonth;
   UINT8 mDay;
   UINT8 mHour;
   UINT8 mMinute;
   UINT8 mSecond;
   INT8 mTimeZoneOffset;
};

// Structure to describe indication TLV 0x1D for NAS ServingSystemIndication
struct sNASServingSystemIndication_3GPPLocationAreaCode
{
   UINT16 mLocationAreaCode;
};

// Structure to describe indication TLV 0x1E for NAS ServingSystemIndication
struct sNASServingSystemIndication_3GPPCellID
{
   UINT32 mCellID;
};

// Structure to describe indication TLV 0x1F for NAS ServingSystemIndication
struct sNASServingSystemIndication_3GPP2ConcurrentService
{
   eQMINASConcurrentService mConcurrentService;
};

// Structure to describe indication TLV 0x20 for NAS ServingSystemIndication
struct sNASServingSystemIndication_3GPP2PRLIndicator
{
   eQMINASPRLIndicator mPRLIndicator;
};

// Structure to describe indication TLV 0x21 for NAS ServingSystemIndication
struct sNASServingSystemIndication_DualTransferModeIndication
{
   eQMINASDualTransferMode mDualTransferMode;
};

// Structure to describe indication TLV 0x22 for NAS ServingSystemIndication
struct sNASServingSystemIndication_DetailedServiceInformation
{
   eQMINASServiceStatus mServiceStatus;
   eQMINASSystemServiceCapabilities mSystemServiceCapabilities;
   eQMINASServiceStatus mCDMA1xEVDOServiceStatus;
   eQMINASCDMA1xEVDOHybridInformation mCDMA1xEVDOHybridInformation;
   eQMINASSystemForbidden mSystemForbidden;
};

// Structure to describe indication TLV 0x23 for NAS ServingSystemIndication
struct sNASServingSystemIndication_CDMASystemInformation
{
   UINT16 mMobileCountryCode;
   UINT8 mIMSI_11_12;
};

// Structure to describe indication TLV 0x24 for NAS ServingSystemIndication
struct sNASServingSystemIndication_CDMA1xEVDOPersonality
{
   eQMINASCDMA1xEVDOPersonality mCDMA1xEVDOPersonality;
};

// Structure to describe indication TLV 0x25 for NAS ServingSystemIndication
struct sNASServingSystemIndication_TrackingAreaCode
{
   UINT16 mTrackingAreaCode;
};

// Structure to describe indication TLV 0x26 for NAS ServingSystemIndication
struct sNASServingSystemIndication_CallBarring
{
   eQMINASCallBarringStatus mCSCallBarringStatus;
   eQMINASCallBarringStatus mPSCallBarringStatus;
};

// Structure to describe indication TLV 0x27 for NAS ServingSystemIndication
struct sNASServingSystemIndication_PLMNChangeStatus
{
   INT8 mNoPLMNChange;
};

// Structure to describe indication TLV 0x28 for NAS ServingSystemIndication
struct sNASServingSystemIndication_UMTSPSC
{
   UINT16 mPrimaryScramblingCode;
};

// Structure to describe indication TLV 0x29 for NAS ServingSystemIndication
struct sNASServingSystemIndication_PCSInfo
{
   UINT16 mMobileCountryCode;
   UINT16 mMobileNetworkCode;
   INT8 mMNCIncludesPCSDigit;
};

// Structure to describe indication TLV 0x2A for NAS ServingSystemIndication
struct sNASServingSystemIndication_HSCallStatus
{
   eQMINASHighSpeedCallStatus mHighSpeedCallStatus;
};

// Structure to describe response TLV 0x01 for NASGetHomeNetwork()
struct sNASGetHomeNetworkResponse_HomeNetwork
{
   UINT16 mMobileCountryCode;
   UINT16 mMobileNetworkCode;
   UINT8 mDescriptionLength;

   // This array must be the size specified by mDescriptionLength
   // char mDescription[1];
};

// Structure to describe response TLV 0x10 for NASGetHomeNetwork()
struct sNASGetHomeNetworkResponse_HomeIDs
{
   UINT16 mSystemID;
   UINT16 mNetworkID;
};

// Structure to describe response TLV 0x11 for NASGetHomeNetwork()
struct sNASGetHomeNetworkResponse_ExtendedHomeNetwork
{
   UINT16 mMobileCountryCode;
   UINT16 mMobileNetworkCode;
   eQMINASNetworkDescriptionDisplays mDisplayNetworkDescription;
   eQMINASNetworkDescriptionEncodings mNetworkDescriptionEncoding;
   UINT8 mNetworkDescriptionLength;

   // This array must be the size specified by mNetworkDescriptionLength
   // UINT8 mNetworkDescription[1];
};

// Structure to describe response TLV 0x10 for NASGetPreferredNetworks()
struct sNASGetPreferredNetworksResponse_Networks
{
   UINT16 mNumberOfPreferredNetworks;

   struct sNetwork
   {
      UINT16 mMobileCountryCode;
      UINT16 mMobileNetworkCode;
   
      // Padding out 6 bits
      UINT8 mReserved1:6;
   
      bool mGSMCompact:1;
      bool mGSM:1;
   
      // Padding out 6 bits
      UINT8 mReserved2:6;
   
      bool mLTE:1;
      bool mUMTS:1;
   };

   // This array must be the size specified by mNumberOfPreferredNetworks
   // sNetwork mNetworks[1];
};

// Structure to describe response TLV 0x11 for NASGetPreferredNetworks()
struct sNASGetPreferredNetworksResponse_StaticNetworks
{
   UINT16 mNumberOfPreferredNetworks;

   struct sNetwork
   {
      UINT16 mMobileCountryCode;
      UINT16 mMobileNetworkCode;
   
      // Padding out 6 bits
      UINT8 mReserved1:6;
   
      bool mGSMCompact:1;
      bool mGSM:1;
   
      // Padding out 6 bits
      UINT8 mReserved2:6;
   
      bool mLTE:1;
      bool mUMTS:1;
   };

   // This array must be the size specified by mNumberOfPreferredNetworks
   // sNetwork mNetworks[1];
};

// Structure to describe request TLV 0x10 for NASSetPreferredNetworks()
struct sNASSetPreferredNetworksRequest_Networks
{
   UINT16 mNumberOfPreferredNetworks;

   struct sNetwork
   {
      UINT16 mMobileCountryCode;
      UINT16 mMobileNetworkCode;
   
      // Padding out 6 bits
      UINT8 mReserved1:6;
   
      bool mGSMCompact:1;
      bool mGSM:1;
   
      // Padding out 6 bits
      UINT8 mReserved2:6;
   
      bool mLTE:1;
      bool mUMTS:1;
   };

   // This array must be the size specified by mNumberOfPreferredNetworks
   // sNetwork mNetworks[1];
};

// Structure to describe response TLV 0x10 for NASGetForbiddenNetworks()
struct sNASGetForbiddenNetworksResponse_Networks
{
   UINT16 mNumberOfForbiddenNetworks;

   struct sNetwork
   {
      UINT16 mMobileCountryCode;
      UINT16 mMobileNetworkCode;
   };

   // This array must be the size specified by mNumberOfForbiddenNetworks
   // sNetwork mNetworks[1];
};

// Structure to describe request TLV 0x10 for NASSetForbiddenNetworks()
struct sNASSetForbiddenNetworksRequest_Networks
{
   UINT16 mNumberOfForbiddenNetworks;

   struct sNetwork
   {
      UINT16 mMobileCountryCode;
      UINT16 mMobileNetworkCode;
   };

   // This array must be the size specified by mNumberOfForbiddenNetworks
   // sNetwork mNetworks[1];
};

// Structure to describe request TLV 0x01 for NASSetTechnologyPreference()
struct sNASSetTechnologyPreferenceRequest_Preference
{
   // mTechnology must be the first two bits of a UINT8
   // whose remaining bits are described in the optional
   // structs following.

   // The following union is based on the value of mTechnology
   union uValOfTechnology
   {
      // Always present
      eQMINASTechPrefs mTechnology:2;

      // If the value of mTechnology == 1
      struct sTechnologyIs1
      {
         // Padding out 2 bits
         UINT8 mReserved1:2;
      
         UINT8 mAnalog:1;
         UINT8 mDigital:1;
         UINT8 mEVDO:1;
         UINT8 mLTE:1;
      
         // Padding out 2 bits
         UINT8 mReserved2:2;
      };

      sTechnologyIs1 mTechnologyIs1;

      // If the value of mTechnology == 2
      struct sTechnologyIs2
      {
         // Padding out 2 bits
         UINT8 mReserved3:2;
      
         UINT8 mGSM:1;
         UINT8 mWCDMA:1;
         UINT8 mEVDO:1;
         UINT8 mLTE:1;
      
         // Padding out 2 bits
         UINT8 mReserved4:2;
      };

      sTechnologyIs2 mTechnologyIs2;

      // Padding out 8 bits
      UINT8 mReserved5;
   };

   uValOfTechnology mValOfTechnology;

   // Padding out 8 bits
   UINT8 mReserved6;

   eQMINASTechPrefDurations mDuration;
};

// Structure to describe response TLV 0x01 for NASGetTechnologyPreference()
struct sNASGetTechnologyPreferenceResponse_ActivePreference
{
   // mTechnology must be the first two bits of a UINT8
   // whose remaining bits are described in the optional
   // structs following.

   // The following union is based on the value of mTechnology
   union uValOfTechnology
   {
      // Always present
      eQMINASTechPrefs mTechnology:2;

      // If the value of mTechnology == 1
      struct sTechnologyIs1
      {
         // Padding out 2 bits
         UINT8 mReserved1:2;
      
         UINT8 mAnalog:1;
         UINT8 mDigital:1;
         UINT8 mEVDO:1;
         UINT8 mLTE:1;
      
         // Padding out 2 bits
         UINT8 mReserved2:2;
      };

      sTechnologyIs1 mTechnologyIs1;

      // If the value of mTechnology == 2
      struct sTechnologyIs2
      {
         // Padding out 2 bits
         UINT8 mReserved3:2;
      
         UINT8 mGSM:1;
         UINT8 mWCDMA:1;
         UINT8 mEVDO:1;
         UINT8 mLTE:1;
      
         // Padding out 2 bits
         UINT8 mReserved4:2;
      };

      sTechnologyIs2 mTechnologyIs2;

      // Padding out 8 bits
      UINT8 mReserved5;
   };

   uValOfTechnology mValOfTechnology;

   // Padding out 8 bits
   UINT8 mReserved6;

   eQMINASTechPrefDurations mDuration;
};

// Structure to describe response TLV 0x10 for NASGetTechnologyPreference()
struct sNASGetTechnologyPreferenceResponse_PersistentPreference
{
   // mTechnology must be the first two bits of a UINT8
   // whose remaining bits are described in the optional
   // structs following.

   // The following union is based on the value of mTechnology
   union uValOfTechnology
   {
      // Always present
      eQMINASTechPrefs mTechnology:2;

      // If the value of mTechnology == 1
      struct sTechnologyIs1
      {
         // Padding out 2 bits
         UINT8 mReserved1:2;
      
         UINT8 mAnalog:1;
         UINT8 mDigital:1;
         UINT8 mEVDO:1;
         UINT8 mLTE:1;
      
         // Padding out 2 bits
         UINT8 mReserved2:2;
      };

      sTechnologyIs1 mTechnologyIs1;

      // If the value of mTechnology == 2
      struct sTechnologyIs2
      {
         // Padding out 2 bits
         UINT8 mReserved3:2;
      
         UINT8 mGSM:1;
         UINT8 mWCDMA:1;
         UINT8 mEVDO:1;
         UINT8 mLTE:1;
      
         // Padding out 2 bits
         UINT8 mReserved4:2;
      };

      sTechnologyIs2 mTechnologyIs2;

      // Padding out 8 bits
      UINT8 mReserved5;
   };

   uValOfTechnology mValOfTechnology;

   // Padding out 8 bits
   UINT8 mReserved6;
};

// Structure to describe response TLV 0x01 for NASGetACCOLC()
struct sNASGetACCOLCResponse_ACCOLC
{
   UINT8 mACCOLC;
};

// Structure to describe request TLV 0x01 for NASSetACCOLC()
struct sNASSetACCOLCRequest_ACCOLC
{
   char mSPC[6];
   UINT8 mACCOLC;
};

// Structure to describe indication TLV 0x01 for NAS GetSystemPreference
struct sNASGetSystemPreferenceIndication_Pref
{
   eQMINASSystemPreferences mSystemPreference;
};

// Structure to describe response TLV 0x11 for NASGetNetworkParameters()
struct sNASGetNetworkParametersResponse_SCI
{
   UINT8 mSlotCycleIndex;
};

// Structure to describe response TLV 0x12 for NASGetNetworkParameters()
struct sNASGetNetworkParametersResponse_SCM
{
   UINT8 mStationClassMark;
};

// Structure to describe response TLV 0x13 for NASGetNetworkParameters()
struct sNASGetNetworkParametersResponse_Registration
{
   INT8 mRegisterOnHomeSystem;
   INT8 mRegisterOnForeignSystem;
   INT8 mRegisterOnForeignNetwork;
};

// Structure to describe response TLV 0x14 for NASGetNetworkParameters()
struct sNASGetNetworkParametersResponse_CDMA1xEVDORevision
{
   INT8 mForceCDMA1xEVDORev0;
};

// Structure to describe response TLV 0x15 for NASGetNetworkParameters()
struct sNASGetNetworkParametersResponse_CDMA1xEVDOSCPCustom
{
   INT8 mCDMA1xEVDOSCPCustomConfig;
   bool mSubtype2PhysicalLayer:1;
   bool mEnhancedCCMAC:1;
   bool mEnhancedACMAC:1;
   bool mEnhancedFTCMAC:1;
   bool mSubtype3RTCMAC:1;
   bool mSubtype1RTCMAC:1;
   bool mEnhancedIdle:1;
   bool mGenericMultimodeCapableDiscPort:1;

   // Padding out 24 bits
   UINT8 mReserved1[3];

   bool mGenericBroadcast:1;

   // Padding out 31 bits
   UINT8 mReserved2:7;
   UINT8 mReserved3[3];

   bool mSNMultiflowPacketApplication:1;
   bool mSNEnhancedMultiflowPacketApplication:1;

   // Padding out 30 bits
   UINT8 mReserved4:6;
   UINT8 mReserved5[3];
};

// Structure to describe response TLV 0x16 for NASGetNetworkParameters()
struct sNASGetNetworkParametersResponse_Roaming
{
   eQMINASRoamingPreferences mRoamPreference;
};

// Structure to describe response TLV 0x17 for NASGetNetworkParameters()
struct sNASGetNetworkParametersResponse_ForceCDMA1xEVDOSCP
{
   eQMINASForceCDMA1xEVDOSCP mForceCDMA1xEVDOSCP;
};

// Structure to describe request TLV 0x10 for NASSetNetworkParameters()
struct sNASSetNetworkParametersRequest_SPC
{
   char mSPC[6];
};

// Structure to describe request TLV 0x14 for NASSetNetworkParameters()
struct sNASSetNetworkParametersRequest_CDMA1xEVDORevision
{
   INT8 mForceCDMA1xEVDORev0;
};

// Structure to describe request TLV 0x15 for NASSetNetworkParameters()
struct sNASSetNetworkParametersRequest_CDMA1xEVDOSCPCustom
{
   INT8 mCDMA1xEVDOSCPCustomConfig;
   bool mSubtype2PhysicalLayer:1;
   bool mEnhancedCCMAC:1;
   bool mEnhancedACMAC:1;
   bool mEnhancedFTCMAC:1;
   bool mSubtype3RTCMAC:1;
   bool mSubtype1RTCMAC:1;
   bool mEnhancedIdle:1;
   bool mGenericMultimodeCapableDiscPort:1;

   // Padding out 24 bits
   UINT8 mReserved1[3];

   bool mGenericBroadcast:1;

   // Padding out 31 bits
   UINT8 mReserved2:7;
   UINT8 mReserved3[3];

   bool mSNMultiflowPacketApplication:1;
   bool mSNEnhancedMultiflowPacketApplication:1;

   // Padding out 30 bits
   UINT8 mReserved4:6;
   UINT8 mReserved5[3];
};

// Structure to describe request TLV 0x16 for NASSetNetworkParameters()
struct sNASSetNetworkParametersRequest_Roaming
{
   eQMINASRoamingPreferences mRoamPreference;
};

// Structure to describe response TLV 0x01 for NASGetRFInfo()
struct sNASGetRFInfoResponse_RFInfo
{
   UINT8 mNumberOfInstances;

   struct sInstance
   {
      eQMINASRadioInterfaces mRadioInterface;
      eQMINASBandClasses mActiveBandClass;
      UINT16 mActiveChannel;
   };

   // This array must be the size specified by mNumberOfInstances
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x01 for NASGetANAAAAuthenticationStatus()
struct sNASGetANAAAAuthenticationStatusResponse_Status
{
   eQMINASANAAAAuthenticationStatus mANAAAAuthenticationStatus;
};

// Structure to describe request TLV 0x10 for NASSetSystemSelectionPref()
struct sNASSetSystemSelectionPrefRequest_EmergencyMode
{
   INT8 mEmergencyModeOn;
};

// Structure to describe request TLV 0x11 for NASSetSystemSelectionPref()
struct sNASSetSystemSelectionPrefRequest_Mode
{
   bool mCDMA1x:1;
   bool mCDMA1xEVDO:1;
   bool mGSM:1;
   bool mUMTS:1;
   bool mLTE:1;
   bool mTDSCDMA:1;

   // Padding out 10 bits
   UINT8 mReserved1:2;
   UINT8 mReserved2;
};

// Structure to describe request TLV 0x12 for NASSetSystemSelectionPref()
struct sNASSetSystemSelectionPrefRequest_Band
{
   bool mBandClass0ASystem:1;
   bool mBandClass0BSystem:1;
   bool mBandClass1:1;
   bool mBandClass2:1;
   bool mBandClass3ASystem:1;
   bool mBandClass4:1;
   bool mBandClass5:1;
   bool mGSMDCS:1;
   bool mGSMPrimary:1;
   bool mGSMExtended:1;
   bool mBandClass6:1;
   bool mBandClass7:1;
   bool mBandClass8:1;
   bool mBandClass9:1;
   bool mBandClass10:1;
   bool mBandClass11:1;
   bool mGSM450:1;
   bool mGSM480:1;
   bool mGSM750:1;
   bool mGSM850:1;
   bool mGSMRailways:1;
   bool mGSMPCS:1;
   bool mWCDMA2100I:1;
   bool mWCDMAPCS1900:1;
   bool mWCDMADCS1800:1;
   bool mWCDMA1700US:1;
   bool mWCDMA850:1;
   bool mWCDMA800:1;
   bool mBandClass12:1;
   bool mBandClass14:1;

   // Padding out 1 bits
   UINT8 mReserved1:1;

   bool mBandClass15:1;

   // Padding out 16 bits
   UINT8 mReserved2[2];

   bool mWCDMA2600:1;
   bool mWCDMA900:1;
   bool mWCDMA1700Japan:1;

   // Padding out 5 bits
   UINT8 mReserved3:5;

   bool mBandClass16:1;
   bool mBandClass17:1;
   bool mBandClass18:1;
   bool mBandClass19:1;

   // Padding out 4 bits
   UINT8 mReserved4:4;
};

// Structure to describe request TLV 0x13 for NASSetSystemSelectionPref()
struct sNASSetSystemSelectionPrefRequest_PRL
{
   eQMINASPRLPreferences mPRLPreference;
};

// Structure to describe request TLV 0x14 for NASSetSystemSelectionPref()
struct sNASSetSystemSelectionPrefRequest_Roaming
{
   eQMINASRoamingPreferences2 mRoamingPreference;
};

// Structure to describe request TLV 0x15 for NASSetSystemSelectionPref()
struct sNASSetSystemSelectionPrefRequest_LTEBand
{
   bool mEUTRABand1:1;
   bool mEUTRABand2:1;
   bool mEUTRABand3:1;
   bool mEUTRABand4:1;
   bool mEUTRABand5:1;
   bool mEUTRABand6:1;
   bool mEUTRABand7:1;
   bool mEUTRABand8:1;
   bool mEUTRABand9:1;
   bool mEUTRABand10:1;
   bool mEUTRABand11:1;
   bool mEUTRABand12:1;
   bool mEUTRABand13:1;
   bool mEUTRABand14:1;

   // Padding out 2 bits
   UINT8 mReserved1:2;

   bool mEUTRABand17:1;
   bool mEUTRABand18:1;
   bool mEUTRABand19:1;
   bool mEUTRABand20:1;
   bool mEUTRABand21:1;

   // Padding out 2 bits
   UINT8 mReserved2:2;

   bool mEUTRABand24:1;
   bool mEUTRABand25:1;

   // Padding out 7 bits
   UINT8 mReserved3:7;

   bool mEUTRABand33:1;
   bool mEUTRABand34:1;
   bool mEUTRABand35:1;
   bool mEUTRABand36:1;
   bool mEUTRABand37:1;
   bool mEUTRABand38:1;
   bool mEUTRABand39:1;
   bool mEUTRABand40:1;
   bool mEUTRABand41:1;
   bool mEUTRABand42:1;
   bool mEUTRABand43:1;

   // Padding out 21 bits
   UINT8 mReserved4:5;
   UINT8 mReserved5[2];
};

// Structure to describe request TLV 0x16 for NASSetSystemSelectionPref()
struct sNASSetSystemSelectionPrefRequest_NetworkSelection
{
   eQMINASNetworkSelection mNetworkSelection;
   UINT16 mMobileCountryCode;
   UINT16 mMobileNetworkCode;
};

// Structure to describe request TLV 0x17 for NASSetSystemSelectionPref()
struct sNASSetSystemSelectionPrefRequest_ChangeDuration
{
   eQMINASChangeDuration mChangeDuration;
};

// Structure to describe request TLV 0x18 for NASSetSystemSelectionPref()
struct sNASSetSystemSelectionPrefRequest_ServiceDomain
{
   eQMINASServiceDomainPrefs mServiceDomainPreference;
};

// Structure to describe request TLV 0x19 for NASSetSystemSelectionPref()
struct sNASSetSystemSelectionPrefRequest_GWAcquisitionOrder
{
   eQMINASAcquisitionOrder mAcquisitionOrderPreference;
};

// Structure to describe request TLV 0x1A for NASSetSystemSelectionPref()
struct sNASSetSystemSelectionPrefRequest_PCSInfo
{
   INT8 mMNCIncludesPCSDigit;
};

// Structure to describe request TLV 0x1B for NASSetSystemSelectionPref()
struct sNASSetSystemSelectionPrefRequest_Domain
{
   eQMINASServiceDomainPrefs mServiceDomainPreference;
};

// Structure to describe request TLV 0x1C for NASSetSystemSelectionPref()
struct sNASSetSystemSelectionPrefRequest_Acquisition
{
   eQMINASAcquisitionOrder mAcquisitionOrderPreference;
};

// Structure to describe request TLV 0x1D for NASSetSystemSelectionPref()
struct sNASSetSystemSelectionPrefRequest_TDSCDMABand
{
   bool mTDSCDMABandA:1;
   bool mTDSCDMABandB:1;
   bool mTDSCDMABandC:1;
   bool mTDSCDMABandD:1;
   bool mTDSCDMABandE:1;
   bool mTDSCDMABandF:1;

   // Padding out 58 bits
   UINT8 mReserved1:2;
   UINT8 mReserved2[7];
};

// Structure to describe request TLV 0x1E for NASSetSystemSelectionPref()
struct sNASSetSystemSelectionPrefRequest_AcquisitionOrder
{
   UINT8 mNumberOfRadioInterfaces;

   // This array must be the size specified by mNumberOfRadioInterfaces
   // eQMINASRadioInterfaces mRadioInterface[1];
};

// Structure to describe request TLV 0x1F for NASSetSystemSelectionPref()
struct sNASSetSystemSelectionPrefRequest_RegistrationRestriction
{
   eQMINASRegistrationRestrictions mRegistrationRestriction;
};

// Structure to describe response TLV 0x10 for NASGetSystemSelectionPref()
struct sNASGetSystemSelectionPrefResponse_EmergencyMode
{
   INT8 mEmergencyModeOn;
};

// Structure to describe response TLV 0x11 for NASGetSystemSelectionPref()
struct sNASGetSystemSelectionPrefResponse_Mode
{
   bool mCDMA1x:1;
   bool mCDMA1xEVDO:1;
   bool mGSM:1;
   bool mUMTS:1;
   bool mLTE:1;
   bool mTDSCDMA:1;

   // Padding out 10 bits
   UINT8 mReserved1:2;
   UINT8 mReserved2;
};

// Structure to describe response TLV 0x12 for NASGetSystemSelectionPref()
struct sNASGetSystemSelectionPrefResponse_Band
{
   bool mBandClass0ASystem:1;
   bool mBandClass0BSystem:1;
   bool mBandClass1:1;
   bool mBandClass2:1;
   bool mBandClass3ASystem:1;
   bool mBandClass4:1;
   bool mBandClass5:1;
   bool mGSMDCS:1;
   bool mGSMPrimary:1;
   bool mGSMExtended:1;
   bool mBandClass6:1;
   bool mBandClass7:1;
   bool mBandClass8:1;
   bool mBandClass9:1;
   bool mBandClass10:1;
   bool mBandClass11:1;
   bool mGSM450:1;
   bool mGSM480:1;
   bool mGSM750:1;
   bool mGSM850:1;
   bool mGSMRailways:1;
   bool mGSMPCS:1;
   bool mWCDMA2100I:1;
   bool mWCDMAPCS1900:1;
   bool mWCDMADCS1800:1;
   bool mWCDMA1700US:1;
   bool mWCDMA850:1;
   bool mWCDMA800:1;
   bool mBandClass12:1;
   bool mBandClass14:1;

   // Padding out 1 bits
   UINT8 mReserved1:1;

   bool mBandClass15:1;

   // Padding out 16 bits
   UINT8 mReserved2[2];

   bool mWCDMA2600:1;
   bool mWCDMA900:1;
   bool mWCDMA1700Japan:1;

   // Padding out 5 bits
   UINT8 mReserved3:5;

   bool mBandClass16:1;
   bool mBandClass17:1;
   bool mBandClass18:1;
   bool mBandClass19:1;

   // Padding out 4 bits
   UINT8 mReserved4:4;
};

// Structure to describe response TLV 0x13 for NASGetSystemSelectionPref()
struct sNASGetSystemSelectionPrefResponse_PRL
{
   eQMINASPRLPreferences mPRLPreference;
};

// Structure to describe response TLV 0x14 for NASGetSystemSelectionPref()
struct sNASGetSystemSelectionPrefResponse_Roaming
{
   eQMINASRoamingPreferences2 mRoamingPreference;
};

// Structure to describe response TLV 0x15 for NASGetSystemSelectionPref()
struct sNASGetSystemSelectionPrefResponse_LTEBand
{
   bool mEUTRABand1:1;
   bool mEUTRABand2:1;
   bool mEUTRABand3:1;
   bool mEUTRABand4:1;
   bool mEUTRABand5:1;
   bool mEUTRABand6:1;
   bool mEUTRABand7:1;
   bool mEUTRABand8:1;
   bool mEUTRABand9:1;
   bool mEUTRABand10:1;
   bool mEUTRABand11:1;
   bool mEUTRABand12:1;
   bool mEUTRABand13:1;
   bool mEUTRABand14:1;

   // Padding out 2 bits
   UINT8 mReserved1:2;

   bool mEUTRABand17:1;
   bool mEUTRABand18:1;
   bool mEUTRABand19:1;
   bool mEUTRABand20:1;
   bool mEUTRABand21:1;

   // Padding out 2 bits
   UINT8 mReserved2:2;

   bool mEUTRABand24:1;
   bool mEUTRABand25:1;

   // Padding out 7 bits
   UINT8 mReserved3:7;

   bool mEUTRABand33:1;
   bool mEUTRABand34:1;
   bool mEUTRABand35:1;
   bool mEUTRABand36:1;
   bool mEUTRABand37:1;
   bool mEUTRABand38:1;
   bool mEUTRABand39:1;
   bool mEUTRABand40:1;
   bool mEUTRABand41:1;
   bool mEUTRABand42:1;
   bool mEUTRABand43:1;

   // Padding out 21 bits
   UINT8 mReserved4:5;
   UINT8 mReserved5[2];
};

// Structure to describe response TLV 0x16 for NASGetSystemSelectionPref()
struct sNASGetSystemSelectionPrefResponse_NetworkSelection
{
   eQMINASNetworkSelection mNetworkSelection;
};

// Structure to describe response TLV 0x18 for NASGetSystemSelectionPref()
struct sNASGetSystemSelectionPrefResponse_Domain
{
   eQMINASServiceDomainPrefs mServiceDomainPreference;
};

// Structure to describe response TLV 0x19 for NASGetSystemSelectionPref()
struct sNASGetSystemSelectionPrefResponse_Acquisition
{
   eQMINASAcquisitionOrder mAcquisitionOrderPreference;
};

// Structure to describe response TLV 0x1A for NASGetSystemSelectionPref()
struct sNASGetSystemSelectionPrefResponse_TDSCDMABand
{
   bool mTDSCDMABandA:1;
   bool mTDSCDMABandB:1;
   bool mTDSCDMABandC:1;
   bool mTDSCDMABandD:1;
   bool mTDSCDMABandE:1;
   bool mTDSCDMABandF:1;

   // Padding out 58 bits
   UINT8 mReserved1:2;
   UINT8 mReserved2[7];
};

// Structure to describe response TLV 0x1B for NASGetSystemSelectionPref()
struct sNASGetSystemSelectionPrefResponse_ManualPLMN
{
   UINT16 mMobileCountryCode;
   UINT16 mMobileNetworkCode;
   INT8 mMNCIncludesPCSDigit;
};

// Structure to describe response TLV 0x1C for NASGetSystemSelectionPref()
struct sNASGetSystemSelectionPrefResponse_AcquisitionOrder
{
   UINT8 mNumberOfRadioInterfaces;

   // This array must be the size specified by mNumberOfRadioInterfaces
   // eQMINASRadioInterfaces mRadioInterface[1];
};

// Structure to describe response TLV 0x1D for NASGetSystemSelectionPref()
struct sNASGetSystemSelectionPrefResponse_RegistrationRestriction
{
   eQMINASRegistrationRestrictions mRegistrationRestriction;
};

// Structure to describe indication TLV 0x10 for NAS SystemSelectionPrefIndication
struct sNASSystemSelectionPrefIndication_EmergencyMode
{
   INT8 mEmergencyModeOn;
};

// Structure to describe indication TLV 0x11 for NAS SystemSelectionPrefIndication
struct sNASSystemSelectionPrefIndication_Mode
{
   bool mCDMA1x:1;
   bool mCDMA1xEVDO:1;
   bool mGSM:1;
   bool mUMTS:1;
   bool mLTE:1;
   bool mTDSCDMA:1;

   // Padding out 10 bits
   UINT8 mReserved1:2;
   UINT8 mReserved2;
};

// Structure to describe indication TLV 0x12 for NAS SystemSelectionPrefIndication
struct sNASSystemSelectionPrefIndication_Band
{
   bool mBandClass0ASystem:1;
   bool mBandClass0BSystem:1;
   bool mBandClass1:1;
   bool mBandClass2:1;
   bool mBandClass3ASystem:1;
   bool mBandClass4:1;
   bool mBandClass5:1;
   bool mGSMDCS:1;
   bool mGSMPrimary:1;
   bool mGSMExtended:1;
   bool mBandClass6:1;
   bool mBandClass7:1;
   bool mBandClass8:1;
   bool mBandClass9:1;
   bool mBandClass10:1;
   bool mBandClass11:1;
   bool mGSM450:1;
   bool mGSM480:1;
   bool mGSM750:1;
   bool mGSM850:1;
   bool mGSMRailways:1;
   bool mGSMPCS:1;
   bool mWCDMA2100I:1;
   bool mWCDMAPCS1900:1;
   bool mWCDMADCS1800:1;
   bool mWCDMA1700US:1;
   bool mWCDMA850:1;
   bool mWCDMA800:1;
   bool mBandClass12:1;
   bool mBandClass14:1;

   // Padding out 1 bits
   UINT8 mReserved1:1;

   bool mBandClass15:1;

   // Padding out 16 bits
   UINT8 mReserved2[2];

   bool mWCDMA2600:1;
   bool mWCDMA900:1;
   bool mWCDMA1700Japan:1;

   // Padding out 5 bits
   UINT8 mReserved3:5;

   bool mBandClass16:1;
   bool mBandClass17:1;
   bool mBandClass18:1;
   bool mBandClass19:1;

   // Padding out 4 bits
   UINT8 mReserved4:4;
};

// Structure to describe indication TLV 0x13 for NAS SystemSelectionPrefIndication
struct sNASSystemSelectionPrefIndication_PRL
{
   eQMINASPRLPreferences mPRLPreference;
};

// Structure to describe indication TLV 0x14 for NAS SystemSelectionPrefIndication
struct sNASSystemSelectionPrefIndication_Roaming
{
   eQMINASRoamingPreferences2 mRoamingPreference;
};

// Structure to describe indication TLV 0x15 for NAS SystemSelectionPrefIndication
struct sNASSystemSelectionPrefIndication_LTEBand
{
   bool mEUTRABand1:1;
   bool mEUTRABand2:1;
   bool mEUTRABand3:1;
   bool mEUTRABand4:1;
   bool mEUTRABand5:1;
   bool mEUTRABand6:1;
   bool mEUTRABand7:1;
   bool mEUTRABand8:1;
   bool mEUTRABand9:1;
   bool mEUTRABand10:1;
   bool mEUTRABand11:1;
   bool mEUTRABand12:1;
   bool mEUTRABand13:1;
   bool mEUTRABand14:1;

   // Padding out 2 bits
   UINT8 mReserved1:2;

   bool mEUTRABand17:1;
   bool mEUTRABand18:1;
   bool mEUTRABand19:1;
   bool mEUTRABand20:1;
   bool mEUTRABand21:1;

   // Padding out 2 bits
   UINT8 mReserved2:2;

   bool mEUTRABand24:1;
   bool mEUTRABand25:1;

   // Padding out 7 bits
   UINT8 mReserved3:7;

   bool mEUTRABand33:1;
   bool mEUTRABand34:1;
   bool mEUTRABand35:1;
   bool mEUTRABand36:1;
   bool mEUTRABand37:1;
   bool mEUTRABand38:1;
   bool mEUTRABand39:1;
   bool mEUTRABand40:1;
   bool mEUTRABand41:1;
   bool mEUTRABand42:1;
   bool mEUTRABand43:1;

   // Padding out 21 bits
   UINT8 mReserved4:5;
   UINT8 mReserved5[2];
};

// Structure to describe indication TLV 0x16 for NAS SystemSelectionPrefIndication
struct sNASSystemSelectionPrefIndication_NetworkSelection
{
   eQMINASNetworkSelection mNetworkSelection;
};

// Structure to describe indication TLV 0x18 for NAS SystemSelectionPrefIndication
struct sNASSystemSelectionPrefIndication_Domain
{
   eQMINASServiceDomainPrefs mServiceDomainPreference;
};

// Structure to describe indication TLV 0x19 for NAS SystemSelectionPrefIndication
struct sNASSystemSelectionPrefIndication_Acquisition
{
   eQMINASAcquisitionOrder mAcquisitionOrderPreference;
};

// Structure to describe indication TLV 0x1A for NAS SystemSelectionPrefIndication
struct sNASSystemSelectionPrefIndication_TDSCDMABand
{
   bool mTDSCDMABandA:1;
   bool mTDSCDMABandB:1;
   bool mTDSCDMABandC:1;
   bool mTDSCDMABandD:1;
   bool mTDSCDMABandE:1;
   bool mTDSCDMABandF:1;

   // Padding out 58 bits
   UINT8 mReserved1:2;
   UINT8 mReserved2[7];
};

// Structure to describe indication TLV 0x1B for NAS SystemSelectionPrefIndication
struct sNASSystemSelectionPrefIndication_ManualPLMN
{
   UINT16 mMobileCountryCode;
   UINT16 mMobileNetworkCode;
   INT8 mMNCIncludesPCSDigit;
};

// Structure to describe indication TLV 0x1C for NAS SystemSelectionPrefIndication
struct sNASSystemSelectionPrefIndication_AcquisitionOrder
{
   UINT8 mNumberOfRadioInterfaces;

   // This array must be the size specified by mNumberOfRadioInterfaces
   // eQMINASRadioInterfaces mRadioInterface[1];
};

// Structure to describe indication TLV 0x1D for NAS SystemSelectionPrefIndication
struct sNASSystemSelectionPrefIndication_RegistrationRestriction
{
   eQMINASRegistrationRestrictions mRegistrationRestriction;
};

// Structure to describe request TLV 0x01 for NASSetDDTMPreference()
struct sNASSetDDTMPreferenceRequest_DDTM
{
   eQMINASDDTMPreferences mDDTMPreference;
   bool mSuppressL2ACK:1;
   bool mSuppress1xRegistrations:1;
   bool mIgnoreServiceOptionPages:1;
   bool mBlockMobileOriginatedSMSAndDBM:1;

   // Padding out 12 bits
   UINT8 mReserved1:4;
   UINT8 mReserved2;

   eQMINASServiceOptionActions mServiceOptionAction;
   UINT8 mNumberOfInstances;

   // This array must be the size specified by mNumberOfInstances
   // UINT16 mServiceOption[1];
};

// Structure to describe response TLV 0x01 for NASGetDDTMPreference()
struct sNASGetDDTMPreferenceResponse_DDTM
{
   eQMINASDDTMPreferences mDDTMPreference;
   bool mSuppressL2ACK:1;
   bool mSuppress1xRegistrations:1;
   bool mIgnoreServiceOptionPages:1;
   bool mBlockMobileOriginatedSMSAndDBM:1;

   // Padding out 12 bits
   UINT8 mReserved1:4;
   UINT8 mReserved2;

   eQMINASServiceOptionActions mServiceOptionAction;
   UINT8 mNumberOfInstances;

   // This array must be the size specified by mNumberOfInstances
   // UINT16 mServiceOption[1];
};

// Structure to describe indication TLV 0x01 for NAS DDTMPreferenceIndication
struct sNASDDTMPreferenceIndication_DDTM
{
   eQMINASDDTMPreferences mDDTMPreference;
   bool mSuppressL2ACK:1;
   bool mSuppress1xRegistrations:1;
   bool mIgnoreServiceOptionPages:1;
   bool mBlockMobileOriginatedSMSAndDBM:1;

   // Padding out 12 bits
   UINT8 mReserved1:4;
   UINT8 mReserved2;

   eQMINASServiceOptionActions mServiceOptionAction;
   UINT8 mNumberOfInstances;

   // This array must be the size specified by mNumberOfInstances
   // UINT16 mServiceOption[1];
};

// Structure to describe response TLV 0x10 for NASGetOperatorNameData()
struct sNASGetOperatorNameDataResponse_ServiceProviderName
{
   UINT8 mDisplayCondition;
   UINT8 mSPNLength;

   // This array must be the size specified by mSPNLength
   // UINT8 mSPN[1];
};

// Structure to describe response TLV 0x11 for NASGetOperatorNameData()
struct sNASGetOperatorNameDataResponse_OperatorPLMNList
{
   UINT16 mPLMNListLength;

   struct sPLNM
   {
      char mMobileCountryCode[3];
      char mMobileNetworkCode[3];
      UINT16 mLocationAreaCode1;
      UINT16 mLocationAreaCode2;
      UINT8 mPLMNNameRecordIdentifier;
   };

   // This array must be the size specified by mPLMNListLength
   // sPLNM mPLNMs[1];
};

// Structure to describe response TLV 0x12 for NASGetOperatorNameData()
struct sNASGetOperatorNameDataResponse_PLMNName
{
   UINT8 mPLMNCount;
   
   struct sPLMNName1
   {
      eQMINASPLMNNameEncodingSchemes mPLMNShortEncoding;
      eQMINASPLMNNameCountryInitials mPLMNShortCountryInitials;
      eQMINASPLMNNameSpareBits mPLMNLongBits;
      eQMINASPLMNNameSpareBits mPLMNSpareBits;
      UINT8 mPLMNLongLength;
   
      // This array must be the size specified by mPLMNLongLength
      // UINT8 mPLMNLong[1];
   };
   
   struct sPLMNName2
   {
      UINT8 mPLMNShortLength;
   
      // This array must be the size specified by mPLMNShortLength
      // UINT8 mPLMNShort[1];
   };
   
   struct sPLMNName
   {
      sPLMNName1 mPLMNName1;
      sPLMNName2 mPLMNName2;
   };
   
   // This array must be the size specified by mPLMNCount
   // sPLMNName mPLMNNames[1];
};

// Structure to describe response TLV 0x13 for NASGetOperatorNameData()
struct sNASGetOperatorNameDataResponse_OperatorStringName
{
   // String is variable length, but must be size of the container
   // char mPLMNOperatorName[1];
};

// Structure to describe response TLV 0x14 for NASGetOperatorNameData()
struct sNASGetOperatorNameDataResponse_NITZInformation1
{
   eQMINASPLMNNameEncodingSchemes mPLMNShortEncoding;
   eQMINASPLMNNameCountryInitials mPLMNShortCountryInitials;
   eQMINASPLMNNameSpareBits mPLMNLongBits;
   eQMINASPLMNNameSpareBits mPLMNSpareBits;
   UINT8 mPLMNLongLength;

   // This array must be the size specified by mPLMNLongLength
   // UINT8 mPLMNLong[1];
};

struct sNASGetOperatorNameDataResponse_NITZInformation2
{
   UINT8 mPLMNShortLength;

   // This array must be the size specified by mPLMNShortLength
   // UINT8 mPLMNShort[1];
};

struct sNASGetOperatorNameDataResponse_NITZInformation
{
   sNASGetOperatorNameDataResponse_NITZInformation1 mNASGetOperatorNameDataResponse_NITZInformation1;
   sNASGetOperatorNameDataResponse_NITZInformation2 mNASGetOperatorNameDataResponse_NITZInformation2;
};

// Structure to describe indication TLV 0x10 for NAS OperatorNameDataIndication
struct sNASOperatorNameDataIndication_ServiceProviderName
{
   UINT8 mDisplayCondition;
   UINT8 mSPNLength;

   // This array must be the size specified by mSPNLength
   // UINT8 mSPN[1];
};

// Structure to describe indication TLV 0x11 for NAS OperatorNameDataIndication
struct sNASOperatorNameDataIndication_OperatorPLMNList
{
   UINT16 mPLMNListLength;

   struct sPLNM
   {
      char mMobileCountryCode[3];
      char mMobileNetworkCode[3];
      UINT16 mLocationAreaCode1;
      UINT16 mLocationAreaCode2;
      UINT8 mPLMNNameRecordIdentifier;
   };

   // This array must be the size specified by mPLMNListLength
   // sPLNM mPLNMs[1];
};

// Structure to describe indication TLV 0x12 for NAS OperatorNameDataIndication
struct sNASOperatorNameDataIndication_PLMNName
{
   UINT8 mPLMNCount;
   
   struct sPLMNName1
   {
      eQMINASPLMNNameEncodingSchemes mPLMNShortEncoding;
      eQMINASPLMNNameCountryInitials mPLMNShortCountryInitials;
      eQMINASPLMNNameSpareBits mPLMNLongBits;
      eQMINASPLMNNameSpareBits mPLMNSpareBits;
      UINT8 mPLMNLongLength;
   
      // This array must be the size specified by mPLMNLongLength
      // UINT8 mPLMNLong[1];
   };
   
   struct sPLMNName2
   {
      UINT8 mPLMNShortLength;
   
      // This array must be the size specified by mPLMNShortLength
      // UINT8 mPLMNShort[1];
   };
   
   struct sPLMNName
   {
      sPLMNName1 mPLMNName1;
      sPLMNName2 mPLMNName2;
   };
   
   // This array must be the size specified by mPLMNCount
   // sPLMNName mPLMNNames[1];
};

// Structure to describe indication TLV 0x13 for NAS OperatorNameDataIndication
struct sNASOperatorNameDataIndication_OperatorStringName
{
   // String is variable length, but must be size of the container
   // char mPLMNOperatorName[1];
};

// Structure to describe indication TLV 0x14 for NAS OperatorNameDataIndication
struct sNASOperatorNameDataIndication_NITZInformation1
{
   eQMINASPLMNNameEncodingSchemes mPLMNShortEncoding;
   eQMINASPLMNNameCountryInitials mPLMNShortCountryInitials;
   eQMINASPLMNNameSpareBits mPLMNLongBits;
   eQMINASPLMNNameSpareBits mPLMNSpareBits;
   UINT8 mPLMNLongLength;

   // This array must be the size specified by mPLMNLongLength
   // UINT8 mPLMNLong[1];
};

struct sNASOperatorNameDataIndication_NITZInformation2
{
   UINT8 mPLMNShortLength;

   // This array must be the size specified by mPLMNShortLength
   // UINT8 mPLMNShort[1];
};

struct sNASOperatorNameDataIndication_NITZInformation
{
   sNASOperatorNameDataIndication_NITZInformation1 mNASOperatorNameDataIndication_NITZInformation1;
   sNASOperatorNameDataIndication_NITZInformation2 mNASOperatorNameDataIndication_NITZInformation2;
};

// Structure to describe response TLV 0x10 for NASGetCSPPLMNMode()
struct sNASGetCSPPLMNModeResponse_Mode
{
   INT8 mRestrictManualPLMNSelection;
};

// Structure to describe indication TLV 0x10 for NAS CSPPLMNModeIndication
struct sNASCSPPLMNModeIndication_Mode
{
   INT8 mRestrictManualPLMNSelection;
};

// Structure to describe request TLV 0x01 for NASUpdateAKEY()
struct sNASUpdateAKEYRequest_AKEY
{
   char mAKEY[26];
};

// Structure to describe request TLV 0x01 for NASGet3GPP2SubscriptionInfo()
struct sNASGet3GPP2SubscriptionInfoRequest_NAMID
{
   UINT8 mNAMID;
};

// Structure to describe request TLV 0x10 for NASGet3GPP2SubscriptionInfo()
struct sNASGet3GPP2SubscriptionInfoRequest_InfoMask
{
   bool mNAMName:1;
   bool mDirectoryNumber:1;
   bool mHomeID:1;
   bool mMINBasedIMSI:1;
   bool mTrueIMSI:1;
   bool mCDMAChannel:1;
   bool mMobileDirectoryNumber:1;

   // Padding out 25 bits
   UINT8 mReserved1:1;
   UINT8 mReserved2[3];
};

// Structure to describe response TLV 0x10 for NASGet3GPP2SubscriptionInfo()
struct sNASGet3GPP2SubscriptionInfoResponse_NAMName
{
   UINT8 mNAMNameLength;

   // This array must be the size specified by mNAMNameLength
   // char mNAMName[1];
};

// Structure to describe response TLV 0x11 for NASGet3GPP2SubscriptionInfo()
struct sNASGet3GPP2SubscriptionInfoResponse_DirectoryNumber
{
   UINT8 mDirectoryNumberLength;

   // This array must be the size specified by mDirectoryNumberLength
   // char mDirectoryNumber[1];
};

// Structure to describe response TLV 0x12 for NASGet3GPP2SubscriptionInfo()
struct sNASGet3GPP2SubscriptionInfoResponse_HomeID
{
   UINT8 mHomeIDCount;

   struct sHomeID
   {
      UINT16 mSystemID;
      UINT16 mNetworkID;
   };

   // This array must be the size specified by mHomeIDCount
   // sHomeID mHomeIDs[1];
};

// Structure to describe response TLV 0x13 for NASGet3GPP2SubscriptionInfo()
struct sNASGet3GPP2SubscriptionInfoResponse_MINBasedIMSI
{
   char mMobileCountryCode[3];
   char mIMSI11_12[2];
   char mIMSIS1[7];
   char mIMSIS2[3];
   UINT8 mIMSIAddressNumber;
};

// Structure to describe response TLV 0x14 for NASGet3GPP2SubscriptionInfo()
struct sNASGet3GPP2SubscriptionInfoResponse_TrueIMSI
{
   char mMobileCountryCode[3];
   char mIMSI11_12[2];
   char mIMSIS1[7];
   char mIMSIS2[3];
   UINT8 mIMSIAddressNumber;
};

// Structure to describe response TLV 0x15 for NASGet3GPP2SubscriptionInfo()
struct sNASGet3GPP2SubscriptionInfoResponse_CDMAChannel
{
   UINT16 mAChannelForPrimaryCarrier;
   UINT16 mBChannelForPrimaryCarrier;
   UINT16 mAChannelForSecondaryCarrier;
   UINT16 mBChannelForSecondaryCarrier;
};

// Structure to describe response TLV 0x16 for NASGet3GPP2SubscriptionInfo()
struct sNASGet3GPP2SubscriptionInfoResponse_MDN
{
   UINT8 mMobileDirectoryNumberLength;

   // This array must be the size specified by mMobileDirectoryNumberLength
   // char mMobileDirectoryNumber[1];
};

// Structure to describe request TLV 0x01 for NASSet3GPP2SubscriptionInfo()
struct sNASSet3GPP2SubscriptionInfoRequest_NAMID
{
   UINT8 mNAMID;
};

// Structure to describe request TLV 0x10 for NASSet3GPP2SubscriptionInfo()
struct sNASSet3GPP2SubscriptionInfoRequest_DirectoryNumber
{
   UINT8 mDirectoryNumberLength;

   // This array must be the size specified by mDirectoryNumberLength
   // char mDirectoryNumber[1];
};

// Structure to describe request TLV 0x11 for NASSet3GPP2SubscriptionInfo()
struct sNASSet3GPP2SubscriptionInfoRequest_HomeID
{
   UINT8 mHomeIDCount;

   struct sHomeID
   {
      UINT16 mSystemID;
      UINT16 mNetworkID;
   };

   // This array must be the size specified by mHomeIDCount
   // sHomeID mHomeIDs[1];
};

// Structure to describe request TLV 0x12 for NASSet3GPP2SubscriptionInfo()
struct sNASSet3GPP2SubscriptionInfoRequest_MINBasedIMSI
{
   char mMobileCountryCode[3];
   char mIMSI11_12[2];
   char mIMSIS1[7];
   char mIMSIS2[3];
   UINT8 mIMSIAddressNumber;
};

// Structure to describe request TLV 0x13 for NASSet3GPP2SubscriptionInfo()
struct sNASSet3GPP2SubscriptionInfoRequest_TrueIMSI
{
   char mMobileCountryCode[3];
   char mIMSI11_12[2];
   char mIMSIS1[7];
   char mIMSIS2[3];
   UINT8 mIMSIAddressNumber;
};

// Structure to describe request TLV 0x14 for NASSet3GPP2SubscriptionInfo()
struct sNASSet3GPP2SubscriptionInfoRequest_CDMAChannel
{
   UINT16 mAChannelForPrimaryCarrier;
   UINT16 mBChannelForPrimaryCarrier;
   UINT16 mAChannelForSecondaryCarrier;
   UINT16 mBChannelForSecondaryCarrier;
};

// Structure to describe request TLV 0x15 for NASSet3GPP2SubscriptionInfo()
struct sNASSet3GPP2SubscriptionInfoRequest_NAMName
{
   UINT8 mNAMNameLength;

   // This array must be the size specified by mNAMNameLength
   // char mNAMName[1];
};

// Structure to describe request TLV 0x16 for NASSet3GPP2SubscriptionInfo()
struct sNASSet3GPP2SubscriptionInfoRequest_MDN
{
   UINT8 mMobileDirectoryNumberLength;

   // This array must be the size specified by mMobileDirectoryNumberLength
   // char mMobileDirectoryNumber[1];
};

// Structure to describe response TLV 0x10 for NASGetMobileCAIRevision()
struct sNASGetMobileCAIRevisionResponse_CAIRevision
{
   eQMINASRevision mCAIRevision;
};

// Structure to describe response TLV 0x10 for NASGetRTREConfig()
struct sNASGetRTREConfigResponse_CurrentRTREConfig
{
   eQMINASRTREConfiguration mRTREConfiguration;
};

// Structure to describe response TLV 0x11 for NASGetRTREConfig()
struct sNASGetRTREConfigResponse_RTREConfigPreference
{
   eQMINASRTREConfiguration mRTREConfiguration;
};

// Structure to describe request TLV 0x01 for NASSetRTREConfig()
struct sNASSetRTREConfigRequest_RTREConfig
{
   eQMINASRTREConfiguration mRTREConfiguration;
};

// Structure to describe response TLV 0x10 for NASGetCellLocationInfo()
struct sNASGetCellLocationInfoResponse_GERANInfo
{
   UINT32 mCellID;

   // Bitfield arrays are not possible in c, unrolling the array
   UINT8 mMobileCountryCode0:4;
   UINT8 mMobileCountryCode1:4;
   UINT8 mMobileCountryCode2:4;

   // Bitfield arrays are not possible in c, unrolling the array
   UINT8 mMobileNetworkCode0:4;
   UINT8 mMobileNetworkCode1:4;
   UINT8 mMobileNetworkCode2:4;

   UINT16 mLocationAreaCode;
   UINT16 mUTRAAbsoluteRFChannelNumber;
   UINT8 mBCC:3;
   UINT8 mNCC:3;

   // Padding out 2 bits
   UINT8 mReserved1:2;

   UINT32 mTimingAdvance;
   eQMINASRXLevel mRXLevel;
   UINT8 mNMRCellCount;

   struct sNMRCell
   {
      UINT32 mCellID;
   
      // Bitfield arrays are not possible in c, unrolling the array
      UINT8 mPLMNMobileCountryCode0:4;
      UINT8 mPLMNMobileCountryCode1:4;
      UINT8 mPLMNMobileCountryCode2:4;
   
      // Bitfield arrays are not possible in c, unrolling the array
      UINT8 mPLMNMobileNetworkCode0:4;
      UINT8 mPLMNMobileNetworkCode1:4;
      UINT8 mPLMNMobileNetworkCode2:4;
   
      UINT16 mLocationAreaCode;
      UINT16 mUTRAAbsoluteRFChannelNumber;
      UINT8 mBaseStationIdentityCodeBCC:3;
      UINT8 mBaseStationIdentityCodeNCC:3;
   
      // Padding out 2 bits
      UINT8 mReserved2:2;
   
      eQMINASRXLevel mRXLevel;
   };

   // This array must be the size specified by mNMRCellCount
   // sNMRCell mNMRCells[1];
};

// Structure to describe response TLV 0x11 for NASGetCellLocationInfo()
struct sNASGetCellLocationInfoResponse_UMTSInfo1
{
   UINT32 mCellID;

   // Bitfield arrays are not possible in c, unrolling the array
   UINT8 mMobileCountryCode0:4;
   UINT8 mMobileCountryCode1:4;
   UINT8 mMobileCountryCode2:4;

   // Bitfield arrays are not possible in c, unrolling the array
   UINT8 mMobileNetworkCode0:4;
   UINT8 mMobileNetworkCode1:4;
   UINT8 mMobileNetworkCode2:4;

   UINT16 mLocationAreaCode;
   UINT16 mUTRAAbsoluteRFChannelNumber;
   UINT16 mPrimaryScramblingCode;
   INT16 mReceivedSignalCodePower;
   UINT16 mECIO;
   UINT8 mUTRAUMTSMonitoredCellCount;

   struct sUMTSMonitoredCell
   {
      UINT16 mUTRAAbsoluteRFChannelNumber;
      UINT16 mPrimaryScramblingCode;
      INT16 mReceivedSignalCodePower;
      UINT16 mECIO;
   };

   // This array must be the size specified by mUTRAUMTSMonitoredCellCount
   // sUMTSMonitoredCell mUMTSMonitoredCells[1];
};

struct sNASGetCellLocationInfoResponse_UMTSInfo2
{
   UINT8 mGERANNBRCellCount;

   struct sGERANNBRCell
   {
      UINT16 mAbsoluteRFChannelNumber;
      UINT8 mNetworkColorCode;
      UINT8 mBaseStationColorCode;
      INT16 mRSSI;
   };

   // This array must be the size specified by mGERANNBRCellCount
   // sGERANNBRCell mGERANNBRCells[1];
};

struct sNASGetCellLocationInfoResponse_UMTSInfo
{
   sNASGetCellLocationInfoResponse_UMTSInfo1 mNASGetCellLocationInfoResponse_UMTSInfo1;
   sNASGetCellLocationInfoResponse_UMTSInfo2 mNASGetCellLocationInfoResponse_UMTSInfo2;
};

// Structure to describe response TLV 0x12 for NASGetCellLocationInfo()
struct sNASGetCellLocationInfoResponse_CDMAInfo
{
   UINT16 mSystemID;
   UINT16 mNetworkID;
   UINT16 mBaseStationID;
   UINT16 mReferencePN;
   INT32 mLatitude;
   INT32 mLongitude;
};

// Structure to describe response TLV 0x13 for NASGetCellLocationInfo()
struct sNASGetCellLocationInfoResponse_IntrafrequencyLTEInfo
{
   INT8 mUEInIdleMode;

   // Bitfield arrays are not possible in c, unrolling the array
   UINT8 mMobileCountryCode0:4;
   UINT8 mMobileCountryCode1:4;
   UINT8 mMobileCountryCode2:4;

   // Bitfield arrays are not possible in c, unrolling the array
   UINT8 mMobileNetworkCode0:4;
   UINT8 mMobileNetworkCode1:4;
   UINT8 mMobileNetworkCode2:4;

   UINT16 mTrackingAreaCode;
   UINT32 mGlobalCellID;
   UINT16 mEUTRAAbsoluteRFChannelNumber;
   UINT16 mServingCellID;
   UINT8 mCellReselectionPriority;
   UINT8 mSNonIntraSearchThreshold;
   UINT8 mServingCellLowThreshold;
   UINT8 mSIntraSearchThreshold;
   UINT8 mCellCount;

   struct sCell
   {
      UINT16 mPhysicalCellID;
      INT16 mRSRQ;
      INT16 mRSRP;
      INT16 mRSSI;
      INT16 mCellSelectionRXLevel;
   };

   // This array must be the size specified by mCellCount
   // sCell mCells[1];
};

// Structure to describe response TLV 0x14 for NASGetCellLocationInfo()
struct sNASGetCellLocationInfoResponse_InterfrequencyLTEInfo
{
   INT8 mUEInIdleMode;
   UINT8 mFrequencyCount;

   struct sFrequency
   {
      UINT16 mEUTRAAbsoluteRFChannelNumber;
      UINT8 mCellSelectionRXLevelLowThreshold;
      UINT8 mCellSelectionRXLevelHighThreshold;
      UINT8 mCellReselectionPriority;
      UINT8 mCellCount;
   
      struct sCell
      {
         UINT16 mPhysicalCellID;
         INT16 mRSRQ;
         INT16 mRSRP;
         INT16 mRSSI;
         INT16 mCellSelectionRXLevel;
      };
   
      // This array must be the size specified by mCellCount
      // sCell mCells[1];
   };

   // This array must be the size specified by mFrequencyCount
   // sFrequency mFrequencys[1];
};

// Structure to describe response TLV 0x15 for NASGetCellLocationInfo()
struct sNASGetCellLocationInfoResponse_LTEInfoNeighboringGSM
{
   INT8 mUEInIdleMode;
   UINT8 mFrequencyCount;

   struct sFrequency
   {
      UINT8 mCellReselectionPriority;
      UINT8 mCellReselectionHighThreshold;
      UINT8 mCellReselectionLowThreshold;
   
      // Bitfield arrays are not possible in c, unrolling the array
      bool mNCCPermitted0:1;
      bool mNCCPermitted1:1;
      bool mNCCPermitted2:1;
      bool mNCCPermitted3:1;
      bool mNCCPermitted4:1;
      bool mNCCPermitted5:1;
      bool mNCCPermitted6:1;
      bool mNCCPermitted7:1;
   
      UINT8 mCellCount;
   
      struct sCell
      {
         UINT16 mAbsoluteRFChannelNumber;
         INT8 mBandIs1900;
         INT8 mCellIDValid;
         UINT8 mBCC:3;
         UINT8 mNCC:3;
      
         // Padding out 2 bits
         UINT8 mReserved1:2;
      
         INT16 mRSSI;
         INT16 mCellSelectionRXLevel;
      };
   
      // This array must be the size specified by mCellCount
      // sCell mCells[1];
   };

   // This array must be the size specified by mFrequencyCount
   // sFrequency mFrequencys[1];
};

// Structure to describe response TLV 0x16 for NASGetCellLocationInfo()
struct sNASGetCellLocationInfoResponse_LTEInfoNeighboringWCDMA
{
   INT8 mUEInIdleMode;
   UINT8 mFrequencyCount;

   struct sFrequency
   {
      UINT16 mUTRAAbsoluteRFChannelNumber;
      UINT8 mCellReselectionPriority;
      UINT8 mCellReselectionHighThreshold;
      UINT8 mCellReselectionLowThreshold;
      UINT8 mCellCount;
   
      struct sCell
      {
         UINT16 mPrimaryScramblingCode;
         INT16 mCPICHRSCP;
         INT16 mCPICHEcNo;
         INT16 mCellSelectionRXLevel;
      };
   
      // This array must be the size specified by mCellCount
      // sCell mCells[1];
   };

   // This array must be the size specified by mFrequencyCount
   // sFrequency mFrequencys[1];
};

// Structure to describe response TLV 0x17 for NASGetCellLocationInfo()
struct sNASGetCellLocationInfoResponse_UMTSCellID
{
   UINT32 mCellID;
};

// Structure to describe request TLV 0x01 for NASGetPLMNName()
struct sNASGetPLMNNameRequest_PLMN
{
   UINT16 mMobileCountryCode;
   UINT16 mMobileNetworkCode;
};

// Structure to describe request TLV 0x10 for NASGetPLMNName()
struct sNASGetPLMNNameRequest_SupressSIMError
{
   INT8 mSIMInitNotChecked;
};

// Structure to describe request TLV 0x11 for NASGetPLMNName()
struct sNASGetPLMNNameRequest_MNCPCSDigitIncludeStatus
{
   INT8 mMNCIncludesPCSDigit;
};

// Structure to describe request TLV 0x12 for NASGetPLMNName()
struct sNASGetPLMNNameRequest_AlwaysSendPLMNName
{
   INT8 mAlwaysSendPLMNName;
};

// Structure to describe request TLV 0x13 for NASGetPLMNName()
struct sNASGetPLMNNameRequest_UseStaticTableOnly
{
   INT8 mUseStaticTableOnly;
};

// Structure to describe response TLV 0x10 for NASGetPLMNName()
struct sNASGetPLMNNameResponse_Name1
{
   eQMINASPLMNNameEncodingSchemes mSPNEncoding;
   UINT8 mSPNLength;

   // This array must be the size specified by mSPNLength
   // UINT8 mSPN[1];
};

struct sNASGetPLMNNameResponse_Name2
{
   eQMINASPLMNNameEncodingSchemes mPLMNShortEncoding;
   eQMINASPLMNNameCountryInitials mPLMNShortCountryInitials;
   eQMINASPLMNNameSpareBits mPLMNSpareBits;
   UINT8 mPLMNShortLength;

   // This array must be the size specified by mPLMNShortLength
   // UINT8 mPLMNShort[1];
};

struct sNASGetPLMNNameResponse_Name3
{
   eQMINASPLMNNameEncodingSchemes mPLMNLongEncoding;
   eQMINASPLMNNameCountryInitials mPLMNLongCountryInitials;
   eQMINASPLMNNameSpareBits mPLMNLongBits;
   UINT8 mPLMNLongLength;

   // This array must be the size specified by mPLMNLongLength
   // UINT8 mPLMNLong[1];
};

struct sNASGetPLMNNameResponse_Name
{
   sNASGetPLMNNameResponse_Name1 mNASGetPLMNNameResponse_Name1;
   sNASGetPLMNNameResponse_Name2 mNASGetPLMNNameResponse_Name2;
   sNASGetPLMNNameResponse_Name3 mNASGetPLMNNameResponse_Name3;
};

// Structure to describe request TLV 0x01 for NASBindSubscription()
struct sNASBindSubscriptionRequest_SubscriptionType
{
   eQMINASSubscriptionType mSubscriptionType;
};

// Structure to describe indication TLV 0x10 for NAS ManagedRoamingIndication
struct sNASManagedRoamingIndication_RadioInterface
{
   eQMINASRadioInterfaces mRadioInterface;
};

// Structure to describe indication TLV 0x10 for NAS DualStandbyPrefIndication
struct sNASDualStandbyPrefIndication_StandbyPreference
{
   eQMINASStandbyPreference mStandbyPreference;
   eQMINASSubscriptionType mPrioritySubscription;
   eQMINASSubscriptionType mActiveSubscription;
   eQMINASSubscriptionType mDefaultDataSubscription;
};

// Structure to describe indication TLV 0x10 for NAS SubscriptionInfoIndication
struct sNASSubscriptionInfoIndication_PrioritySubscriptionInfo
{
   eQMINASSubscriptionType mSubscriptionType;
};

// Structure to describe indication TLV 0x11 for NAS SubscriptionInfoIndication
struct sNASSubscriptionInfoIndication_ActiveSubscriptionInfo
{
   eQMINASActiveSubscription mActiveSubscription;
};

// Structure to describe indication TLV 0x12 for NAS SubscriptionInfoIndication
struct sNASSubscriptionInfoIndication_DefaultDataSubscriptionInfo
{
   INT8 mDefaultDataSubscription;
};

// Structure to describe response TLV 0x10 for NASGetModePref()
struct sNASGetModePrefResponse_ModePreferenceForIDX0
{
   bool mCDMA1x:1;
   bool mCDMA1xEVDO:1;
   bool mGSM:1;
   bool mUMTS:1;
   bool mLTE:1;
   bool mTDSCDMA:1;

   // Padding out 10 bits
   UINT8 mReserved1:2;
   UINT8 mReserved2;
};

// Structure to describe response TLV 0x11 for NASGetModePref()
struct sNASGetModePrefResponse_ModePreferenceForIDX1
{
   bool mCDMA1x:1;
   bool mCDMA1xEVDO:1;
   bool mGSM:1;
   bool mUMTS:1;
   bool mLTE:1;
   bool mTDSCDMA:1;

   // Padding out 10 bits
   UINT8 mReserved1:2;
   UINT8 mReserved2;
};

// Structure to describe request TLV 0x10 for NASSetDualStandbyPreference()
struct sNASSetDualStandbyPreferenceRequest_StandbyPreference
{
   eQMINASStandbyPreference mStandbyPreference;
};

// Structure to describe request TLV 0x11 for NASSetDualStandbyPreference()
struct sNASSetDualStandbyPreferenceRequest_PrioritySubs
{
   eQMINASSubscriptionType mSubscriptionType;
};

// Structure to describe request TLV 0x12 for NASSetDualStandbyPreference()
struct sNASSetDualStandbyPreferenceRequest_DefaultDataSubs
{
   eQMINASSubscriptionType mSubscriptionType;
};

// Structure to describe indication TLV 0x01 for NAS NetworkTimeIndication
struct sNASNetworkTimeIndication_UniversalTime
{
   UINT16 mYear;
   UINT8 mMonth;
   UINT8 mDay;
   UINT8 mHour;
   UINT8 mMinute;
   UINT8 mSecond;
   eQMINASDayOfWeek mDayOfWeek;
};

// Structure to describe indication TLV 0x10 for NAS NetworkTimeIndication
struct sNASNetworkTimeIndication_TimeZone
{
   INT8 mTimeZoneOffset;
};

// Structure to describe indication TLV 0x11 for NAS NetworkTimeIndication
struct sNASNetworkTimeIndication_DaylightSavingAdjustment
{
   eQMINASDaylightSavingsAdjustment mDaylightSavingsAdjustment;
};

// Structure to describe indication TLV 0x12 for NAS NetworkTimeIndication
struct sNASNetworkTimeIndication_RadioInterface
{
   eQMINASRadioInterfaces mRadioInterface;
};

// Structure to describe response TLV 0x10 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_CDMAServiceStatusInfo
{
   eQMINASServiceStatus mServiceStatus;
   eQMINASPreferredDataBath mPreferredDataPath;
};

// Structure to describe response TLV 0x11 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_CDMA1xEVDOServiceStatusInfo
{
   eQMINASServiceStatus mServiceStatus;
   eQMINASPreferredDataBath mPreferredDataPath;
};

// Structure to describe response TLV 0x12 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_GSMServiceStatusInfo
{
   eQMINASServiceStatus mServiceStatus;
   eQMINASServiceStatus mTrueServiceStatus;
   eQMINASPreferredDataBath mPreferredDataPath;
};

// Structure to describe response TLV 0x13 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_WCDMAServiceStatusInfo
{
   eQMINASServiceStatus mServiceStatus;
   eQMINASServiceStatus mTrueServiceStatus;
   eQMINASPreferredDataBath mPreferredDataPath;
};

// Structure to describe response TLV 0x14 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_LTEServiceStatusInfo
{
   eQMINASServiceStatus mServiceStatus;
   eQMINASServiceStatus mTrueServiceStatus;
   eQMINASPreferredDataBath mPreferredDataPath;
};

// Structure to describe response TLV 0x15 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_CDMASystemInfo
{
   INT8 mServiceDomainValid;
   eQMINASSystemServiceCapabilities mServiceDomain;
   INT8 mServiceCapabilityValid;
   eQMINASSystemServiceCapabilities mSystemServiceCapabilities;
   INT8 mRoamStatusValid;
   eQMINASRoamStatus mRoamStatus;
   INT8 mSystemForbiddenValid;
   eQMINASSystemForbidden mSystemForbidden;
   INT8 mSystemPRLMatchValid;
   eQMINASPRLIndicator mSystemPRLMatch;
   INT8 mPRevInUseValid;
   eQMINASRevision mProtocolRevisionInUse;
   INT8 mBaseStationPRevValid;
   eQMINASRevision mBaseStationProtocolRevision;
   INT8 mConcurrentServiceSupportedValid;
   eQMINASConcurrentServiceSupported mConcurrentServiceSupported;
   INT8 mCDMASystemIDValid;
   UINT16 mSystemID;
   UINT16 mNetworkID;
   INT8 mBaseStationInfoValid;
   UINT16 mBaseStationID;
   INT32 mLatitude;
   INT32 mLongitude;
   INT8 mPacketZoneValid;
   UINT16 mPacketZone;
   INT8 mNetworkIDValid;
   char mMobileCountryCode[3];
   char mMobileNetworkCode[3];
};

// Structure to describe response TLV 0x16 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_CDMA1xEVDOSystemInfo
{
   INT8 mServiceDomainValid;
   eQMINASSystemServiceCapabilities mServiceDomain;
   INT8 mServiceCapabilityValid;
   eQMINASSystemServiceCapabilities mSystemServiceCapabilities;
   INT8 mRoamStatusValid;
   eQMINASRoamStatus mRoamStatus;
   INT8 mSystemForbiddenValid;
   eQMINASSystemForbidden mSystemForbidden;
   INT8 mSystemPRLMatchValid;
   eQMINASPRLIndicator mSystemPRLMatch;
   INT8 mCDMA1xEVDOPersonalityValid;
   eQMINASCDMA1xEVDOPersonality mCDMA1xEVDOPersonality;
   INT8 mCDMA1xEVDOActiveProtocolValid;
   eQMINASCDMA1xEVDOActiveProtocol mCDMA1xEVDOActiveProtocol;
   INT8 mSectorIDValid;
   UINT8 mSectorID[16];
};

// Structure to describe response TLV 0x17 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_GSMSystemInfo
{
   INT8 mServiceDomainValid;
   eQMINASSystemServiceCapabilities mServiceDomain;
   INT8 mServiceCapabilityValid;
   eQMINASSystemServiceCapabilities mSystemServiceCapabilities;
   INT8 mRoamStatusValid;
   eQMINASRoamStatus mRoamStatus;
   INT8 mSystemForbiddenValid;
   eQMINASSystemForbidden mSystemForbidden;
   INT8 mLocationAreaCodeValid;
   UINT16 mLocationAreaCode;
   INT8 mCellIDValid;
   UINT32 mCellID;
   INT8 mRegistrationRejectInformationValid;
   eQMINASSystemServiceCapabilities mRegistrationRejectServiceDomain;
   UINT8 mRejectCause;
   INT8 mNetworkIDValid;
   char mMobileCountryCode[3];
   char mMobileNetworkCode[3];
   INT8 mEGPRSSupportValid;
   eQMINASEGPRSSupport mEGPRSSupport;
   INT8 mDTMSupportValid;
   eQMINASDTMSupport mDTMSupport;
};

// Structure to describe response TLV 0x18 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_WCDMASystemInfo
{
   INT8 mServiceDomainValid;
   eQMINASSystemServiceCapabilities mServiceDomain;
   INT8 mServiceCapabilityValid;
   eQMINASSystemServiceCapabilities mSystemServiceCapabilities;
   INT8 mRoamStatusValid;
   eQMINASRoamStatus mRoamStatus;
   INT8 mSystemForbiddenValid;
   eQMINASSystemForbidden mSystemForbidden;
   INT8 mLocationAreaCodeValid;
   UINT16 mLocationAreaCode;
   INT8 mCellIDValid;
   UINT32 mCellID;
   INT8 mRegistrationRejectInformationValid;
   eQMINASSystemServiceCapabilities mRegistrationRejectServiceDomain;
   UINT8 mRejectCause;
   INT8 mNetworkIDValid;
   char mMobileCountryCode[3];
   char mMobileNetworkCode[3];
   INT8 mHighSpeedCallStatusValid;
   eQMINASHighSpeedCallStatus mHighSpeedCallStatus;
   INT8 mHighSpeedServiceIndicationValid;
   eQMINASHighSpeedCallStatus mHighSpeedServiceIndication;
   INT8 mPrimaryScramblingCodeValue;
   UINT16 mPrimaryScramblingCode;
};

// Structure to describe response TLV 0x19 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_LTESystemInfo
{
   INT8 mServiceDomainValid;
   eQMINASSystemServiceCapabilities mServiceDomain;
   INT8 mServiceCapabilityValid;
   eQMINASSystemServiceCapabilities mSystemServiceCapabilities;
   INT8 mRoamStatusValid;
   eQMINASRoamStatus mRoamStatus;
   INT8 mSystemForbiddenValid;
   eQMINASSystemForbidden mSystemForbidden;
   INT8 mLocationAreaCodeValid;
   UINT16 mLocationAreaCode;
   INT8 mCellIDValid;
   UINT32 mCellID;
   INT8 mRegistrationRejectInformationValid;
   eQMINASSystemServiceCapabilities mRegistrationRejectServiceDomain;
   UINT8 mRejectCause;
   INT8 mNetworkIDValid;
   char mMobileCountryCode[3];
   char mMobileNetworkCode[3];
   INT8 mTrackingAreaCodeValid;
   UINT16 mTrackingAreaCode;
};

// Structure to describe response TLV 0x1A for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_MoreCDMASystemInfo
{
   UINT16 mGeoSystemIndex;
   UINT16 mRegistrationPeriod;
};

// Structure to describe response TLV 0x1B for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_MoreCDMA1xEVDOSystemInfo
{
   UINT16 mGeoSystemIndex;
};

// Structure to describe response TLV 0x1C for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_MoreGSMSystemInfo
{
   UINT16 mGeoSystemIndex;
   eQMINASCellBroadcastCaps mCellBroadcastCapability;
};

// Structure to describe response TLV 0x1D for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_MoreWCDMASystemInfo
{
   UINT16 mGeoSystemIndex;
   eQMINASCellBroadcastCaps mCellBroadcastCapability;
};

// Structure to describe response TLV 0x1E for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_MoreLTESystemInfo
{
   UINT16 mGeoSystemIndex;
};

// Structure to describe response TLV 0x1F for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_GSMCallBarring
{
   eQMINASCallBarringStatus mCSCallBarringStatus;
   eQMINASCallBarringStatus mPSCallBarringStatus;
};

// Structure to describe response TLV 0x20 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_WCDMACallBarring
{
   eQMINASCallBarringStatus mCSCallBarringStatus;
   eQMINASCallBarringStatus mPSCallBarringStatus;
};

// Structure to describe response TLV 0x21 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_LTEVoice
{
   INT8 mLTEVoiceSupported;
};

// Structure to describe response TLV 0x22 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_GSMCipher
{
   eQMINASServiceDomains mCipheringOnServiceDomain;
};

// Structure to describe response TLV 0x23 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_WCDMACipher
{
   eQMINASServiceDomains mCipheringOnServiceDomain;
};

// Structure to describe response TLV 0x24 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_TDSCDMAServiceStatusInfo
{
   eQMINASServiceStatus mServiceStatus;
   eQMINASServiceStatus mTrueServiceStatus;
   eQMINASPreferredDataBath mPreferredDataPath;
};

// Structure to describe response TLV 0x25 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_TDSCDMASystemInfo
{
   INT8 mServiceDomainValid;
   eQMINASSystemServiceCapabilities mServiceDomain;
   INT8 mServiceCapabilityValid;
   eQMINASSystemServiceCapabilities mSystemServiceCapabilities;
   INT8 mRoamStatusValid;
   eQMINASRoamStatus mRoamStatus;
   INT8 mSystemForbiddenValid;
   eQMINASSystemForbidden mSystemForbidden;
   INT8 mLocationAreaCodeValid;
   UINT16 mLocationAreaCode;
   INT8 mCellIDValid;
   UINT32 mCellID;
   INT8 mRegistrationRejectInformationValid;
   eQMINASSystemServiceCapabilities mRegistrationRejectServiceDomain;
   UINT8 mRejectCause;
   INT8 mNetworkIDValid;
   char mMobileCountryCode[3];
   char mMobileNetworkCode[3];
   INT8 mHighSpeedCallStatusValid;
   eQMINASHighSpeedCallStatus mHighSpeedCallStatus;
   INT8 mHighSpeedServiceIndicationValid;
   eQMINASHighSpeedCallStatus mHighSpeedServiceIndication;
   INT8 mCellParameterIDValid;
   UINT16 mCellParameterID;
   INT8 mCellBroadcastCapabilityValid;
   eQMINASCellBroadcastCaps2 mCellBroadcastCapability;
   INT8 mCSBarringStatusValid;
   eQMINASCallBarringStatus mCSCallBarringStatus;
   INT8 mPSBarringStatusValid;
   eQMINASCallBarringStatus mPSCallBarringStatus;
   INT8 mCipheringValid;
   eQMINASServiceDomains mCipheringOnServiceDomain;
};

// Structure to describe response TLV 0x26 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_EMBMSCoverage
{
   INT8 mEMBMSSupported;
};

// Structure to describe response TLV 0x27 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_SIMRejectInfo
{
   eQMINASSIMRejectStates mSIMRejectInfo;
};

// Structure to describe response TLV 0x28 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_WCDMAEUTRADetection
{
   eQMINASEUTRAStatus mEUTRADetectionStatus;
};

// Structure to describe response TLV 0x29 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_LTEIMSVoice
{
   INT8 mIMSVoiceSupportAvailable;
};

// Structure to describe response TLV 0x2A for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_LTEVoiceDomain
{
   eQMINASLTEVoiceDomains mLTEVoiceDomain;
};

// Structure to describe response TLV 0x2B for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_CDMARegZoneID
{
   UINT16 mCDMARegZoneID;
};

// Structure to describe response TLV 0x2C for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_GSMRAC
{
   UINT8 mGSMRoutingAreaCode;
};

// Structure to describe response TLV 0x2D for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_WCDMARAC
{
   UINT8 mWCDMARoutingAreaCode;
};

// Structure to describe response TLV 0x2E for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_CDMAResolvedMCC
{
   UINT16 mCDMAMCCResolvedViaSIDLookup;
};

// Structure to describe response TLV 0x2F for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_RegistrationRestriction
{
   eQMINASRegistrationRestrictions mRegistrationRestriction;
};

// Structure to describe response TLV 0x30 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_TDSCDMARegistrationDomain
{
   eQMINASRegistrationDomains mTDSCDMARegistrationDomain;
};

// Structure to describe response TLV 0x31 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_LTERegistrationDomain
{
   eQMINASRegistrationDomains mTDSCDMARegistrationDomain;
};

// Structure to describe response TLV 0x32 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_WCDMARegistrationDomain
{
   eQMINASRegistrationDomains mTDSCDMARegistrationDomain;
};

// Structure to describe response TLV 0x33 for NASGetSystemInfo()
struct sNASGetSystemInfoResponse_GSMRegistrationDomain
{
   eQMINASRegistrationDomains mTDSCDMARegistrationDomain;
};

// Structure to describe indication TLV 0x10 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_CDMAServiceStatusInfo
{
   eQMINASServiceStatus mServiceStatus;
   eQMINASPreferredDataBath mPreferredDataPath;
};

// Structure to describe indication TLV 0x11 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_CDMA1xEVDOServiceStatusInfo
{
   eQMINASServiceStatus mServiceStatus;
   eQMINASPreferredDataBath mPreferredDataPath;
};

// Structure to describe indication TLV 0x12 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_GSMServiceStatusInfo
{
   eQMINASServiceStatus mServiceStatus;
   eQMINASServiceStatus mTrueServiceStatus;
   eQMINASPreferredDataBath mPreferredDataPath;
};

// Structure to describe indication TLV 0x13 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_WCDMAServiceStatusInfo
{
   eQMINASServiceStatus mServiceStatus;
   eQMINASServiceStatus mTrueServiceStatus;
   eQMINASPreferredDataBath mPreferredDataPath;
};

// Structure to describe indication TLV 0x14 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_LTEServiceStatusInfo
{
   eQMINASServiceStatus mServiceStatus;
   eQMINASServiceStatus mTrueServiceStatus;
   eQMINASPreferredDataBath mPreferredDataPath;
};

// Structure to describe indication TLV 0x15 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_CDMASystemInfo
{
   INT8 mServiceDomainValid;
   eQMINASSystemServiceCapabilities mServiceDomain;
   INT8 mServiceCapabilityValid;
   eQMINASSystemServiceCapabilities mSystemServiceCapabilities;
   INT8 mRoamStatusValid;
   eQMINASRoamStatus mRoamStatus;
   INT8 mSystemForbiddenValid;
   eQMINASSystemForbidden mSystemForbidden;
   INT8 mSystemPRLMatchValid;
   eQMINASPRLIndicator mSystemPRLMatch;
   INT8 mPRevInUseValid;
   eQMINASRevision mProtocolRevisionInUse;
   INT8 mBaseStationPRevValid;
   eQMINASRevision mBaseStationProtocolRevision;
   INT8 mConcurrentServiceSupportedValid;
   eQMINASConcurrentServiceSupported mConcurrentServiceSupported;
   INT8 mCDMASystemIDValid;
   UINT16 mSystemID;
   UINT16 mNetworkID;
   INT8 mBaseStationInfoValid;
   UINT16 mBaseStationID;
   INT32 mLatitude;
   INT32 mLongitude;
   INT8 mPacketZoneValid;
   UINT16 mPacketZone;
   INT8 mNetworkIDValid;
   char mMobileCountryCode[3];
   char mMobileNetworkCode[3];
};

// Structure to describe indication TLV 0x16 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_CDMA1xEVDOSystemInfo
{
   INT8 mServiceDomainValid;
   eQMINASSystemServiceCapabilities mServiceDomain;
   INT8 mServiceCapabilityValid;
   eQMINASSystemServiceCapabilities mSystemServiceCapabilities;
   INT8 mRoamStatusValid;
   eQMINASRoamStatus mRoamStatus;
   INT8 mSystemForbiddenValid;
   eQMINASSystemForbidden mSystemForbidden;
   INT8 mSystemPRLMatchValid;
   eQMINASPRLIndicator mSystemPRLMatch;
   INT8 mCDMA1xEVDOPersonalityValid;
   eQMINASCDMA1xEVDOPersonality mCDMA1xEVDOPersonality;
   INT8 mCDMA1xEVDOActiveProtocolValid;
   eQMINASCDMA1xEVDOActiveProtocol mCDMA1xEVDOActiveProtocol;
   INT8 mSectorIDValid;
   UINT8 mSectorID[16];
};

// Structure to describe indication TLV 0x17 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_GSMSystemInfo
{
   INT8 mServiceDomainValid;
   eQMINASSystemServiceCapabilities mServiceDomain;
   INT8 mServiceCapabilityValid;
   eQMINASSystemServiceCapabilities mSystemServiceCapabilities;
   INT8 mRoamStatusValid;
   eQMINASRoamStatus mRoamStatus;
   INT8 mSystemForbiddenValid;
   eQMINASSystemForbidden mSystemForbidden;
   INT8 mLocationAreaCodeValid;
   UINT16 mLocationAreaCode;
   INT8 mCellIDValid;
   UINT32 mCellID;
   INT8 mRegistrationRejectInformationValid;
   eQMINASSystemServiceCapabilities mRegistrationRejectServiceDomain;
   UINT8 mRejectCause;
   INT8 mNetworkIDValid;
   char mMobileCountryCode[3];
   char mMobileNetworkCode[3];
   INT8 mEGPRSSupportValid;
   eQMINASEGPRSSupport mEGPRSSupport;
   INT8 mDTMSupportValid;
   eQMINASDTMSupport mDTMSupport;
};

// Structure to describe indication TLV 0x18 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_WCDMASystemInfo
{
   INT8 mServiceDomainValid;
   eQMINASSystemServiceCapabilities mServiceDomain;
   INT8 mServiceCapabilityValid;
   eQMINASSystemServiceCapabilities mSystemServiceCapabilities;
   INT8 mRoamStatusValid;
   eQMINASRoamStatus mRoamStatus;
   INT8 mSystemForbiddenValid;
   eQMINASSystemForbidden mSystemForbidden;
   INT8 mLocationAreaCodeValid;
   UINT16 mLocationAreaCode;
   INT8 mCellIDValid;
   UINT32 mCellID;
   INT8 mRegistrationRejectInformationValid;
   eQMINASSystemServiceCapabilities mRegistrationRejectServiceDomain;
   UINT8 mRejectCause;
   INT8 mNetworkIDValid;
   char mMobileCountryCode[3];
   char mMobileNetworkCode[3];
   INT8 mHighSpeedCallStatusValid;
   eQMINASHighSpeedCallStatus mHighSpeedCallStatus;
   INT8 mHighSpeedServiceIndicationValid;
   eQMINASHighSpeedCallStatus mHighSpeedServiceIndication;
   INT8 mPrimaryScramblingCodeValue;
   UINT16 mPrimaryScramblingCode;
};

// Structure to describe indication TLV 0x19 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_LTESystemInfo
{
   INT8 mServiceDomainValid;
   eQMINASSystemServiceCapabilities mServiceDomain;
   INT8 mServiceCapabilityValid;
   eQMINASSystemServiceCapabilities mSystemServiceCapabilities;
   INT8 mRoamStatusValid;
   eQMINASRoamStatus mRoamStatus;
   INT8 mSystemForbiddenValid;
   eQMINASSystemForbidden mSystemForbidden;
   INT8 mLocationAreaCodeValid;
   UINT16 mLocationAreaCode;
   INT8 mCellIDValid;
   UINT32 mCellID;
   INT8 mRegistrationRejectInformationValid;
   eQMINASSystemServiceCapabilities mRegistrationRejectServiceDomain;
   UINT8 mRejectCause;
   INT8 mNetworkIDValid;
   char mMobileCountryCode[3];
   char mMobileNetworkCode[3];
   INT8 mTrackingAreaCodeValid;
   UINT16 mTrackingAreaCode;
};

// Structure to describe indication TLV 0x1A for NAS SystemInfoIndication
struct sNASSystemInfoIndication_MoreCDMASystemInfo
{
   UINT16 mGeoSystemIndex;
   UINT16 mRegistrationPeriod;
};

// Structure to describe indication TLV 0x1B for NAS SystemInfoIndication
struct sNASSystemInfoIndication_MoreCDMA1xEVDOSystemInfo
{
   UINT16 mGeoSystemIndex;
};

// Structure to describe indication TLV 0x1C for NAS SystemInfoIndication
struct sNASSystemInfoIndication_MoreGSMSystemInfo
{
   UINT16 mGeoSystemIndex;
   eQMINASCellBroadcastCaps mCellBroadcastCapability;
};

// Structure to describe indication TLV 0x1D for NAS SystemInfoIndication
struct sNASSystemInfoIndication_MoreWCDMASystemInfo
{
   UINT16 mGeoSystemIndex;
   eQMINASCellBroadcastCaps mCellBroadcastCapability;
};

// Structure to describe indication TLV 0x1E for NAS SystemInfoIndication
struct sNASSystemInfoIndication_MoreLTESystemInfo
{
   UINT16 mGeoSystemIndex;
};

// Structure to describe indication TLV 0x1F for NAS SystemInfoIndication
struct sNASSystemInfoIndication_GSMCallBarring
{
   eQMINASCallBarringStatus mCSCallBarringStatus;
   eQMINASCallBarringStatus mPSCallBarringStatus;
};

// Structure to describe indication TLV 0x20 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_WCDMACallBarring
{
   eQMINASCallBarringStatus mCSCallBarringStatus;
   eQMINASCallBarringStatus mPSCallBarringStatus;
};

// Structure to describe indication TLV 0x21 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_LTEVoice
{
   INT8 mLTEVoiceSupported;
};

// Structure to describe indication TLV 0x22 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_GSMCipher
{
   eQMINASServiceDomains mCipheringOnServiceDomain;
};

// Structure to describe indication TLV 0x23 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_WCDMACipher
{
   eQMINASServiceDomains mCipheringOnServiceDomain;
};

// Structure to describe indication TLV 0x24 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_NoPLMNChange
{
   INT8 mNoPLMNChange;
};

// Structure to describe indication TLV 0x25 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_TDSCDMAServiceStatusInfo
{
   eQMINASServiceStatus mServiceStatus;
   eQMINASServiceStatus mTrueServiceStatus;
   eQMINASPreferredDataBath mPreferredDataPath;
};

// Structure to describe indication TLV 0x26 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_TDSCDMASystemInfo
{
   INT8 mServiceDomainValid;
   eQMINASSystemServiceCapabilities mServiceDomain;
   INT8 mServiceCapabilityValid;
   eQMINASSystemServiceCapabilities mSystemServiceCapabilities;
   INT8 mRoamStatusValid;
   eQMINASRoamStatus mRoamStatus;
   INT8 mSystemForbiddenValid;
   eQMINASSystemForbidden mSystemForbidden;
   INT8 mLocationAreaCodeValid;
   UINT16 mLocationAreaCode;
   INT8 mCellIDValid;
   UINT32 mCellID;
   INT8 mRegistrationRejectInformationValid;
   eQMINASSystemServiceCapabilities mRegistrationRejectServiceDomain;
   UINT8 mRejectCause;
   INT8 mNetworkIDValid;
   char mMobileCountryCode[3];
   char mMobileNetworkCode[3];
   INT8 mHighSpeedCallStatusValid;
   eQMINASHighSpeedCallStatus mHighSpeedCallStatus;
   INT8 mHighSpeedServiceIndicationValid;
   eQMINASHighSpeedCallStatus mHighSpeedServiceIndication;
   INT8 mCellParameterIDValid;
   UINT16 mCellParameterID;
   INT8 mCellBroadcastCapabilityValid;
   eQMINASCellBroadcastCaps2 mCellBroadcastCapability;
   INT8 mCSBarringStatusValid;
   eQMINASCallBarringStatus mCSCallBarringStatus;
   INT8 mPSBarringStatusValid;
   eQMINASCallBarringStatus mPSCallBarringStatus;
   INT8 mCipheringValid;
   eQMINASServiceDomains mCipheringOnServiceDomain;
};

// Structure to describe indication TLV 0x27 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_EMBMSCoverage
{
   INT8 mEMBMSSupported;
};

// Structure to describe indication TLV 0x28 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_SIMRejectInfo
{
   eQMINASSIMRejectStates mSIMRejectInfo;
};

// Structure to describe indication TLV 0x29 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_WCDMAEUTRADetection
{
   eQMINASEUTRAStatus mEUTRADetectionStatus;
};

// Structure to describe indication TLV 0x2A for NAS SystemInfoIndication
struct sNASSystemInfoIndication_LTEIMSVoice
{
   INT8 mIMSVoiceSupportAvailable;
};

// Structure to describe indication TLV 0x2B for NAS SystemInfoIndication
struct sNASSystemInfoIndication_LTEVoiceDomain
{
   eQMINASLTEVoiceDomains mLTEVoiceDomain;
};

// Structure to describe indication TLV 0x2C for NAS SystemInfoIndication
struct sNASSystemInfoIndication_CDMARegZoneID
{
   UINT16 mCDMARegZoneID;
};

// Structure to describe indication TLV 0x2D for NAS SystemInfoIndication
struct sNASSystemInfoIndication_GSMRAC
{
   UINT8 mGSMRoutingAreaCode;
};

// Structure to describe indication TLV 0x2E for NAS SystemInfoIndication
struct sNASSystemInfoIndication_WCDMARAC
{
   UINT8 mWCDMARoutingAreaCode;
};

// Structure to describe indication TLV 0x2F for NAS SystemInfoIndication
struct sNASSystemInfoIndication_CDMAResolvedMCC
{
   UINT16 mCDMAMCCResolvedViaSIDLookup;
};

// Structure to describe indication TLV 0x30 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_RegistrationRestriction
{
   eQMINASRegistrationRestrictions mRegistrationRestriction;
};

// Structure to describe indication TLV 0x31 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_TDSCDMARegistrationDomain
{
   eQMINASRegistrationDomains mTDSCDMARegistrationDomain;
};

// Structure to describe indication TLV 0x32 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_LTERegistrationDomain
{
   eQMINASRegistrationDomains mTDSCDMARegistrationDomain;
};

// Structure to describe indication TLV 0x33 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_WCDMARegistrationDomain
{
   eQMINASRegistrationDomains mTDSCDMARegistrationDomain;
};

// Structure to describe indication TLV 0x34 for NAS SystemInfoIndication
struct sNASSystemInfoIndication_GSMRegistrationDomain
{
   eQMINASRegistrationDomains mTDSCDMARegistrationDomain;
};

// Structure to describe response TLV 0x10 for NASGetSignalInfo()
struct sNASGetSignalInfoResponse_CDMASignalInfo
{
   INT8 mRSSI;
   UINT16 mECIO;
};

// Structure to describe response TLV 0x11 for NASGetSignalInfo()
struct sNASGetSignalInfoResponse_CDMA1xEVDOSignalInfo
{
   INT8 mRSSI;
   UINT16 mECIO;
   eQMINASSINRLevels mSINR;
   UINT32 mIO;
};

// Structure to describe response TLV 0x12 for NASGetSignalInfo()
struct sNASGetSignalInfoResponse_GSMSignalInfo
{
   INT8 mRSSI;
};

// Structure to describe response TLV 0x13 for NASGetSignalInfo()
struct sNASGetSignalInfoResponse_WCDMASignalInfo
{
   INT8 mRSSI;
   UINT16 mECIO;
};

// Structure to describe response TLV 0x14 for NASGetSignalInfo()
struct sNASGetSignalInfoResponse_LTESignalInfo
{
   INT8 mRSSI;
   INT8 mRSRQ;
   INT16 mRSRP;
   INT16 mSNR;
};

// Structure to describe response TLV 0x15 for NASGetSignalInfo()
struct sNASGetSignalInfoResponse_TDSCDMASignalInfo
{
   INT8 mPCCPCHRSCP;
};

// Structure to describe request TLV 0x10 for NASConfigureSignalInfo()
struct sNASConfigureSignalInfoRequest_RSSIThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT8 mThreshold[1];
};

// Structure to describe request TLV 0x11 for NASConfigureSignalInfo()
struct sNASConfigureSignalInfoRequest_ECIOThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT16 mThreshold[1];
};

// Structure to describe request TLV 0x12 for NASConfigureSignalInfo()
struct sNASConfigureSignalInfoRequest_CDMA1xEVDOSINRThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // UINT8 mThreshold[1];
};

// Structure to describe request TLV 0x13 for NASConfigureSignalInfo()
struct sNASConfigureSignalInfoRequest_LTESINRThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT8 mThreshold[1];
};

// Structure to describe request TLV 0x14 for NASConfigureSignalInfo()
struct sNASConfigureSignalInfoRequest_IOThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT32 mThreshold[1];
};

// Structure to describe request TLV 0x15 for NASConfigureSignalInfo()
struct sNASConfigureSignalInfoRequest_RSRQThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT8 mThreshold[1];
};

// Structure to describe request TLV 0x16 for NASConfigureSignalInfo()
struct sNASConfigureSignalInfoRequest_RSRPThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT16 mThreshold[1];
};

// Structure to describe request TLV 0x17 for NASConfigureSignalInfo()
struct sNASConfigureSignalInfoRequest_LTEConfig
{
   eQMINASLTESignalRates mLTESignalCheckRate;
   eQMINASLTESignalRates mLTESignalAveragingRate;
};

// Structure to describe request TLV 0x18 for NASConfigureSignalInfo()
struct sNASConfigureSignalInfoRequest_RSCPThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT8 mRSCPThreshold[1];
};

// Structure to describe indication TLV 0x10 for NAS SignalInfoIndication
struct sNASSignalInfoIndication_CDMASignalInfo
{
   INT8 mRSSI;
   UINT16 mECIO;
};

// Structure to describe indication TLV 0x11 for NAS SignalInfoIndication
struct sNASSignalInfoIndication_CDMA1xEVDOSignalInfo
{
   INT8 mRSSI;
   UINT16 mECIO;
   eQMINASSINRLevels mSINR;
   UINT32 mIO;
};

// Structure to describe indication TLV 0x12 for NAS SignalInfoIndication
struct sNASSignalInfoIndication_GSMSignalInfo
{
   INT8 mRSSI;
};

// Structure to describe indication TLV 0x13 for NAS SignalInfoIndication
struct sNASSignalInfoIndication_WCDMASignalInfo
{
   INT8 mRSSI;
   UINT16 mECIO;
};

// Structure to describe indication TLV 0x14 for NAS SignalInfoIndication
struct sNASSignalInfoIndication_LTESignalInfo
{
   INT8 mRSSI;
   INT8 mRSRQ;
   INT16 mRSRP;
   INT16 mSNR;
};

// Structure to describe indication TLV 0x15 for NAS SignalInfoIndication
struct sNASSignalInfoIndication_TDSCDMASignalInfo
{
   INT8 mPCCPCHRSCP;
};

// Structure to describe response TLV 0x10 for NASGetErrorRate()
struct sNASGetErrorRateResponse_CDMAFrameErrorRate
{
   UINT16 mErrorRate;
};

// Structure to describe response TLV 0x11 for NASGetErrorRate()
struct sNASGetErrorRateResponse_CDMA1xEVDOPacketErrorRate
{
   UINT16 mErrorRate;
};

// Structure to describe response TLV 0x12 for NASGetErrorRate()
struct sNASGetErrorRateResponse_GSMBitErrorRate
{
   UINT8 mErrorRate;
};

// Structure to describe response TLV 0x13 for NASGetErrorRate()
struct sNASGetErrorRateResponse_WCDMABlockErrorRate
{
   UINT8 mErrorRate;
};

// Structure to describe response TLV 0x14 for NASGetErrorRate()
struct sNASGetErrorRateResponse_TDSCDMABlockErrorRate
{
   UINT8 mErrorRate;
};

// Structure to describe indication TLV 0x10 for NAS ErrorRateIndication
struct sNASErrorRateIndication_CDMAFrameErrorRate
{
   UINT16 mErrorRate;
};

// Structure to describe indication TLV 0x11 for NAS ErrorRateIndication
struct sNASErrorRateIndication_CDMA1xEVDOPacketErrorRate
{
   UINT16 mErrorRate;
};

// Structure to describe indication TLV 0x12 for NAS ErrorRateIndication
struct sNASErrorRateIndication_GSMBitErrorRate
{
   UINT8 mErrorRate;
};

// Structure to describe indication TLV 0x13 for NAS ErrorRateIndication
struct sNASErrorRateIndication_WCDMAFrameErrorRate
{
   UINT8 mErrorRate;
};

// Structure to describe indication TLV 0x14 for NAS ErrorRateIndication
struct sNASErrorRateIndication_TDSCDMABlockErrorRate
{
   UINT8 mErrorRate;
};

// Structure to describe indication TLV 0x01 for NAS EVDOSessionCloseIndication
struct sNASEVDOSessionCloseIndication_Reason
{
   eQMINASEVDOSessionCloseReasons mSessionCloseReason;
};

// Structure to describe indication TLV 0x01 for NAS EVDOUATIUpdateIndication
struct sNASEVDOUATIUpdateIndication_UATI
{
   UINT8 mUATI[16];
};

// Structure to describe request TLV 0x01 for NASGetEVDOProtocolSubtype()
struct sNASGetEVDOProtocolSubtypeRequest_Protocol
{
   UINT32 mEVDOProtocol;
};

// Structure to describe response TLV 0x10 for NASGetEVDOProtocolSubtype()
struct sNASGetEVDOProtocolSubtypeResponse_Subtype
{
   UINT16 mEVDOProtocolSubtype;
};

// Structure to describe response TLV 0x10 for NASGetEVDOColorCode()
struct sNASGetEVDOColorCodeResponse_Value
{
   UINT8 mEVDOColorCode;
};

// Structure to describe response TLV 0x10 for NASGetAcquisitionSystemMode()
struct sNASGetAcquisitionSystemModeResponse_CDMA
{
   eQMINASRadioSystemModes mRadioSystemMode;
};

// Structure to describe response TLV 0x11 for NASGetAcquisitionSystemMode()
struct sNASGetAcquisitionSystemModeResponse_CDMA1xEVDO
{
   eQMINASRadioSystemModes mRadioSystemMode;
};

// Structure to describe response TLV 0x12 for NASGetAcquisitionSystemMode()
struct sNASGetAcquisitionSystemModeResponse_GSM
{
   eQMINASRadioSystemModes mRadioSystemMode;
};

// Structure to describe response TLV 0x13 for NASGetAcquisitionSystemMode()
struct sNASGetAcquisitionSystemModeResponse_UMTS
{
   eQMINASRadioSystemModes mRadioSystemMode;
};

// Structure to describe response TLV 0x14 for NASGetAcquisitionSystemMode()
struct sNASGetAcquisitionSystemModeResponse_LTE
{
   eQMINASRadioSystemModes mRadioSystemMode;
};

// Structure to describe response TLV 0x15 for NASGetAcquisitionSystemMode()
struct sNASGetAcquisitionSystemModeResponse_TDSCDMA
{
   eQMINASRadioSystemModes mRadioSystemMode;
};

// Structure to describe request TLV 0x01 for NASSetRXDiversity()
struct sNASSetRXDiversityRequest_Diversity
{
   eQMINASRadioInterfaces mRadioInterface;
   bool mEnableRXChain0:1;
   bool mEnableRXChain1:1;

   // Padding out 6 bits
   UINT8 mReserved1:6;
};

// Structure to describe response TLV 0x10 for NASGetRXTXInfo()
struct sNASGetRXTXInfoResponse_RX0Info
{
   INT8 mRadioTuned;
   INT32 mRXPower;
   INT32 mEcIo;
   INT32 mRSCP;
   INT32 mRSRP;
   INT32 mPhase;
};

// Structure to describe response TLV 0x11 for NASGetRXTXInfo()
struct sNASGetRXTXInfoResponse_RX1Info
{
   INT8 mRadioTuned;
   INT32 mRXPower;
   INT32 mEcIo;
   INT32 mRSCP;
   INT32 mRSRP;
   INT32 mPhase;
};

// Structure to describe response TLV 0x12 for NASGetRXTXInfo()
struct sNASGetRXTXInfoResponse_TXInfo
{
   INT8 mInTraffic;
   INT32 mTXPower;
};

// Structure to describe request TLV 0x01 for NASUpdateAKEYExtended()
struct sNASUpdateAKEYExtendedRequest_AKEY
{
   char mSPC[6];
   char mAKEY[26];
};

// Structure to describe response TLV 0x10 for NASGetDualStandbyPreference()
struct sNASGetDualStandbyPreferenceResponse_StandbyPreference
{
   eQMINASStandbyPreference mStandbyPreference;
};

// Structure to describe response TLV 0x11 for NASGetDualStandbyPreference()
struct sNASGetDualStandbyPreferenceResponse_PrioritySubs
{
   eQMINASSubscriptionType mSubscriptionType;
};

// Structure to describe response TLV 0x12 for NASGetDualStandbyPreference()
struct sNASGetDualStandbyPreferenceResponse_ActiveSubs
{
   eQMINASSubscriptionType mSubscriptionType;
};

// Structure to describe response TLV 0x13 for NASGetDualStandbyPreference()
struct sNASGetDualStandbyPreferenceResponse_DefaultDataSubs
{
   eQMINASSubscriptionType mSubscriptionType;
};

// Structure to describe request TLV 0x01 for NASBlockLTEPLMN()
struct sNASBlockLTEPLMNRequest_PLMN
{
   UINT16 mMobileCountryCode;
   UINT16 mMobileNetworkCode;
   INT8 mMNCIncludesPCSDigit;
};

// Structure to describe request TLV 0x10 for NASBlockLTEPLMN()
struct sNASBlockLTEPLMNRequest_AbsoluteTime
{
   UINT32 mBlockingIntervalInMilliseconds;
};

// Structure to describe request TLV 0x11 for NASBlockLTEPLMN()
struct sNASBlockLTEPLMNRequest_T3204Multiplier
{
   UINT32 mBlockingIntervalAsT3204Multiplier;
};

// Structure to describe request TLV 0x01 for NASUnblockLTEPLMN()
struct sNASUnblockLTEPLMNRequest_PLMN
{
   UINT16 mMobileCountryCode;
   UINT16 mMobileNetworkCode;
   INT8 mMNCIncludesPCSDigit;
};

// Structure to describe indication TLV 0x10 for NAS CurrentPLMNNameIndication
struct sNASCurrentPLMNNameIndication_PLMN
{
   UINT16 mMobileCountryCode;
   UINT16 mMobileNetworkCode;
   INT8 mMNCIncludesPCSDigit;
};

// Structure to describe indication TLV 0x11 for NAS CurrentPLMNNameIndication
struct sNASCurrentPLMNNameIndication_SPN
{
   eQMINASPLMNNameEncodingSchemes mSPNEncoding;
   UINT8 mSPNLength;

   // This array must be the size specified by mSPNLength
   // UINT8 mSPN[1];
};

// Structure to describe indication TLV 0x12 for NAS CurrentPLMNNameIndication
struct sNASCurrentPLMNNameIndication_ShortName
{
   eQMINASPLMNNameEncodingSchemes mPLMNShortEncoding;
   eQMINASPLMNNameCountryInitials mPLMNShortCountryInitials;
   eQMINASPLMNNameSpareBits mPLMNSpareBits;
   UINT8 mPLMNShortLength;

   // This array must be the size specified by mPLMNShortLength
   // UINT8 mPLMNShort[1];
};

// Structure to describe indication TLV 0x13 for NAS CurrentPLMNNameIndication
struct sNASCurrentPLMNNameIndication_LongName
{
   eQMINASPLMNNameEncodingSchemes mPLMNLongEncoding;
   eQMINASPLMNNameCountryInitials mPLMNLongCountryInitials;
   eQMINASPLMNNameSpareBits mPLMNLongBits;
   UINT8 mPLMNLongLength;

   // This array must be the size specified by mPLMNLongLength
   // UINT8 mPLMNLong[1];
};

// Structure to describe request TLV 0x01 for NASConfigureEMBMS()
struct sNASConfigureEMBMSRequest_Config
{
   INT8 mEMBMSEnabled;
};

// Structure to describe response TLV 0x10 for NASGetEMBMSStatus()
struct sNASGetEMBMSStatusResponse_Status
{
   INT8 mEMBMSEnabled;
};

// Structure to describe indication TLV 0x01 for NAS EMBMSStatusIndication
struct sNASEMBMSStatusIndication_Status
{
   INT8 mEMBMSEnabled;
};

// Structure to describe response TLV 0x10 for NASGetCDMAPositionInfo()
struct sNASGetCDMAPositionInfoResponse_Info
{
   INT8 mUEInIdleMode;
   UINT8 mBaseStationCount;

   struct sBaseStation
   {
      eQMINASCDMAPilotTypes mPilotType;
      UINT16 mSystemID;
      UINT16 mNetworkID;
      UINT16 mBaseStationID;
      UINT16 mPilotPN;
      UINT16 mPilotStrength;
      INT32 mLatitude;
      INT32 mLongitude;
      UINT64 mGPSTimeInMilliseconds;
   };

   // This array must be the size specified by mBaseStationCount
   // sBaseStation mBaseStations[1];
};

// Structure to describe indication TLV 0x01 for NAS RFBandInfoIndication
struct sNASRFBandInfoIndication_BandInfo
{
   eQMINASRadioInterfaces mRadioInterface;
   eQMINASBandClasses mActiveBandClass;
   UINT16 mActiveChannel;
};

// Structure to describe indication TLV 0x01 for NAS NetworkRejectIndication
struct sNASNetworkRejectIndication_RadioInterface
{
   eQMINASRadioInterfaces mRadioInterface;
};

// Structure to describe indication TLV 0x03 for NAS NetworkRejectIndication
struct sNASNetworkRejectIndication_RejectCause
{
   UINT8 mRejectCause;
};

// Structure to describe response TLV 0x10 for NASGetManagedRoamingConfig()
struct sNASGetManagedRoamingConfigResponse_Config
{
   INT8 mManagedRoamingSupported;
};

// Structure to describe indication TLV 0x10 for NAS RTREConfigurationIndication
struct sNASRTREConfigurationIndication_CurrentConfig
{
   eQMINASRTREConfiguration mRTREConfiguration;
};

// Structure to describe indication TLV 0x11 for NAS RTREConfigurationIndication
struct sNASRTREConfigurationIndication_ConfigPreference
{
   eQMINASRTREConfiguration mRTREPreference;
};

// Structure to describe response TLV 0x10 for NASGetCentralizedEONSSupport()
struct sNASGetCentralizedEONSSupportResponse_Status
{
   INT8 mCentralizedEONSSupported;
};

// Structure to describe request TLV 0x10 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_CDMARSSIThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT16 mThreshold[1];
};

// Structure to describe request TLV 0x11 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_CDMARSSIDelta
{
   UINT16 mDelta;
};

// Structure to describe request TLV 0x12 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_CDMAECIOThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT16 mThreshold[1];
};

// Structure to describe request TLV 0x13 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_CDMAECIODelta
{
   UINT16 mDelta;
};

// Structure to describe request TLV 0x14 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_HDRRSSIThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT16 mThreshold[1];
};

// Structure to describe request TLV 0x15 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_HDRRSSIDelta
{
   UINT16 mDelta;
};

// Structure to describe request TLV 0x16 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_HDRECIOThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT16 mThreshold[1];
};

// Structure to describe request TLV 0x17 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_HDRECIODelta
{
   UINT16 mDelta;
};

// Structure to describe request TLV 0x18 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_HDRSINRThresholdList
{
   UINT8 mSINRThresholdCount;

   // This array must be the size specified by mSINRThresholdCount
   // eQMINASSINRLevels mSINRThreshold[1];
};

// Structure to describe request TLV 0x19 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_HDRSINRDelta
{
   UINT16 mDelta;
};

// Structure to describe request TLV 0x1A for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_HDRIOThreshold
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT16 mThreshold[1];
};

// Structure to describe request TLV 0x1B for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_HDRIODelta
{
   UINT16 mDelta;
};

// Structure to describe request TLV 0x1C for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_GSMRSSIThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT16 mThreshold[1];
};

// Structure to describe request TLV 0x1D for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_GSMRSSIDelta
{
   UINT16 mDelta;
};

// Structure to describe request TLV 0x1E for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_WCDMARSSIThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT16 mThreshold[1];
};

// Structure to describe request TLV 0x1F for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_WCDMARSSIDelta
{
   UINT16 mDelta;
};

// Structure to describe request TLV 0x20 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_WCDMAECIOThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT16 mThreshold[1];
};

// Structure to describe request TLV 0x21 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_WCDMAECIODelta
{
   UINT16 mDelta;
};

// Structure to describe request TLV 0x22 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_LTERSSIThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT16 mThreshold[1];
};

// Structure to describe request TLV 0x23 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_LTERSSIDelta
{
   UINT16 mDelta;
};

// Structure to describe request TLV 0x24 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_LTESNRThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT16 mThreshold[1];
};

// Structure to describe request TLV 0x25 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_LTESNRDelta
{
   UINT16 mDelta;
};

// Structure to describe request TLV 0x26 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_LTERSRQThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT16 mThreshold[1];
};

// Structure to describe request TLV 0x27 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_LTERSRQDelta
{
   UINT16 mDelta;
};

// Structure to describe request TLV 0x28 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_LTERSRPThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT16 mThreshold[1];
};

// Structure to describe request TLV 0x29 for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_LTERSRPDelta
{
   UINT16 mDelta;
};

// Structure to describe request TLV 0x2A for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_LTESignalReportConfig
{
   eQMINASReportRate mReportRate;
   eQMINASAveragePeriod mAveragePeriod;
};

// Structure to describe request TLV 0x2B for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_TDSCDMARSCPThresholdList
{
   UINT8 mThresholdCount;

   // This array must be the size specified by mThresholdCount
   // INT16 mThreshold[1];
};

// Structure to describe request TLV 0x2C for NASConfigureSignalInfo2()
struct sNASConfigureSignalInfo2Request_TDSCDMARSCPDelta
{
   UINT16 mDelta;
};

// Structure to describe response TLV 0x10 for NASGetTDSCDMACellInfo()
struct sNASGetTDSCDMACellInfoResponse_CellInfo
{
   // Bitfield arrays are not possible in c, unrolling the array
   UINT8 mMobileCountryCode0:4;
   UINT8 mMobileCountryCode1:4;
   UINT8 mMobileCountryCode2:4;

   // Bitfield arrays are not possible in c, unrolling the array
   UINT8 mMobileNetworkCode0:4;
   UINT8 mMobileNetworkCode1:4;
   UINT8 mMobileNetworkCode2:4;

   INT8 mMNCIncludesPCSDigit;
   UINT16 mLocationAreaCode;
   UINT16 mAbsoluteRFChannelNumber;
   UINT32 mCellID;
   UINT8 mCellParameterID;
   UINT8 mPathLossdB;
   float mTimingAdvanceSeconds;
   float mRSCPdBm;
};

// Structure to describe response TLV 0x11 for NASGetTDSCDMACellInfo()
struct sNASGetTDSCDMACellInfoResponse_NeighborCellInfo
{
   UINT8 mNeighborCellCount;

   struct sNeighborCell
   {
      UINT16 mAbsoluteRFChannelNumber;
      UINT8 mCellParameterID;
      float mRSCPdBm;
   };

   // This array must be the size specified by mNeighborCellCount
   // sNeighborCell mNeighborCells[1];
};

// Structure to describe request TLV 0x10 for WMSSetEventReport()
struct sWMSSetEventReportRequest_NewMTMessageIndicator
{
   INT8 mReportNewMTMessages;
};

// Structure to describe indication TLV 0x10 for WMS EventReport
struct sWMSEventReportIndication_ReceivedMTMessage
{
   eQMIWMSStorageTypes mStorageType;
   UINT32 mStorageIndex;
};

// Structure to describe indication TLV 0x11 for WMS EventReport
struct sWMSEventReportIndication_TransferRouteMTMessage
{
   INT8 mACKRequired;
   UINT32 mTransactionID;
   eQMIWMSMessageFormats mMessageFormat;
   UINT16 mRawMessageLength;

   // This array must be the size specified by mRawMessageLength
   // UINT8 mRawMessage[1];
};

// Structure to describe indication TLV 0x12 for WMS EventReport
struct sWMSEventReportIndication_MessageMode
{
   eQMIWMSMessageProtocols mMode;
};

// Structure to describe indication TLV 0x13 for WMS EventReport
struct sWMSEventReportIndication_ReceivedETWSMessage
{
   eQMIWMSNotificationType mNotificationType;
   UINT16 mRawMessageLength;

   // This array must be the size specified by mRawMessageLength
   // UINT8 mRawMessage[1];
};

// Structure to describe indication TLV 0x14 for WMS EventReport
struct sWMSEventReportIndication_ReceivedETWSPLMNInfo
{
   UINT16 mMobileCountryCode;
   UINT16 mMobileNetworkCode;
};

// Structure to describe indication TLV 0x15 for WMS EventReport
struct sWMSEventReportIndication_ReceivedSMSCAddress
{
   UINT8 mSMSCAddressLength;

   // This array must be the size specified by mSMSCAddressLength
   // char mSMSCAddress[1];
};

// Structure to describe indication TLV 0x16 for WMS EventReport
struct sWMSEventReportIndication_SMSOnIMS
{
   INT8 mMessageReceivedFromIMS;
};

// Structure to describe request TLV 0x01 for WMSRawSend()
struct sWMSRawSendRequest_MessageData
{
   eQMIWMSMessageFormats mMessageFormat;
   UINT16 mRawMessageLength;

   // This array must be the size specified by mRawMessageLength
   // UINT8 mRawMessage[1];
};

// Structure to describe request TLV 0x10 for WMSRawSend()
struct sWMSRawSendRequest_ForceOnDC
{
   INT8 mForceSendOnDC;
   eQMIWMSCDMAServiceOptions mServiceOption;
};

// Structure to describe request TLV 0x11 for WMSRawSend()
struct sWMSRawSendRequest_FollowOnDC
{
   INT8 mDoNotDisconnectDC;
};

// Structure to describe request TLV 0x12 for WMSRawSend()
struct sWMSRawSendRequest_LinkControl
{
   UINT8 mLinkTimerInSeconds;
};

// Structure to describe request TLV 0x13 for WMSRawSend()
struct sWMSRawSendRequest_SMSOnIMS
{
   INT8 mMessageToBeSentOnIMS;
};

// Structure to describe request TLV 0x14 for WMSRawSend()
struct sWMSRawSendRequest_RetryMessage
{
   INT8 mMessageIsARetry;
};

// Structure to describe request TLV 0x15 for WMSRawSend()
struct sWMSRawSendRequest_RetryMessageID
{
   UINT32 mMessageRetryID;
};

// Structure to describe response TLV 0x01 for WMSRawSend()
struct sWMSRawSendResponse_MessageID
{
   UINT16 mMessageID;
};

// Structure to describe response TLV 0x10 for WMSRawSend()
struct sWMSRawSendResponse_CauseCode
{
   UINT16 mCauseCode;
};

// Structure to describe response TLV 0x11 for WMSRawSend()
struct sWMSRawSendResponse_ErrorClass
{
   eQMIWMSErrorClasses mErrorClass;
};

// Structure to describe response TLV 0x12 for WMSRawSend()
struct sWMSRawSendResponse_CauseInfo
{
   UINT16 mGSMWCDMARPCause;
   UINT8 mGSMWCDMATPCause;
};

// Structure to describe response TLV 0x13 for WMSRawSend()
struct sWMSRawSendResponse_MessageDeliveryFailureType
{
   eQMIWMSMessageDeliveryFailureType mMessageDeliveryFailureType;
};

// Structure to describe response TLV 0x14 for WMSRawSend()
struct sWMSRawSendResponse_MessageDeliveryFailureCause
{
   eQMIWMSDeliveryFailures mDeliveryFailureCause;
};

// Structure to describe response TLV 0x15 for WMSRawSend()
struct sWMSRawSendResponse_CallControlModifiedInfo
{
   UINT8 mAlphaIDLength;

   // This array must be the size specified by mAlphaIDLength
   // UINT8 mAlphaID[1];
};

// Structure to describe request TLV 0x01 for WMSRawWrite()
struct sWMSRawWriteRequest_MessageData
{
   eQMIWMSStorageTypes mStorageType;
   eQMIWMSMessageFormats mMessageFormat;
   UINT16 mRawMessageLength;

   // This array must be the size specified by mRawMessageLength
   // UINT8 mRawMessage[1];
};

// Structure to describe response TLV 0x01 for WMSRawWrite()
struct sWMSRawWriteResponse_MessageIndex
{
   UINT32 mStorageIndex;
};

// Structure to describe request TLV 0x01 for WMSRawRead()
struct sWMSRawReadRequest_MessageIndex
{
   eQMIWMSStorageTypes mStorageType;
   UINT32 mStorageIndex;
};

// Structure to describe request TLV 0x10 for WMSRawRead()
struct sWMSRawReadRequest_MessageMode
{
   eQMIWMSMessageProtocols mMode;
};

// Structure to describe request TLV 0x11 for WMSRawRead()
struct sWMSRawReadRequest_SMSOnIMS
{
   INT8 mMessageToBeReadFromIMS;
};

// Structure to describe response TLV 0x01 for WMSRawRead()
struct sWMSRawReadResponse_MessageData
{
   eQMIWMSMessageTags mMessageTag;
   eQMIWMSMessageFormats mMessageFormat;
   UINT16 mRawMessageLength;

   // This array must be the size specified by mRawMessageLength
   // UINT8 mRawMessage[1];
};

// Structure to describe request TLV 0x01 for WMSModifyTag()
struct sWMSModifyTagRequest_MessageTag
{
   eQMIWMSStorageTypes mStorageType;
   UINT32 mStorageIndex;
   eQMIWMSMessageTags mMessageTag;
};

// Structure to describe request TLV 0x10 for WMSModifyTag()
struct sWMSModifyTagRequest_MessageMode
{
   eQMIWMSMessageProtocols mMode;
};

// Structure to describe request TLV 0x01 for WMSDelete()
struct sWMSDeleteRequest_MemoryStorage
{
   eQMIWMSStorageTypes mStorageType;
};

// Structure to describe request TLV 0x10 for WMSDelete()
struct sWMSDeleteRequest_MessageIndex
{
   UINT32 mStorageIndex;
};

// Structure to describe request TLV 0x11 for WMSDelete()
struct sWMSDeleteRequest_MessageTag
{
   eQMIWMSMessageTags mMessageTag;
};

// Structure to describe request TLV 0x12 for WMSDelete()
struct sWMSDeleteRequest_MessageMode
{
   eQMIWMSMessageProtocols mMode;
};

// Structure to describe response TLV 0x01 for WMSGetMessageProtocol()
struct sWMSGetMessageProtocolResponse_MessageProtocol
{
   eQMIWMSMessageProtocols mMode;
};

// Structure to describe request TLV 0x01 for WMSListMessages()
struct sWMSListMessagesRequest_MemoryStorage
{
   eQMIWMSStorageTypes mStorageType;
};

// Structure to describe request TLV 0x10 for WMSListMessages()
struct sWMSListMessagesRequest_MessageTag
{
   eQMIWMSMessageTags mMessageTag;
};

// Structure to describe request TLV 0x11 for WMSListMessages()
struct sWMSListMessagesRequest_MessageMode
{
   eQMIWMSMessageProtocols mMode;
};

// Structure to describe response TLV 0x01 for WMSListMessages()
struct sWMSListMessagesResponse_MessageList
{
   UINT32 mNumberOfMessages;

   struct sMessage
   {
      UINT32 mStorageIndex;
      eQMIWMSMessageTags mMessageTag;
   };

   // This array must be the size specified by mNumberOfMessages
   // sMessage mMessages[1];
};

// Structure to describe request TLV 0x01 for WMSSetRoutes()
struct sWMSSetRoutesRequest_RouteList
{
   UINT16 mNumberOfRoutes;

   struct sRoute
   {
      eQMIWMSMessageTypes mMessageType;
      eQMIWMSMessageClasses mMessageClass;
      eQMIWMSStorageTypes mStorageType;
      eQMIWMSReceiptActions mReceiptAction;
   };

   // This array must be the size specified by mNumberOfRoutes
   // sRoute mRoutes[1];
};

// Structure to describe request TLV 0x10 for WMSSetRoutes()
struct sWMSSetRoutesRequest_TransferStatusReport
{
   INT8 mTransferStatusReports;
};

// Structure to describe response TLV 0x01 for WMSGetRoutes()
struct sWMSGetRoutesResponse_RouteList
{
   UINT16 mNumberOfRoutes;

   struct sRoute
   {
      eQMIWMSMessageTypes mMessageType;
      eQMIWMSMessageClasses mMessageClass;
      eQMIWMSStorageTypes mStorageType;
      eQMIWMSRouteValues mRouteValue;
   };

   // This array must be the size specified by mNumberOfRoutes
   // sRoute mRoutes[1];
};

// Structure to describe response TLV 0x10 for WMSGetRoutes()
struct sWMSGetRoutesResponse_TransferStatusReport
{
   INT8 mTransferStatusReports;
};

// Structure to describe response TLV 0x01 for WMSGetSMSCAddress()
struct sWMSGetSMSCAddressResponse_Address
{
   char mSMSCAddressType[3];
   UINT8 mSMSCAddressLength;

   // This array must be the size specified by mSMSCAddressLength
   // char mSMSCAddress[1];
};

// Structure to describe request TLV 0x01 for WMSSetSMSCAddress()
struct sWMSSetSMSCAddressRequest_Address
{
   // String is variable length, but must be size of the container
   // char mSMSCAddress[1];
};

// Structure to describe request TLV 0x10 for WMSSetSMSCAddress()
struct sWMSSetSMSCAddressRequest_AddressType
{
   // String is variable length, but must be size of the container
   // char mSMSCAddressType[1];
};

// Structure to describe request TLV 0x01 for WMSGetStorageMaxSize()
struct sWMSGetStorageMaxSizeRequest_MemoryStorage
{
   eQMIWMSStorageTypes mStorageType;
};

// Structure to describe request TLV 0x10 for WMSGetStorageMaxSize()
struct sWMSGetStorageMaxSizeRequest_MessageMode
{
   eQMIWMSMessageProtocols mMode;
};

// Structure to describe response TLV 0x01 for WMSGetStorageMaxSize()
struct sWMSGetStorageMaxSizeResponse_MaxSize
{
   UINT32 mMaxStorageSizeInMessages;
};

// Structure to describe response TLV 0x10 for WMSGetStorageMaxSize()
struct sWMSGetStorageMaxSizeResponse_AvailableSize
{
   UINT32 mFreeStorageSizeInMessages;
};

// Structure to describe request TLV 0x01 for WMSSendACK()
struct sWMSSendACKRequest_ACK
{
   UINT32 mTransactionID;
   eQMIWMSMessageProtocols mMode;
   INT8 mProcessedSuccessfully;
};

// Structure to describe request TLV 0x10 for WMSSendACK()
struct sWMSSendACKRequest_3GPP2FailureInfo
{
   eQMIWMSErrorClasses2 mErrorClass;
   UINT8 mTransportLayerStatus;
};

// Structure to describe request TLV 0x11 for WMSSendACK()
struct sWMSSendACKRequest_3GPPFailureInfo
{
   UINT8 mGSMWCDMARPCause;
   UINT8 mGSMWCDMATPCause;
};

// Structure to describe request TLV 0x12 for WMSSendACK()
struct sWMSSendACKRequest_SMSOnIMS
{
   INT8 mACKToBeSentOnIMS;
};

// Structure to describe response TLV 0x10 for WMSSendACK()
struct sWMSSendACKResponse_ACKFailureCause
{
   eQMIWMSACKFailureCause mACKFailureCause;
};

// Structure to describe request TLV 0x01 for WMSSetRetryPeriod()
struct sWMSSetRetryPeriodRequest_Period
{
   UINT32 mRetryPeriodInSeconds;
};

// Structure to describe request TLV 0x01 for WMSSetRetryInterval()
struct sWMSSetRetryIntervalRequest_Interval
{
   UINT32 mRetryIntervalInSeconds;
};

// Structure to describe request TLV 0x01 for WMSSetDCDisconnectTimer()
struct sWMSSetDCDisconnectTimerRequest_Timer
{
   UINT32 mDCDisconnectTimerInSeconds;
};

// Structure to describe request TLV 0x01 for WMSSetMemoryStatus()
struct sWMSSetMemoryStatusRequest_Status
{
   INT8 mMemoryIsAvailable;
};

// Structure to describe request TLV 0x01 for WMSSetBroadcastActivation()
struct sWMSSetBroadcastActivationRequest_BCInfo
{
   eQMIWMSMessageProtocols mMode;
   INT8 mActivateBroadcast;
};

// Structure to describe request TLV 0x01 for WMSSetBroadcastConfig()
struct sWMSSetBroadcastConfigRequest_Mode
{
   eQMIWMSMessageProtocols mMode;
};

// Structure to describe request TLV 0x10 for WMSSetBroadcastConfig()
struct sWMSSetBroadcastConfigRequest_3GPPInfo
{
   UINT16 mNumberOfInstances;

   struct sInstance
   {
      UINT16 mMessageIDStart;
      UINT16 mMessageIDEnd;
      INT8 mSelected;
   };

   // This array must be the size specified by mNumberOfInstances
   // sInstance mInstances[1];
};

// Structure to describe request TLV 0x11 for WMSSetBroadcastConfig()
struct sWMSSetBroadcastConfigRequest_3GPP2Info
{
   UINT16 mNumberOfInstances;

   struct sInstance
   {
      UINT16 mServiceCategory;
      eQMIWMSLanguage mLanguage;
      INT8 mSelected;
   };

   // This array must be the size specified by mNumberOfInstances
   // sInstance mInstances[1];
};

// Structure to describe request TLV 0x01 for WMSGetBroadcastConfig()
struct sWMSGetBroadcastConfigRequest_Mode
{
   eQMIWMSMessageProtocols mMode;
};

// Structure to describe response TLV 0x10 for WMSGetBroadcastConfig()
struct sWMSGetBroadcastConfigResponse_3GPPInfo
{
   INT8 mActivated;
   UINT16 mNumberOfInstances;

   struct sInstance
   {
      UINT16 mMessageIDStart;
      UINT16 mMessageIDEnd;
      INT8 mSelected;
   };

   // This array must be the size specified by mNumberOfInstances
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x11 for WMSGetBroadcastConfig()
struct sWMSGetBroadcastConfigResponse_3GPP2Info
{
   INT8 mActivated;
   UINT16 mNumberOfInstances;

   struct sInstance
   {
      UINT16 mServiceCategory;
      eQMIWMSLanguage mLanguage;
      INT8 mSelected;
   };

   // This array must be the size specified by mNumberOfInstances
   // sInstance mInstances[1];
};

// Structure to describe indication TLV 0x01 for WMS MemoryFullIndication
struct sWMSMemoryFullIndication_Info
{
   eQMIWMSStorageTypes mStorageType;
   eQMIWMSMessageProtocols mMode;
};

// Structure to describe response TLV 0x01 for WMSGetDomainPreference()
struct sWMSGetDomainPreferenceResponse_Pref
{
   eQMIWMSGSMWCDMADomains mDomainPreference;
};

// Structure to describe request TLV 0x01 for WMSSetDomainPreference()
struct sWMSSetDomainPreferenceRequest_Pref
{
   eQMIWMSGSMWCDMADomains mDomainPreference;
};

// Structure to describe request TLV 0x01 for WMSSendFromMemoryStore()
struct sWMSSendFromMemoryStoreRequest_Info
{
   eQMIWMSStorageTypes mStorageType;
   UINT32 mStorageIndex;
   eQMIWMSMessageProtocols mMode;
};

// Structure to describe request TLV 0x10 for WMSSendFromMemoryStore()
struct sWMSSendFromMemoryStoreRequest_SMSOnIMS
{
   INT8 mMessageToBeSentOnIMS;
};

// Structure to describe response TLV 0x10 for WMSSendFromMemoryStore()
struct sWMSSendFromMemoryStoreResponse_MessageID
{
   UINT16 mMessageID;
};

// Structure to describe response TLV 0x11 for WMSSendFromMemoryStore()
struct sWMSSendFromMemoryStoreResponse_CauseCode
{
   UINT16 mCauseCode;
};

// Structure to describe response TLV 0x12 for WMSSendFromMemoryStore()
struct sWMSSendFromMemoryStoreResponse_ErrorClass
{
   eQMIWMSErrorClasses mErrorClass;
};

// Structure to describe response TLV 0x13 for WMSSendFromMemoryStore()
struct sWMSSendFromMemoryStoreResponse_CauseInfo
{
   UINT16 mGSMWCDMARPCause;
   UINT8 mGSMWCDMATPCause;
};

// Structure to describe response TLV 0x14 for WMSSendFromMemoryStore()
struct sWMSSendFromMemoryStoreResponse_MessageDeliveryFailureType
{
   eQMIWMSMessageDeliveryFailureType mMessageDeliveryFailureType;
};

// Structure to describe response TLV 0x01 for WMSGetWaitingMessage()
struct sWMSGetWaitingMessageResponse_WaitingMessageInfo
{
   UINT8 mNumberOfWaitingMessages;
   eQMIWMSWaitingMessageType mWaitingMessageType;
   INT8 mActiveIndication;
   UINT8 mMessageCount;
};

// Structure to describe indication TLV 0x01 for WMS WaitingMessageIndication
struct sWMSWaitingMessageIndication_WaitingMessageInfo
{
   UINT8 mNumberOfWaitingMessages;
   eQMIWMSWaitingMessageType mWaitingMessageType;
   INT8 mActiveIndication;
   UINT8 mMessageCount;
};

// Structure to describe request TLV 0x01 for WMSSetPrimaryClient()
struct sWMSSetPrimaryClientRequest_PrimaryClientInfo
{
   INT8 mPrimaryClient;
};

// Structure to describe indication TLV 0x01 for WMS SMSCAddressIndication
struct sWMSSMSCAddressIndication_Address
{
   char mSMSCAddressType[3];
   UINT8 mSMSCAddressLength;

   // This array must be the size specified by mSMSCAddressLength
   // char mSMSCAddress[1];
};

// Structure to describe request TLV 0x10 for WMSIndicatorRegistration()
struct sWMSIndicatorRegistrationRequest_TransportLayerInfoEvents
{
   INT8 mTransportLayerInfoEvents;
};

// Structure to describe request TLV 0x11 for WMSIndicatorRegistration()
struct sWMSIndicatorRegistrationRequest_NetworkRegistrationInfoEvents
{
   INT8 mNetworkRegistrationInfoEvents;
};

// Structure to describe request TLV 0x12 for WMSIndicatorRegistration()
struct sWMSIndicatorRegistrationRequest_CallStatusInfoEvents
{
   INT8 mCallStatusInfoEvents;
};

// Structure to describe request TLV 0x13 for WMSIndicatorRegistration()
struct sWMSIndicatorRegistrationRequest_ServiceReadyEvents
{
   INT8 mServiceReadyEvents;
};

// Structure to describe request TLV 0x14 for WMSIndicatorRegistration()
struct sWMSIndicatorRegistrationRequest_BroadcastConfigEvents
{
   INT8 mBroadcastConfigEvents;
};

// Structure to describe response TLV 0x10 for WMSGetTransportInfoLayer()
struct sWMSGetTransportInfoLayerResponse_TransportLayerRegistrationInfo
{
   INT8 mRegistered;
};

// Structure to describe response TLV 0x11 for WMSGetTransportLayerInfo()
struct sWMSGetTransportLayerInfoResponse_TransportLayerInfo
{
   eQMIWMSTransportType mTransportType;
   eQMIWMSTransportCapability mTransportCapability;
};

// Structure to describe indication TLV 0x01 for WMS TransportLayerInfoIndication
struct sWMSTransportLayerInfoIndication_TransportLayerRegInfo
{
   INT8 mRegistered;
};

// Structure to describe indication TLV 0x10 for WMS TransportLayerInfoIndication
struct sWMSTransportLayerInfoIndication_TransportLayerInfo
{
   eQMIWMSTransportType mTransportType;
   eQMIWMSTransportCapability mTransportCapability;
};

// Structure to describe response TLV 0x10 for WMSGetNetworkRegistrationInfo()
struct sWMSGetNetworkRegistrationInfoResponse_NetworkRegistrationInfo
{
   eQMIWMSNetworkRegistrationStatus mNetworkRegistrationStatus;
};

// Structure to describe indication TLV 0x01 for WMS NetworkRegistrationInfoIndication
struct sWMSNetworkRegistrationInfoIndication_NetworkRegistrationInfo
{
   eQMIWMSNetworkRegistrationStatus mNetworkRegistrationStatus;
};

// Structure to describe request TLV 0x01 for WMSBindSubscription()
struct sWMSBindSubscriptionRequest_SubscriptionType
{
   eQMIWMSSubscriptionType mSubscriptionType;
};

// Structure to describe response TLV 0x10 for WMSGetIndicatorRegistration()
struct sWMSGetIndicatorRegistrationResponse_TransportLayerInfoEvents
{
   INT8 mTransportLayerInfoEvents;
};

// Structure to describe response TLV 0x11 for WMSGetIndicatorRegistration()
struct sWMSGetIndicatorRegistrationResponse_NetworkRegistrationInfoEvents
{
   INT8 mNetworkRegistrationInfoEvents;
};

// Structure to describe response TLV 0x12 for WMSGetIndicatorRegistration()
struct sWMSGetIndicatorRegistrationResponse_CallStatusInfoEvents
{
   INT8 mCallStatusInfoEvents;
};

// Structure to describe response TLV 0x13 for WMSGetIndicatorRegistration()
struct sWMSGetIndicatorRegistrationResponse_ServiceReadyEvents
{
   INT8 mServiceReadyEvents;
};

// Structure to describe response TLV 0x14 for WMSGetIndicatorRegistration()
struct sWMSGetIndicatorRegistrationResponse_BroadcastConfigEvents
{
   INT8 mBroadcastConfigEvents;
};

// Structure to describe request TLV 0x01 for WMSGetSMSParameters()
struct sWMSGetSMSParametersRequest_MessageMode
{
   eQMIWMSSMSMessageMode mSMSMessageMode;
};

// Structure to describe response TLV 0x10 for WMSGetSMSParameters()
struct sWMSGetSMSParametersResponse_DestinationAddress
{
   UINT8 mDestinationAddressLength;

   // This array must be the size specified by mDestinationAddressLength
   // UINT8 mDestinationAddress[1];
};

// Structure to describe response TLV 0x11 for WMSGetSMSParameters()
struct sWMSGetSMSParametersResponse_ProtocolIdentifierData
{
   eQMIWMSProtocolIdentifierData mProtocolIdentifierData;
};

// Structure to describe response TLV 0x12 for WMSGetSMSParameters()
struct sWMSGetSMSParametersResponse_DataCodingScheme
{
   UINT8 mDataCodingScheme;
};

// Structure to describe response TLV 0x13 for WMSGetSMSParameters()
struct sWMSGetSMSParametersResponse_ValidityPeriod
{
   UINT8 mValidityPeriod;
};

// Structure to describe request TLV 0x01 for WMSSetSMSParameters()
struct sWMSSetSMSParametersRequest_MessageMode
{
   eQMIWMSSMSMessageMode mSMSMessageMode;
};

// Structure to describe request TLV 0x10 for WMSSetSMSParameters()
struct sWMSSetSMSParametersRequest_DestinationAddress
{
   UINT8 mDestinationAddressLength;

   // This array must be the size specified by mDestinationAddressLength
   // UINT8 mDestinationAddress[1];
};

// Structure to describe request TLV 0x11 for WMSSetSMSParameters()
struct sWMSSetSMSParametersRequest_ProtocolIdentifierData
{
   eQMIWMSProtocolIdentifierData mProtocolIdentifierData;
};

// Structure to describe request TLV 0x12 for WMSSetSMSParameters()
struct sWMSSetSMSParametersRequest_DataCodingScheme
{
   UINT8 mDataCodingScheme;
};

// Structure to describe request TLV 0x13 for WMSSetSMSParameters()
struct sWMSSetSMSParametersRequest_ValidityPeriod
{
   UINT8 mValidityPeriod;
};

// Structure to describe indication TLV 0x01 for WMS CallStatusIndication
struct sWMSCallStatusIndication_SMSCallStatusInfo
{
   eQMIWMSSMSCallStatus mSMSCallStatus;
};

// Structure to describe response TLV 0x10 for WMSGetDomainPreferenceConfig()
struct sWMSGetDomainPreferenceConfigResponse_LTEDomain
{
   eQMIWMSLTEDomains mLTEDomainPreference;
};

// Structure to describe response TLV 0x11 for WMSGetDomainPreferenceConfig()
struct sWMSGetDomainPreferenceConfigResponse_GWDomain
{
   eQMIWMSGSMWCDMADomains mDomainPreference;
};

// Structure to describe response TLV 0x10 for WMSSetDomainPreferenceConfig()
struct sWMSSetDomainPreferenceConfigResponse_LTEDomain
{
   eQMIWMSLTEDomains mLTEDomainPreference;
};

// Structure to describe response TLV 0x11 for WMSSetDomainPreferenceConfig()
struct sWMSSetDomainPreferenceConfigResponse_GWDomain
{
   eQMIWMSGSMWCDMADomains mDomainPreference;
};

// Structure to describe response TLV 0x10 for WMSSetDomainPreferenceConfig()
struct sWMSSetDomainPreferenceConfigResponse_LTEOutcome
{
   UINT16 mLTEDomainPreferenceOutcome;
};

// Structure to describe response TLV 0x11 for WMSSetDomainPreferenceConfig()
struct sWMSSetDomainPreferenceConfigResponse_GWOutcome
{
   UINT16 mGWDomainPreferenceOutcome;
};

// Structure to describe response TLV 0x10 for WMSGetRetryPeriod()
struct sWMSGetRetryPeriodResponse_RetryPeriod
{
   UINT32 mRetryPeriodInSeconds;
};

// Structure to describe response TLV 0x10 for WMSGetRetryInterval()
struct sWMSGetRetryIntervalResponse_RetryInterval
{
   UINT32 mRetryIntervalInSeconds;
};

// Structure to describe response TLV 0x10 for WMSGetDCDisconnectTimer()
struct sWMSGetDCDisconnectTimerResponse_DCDisconnectTimer
{
   UINT32 mDCDisconnectTimerInSeconds;
};

// Structure to describe response TLV 0x10 for WMSGetMemoryStatus()
struct sWMSGetMemoryStatusResponse_MemoryStatus
{
   INT8 mMemoryIsAvailable;
};

// Structure to describe response TLV 0x10 for WMSGetPrimaryClient()
struct sWMSGetPrimaryClientResponse_PrimaryClientInfo
{
   INT8 mPrimaryClient;
};

// Structure to describe response TLV 0x10 for WMSGetSubscriptionBinding()
struct sWMSGetSubscriptionBindingResponse_SubscriptionType
{
   eQMIWMSSubscriptionType mSubscriptionType;
};

// Structure to describe request TLV 0x01 for WMSAsyncRawSend()
struct sWMSAsyncRawSendRequest_MessageData
{
   eQMIWMSMessageFormats mMessageFormat;
   UINT16 mRawMessageLength;

   // This array must be the size specified by mRawMessageLength
   // UINT8 mRawMessage[1];
};

// Structure to describe request TLV 0x10 for WMSAsyncRawSend()
struct sWMSAsyncRawSendRequest_ForceOnDC
{
   INT8 mForceSendOnDC;
   eQMIWMSCDMAServiceOptions mServiceOption;
};

// Structure to describe request TLV 0x11 for WMSAsyncRawSend()
struct sWMSAsyncRawSendRequest_FollowOnDC
{
   INT8 mDoNotDisconnectDC;
};

// Structure to describe request TLV 0x12 for WMSAsyncRawSend()
struct sWMSAsyncRawSendRequest_LinkControl
{
   UINT8 mLinkTimerInSeconds;
};

// Structure to describe request TLV 0x13 for WMSAsyncRawSend()
struct sWMSAsyncRawSendRequest_SMSOnIMS
{
   INT8 mMessageToBeSentOnIMS;
};

// Structure to describe request TLV 0x14 for WMSAsyncRawSend()
struct sWMSAsyncRawSendRequest_RetryMessage
{
   INT8 mMessageIsARetry;
};

// Structure to describe request TLV 0x15 for WMSAsyncRawSend()
struct sWMSAsyncRawSendRequest_RetryMessageID
{
   UINT32 mMessageRetryID;
};

// Structure to describe request TLV 0x16 for WMSAsyncRawSend()
struct sWMSAsyncRawSendRequest_UserData
{
   UINT32 mUserData;
};

// Structure to describe indication TLV 0x01 for WMS AsyncRawSendIndication
struct sWMSAsyncRawSendIndication_Status
{
   eQMIErrors mQMIError;
};

// Structure to describe indication TLV 0x10 for WMS AsyncRawSendIndication
struct sWMSAsyncRawSendIndication_MessageID
{
   UINT16 mMessageID;
};

// Structure to describe indication TLV 0x11 for WMS AsyncRawSendIndication
struct sWMSAsyncRawSendIndication_CauseCode
{
   UINT16 mCauseCode;
};

// Structure to describe indication TLV 0x12 for WMS AsyncRawSendIndication
struct sWMSAsyncRawSendIndication_ErrorClass
{
   eQMIWMSErrorClasses mErrorClass;
};

// Structure to describe indication TLV 0x13 for WMS AsyncRawSendIndication
struct sWMSAsyncRawSendIndication_CauseInfo
{
   UINT16 mGSMWCDMARPCause;
   UINT8 mGSMWCDMATPCause;
};

// Structure to describe indication TLV 0x14 for WMS AsyncRawSendIndication
struct sWMSAsyncRawSendIndication_MessageDeliveryFailureType
{
   eQMIWMSMessageDeliveryFailureType mMessageDeliveryFailureType;
};

// Structure to describe indication TLV 0x15 for WMS AsyncRawSendIndication
struct sWMSAsyncRawSendIndication_MessageDeliveryFailureCause
{
   eQMIWMSDeliveryFailures mDeliveryFailureCause;
};

// Structure to describe indication TLV 0x16 for WMS AsyncRawSendIndication
struct sWMSAsyncRawSendIndication_CallControlModifiedInfo
{
   UINT8 mAlphaIDLength;

   // This array must be the size specified by mAlphaIDLength
   // UINT8 mAlphaID[1];
};

// Structure to describe indication TLV 0x17 for WMS AsyncRawSendIndication
struct sWMSAsyncRawSendIndication_UserData
{
   UINT32 mUserData;
};

// Structure to describe request TLV 0x01 for WMSAsyncSendACK()
struct sWMSAsyncSendACKRequest_ACK
{
   UINT32 mTransactionID;
   eQMIWMSMessageProtocols mMode;
   INT8 mProcessedSuccessfully;
};

// Structure to describe request TLV 0x10 for WMSAysncSendACK()
struct sWMSAysncSendACKRequest_3GPP2FailureInfo
{
   eQMIWMSErrorClasses2 mErrorClass;
   UINT8 mTransportLayerStatus;
};

// Structure to describe request TLV 0x11 for WMSAsyncSendACK()
struct sWMSAsyncSendACKRequest_3GPPFailureInfo
{
   UINT8 mGSMWCDMARPCause;
   UINT8 mGSMWCDMATPCause;
};

// Structure to describe request TLV 0x12 for WMSAsyncSendACK()
struct sWMSAsyncSendACKRequest_SMSOnIMS
{
   INT8 mACKToBeSentOnIMS;
};

// Structure to describe request TLV 0x13 for WMSAsyncSendACK()
struct sWMSAsyncSendACKRequest_UserData
{
   UINT32 mUserData;
};

// Structure to describe indication TLV 0x01 for WMS AsyncSendACKIndication
struct sWMSAsyncSendACKIndication_Status
{
   eQMIErrors mQMIError;
};

// Structure to describe indication TLV 0x10 for WMS AsyncSendACKIndication
struct sWMSAsyncSendACKIndication_ACKFailureCause
{
   eQMIWMSACKFailureCause mACKFailureCause;
};

// Structure to describe indication TLV 0x11 for WMS AsyncSendACKIndication
struct sWMSAsyncSendACKIndication_UserData
{
   UINT32 mUserData;
};

// Structure to describe request TLV 0x01 for WMSAsyncSendFromMemoryStore()
struct sWMSAsyncSendFromMemoryStoreRequest_Info
{
   eQMIWMSStorageTypes mStorageType;
   UINT32 mStorageIndex;
   eQMIWMSMessageProtocols mMode;
};

// Structure to describe request TLV 0x10 for WMSAsyncSendFromMemoryStore()
struct sWMSAsyncSendFromMemoryStoreRequest_SMSOnIMS
{
   INT8 mMessageToBeSentOnIMS;
};

// Structure to describe request TLV 0x11 for WMSAsyncSendFromMemoryStore()
struct sWMSAsyncSendFromMemoryStoreRequest_UserData
{
   UINT32 mUserData;
};

// Structure to describe indication TLV 0x01 for WMS AsyncSendFromMemoryStoreIndication
struct sWMSAsyncSendFromMemoryStoreIndication_Status
{
   eQMIErrors mQMIError;
};

// Structure to describe indication TLV 0x10 for WMS AsyncSendFromMemoryStoreIndication
struct sWMSAsyncSendFromMemoryStoreIndication_MessageID
{
   UINT16 mMessageID;
};

// Structure to describe indication TLV 0x11 for WMS AsyncSendFromMemoryStoreIndication
struct sWMSAsyncSendFromMemoryStoreIndication_CauseCode
{
   UINT16 mCauseCode;
};

// Structure to describe indication TLV 0x12 for WMS AsyncSendFromMemoryStoreIndication
struct sWMSAsyncSendFromMemoryStoreIndication_ErrorClass
{
   eQMIWMSErrorClasses mErrorClass;
};

// Structure to describe indication TLV 0x13 for WMS AsyncSendFromMemoryStoreIndication
struct sWMSAsyncSendFromMemoryStoreIndication_CauseInfo
{
   UINT16 mGSMWCDMARPCause;
   UINT8 mGSMWCDMATPCause;
};

// Structure to describe indication TLV 0x14 for WMS AsyncSendFromMemoryStoreIndication
struct sWMSAsyncSendFromMemoryStoreIndication_MessageDeliveryFailureType
{
   eQMIWMSMessageDeliveryFailureType mMessageDeliveryFailureType;
};

// Structure to describe indication TLV 0x15 for WMS AsyncSendFromMemoryStoreIndication
struct sWMSAsyncSendFromMemoryStoreIndication_MessageDeliveryFailureCause
{
   eQMIWMSDeliveryFailures mDeliveryFailureCause;
};

// Structure to describe indication TLV 0x16 for WMS AsyncSendFromMemoryStoreIndication
struct sWMSAsyncSendFromMemoryStoreIndication_CallControlModifiedInfo
{
   UINT8 mAlphaIDLength;

   // This array must be the size specified by mAlphaIDLength
   // UINT8 mAlphaID[1];
};

// Structure to describe indication TLV 0x17 for WMS AsyncSendFromMemoryStoreIndication
struct sWMSAsyncSendFromMemoryStoreIndication_UserData
{
   UINT32 mUserData;
};

// Structure to describe response TLV 0x10 for WMSGetServiceReadyStatus()
struct sWMSGetServiceReadyStatusResponse_ServiceReadyEvents
{
   INT8 mServiceReadyEvents;
};

// Structure to describe response TLV 0x11 for WMSGetServiceReadyStatus()
struct sWMSGetServiceReadyStatusResponse_ServiceReadyStatus
{
   eQMIWMSServiceReadyStatus mReadyStatus;
};

// Structure to describe indication TLV 0x01 for WMS ServiceReadyStatusIndication
struct sWMSServiceReadyStatusIndication_ServiceReadyStatus
{
   eQMIWMSServiceReadyStatus mReadyStatus;
};

// Structure to describe indication TLV 0x01 for WMS BroadcastConfigIndication
struct sWMSBroadcastConfigIndication_Mode
{
   eQMIWMSMessageProtocols mMode;
};

// Structure to describe indication TLV 0x10 for WMS BroadcastConfigIndication
struct sWMSBroadcastConfigIndication_3GPPInfo
{
   INT8 mActivated;
   UINT16 mNumberOfInstances;

   struct sInstance
   {
      UINT16 mMessageIDStart;
      UINT16 mMessageIDEnd;
      INT8 mSelected;
   };

   // This array must be the size specified by mNumberOfInstances
   // sInstance mInstances[1];
};

// Structure to describe indication TLV 0x11 for WMS BroadcastConfigIndication
struct sWMSBroadcastConfigIndication_3GPP2Info
{
   INT8 mActivated;
   UINT16 mNumberOfInstances;

   struct sInstance
   {
      UINT16 mServiceCategory;
      eQMIWMSLanguage mLanguage;
      INT8 mSelected;
   };

   // This array must be the size specified by mNumberOfInstances
   // sInstance mInstances[1];
};

// Structure to describe request TLV 0x10 for PDSSetEventReport()
struct sPDSSetEventReportRequest_NMEAIndicator
{
   INT8 mReportNMEASentences;
};

// Structure to describe request TLV 0x11 for PDSSetEventReport()
struct sPDSSetEventReportRequest_ModeIndicator
{
   INT8 mReportNMEASentencesPlusMode;
};

// Structure to describe request TLV 0x12 for PDSSetEventReport()
struct sPDSSetEventReportRequest_RawIndicator
{
   INT8 mReportRawPositionData;
};

// Structure to describe request TLV 0x13 for PDSSetEventReport()
struct sPDSSetEventReportRequest_XTRARequestIndicator
{
   INT8 mReportExternalXTRADataRequests;
};

// Structure to describe request TLV 0x14 for PDSSetEventReport()
struct sPDSSetEventReportRequest_TimeInjectionIndicator
{
   INT8 mReportExternalTimeInjections;
};

// Structure to describe request TLV 0x15 for PDSSetEventReport()
struct sPDSSetEventReportRequest_WiFiIndicator
{
   INT8 mReportExternalWiFiRequests;
};

// Structure to describe request TLV 0x16 for PDSSetEventReport()
struct sPDSSetEventReportRequest_SatelliteIndicator
{
   INT8 mReportSatelliteInfo;
};

// Structure to describe request TLV 0x17 for PDSSetEventReport()
struct sPDSSetEventReportRequest_VXNetworkIndicator
{
   INT8 mReportVXNetworkInitiatedPrompts;
};

// Structure to describe request TLV 0x18 for PDSSetEventReport()
struct sPDSSetEventReportRequest_SUPLNetworkIndicator
{
   INT8 mReportSUPLNetworkInitiatedPrompts;
};

// Structure to describe request TLV 0x19 for PDSSetEventReport()
struct sPDSSetEventReportRequest_UMTSCPNetworkIndicator
{
   INT8 mReportUMTSCPNetworkInitiatedPrompts;
};

// Structure to describe request TLV 0x1A for PDSSetEventReport()
struct sPDSSetEventReportRequest_PDSCommIndicator
{
   INT8 mReportPDSCommEvents;
};

// Structure to describe request TLV 0x1B for PDSSetEventReport()
struct sPDSSetEventReportRequest_AccelerometerDataIndicator
{
   INT8 mReportAccelerometerDataStatus;
};

// Structure to describe request TLV 0x1C for PDSSetEventReport()
struct sPDSSetEventReportRequest_GyroDataIndicator
{
   INT8 mReportGyroDataStatus;
};

// Structure to describe request TLV 0x1D for PDSSetEventReport()
struct sPDSSetEventReportRequest_TimeSyncIndication
{
   INT8 mReportTimeSyncRequest;
};

// Structure to describe request TLV 0x1E for PDSSetEventReport()
struct sPDSSetEventReportRequest_PositionReliablilityIndicator
{
   INT8 mReportPositionReliability;
};

// Structure to describe request TLV 0x1F for PDSSetEventReport()
struct sPDSSetEventReportRequest_SensorDataUsageIndicator
{
   INT8 mReportSensorDataUsage;
};

// Structure to describe request TLV 0x20 for PDSSetEventReport()
struct sPDSSetEventReportRequest_TimeSourceInformationIndicator
{
   INT8 mReportTimeSourceInformation;
};

// Structure to describe request TLV 0x21 for PDSSetEventReport()
struct sPDSSetEventReportRequest_HeadingUncertaintyIndicator
{
   INT8 mReportHeadingUncertaintyInformation;
};

// Structure to describe request TLV 0x22 for PDSSetEventReport()
struct sPDSSetEventReportRequest_NMEADebugStringIndicator
{
   INT8 mReportNMEADebugStrings;
};

// Structure to describe request TLV 0x23 for PDSSetEventReport()
struct sPDSSetEventReportRequest_ExternalXTRADataIndicator
{
   INT8 mReportExtendedXTRAData;
};

// Structure to describe request TLV 0x24 for PDSSetEventReport()
struct sPDSSetEventReportRequest_ServiceResetStatus
{
   INT8 mReportServiceResetStatus;
};

// Structure to describe indication TLV 0x10 for PDS EventReport
struct sPDSEventReportIndication_NMEASentence
{
   // String is variable length, but must be size of the container
   // char mNMEASentence[1];
};

// Structure to describe indication TLV 0x11 for PDS EventReport
struct sPDSEventReportIndication_NMEASentencePlusMode
{
   eQMIPDSNMEASentenceOperatingModes mNMEASentenceOperatingMode;
   UINT16 mNMEASentenceLength;

   // This array must be the size specified by mNMEASentenceLength
   // char mNMEASentence[1];
};

// Structure to describe indication TLV 0x12 for PDS EventReport
struct sPDSEventReportIndication_PositionSessionStatus
{
   eQMIPDSSessionStatus mSessionStatus;
};

// Structure to describe indication TLV 0x13 for PDS EventReport
struct sPDSEventReportIndication_ParsedPositionData
{
   bool mTimestampCalendarValid:1;
   bool mTimestampUTCValid:1;
   bool mLeapSecondsValid:1;
   bool mTimeUncertaintyValid:1;
   bool mLatitudeValid:1;
   bool mLongitudeValid:1;
   bool mEllipsoidAltitudeValid:1;
   bool mMeanSeaLevelAltitudeValid:1;
   bool mHorizontalSpeedValid:1;
   bool mVerticalSpeedValid:1;
   bool mHeadingValid:1;
   bool mHorizontalUncertaintyCircularValid:1;
   bool mHorizontalUncertaintyEllipseSemiMajorValid:1;
   bool mHorizontalUncertaintyEllipseSemiMinorValid:1;
   bool mHorizontalUncertaintyEllipseOrientAzimuthValid:1;
   bool mVerticalUncertaintyValid:1;
   bool mHorizontalVelocityUncertaintyValid:1;
   bool mVerticalVelocityUncertaintyValid:1;
   bool mHorizontalConfidenceValid:1;
   bool mPositionDOPValid:1;
   bool mHorizontalDOPValid:1;
   bool mVerticalDOPValid:1;
   bool mOperatingModeUsedValid:1;

   // Padding out 9 bits
   UINT8 mReserved1:1;
   UINT8 mReserved2;

   UINT16 mCalendarYear;
   eQMIPDSCalendarMonths mCalendarMonth;
   eQMIPDSCalendarDays mCalendarDay;
   UINT8 mCalendarDayOfMonth;
   UINT8 mCalendarHour;
   UINT8 mCalendarMinute;
   UINT8 mCalendarSecond;
   UINT16 mCalendarMillisecond;
   UINT8 mCalendarLeapSeconds;
   UINT64 mUTCTimestamp;
   UINT32 mUTCTimestampUncertainty;
   double mLatitude;
   double mLongitude;
   float mEllipsoidAltitude;
   float mMeanSeaLevelAltitude;
   float mHorizontalSpeed;
   float mVerticalSpeed;
   float mHeading;
   float mHorizontalUncertaintyCircular;
   float mHorizontalUncertaintyEllipseSemiMajor;
   float mHorizontalUncertaintyEllipseSemiMinor;
   float mHorizontalUncertaintyEllipseOrientAzimuth;
   float mVerticalUncertainty;
   float mHorizontalVelocityUncertainty;
   float mVerticalVelocityUncertainty;
   UINT8 mHorizontalConfidence;
   float mPositionDOP;
   float mHorizontalDOP;
   float mVerticalDOP;
   eQMIPDSNMEASentenceOperatingModes mOperatingMode;
};

// Structure to describe indication TLV 0x14 for PDS EventReport
struct sPDSEventReportIndication_ExternalXTRARequest
{
   UINT16 mMaximumFileSize;
   UINT8 mURLRecordCount;

   struct sURL
   {
      UINT8 mURLLength;
   
      // This array must be the size specified by mURLLength
      // char mURL[1];
   };

   // This array must be the size specified by mURLRecordCount
   // sURL mURLs[1];
};

// Structure to describe indication TLV 0x15 for PDS EventReport
struct sPDSEventReportIndication_ExternalTimeInjectionRequest
{
   UINT32 mDelayThresholdMilliseconds;
   UINT8 mURLRecordCount;

   struct sURL
   {
      UINT8 mURLLength;
   
      // This array must be the size specified by mURLLength
      // char mURL[1];
   };

   // This array must be the size specified by mURLRecordCount
   // sURL mURLs[1];
};

// Structure to describe indication TLV 0x16 for PDS EventReport
struct sPDSEventReportIndication_ExternalWiFiPositionRequest
{
   eQMIPDSWiFiRequestTypes mWiFiRequestType;
   UINT16 mWiFiRequestTimeBetweenFixesMilliseconds;
};

// Structure to describe indication TLV 0x17 for PDS EventReport
struct sPDSEventReportIndication_SatelliteInfo
{
   bool mIonoValid:1;
   bool mSatelliteCountValid:1;
   bool mSatelliteListValid:1;

   // Padding out 29 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2[3];

   INT8 mIonosphericCorrections;
   UINT8 mSVRecordCount;

   struct sSV
   {
      bool mSystemValid:1;
      bool mPRNValid:1;
      bool mHealthStatusValid:1;
      bool mProcessStatusValid:1;
      bool mEphemerisStateValid:1;
      bool mAlmanacStateValid:1;
      bool mElevationValid:1;
      bool mAzimuthValid:1;
      bool mCN0Valid:1;
   
      // Padding out 23 bits
      UINT8 mReserved3:7;
      UINT8 mReserved4[2];
   
      eQMIPDSSVSystems mSystem;
      UINT8 mPRN;
      eQMIPDSSVHealthStatus mHealthLevel;
      eQMIPDSSVProcessingStatus mProcessingStatus;
      eQMIPDSSVEphemerisStatus mEphemerisState;
      eQMIPDSSVAlmanacStatus mAlmanacState;
      INT32 mElevation;
      UINT16 mAzimuth;
      UINT16 mCN0;
   };

   // This array must be the size specified by mSVRecordCount
   // sSV mSVs[1];
};

// Structure to describe indication TLV 0x18 for PDS EventReport
struct sPDSEventReportIndication_VXNetworkInitiatedPrompt
{
   bool mPrivacyValid:1;
   bool mQoSValid:1;
   bool mCountValid:1;
   bool mIntervalValid:1;
   bool mModeValid:1;
   bool mRequestorIDValid:1;

   // Padding out 26 bits
   UINT8 mReserved1:2;
   UINT8 mReserved2[3];

   eQMIPDSPrivacyModes mPrivacy;
   UINT8 mQoS;
   UINT32 mPositionCount;
   UINT32 mIntervalBetweenFixesSeconds;
   eQMIPDSVXModes mMode;
   eQMIPDSVXDataCodingSchemes mRequestorIDDCS;
   UINT8 mRequestorIDLength;

   // This array must be the size specified by mRequestorIDLength
   // UINT8 mRequestorID[1];
};

// Structure to describe indication TLV 0x19 for PDS EventReport
struct sPDSEventReportIndication_SUPLNetworkInitiatedPrompt1
{
   bool mPrivacyValid:1;
   bool mINITHashValid:1;
   bool mModeValid:1;
   bool mSLPSessionIDValid:1;
   bool mSLPServerIPv4AddressValid:1;
   bool mSLPServerIPv6AddressValid:1;
   bool mSLPServerURLAddressValid:1;
   bool mDCSValid:1;
   bool mRequestorIDValid:1;
   bool mClientNameValid:1;
   bool mQoPHorizontalAccuracyValid:1;
   bool mQoPVerticalAccuracyValid:1;
   bool mQoPMaxLocationAgeValid:1;
   bool mQoPDelayValid:1;

   // Padding out 18 bits
   UINT8 mReserved1:2;
   UINT8 mReserved2[2];

   eQMIPDSPrivacyModes mPrivacy;
   UINT64 mINITHash;
   eQMIPDSSUPLModes mMode;
   UINT32 mSLPSessionID;
   UINT32 mSLPServerIPv4Port;
   UINT8 mSLPServerIPv4Address[4];
   UINT32 mSLPServerIPv6Port;
   UINT8 mSLPServerIPv6Address[16];
   UINT8 mSLPServerURLLength;

   // This array must be the size specified by mSLPServerURLLength
   // char mSLPServerURLAddress[1];
};

struct sPDSEventReportIndication_SUPLNetworkInitiatedPrompt2
{
   eQMIPDSSUPLDataCodingSchemes mRequestDCS;
   eQMIPDSSUPLIDNameDataCodingSchemes mRequestorIDDCS;
   UINT8 mRequestorIDLength;

   // This array must be the size specified by mRequestorIDLength
   // UINT8 mRequestorID[1];
};

struct sPDSEventReportIndication_SUPLNetworkInitiatedPrompt3
{
   eQMIPDSSUPLIDNameDataCodingSchemes mClientNameDCS;
   UINT8 mClientNameLength;

   // This array must be the size specified by mClientNameLength
   // UINT8 mClientName[1];
};

struct sPDSEventReportIndication_SUPLNetworkInitiatedPrompt4
{
   UINT8 mQoPHorizontalAccuracy;
   UINT8 mQoPVerticalAccuracy;
   UINT8 mQoPMaxLocationAge;
   UINT8 mQoPDelay;
};

struct sPDSEventReportIndication_SUPLNetworkInitiatedPrompt
{
   sPDSEventReportIndication_SUPLNetworkInitiatedPrompt1 mPDSEventReportIndication_SUPLNetworkInitiatedPrompt1;
   sPDSEventReportIndication_SUPLNetworkInitiatedPrompt2 mPDSEventReportIndication_SUPLNetworkInitiatedPrompt2;
   sPDSEventReportIndication_SUPLNetworkInitiatedPrompt3 mPDSEventReportIndication_SUPLNetworkInitiatedPrompt3;
   sPDSEventReportIndication_SUPLNetworkInitiatedPrompt4 mPDSEventReportIndication_SUPLNetworkInitiatedPrompt4;
};

// Structure to describe indication TLV 0x1A for PDS EventReport
struct sPDSEventReportIndication_UMTSCPNetworkInitiatedPrompt1
{
   bool mPrivacyValid:1;
   bool mInvokeIDValid:1;
   bool mNotificationTextValid:1;
   bool mClientAddressValid:1;
   bool mLocationTypeValid:1;
   bool mRequestorIDValid:1;
   bool mCodewordStringValid:1;
   bool mServiceTypeIDValid:1;

   // Padding out 24 bits
   UINT8 mReserved1[3];

   eQMIPDSPrivacyModes mPrivacy;
   UINT8 mInvokeID;
   eQMIPDSUMTSCPDataCodingSchemes mNotificationTextDCS;
   UINT8 mNotificationTextLength;

   // This array must be the size specified by mNotificationTextLength
   // UINT8 mNotificationText[1];
};

struct sPDSEventReportIndication_UMTSCPNetworkInitiatedPrompt2
{
   UINT8 mClientAddressLength;

   // This array must be the size specified by mClientAddressLength
   // char mClientAddress[1];
};

struct sPDSEventReportIndication_UMTSCPNetworkInitiatedPrompt3
{
   eQMIPDSUMTSCPLocationTypes mLocationType;
   eQMIPDSUMTSCPDataCodingSchemes mRequestorIDDCS;
   UINT8 mRequestorIDLength;

   // This array must be the size specified by mRequestorIDLength
   // UINT8 mRequestorID[1];
};

struct sPDSEventReportIndication_UMTSCPNetworkInitiatedPrompt4
{
   eQMIPDSUMTSCPDataCodingSchemes mCodewordDCS;
   UINT8 mCodewordLength;

   // This array must be the size specified by mCodewordLength
   // UINT8 mCodeword[1];
};

struct sPDSEventReportIndication_UMTSCPNetworkInitiatedPrompt5
{
   UINT8 mServiceTypeID;
};

struct sPDSEventReportIndication_UMTSCPNetworkInitiatedPrompt
{
   sPDSEventReportIndication_UMTSCPNetworkInitiatedPrompt1 mPDSEventReportIndication_UMTSCPNetworkInitiatedPrompt1;
   sPDSEventReportIndication_UMTSCPNetworkInitiatedPrompt2 mPDSEventReportIndication_UMTSCPNetworkInitiatedPrompt2;
   sPDSEventReportIndication_UMTSCPNetworkInitiatedPrompt3 mPDSEventReportIndication_UMTSCPNetworkInitiatedPrompt3;
   sPDSEventReportIndication_UMTSCPNetworkInitiatedPrompt4 mPDSEventReportIndication_UMTSCPNetworkInitiatedPrompt4;
   sPDSEventReportIndication_UMTSCPNetworkInitiatedPrompt5 mPDSEventReportIndication_UMTSCPNetworkInitiatedPrompt5;
};

// Structure to describe indication TLV 0x1B for PDS EventReport
struct sPDSEventReportIndication_CommEvents
{
   eQMIPDSCommEventTypes mType;
   eQMIPDSCommEventProtocols mProtocolDataType;
};

// Structure to describe indication TLV 0x1C for PDS EventReport
struct sPDSEventReportIndication_PositionSource
{
   bool mGPS:1;
   bool mCellID:1;
   bool mGlonass:1;
   bool mNetwork:1;
   bool mEPI:1;

   // Padding out 27 bits
   UINT8 mReserved1:3;
   UINT8 mReserved2[3];
};

// Structure to describe indication TLV 0x1D for PDS EventReport
struct sPDSEventReportIndication_AccelerometerStreamingStatus
{
   eQMIPDSStreamingStatus mAccelerometerStreamingStatus;
};

// Structure to describe indication TLV 0x1E for PDS EventReport
struct sPDSEventReportIndication_GyroStreamingStatus
{
   eQMIPDSStreamingStatus mGyroStreamingStatus;
};

// Structure to describe indication TLV 0x1F for PDS EventReport
struct sPDSEventReportIndication_TimeSyncRequest
{
   UINT32 mReferenceCounter;
};

// Structure to describe indication TLV 0x20 for PDS EventReport
struct sPDSEventReportIndication_PositionReliabilityCounter
{
   eQMIPDSReliabilityIndicator mReliabilityIndicatorHorizontal;
   eQMIPDSReliabilityIndicator mReliabilityIndicatorVertical;
};

// Structure to describe indication TLV 0x21 for PDS EventReport
struct sPDSEventReportIndication_SensorDataUsage
{
   bool mAccelerometer:1;
   bool mGyro:1;

   // Padding out 14 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2;

   bool mHeadingAidedWithSensorData:1;
   bool mSpeedAidedWithSensorData:1;
   bool mPositionAidedWithSensorData:1;
   bool mVelocityAidedWithSensorData:1;

   // Padding out 12 bits
   UINT8 mReserved3:4;
   UINT8 mReserved4;
};

// Structure to describe indication TLV 0x22 for PDS EventReport
struct sPDSEventReportIndication_TimeSourceInformation
{
   INT8 mTimeValid;
   eQMIPDSTimeSource mTimeSource;
   UINT32 mTimeUncertaintyMilliseconds;
   UINT16 mGPSWeekNumber;
   UINT32 mGPSTimeOfWeekMilliseconds;
   UINT32 mReserved1;
   UINT32 mReserved2;
   UINT32 mReserved3;
   UINT32 mReserved4;
   UINT32 mReserved5;
};

// Structure to describe indication TLV 0x23 for PDS EventReport
struct sPDSEventReportIndication_EncryptedPositionInformation
{
   eQMIPDSEncryptionAlgorithm mEncryptionAlgorithm;
   UINT8 mEncryptionDataLength;

   // This array must be the size specified by mEncryptionDataLength
   // UINT8 mEncryptionData[1];
};

// Structure to describe indication TLV 0x24 for PDS EventReport
struct sPDSEventReportIndication_HeadingUncertaintyInformation
{
   float mHeadingUncertainty;
   float mGNSSHeadingUncertainty;
   UINT32 mReserved1;
   UINT32 mReserved2;
};

// Structure to describe indication TLV 0x25 for PDS EventReport
struct sPDSEventReportIndication_ProprietaryNMEADebugSentences
{
   // String is variable length, but must be size of the container
   // char mNMEADebug[1];
};

// Structure to describe indication TLV 0x26 for PDS EventReport
struct sPDSEventReportIndication_ExtendedExternalXTRADatabaseRequest
{
   UINT32 mMaximumFileSize;
   UINT8 mURLCount;
   UINT8 mURLLength;

   // This array must be the size specified by mURLLength
   // char mURL[1];
};

// Structure to describe indication TLV 0x27 for PDS EventReport
struct sPDSEventReportIndication_ServiceResetStatus
{
   eQMIPDSResetStates mServiceResetStatus;
};

// Structure to describe response TLV 0x01 for PDSGetServiceState()
struct sPDSGetServiceStateResponse_State
{
   INT8 mServiceEnabled;
   eQMIPDSTrackingSessionStates mTrackingSessionState;
};

// Structure to describe indication TLV 0x01 for PDS ServiceStateIndication
struct sPDSServiceStateIndication_State
{
   INT8 mServiceEnabled;
   eQMIPDSTrackingSessionStates mTrackingSessionState;
};

// Structure to describe request TLV 0x01 for PDSSetServiceState()
struct sPDSSetServiceStateRequest_State
{
   INT8 mServiceEnabled;
};

// Structure to describe request TLV 0x01 for PDSStartTrackingSession()
struct sPDSStartTrackingSessionRequest_Session
{
   eQMIPDSSessionControlTypes mSessionControl;
   eQMIPDSSessionTypes mSessionType;
   eQMIPDSOperationTypes mSessionOperation;
   eQMIPDSServerOptions mServerOption;
   UINT8 mTimeoutSeconds;
   UINT32 mSessionFixRequests;
   UINT32 mFixRequestIntervalSeconds;
   UINT32 mDesiredAccuracyMeters;
};

// Structure to describe response TLV 0x01 for PDSGetTrackingSessionInfo()
struct sPDSGetTrackingSessionInfoResponse_Info
{
   eQMIPDSSessionControlTypes mSessionControl;
   eQMIPDSSessionTypes mSessionType;
   eQMIPDSOperationTypes mSessionOperation;
   eQMIPDSServerOptions mServerOption;
   UINT8 mTimeoutSeconds;
   UINT32 mSessionFixRequests;
   UINT32 mFixRequestIntervalSeconds;
   UINT32 mDesiredAccuracyMeters;
};

// Structure to describe response TLV 0x01 for PDSGetNMEAConfig()
struct sPDSGetNMEAConfigResponse_Config
{
   bool mGPGGANMEASentences:1;
   bool mGPRMCNMEASentences:1;
   bool mGPGSVNMEASentences:1;
   bool mGPGSANMEASentences:1;
   bool mGPVTGNMEASentences:1;
   bool mGLGSVNMEASentences:1;
   bool mGNGSANMEASentences:1;
   bool mGNGNSNMEASentences:1;
   eQMIPDSOutputDevices mOutputDevice;
   eQMIPDSNMEAReportingOptions mNMEAReporting;
};

// Structure to describe response TLV 0x10 for PDSGetNMEAConfig()
struct sPDSGetNMEAConfigResponse_AdditionalConfig
{
   bool mPQXFI:1;
   bool mPSTIS:1;

   // Padding out 14 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2;
};

// Structure to describe request TLV 0x01 for PDSSetNMEAConfig()
struct sPDSSetNMEAConfigRequest_Config
{
   bool mGPGGANMEASentences:1;
   bool mGPRMCNMEASentences:1;
   bool mGPGSVNMEASentences:1;
   bool mGPGSANMEASentences:1;
   bool mGPVTGNMEASentences:1;
   bool mGLGSVNMEASentences:1;
   bool mGNGSANMEASentences:1;
   bool mGNGNSNMEASentences:1;
   eQMIPDSOutputDevices mOutputDevice;
   eQMIPDSNMEAReportingOptions mNMEAReporting;
};

// Structure to describe request TLV 0x10 for PDSSetNMEAConfig()
struct sPDSSetNMEAConfigRequest_AdditionalConfig
{
   bool mPQXFI:1;
   bool mPSTIS:1;

   // Padding out 14 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2;
};

// Structure to describe request TLV 0x01 for PDSInjectTimeReference()
struct sPDSInjectTimeReferenceRequest_Time
{
   UINT64 mSystemTimeMilliseconds;
   UINT16 mSystemDiscontinuties;
};

// Structure to describe response TLV 0x01 for PDSGetDefaults()
struct sPDSGetDefaultsResponse_Defaults
{
   eQMIPDSOperationTypes mSessionOperation;
   UINT8 mTimeoutSeconds;
   UINT32 mFixRequestIntervalSeconds;
   UINT32 mDesiredAccuracyMeters;
};

// Structure to describe request TLV 0x01 for PDSSetDefaults()
struct sPDSSetDefaultsRequest_Defaults
{
   eQMIPDSOperationTypes mSessionOperation;
   UINT8 mTimeoutSeconds;
   UINT32 mFixRequestIntervalSeconds;
   UINT32 mDesiredAccuracyMeters;
};

// Structure to describe response TLV 0x10 for PDSGetXTRAParameters()
struct sPDSGetXTRAParametersResponse_Automatic
{
   INT8 mAutomaticDownloadEnabled;
   UINT16 mDownloadIntervalInHours;
};

// Structure to describe response TLV 0x11 for PDSGetXTRAParameters()
struct sPDSGetXTRAParametersResponse_Medium
{
   UINT8 mMediumPreferences;

   // This array must be the size specified by mMediumPreferences
   // eQMIPDSMediums mMediumPreference[1];
};

// Structure to describe response TLV 0x12 for PDSGetXTRAParameters()
struct sPDSGetXTRAParametersResponse_Network
{
   eQMIPDSWWANNetworkPreferences mWWANNetworkPreference;
};

// Structure to describe response TLV 0x13 for PDSGetXTRAParameters()
struct sPDSGetXTRAParametersResponse_Validity
{
   UINT16 mValidPeriodGPSStartWeek;
   UINT16 mValidPeriodGPSStartWeekOffsetInMinutes;
   UINT16 mValidPeriodDurationInHours;
};

// Structure to describe response TLV 0x14 for PDSGetXTRAParameters()
struct sPDSGetXTRAParametersResponse_Embedded
{
   INT8 mEmbeddedXTRADataClientEnabled;
   INT8 mEmbeddedXTRATimeClientEnabled;
};

// Structure to describe request TLV 0x10 for PDSSetXTRAParameters()
struct sPDSSetXTRAParametersRequest_Automatic
{
   INT8 mAutomaticDownloadEnabled;
   UINT16 mDownloadIntervalInHours;
};

// Structure to describe request TLV 0x11 for PDSSetXTRAParameters()
struct sPDSSetXTRAParametersRequest_Medium
{
   UINT8 mMediumPreferences;

   // This array must be the size specified by mMediumPreferences
   // eQMIPDSMediums mMediumPreference[1];
};

// Structure to describe request TLV 0x12 for PDSSetXTRAParameters()
struct sPDSSetXTRAParametersRequest_Network
{
   eQMIPDSWWANNetworkPreferences mWWANNetworkPreference;
};

// Structure to describe request TLV 0x14 for PDSSetXTRAParameters()
struct sPDSSetXTRAParametersRequest_Embedded
{
   INT8 mEmbeddedXTRADataClientEnabled;
   INT8 mEmbeddedXTRATimeClientEnabled;
};

// Structure to describe request TLV 0x12 for PDSGetAGPSConfig()
struct sPDSGetAGPSConfigRequest_NetworkMode
{
   eQMIPDSNetworkMode mNetworkMode;
};

// Structure to describe response TLV 0x10 for PDSGetAGPSConfig()
struct sPDSGetAGPSConfigResponse_ServerAddress
{
   UINT8 mServerAddress[4];
   UINT32 mServerPort;
};

// Structure to describe response TLV 0x11 for PDSGetAGPSConfig()
struct sPDSGetAGPSConfigResponse_ServerURL
{
   UINT8 mURLLength;

   // This array must be the size specified by mURLLength
   // char mURL[1];
};

// Structure to describe request TLV 0x10 for PDSSetAGPSConfig()
struct sPDSSetAGPSConfigRequest_Server
{
   UINT8 mServerAddress[4];
   UINT32 mServerPort;
};

// Structure to describe request TLV 0x11 for PDSSetAGPSConfig()
struct sPDSSetAGPSConfigRequest_ServerURL
{
   UINT8 mURLLength;

   // This array must be the size specified by mURLLength
   // char mURL[1];
};

// Structure to describe request TLV 0x12 for PDSSetAGPSConfig()
struct sPDSSetAGPSConfigRequest_NetworkMode
{
   eQMIPDSNetworkMode mNetworkMode;
};

// Structure to describe response TLV 0x01 for PDSGetServiceAutoTrackingState()
struct sPDSGetServiceAutoTrackingStateResponse_State
{
   INT8 mAutoTrackingEnabled;
};

// Structure to describe request TLV 0x01 for PDSSetServiceAutoTrackingState()
struct sPDSSetServiceAutoTrackingStateRequest_State
{
   INT8 mAutoTrackingEnabled;
};

// Structure to describe response TLV 0x01 for PDSGetCOMPortAutoTrackingConfig()
struct sPDSGetCOMPortAutoTrackingConfigResponse_Config
{
   INT8 mAutoTrackingEnabled;
};

// Structure to describe request TLV 0x01 for PDSSetCOMPortAutoTrackingConfig()
struct sPDSSetCOMPortAutoTrackingConfigRequest_Config
{
   INT8 mAutoTrackingEnabled;
};

// Structure to describe request TLV 0x10 for PDSResetPDSData()
struct sPDSResetPDSDataRequest_GPSData
{
   bool mResetEPH:1;
   bool mResetALM:1;
   bool mResetPOS:1;
   bool mResetTIME:1;
   bool mResetIONO:1;
   bool mResetUTC:1;
   bool mResetHEALTH:1;
   bool mResetSVDIR:1;
   bool mResetSVSTEER:1;
   bool mResetSADATA:1;
   bool mResetRTI:1;
   bool mResetALMCORR:1;
   bool mResetFREQBIASEST:1;

   // Padding out 19 bits
   UINT8 mReserved1:3;
   UINT8 mReserved2[2];
};

// Structure to describe request TLV 0x11 for PDSResetPDSData()
struct sPDSResetPDSDataRequest_CellData
{
   bool mResetPOS:1;
   bool mResetLATESTGPSPOS:1;
   bool mResetOTAPOS:1;
   bool mResetEXTREFPOS:1;
   bool mResetTIMETAG:1;
   bool mResetCELLID:1;
   bool mResetCACHEDCELLID:1;
   bool mResetLASTSRVCELL:1;
   bool mResetCURSRVCELL:1;
   bool mResetNEIGHBORINFO:1;

   // Padding out 22 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[2];
};

// Structure to describe request TLV 0x10 for PDSSinglePositionFix()
struct sPDSSinglePositionFixRequest_Mode
{
   eQMIPDSOperationTypes mSessionOperation;
};

// Structure to describe request TLV 0x11 for PDSSinglePositionFix()
struct sPDSSinglePositionFixRequest_Timeout
{
   UINT8 mTimeoutSeconds;
};

// Structure to describe request TLV 0x12 for PDSSinglePositionFix()
struct sPDSSinglePositionFixRequest_Accuracy
{
   UINT32 mDesiredAccuracyMeters;
};

// Structure to describe response TLV 0x01 for PDSGetServiceVersion()
struct sPDSGetServiceVersionResponse_Version
{
   UINT8 mServiceMajorVersion;
   UINT8 mServiceMinorVersion;
};

// Structure to describe request TLV 0x01 for PDSInjectXTRAData()
struct sPDSInjectXTRADataRequest_Data
{
   UINT8 mSequenceNumber;
   UINT16 mTotalLength;
   UINT16 mSequenceLength;

   // This array must be the size specified by mSequenceLength
   // UINT8 mData[1];
};

// Structure to describe request TLV 0x10 for PDSInjectPositionData()
struct sPDSInjectPositionDataRequest_Timestamp
{
   UINT64 mUTCTimestamp;
};

// Structure to describe request TLV 0x11 for PDSInjectPositionData()
struct sPDSInjectPositionDataRequest_Latitude
{
   double mLatitude;
};

// Structure to describe request TLV 0x12 for PDSInjectPositionData()
struct sPDSInjectPositionDataRequest_Longitude
{
   double mLongitude;
};

// Structure to describe request TLV 0x13 for PDSInjectPositionData()
struct sPDSInjectPositionDataRequest_AltitudeEllipsoid
{
   float mEllipsoidAltitude;
};

// Structure to describe request TLV 0x14 for PDSInjectPositionData()
struct sPDSInjectPositionDataRequest_AltitudeSeaLevel
{
   float mMeanSeaLevelAltitude;
};

// Structure to describe request TLV 0x15 for PDSInjectPositionData()
struct sPDSInjectPositionDataRequest_HorizontalUncertainty
{
   float mHorizontalUncertaintyCircular;
};

// Structure to describe request TLV 0x16 for PDSInjectPositionData()
struct sPDSInjectPositionDataRequest_VerticalUncertainty
{
   float mVerticalUncertainty;
};

// Structure to describe request TLV 0x17 for PDSInjectPositionData()
struct sPDSInjectPositionDataRequest_HorizontalConfidence
{
   UINT8 mHorizontalConfidence;
};

// Structure to describe request TLV 0x18 for PDSInjectPositionData()
struct sPDSInjectPositionDataRequest_VerticalConfidence
{
   UINT8 mVerticalConfidence;
};

// Structure to describe request TLV 0x19 for PDSInjectPositionData()
struct sPDSInjectPositionDataRequest_Source
{
   eQMIPDSInjectedPositionSources mSource;
};

// Structure to describe request TLV 0x1A for PDSInjectPositionData()
struct sPDSInjectPositionDataRequest_TimeType
{
   eQMIPDSTimeType mTimeType;
};

// Structure to describe request TLV 0x1B for PDSInjectPositionData()
struct sPDSInjectPositionDataRequest_PositionReliability
{
   eQMIPDSReliabilityIndicator mReliabilityIndicatorHorizontal;
   eQMIPDSReliabilityIndicator mReliabilityIndicatorVertical;
};

// Structure to describe request TLV 0x1C for PDSInjectPositionData()
struct sPDSInjectPositionDataRequest_AltitudeInformation
{
   eQMIPDSAltitudeSource mAltitudeSource;
   eQMIPDSSourceLinkage mSourceLinkage;
   eQMIPDSUncertaintyCoverage mUncertaintyCoverage;
};

// Structure to describe request TLV 0x10 for PDSInjectWiFiPositionData()
struct sPDSInjectWiFiPositionDataRequest_Time
{
   UINT32 mWiFiTimeCounterMilliseconds;
};

// Structure to describe request TLV 0x11 for PDSInjectWiFiPositionData()
struct sPDSInjectWiFiPositionDataRequest_Position
{
   INT32 mWiFiLatitude;
   INT32 mWiFiLongitude;
   UINT16 mHEPEInMeters;
   UINT8 mAPCount;
   UINT8 mErrorCode;
};

// Structure to describe request TLV 0x12 for PDSInjectWiFiPositionData()
struct sPDSInjectWiFiPositionDataRequest_APInfo
{
   UINT8 mAPCount;

   struct sAP
   {
      UINT8 mMACAddress[6];
      UINT32 mRSSI;
      UINT16 mBeaconChannel;
      bool mUsedForPosition:1;
      bool mHiddenSSID:1;
      bool mEncryptionOn:1;
      bool mInfrastructureMode:1;
   
      // Padding out 4 bits
      UINT8 mReserved1:4;
   };

   // This array must be the size specified by mAPCount
   // sAP mAPs[1];
};

// Structure to describe request TLV 0x13 for PDSInjectWiFiPositionData()
struct sPDSInjectWiFiPositionDataRequest_PositionReliability
{
   eQMIPDSReliabilityIndicator mReliabilityIndicatorHorizontal;
};

// Structure to describe response TLV 0x10 for PDSGetSBASConfig()
struct sPDSGetSBASConfigResponse_Config
{
   eQMIPDSSBASStates mState;
};

// Structure to describe request TLV 0x10 for PDSSetSBASConfig()
struct sPDSSetSBASConfigRequest_Config
{
   INT8 mEnableSBAS;
};

// Structure to describe request TLV 0x01 for PDSSendNetworkInitiatedResponse()
struct sPDSSendNetworkInitiatedResponseRequest_Action
{
   INT8 mAllowRequest;
};

// Structure to describe request TLV 0x10 for PDSSendNetworkInitiatedResponse()
struct sPDSSendNetworkInitiatedResponseRequest_VX
{
   bool mPrivacyValid:1;
   bool mQoSValid:1;
   bool mCountValid:1;
   bool mIntervalValid:1;
   bool mModeValid:1;
   bool mRequestorIDValid:1;

   // Padding out 26 bits
   UINT8 mReserved1:2;
   UINT8 mReserved2[3];

   eQMIPDSPrivacyModes mPrivacy;
   UINT8 mQoS;
   UINT32 mPositionCount;
   UINT32 mIntervalBetweenFixesSeconds;
   eQMIPDSVXModes mMode;
   eQMIPDSVXDataCodingSchemes mRequestorIDDCS;
   UINT8 mRequestorIDLength;

   // This array must be the size specified by mRequestorIDLength
   // UINT8 mRequestorID[1];
};

// Structure to describe request TLV 0x11 for PDSSendNetworkInitiatedResponse()
struct sPDSSendNetworkInitiatedResponseRequest_SUPL1
{
   bool mPrivacyValid:1;
   bool mINITHashValid:1;
   bool mModeValid:1;
   bool mSLPSessionIDValid:1;
   bool mSLPServerIPv4AddressValid:1;
   bool mSLPServerIPv6AddressValid:1;
   bool mSLPServerURLAddressValid:1;
   bool mDCSValid:1;
   bool mRequestorIDValid:1;
   bool mClientNameValid:1;
   bool mQoPHorizontalAccuracyValid:1;
   bool mQoPVerticalAccuracyValid:1;
   bool mQoPMaxLocationAgeValid:1;
   bool mQoPDelayValid:1;

   // Padding out 18 bits
   UINT8 mReserved1:2;
   UINT8 mReserved2[2];

   eQMIPDSPrivacyModes mPrivacy;
   UINT64 mINITHash;
   eQMIPDSSUPLModes mMode;
   UINT32 mSLPSessionID;
   UINT32 mSLPServerIPv4Port;
   UINT8 mSLPServerIPv4Address[4];
   UINT32 mSLPServerIPv6Port;
   UINT8 mSLPServerIPv6Address[16];
   UINT8 mSLPServerURLLength;

   // This array must be the size specified by mSLPServerURLLength
   // char mSLPServerURLAddress[1];
};

struct sPDSSendNetworkInitiatedResponseRequest_SUPL2
{
   eQMIPDSSUPLDataCodingSchemes mRequestDCS;
   eQMIPDSSUPLIDNameDataCodingSchemes mRequestorIDDCS;
   UINT8 mRequestorIDLength;

   // This array must be the size specified by mRequestorIDLength
   // UINT8 mRequestorID[1];
};

struct sPDSSendNetworkInitiatedResponseRequest_SUPL3
{
   eQMIPDSSUPLIDNameDataCodingSchemes mClientNameDCS;
   UINT8 mClientNameLength;

   // This array must be the size specified by mClientNameLength
   // UINT8 mClientName[1];
};

struct sPDSSendNetworkInitiatedResponseRequest_SUPL4
{
   UINT8 mQoPHorizontalAccuracy;
   UINT8 mQoPVerticalAccuracy;
   UINT8 mQoPMaxLocationAge;
   UINT8 mQoPDelay;
};

struct sPDSSendNetworkInitiatedResponseRequest_SUPL
{
   sPDSSendNetworkInitiatedResponseRequest_SUPL1 mPDSSendNetworkInitiatedResponseRequest_SUPL1;
   sPDSSendNetworkInitiatedResponseRequest_SUPL2 mPDSSendNetworkInitiatedResponseRequest_SUPL2;
   sPDSSendNetworkInitiatedResponseRequest_SUPL3 mPDSSendNetworkInitiatedResponseRequest_SUPL3;
   sPDSSendNetworkInitiatedResponseRequest_SUPL4 mPDSSendNetworkInitiatedResponseRequest_SUPL4;
};

// Structure to describe request TLV 0x12 for PDSSendNetworkInitiatedResponse()
struct sPDSSendNetworkInitiatedResponseRequest_UMTSCP1
{
   bool mPrivacyValid:1;
   bool mInvokeIDValid:1;
   bool mNotificationTextValid:1;
   bool mClientAddressValid:1;
   bool mLocationTypeValid:1;
   bool mRequestorIDValid:1;
   bool mCodewordStringValid:1;
   bool mServiceTypeIDValid:1;

   // Padding out 24 bits
   UINT8 mReserved1[3];

   eQMIPDSPrivacyModes mPrivacy;
   UINT8 mInvokeID;
   eQMIPDSUMTSCPDataCodingSchemes mNotificationTextDCS;
   UINT8 mNotificationTextLength;

   // This array must be the size specified by mNotificationTextLength
   // UINT8 mNotificationText[1];
};

struct sPDSSendNetworkInitiatedResponseRequest_UMTSCP2
{
   UINT8 mClientAddressLength;

   // This array must be the size specified by mClientAddressLength
   // char mClientAddress[1];
};

struct sPDSSendNetworkInitiatedResponseRequest_UMTSCP3
{
   eQMIPDSUMTSCPLocationTypes mLocationType;
   eQMIPDSUMTSCPDataCodingSchemes mRequestorIDDCS;
   UINT8 mRequestorIDLength;

   // This array must be the size specified by mRequestorIDLength
   // UINT8 mRequestorID[1];
};

struct sPDSSendNetworkInitiatedResponseRequest_UMTSCP4
{
   eQMIPDSUMTSCPDataCodingSchemes mCodewordDCS;
   UINT8 mCodewordLength;

   // This array must be the size specified by mCodewordLength
   // UINT8 mCodeword[1];
};

struct sPDSSendNetworkInitiatedResponseRequest_UMTSCP5
{
   UINT8 mServiceTypeID;
};

struct sPDSSendNetworkInitiatedResponseRequest_UMTSCP
{
   sPDSSendNetworkInitiatedResponseRequest_UMTSCP1 mPDSSendNetworkInitiatedResponseRequest_UMTSCP1;
   sPDSSendNetworkInitiatedResponseRequest_UMTSCP2 mPDSSendNetworkInitiatedResponseRequest_UMTSCP2;
   sPDSSendNetworkInitiatedResponseRequest_UMTSCP3 mPDSSendNetworkInitiatedResponseRequest_UMTSCP3;
   sPDSSendNetworkInitiatedResponseRequest_UMTSCP4 mPDSSendNetworkInitiatedResponseRequest_UMTSCP4;
   sPDSSendNetworkInitiatedResponseRequest_UMTSCP5 mPDSSendNetworkInitiatedResponseRequest_UMTSCP5;
};

// Structure to describe request TLV 0x01 for PDSInjectAbsoluteTime()
struct sPDSInjectAbsoluteTimeRequest_Time
{
   UINT64 mTimestampMilliseconds;
   UINT32 mTimeUncertaintyMilliseconds;
   eQMIPDSTimeBases mTimeBase;
   INT8 mForceAcceptance;
};

// Structure to describe request TLV 0x01 for PDSInjectEFSData()
struct sPDSInjectEFSDataRequest_DateFile1
{
   UINT8 mFilenameLength;

   // This array must be the size specified by mFilenameLength
   // char mFilename[1];
};

struct sPDSInjectEFSDataRequest_DateFile2
{
   eQMIPDSEFSFileOperations mFileOperation;
   UINT32 mDataLength;
   UINT8 mPartNumber;
   UINT8 mTotalParts;

   // This array must be the size specified by mDataLength
   // UINT8 mData[1];
};

struct sPDSInjectEFSDataRequest_DateFile
{
   sPDSInjectEFSDataRequest_DateFile1 mPDSInjectEFSDataRequest_DateFile1;
   sPDSInjectEFSDataRequest_DateFile2 mPDSInjectEFSDataRequest_DateFile2;
};

// Structure to describe response TLV 0x10 for PDSGetDPOConfig()
struct sPDSGetDPOConfigResponse_Config
{
   INT8 mDataPowerOptimizationEnabled;
};

// Structure to describe request TLV 0x10 for PDSSetDPOConfig()
struct sPDSSetDPOConfigRequest_Config
{
   eQMIPDSConfig mConfiguration;
};

// Structure to describe response TLV 0x10 for PDSGetODPConfig()
struct sPDSGetODPConfigResponse_Config
{
   eQMIPDSODPStates mOnDemandPositioning;
};

// Structure to describe request TLV 0x10 for PDSSetODPConfig()
struct sPDSSetODPConfigRequest_Config
{
   eQMIPDSODPStates mOnDemandPositioning;
};

// Structure to describe response TLV 0x10 for PDSGetGPSState()
struct sPDSGetGPSStateResponse_State
{
   INT8 mEngineEnabled;
   bool mPositionValid:1;
   bool mAltitudeVerticalUncertaintyValid:1;
   bool mTimeMillisecondsValid:1;
   bool mTimeWeekNumberValid:1;
   bool mTimeUncertaintyValid:1;
   bool mIonoValid:1;
   bool mGPSEphemerisValid:1;
   bool mGPSAlmanacValid:1;
   bool mGPSHealthValid:1;
   bool mGPSVisibleSVsValid:1;
   bool mGlonassEphemerisValid:1;
   bool mGlonassAlmanacValid:1;
   bool mGlonassHealthValid:1;
   bool mGlonassVisibleSVsValid:1;
   bool mSBASEphemerisValid:1;
   bool mSBASAlmanacValid:1;
   bool mSBASHealthValid:1;
   bool mSBASVisibleSVsValid:1;
   bool mXTRAInformationValid:1;

   // Padding out 5 bits
   UINT8 mReserved1:5;

   double mLatitude;
   double mLongitude;
   float mHorizontalUncertaintyCircular;
   float mEllipsoidAltitude;
   float mVerticalUncertainty;
   UINT32 mTimestampInTOWMilliseconds;
   UINT16 mGPSWeekNumber;
   UINT32 mTimeUncertaintyMilliseconds;
   INT8 mIonoIsValid;
   UINT32 mGPSEphemerisSVMask;
   UINT32 mGPSAlmanacSVMask;
   UINT32 mGPSHealthSVMask;
   UINT32 mGPSVisibleSVMask;
   UINT32 mGlonassEphemerisSVMask;
   UINT32 mGlonassAlmanacSVMask;
   UINT32 mGlonassHealthSVMask;
   UINT32 mGlonassVisibleSVMask;
   UINT32 mSBASEphemerisSVMask;
   UINT32 mSBASAlmanacSVMask;
   UINT32 mSBASHealthSVMask;
   UINT32 mSBASVisibleSVMask;
   UINT16 mXTRAGPSStartWeek;
   UINT16 mXTRAGPSStartMinutes;
   INT32 mElevation;
};

// Structure to describe request TLV 0x01 for PDSSetPPMEventReport()
struct sPDSSetPPMEventReportRequest_ReportPPMEvents
{
   INT8 mReportPPMEvents;
};

// Structure to describe request TLV 0x01 for PDSSetSPIStreamingReport()
struct sPDSSetSPIStreamingReportRequest_ReportEvents
{
   INT8 mReportSPIEvents;
};

// Structure to describe indication TLV 0x10 for PDS SetSPIStreamingReportIndication
struct sPDSSetSPIStreamingReportIndication_SPIStreamingRequest
{
   INT8 mStreamSPIStatus;
};

// Structure to describe request TLV 0x01 for PDSSetSPIStatus()
struct sPDSSetSPIStatusRequest_Status
{
   eQMIPDSSPIState mSPIState;
   UINT8 mSPIConfidence;
};

// Structure to describe request TLV 0x01 for PDSSetPPMReportingState()
struct sPDSSetPPMReportingStateRequest_PPMReportingState
{
   eQMIPDSReportingState mReportingState;
   UINT16 mReportingPeriodSeconds;
};

// Structure to describe indication TLV 0x10 for PDS SetPPMReportingStateIndication
struct sPDSSetPPMReportingStateIndication_PPMPageReport
{
   UINT32 mPageTimeMilliseconds;
   UINT16 mPagePNOffset;
   UINT8 mPageBandClass;
   UINT16 mPageCDMAFrequency;
   UINT16 mPageBaseID;
   UINT16 mPageSystemID;
   UINT16 mPageNetworkID;
};

// Structure to describe indication TLV 0x11 for PDS SetPPMReportingStateIndication
struct sPDSSetPPMReportingStateIndication_PPMMeasurementReport
{
   UINT32 mReferenceTimeMilliseconds;
   UINT8 mReferenceBandClass;
   UINT16 mReferenceCDMAFrequency;
   UINT8 mReferenceTotalRXPower;
   UINT16 mReferencePNOffset;
   UINT8 mReferenceStrength;
   UINT8 mReferenceRMSErrorPhase;
   UINT8 mNumberOfPilots;

   struct sPilot
   {
      UINT16 mPilotPNOffset;
      UINT8 mPilotStrength;
      UINT8 mPilotRMSErrorPhase;
      INT16 mPilotPhase;
   };

   // This array must be the size specified by mNumberOfPilots
   // sPilot mPilots[1];
};

// Structure to describe indication TLV 0x12 for PDS SetPPMReportingStateIndication
struct sPDSSetPPMReportingStateIndication_PPMSuspendReason
{
   eQMIPDSSuspendReason mSuspendReason;
};

// Structure to describe indication TLV 0x13 for PDS SetPPMReportingStateIndication
struct sPDSSetPPMReportingStateIndication_PPMStopReason
{
   eQMIPDSStopReason mStopReason;
};

// Structure to describe request TLV 0x01 for PDSForceReceiverOff()
struct sPDSForceReceiverOffRequest_ForceReceiverOff
{
   eQMIPDSForceReceiverOff mForceReceiverOff;
};

// Structure to describe response TLV 0x10 for PDSGetPositionMethodsState()
struct sPDSGetPositionMethodsStateResponse_XTRATime
{
   eQMIPDSMethodStates mMethodState;
};

// Structure to describe response TLV 0x11 for PDSGetPositionMethodsState()
struct sPDSGetPositionMethodsStateResponse_XTRAData
{
   eQMIPDSMethodStates mMethodState;
};

// Structure to describe response TLV 0x12 for PDSGetPositionMethodsState()
struct sPDSGetPositionMethodsStateResponse_WiFi
{
   eQMIPDSMethodStates mMethodState;
};

// Structure to describe request TLV 0x10 for PDSSetPositionMethodsState()
struct sPDSSetPositionMethodsStateRequest_XTRATime
{
   eQMIPDSMethodStates mMethodState;
};

// Structure to describe request TLV 0x11 for PDSSetPositionMethodsState()
struct sPDSSetPositionMethodsStateRequest_XTRAData
{
   eQMIPDSMethodStates mMethodState;
};

// Structure to describe request TLV 0x12 for PDSSetPositionMethodsState()
struct sPDSSetPositionMethodsStateRequest_WiFi
{
   eQMIPDSMethodStates mMethodState;
};

// Structure to describe request TLV 0x10 for PDSInjectSensorData()
struct sPDSInjectSensorDataRequest_3AxisAccelerometerData
{
   UINT32 mSensorTimeOfFirstSampleMilliseconds;
   bool mSignReversalRequired:1;

   // Padding out 7 bits
   UINT8 mReserved1:7;

   UINT8 mNumberOfSamples;

   struct sSample
   {
      UINT16 mSampleTimeOffsetMilliseconds;
      float mXAxisSample;
      float mYAxisSample;
      float mZAxisSample;
   };

   // This array must be the size specified by mNumberOfSamples
   // sSample mSamples[1];
};

// Structure to describe request TLV 0x11 for PDSInjectSensorData()
struct sPDSInjectSensorDataRequest_3AxisGyroData
{
   UINT32 mSensorTimeOfFirstSampleMilliseconds;
   bool mSignReversalRequired:1;

   // Padding out 7 bits
   UINT8 mReserved1:7;

   UINT8 mNumberOfSamples;

   struct sSample
   {
      UINT16 mSampleTimeOffsetMilliseconds;
      float mXAxisSample;
      float mYAxisSample;
      float mZAxisSample;
   };

   // This array must be the size specified by mNumberOfSamples
   // sSample mSamples[1];
};

// Structure to describe response TLV 0x10 for PDSInjectSensorData()
struct sPDSInjectSensorDataResponse_AccelerometerSampleSetCount
{
   UINT8 mAccelerometerSampleSetCount;
};

// Structure to describe response TLV 0x11 for PDSInjectSensorData()
struct sPDSInjectSensorDataResponse_GyroSampleSetCount
{
   UINT8 mGyroSampleSetCount;
};

// Structure to describe request TLV 0x01 for PDSInjectTimeSyncData()
struct sPDSInjectTimeSyncDataRequest_TimeTag
{
   UINT32 mReferenceCounter;
   UINT32 mSensorProcessorRXTimeMilliseconds;
   UINT32 mSensorProcessorTXTimeMilliseconds;
};

// Structure to describe response TLV 0x10 for PDSGetSensorConfig()
struct sPDSGetSensorConfigResponse_CradleMountState
{
   eQMIPDSCradleMountState mCradleMountState;
   UINT8 mCradleMountStateConfidence;
};

// Structure to describe response TLV 0x11 for PDSGetSensorConfig()
struct sPDSGetSensorConfigResponse_ExternalPowerSupplyState
{
   eQMIPDSExternalPowerState mExternalPowerSupplyState;
};

// Structure to describe request TLV 0x10 for PDSSetSensorConfig()
struct sPDSSetSensorConfigRequest_CradleMountState
{
   eQMIPDSCradleMountState mCradleMountState;
   UINT8 mCradleMountStateConfidence;
};

// Structure to describe request TLV 0x11 for PDSSetSensorConfig()
struct sPDSSetSensorConfigRequest_ExternalPowerState
{
   eQMIPDSExternalPowerState mExternalPowerSupplyState;
};

// Structure to describe response TLV 0x10 for PDSGetSensorNavigation()
struct sPDSGetSensorNavigationResponse_GyroDataVariance
{
   float mGyroDataVariance;
};

// Structure to describe response TLV 0x11 for PDSGetSensorNavigation()
struct sPDSGetSensorNavigationResponse_GyroBiasRandomWalk
{
   float mGyroBiasRandomWalk;
};

// Structure to describe response TLV 0x12 for PDSGetSensorNavigation()
struct sPDSGetSensorNavigationResponse_VirtualGyroGravityAveragingLength
{
   UINT32 mVirtualGyroGravityAveragingLengthMilliseconds;
};

// Structure to describe response TLV 0x13 for PDSGetSensorNavigation()
struct sPDSGetSensorNavigationResponse_HeadingFilterMode
{
   INT8 mHeadingFilterMode;
};

// Structure to describe response TLV 0x14 for PDSGetSensorNavigation()
struct sPDSGetSensorNavigationResponse_VelocityConstraintVariance
{
   float mVelocityConstraintVariance;
};

// Structure to describe response TLV 0x15 for PDSGetSensorNavigation()
struct sPDSGetSensorNavigationResponse_PositioningMethod
{
   bool mSuppressInjectedPosition:1;
   bool mReportUsingInstantaneousInformation:1;
   bool mDisablePersistentMemoryStorage:1;

   // Padding out 29 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2[3];
};

// Structure to describe response TLV 0x16 for PDSGetSensorNavigation()
struct sPDSGetSensorNavigationResponse_VelocityRWSD
{
   float mVelocityRandomWalkSpectralDensity;
};

// Structure to describe response TLV 0x17 for PDSGetSensorNavigation()
struct sPDSGetSensorNavigationResponse_AccelerationRWSD
{
   float mAccelerationRandomWalkSpectralDensity;
};

// Structure to describe response TLV 0x18 for PDSGetSensorNavigation()
struct sPDSGetSensorNavigationResponse_AngleRWSD
{
   float mAngleRandomWalkSpectralDensity;
};

// Structure to describe response TLV 0x19 for PDSGetSensorNavigation()
struct sPDSGetSensorNavigationResponse_RateRWSD
{
   float mRateRandomWalkSpectralDensity;
};

// Structure to describe response TLV 0x1A for PDSGetSensorNavigation()
struct sPDSGetSensorNavigationResponse_SensorAlgorithmConfig
{
   bool mDisableINSPositioningFilter:1;

   // Padding out 31 bits
   UINT8 mReserved1:7;
   UINT8 mReserved2[3];
};

// Structure to describe response TLV 0x1B for PDSGetSensorNavigation()
struct sPDSGetSensorNavigationResponse_MaximumDeadReckoningTime
{
   UINT32 mMaximumDeadReckoningTimeSeconds;
};

// Structure to describe response TLV 0x1C for PDSGetSensorNavigation()
struct sPDSGetSensorNavigationResponse_INSFilterLowThreshold
{
   UINT8 mINSFilterCrossCheckSigmaLowThreshold;
};

// Structure to describe response TLV 0x1D for PDSGetSensorNavigation()
struct sPDSGetSensorNavigationResponse_INSFilterHighThreshold
{
   UINT8 mINSFilterCrossCheckSigmaHighThreshold;
};

// Structure to describe request TLV 0x10 for PDSSetNavigationConfig()
struct sPDSSetNavigationConfigRequest_GyroDataVariance
{
   float mGyroDataVariance;
};

// Structure to describe request TLV 0x11 for PDSSetNavigationConfig()
struct sPDSSetNavigationConfigRequest_GyroBiasRandomWalk
{
   float mGyroBiasRandomWalk;
};

// Structure to describe request TLV 0x12 for PDSSetNavigationConfig()
struct sPDSSetNavigationConfigRequest_VirtualGyroCravityAveragingLength
{
   UINT32 mVirtualGyroGravityAveragingLengthMilliseconds;
};

// Structure to describe request TLV 0x13 for PDSSetNavigationConfig()
struct sPDSSetNavigationConfigRequest_HeadingFilterMode
{
   INT8 mHeadingFilterMode;
};

// Structure to describe request TLV 0x14 for PDSSetNavigationConfig()
struct sPDSSetNavigationConfigRequest_VelocityConstraintVariance
{
   float mVelocityConstraintVariance;
};

// Structure to describe request TLV 0x15 for PDSSetNavigationConfig()
struct sPDSSetNavigationConfigRequest_PositioningMethod
{
   bool mSuppressInjectedPosition:1;
   bool mReportUsingInstantaneousInformation:1;
   bool mDisablePersistentMemoryStorage:1;

   // Padding out 29 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2[3];
};

// Structure to describe request TLV 0x16 for PDSSetSensorNavigation()
struct sPDSSetSensorNavigationRequest_VelocityRWSD
{
   float mVelocityRandomWalkSpectralDensity;
};

// Structure to describe request TLV 0x17 for PDSSetSensorNavigation()
struct sPDSSetSensorNavigationRequest_AccelerationRWSD
{
   float mAccelerationRandomWalkSpectralDensity;
};

// Structure to describe request TLV 0x18 for PDSSetSensorNavigation()
struct sPDSSetSensorNavigationRequest_AngleRWSD
{
   float mAngleRandomWalkSpectralDensity;
};

// Structure to describe request TLV 0x19 for PDSSetSensorNavigation()
struct sPDSSetSensorNavigationRequest_RateRWSD
{
   float mRateRandomWalkSpectralDensity;
};

// Structure to describe request TLV 0x1A for PDSSetSensorNavigation()
struct sPDSSetSensorNavigationRequest_SensorAlgorithmConfig
{
   bool mDisableINSPositioningFilter:1;

   // Padding out 31 bits
   UINT8 mReserved1:7;
   UINT8 mReserved2[3];
};

// Structure to describe request TLV 0x1B for PDSSetSensorNavigation()
struct sPDSSetSensorNavigationRequest_MaximumDeadReckoningTime
{
   UINT32 mMaximumDeadReckoningTimeSeconds;
};

// Structure to describe request TLV 0x1C for PDSSetSensorNavigation()
struct sPDSSetSensorNavigationRequest_INSFilterLowThreshold
{
   UINT8 mINSFilterCrossCheckSigmaLowThreshold;
};

// Structure to describe request TLV 0x1D for PDSSetSensorNavigation()
struct sPDSSetSensorNavigationRequest_INSFilterHighThreshold
{
   UINT8 mINSFilterCrossCheckSigmaHighThreshold;
};

// Structure to describe request TLV 0x10 for PDSSetWLANBlanking()
struct sPDSSetWLANBlankingRequest_BlankingEnable
{
   eQMIPDSBlankingEnable mBlankingEnable;
};

// Structure to describe request TLV 0x11 for PDSSetWLANBlanking()
struct sPDSSetWLANBlankingRequest_ActiveChannels
{
   bool mChannel1:1;
   bool mChannel2:1;
   bool mChannel3:1;
   bool mChannel4:1;
   bool mChannel5:1;
   bool mChannel6:1;
   bool mChannel7:1;
   bool mChannel8:1;
   bool mChannel9:1;
   bool mChannel10:1;
   bool mChannel11:1;
   bool mChannel12:1;
   bool mChannel13:1;
   bool mChannel14:1;

   // Padding out 18 bits
   UINT8 mReserved1:2;
   UINT8 mReserved2[2];
};

// Structure to describe request TLV 0x12 for PDSSetWLANBlanking()
struct sPDSSetWLANBlankingRequest_BluetoothJamming
{
   INT8 mBluetoothJammingActive;
};

// Structure to describe request TLV 0x13 for PDSSetWLANBlanking()
struct sPDSSetWLANBlankingRequest_IMDJamming
{
   eQMIPDSIMDJammingStates mSimulateIMDJamming;
};

// Structure to describe request TLV 0x14 for PDSSetWLANBlanking()
struct sPDSSetWLANBlankingRequest_JammingBands
{
   eQMIPDSIMDJammingBands mBandstoJam;
};

// Structure to describe request TLV 0x10 for PDSSetSecurityChallengeReport()
struct sPDSSetSecurityChallengeReportRequest_EnableSecurity
{
   eQMIPDSReportSecurityChallenge mReportSecurityChallenge;
};

// Structure to describe indication TLV 0x10 for PDS SetSecurityChallengeReportIndication
struct sPDSSetSecurityChallengeReportIndication_EnableSecurity
{
   eQMIPDSEncryptionAlgorithm mEncryptionAlgorithm;
   UINT8 mEncryptionDataID;
   UINT8 mEncryptionDataLength;

   // This array must be the size specified by mEncryptionDataLength
   // UINT8 mEncryptionData[1];
};

// Structure to describe request TLV 0x10 for PDSSetSecurityChallenge()
struct sPDSSetSecurityChallengeRequest_SecureHashAlgorithm
{
   UINT8 mEncryptionDataID;
   UINT8 mEncryptionDataLength;

   // This array must be the size specified by mEncryptionDataLength
   // UINT8 mEncryptionData[1];
};

// Structure to describe response TLV 0x10 for PDSGetSecurityEncryptionConfig()
struct sPDSGetSecurityEncryptionConfigResponse_SecureClientTimestamp
{
   eQMIPDSEncryptionAlgorithm mEncryptionAlgorithm;
   UINT64 mEncryptionTimestampMilliseconds;
   UINT8 mEncryptionDataLength;

   // This array must be the size specified by mEncryptionDataLength
   // UINT8 mEncryptionData[1];
};

// Structure to describe request TLV 0x10 for PDSSetSecurityUpdateRate()
struct sPDSSetSecurityUpdateRateRequest_EnableSecurity
{
   UINT8 mUpdateRate;
};

// Structure to describe request TLV 0x10 for PDSSetCellDatabaseControl()
struct sPDSSetCellDatabaseControlRequest_Control
{
   bool mPositionInfoStorage:1;
   bool mTimeInfoStorage:1;

   // Padding out 30 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[3];
};

// Structure to describe request TLV 0x01 for PDSInjectMotionData()
struct sPDSInjectMotionDataRequest_MotionData
{
   eQMIPDSMotionStates mMotionState;
   eQMIPDSMotionModes mMotionMode;
   eQMIPDSMotionSubmodes mMotionSubmode;
   float mStateProbability;
   UINT16 mMotionDataAgeMilliseconds;
   UINT16 mMotionDataTimoutMilliseconds;
};

// Structure to describe request TLV 0x01 for PDSSetGNSSEngineErrorRecoveryReport()
struct sPDSSetGNSSEngineErrorRecoveryReportRequest_Indicator
{
   INT8 mReportGNSSEngineErrorRecovery;
};

// Structure to describe indication TLV 0x01 for PDS GNSSEngineErrorRecoveryReport
struct sPDSGNSSEngineErrorRecoveryReportIndication_Reason
{
   eQMIPDSResetReasons mResetReason;
};

// Structure to describe indication TLV 0x10 for PDS GNSSEngineErrorRecoveryReport
struct sPDSGNSSEngineErrorRecoveryReportIndication_CurrentSystemTime
{
   UINT16 mGPSWeekNumber;
   UINT32 mGPSTimeOfWeekMilliseconds;
};

// Structure to describe indication TLV 0x11 for PDS GNSSEngineErrorRecoveryReport
struct sPDSGNSSEngineErrorRecoveryReportIndication_ScanParameters
{
   eQMIPDSSVSystems mSystem;
   UINT8 mPRN;
   UINT16 mCN0;
   UINT16 mSubMillisecondDifferenceMicroseconds;
   INT16 mDopplerDifferenceHz;
};

// Structure to describe indication TLV 0x12 for PDS GNSSEngineErrorRecoveryReport
struct sPDSGNSSEngineErrorRecoveryReportIndication_ClockParameters
{
   eQMIPDSSVSystems mSystem;
   eQMIPDSTimeSource mCurrentTimeSource;
   eQMIPDSTimeSource mInjectedTimeSource;
   UINT32 mTimeDifferenceMilliseconds;
   UINT32 mTimeUncertaintyMicroseconds;
};

// Structure to describe indication TLV 0x13 for PDS GNSSEngineErrorRecoveryReport
struct sPDSGNSSEngineErrorRecoveryReportIndication_SubframeParameters
{
   UINT8 mPRN;
   INT32 mGPSDataDemodulationINProgress;
   UINT16 mObservedCycleSlips;
   UINT16 mParityErrors;
   UINT32 mGPSSubframeWordWithTimingError;
};

// Structure to describe indication TLV 0x14 for PDS GNSSEngineErrorRecoveryReport
struct sPDSGNSSEngineErrorRecoveryReportIndication_TimeParameters
{
   eQMIPDSSVSystems mSystem;
   UINT8 mPRN;
   UINT16 mGPSWeekNumber;
   UINT32 mGPSTimeOfWeekMilliseconds;
   UINT16 mGNSSNumberOfDays;
   UINT32 mGNSSMilliseconds;
   UINT32 mClockDifferenceMilliseconds;
};

// Structure to describe indication TLV 0x15 for PDS GNSSEngineErrorRecoveryReport
struct sPDSGNSSEngineErrorRecoveryReportIndication_MeasurementParameters
{
   eQMIPDSSVSystems mSystem;
   UINT16 mGPSWeekNumber;
   UINT32 mGPSTimeOfWeekMilliseconds;
   UINT8 mObservedMeasurementInconsistencies;
   UINT16 mObservationDuration;
};

// Structure to describe indication TLV 0x16 for PDS GNSSEngineErrorRecoveryReport
struct sPDSGNSSEngineErrorRecoveryReportIndication_RFParameters
{
   UINT32 mQualitySignalStrength;
   UINT32 mResetSignalStrength;
};

// Structure to describe request TLV 0x10 for PDSInjectTestData()
struct sPDSInjectTestDataRequest_TimeUncertainty
{
   UINT32 mTimeUncertaintyMicroseconds;
};

// Structure to describe request TLV 0x11 for PDSInjectTestData()
struct sPDSInjectTestDataRequest_PositionUncertainty
{
   UINT32 mPositionUncertaintyCentimeters;
};

// Structure to describe request TLV 0x12 for PDSInjectTestData()
struct sPDSInjectTestDataRequest_TimeOffset
{
   UINT64 mTimeOffsetMicroseconds;
};

// Structure to describe request TLV 0x13 for PDSInjectTestData()
struct sPDSInjectTestDataRequest_PositionOffset
{
   UINT32 mPositionOffsetMeters;
};

// Structure to describe request TLV 0x01 for PDSSetGNSSRFConfig()
struct sPDSSetGNSSRFConfigRequest_PeakAntennaGain
{
   INT16 mPeakAntennaGain;
};

// Structure to describe request TLV 0x10 for PDSSetGNSSRFConfig()
struct sPDSSetGNSSRFConfigRequest_GPSRFLoss
{
   UINT16 mRFLoss;
};

// Structure to describe request TLV 0x11 for PDSSetGNSSRFConfig()
struct sPDSSetGNSSRFConfigRequest_GlonassRFLoss
{
   UINT16 mRFLossFrequency0;
   UINT16 mRFLossFrequencyMinus7;
   UINT16 mRFLossFrequencyPlus6;
};

// Structure to describe request TLV 0x10 for AUTHStartEAPSession()
struct sAUTHStartEAPSessionRequest_MethodMask
{
   bool mEAPSIM:1;
   bool mEAPAKA:1;

   // Padding out 30 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[3];
};

// Structure to describe request TLV 0x11 for AUTHStartEAPSession()
struct sAUTHStartEAPSessionRequest_UserID
{
   UINT8 mUserIDLength;

   // This array must be the size specified by mUserIDLength
   // UINT8 mUserID[1];
};

// Structure to describe request TLV 0x12 for AUTHStartEAPSession()
struct sAUTHStartEAPSessionRequest_MetaIdentity
{
   UINT8 mMetaIdentityLength;

   // This array must be the size specified by mMetaIdentityLength
   // UINT8 mMetaIdentity[1];
};

// Structure to describe request TLV 0x13 for AUTHStartEAPSession()
struct sAUTHStartEAPSessionRequest_SIMAKAAlgorithm
{
   eQMIAUTHSIMAKAAlgorithm mSIMAKAAlgorithm;
};

// Structure to describe request TLV 0x01 for AUTHSendEAPPacket()
struct sAUTHSendEAPPacketRequest_RequestPacket
{
   UINT8 mEAPRequestPacket[9999];
};

// Structure to describe response TLV 0x01 for AUTHSendEAPPacket()
struct sAUTHSendEAPPacketResponse_ResponsePacket
{
   UINT8 mResponsePacket[9999];
};

// Structure to describe indication TLV 0x01 for AUTH EAPSessionResultIndication
struct sAUTHEAPSessionResultIndication_Result
{
   eQMIAUTHEAPResult mResult;
};

// Structure to describe response TLV 0x01 for AUTHGetEAPSessionKeys()
struct sAUTHGetEAPSessionKeysResponse_SessionKeys
{
   UINT8 mSessionKeys[9999];
};

// Structure to describe request TLV 0x01 for AUTHRunAKA()
struct sAUTHRunAKARequest_Version
{
   eQMIAUTHAKAVersion mAKAVersion;
};

// Structure to describe request TLV 0x10 for AUTHRunAKA()
struct sAUTHRunAKARequest_Parameters1
{
   UINT8 mRandomChallengeLength;

   // This array must be the size specified by mRandomChallengeLength
   // UINT8 mRandomChallenge[1];
};

struct sAUTHRunAKARequest_Parameters2
{
   UINT8 mAuthenticationTokenLength;

   // This array must be the size specified by mAuthenticationTokenLength
   // UINT8 mAuthenticationToken[1];
};

struct sAUTHRunAKARequest_Parameters
{
   sAUTHRunAKARequest_Parameters1 mAUTHRunAKARequest_Parameters1;
   sAUTHRunAKARequest_Parameters2 mAUTHRunAKARequest_Parameters2;
};

// Structure to describe response TLV 0x01 for AUTHRunAKA()
struct sAUTHRunAKAResponse_Handle
{
   UINT32 mHandle;
};

// Structure to describe indication TLV 0x01 for AUTH AKAResultIndication
struct sAUTHAKAResultIndication_Result
{
   UINT32 mHandle;
   eQMIAUTHAKAResult mResult;
};

// Structure to describe indication TLV 0x10 for AUTH AKAResultIndication
struct sAUTHAKAResultIndication_V1V2ResponseData1
{
   UINT8 mDigestLength;

   // This array must be the size specified by mDigestLength
   // UINT8 mDigest[1];
};

struct sAUTHAKAResultIndication_V1V2ResponseData2
{
   UINT8 mAKADataLength;

   // This array must be the size specified by mAKADataLength
   // UINT8 mAKAData[1];
};

struct sAUTHAKAResultIndication_V1V2ResponseData
{
   sAUTHAKAResultIndication_V1V2ResponseData1 mAUTHAKAResultIndication_V1V2ResponseData1;
   sAUTHAKAResultIndication_V1V2ResponseData2 mAUTHAKAResultIndication_V1V2ResponseData2;
};

// Structure to describe request TLV 0x10 for VoiceIndicationRegistration()
struct sVoiceIndicationRegistrationRequest_DTMF
{
   INT8 mEnableEvents;
};

// Structure to describe request TLV 0x11 for VoiceIndicationRegistration()
struct sVoiceIndicationRegistrationRequest_VoicePrivacy
{
   INT8 mEnableEvents;
};

// Structure to describe request TLV 0x12 for VoiceIndicationRegistration()
struct sVoiceIndicationRegistrationRequest_SupplementaryService
{
   INT8 mEnableEvents;
};

// Structure to describe request TLV 0x13 for VoiceIndicationRegistration()
struct sVoiceIndicationRegistrationRequest_CallNotification
{
   INT8 mEnableEvents;
};

// Structure to describe request TLV 0x14 for VoiceIndicationRegistration()
struct sVoiceIndicationRegistrationRequest_Handover
{
   INT8 mEnableEvents;
};

// Structure to describe request TLV 0x15 for VoiceIndicationRegistration()
struct sVoiceIndicationRegistrationRequest_SpeechCodec
{
   INT8 mEnableEvents;
};

// Structure to describe request TLV 0x16 for VoiceIndicationRegistration()
struct sVoiceIndicationRegistrationRequest_USSDNotification
{
   INT8 mEnableEvents;
};

// Structure to describe request TLV 0x17 for VoiceIndicationRegistration()
struct sVoiceIndicationRegistrationRequest_Supplementary
{
   INT8 mEnableEvents;
};

// Structure to describe request TLV 0x18 for VoiceIndicationRegistration()
struct sVoiceIndicationRegistrationRequest_Modification
{
   INT8 mEnableEvents;
};

// Structure to describe request TLV 0x19 for VoiceIndicationRegistration()
struct sVoiceIndicationRegistrationRequest_UUS
{
   INT8 mEnableEvents;
};

// Structure to describe request TLV 0x1A for VoiceIndicationRegistration()
struct sVoiceIndicationRegistrationRequest_AOC
{
   INT8 mEnableEvents;
};

// Structure to describe request TLV 0x01 for VoiceCallOriginate()
struct sVoiceCallOriginateRequest_CallingNumber
{
   // String is variable length, but must be size of the container
   // char mCallingNumber[1];
};

// Structure to describe request TLV 0x10 for VoiceCallOriginate()
struct sVoiceCallOriginateRequest_CallType
{
   eQMIVoiceCallTypes mCallType;
};

// Structure to describe request TLV 0x11 for VoiceCallOriginate()
struct sVoiceCallOriginateRequest_CLIR
{
   eQMIVoiceCLIRTypes mCLIRType;
};

// Structure to describe request TLV 0x12 for VoiceCallOriginate()
struct sVoiceCallOriginateRequest_UUS
{
   eQMIVoiceUUSTypes mUUSType;
   eQMIVoiceUUSDataCodingSchemes mUUSDCS;
   UINT8 mUUSLength;

   // This array must be the size specified by mUUSLength
   // UINT8 mUUSData[1];
};

// Structure to describe request TLV 0x13 for VoiceCallOriginate()
struct sVoiceCallOriginateRequest_CUG
{
   UINT16 mCUGIndex;
   INT8 mSuppressPreferentialCUG;
   INT8 mSuppressOASubscription;
};

// Structure to describe request TLV 0x14 for VoiceCallOriginate()
struct sVoiceCallOriginateRequest_EmergencyCategory
{
   bool mPolice:1;
   bool mAmbulance:1;
   bool mFireBrigade:1;
   bool mMarineGuard:1;
   bool mMountainRescue:1;
   bool mManualCall:1;
   bool mAutomaticCall:1;
   bool mReserved:1;
};

// Structure to describe request TLV 0x15 for VoiceCallOriginate()
struct sVoiceCallOriginateRequest_CalledPartySubaddress
{
   eQMIVoiceSubaddressTypes mSubaddressType;
   eQMIVoiceEvenOddIndicators mAddressSignals;
   UINT8 mSubaddressLength;

   // This array must be the size specified by mSubaddressLength
   // UINT8 mSubaddress[1];
};

// Structure to describe request TLV 0x16 for VoiceCallOriginate()
struct sVoiceCallOriginateRequest_ServiceType
{
   eQMIVoiceServiceTypes mServiceType;
};

// Structure to describe request TLV 0x17 for VoiceCallOriginate()
struct sVoiceCallOriginateRequest_SIPURIOverflow
{
   // String is variable length, but must be size of the container
   // char mSIPURIOverflow[1];
};

// Structure to describe request TLV 0x18 for VoiceCallOriginate()
struct sVoiceCallOriginateRequest_AudioAttribute
{
   bool mVoiceCallTX:1;
   bool mVoiceCallRX:1;

   // Padding out 62 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[7];
};

// Structure to describe request TLV 0x19 for VoiceCallOriginate()
struct sVoiceCallOriginateRequest_VideoAttribute
{
   bool mVoiceCallTX:1;
   bool mVoiceCallRX:1;

   // Padding out 62 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[7];
};

// Structure to describe response TLV 0x10 for VoiceCallOriginate()
struct sVoiceCallOriginateResponse_CallID
{
   UINT8 mCallID;
};

// Structure to describe response TLV 0x11 for VoiceCallOriginate()
struct sVoiceCallOriginateResponse_AlphaID
{
   eQMIVoiceUSSDAlphaCodingSchemes mAlphaDCS;
   UINT8 mAlphaLength;

   // This array must be the size specified by mAlphaLength
   // UINT8 mAlphaData[1];
};

// Structure to describe response TLV 0x12 for VoiceCallOriginate()
struct sVoiceCallOriginateResponse_CallControl
{
   eQMIVoiceCallControlResultTypes mCallControlResult;
};

// Structure to describe response TLV 0x13 for VoiceCallOriginate()
struct sVoiceCallOriginateResponse_SupplementaryService
{
   eQMIVoiceSupplementaryServiceRequests mSupplementaryService;
   eQMIVoiceSupplementaryServiceReasons mReason;
};

// Structure to describe response TLV 0x14 for VoiceCallOriginate()
struct sVoiceCallOriginateResponse_EndReason
{
   eQMIVoiceEndReasons mEndReason;
};

// Structure to describe request TLV 0x01 for VoiceCallEnd()
struct sVoiceCallEndRequest_CallID
{
   UINT8 mCallID;
};

// Structure to describe response TLV 0x10 for VoiceCallEnd()
struct sVoiceCallEndResponse_CallID
{
   UINT8 mCallID;
};

// Structure to describe request TLV 0x01 for VoiceCallAnswer()
struct sVoiceCallAnswerRequest_CallID
{
   UINT8 mCallID;
};

// Structure to describe request TLV 0x10 for VoiceCallAnswer()
struct sVoiceCallAnswerRequest_CallType
{
   eQMIVoiceCallTypes mCallType;
};

// Structure to describe request TLV 0x11 for VoiceCallAnswer()
struct sVoiceCallAnswerRequest_AudioAttribute
{
   bool mVoiceCallTX:1;
   bool mVoiceCallRX:1;

   // Padding out 62 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[7];
};

// Structure to describe request TLV 0x12 for VoiceCallAnswer()
struct sVoiceCallAnswerRequest_VideoAttribute
{
   bool mVoiceCallTX:1;
   bool mVoiceCallRX:1;

   // Padding out 62 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[7];
};

// Structure to describe response TLV 0x10 for VoiceCallAnswer()
struct sVoiceCallAnswerResponse_CallID
{
   UINT8 mCallID;
};

// Structure to describe request TLV 0x01 for VoiceGetCallInfo()
struct sVoiceGetCallInfoRequest_CallID
{
   UINT8 mCallID;
};

// Structure to describe response TLV 0x10 for VoiceGetCallInfo()
struct sVoiceGetCallInfoResponse_CallInfo
{
   UINT8 mCallID;
   eQMIVoiceCallStates mCallState;
   eQMIVoiceCallTypes mCallType;
   eQMIVoiceCallDirections mDirection;
   eQMIVoiceCallModes mMode;
};

// Structure to describe response TLV 0x11 for VoiceGetCallInfo()
struct sVoiceGetCallInfoResponse_RemotePartyNumber
{
   eQMIVoicePresentationIndicators mPresentationIndicator;
   UINT8 mNumberLength;

   // This array must be the size specified by mNumberLength
   // char mNumber[1];
};

// Structure to describe response TLV 0x12 for VoiceGetCallInfo()
struct sVoiceGetCallInfoResponse_ServiceOption
{
   UINT16 mServiceOption;
};

// Structure to describe response TLV 0x13 for VoiceGetCallInfo()
struct sVoiceGetCallInfoResponse_VoicePrivacy
{
   eQMIVoicePrivacyLevels mVoicePrivacy;
};

// Structure to describe response TLV 0x14 for VoiceGetCallInfo()
struct sVoiceGetCallInfoResponse_OTASPStatus
{
   eQMIVoiceOTASPStati mOTASPStatus;
};

// Structure to describe response TLV 0x15 for VoiceGetCallInfo()
struct sVoiceGetCallInfoResponse_RemotePartyName
{
   eQMIVoicePresentationIndicators mPresentationIndicator;
   UINT8 mCodingScheme;
   UINT8 mCallerNameLength;

   // This array must be the size specified by mCallerNameLength
   // UINT8 mCallerName[1];
};

// Structure to describe response TLV 0x16 for VoiceGetCallInfo()
struct sVoiceGetCallInfoResponse_UUS
{
   eQMIVoiceUUSTypes mUUSType;
   eQMIVoiceUUSDataCodingSchemes mUUSDCS;
   UINT8 mUUSLength;

   // This array must be the size specified by mUUSLength
   // UINT8 mUUSData[1];
};

// Structure to describe response TLV 0x17 for VoiceGetCallInfo()
struct sVoiceGetCallInfoResponse_AlertingType
{
   eQMIVoiceAlertingTypes mAlertingType;
};

// Structure to describe response TLV 0x18 for VoiceGetCallInfo()
struct sVoiceGetCallInfoResponse_AlphaID
{
   eQMIVoiceUSSDAlphaCodingSchemes mAlphaDCS;
   UINT8 mAlphaLength;

   // This array must be the size specified by mAlphaLength
   // UINT8 mAlphaData[1];
};

// Structure to describe response TLV 0x19 for VoiceGetCallInfo()
struct sVoiceGetCallInfoResponse_ConnectedNumberInfo
{
   UINT8 mPresentationIndicator;
   eQMIVoiceScreeningIndicators mScreeningIndicator;
   eQMIVoiceNumberTypes mNumberType;
   eQMIVoiceNumberPlans mNumberPlan;
   UINT8 mNumberLength;

   // This array must be the size specified by mNumberLength
   // char mNumber[1];
};

// Structure to describe response TLV 0x1A for VoiceGetCallInfo()
struct sVoiceGetCallInfoResponse_DiagnosticInfo
{
   UINT8 mDiagnosticInfoLength;

   // This array must be the size specified by mDiagnosticInfoLength
   // UINT8 mDiagnosticInfo[1];
};

// Structure to describe response TLV 0x1B for VoiceGetCallInfo()
struct sVoiceGetCallInfoResponse_AlertingPattern
{
   eQMIVoiceAlertingPatterns mAlertingPattern;
};

// Structure to describe response TLV 0x1C for VoiceGetCallInfo()
struct sVoiceGetCallInfoResponse_AudioAttribute
{
   bool mVoiceCallTX:1;
   bool mVoiceCallRX:1;

   // Padding out 62 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[7];
};

// Structure to describe response TLV 0x1D for VoiceGetCallInfo()
struct sVoiceGetCallInfoResponse_VideoAttribute
{
   bool mVoiceCallTX:1;
   bool mVoiceCallRX:1;

   // Padding out 62 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[7];
};

// Structure to describe indication TLV 0x10 for Voice OTASPIndication
struct sVoiceOTASPIndication_OTASPStatus
{
   UINT8 mCallID;
   eQMIVoiceOTASPStati mOTASPStatus;
};

// Structure to describe indication TLV 0x01 for Voice InfoRecordIndication
struct sVoiceInfoRecordIndication_CallID
{
   UINT8 mCallID;
};

// Structure to describe indication TLV 0x10 for Voice InfoRecordIndication
struct sVoiceInfoRecordIndication_Signal
{
   UINT8 mSignalType;
   UINT8 mAlertPitch;
   UINT8 mSignal;
};

// Structure to describe indication TLV 0x11 for Voice InfoRecordIndication
struct sVoiceInfoRecordIndication_CallerID
{
   UINT8 mPresentationIndicator;
   UINT8 mCallerIDLength;

   // This array must be the size specified by mCallerIDLength
   // char mCallerID[1];
};

// Structure to describe indication TLV 0x12 for Voice InfoRecordIndication
struct sVoiceInfoRecordIndication_Display
{
   // String is variable length, but must be size of the container
   // char mDisplayBuffer[1];
};

// Structure to describe indication TLV 0x13 for Voice InfoRecordIndication
struct sVoiceInfoRecordIndication_ExtendedDisplay
{
   UINT8 mExtendedDisplayBuffer[182];
};

// Structure to describe indication TLV 0x14 for Voice InfoRecordIndication
struct sVoiceInfoRecordIndication_CallerName
{
   // String is variable length, but must be size of the container
   // char mCallerName[1];
};

// Structure to describe indication TLV 0x15 for Voice InfoRecordIndication
struct sVoiceInfoRecordIndication_CallWaiting
{
   INT8 mNewCallWaiting;
};

// Structure to describe indication TLV 0x16 for Voice InfoRecordIndication
struct sVoiceInfoRecordIndication_ConnectedNumberInfo
{
   UINT8 mPresentationIndicator;
   eQMIVoiceScreeningIndicators mScreeningIndicator;
   eQMIVoiceNumberTypes mNumberType;
   eQMIVoiceNumberPlans mNumberPlan;
   UINT8 mNumberLength;

   // This array must be the size specified by mNumberLength
   // char mNumber[1];
};

// Structure to describe indication TLV 0x17 for Voice InfoRecordIndication
struct sVoiceInfoRecordIndication_CallingPartyNumberInfo
{
   UINT8 mPresentationIndicator;
   eQMIVoiceScreeningIndicators mScreeningIndicator;
   eQMIVoiceNumberTypes mNumberType;
   eQMIVoiceNumberPlans mNumberPlan;
   UINT8 mNumberLength;

   // This array must be the size specified by mNumberLength
   // char mNumber[1];
};

// Structure to describe indication TLV 0x18 for Voice InfoRecordIndication
struct sVoiceInfoRecordIndication_CalledPartyNumberInfo
{
   UINT8 mPresentationIndicator;
   eQMIVoiceScreeningIndicators mScreeningIndicator;
   eQMIVoiceNumberTypes mNumberType;
   eQMIVoiceNumberPlans mNumberPlan;
   UINT8 mNumberLength;

   // This array must be the size specified by mNumberLength
   // char mNumber[1];
};

// Structure to describe indication TLV 0x19 for Voice InfoRecordIndication
struct sVoiceInfoRecordIndication_RedirectingNumberInfo
{
   UINT8 mPresentationIndicator;
   eQMIVoiceScreeningIndicators mScreeningIndicator;
   eQMIVoiceNumberTypes mNumberType;
   eQMIVoiceNumberPlans mNumberPlan;
   UINT8 mRedirectReason;
   UINT8 mNumberLength;

   // This array must be the size specified by mNumberLength
   // char mNumber[1];
};

// Structure to describe indication TLV 0x1A for Voice InfoRecordIndication
struct sVoiceInfoRecordIndication_CLIRCause
{
   eQMIVoiceCLIRCauses mCLIRCause;
};

// Structure to describe indication TLV 0x1B for Voice InfoRecordIndication
struct sVoiceInfoRecordIndication_NSSAudio
{
   UINT8 mUpLink;
   UINT8 mDownLink;
};

// Structure to describe indication TLV 0x1C for Voice InfoRecordIndication
struct sVoiceInfoRecordIndication_NSSRelease
{
   eQMIVoiceNSSReleases mNSSRelease;
};

// Structure to describe indication TLV 0x1D for Voice InfoRecordIndication
struct sVoiceInfoRecordIndication_LineControlInfo
{
   INT8 mPolarityIncluded;
   INT8 mToggleMode;
   INT8 mReversePolarity;
   UINT8 mPowerDenialTime;
};

// Structure to describe indication TLV 0x1E for Voice InfoRecordIndication
struct sVoiceInfoRecordIndication_ExtendedDisplayInfo
{
   UINT8 mDisplayType;
   UINT8 mExtendedDisplayInfoLength;

   // This array must be the size specified by mExtendedDisplayInfoLength
   // UINT8 mExtendedDisplayInfo[1];
};

// Structure to describe request TLV 0x01 for VoiceSendFlash()
struct sVoiceSendFlashRequest_CallID
{
   UINT8 mCallID;
};

// Structure to describe request TLV 0x10 for VoiceSendFlash()
struct sVoiceSendFlashRequest_Payload
{
   // String is variable length, but must be size of the container
   // char mFlashPayload[1];
};

// Structure to describe request TLV 0x11 for VoiceSendFlash()
struct sVoiceSendFlashRequest_Type
{
   eQMIVoiceFlashTypes mFlashType;
};

// Structure to describe response TLV 0x10 for VoiceSendFlash()
struct sVoiceSendFlashResponse_CallID
{
   UINT8 mCallID;
};

// Structure to describe request TLV 0x01 for VoiceBurstDTMF()
struct sVoiceBurstDTMFRequest_Info
{
   UINT8 mCallID;
   UINT8 mDTMFBufferLength;

   // This array must be the size specified by mDTMFBufferLength
   // char mDTMFBuffer[1];
};

// Structure to describe request TLV 0x10 for VoiceBurstDTMF()
struct sVoiceBurstDTMFRequest_Lengths
{
   eQMIVoicePulseWidths mDTMFPulseWidth;
   eQMIVoiceInterdigitIntervals mDTMFInterdigitInterval;
};

// Structure to describe request TLV 0x01 for VoiceStartContinuousDTMF()
struct sVoiceStartContinuousDTMFRequest_Info
{
   UINT8 mCallID;
   char mDTMFDigit;
};

// Structure to describe response TLV 0x10 for VoiceStartContinuous()
struct sVoiceStartContinuousResponse_CallID
{
   UINT8 mCallID;
};

// Structure to describe request TLV 0x01 for VoiceStopContinuousDTMF()
struct sVoiceStopContinuousDTMFRequest_CallID
{
   UINT8 mCallID;
};

// Structure to describe response TLV 0x10 for VoiceStopContinuous()
struct sVoiceStopContinuousResponse_CallID
{
   UINT8 mCallID;
};

// Structure to describe indication TLV 0x01 for Voice DTMFIndication
struct sVoiceDTMFIndication_DTMFInfo
{
   UINT8 mCallID;
   eQMIVoiceDTMFEvents mDTMFEvent;
   UINT8 mDTMFBufferLength;

   // This array must be the size specified by mDTMFBufferLength
   // char mDTMFBuffer[1];
};

// Structure to describe indication TLV 0x10 for Voice DTMFIndication
struct sVoiceDTMFIndication_Lengths
{
   eQMIVoicePulseWidths mDTMFPulseWidth;
   eQMIVoiceInterdigitIntervals mDTMFInterdigitInterval;
};

// Structure to describe request TLV 0x01 for VoiceSetPreferredPrivacy()
struct sVoiceSetPreferredPrivacyRequest_Preference
{
   eQMIVoiceDTMFPrivacyLevels mPrivacyLevel;
};

// Structure to describe indication TLV 0x01 for Voice PrivacyIndication
struct sVoicePrivacyIndication_DTMFInfo
{
   UINT8 mCallID;
   eQMIVoiceDTMFPrivacyLevels mPrivacyLevel;
};

// Structure to describe indication TLV 0x01 for Voice AllCallStatusIndication
struct sVoiceAllCallStatusIndication_CallInfo
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      eQMIVoiceCallStates mCallState;
      eQMIVoiceCallTypes mCallType;
      eQMIVoiceCallDirections mDirection;
      eQMIVoiceCallModes mMode;
      INT8 mMultiparty;
      eQMIVoiceALSLineIndicators mALSLineIndicator;
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe indication TLV 0x10 for Voice AllCallStatusIndication
struct sVoiceAllCallStatusIndication_RemotePartyNumber
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      eQMIVoicePresentationIndicators mPresentationIndicator;
      UINT8 mNumberLength;
   
      // This array must be the size specified by mNumberLength
      // char mNumber[1];
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe indication TLV 0x11 for Voice AllCallStatusIndication
struct sVoiceAllCallStatusIndication_RemotePartyName
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      eQMIVoicePresentationIndicators mPresentationIndicator;
      UINT8 mCodingScheme;
      UINT8 mCallerNameLength;
   
      // This array must be the size specified by mCallerNameLength
      // UINT8 mCallerName[1];
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe indication TLV 0x12 for Voice AllCallStatusIndication
struct sVoiceAllCallStatusIndication_AlertingType
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      eQMIVoiceAlertingTypes mAlertingType;
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe indication TLV 0x13 for Voice AllCallStatusIndication
struct sVoiceAllCallStatusIndication_ServiceOption
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      UINT16 mServiceOption;
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe indication TLV 0x14 for Voice AllCallStatusIndication
struct sVoiceAllCallStatusIndication_CallEndReason
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      eQMIVoiceEndReasons mEndReason;
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe indication TLV 0x15 for Voice AllCallStatusIndication
struct sVoiceAllCallStatusIndication_AlphaID
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      eQMIVoiceUSSDAlphaCodingSchemes mAlphaDCS;
      UINT8 mAlphaLength;
   
      // This array must be the size specified by mAlphaLength
      // UINT8 mAlphaData[1];
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe indication TLV 0x16 for Voice AllCallStatusIndication
struct sVoiceAllCallStatusIndication_ConnectedNumberInfo
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      UINT8 mPresentationIndicator;
      eQMIVoiceScreeningIndicators mScreeningIndicator;
      eQMIVoiceNumberTypes mNumberType;
      eQMIVoiceNumberPlans mNumberPlan;
      UINT8 mNumberLength;
   
      // This array must be the size specified by mNumberLength
      // char mNumber[1];
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe indication TLV 0x17 for Voice AllCallStatusIndication
struct sVoiceAllCallStatusIndication_DiagnosticInfo
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      UINT8 mDiagnosticInfoLength;
   
      // This array must be the size specified by mDiagnosticInfoLength
      // UINT8 mDiagnosticInfo[1];
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe indication TLV 0x18 for Voice AllCallStatusIndication
struct sVoiceAllCallStatusIndication_CalledPartyNumberInfo
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      UINT8 mPresentationIndicator;
      eQMIVoiceScreeningIndicators mScreeningIndicator;
      eQMIVoiceNumberTypes mNumberType;
      eQMIVoiceNumberPlans mNumberPlan;
      UINT8 mNumberLength;
   
      // This array must be the size specified by mNumberLength
      // char mNumber[1];
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe indication TLV 0x19 for Voice AllCallStatusIndication
struct sVoiceAllCallStatusIndication_RedirectingNumberInfo
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      UINT8 mPresentationIndicator;
      eQMIVoiceScreeningIndicators mScreeningIndicator;
      eQMIVoiceNumberTypes mNumberType;
      eQMIVoiceNumberPlans mNumberPlan;
      UINT8 mNumberLength;
   
      // This array must be the size specified by mNumberLength
      // char mNumber[1];
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe indication TLV 0x1A for Voice AllCallStatusIndication
struct sVoiceAllCallStatusIndication_AlertingPattern
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      eQMIVoiceAlertingPatterns mAlertingPattern;
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe indication TLV 0x1B for Voice AllCallStatusIndication
struct sVoiceAllCallStatusIndication_AudioAttributesArray
{
   UINT8 mAttributesCount;

   struct sAttributes
   {
      UINT8 mCallID;
      bool mVoiceCallTX:1;
      bool mVoiceCallRX:1;
   
      // Padding out 62 bits
      UINT8 mReserved1:6;
      UINT8 mReserved2[7];
   };

   // This array must be the size specified by mAttributesCount
   // sAttributes mAttributeses[1];
};

// Structure to describe indication TLV 0x1C for Voice AllCallStatusIndication
struct sVoiceAllCallStatusIndication_VideoAttributesArray
{
   UINT8 mAttributesCount;

   struct sAttributes
   {
      UINT8 mCallID;
      bool mVoiceCallTX:1;
      bool mVoiceCallRX:1;
   
      // Padding out 62 bits
      UINT8 mReserved1:6;
      UINT8 mReserved2[7];
   };

   // This array must be the size specified by mAttributesCount
   // sAttributes mAttributeses[1];
};

// Structure to describe response TLV 0x10 for VoiceGetAllCallInfo()
struct sVoiceGetAllCallInfoResponse_CallInfo
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      eQMIVoiceCallStates mCallState;
      eQMIVoiceCallTypes mCallType;
      eQMIVoiceCallDirections mDirection;
      eQMIVoiceCallModes mMode;
      INT8 mMultiparty;
      eQMIVoiceALSLineIndicators mALSLineIndicator;
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x11 for VoiceGetAllCallInfo()
struct sVoiceGetAllCallInfoResponse_RemotePartyNumber
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      eQMIVoicePresentationIndicators mPresentationIndicator;
      UINT8 mNumberLength;
   
      // This array must be the size specified by mNumberLength
      // char mNumber[1];
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x12 for VoiceGetAllCallInfo()
struct sVoiceGetAllCallInfoResponse_RemotePartyName
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      eQMIVoicePresentationIndicators mPresentationIndicator;
      UINT8 mCodingScheme;
      UINT8 mCallerNameLength;
   
      // This array must be the size specified by mCallerNameLength
      // UINT8 mCallerName[1];
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x13 for VoiceGetAllCallInfo()
struct sVoiceGetAllCallInfoResponse_AlertingType
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      eQMIVoiceAlertingTypes mAlertingType;
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x14 for VoiceGetAllCallInfo()
struct sVoiceGetAllCallInfoResponse_UUS
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      eQMIVoiceUUSTypes mUUSType;
      eQMIVoiceUUSDataCodingSchemes mUUSDCS;
      UINT8 mUUSLength;
   
      // This array must be the size specified by mUUSLength
      // UINT8 mUUSData[1];
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x15 for VoiceGetAllCallInfo()
struct sVoiceGetAllCallInfoResponse_ServiceOption
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      UINT16 mServiceOption;
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x16 for VoiceGetAllCallInfo()
struct sVoiceGetAllCallInfoResponse_OTASPStatus
{
   eQMIVoiceOTASPStati mOTASPStatus;
};

// Structure to describe response TLV 0x17 for VoiceGetAllCallInfo()
struct sVoiceGetAllCallInfoResponse_VoicePrivacy
{
   eQMIVoicePrivacyLevels mVoicePrivacy;
};

// Structure to describe response TLV 0x18 for VoiceGetAllCallInfo()
struct sVoiceGetAllCallInfoResponse_CallEndReason
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      eQMIVoiceEndReasons mEndReason;
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x19 for VoiceGetAllCallInfo()
struct sVoiceGetAllCallInfoResponse_AlphaID
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      eQMIVoiceUSSDAlphaCodingSchemes mAlphaDCS;
      UINT8 mAlphaLength;
   
      // This array must be the size specified by mAlphaLength
      // UINT8 mAlphaData[1];
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x1A for VoiceGetAllCallInfo()
struct sVoiceGetAllCallInfoResponse_ConnectedNumberInfo
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      UINT8 mPresentationIndicator;
      eQMIVoiceScreeningIndicators mScreeningIndicator;
      eQMIVoiceNumberTypes mNumberType;
      eQMIVoiceNumberPlans mNumberPlan;
      UINT8 mNumberLength;
   
      // This array must be the size specified by mNumberLength
      // char mNumber[1];
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x1B for VoiceGetAllCallInfo()
struct sVoiceGetAllCallInfoResponse_DiagnosticInfo
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      UINT8 mDiagnosticInfoLength;
   
      // This array must be the size specified by mDiagnosticInfoLength
      // UINT8 mDiagnosticInfo[1];
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x1C for VoiceGetAllCallInfo()
struct sVoiceGetAllCallInfoResponse_CalledPartyNumberInfo
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      UINT8 mPresentationIndicator;
      eQMIVoiceScreeningIndicators mScreeningIndicator;
      eQMIVoiceNumberTypes mNumberType;
      eQMIVoiceNumberPlans mNumberPlan;
      UINT8 mNumberLength;
   
      // This array must be the size specified by mNumberLength
      // char mNumber[1];
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x1D for VoiceGetAllCallInfo()
struct sVoiceGetAllCallInfoResponse_RedirectingNumberInfo
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      UINT8 mPresentationIndicator;
      eQMIVoiceScreeningIndicators mScreeningIndicator;
      eQMIVoiceNumberTypes mNumberType;
      eQMIVoiceNumberPlans mNumberPlan;
      UINT8 mNumberLength;
   
      // This array must be the size specified by mNumberLength
      // char mNumber[1];
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x1E for VoiceGetAllCallInfo()
struct sVoiceGetAllCallInfoResponse_AlertingPattern
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      UINT8 mCallID;
      eQMIVoiceAlertingPatterns mAlertingPattern;
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x1F for VoiceGetAllCallInfo()
struct sVoiceGetAllCallInfoResponse_AudioAttributesArray
{
   UINT8 mAttributesCount;

   struct sAttributes
   {
      UINT8 mCallID;
      bool mVoiceCallTX:1;
      bool mVoiceCallRX:1;
   
      // Padding out 62 bits
      UINT8 mReserved1:6;
      UINT8 mReserved2[7];
   };

   // This array must be the size specified by mAttributesCount
   // sAttributes mAttributeses[1];
};

// Structure to describe response TLV 0x20 for VoiceGetAllCallInfo()
struct sVoiceGetAllCallInfoResponse_VideoAttributesArray
{
   UINT8 mAttributesCount;

   struct sAttributes
   {
      UINT8 mCallID;
      bool mVoiceCallTX:1;
      bool mVoiceCallRX:1;
   
      // Padding out 62 bits
      UINT8 mReserved1:6;
      UINT8 mReserved2[7];
   };

   // This array must be the size specified by mAttributesCount
   // sAttributes mAttributeses[1];
};

// Structure to describe request TLV 0x01 for VoiceManageCalls()
struct sVoiceManageCallsRequest_Info
{
   eQMIVoiceSupplementaryServiceCallTypes mSupplementaryServiceType;
};

// Structure to describe request TLV 0x10 for VoiceManageCalls()
struct sVoiceManageCallsRequest_CallID
{
   UINT8 mCallID;
};

// Structure to describe response TLV 0x10 for VoiceManageCalls()
struct sVoiceManageCallsResponse_FailureCause
{
   eQMIVoiceEndReasons mFailureCause;
};

// Structure to describe indication TLV 0x01 for Voice SupplementaryServiceIndication
struct sVoiceSupplementaryServiceIndication_Info
{
   UINT8 mCallID;
   eQMIVoiceSupplementaryNotificationTypes mNotificationType;
};

// Structure to describe indication TLV 0x10 for Voice SupplementaryServiceIndication
struct sVoiceSupplementaryServiceIndication_CUG
{
   UINT16 mCUGIndex;
};

// Structure to describe indication TLV 0x11 for Voice SupplementaryServiceIndication
struct sVoiceSupplementaryServiceIndication_ECT
{
   eQMIVoiceECTCallStates mECTCallState;
   eQMIVoicePresentationIndicators mPresentationIndicator;
   UINT8 mNumberLength;

   // This array must be the size specified by mNumberLength
   // char mNumber[1];
};

// Structure to describe request TLV 0x01 for VoiceSetSupplementaryService()
struct sVoiceSetSupplementaryServiceRequest_Info
{
   eQMIVoiceSupplementaryServiceTypes mService;
   eQMIVoiceSupplementaryServiceReasons mReason;
};

// Structure to describe request TLV 0x10 for VoiceSetSupplementaryService()
struct sVoiceSetSupplementaryServiceRequest_ServiceClass
{
   bool mSupplementaryServiceClassVoice:1;
   bool mSupplementaryServiceClassData:1;
   bool mSupplementaryServiceClassFax:1;
   bool mSupplementaryServiceClassSMS:1;
   bool mSupplementaryServiceClassDataCircuitSync:1;
   bool mSupplementaryServiceClassDataCircuitAsync:1;
   bool mSupplementaryServiceClassPacketAccess:1;
   bool mSupplementaryServiceClassPadAccess:1;
};

// Structure to describe request TLV 0x11 for VoiceSetSupplementaryService()
struct sVoiceSetSupplementaryServiceRequest_Password
{
   char mCallBarringPassword[4];
};

// Structure to describe request TLV 0x12 for VoiceSetSupplementaryService()
struct sVoiceSetSupplementaryServiceRequest_Number
{
   // String is variable length, but must be size of the container
   // char mCallForwardingNumber[1];
};

// Structure to describe request TLV 0x13 for VoiceSetSupplementaryService()
struct sVoiceSetSupplementaryServiceRequest_Timer
{
   UINT8 mNoReplyTimerSeconds;
};

// Structure to describe request TLV 0x14 for VoiceSetSupplementaryService()
struct sVoiceSetSupplementaryServiceRequest_CallForwarding
{
   eQMIVoiceNumberTypes mNumberType;
   eQMIVoiceNumberPlans mNumberPlan;
};

// Structure to describe request TLV 0x15 for VoiceSetSupplementaryService()
struct sVoiceSetSupplementaryServiceRequest_ExtendedServiceClass
{
   eQMIVoiceExtendedServiceClass mExtendedServiceClass;
};

// Structure to describe response TLV 0x10 for VoiceSetSupplementaryService()
struct sVoiceSetSupplementaryServiceResponse_FailureCause
{
   eQMIVoiceEndReasons mFailureCause;
};

// Structure to describe response TLV 0x11 for VoiceSetSupplementaryService()
struct sVoiceSetSupplementaryServiceResponse_AlphaID
{
   eQMIVoiceUSSDAlphaCodingSchemes mAlphaDCS;
   UINT8 mAlphaLength;

   // This array must be the size specified by mAlphaLength
   // UINT8 mAlphaData[1];
};

// Structure to describe response TLV 0x12 for VoiceSetSupplementaryService()
struct sVoiceSetSupplementaryServiceResponse_CallControl
{
   eQMIVoiceCallControlResultTypes mCallControlResult;
};

// Structure to describe response TLV 0x13 for VoiceSetSupplementaryService()
struct sVoiceSetSupplementaryServiceResponse_CallID
{
   UINT8 mCallID;
};

// Structure to describe response TLV 0x14 for VoiceSetSupplementaryService()
struct sVoiceSetSupplementaryServiceResponse_SupplementaryService
{
   eQMIVoiceSupplementaryServiceRequests mSupplementaryService;
   eQMIVoiceSupplementaryServiceReasons mReason;
};

// Structure to describe response TLV 0x15 for VoiceSetSupplementaryService()
struct sVoiceSetSupplementaryServiceResponse_ServiceStatus
{
   INT8 mActive;
   INT8 mProvisioned;
};

// Structure to describe request TLV 0x10 for VoiceGetCallWaiting()
struct sVoiceGetCallWaitingRequest_ServiceClass
{
   bool mSupplementaryServiceClassVoice:1;
   bool mSupplementaryServiceClassData:1;
   bool mSupplementaryServiceClassFax:1;
   bool mSupplementaryServiceClassSMS:1;
   bool mSupplementaryServiceClassDataCircuitSync:1;
   bool mSupplementaryServiceClassDataCircuitAsync:1;
   bool mSupplementaryServiceClassPacketAccess:1;
   bool mSupplementaryServiceClassPadAccess:1;
};

// Structure to describe request TLV 0x11 for VoiceGetCallWaiting()
struct sVoiceGetCallWaitingRequest_ExtendedServiceClass
{
   eQMIVoiceExtendedServiceClass mExtendedServiceClass;
};

// Structure to describe response TLV 0x10 for VoiceGetCallWaiting()
struct sVoiceGetCallWaitingResponse_ServiceClass
{
   bool mSupplementaryServiceClassVoice:1;
   bool mSupplementaryServiceClassData:1;
   bool mSupplementaryServiceClassFax:1;
   bool mSupplementaryServiceClassSMS:1;
   bool mSupplementaryServiceClassDataCircuitSync:1;
   bool mSupplementaryServiceClassDataCircuitAsync:1;
   bool mSupplementaryServiceClassPacketAccess:1;
   bool mSupplementaryServiceClassPadAccess:1;
};

// Structure to describe response TLV 0x11 for VoiceGetCallWaiting()
struct sVoiceGetCallWaitingResponse_FailureCause
{
   eQMIVoiceEndReasons mFailureCause;
};

// Structure to describe response TLV 0x12 for VoiceGetCallWaiting()
struct sVoiceGetCallWaitingResponse_AlphaID
{
   eQMIVoiceUSSDAlphaCodingSchemes mAlphaDCS;
   UINT8 mAlphaLength;

   // This array must be the size specified by mAlphaLength
   // UINT8 mAlphaData[1];
};

// Structure to describe response TLV 0x13 for VoiceGetCallWaitinge()
struct sVoiceGetCallWaitingeResponse_CallControl
{
   eQMIVoiceCallControlResultTypes mCallControlResult;
};

// Structure to describe response TLV 0x14 for VoiceGetCallWaiting()
struct sVoiceGetCallWaitingResponse_CallID
{
   UINT8 mCallID;
};

// Structure to describe response TLV 0x15 for VoiceGetCallWaiting()
struct sVoiceGetCallWaitingResponse_SupplementaryService
{
   eQMIVoiceSupplementaryServiceRequests mSupplementaryService;
   eQMIVoiceSupplementaryServiceReasons mReason;
};

// Structure to describe response TLV 0x16 for VoiceGetCallWaiting()
struct sVoiceGetCallWaitingResponse_ExtendedServiceClass
{
   eQMIVoiceExtendedServiceClass mExtendedServiceClass;
};

// Structure to describe request TLV 0x01 for VoiceGetCallBarring()
struct sVoiceGetCallBarringRequest_Reason
{
   eQMIVoiceSupplementaryServiceReasons mReason;
};

// Structure to describe request TLV 0x10 for VoiceGetCallBarring()
struct sVoiceGetCallBarringRequest_ServiceClass
{
   bool mSupplementaryServiceClassVoice:1;
   bool mSupplementaryServiceClassData:1;
   bool mSupplementaryServiceClassFax:1;
   bool mSupplementaryServiceClassSMS:1;
   bool mSupplementaryServiceClassDataCircuitSync:1;
   bool mSupplementaryServiceClassDataCircuitAsync:1;
   bool mSupplementaryServiceClassPacketAccess:1;
   bool mSupplementaryServiceClassPadAccess:1;
};

// Structure to describe request TLV 0x11 for VoiceGetCallBarring()
struct sVoiceGetCallBarringRequest_ExtendedServiceClass
{
   eQMIVoiceExtendedServiceClass mExtendedServiceClass;
};

// Structure to describe response TLV 0x10 for VoiceGetCallBarring()
struct sVoiceGetCallBarringResponse_ServiceClass
{
   bool mSupplementaryServiceClassVoice:1;
   bool mSupplementaryServiceClassData:1;
   bool mSupplementaryServiceClassFax:1;
   bool mSupplementaryServiceClassSMS:1;
   bool mSupplementaryServiceClassDataCircuitSync:1;
   bool mSupplementaryServiceClassDataCircuitAsync:1;
   bool mSupplementaryServiceClassPacketAccess:1;
   bool mSupplementaryServiceClassPadAccess:1;
};

// Structure to describe response TLV 0x11 for VoiceGetCallBarring()
struct sVoiceGetCallBarringResponse_FailureCause
{
   eQMIVoiceEndReasons mFailureCause;
};

// Structure to describe response TLV 0x12 for VoiceGetCallBarring()
struct sVoiceGetCallBarringResponse_AlphaID
{
   eQMIVoiceUSSDAlphaCodingSchemes mAlphaDCS;
   UINT8 mAlphaLength;

   // This array must be the size specified by mAlphaLength
   // UINT8 mAlphaData[1];
};

// Structure to describe response TLV 0x13 for VoiceGetCallBarring()
struct sVoiceGetCallBarringResponse_CallControl
{
   eQMIVoiceCallControlResultTypes mCallControlResult;
};

// Structure to describe response TLV 0x14 for VoiceGetCallBarring()
struct sVoiceGetCallBarringResponse_CallID
{
   UINT8 mCallID;
};

// Structure to describe response TLV 0x15 for VoiceGetCallBarring()
struct sVoiceGetCallBarringResponse_SupplementaryService
{
   eQMIVoiceSupplementaryServiceRequests mSupplementaryService;
   eQMIVoiceSupplementaryServiceReasons mReason;
};

// Structure to describe response TLV 0x16 for VoiceGetCallBarring()
struct sVoiceGetCallBarringResponse_ExtendedServiceClass
{
   eQMIVoiceExtendedServiceClass mExtendedServiceClass;
};

// Structure to describe response TLV 0x10 for VoiceGetCLIP()
struct sVoiceGetCLIPResponse_Status
{
   INT8 mCLIPActive;
   INT8 mCLIPProvisioned;
};

// Structure to describe response TLV 0x11 for VoiceGetCLIP()
struct sVoiceGetCLIPResponse_FailureCause
{
   eQMIVoiceEndReasons mFailureCause;
};

// Structure to describe response TLV 0x12 for VoiceGetCLIP()
struct sVoiceGetCLIPResponse_AlphaID
{
   eQMIVoiceUSSDAlphaCodingSchemes mAlphaDCS;
   UINT8 mAlphaLength;

   // This array must be the size specified by mAlphaLength
   // UINT8 mAlphaData[1];
};

// Structure to describe response TLV 0x13 for VoiceGetCLIP()
struct sVoiceGetCLIPResponse_CallControl
{
   eQMIVoiceCallControlResultTypes mCallControlResult;
};

// Structure to describe response TLV 0x14 for VoiceGetCLIP()
struct sVoiceGetCLIPResponse_CallID
{
   UINT8 mCallID;
};

// Structure to describe response TLV 0x15 for VoiceGetCLIP()
struct sVoiceGetCLIPResponse_SupplementaryService
{
   eQMIVoiceSupplementaryServiceRequests mSupplementaryService;
   eQMIVoiceSupplementaryServiceReasons mReason;
};

// Structure to describe response TLV 0x10 for VoiceGetCLIR()
struct sVoiceGetCLIRResponse_Status
{
   INT8 mCLIRActive;
   eQMIVoiceProvisioningStates mCLIRProvisionStatus;
};

// Structure to describe response TLV 0x11 for VoiceGetCLIR()
struct sVoiceGetCLIRResponse_FailureCause
{
   eQMIVoiceEndReasons mFailureCause;
};

// Structure to describe response TLV 0x12 for VoiceGetCLIR()
struct sVoiceGetCLIRResponse_AlphaID
{
   eQMIVoiceUSSDAlphaCodingSchemes mAlphaDCS;
   UINT8 mAlphaLength;

   // This array must be the size specified by mAlphaLength
   // UINT8 mAlphaData[1];
};

// Structure to describe response TLV 0x13 for VoiceGetCLIR()
struct sVoiceGetCLIRResponse_CallControl
{
   eQMIVoiceCallControlResultTypes mCallControlResult;
};

// Structure to describe response TLV 0x14 for VoiceGetCLIR()
struct sVoiceGetCLIRResponse_CallID
{
   UINT8 mCallID;
};

// Structure to describe response TLV 0x15 for VoiceGetCLIR()
struct sVoiceGetCLIRResponse_SupplementaryService
{
   eQMIVoiceSupplementaryServiceRequests mSupplementaryService;
   eQMIVoiceSupplementaryServiceReasons mReason;
};

// Structure to describe request TLV 0x01 for VoiceGetCallForwarding()
struct sVoiceGetCallForwardingRequest_Reason
{
   eQMIVoiceSupplementaryServiceReasons mReason;
};

// Structure to describe request TLV 0x10 for VoiceGetCallForwarding()
struct sVoiceGetCallForwardingRequest_ServiceClass
{
   bool mSupplementaryServiceClassVoice:1;
   bool mSupplementaryServiceClassData:1;
   bool mSupplementaryServiceClassFax:1;
   bool mSupplementaryServiceClassSMS:1;
   bool mSupplementaryServiceClassDataCircuitSync:1;
   bool mSupplementaryServiceClassDataCircuitAsync:1;
   bool mSupplementaryServiceClassPacketAccess:1;
   bool mSupplementaryServiceClassPadAccess:1;
};

// Structure to describe request TLV 0x11 for VoiceGetCallForwarding()
struct sVoiceGetCallForwardingRequest_ExtendedServiceClass
{
   eQMIVoiceExtendedServiceClass mExtendedServiceClass;
};

// Structure to describe response TLV 0x10 for VoiceGetCallForwarding()
struct sVoiceGetCallForwardingResponse_Info
{
   UINT8 mInstanceCount;
   
   struct sInstance1
   {
      INT8 mServiceActive;
      bool mSupplementaryServiceClassVoice:1;
      bool mSupplementaryServiceClassData:1;
      bool mSupplementaryServiceClassFax:1;
      bool mSupplementaryServiceClassSMS:1;
      bool mSupplementaryServiceClassDataCircuitSync:1;
      bool mSupplementaryServiceClassDataCircuitAsync:1;
      bool mSupplementaryServiceClassPacketAccess:1;
      bool mSupplementaryServiceClassPadAccess:1;
      UINT8 mNumberLength;
   
      // This array must be the size specified by mNumberLength
      // char mNumber[1];
   };
   
   struct sInstance2
   {
      UINT8 mNoReplyTimerSeconds;
   };
   
   struct sInstance
   {
      sInstance1 mInstance1;
      sInstance2 mInstance2;
   };
   
   // This array must be the size specified by mInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x11 for VoiceGetCallForwarding()
struct sVoiceGetCallForwardingResponse_FailureCause
{
   eQMIVoiceEndReasons mFailureCause;
};

// Structure to describe response TLV 0x12 for VoiceGetCallForwarding()
struct sVoiceGetCallForwardingResponse_AlphaID
{
   eQMIVoiceUSSDAlphaCodingSchemes mAlphaDCS;
   UINT8 mAlphaLength;

   // This array must be the size specified by mAlphaLength
   // UINT8 mAlphaData[1];
};

// Structure to describe response TLV 0x13 for VoiceGetCallForwarding()
struct sVoiceGetCallForwardingResponse_CallControl
{
   eQMIVoiceCallControlResultTypes mCallControlResult;
};

// Structure to describe response TLV 0x14 for VoiceGetCallForwarding()
struct sVoiceGetCallForwardingResponse_CallID
{
   UINT8 mCallID;
};

// Structure to describe response TLV 0x15 for VoiceGetCallForwarding()
struct sVoiceGetCallForwardingResponse_SupplementaryService
{
   eQMIVoiceSupplementaryServiceRequests mSupplementaryService;
   eQMIVoiceSupplementaryServiceReasons mReason;
};

// Structure to describe response TLV 0x16 for VoiceGetCallForwarding()
struct sVoiceGetCallForwardingResponse_ExtendedInfo
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      INT8 mServiceActive;
      bool mSupplementaryServiceClassVoice:1;
      bool mSupplementaryServiceClassData:1;
      bool mSupplementaryServiceClassFax:1;
      bool mSupplementaryServiceClassSMS:1;
      bool mSupplementaryServiceClassDataCircuitSync:1;
      bool mSupplementaryServiceClassDataCircuitAsync:1;
      bool mSupplementaryServiceClassPacketAccess:1;
      bool mSupplementaryServiceClassPadAccess:1;
      UINT8 mNoReplyTimerSeconds;
      UINT8 mPresentationIndicator;
      eQMIVoiceScreeningIndicators mScreeningIndicator;
      eQMIVoiceNumberTypes mNumberType;
      eQMIVoiceNumberPlans mNumberPlan;
      UINT8 mNumberLength;
   
      // This array must be the size specified by mNumberLength
      // char mNumber[1];
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe response TLV 0x17 for VoiceGetCallForwarding()
struct sVoiceGetCallForwardingResponse_ExtendedInfo2
{
   UINT8 mCallInstanceCount;

   struct sInstance
   {
      INT8 mServiceActive;
      eQMIVoiceExtendedServiceClass mExtendedServiceClass;
      UINT8 mNoReplyTimerSeconds;
      UINT8 mPresentationIndicator;
      eQMIVoiceScreeningIndicators mScreeningIndicator;
      eQMIVoiceNumberTypes mNumberType;
      eQMIVoiceNumberPlans mNumberPlan;
      UINT8 mNumberLength;
   
      // This array must be the size specified by mNumberLength
      // char mNumber[1];
   };

   // This array must be the size specified by mCallInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe request TLV 0x01 for VoiceSetCallBarringPassword()
struct sVoiceSetCallBarringPasswordRequest_Info
{
   eQMIVoiceSupplementaryServiceReasons mReason;
   char mOldCallBarringPassword[4];
   char mNewCallBarringPassword[4];
   char mNewCallBarringPasswordRepeat[4];
};

// Structure to describe response TLV 0x10 for VoiceSetCallBarring()
struct sVoiceSetCallBarringResponse_FailureCause
{
   eQMIVoiceEndReasons mFailureCause;
};

// Structure to describe response TLV 0x11 for VoiceSetCallBarring()
struct sVoiceSetCallBarringResponse_AlphaID
{
   eQMIVoiceUSSDAlphaCodingSchemes mAlphaDCS;
   UINT8 mAlphaLength;

   // This array must be the size specified by mAlphaLength
   // UINT8 mAlphaData[1];
};

// Structure to describe response TLV 0x12 for VoiceSetCallBarring()
struct sVoiceSetCallBarringResponse_CallControl
{
   eQMIVoiceCallControlResultTypes mCallControlResult;
};

// Structure to describe response TLV 0x13 for VoiceSetCallBarring()
struct sVoiceSetCallBarringResponse_CallID
{
   UINT8 mCallID;
};

// Structure to describe response TLV 0x14 for VoiceSetCallBarring()
struct sVoiceSetCallBarringResponse_SupplementaryService
{
   eQMIVoiceSupplementaryServiceRequests mSupplementaryService;
   eQMIVoiceSupplementaryServiceReasons mReason;
};

// Structure to describe request TLV 0x01 for VoiceInitiateUSSD()
struct sVoiceInitiateUSSDRequest_Info
{
   eQMIVoiceUSSDDataCodingSchemes mUSSDCS;
   UINT8 mUSSLength;

   // This array must be the size specified by mUSSLength
   // UINT8 mUSSData[1];
};

// Structure to describe response TLV 0x10 for VoiceInitiateUSSD()
struct sVoiceInitiateUSSDResponse_FailCause
{
   UINT16 mFailureCause;
};

// Structure to describe response TLV 0x11 for VoiceInitiateUSSD()
struct sVoiceInitiateUSSDResponse_AlphaID
{
   eQMIVoiceUSSDAlphaCodingSchemes mAlphaDCS;
   UINT8 mAlphaLength;

   // This array must be the size specified by mAlphaLength
   // UINT8 mAlphaData[1];
};

// Structure to describe response TLV 0x12 for VoiceInitiateUSSD()
struct sVoiceInitiateUSSDResponse_Data
{
   eQMIVoiceUSSDDataCodingSchemes mUSSDCS;
   UINT8 mUSSLength;

   // This array must be the size specified by mUSSLength
   // UINT8 mUSSData[1];
};

// Structure to describe response TLV 0x13 for VoiceInitiateUSSD()
struct sVoiceInitiateUSSDResponse_CallControl
{
   eQMIVoiceCallControlResultTypes mCallControlResult;
};

// Structure to describe response TLV 0x14 for VoiceInitiateUSSD()
struct sVoiceInitiateUSSDResponse_CallID
{
   UINT8 mCallID;
};

// Structure to describe response TLV 0x15 for VoiceInitiateUSSD()
struct sVoiceInitiateUSSDResponse_SupplementaryService
{
   eQMIVoiceSupplementaryServiceRequests mSupplementaryService;
   eQMIVoiceSupplementaryServiceReasons mReason;
};

// Structure to describe response TLV 0x16 for VoiceInitiateUSSD()
struct sVoiceInitiateUSSDResponse_EncodedData
{
   UINT8 mServiceInformationLength;

   // This array must be the size specified by mServiceInformationLength
   // wchar_t mServiceInformation[1];
};

// Structure to describe request TLV 0x01 for VoiceAnswerUSSD()
struct sVoiceAnswerUSSDRequest_Info
{
   eQMIVoiceUSSDDataCodingSchemes mUSSDCS;
   UINT8 mUSSLength;

   // This array must be the size specified by mUSSLength
   // UINT8 mUSSData[1];
};

// Structure to describe indication TLV 0x01 for Voice USSDIndication
struct sVoiceUSSDIndication_Type
{
   eQMIVoiceUSSDNotifcationTypes mNotificationType;
};

// Structure to describe indication TLV 0x10 for Voice USSDIndication
struct sVoiceUSSDIndication_Data
{
   eQMIVoiceUSSDDataCodingSchemes mUSSDCS;
   UINT8 mUSSLength;

   // This array must be the size specified by mUSSLength
   // UINT8 mUSSData[1];
};

// Structure to describe indication TLV 0x11 for Voice USSDIndication
struct sVoiceUSSDIndication_EncodedData
{
   UINT8 mServiceInformationLength;

   // This array must be the size specified by mServiceInformationLength
   // wchar_t mServiceInformation[1];
};

// Structure to describe indication TLV 0x01 for Voice USSIndication
struct sVoiceUSSIndication_Info
{
   UINT8 mCallID;
   eQMIVoiceUUSTypes mUUSType;
   eQMIVoiceUUSDataCodingSchemes mUUSDCS;
   UINT8 mUUSLength;

   // This array must be the size specified by mUUSLength
   // UINT8 mUUSData[1];
};

// Structure to describe request TLV 0x10 for VoiceSetConfig()
struct sVoiceSetConfigRequest_AutoAnswer
{
   INT8 mAutoAnswer;
};

// Structure to describe request TLV 0x11 for VoiceSetConfig()
struct sVoiceSetConfigRequest_AirTimer
{
   UINT8 mNAMID;
   UINT32 mAirTimerMinutes;
};

// Structure to describe request TLV 0x12 for VoiceSetConfig()
struct sVoiceSetConfigRequest_RoamTimer
{
   UINT8 mNAMID;
   UINT32 mRoamTimerMinutes;
};

// Structure to describe request TLV 0x13 for VoiceSetConfig()
struct sVoiceSetConfigRequest_TTYMode
{
   eQMIVoiceTTYModes mTTYMode;
};

// Structure to describe request TLV 0x14 for VoiceSetConfig()
struct sVoiceSetConfigRequest_PreferredVoiceSO
{
   UINT8 mNAMID;
   INT8 mEVRCCapability;
   eQMIVoiceServiceOptions mHomePageVoiceServiceOption;
   eQMIVoiceServiceOptions mHomeOriginationVoiceServiceOption;
   eQMIVoiceServiceOptions mRoamOriginationVoiceServiceOption;
};

// Structure to describe request TLV 0x15 for VoiceSetConfig()
struct sVoiceSetConfigRequest_PreferredVoiceDomain
{
   eQMIVoiceDomains mPreferredDomain;
};

// Structure to describe response TLV 0x10 for VoiceSetConfig()
struct sVoiceSetConfigResponse_AutoAnswer
{
   INT8 mWriteFailed;
};

// Structure to describe response TLV 0x11 for VoiceSetConfig()
struct sVoiceSetConfigResponse_AirTimer
{
   INT8 mWriteFailed;
};

// Structure to describe response TLV 0x12 for VoiceSetConfig()
struct sVoiceSetConfigResponse_RoamTimer
{
   INT8 mWriteFailed;
};

// Structure to describe response TLV 0x13 for VoiceSetConfig()
struct sVoiceSetConfigResponse_TTYMode
{
   INT8 mWriteFailed;
};

// Structure to describe response TLV 0x14 for VoiceSetConfig()
struct sVoiceSetConfigResponse_PreferredVoiceSO
{
   INT8 mWriteFailed;
};

// Structure to describe response TLV 0x15 for VoiceSetConfig()
struct sVoiceSetConfigResponse_PreferredVoiceDomain
{
   INT8 mWriteFailed;
};

// Structure to describe request TLV 0x10 for VoiceGetConfig()
struct sVoiceGetConfigRequest_AutoAnswer
{
   INT8 mInclude;
};

// Structure to describe request TLV 0x11 for VoiceGetConfig()
struct sVoiceGetConfigRequest_AirTimer
{
   INT8 mInclude;
};

// Structure to describe request TLV 0x12 for VoiceGetConfig()
struct sVoiceGetConfigRequest_RoamTimer
{
   INT8 mInclude;
};

// Structure to describe request TLV 0x13 for VoiceGetConfig()
struct sVoiceGetConfigRequest_TTYMode
{
   INT8 mInclude;
};

// Structure to describe request TLV 0x14 for VoiceGetConfig()
struct sVoiceGetConfigRequest_PreferredVoiceSO
{
   INT8 mInclude;
};

// Structure to describe request TLV 0x15 for VoiceGetConfig()
struct sVoiceGetConfigRequest_AMRStatus
{
   INT8 mInclude;
};

// Structure to describe request TLV 0x16 for VoiceGetConfig()
struct sVoiceGetConfigRequest_PreferredVoicePrivacy
{
   INT8 mInclude;
};

// Structure to describe request TLV 0x17 for VoiceGetConfig()
struct sVoiceGetConfigRequest_NAM
{
   UINT8 mNAMID;
};

// Structure to describe request TLV 0x18 for VoiceGetConfig()
struct sVoiceGetConfigRequest_VoiceDomain
{
   INT8 mInclude;
};

// Structure to describe response TLV 0x10 for VoiceGetConfig()
struct sVoiceGetConfigResponse_AutoAnswer
{
   INT8 mAutoAnswer;
};

// Structure to describe response TLV 0x11 for VoiceGetConfig()
struct sVoiceGetConfigResponse_AirTimer
{
   UINT8 mNAMID;
   UINT32 mAirTimerMinutes;
};

// Structure to describe response TLV 0x12 for VoiceGetConfig()
struct sVoiceGetConfigResponse_RoamTimer
{
   UINT8 mNAMID;
   UINT32 mRoamTimerMinutes;
};

// Structure to describe response TLV 0x13 for VoiceGetConfig()
struct sVoiceGetConfigResponse_TTYMode
{
   eQMIVoiceTTYModes mTTYMode;
};

// Structure to describe response TLV 0x14 for VoiceGetConfig()
struct sVoiceGetConfigResponse_PreferredVoiceSO
{
   UINT8 mNAMID;
   INT8 mEVRCCapability;
   eQMIVoiceServiceOptions mHomePageVoiceServiceOption;
   eQMIVoiceServiceOptions mHomeOriginationVoiceServiceOption;
   eQMIVoiceServiceOptions mRoamOriginationVoiceServiceOption;
};

// Structure to describe response TLV 0x15 for VoiceGetConfig()
struct sVoiceGetConfigResponse_AMRConfig
{
   INT8 mGSMAMR;
   bool mWCDMAAMRWB:1;
   bool mGSMHRAMR:1;
   bool mGSMAMRWB:1;
   bool mGSMAMRNB:1;

   // Padding out 4 bits
   UINT8 mReserved1:4;
};

// Structure to describe response TLV 0x16 for VoiceGetConfig()
struct sVoiceGetConfigResponse_Privacy
{
   eQMIVoicePrivacyLevels mVoicePrivacy;
};

// Structure to describe response TLV 0x17 for VoiceGetConfig()
struct sVoiceGetConfigResponse_PreferredVoiceDomain
{
   eQMIVoiceDomains mPreferredDomain;
};

// Structure to describe request TLV 0x01 for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_Info
{
   eQMIVoiceSupplementaryServiceRequests mSupplementaryServiceRequest;
   INT8 mModifiedByCallControl;
};

// Structure to describe request TLV 0x10 for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_Class
{
   bool mSupplementaryServiceClassVoice:1;
   bool mSupplementaryServiceClassData:1;
   bool mSupplementaryServiceClassFax:1;
   bool mSupplementaryServiceClassSMS:1;
   bool mSupplementaryServiceClassDataCircuitSync:1;
   bool mSupplementaryServiceClassDataCircuitAsync:1;
   bool mSupplementaryServiceClassPacketAccess:1;
   bool mSupplementaryServiceClassPadAccess:1;
};

// Structure to describe request TLV 0x11 for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_Reason
{
   eQMIVoiceSupplementaryServiceReasons mReason;
};

// Structure to describe request TLV 0x12 for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_Number
{
   // String is variable length, but must be size of the container
   // char mCallForwardingNumber[1];
};

// Structure to describe request TLV 0x13 for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_Timer
{
   UINT8 mNoReplyTimerSeconds;
};

// Structure to describe request TLV 0x14 for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_USSData
{
   eQMIVoiceUSSDDataCodingSchemes mUSSDCS;
   UINT8 mUSSLength;

   // This array must be the size specified by mUSSLength
   // UINT8 mUSSData[1];
};

// Structure to describe request TLV 0x15 for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_CallID
{
   UINT8 mCallID;
};

// Structure to describe request TLV 0x16 for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_AlphaID
{
   eQMIVoiceUSSDAlphaCodingSchemes mAlphaDCS;
   UINT8 mAlphaLength;

   // This array must be the size specified by mAlphaLength
   // UINT8 mAlphaData[1];
};

// Structure to describe request TLV 0x17 for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_Password
{
   char mCallBarringPassword[4];
};

// Structure to describe request TLV 0x18 for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_NewPassword
{
   char mNewCallBarringPassword[4];
   char mNewCallBarringPasswordRepeat[4];
};

// Structure to describe request TLV 0x19 for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_DataSource
{
   INT8 mResponseData;
};

// Structure to describe request TLV 0x1A for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_FailCause
{
   UINT16 mFailureCause;
};

// Structure to describe request TLV 0x1B for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_CallForwarding
{
   UINT8 mInstanceCount;
   
   struct sInstance1
   {
      INT8 mServiceActive;
      bool mSupplementaryServiceClassVoice:1;
      bool mSupplementaryServiceClassData:1;
      bool mSupplementaryServiceClassFax:1;
      bool mSupplementaryServiceClassSMS:1;
      bool mSupplementaryServiceClassDataCircuitSync:1;
      bool mSupplementaryServiceClassDataCircuitAsync:1;
      bool mSupplementaryServiceClassPacketAccess:1;
      bool mSupplementaryServiceClassPadAccess:1;
      UINT8 mNumberLength;
   
      // This array must be the size specified by mNumberLength
      // char mNumber[1];
   };
   
   struct sInstance2
   {
      UINT8 mNoReplyTimerSeconds;
   };
   
   struct sInstance
   {
      sInstance1 mInstance1;
      sInstance2 mInstance2;
   };
   
   // This array must be the size specified by mInstanceCount
   // sInstance mInstances[1];
};

// Structure to describe request TLV 0x1C for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_CLIR
{
   INT8 mCLIRActive;
   eQMIVoiceProvisioningStates mCLIRProvisionStatus;
};

// Structure to describe request TLV 0x1D for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_CLIP
{
   INT8 mCLIPActive;
   INT8 mCLIPProvisioned;
};

// Structure to describe request TLV 0x1E for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_COLP
{
   INT8 mActive;
   INT8 mProvisioned;
};

// Structure to describe request TLV 0x1F for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_COLR
{
   INT8 mActive;
   INT8 mProvisioned;
};

// Structure to describe request TLV 0x20 for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_CNAP
{
   INT8 mActive;
   INT8 mProvisioned;
};

// Structure to describe request TLV 0x21 for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_EncodedData
{
   UINT8 mServiceInformationLength;

   // This array must be the size specified by mServiceInformationLength
   // wchar_t mServiceInformation[1];
};

// Structure to describe request TLV 0x22 for VoiceSupplementaryService()
struct sVoiceSupplementaryServiceRequestIndication_ExtendedServiceClass
{
   eQMIVoiceExtendedServiceClass mExtendedServiceClass;
};

// Structure to describe request TLV 0x01 for VoiceAsyncInitiateUSSD()
struct sVoiceAsyncInitiateUSSDRequest_Info
{
   eQMIVoiceUSSDDataCodingSchemes mUSSDCS;
   UINT8 mUSSLength;

   // This array must be the size specified by mUSSLength
   // UINT8 mUSSData[1];
};

// Structure to describe indication TLV 0x10 for Voice USSDAsyncIndication
struct sVoiceUSSDAsyncIndication_Error
{
   eQMIErrors mQMIError;
};

// Structure to describe indication TLV 0x11 for Voice USSDAsyncIndication
struct sVoiceUSSDAsyncIndication_FailCause
{
   UINT16 mFailureCause;
};

// Structure to describe indication TLV 0x12 for Voice USSDAsyncIndication
struct sVoiceUSSDAsyncIndication_Info
{
   eQMIVoiceUSSDDataCodingSchemes mUSSDCS;
   UINT8 mUSSLength;

   // This array must be the size specified by mUSSLength
   // UINT8 mUSSData[1];
};

// Structure to describe indication TLV 0x13 for Voice USSDAsyncIndication
struct sVoiceUSSDAsyncIndication_AlphaID
{
   eQMIVoiceUSSDAlphaCodingSchemes mAlphaDCS;
   UINT8 mAlphaLength;

   // This array must be the size specified by mAlphaLength
   // UINT8 mAlphaData[1];
};

// Structure to describe indication TLV 0x14 for Voice USSDAsyncIndication
struct sVoiceUSSDAsyncIndication_EncodedData
{
   UINT8 mServiceInformationLength;

   // This array must be the size specified by mServiceInformationLength
   // wchar_t mServiceInformation[1];
};

// Structure to describe request TLV 0x01 for VoiceBindSubscription()
struct sVoiceBindSubscriptionRequest_Type
{
   eQMIVoiceSubscriptionTypes mSubscriptionType;
};

// Structure to describe request TLV 0x01 for VoiceALSSetLineSwitching()
struct sVoiceALSSetLineSwitchingRequest_Switching
{
   INT8 mLineSwitchingAllowed;
};

// Structure to describe request TLV 0x01 for VoiceALSSelectLine()
struct sVoiceALSSelectLineRequest_Line
{
   eQMIVoiceALSLines mLineValue;
};

// Structure to describe request TLV 0x01 for VoiceAOCSetACMMaximum()
struct sVoiceAOCSetACMMaximumRequest_ACMMax
{
   UINT32 mACMMaximum;
};

// Structure to describe request TLV 0x01 for VoiceAOCGetCallMeterInfo()
struct sVoiceAOCGetCallMeterInfoRequest_Mask
{
   bool mACM:1;
   bool mACMMaximum:1;
   bool mCCM:1;

   // Padding out 13 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2;
};

// Structure to describe response TLV 0x10 for VoiceAOCGetCallMeterInfo()
struct sVoiceAOCGetCallMeterInfoResponse_ACM
{
   UINT32 mACM;
};

// Structure to describe response TLV 0x11 for VoiceAOCGetCallMeterInfo()
struct sVoiceAOCGetCallMeterInfoResponse_ACMMax
{
   UINT32 mACMMaximum;
};

// Structure to describe response TLV 0x12 for VoiceAOCGetCallMeterInfo()
struct sVoiceAOCGetCallMeterInfoResponse_CCM
{
   UINT32 mCCM;
};

// Structure to describe response TLV 0x10 for VoiceGetCOLP()
struct sVoiceGetCOLPResponse_COLP
{
   INT8 mActive;
   INT8 mProvisioned;
};

// Structure to describe response TLV 0x11 for VoiceGetCOLP()
struct sVoiceGetCOLPResponse_FailCause
{
   UINT16 mFailureCause;
};

// Structure to describe response TLV 0x12 for VoiceGetCOLP()
struct sVoiceGetCOLPResponse_AlphaID
{
   eQMIVoiceUSSDAlphaCodingSchemes mAlphaDCS;
   UINT8 mAlphaLength;

   // This array must be the size specified by mAlphaLength
   // UINT8 mAlphaData[1];
};

// Structure to describe response TLV 0x13 for VoiceGetCOLP()
struct sVoiceGetCOLPResponse_CallControl
{
   eQMIVoiceCallControlResultTypes mCallControlResult;
};

// Structure to describe response TLV 0x14 for VoiceGetCOLP()
struct sVoiceGetCOLPResponse_CallID
{
   UINT8 mCallID;
};

// Structure to describe response TLV 0x15 for VoiceGetCOLP()
struct sVoiceGetCOLPResponse_SupplementaryService
{
   eQMIVoiceSupplementaryServiceRequests mSupplementaryService;
   eQMIVoiceSupplementaryServiceReasons mReason;
};

// Structure to describe response TLV 0x10 for VoiceGetCOLR()
struct sVoiceGetCOLRResponse_COLR
{
   INT8 mActive;
   INT8 mProvisioned;
};

// Structure to describe response TLV 0x11 for VoiceGetCOLR()
struct sVoiceGetCOLRResponse_FailCause
{
   UINT16 mFailureCause;
};

// Structure to describe response TLV 0x12 for VoiceGetCOLR()
struct sVoiceGetCOLRResponse_AlphaID
{
   eQMIVoiceUSSDAlphaCodingSchemes mAlphaDCS;
   UINT8 mAlphaLength;

   // This array must be the size specified by mAlphaLength
   // UINT8 mAlphaData[1];
};

// Structure to describe response TLV 0x13 for VoiceGetCOLR()
struct sVoiceGetCOLRResponse_CallControl
{
   eQMIVoiceCallControlResultTypes mCallControlResult;
};

// Structure to describe response TLV 0x14 for VoiceGetCOLR()
struct sVoiceGetCOLRResponse_CallID
{
   UINT8 mCallID;
};

// Structure to describe response TLV 0x15 for VoiceGetCOLR()
struct sVoiceGetCOLRResponse_SupplementaryService
{
   eQMIVoiceSupplementaryServiceRequests mSupplementaryService;
   eQMIVoiceSupplementaryServiceReasons mReason;
};

// Structure to describe response TLV 0x10 for VoiceGetCNAP()
struct sVoiceGetCNAPResponse_CNAP
{
   INT8 mActive;
   INT8 mProvisioned;
};

// Structure to describe response TLV 0x11 for VoiceGetCNAP()
struct sVoiceGetCNAPResponse_FailCause
{
   UINT16 mFailureCause;
};

// Structure to describe response TLV 0x12 for VoiceGetCNAP()
struct sVoiceGetCNAPResponse_AlphaID
{
   eQMIVoiceUSSDAlphaCodingSchemes mAlphaDCS;
   UINT8 mAlphaLength;

   // This array must be the size specified by mAlphaLength
   // UINT8 mAlphaData[1];
};

// Structure to describe response TLV 0x13 for VoiceGetCNAP()
struct sVoiceGetCNAPResponse_CallControl
{
   eQMIVoiceCallControlResultTypes mCallControlResult;
};

// Structure to describe response TLV 0x14 for VoiceGetCNAP()
struct sVoiceGetCNAPResponse_CallID
{
   UINT8 mCallID;
};

// Structure to describe response TLV 0x15 for VoiceGetCNAP()
struct sVoiceGetCNAPResponse_SupplementaryService
{
   eQMIVoiceSupplementaryServiceRequests mSupplementaryService;
   eQMIVoiceSupplementaryServiceReasons mReason;
};

// Structure to describe request TLV 0x01 for VoiceManageIPCalls()
struct sVoiceManageIPCallsRequest_Info
{
   eQMIVoiceVoIPSUPSCallTypes mSupplementaryServiceType;
};

// Structure to describe request TLV 0x10 for VoiceManageIPCalls()
struct sVoiceManageIPCallsRequest_CallID
{
   UINT8 mCallID;
};

// Structure to describe request TLV 0x11 for VoiceManageIPCalls()
struct sVoiceManageIPCallsRequest_CallType
{
   eQMIVoiceCallTypes mCallType;
};

// Structure to describe request TLV 0x12 for VoiceManageIPCalls()
struct sVoiceManageIPCallsRequest_AudioAttribute
{
   bool mVoiceCallTX:1;
   bool mVoiceCallRX:1;

   // Padding out 62 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[7];
};

// Structure to describe request TLV 0x13 for VoiceManageIPCalls()
struct sVoiceManageIPCallsRequest_VideoAttribute
{
   bool mVoiceCallTX:1;
   bool mVoiceCallRX:1;

   // Padding out 62 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[7];
};

// Structure to describe request TLV 0x14 for VoiceManageIPCalls()
struct sVoiceManageIPCallsRequest_SIPURI
{
   // String is variable length, but must be size of the container
   // char mSIPURIOverflow[1];
};

// Structure to describe response TLV 0x10 for VoiceManageIPCalls()
struct sVoiceManageIPCallsResponse_CallID
{
   UINT8 mCallID;
};

// Structure to describe response TLV 0x11 for VoiceManageIPCalls()
struct sVoiceManageIPCallsResponse_FailureCause
{
   eQMIVoiceEndReasons mFailureCause;
};

// Structure to describe response TLV 0x10 for VoiceALSGetLineSwitchingStatus()
struct sVoiceALSGetLineSwitchingStatusResponse_SwitchValue
{
   eQMIVoiceSwitchValue mSwitchValue;
};

// Structure to describe response TLV 0x10 for VoiceALSGetSelectedLine()
struct sVoiceALSGetSelectedLineResponse_Line
{
   eQMIVoiceALSLines mLineValue;
};

// Structure to describe indication TLV 0x01 for Voice CallModifiedIndication
struct sVoiceCallModifiedIndication_CallID
{
   UINT8 mCallID;
};

// Structure to describe indication TLV 0x10 for Voice CallModifiedIndication
struct sVoiceCallModifiedIndication_CallType
{
   eQMIVoiceCallTypes mCallType;
};

// Structure to describe indication TLV 0x11 for Voice CallModifiedIndication
struct sVoiceCallModifiedIndication_AudioAttribute
{
   bool mVoiceCallTX:1;
   bool mVoiceCallRX:1;

   // Padding out 62 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[7];
};

// Structure to describe indication TLV 0x12 for Voice CallModifiedIndication
struct sVoiceCallModifiedIndication_VideoAttribute
{
   bool mVoiceCallTX:1;
   bool mVoiceCallRX:1;

   // Padding out 62 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[7];
};

// Structure to describe indication TLV 0x01 for Voice CallModifyAcceptIndication
struct sVoiceCallModifyAcceptIndication_CallID
{
   UINT8 mCallID;
};

// Structure to describe indication TLV 0x10 for Voice CallModifyAcceptIndication
struct sVoiceCallModifyAcceptIndication_CallType
{
   eQMIVoiceCallTypes mCallType;
};

// Structure to describe indication TLV 0x11 for Voice CallModifyAcceptIndication
struct sVoiceCallModifyAcceptIndication_AudioAttribute
{
   bool mVoiceCallTX:1;
   bool mVoiceCallRX:1;

   // Padding out 62 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[7];
};

// Structure to describe indication TLV 0x12 for Voice CallModifyAcceptIndication
struct sVoiceCallModifyAcceptIndication_VideoAttribute
{
   bool mVoiceCallTX:1;
   bool mVoiceCallRX:1;

   // Padding out 62 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[7];
};

// Structure to describe indication TLV 0x10 for Voice SpeechCodecInformationIndication
struct sVoiceSpeechCodecInformationIndication_NetworkMode
{
   eQMIVoiceNetworkMode mNetworkMode;
};

// Structure to describe indication TLV 0x11 for Voice SpeechCodecInformationIndication
struct sVoiceSpeechCodecInformationIndication_Type
{
   eQMIVoiceSpeechCodecType mType;
};

// Structure to describe indication TLV 0x12 for Voice SpeechCodecInformationIndication
struct sVoiceSpeechCodecInformationIndication_SamplingRate
{
   UINT32 mSamplingRateHz;
};

// Structure to describe indication TLV 0x01 for Voice HandoverIndication
struct sVoiceHandoverIndication_State
{
   eQMIVoiceHandoverStates mHandoverState;
};

// Structure to describe request TLV 0x10 for CAT2SetEventReport()
struct sCAT2SetEventReportRequest_ReportMask
{
   bool mDisplayText:1;
   bool mGetInkey:1;
   bool mGetInput:1;
   bool mSetupMenu:1;
   bool mSelectItem:1;
   bool mSendSMSAlphaIdentifier:1;
   bool mSetupEventUserActivity:1;
   bool mSetupEventIdleScreenNotify:1;
   bool mSetupEventLanguageSelNotify:1;
   bool mSetupIdleModeText:1;
   bool mLanguageNotification:1;
   bool mRefresh:1;
   bool mEndProactiveSession:1;
   bool mPlayTone:1;
   bool mSetupCall:1;
   bool mSendDTMF:1;
   bool mLaunchBrowser:1;
   bool mSendSS:1;
   bool mSendUSSD:1;
   bool mProvideLocalInformationLanguage:1;
   bool mBearerIndependentProtocol:1;
   bool mSetupEventBrowserTermination:1;
   bool mProvideLocalInformationTime:1;
   bool mActivate:1;

   // Padding out 1 bits
   UINT8 mReserved1:1;

   bool mSetupEventHCIConnectivity:1;

   // Padding out 6 bits
   UINT8 mReserved2:6;
};

// Structure to describe request TLV 0x11 for CAT2SetEventReport()
struct sCAT2SetEventReportRequest_DecodeReportMask
{
   bool mDisplayText:1;
   bool mGetInkey:1;
   bool mGetInput:1;
   bool mSetupMenu:1;
   bool mSelectItem:1;
   bool mSendSMSAlphaIdentifier:1;
   bool mSetupEventUserActivity:1;
   bool mSetupEventIdleScreenNotify:1;
   bool mSetupEventLanguageSelNotify:1;
   bool mSetupIdleModeText:1;
   bool mLanguageNotification:1;

   // Padding out 1 bits
   UINT8 mReserved1:1;

   bool mEndProactiveSession:1;
   bool mPlayTone:1;
   bool mSetupCall:1;
   bool mSendDTMF:1;
   bool mLaunchBrowser:1;
   bool mSendSS:1;
   bool mSendUSSD:1;
   bool mProvideLocalInformationLanguage:1;
   bool mBearerIndependentProtocol:1;

   // Padding out 2 bits
   UINT8 mReserved2:2;

   bool mSCWSEvent:1;
   bool mActivate:1;
   bool mSetupEventHCIConnectivity:1;

   // Padding out 6 bits
   UINT8 mReserved3:6;
};

// Structure to describe request TLV 0x12 for CAT2SetEventReport()
struct sCAT2SetEventReportRequest_Slot
{
   bool mSlot1:1;
   bool mSlot2:1;

   // Padding out 6 bits
   UINT8 mReserved1:6;
};

// Structure to describe response TLV 0x10 for CAT2SetEventReport()
struct sCAT2SetEventReportResponse_RegStatusMask
{
   bool mDisplayText:1;
   bool mGetInkey:1;
   bool mGetInput:1;
   bool mSetupMenu:1;
   bool mSelectItem:1;
   bool mSendSMSAlphaIdentifier:1;
   bool mSetupEventUserActivity:1;
   bool mSetupEventIdleScreenNotify:1;
   bool mSetupEventLanguageSelNotify:1;
   bool mSetupIdleModeText:1;
   bool mLanguageNotification:1;
   bool mRefresh:1;
   bool mEndProactiveSession:1;
   bool mPlayTone:1;
   bool mSetupCall:1;
   bool mSendDTMF:1;
   bool mLaunchBrowser:1;
   bool mSendSS:1;
   bool mSendUSSD:1;
   bool mProvideLocalInformationLanguage:1;
   bool mBearerIndependentProtocol:1;
   bool mSetupEventBrowserTermination:1;
   bool mProvideLocalInformationTime:1;
   bool mActivate:1;

   // Padding out 1 bits
   UINT8 mReserved1:1;

   bool mSetupEventHCIConnectivity:1;

   // Padding out 6 bits
   UINT8 mReserved2:6;
};

// Structure to describe response TLV 0x11 for CAT2SetEventReport()
struct sCAT2SetEventReportResponse_DecodedRegStatusMask
{
   bool mDisplayText:1;
   bool mGetInkey:1;
   bool mGetInput:1;
   bool mSetupMenu:1;
   bool mSelectItem:1;
   bool mSendSMSAlphaIdentifier:1;
   bool mSetupEventUserActivity:1;
   bool mSetupEventIdleScreenNotify:1;
   bool mSetupEventLanguageSelNotify:1;
   bool mSetupIdleModeText:1;
   bool mLanguageNotification:1;

   // Padding out 1 bits
   UINT8 mReserved1:1;

   bool mEndProactiveSession:1;
   bool mPlayTone:1;
   bool mSetupCall:1;
   bool mSendDTMF:1;
   bool mLaunchBrowser:1;
   bool mSendSS:1;
   bool mSendUSSD:1;
   bool mProvideLocalInformationLanguage:1;
   bool mBearerIndependentProtocol:1;

   // Padding out 2 bits
   UINT8 mReserved2:2;

   bool mSCWSEvent:1;
   bool mActivate:1;
   bool mSetupEventHCIConnectivity:1;

   // Padding out 6 bits
   UINT8 mReserved3:6;
};

// Structure to describe indication TLV 0x10 for CAT2 EventReport
struct sCAT2EventReportIndication_DisplayTextEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mDisplayTextCommand[1];
};

// Structure to describe indication TLV 0x11 for CAT2 EventReport
struct sCAT2EventReportIndication_GetInkeyEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mGetInkeyCommand[1];
};

// Structure to describe indication TLV 0x12 for CAT2 EventReport
struct sCAT2EventReportIndication_GetInputEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mGetInputCommand[1];
};

// Structure to describe indication TLV 0x13 for CAT2 EventReport
struct sCAT2EventReportIndication_SetupMenuEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSetupMenuCommand[1];
};

// Structure to describe indication TLV 0x14 for CAT2 EventReport
struct sCAT2EventReportIndication_SelectItemEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSelectItemCommand[1];
};

// Structure to describe indication TLV 0x15 for CAT2 EventReport
struct sCAT2EventReportIndication_AlphaIDAvailable
{
   eQMICATAlphaIDCommandType mAlphaIDCommandType;
   UINT16 mAlphaIDLength;

   // This array must be the size specified by mAlphaIDLength
   // UINT8 mAlphaID[1];
};

// Structure to describe indication TLV 0x16 for CAT2 EventReport
struct sCAT2EventReportIndication_SetupEventList
{
   bool mUserActivityNotify:1;
   bool mIdleScreenAvailable:1;
   bool mLanguageSelectionNotify:1;

   // Padding out 29 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2[3];
};

// Structure to describe indication TLV 0x17 for CAT2 EventReport
struct sCAT2EventReportIndication_SetupIdleModeTextEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSetupIdleModeTextCommand[1];
};

// Structure to describe indication TLV 0x18 for CAT2 EventReport
struct sCAT2EventReportIndication_LanguageNotificationEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mLanguageNotificationCommand[1];
};

// Structure to describe indication TLV 0x19 for CAT2 EventReport
struct sCAT2EventReportIndication_RefreshEvent
{
   UINT16 mRefreshMode;
   eQMICATRefreshStage mRefreshStage;
};

// Structure to describe indication TLV 0x1A for CAT2 EventReport
struct sCAT2EventReportIndication_EndProactiveSession
{
   eQMICATProactiveSessionEndType mProactiveSessionEndType;
};

// Structure to describe indication TLV 0x1B for CAT2 EventReport
struct sCAT2EventReportIndication_DecodedHeaderID
{
   eQMICATCommandID mCommandID;
   UINT32 mReferenceID;
   UINT8 mCommandNumber;
};

// Structure to describe indication TLV 0x1C for CAT2 EventReport
struct sCAT2EventReportIndication_TextString
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe indication TLV 0x1D for CAT2 EventReport
struct sCAT2EventReportIndication_HighPriority
{
   eQMICATHighPriority mHighPriority;
};

// Structure to describe indication TLV 0x1E for CAT2 EventReport
struct sCAT2EventReportIndication_UserControl
{
   eQMICATUserControl mUserControl;
};

// Structure to describe indication TLV 0x1F for CAT2 EventReport
struct sCAT2EventReportIndication_Icon
{
   eQMICATIconQualifier mIconQualifier;
   UINT8 mHeight;
   UINT8 mWidth;
   eQMICATImageCodingScheme mImageCodingScheme;
   UINT8 mRecordNumber;
   UINT16 mIconDataLength;

   // This array must be the size specified by mIconDataLength
   // UINT8 mIconData[1];
};

// Structure to describe indication TLV 0x20 for CAT2 EventReport
struct sCAT2EventReportIndication_Duration
{
   eQMICATTimeUnits mUnits;
   UINT8 mInterval;
};

// Structure to describe indication TLV 0x21 for CAT2 EventReport
struct sCAT2EventReportIndication_ResponseFormat
{
   eQMICATResponseFormat mResponseFormat;
};

// Structure to describe indication TLV 0x22 for CAT2 EventReport
struct sCAT2EventReportIndication_HelpAvailable
{
   eQMICATHelpAvailable mHelpAvailable;
};

// Structure to describe indication TLV 0x23 for CAT2 EventReport
struct sCAT2EventReportIndication_ResponsePackingFormat
{
   eQMICATResponsePackingFormat mResponsePackingFormat;
};

// Structure to describe indication TLV 0x24 for CAT2 EventReport
struct sCAT2EventReportIndication_ResponseLength
{
   UINT8 mMaximumUserInput;
   UINT8 mMinimumUserInput;
};

// Structure to describe indication TLV 0x25 for CAT2 EventReport
struct sCAT2EventReportIndication_ShowUserInput
{
   eQMICATShowUserInput mShowUserInput;
};

// Structure to describe indication TLV 0x26 for CAT2 EventReport
struct sCAT2EventReportIndication_Tone
{
   eQMICATTone mTone;
};

// Structure to describe indication TLV 0x27 for CAT2 EventReport
struct sCAT2EventReportIndication_SoftkeySelection
{
   eQMICATSoftkeySelection mSoftkeySelection;
};

// Structure to describe indication TLV 0x28 for CAT2 EventReport
struct sCAT2EventReportIndication_Items
{
   UINT8 mItemsLength;

   struct sItem
   {
      UINT8 mItemID;
      UINT8 mItemTextLength;
   
      // This array must be the size specified by mItemTextLength
      // UINT8 mItemText[1];
   };

   // This array must be the size specified by mItemsLength
   // sItem mItems[1];
};

// Structure to describe indication TLV 0x29 for CAT2 EventReport
struct sCAT2EventReportIndication_DefaultItem
{
   UINT8 mDefaultItem;
};

// Structure to describe indication TLV 0x2A for CAT2 EventReport
struct sCAT2EventReportIndication_NextActionIdentifier
{
   UINT8 mActionsLength;

   // This array must be the size specified by mActionsLength
   // eQMICATNextAction mNextAction[1];
};

// Structure to describe indication TLV 0x2B for CAT2 EventReport
struct sCAT2EventReportIndication_IconIDList
{
   eQMICATDisplayIconOnly mDisplayIconOnly;
   UINT8 mItemsLength;

   struct sItem
   {
      eQMICATIconQualifier mIconQualifier;
      UINT8 mHeight;
      UINT8 mWidth;
      eQMICATImageCodingScheme mImageCodingScheme;
      UINT8 mRecordNumber;
      UINT16 mIconDataLength;
   
      // This array must be the size specified by mIconDataLength
      // UINT8 mIconData[1];
   };

   // This array must be the size specified by mItemsLength
   // sItem mItems[1];
};

// Structure to describe indication TLV 0x2C for CAT2 EventReport
struct sCAT2EventReportIndication_Presentation
{
   eQMICATPresentation mPresentation;
};

// Structure to describe indication TLV 0x2D for CAT2 EventReport
struct sCAT2EventReportIndication_PackingRequired
{
   eQMICATPackingRequired mPackingRequired;
};

// Structure to describe indication TLV 0x2E for CAT2 EventReport
struct sCAT2EventReportIndication_SMSTPDU
{
   UINT8 mSMSTPDUDataLength;

   // This array must be the size specified by mSMSTPDUDataLength
   // UINT8 mSMSTPDUData[1];
};

// Structure to describe indication TLV 0x2F for CAT2 EventReport
struct sCAT2EventReportIndication_IsCDMASMS
{
   eQMICATIsCDMASMS mIsCDMASMS;
};

// Structure to describe indication TLV 0x30 for CAT2 EventReport
struct sCAT2EventReportIndication_Address
{
   eQMICATAddressTON mAddressTON;
   eQMICATAddressNPI mAddressNPI;
   UINT8 mAddressDataLength;

   // This array must be the size specified by mAddressDataLength
   // char mAddressData[1];
};

// Structure to describe indication TLV 0x31 for CAT2 EventReport
struct sCAT2EventReportIndication_CallSetupRequirement
{
   eQMICATCallSetupRequirement mCallSetupRequirement;
};

// Structure to describe indication TLV 0x32 for CAT2 EventReport
struct sCAT2EventReportIndication_Redial
{
   eQMICATRedialNecessary mRedialNecessary;
   eQMICATTimeUnits mUnits;
   UINT8 mInterval;
};

// Structure to describe indication TLV 0x33 for CAT2 EventReport
struct sCAT2EventReportIndication_Subaddress
{
   UINT8 mSubaddressDataLength;

   struct sSubaddressData
   {
      UINT8 mSubaddressData1:4;
      UINT8 mSubaddressData2:4;
   };

   // This array must be the size specified by mSubaddressDataLength
   // sSubaddressData mSubaddressDatas[1];
};

// Structure to describe indication TLV 0x34 for CAT2 EventReport
struct sCAT2EventReportIndication_CapabilitiesConfiguration
{
   UINT8 mCapabilitesConfigurationLength;

   // This array must be the size specified by mCapabilitesConfigurationLength
   // UINT8 mCapabilitiesConfiguration[1];
};

// Structure to describe indication TLV 0x35 for CAT2 EventReport
struct sCAT2EventReportIndication_DTMF
{
   UINT8 mDTMFDataLength;

   struct sDTMFData
   {
      UINT8 mDTMFData1:4;
      UINT8 mDTMFData2:4;
   };

   // This array must be the size specified by mDTMFDataLength
   // sDTMFData mDTMFDatas[1];
};

// Structure to describe indication TLV 0x36 for CAT2 EventReport
struct sCAT2EventReportIndication_SpecificLanguageNotification
{
   eQMICATSpecificLanguageNotfication mSpecificLanguageNotification;
};

// Structure to describe indication TLV 0x37 for CAT2 EventReport
struct sCAT2EventReportIndication_Language
{
   char mLanguage[2];
};

// Structure to describe indication TLV 0x38 for CAT2 EventReport
struct sCAT2EventReportIndication_LaunchMode
{
   eQMICATLaunchMode mLaunchMode;
};

// Structure to describe indication TLV 0x39 for CAT2 EventReport
struct sCAT2EventReportIndication_URL
{
   UINT8 mURLDataLength;

   // This array must be the size specified by mURLDataLength
   // char mURLData[1];
};

// Structure to describe indication TLV 0x3A for CAT2 EventReport
struct sCAT2EventReportIndication_BrowserID
{
   UINT8 mBrowserID;
};

// Structure to describe indication TLV 0x3B for CAT2 EventReport
struct sCAT2EventReportIndication_BearerList
{
   UINT8 mBearerListLength;

   // This array must be the size specified by mBearerListLength
   // eQMICATBearer mBearerList[1];
};

// Structure to describe indication TLV 0x3C for CAT2 EventReport
struct sCAT2EventReportIndication_ProvisioningFile
{
   UINT32 mNumberOfProvisioningFiles;

   struct sFile
   {
      UINT8 mPathLength;
   
      // This array must be the size specified by mPathLength
      // char mPath[1];
   };

   // This array must be the size specified by mNumberOfProvisioningFiles
   // sFile mFiles[1];
};

// Structure to describe indication TLV 0x3D for CAT2 EventReport
struct sCAT2EventReportIndication_USSDString
{
   eQMICATUSSDDataCodingScheme mOriginalDataCodingScheme;
   eQMICATUSSDDataCodingScheme mDataCodingScheme;
   UINT8 mUSSDTextLength;

   // This array must be the size specified by mUSSDTextLength
   // UINT8 mUSSDText[1];
};

// Structure to describe indication TLV 0x3E for CAT2 EventReport
struct sCAT2EventReportIndication_DefaultText
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe indication TLV 0x3F for CAT2 EventReport
struct sCAT2EventReportIndication_ImmediateResponseRequired
{
   eQMICATImmediateResponse mImmediateResponse;
};

// Structure to describe indication TLV 0x40 for CAT2 EventReport
struct sCAT2EventReportIndication_UserConfirmationAlpha
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe indication TLV 0x41 for CAT2 EventReport
struct sCAT2EventReportIndication_SetupCallDisplayAlpha
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe indication TLV 0x42 for CAT2 EventReport
struct sCAT2EventReportIndication_UserConfirmationIcon
{
   eQMICATIconQualifier mIconQualifier;
   UINT8 mHeight;
   UINT8 mWidth;
   eQMICATImageCodingScheme mImageCodingScheme;
   UINT8 mRecordNumber;
   UINT16 mIconDataLength;

   // This array must be the size specified by mIconDataLength
   // UINT8 mIconData[1];
};

// Structure to describe indication TLV 0x43 for CAT2 EventReport
struct sCAT2EventReportIndication_SetupCallDisplayIcon
{
   eQMICATIconQualifier mIconQualifier;
   UINT8 mHeight;
   UINT8 mWidth;
   eQMICATImageCodingScheme mImageCodingScheme;
   UINT8 mRecordNumber;
   UINT16 mIconDataLength;

   // This array must be the size specified by mIconDataLength
   // UINT8 mIconData[1];
};

// Structure to describe indication TLV 0x44 for CAT2 EventReport
struct sCAT2EventReportIndication_GatewayProxy
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe indication TLV 0x45 for CAT2 EventReport
struct sCAT2EventReportIndication_Alpha
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe indication TLV 0x46 for CAT2 EventReport
struct sCAT2EventReportIndication_NotificationRequired
{
   eQMICATNotificationRequired mNotificationRequired;
};

// Structure to describe indication TLV 0x47 for CAT2 EventReport
struct sCAT2EventReportIndication_PlayToneEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mPlayToneCommand[1];
};

// Structure to describe indication TLV 0x48 for CAT2 EventReport
struct sCAT2EventReportIndication_SetupCallEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSetupCallCommand[1];
};

// Structure to describe indication TLV 0x49 for CAT2 EventReport
struct sCAT2EventReportIndication_SendDTMFEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendDTMFCommand[1];
};

// Structure to describe indication TLV 0x4A for CAT2 EventReport
struct sCAT2EventReportIndication_LaunchBrowserEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mLaunchBrowserCommand[1];
};

// Structure to describe indication TLV 0x4B for CAT2 EventReport
struct sCAT2EventReportIndication_SendSMSEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendSMSCommand[1];
};

// Structure to describe indication TLV 0x4C for CAT2 EventReport
struct sCAT2EventReportIndication_SendSSEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendSSCommand[1];
};

// Structure to describe indication TLV 0x4D for CAT2 EventReport
struct sCAT2EventReportIndication_SendUSSDEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendUSSDCommand[1];
};

// Structure to describe indication TLV 0x4E for CAT2 EventReport
struct sCAT2EventReportIndication_ProvideLocalInformationEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mProvideLocalInformationCommand[1];
};

// Structure to describe indication TLV 0x4F for CAT2 EventReport
struct sCAT2EventReportIndication_SetupRawEventList
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSetupEventListCommand[1];
};

// Structure to describe indication TLV 0x50 for CAT2 EventReport
struct sCAT2EventReportIndication_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe indication TLV 0x51 for CAT2 EventReport
struct sCAT2EventReportIndication_OpenChannelEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mOpenChannelCommand[1];
};

// Structure to describe indication TLV 0x52 for CAT2 EventReport
struct sCAT2EventReportIndication_CloseChannelEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mCloseChannelCommand[1];
};

// Structure to describe indication TLV 0x53 for CAT2 EventReport
struct sCAT2EventReportIndication_SendDataEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendDataCommand[1];
};

// Structure to describe indication TLV 0x54 for CAT2 EventReport
struct sCAT2EventReportIndication_ReceiveDataEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mReceiveDataCommand[1];
};

// Structure to describe indication TLV 0x55 for CAT2 EventReport
struct sCAT2EventReportIndication_OnDemmandLinkEstablish
{
   eQMICATOnDemandLinkEstablish mOnDemandLinkEstablish;
};

// Structure to describe indication TLV 0x56 for CAT2 EventReport
struct sCAT2EventReportIndication_CSDBearerDescription
{
   UINT8 mSpeed;
   eQMICATCSDBearerName mCSDBearerName;
   eQMICATConnectionElement mConnectionElement;
};

// Structure to describe indication TLV 0x57 for CAT2 EventReport
struct sCAT2EventReportIndication_GPRSBearerDescription
{
   UINT8 mPrecedenceClass;
   UINT8 mDelayClass;
   UINT8 mReliabilityClass;
   UINT8 mPeakThroughput;
   UINT8 mMeanThroughput;
   eQMICATPacketDataProtocol mPacketDataProtocol;
};

// Structure to describe indication TLV 0x58 for CAT2 EventReport
struct sCAT2EventReportIndication_EUTRANExternalParameterBearerDescription
{
   eQMICATTrafficClass mTrafficClass;
   UINT16 mMaxUploadBitrate;
   UINT16 mMaxDownloadBitrate;
   UINT16 mGuaranteedUploadBitrate;
   UINT16 mGuaranteedDownloadBitrate;
   eQMICATDeliveryOrder mDeliveryOrder;
   UINT8 mMaxSDUSize;
   UINT8 mMaxSDUErrorRatio;
   UINT8 mResidualBitErrorRatio;
   eQMICATDeliverErrorSDU mDeliverErrorSDU;
   UINT8 mTransferDelay;
   UINT8 mTrafficHandlingPRI;
   eQMICATPDPType mPDPType;
};

// Structure to describe indication TLV 0x59 for CAT2 EventReport
struct sCAT2EventReportIndication_EUTRANExternalMappedUTRANBearerDescription
{
   UINT8 mQCI;
   UINT8 mMaxUploadBitrate;
   UINT8 mMaxDownloadBitrate;
   UINT8 mGuaranteedUploadBitrate;
   UINT8 mGuaranteedDownloadBitrate;
   UINT8 mMaximumUploadBitrateExt;
   UINT8 mMaximumDownloadBitrateExt;
   UINT8 mGuaranteedUploadBitrateExt;
   UINT8 mGuaranteedDownloadBitrateExt;
   eQMICATPDPType mPDPType;
};

// Structure to describe indication TLV 0x5A for CAT2 EventReport
struct sCAT2EventReportIndication_BufferSize
{
   UINT16 mBufferSize;
};

// Structure to describe indication TLV 0x5B for CAT2 EventReport
struct sCAT2EventReportIndication_NetworkAccessName
{
   UINT8 mNetworkAccessNameLength;

   // This array must be the size specified by mNetworkAccessNameLength
   // UINT8 mNetworkAccessName[1];
};

// Structure to describe indication TLV 0x5C for CAT2 EventReport
struct sCAT2EventReportIndication_OtherAddress
{
   eQMICATAddressType mAddressType;
   UINT8 mAddressDataLength;

   // This array must be the size specified by mAddressDataLength
   // char mAddressData[1];
};

// Structure to describe indication TLV 0x5D for CAT2 EventReport
struct sCAT2EventReportIndication_UserLogin
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe indication TLV 0x5E for CAT2 EventReport
struct sCAT2EventReportIndication_UserPassword
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe indication TLV 0x5F for CAT2 EventReport
struct sCAT2EventReportIndication_TransportLevel
{
   eQMICATTransportProtocol mTransportProtocol;
   UINT16 mPortNumber;
};

// Structure to describe indication TLV 0x60 for CAT2 EventReport
struct sCAT2EventReportIndication_DataDestinationAddress
{
   eQMICATAddressType mAddressType;
   UINT8 mAddressDataLength;

   // This array must be the size specified by mAddressDataLength
   // char mAddressData[1];
};

// Structure to describe indication TLV 0x61 for CAT2 EventReport
struct sCAT2EventReportIndication_ChannelDataLength
{
   UINT8 mChannelDataLength;
};

// Structure to describe indication TLV 0x62 for CAT2 EventReport
struct sCAT2EventReportIndication_SendDataImmediately
{
   eQMICATSendDataImmediately mSendDataImmediately;
};

// Structure to describe indication TLV 0x63 for CAT2 EventReport
struct sCAT2EventReportIndication_ChannelData
{
   UINT16 mChannelDataLength;

   // This array must be the size specified by mChannelDataLength
   // UINT8 mChannelData[1];
};

// Structure to describe indication TLV 0x64 for CAT2 EventReport
struct sCAT2EventReportIndication_ChannelID
{
   UINT8 mChannelID;
};

// Structure to describe indication TLV 0x65 for CAT2 EventReport
struct sCAT2EventReportIndication_ItemsWithDCS
{
   UINT8 mItemsLength;

   struct sItem
   {
      UINT8 mItemID;
      eQMICATDataCodingScheme mDataCodingScheme;
      UINT8 mItemTextLength;
   
      // This array must be the size specified by mItemTextLength
      // UINT8 mItemText[1];
   };

   // This array must be the size specified by mItemsLength
   // sItem mItems[1];
};

// Structure to describe indication TLV 0x66 for CAT2 EventReport
struct sCAT2EventReportIndication_Activate
{
   UINT32 mReferenceID;
   UINT16 mActivateLength;

   // This array must be the size specified by mActivateLength
   // UINT8 mActivate[1];
};

// Structure to describe indication TLV 0x67 for CAT2 EventReport
struct sCAT2EventReportIndication_ActivateTarget
{
   eQMICATActivateTargets mActivateTarget;
};

// Structure to describe response TLV 0x01 for CAT2GetServiceState()
struct sCAT2GetServiceStateResponse_CATServiceState
{
   bool mCommonDisplayText:1;
   bool mCommonGetInkey:1;
   bool mCommonGetInput:1;
   bool mCommonSetupMenu:1;
   bool mCommonSelectItem:1;
   bool mCommonSendSMSAlphaIdentifier:1;
   bool mCommonSetupEventUserActivity:1;
   bool mCommonSetupEventIdleScreenNotify:1;
   bool mCommonSetupEventLanguageSelNotify:1;
   bool mCommonSetupIdleModeText:1;
   bool mCommonLanguageNotification:1;
   bool mCommonRefresh:1;
   bool mCommonEndProactiveSession:1;
   bool mCommonPlayTone:1;
   bool mCommonSetupCall:1;
   bool mCommonSendDTMF:1;
   bool mCommonLaunchBrowser:1;
   bool mCommonSendSS:1;
   bool mCommonSendUSSD:1;
   bool mCommonProvideLocalInformationLanguage:1;
   bool mCommonBearerIndependentProtocol:1;
   bool mCommonSetupEventBrowserTermination:1;
   bool mCommonProvideLocalInformationTime:1;
   bool mCommonActivate:1;

   // Padding out 1 bits
   UINT8 mReserved1:1;

   bool mCommonSetupEventHCIConnectivity:1;

   // Padding out 6 bits
   UINT8 mReserved2:6;

   bool mControlDisplayText:1;
   bool mControlGetInkey:1;
   bool mControlGetInput:1;
   bool mControlSetupMenu:1;
   bool mControlSelectItem:1;
   bool mControlSendSMSAlphaIdentifier:1;
   bool mControlSetupEventUserActivity:1;
   bool mControlSetupEventIdleScreenNotify:1;
   bool mControlSetupEventLanguageSelNotify:1;
   bool mControlSetupIdleModeText:1;
   bool mControlLanguageNotification:1;
   bool mControlRefresh:1;
   bool mControlEndProactiveSession:1;
   bool mControlPlayTone:1;
   bool mControlSetupCall:1;
   bool mControlSendDTMF:1;
   bool mControlLaunchBrowser:1;
   bool mControlSendSS:1;
   bool mControlSendUSSD:1;
   bool mControlProvideLocalInformationLanguage:1;
   bool mControlBearerIndependentProtocol:1;
   bool mControlSetupEventBrowserTermination:1;
   bool mControlProvideLocalInformationTime:1;
   bool mControlActivate:1;

   // Padding out 1 bits
   UINT8 mReserved3:1;

   bool mControlSetupEventHCIConnectivity:1;

   // Padding out 6 bits
   UINT8 mReserved4:6;
};

// Structure to describe response TLV 0x10 for CAT2GetServiceState()
struct sCAT2GetServiceStateResponse_DecodedCATServiceState
{
   bool mCommonDisplayText:1;
   bool mCommonGetInkey:1;
   bool mCommonGetInput:1;
   bool mCommonSetupMenu:1;
   bool mCommonSelectItem:1;
   bool mCommonSendSMSAlphaIdentifier:1;
   bool mCommonSetupEventUserActivity:1;
   bool mCommonSetupEventIdleScreenNotify:1;
   bool mCommonSetupEventLanguageSelNotify:1;
   bool mCommonSetupIdleModeText:1;
   bool mCommonLanguageNotification:1;

   // Padding out 1 bits
   UINT8 mReserved1:1;

   bool mCommonEndProactiveSession:1;
   bool mCommonPlayTone:1;
   bool mCommonSetupCall:1;
   bool mCommonSendDTMF:1;
   bool mCommonLaunchBrowser:1;
   bool mCommonSendSS:1;
   bool mCommonSendUSSD:1;
   bool mCommonProvideLocalInformationLanguage:1;
   bool mCommonBearerIndependentProtocol:1;

   // Padding out 2 bits
   UINT8 mReserved2:2;

   bool mCommonSCWSEvent:1;
   bool mCommonActivate:1;
   bool mCommonSetupEventHCIConnectivity:1;

   // Padding out 6 bits
   UINT8 mReserved3:6;

   bool mControlDisplayText:1;
   bool mControlGetInkey:1;
   bool mControlGetInput:1;
   bool mControlSetupMenu:1;
   bool mControlSelectItem:1;
   bool mControlSendSMSAlphaIdentifier:1;
   bool mControlSetupEventUserActivity:1;
   bool mControlSetupEventIdleScreenNotify:1;
   bool mControlSetupEventLanguageSelNotify:1;
   bool mControlSetupIdleModeText:1;
   bool mControlLanguageNotification:1;

   // Padding out 1 bits
   UINT8 mReserved4:1;

   bool mControlEndProactiveSession:1;
   bool mControlPlayTone:1;
   bool mControlSetupCall:1;
   bool mControlSendDTMF:1;
   bool mControlLaunchBrowser:1;
   bool mControlSendSS:1;
   bool mControlSendUSSD:1;
   bool mControlProvideLocalInformationLanguage:1;
   bool mControlBearerIndependentProtocol:1;

   // Padding out 2 bits
   UINT8 mReserved5:2;

   bool mControlSCWSEvent:1;
   bool mControlActivate:1;
   bool mControlSetupEventHCIConnectivity:1;

   // Padding out 6 bits
   UINT8 mReserved6:6;
};

// Structure to describe request TLV 0x01 for CAT2SendTerminalResponse()
struct sCAT2SendTerminalResponseRequest_TerminalResponseType
{
   UINT32 mReferenceID;
   UINT16 mTerminalResponseLength;

   // This array must be the size specified by mTerminalResponseLength
   // UINT8 mTerminalResponse[1];
};

// Structure to describe request TLV 0x10 for CAT2SendTerminalResponse()
struct sCAT2SendTerminalResponseRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe response TLV 0x10 for CAT2SendTerminal()
struct sCAT2SendTerminalResponseResponse_TRResponse
{
   UINT8 mSW1;
   UINT8 mSW2;
   UINT8 mTerminalResponseLength;

   // This array must be the size specified by mTerminalResponseLength
   // UINT8 mTerminalResponseData[1];
};

// Structure to describe request TLV 0x01 for CAT2EnvelopeCommand()
struct sCAT2EnvelopeCommandRequest_EnvelopeCommand
{
   eQMICATEnvelopeCommandType mEnvelopeCommandType;
   UINT16 mEnvelopeLength;

   // This array must be the size specified by mEnvelopeLength
   // UINT8 mEnvelopeData[1];
};

// Structure to describe request TLV 0x10 for CAT2EnvelopeCommand()
struct sCAT2EnvelopeCommandRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe response TLV 0x10 for CAT2EnvelopeCommand()
struct sCAT2EnvelopeCommandResponse_RawResponse
{
   UINT8 mSW1;
   UINT8 mSW2;
   UINT8 mEnvelopeResponseLength;

   // This array must be the size specified by mEnvelopeResponseLength
   // UINT8 mEnvelopeResponseData[1];
};

// Structure to describe request TLV 0x01 for CAT2GetEventReport()
struct sCAT2GetEventReportRequest_CommandInput
{
   UINT32 mCommandID;
   eQMICATCommandFormat mCommandFormat;
};

// Structure to describe response TLV 0x10 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_DisplayTextEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mDisplayTextCommand[1];
};

// Structure to describe response TLV 0x11 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_GetInkeyEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mGetInkeyCommand[1];
};

// Structure to describe response TLV 0x12 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_GetInputEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mGetInputCommand[1];
};

// Structure to describe response TLV 0x13 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_SetupMenuEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSetupMenuCommand[1];
};

// Structure to describe response TLV 0x14 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_SelectItemEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSelectItemCommand[1];
};

// Structure to describe response TLV 0x15 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_AlphaIDAvailable
{
   eQMICATAlphaIDCommandType mAlphaIDCommandType;
   UINT16 mAlphaIDLength;

   // This array must be the size specified by mAlphaIDLength
   // UINT8 mAlphaID[1];
};

// Structure to describe response TLV 0x16 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_SetupEventList
{
   bool mUserActivityNotify:1;
   bool mIdleScreenAvailable:1;
   bool mLanguageSelectionNotify:1;

   // Padding out 29 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2[3];
};

// Structure to describe response TLV 0x17 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_SetupIdleModeTextEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSetupIdleModeTextCommand[1];
};

// Structure to describe response TLV 0x18 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_LanguageNotificationEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mLanguageNotificationCommand[1];
};

// Structure to describe response TLV 0x19 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_RefreshEvent
{
   UINT16 mRefreshMode;
   eQMICATRefreshStage mRefreshStage;
};

// Structure to describe response TLV 0x1A for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_EndProactiveSession
{
   eQMICATProactiveSessionEndType mProactiveSessionEndType;
};

// Structure to describe response TLV 0x1B for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_DecodedHeaderID
{
   eQMICATCommandID mCommandID;
   UINT32 mReferenceID;
   UINT8 mCommandNumber;
};

// Structure to describe response TLV 0x1C for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_TextString
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x1D for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_HighPriority
{
   eQMICATHighPriority mHighPriority;
};

// Structure to describe response TLV 0x1E for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_UserControl
{
   eQMICATUserControl mUserControl;
};

// Structure to describe response TLV 0x1F for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_Icon
{
   eQMICATIconQualifier mIconQualifier;
   UINT8 mHeight;
   UINT8 mWidth;
   eQMICATImageCodingScheme mImageCodingScheme;
   UINT8 mRecordNumber;
   UINT16 mIconDataLength;

   // This array must be the size specified by mIconDataLength
   // UINT8 mIconData[1];
};

// Structure to describe response TLV 0x20 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_Duration
{
   eQMICATTimeUnits mUnits;
   UINT8 mInterval;
};

// Structure to describe response TLV 0x21 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_ResponseFormat
{
   eQMICATResponseFormat mResponseFormat;
};

// Structure to describe response TLV 0x22 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_HelpAvailable
{
   eQMICATHelpAvailable mHelpAvailable;
};

// Structure to describe response TLV 0x23 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_ResponsePackingFormat
{
   eQMICATResponsePackingFormat mResponsePackingFormat;
};

// Structure to describe response TLV 0x24 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_ResponseLength
{
   UINT8 mMaximumUserInput;
   UINT8 mMinimumUserInput;
};

// Structure to describe response TLV 0x25 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_ShowUserInput
{
   eQMICATShowUserInput mShowUserInput;
};

// Structure to describe response TLV 0x26 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_Tone
{
   eQMICATTone mTone;
};

// Structure to describe response TLV 0x27 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_SoftkeySelection
{
   eQMICATSoftkeySelection mSoftkeySelection;
};

// Structure to describe response TLV 0x28 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_Items
{
   UINT8 mItemsLength;

   struct sItem
   {
      UINT8 mItemID;
      UINT8 mItemTextLength;
   
      // This array must be the size specified by mItemTextLength
      // UINT8 mItemText[1];
   };

   // This array must be the size specified by mItemsLength
   // sItem mItems[1];
};

// Structure to describe response TLV 0x29 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_DefaultItems
{
   UINT8 mDefaultItem;
};

// Structure to describe response TLV 0x2A for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_NextActionIdentifier
{
   UINT8 mActionsLength;

   // This array must be the size specified by mActionsLength
   // eQMICATNextAction mNextAction[1];
};

// Structure to describe response TLV 0x2B for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_IconIDList
{
   eQMICATDisplayIconOnly mDisplayIconOnly;
   UINT8 mItemsLength;

   struct sItem
   {
      eQMICATIconQualifier mIconQualifier;
      UINT8 mHeight;
      UINT8 mWidth;
      eQMICATImageCodingScheme mImageCodingScheme;
      UINT8 mRecordNumber;
      UINT16 mIconDataLength;
   
      // This array must be the size specified by mIconDataLength
      // UINT8 mIconData[1];
   };

   // This array must be the size specified by mItemsLength
   // sItem mItems[1];
};

// Structure to describe response TLV 0x2C for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_Presentation
{
   eQMICATPresentation mPresentation;
};

// Structure to describe response TLV 0x2D for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_PackingRequired
{
   eQMICATPackingRequired mPackingRequired;
};

// Structure to describe response TLV 0x2E for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_SMSTPDU
{
   UINT8 mSMSTPDUDataLength;

   // This array must be the size specified by mSMSTPDUDataLength
   // UINT8 mSMSTPDUData[1];
};

// Structure to describe response TLV 0x2F for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_IsCDMASMS
{
   eQMICATIsCDMASMS mIsCDMASMS;
};

// Structure to describe response TLV 0x30 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_Address
{
   eQMICATAddressTON mAddressTON;
   eQMICATAddressNPI mAddressNPI;
   UINT8 mAddressDataLength;

   // This array must be the size specified by mAddressDataLength
   // char mAddressData[1];
};

// Structure to describe response TLV 0x31 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_CallSetupRequirement
{
   eQMICATCallSetupRequirement mCallSetupRequirement;
};

// Structure to describe response TLV 0x32 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_Redial
{
   eQMICATRedialNecessary mRedialNecessary;
   eQMICATTimeUnits mUnits;
   UINT8 mInterval;
};

// Structure to describe response TLV 0x33 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_Subaddress
{
   UINT8 mSubaddressDataLength;

   struct sSubaddressData
   {
      UINT8 mSubaddressData1:4;
      UINT8 mSubaddressData2:4;
   };

   // This array must be the size specified by mSubaddressDataLength
   // sSubaddressData mSubaddressDatas[1];
};

// Structure to describe response TLV 0x34 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_CapabilityConfiguration
{
   UINT8 mCapabilitesConfigurationLength;

   // This array must be the size specified by mCapabilitesConfigurationLength
   // UINT8 mCapabilitiesConfiguration[1];
};

// Structure to describe response TLV 0x35 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_DTMF
{
   UINT8 mDTMFDataLength;

   struct sDTMFData
   {
      UINT8 mDTMFData1:4;
      UINT8 mDTMFData2:4;
   };

   // This array must be the size specified by mDTMFDataLength
   // sDTMFData mDTMFDatas[1];
};

// Structure to describe response TLV 0x36 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_SpecificLanguageNotification
{
   eQMICATSpecificLanguageNotfication mSpecificLanguageNotification;
};

// Structure to describe response TLV 0x37 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_Language
{
   char mLanguage[2];
};

// Structure to describe response TLV 0x38 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_LaunchMode
{
   eQMICATLaunchMode mLaunchMode;
};

// Structure to describe response TLV 0x39 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_URL
{
   UINT8 mURLDataLength;

   // This array must be the size specified by mURLDataLength
   // char mURLData[1];
};

// Structure to describe response TLV 0x3A for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_BrowserID
{
   UINT8 mBrowserID;
};

// Structure to describe response TLV 0x3B for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_BearerList
{
   UINT8 mBearerListLength;

   // This array must be the size specified by mBearerListLength
   // eQMICATBearer mBearerList[1];
};

// Structure to describe response TLV 0x3C for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_ProvisioningFiles
{
   UINT32 mNumberOfProvisioningFiles;

   struct sFile
   {
      UINT8 mPathLength;
   
      // This array must be the size specified by mPathLength
      // char mPath[1];
   };

   // This array must be the size specified by mNumberOfProvisioningFiles
   // sFile mFiles[1];
};

// Structure to describe response TLV 0x3D for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_USSDString
{
   eQMICATUSSDDataCodingScheme mOriginalDataCodingScheme;
   eQMICATUSSDDataCodingScheme mDataCodingScheme;
   UINT8 mUSSDTextLength;

   // This array must be the size specified by mUSSDTextLength
   // UINT8 mUSSDText[1];
};

// Structure to describe response TLV 0x3E for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_DefaultText
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x3F for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_ImmediateResponseRequest
{
   eQMICATImmediateResponse mImmediateResponse;
};

// Structure to describe response TLV 0x40 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_UserConfirmationAlpha
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x41 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_SetupCallDisplayAlpha
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x42 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_UserConfirmationIcon
{
   eQMICATIconQualifier mIconQualifier;
   UINT8 mHeight;
   UINT8 mWidth;
   eQMICATImageCodingScheme mImageCodingScheme;
   UINT8 mRecordNumber;
   UINT16 mIconDataLength;

   // This array must be the size specified by mIconDataLength
   // UINT8 mIconData[1];
};

// Structure to describe response TLV 0x43 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_SetupCallDisplayIcon
{
   eQMICATIconQualifier mIconQualifier;
   UINT8 mHeight;
   UINT8 mWidth;
   eQMICATImageCodingScheme mImageCodingScheme;
   UINT8 mRecordNumber;
   UINT16 mIconDataLength;

   // This array must be the size specified by mIconDataLength
   // UINT8 mIconData[1];
};

// Structure to describe response TLV 0x44 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_GatewayProxy
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x45 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_Alpha
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x46 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_NotificationRequired
{
   eQMICATNotificationRequired mNotificationRequired;
};

// Structure to describe response TLV 0x47 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_PlayToneEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mPlayToneCommand[1];
};

// Structure to describe response TLV 0x48 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_SetupCallEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSetupCallCommand[1];
};

// Structure to describe response TLV 0x49 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_SendDTMFEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendDTMFCommand[1];
};

// Structure to describe response TLV 0x4A for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_LaunchBrowserEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mLaunchBrowserCommand[1];
};

// Structure to describe response TLV 0x4B for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_SendSMSEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendSMSCommand[1];
};

// Structure to describe response TLV 0x4C for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_SendSSEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendSSCommand[1];
};

// Structure to describe response TLV 0x4D for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_SendUSSDEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendUSSDCommand[1];
};

// Structure to describe response TLV 0x4E for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_ProvideLocalInformationEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mProvideLocalInformationCommand[1];
};

// Structure to describe response TLV 0x4F for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_SetupEventListRawEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSetupEventListCommand[1];
};

// Structure to describe response TLV 0x50 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe response TLV 0x51 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_OpenChannelEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mOpenChannelCommand[1];
};

// Structure to describe response TLV 0x52 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_CloseChannelEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mCloseChannelCommand[1];
};

// Structure to describe response TLV 0x53 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_SendDataEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendDataCommand[1];
};

// Structure to describe response TLV 0x54 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_ReceiveDataEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mReceiveDataCommand[1];
};

// Structure to describe response TLV 0x55 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_OnDemandLinkEstablish
{
   eQMICATOnDemandLinkEstablish mOnDemandLinkEstablish;
};

// Structure to describe response TLV 0x56 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_CSDBearerDescription
{
   UINT8 mSpeed;
   eQMICATCSDBearerName mCSDBearerName;
   eQMICATConnectionElement mConnectionElement;
};

// Structure to describe response TLV 0x57 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_GPRSBearerDescription
{
   UINT8 mPrecedenceClass;
   UINT8 mDelayClass;
   UINT8 mReliabilityClass;
   UINT8 mPeakThroughput;
   UINT8 mMeanThroughput;
   eQMICATPacketDataProtocol mPacketDataProtocol;
};

// Structure to describe response TLV 0x58 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_EUTRANExternalParameterBearerDescription
{
   eQMICATTrafficClass mTrafficClass;
   UINT16 mMaxUploadBitrate;
   UINT16 mMaxDownloadBitrate;
   UINT16 mGuaranteedUploadBitrate;
   UINT16 mGuaranteedDownloadBitrate;
   eQMICATDeliveryOrder mDeliveryOrder;
   UINT8 mMaxSDUSize;
   UINT8 mMaxSDUErrorRatio;
   UINT8 mResidualBitErrorRatio;
   eQMICATDeliverErrorSDU mDeliverErrorSDU;
   UINT8 mTransferDelay;
   UINT8 mTrafficHandlingPRI;
   eQMICATPDPType mPDPType;
};

// Structure to describe response TLV 0x59 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_EUTRANExternalMappedUTRANBearerDescription
{
   UINT8 mQCI;
   UINT8 mMaxUploadBitrate;
   UINT8 mMaxDownloadBitrate;
   UINT8 mGuaranteedUploadBitrate;
   UINT8 mGuaranteedDownloadBitrate;
   UINT8 mMaximumUploadBitrateExt;
   UINT8 mMaximumDownloadBitrateExt;
   UINT8 mGuaranteedUploadBitrateExt;
   UINT8 mGuaranteedDownloadBitrateExt;
   eQMICATPDPType mPDPType;
};

// Structure to describe response TLV 0x5A for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_BufferSize
{
   UINT16 mBufferSize;
};

// Structure to describe response TLV 0x5B for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_NetworkAccessName
{
   UINT8 mNetworkAccessNameLength;

   // This array must be the size specified by mNetworkAccessNameLength
   // UINT8 mNetworkAccessName[1];
};

// Structure to describe response TLV 0x5C for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_OtherAddress
{
   eQMICATAddressType mAddressType;
   UINT8 mAddressDataLength;

   // This array must be the size specified by mAddressDataLength
   // char mAddressData[1];
};

// Structure to describe response TLV 0x5D for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_UserLogin
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x5E for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_UserPassword
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x5F for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_TransportLevel
{
   eQMICATTransportProtocol mTransportProtocol;
   UINT16 mPortNumber;
};

// Structure to describe response TLV 0x60 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_DataDestinationAddress
{
   eQMICATAddressType mAddressType;
   UINT8 mAddressDataLength;

   // This array must be the size specified by mAddressDataLength
   // char mAddressData[1];
};

// Structure to describe response TLV 0x61 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_ChannelDataLength
{
   UINT8 mChannelDataLength;
};

// Structure to describe response TLV 0x62 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_SendDataImmediately
{
   eQMICATSendDataImmediately mSendDataImmediately;
};

// Structure to describe response TLV 0x63 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_ChannelData
{
   UINT16 mChannelDataLength;

   // This array must be the size specified by mChannelDataLength
   // UINT8 mChannelData[1];
};

// Structure to describe response TLV 0x64 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_ChannelID
{
   UINT8 mChannelID;
};

// Structure to describe response TLV 0x65 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_ItemsWithDCS
{
   UINT8 mItemsLength;

   struct sItem
   {
      UINT8 mItemID;
      eQMICATDataCodingScheme mDataCodingScheme;
      UINT8 mItemTextLength;
   
      // This array must be the size specified by mItemTextLength
      // UINT8 mItemText[1];
   };

   // This array must be the size specified by mItemsLength
   // sItem mItems[1];
};

// Structure to describe response TLV 0x66 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_Activate
{
   UINT32 mReferenceID;
   UINT16 mActivateLength;

   // This array must be the size specified by mActivateLength
   // UINT8 mActivate[1];
};

// Structure to describe response TLV 0x67 for CAT2GetEventReport()
struct sCAT2GetEventReportResponse_ActivateTarget
{
   eQMICATActivateTargets mActivateTarget;
};

// Structure to describe request TLV 0x01 for CAT2SendDecodedTerminalResponse()
struct sCAT2SendDecodedTerminalResponseRequest_TerminalResponse
{
   UINT32 mReferenceID;
   UINT8 mCommandNumber;
   eQMICATResponseCommand mResponseCommand;
   UINT8 mGeneralResult;
   UINT8 mAdditionalInformationLength;

   // This array must be the size specified by mAdditionalInformationLength
   // UINT8 mTerminalResponseAdditionalInformation[1];
};

// Structure to describe request TLV 0x10 for CAT2SendDecodedTerminalResponse()
struct sCAT2SendDecodedTerminalResponseRequest_TextString
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe request TLV 0x11 for CAT2SendDecodedTerminalResponse()
struct sCAT2SendDecodedTerminalResponseRequest_ItemIdentifier
{
   UINT8 mItemIdentifier;
};

// Structure to describe request TLV 0x12 for CAT2SendDecodedTerminalResponse()
struct sCAT2SendDecodedTerminalResponseRequest_GetInkeyExtraInfo
{
   eQMICATTimeUnits mUnits;
   UINT8 mInterval;
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe request TLV 0x13 for CAT2SendDecodedTerminalResponse()
struct sCAT2SendDecodedTerminalResponseRequest_LanguageInfo
{
   char mLanguage[2];
};

// Structure to describe request TLV 0x14 for CAT2SendDecodedTerminalResponse()
struct sCAT2SendDecodedTerminalResponseRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe request TLV 0x15 for CAT2SendDecodedTerminalResponse()
struct sCAT2SendDecodedTerminalResponseRequest_GetInkeyYesInput
{
   eQMICATTimeUnits mUnits;
   UINT8 mInterval;
   INT8 mGetInkeyYesInput;
};

// Structure to describe response TLV 0x10 for CAT2SendDecodedTerminal()
struct sCAT2SendDecodedTerminalResponseResponse_TRResponse
{
   UINT8 mSW1;
   UINT8 mSW2;
   UINT8 mTerminalResponseLength;

   // This array must be the size specified by mTerminalResponseLength
   // UINT8 mTerminalResponseData[1];
};

// Structure to describe request TLV 0x01 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandRequest_EnvelopeCommand
{
   eQMICATDecodedEnvelopeCommand mEnvelopeCommand;
};

// Structure to describe request TLV 0x10 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandRequest_ItemIdentifier
{
   UINT8 mItemIdentifier;
};

// Structure to describe request TLV 0x11 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandRequest_HelpRequest
{
   eQMICATHelpRequest mHelpRequest;
};

// Structure to describe request TLV 0x12 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandRequest_Language
{
   char mLanguage[2];
};

// Structure to describe request TLV 0x13 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe request TLV 0x14 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandRequest_Address
{
   eQMICATAddressTON mAddressTON;
   eQMICATAddressNPI mAddressNPI;
   UINT8 mAddressDataLength;

   // This array must be the size specified by mAddressDataLength
   // char mAddressData[1];
};

// Structure to describe request TLV 0x15 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandRequest_Subaddress
{
   UINT8 mSubaddressDataLength;

   struct sSubaddressData
   {
      UINT8 mSubaddressData1:4;
      UINT8 mSubaddressData2:4;
   };

   // This array must be the size specified by mSubaddressDataLength
   // sSubaddressData mSubaddressDatas[1];
};

// Structure to describe request TLV 0x16 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandRequest_CapabilityConfigParam1
{
   UINT8 mCapabilitesConfigurationLength;

   // This array must be the size specified by mCapabilitesConfigurationLength
   // UINT8 mCapabilitiesConfiguration[1];
};

// Structure to describe request TLV 0x17 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandRequest_CapabilityConfigParam2
{
   UINT8 mCapabilitesConfigurationLength;

   // This array must be the size specified by mCapabilitesConfigurationLength
   // UINT8 mCapabilitiesConfiguration[1];
};

// Structure to describe request TLV 0x18 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandRequest_USSDString
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe request TLV 0x19 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandRequest_PDPContextActivation
{
   UINT8 mPDPContextActivationDataLength;

   // This array must be the size specified by mPDPContextActivationDataLength
   // UINT8 mPDPContextActivationData[1];
};

// Structure to describe request TLV 0x1A for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandRequest_EPSPDNConnectActivation
{
   UINT8 mEPSPDNConnectActivationDataLength;

   // This array must be the size specified by mEPSPDNConnectActivationDataLength
   // UINT8 mEPSPDNConnectActivationData[1];
};

// Structure to describe request TLV 0x1B for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandRequest_BrowserTerminationCause
{
   eQMICATBrowserTerminationCauses mBrowserTerminationCause;
};

// Structure to describe response TLV 0x10 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandResponse_CallControlResult
{
   eQMICATCallControlResult mCallControlResult;
};

// Structure to describe response TLV 0x11 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandResponse_Address
{
   eQMICATAddressTON mAddressTON;
   eQMICATAddressNPI mAddressNPI;
   UINT8 mAddressDataLength;

   // This array must be the size specified by mAddressDataLength
   // char mAddressData[1];
};

// Structure to describe response TLV 0x12 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandResponse_Subaddress
{
   UINT8 mSubaddressDataLength;

   struct sSubaddressData
   {
      UINT8 mSubaddressData1:4;
      UINT8 mSubaddressData2:4;
   };

   // This array must be the size specified by mSubaddressDataLength
   // sSubaddressData mSubaddressDatas[1];
};

// Structure to describe response TLV 0x13 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandResponse_CapabilityConfigParam1
{
   UINT8 mCapabilitesConfigurationLength;

   // This array must be the size specified by mCapabilitesConfigurationLength
   // UINT8 mCapabilitiesConfiguration[1];
};

// Structure to describe response TLV 0x14 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandResponse_CapabilityConfigParam2
{
   UINT8 mCapabilitesConfigurationLength;

   // This array must be the size specified by mCapabilitesConfigurationLength
   // UINT8 mCapabilitiesConfiguration[1];
};

// Structure to describe response TLV 0x15 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandResponse_USSDString
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x16 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandResponse_PDPContextActivation
{
   UINT8 mPDPContextActivationDataLength;

   // This array must be the size specified by mPDPContextActivationDataLength
   // UINT8 mPDPContextActivationData[1];
};

// Structure to describe response TLV 0x17 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandResponse_EPSPDNConnectActivation
{
   UINT8 mEPSPDNConnectActivationDataLength;

   // This array must be the size specified by mEPSPDNConnectActivationDataLength
   // UINT8 mEPSPDNConnectActivationData[1];
};

// Structure to describe response TLV 0x18 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandResponse_Alpha
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x19 for CAT2SendDecodedEnvelopeCommand()
struct sCAT2SendDecodedEnvelopeCommandResponse_BCRepeatIndicator
{
   eQMICATBearerCapabilityRepeatIndicator mBearerCapabilityRepeatIndicator;
};

// Structure to describe request TLV 0x10 for CAT2EventConfirmation()
struct sCAT2EventConfirmationRequest_UserConfirmed
{
   eQMICATUserConfirmed mUserConfirmed;
};

// Structure to describe request TLV 0x11 for CAT2EventConfirmation()
struct sCAT2EventConfirmationRequest_IconIsDisplayed
{
   eQMICATIconIsDisplayed mIconIsDisplayed;
};

// Structure to describe request TLV 0x12 for CAT2EventConfirmation()
struct sCAT2EventConfirmationRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe request TLV 0x01 for CAT2SCWSOpenChannel()
struct sCAT2SCWSOpenChannelRequest_ChannelStatus
{
   UINT32 mChannelID;
   eQMICATChannelState mChannelState;
};

// Structure to describe request TLV 0x10 for CAT2SCWSOpenChannel()
struct sCAT2SCWSOpenChannelRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe indication TLV 0x10 for CAT2 SCWSOpenChannelIndication
struct sCAT2SCWSOpenChannelIndication_OpenChannelInformation
{
   UINT32 mChannelID;
   UINT16 mPortNumber;
   UINT16 mBufferSize;
};

// Structure to describe indication TLV 0x11 for CAT2 SCWSOpenChannelIndication
struct sCAT2SCWSOpenChannelIndication_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe request TLV 0x01 for CAT2SCWSCloseChannel()
struct sCAT2SCWSCloseChannelRequest_ChannelStatus
{
   UINT32 mChannelID;
   eQMICATChannelState mChannelState;
};

// Structure to describe request TLV 0x10 for CAT2SCWSCloseChannel()
struct sCAT2SCWSCloseChannelRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe indication TLV 0x10 for CAT2 SCWSCloseChannelIndication
struct sCAT2SCWSCloseChannelIndication_CloseChannelInfo
{
   UINT32 mChannelID;
   eQMICATChannelState mChannelState;
};

// Structure to describe response TLV 0x11 for CAT2SCWSCloseChannel()
struct sCAT2SCWSCloseChannelResponse_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe request TLV 0x01 for CAT2SCWSSendData()
struct sCAT2SCWSSendDataRequest_ChannelStatus
{
   UINT32 mChannelID;
   eQMICATSendDataResult mDataSendResult;
};

// Structure to describe request TLV 0x10 for CAT2SCWSSendData()
struct sCAT2SCWSSendDataRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe indication TLV 0x10 for CAT2 SCWSSendDataIndication
struct sCAT2SCWSSendDataIndication_SendDataInfo
{
   UINT32 mChannelID;
   UINT8 mTotalPackets;
   UINT8 mCurrentPacket;
   UINT16 mDataLength;

   // This array must be the size specified by mDataLength
   // UINT8 mData[1];
};

// Structure to describe indication TLV 0x11 for CAT2 SCWSSendDataIndication
struct sCAT2SCWSSendDataIndication_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe request TLV 0x01 for CAT2SCWSDataAvailable()
struct sCAT2SCWSDataAvailableRequest_RemainingData
{
   UINT32 mChannelID;
   UINT16 mDataLength;

   // This array must be the size specified by mDataLength
   // UINT8 mData[1];
};

// Structure to describe request TLV 0x10 for CAT2SCWSDataAvailable()
struct sCAT2SCWSDataAvailableRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe request TLV 0x01 for CAT2SCWSChannelStatus()
struct sCAT2SCWSChannelStatusRequest_ChannelStatus
{
   UINT32 mChannelID;
   eQMICATChannelState mChannelState;
};

// Structure to describe request TLV 0x10 for CAT2SCWSChannelStatus()
struct sCAT2SCWSChannelStatusRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe request TLV 0x10 for CAT2GetTerminalProfile()
struct sCAT2GetTerminalProfileRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe response TLV 0x10 for CAT2GetTerminalProfile()
struct sCAT2GetTerminalProfileResponse_RawData
{
   UINT8 mTerminalProfileLength;

   // This array must be the size specified by mTerminalProfileLength
   // UINT8 mTerminalProfileData[1];
};

// Structure to describe request TLV 0x01 for CAT2SetConfiguration()
struct sCAT2SetConfigurationRequest_Mode
{
   eQMICATConfigModes mConfigMode;
};

// Structure to describe request TLV 0x10 for CAT2SetConfiguration()
struct sCAT2SetConfigurationRequest_CustomData
{
   UINT8 mTerminalProfileLength;

   // This array must be the size specified by mTerminalProfileLength
   // UINT8 mTerminalProfileData[1];
};

// Structure to describe response TLV 0x10 for CAT2GetConfiguration()
struct sCAT2GetConfigurationResponse_Mode
{
   eQMICATConfigModes mConfigMode;
};

// Structure to describe response TLV 0x11 for CAT2GetConfiguration()
struct sCAT2GetConfigurationResponse_CustomData
{
   UINT8 mTerminalProfileLength;

   // This array must be the size specified by mTerminalProfileLength
   // UINT8 mTerminalProfileData[1];
};

// Structure to describe request TLV 0x01 for UIMReadTransparent()
struct sUIMReadTransparentRequest_SessionInfo
{
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe request TLV 0x03 for UIMReadTransparent()
struct sUIMReadTransparentRequest_Buffer
{
   UINT16 mOffset;
   UINT16 mLength;
};

// Structure to describe request TLV 0x10 for UIMReadTransparent()
struct sUIMReadTransparentRequest_ResponseInIndication
{
   UINT32 mIndicationToken;
};

// Structure to describe request TLV 0x11 for UIMReadTransparent()
struct sUIMReadTransparentRequest_Encryption
{
   INT8 mDataEncrypted;
};

// Structure to describe response TLV 0x10 for UIMReadTransparent()
struct sUIMReadTransparentResponse_CardResult
{
   UINT8 mSW1;
   UINT8 mSW2;
};

// Structure to describe response TLV 0x11 for UIMReadTransparent()
struct sUIMReadTransparentResponse_ReadResult
{
   UINT16 mContentLength;

   // This array must be the size specified by mContentLength
   // UINT8 mContent[1];
};

// Structure to describe response TLV 0x12 for UIMReadTransparent()
struct sUIMReadTransparentResponse_ResponseInIndication
{
   UINT32 mIndicationToken;
};

// Structure to describe response TLV 0x13 for UIMReadTransparent()
struct sUIMReadTransparentResponse_Encryption
{
   INT8 mDataEncrypted;
};

// Structure to describe response TLV 0x14 for UIMReadTransparent()
struct sUIMReadTransparentResponse_RequestedLength
{
   UINT16 mFileLength;
};

// Structure to describe indication TLV 0x01 for UIM ReadTransparentIndication
struct sUIMReadTransparentIndication_OriginalToken
{
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x10 for UIM ReadTransparentIndication
struct sUIMReadTransparentIndication_CardResult
{
   UINT8 mSW1;
   UINT8 mSW2;
};

// Structure to describe indication TLV 0x11 for UIM ReadTransparentIndication
struct sUIMReadTransparentIndication_ReadResult
{
   UINT16 mContentLength;

   // This array must be the size specified by mContentLength
   // UINT8 mContent[1];
};

// Structure to describe indication TLV 0x12 for UIM ReadTransparentIndication
struct sUIMReadTransparentIndication_Encryption
{
   INT8 mDataEncrypted;
};

// Structure to describe indication TLV 0x13 for UIM ReadTransparentIndication
struct sUIMReadTransparentIndication_RequestedLength
{
   UINT16 mFileLength;
};

// Structure to describe request TLV 0x01 for UIMReadRecord()
struct sUIMReadRecordRequest_SessionInfo
{
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe request TLV 0x03 for UIMReadRecord()
struct sUIMReadRecordRequest_Record
{
   UINT16 mRecordNumber;
   UINT16 mContentLength;
};

// Structure to describe request TLV 0x10 for UIMReadRecord()
struct sUIMReadRecordRequest_LastRecord
{
   UINT16 mRecordNumber;
};

// Structure to describe request TLV 0x11 for UIMReadRecord()
struct sUIMReadRecordRequest_ResponseInIndication
{
   UINT32 mIndicationToken;
};

// Structure to describe response TLV 0x10 for UIMReadRecord()
struct sUIMReadRecordResponse_CardResult
{
   UINT8 mSW1;
   UINT8 mSW2;
};

// Structure to describe response TLV 0x11 for UIMReadRecord()
struct sUIMReadRecordResponse_ReadResult
{
   UINT16 mContentLength;

   // This array must be the size specified by mContentLength
   // UINT8 mContent[1];
};

// Structure to describe response TLV 0x12 for UIMReadRecord()
struct sUIMReadRecordResponse_AdditionalReadResult
{
   UINT16 mContentLength;

   struct sRecord
   {
      // This array must be the size specified by mContentLength
      // UINT8 mContent[1];
   };

   // This array is variable length based on the size of the container
   // sRecord mRecords[1];
};

// Structure to describe response TLV 0x13 for UIMReadRecord()
struct sUIMReadRecordResponse_ResponseInIndication
{
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x01 for UIM ReadRecordIndication
struct sUIMReadRecordIndication_OriginalToken
{
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x10 for UIM ReadRecordIndication
struct sUIMReadRecordIndication_CardResult
{
   UINT8 mSW1;
   UINT8 mSW2;
};

// Structure to describe indication TLV 0x11 for UIM ReadRecordIndication
struct sUIMReadRecordIndication_ReadResult
{
   UINT16 mContentLength;

   // This array must be the size specified by mContentLength
   // UINT8 mContent[1];
};

// Structure to describe indication TLV 0x12 for UIM ReadRecordIndication
struct sUIMReadRecordIndication_AdditionalReadResult
{
   UINT16 mContentLength;

   struct sRecord
   {
      // This array must be the size specified by mContentLength
      // UINT8 mContent[1];
   };

   // This array is variable length based on the size of the container
   // sRecord mRecords[1];
};

// Structure to describe request TLV 0x01 for UIMWriteTransparent()
struct sUIMWriteTransparentRequest_SessionInfo
{
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe request TLV 0x03 for UIMWriteTransparent()
struct sUIMWriteTransparentRequest_Buffer
{
   UINT16 mOffset;
   UINT16 mContentLength;

   // This array must be the size specified by mContentLength
   // UINT8 mContent[1];
};

// Structure to describe request TLV 0x10 for UIMWriteTransparent()
struct sUIMWriteTransparentRequest_ResponseInIndicaiton
{
   UINT32 mIndicationToken;
};

// Structure to describe response TLV 0x10 for UIMWriteTransparent()
struct sUIMWriteTransparentResponse_CardResult
{
   UINT8 mSW1;
   UINT8 mSW2;
};

// Structure to describe response TLV 0x11 for UIMWriteTransparent()
struct sUIMWriteTransparentResponse_ResponseInIndication
{
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x01 for UIM WriteTransparentIndication
struct sUIMWriteTransparentIndication_OriginalToken
{
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x10 for UIM WriteTransparentIndication
struct sUIMWriteTransparentIndication_CardResult
{
   UINT8 mSW1;
   UINT8 mSW2;
};

// Structure to describe request TLV 0x01 for UIMWriteRecord()
struct sUIMWriteRecordRequest_SessionInfo
{
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe request TLV 0x03 for UIMWriteRecord()
struct sUIMWriteRecordRequest_Record
{
   UINT16 mRecordNumber;
   UINT16 mContentLength;

   // This array must be the size specified by mContentLength
   // UINT8 mContent[1];
};

// Structure to describe request TLV 0x10 for UIMWriteRecord()
struct sUIMWriteRecordRequest_ResponseInIndication
{
   UINT32 mIndicationToken;
};

// Structure to describe response TLV 0x10 for UIMWriteRecord()
struct sUIMWriteRecordResponse_CardResult
{
   UINT8 mSW1;
   UINT8 mSW2;
};

// Structure to describe response TLV 0x11 for UIMWriteRecord()
struct sUIMWriteRecordResponse_ResponseInIndication
{
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x01 for UIM WriteRecordIndication
struct sUIMWriteRecordIndication_OriginalToken
{
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x10 for UIM WriteRecordIndication
struct sUIMWriteRecordIndication_CardResult
{
   UINT8 mSW1;
   UINT8 mSW2;
};

// Structure to describe request TLV 0x01 for UIMGetFileAttributes()
struct sUIMGetFileAttributesRequest_SessionInfo
{
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe request TLV 0x10 for UIMGetFileAttributes()
struct sUIMGetFileAttributesRequest_ResponseInIndication
{
   UINT32 mIndicationToken;
};

// Structure to describe response TLV 0x10 for UIMGetFileAttributes()
struct sUIMGetFileAttributesResponse_CardResult
{
   UINT8 mSW1;
   UINT8 mSW2;
};

// Structure to describe response TLV 0x11 for UIMGetFileAttributes()
struct sUIMGetFileAttributesResponse_Attributes
{
   UINT16 mFileSize;
   UINT16 mFileID;
   eQMIUIMFileTypes mFileType;
   UINT16 mRecordSize;
   UINT16 mRecordCount;
   eQMIUIMSecurityAttributes mReadSecurityAttributes;
   bool mReadPIN1:1;
   bool mReadPIN2:1;
   bool mReadUPIN:1;
   bool mReadADM:1;

   // Padding out 4 bits
   UINT8 mReserved1:4;

   eQMIUIMSecurityAttributes mWriteSecurityAttributes;
   bool mWritePIN1:1;
   bool mWritePIN2:1;
   bool mWriteUPIN:1;
   bool mWriteADM:1;

   // Padding out 4 bits
   UINT8 mReserved2:4;

   eQMIUIMSecurityAttributes mIncreaseSecurityAttributes;
   bool mIncreasePIN1:1;
   bool mIncreasePIN2:1;
   bool mIncreaseUPIN:1;
   bool mIncreaseADM:1;

   // Padding out 4 bits
   UINT8 mReserved3:4;

   eQMIUIMSecurityAttributes mDeactivateSecurityAttributes;
   bool mDeactivatePIN1:1;
   bool mDeactivatePIN2:1;
   bool mDeactivateUPIN:1;
   bool mDeactivateADM:1;

   // Padding out 4 bits
   UINT8 mReserved4:4;

   eQMIUIMSecurityAttributes mActivateSecurityAttributes;
   bool mActivatePIN1:1;
   bool mActivatePIN2:1;
   bool mActivateUPIN:1;
   bool mActivateADM:1;

   // Padding out 4 bits
   UINT8 mReserved5:4;

   UINT16 mValueLength;

   // This array must be the size specified by mValueLength
   // UINT8 mValue[1];
};

// Structure to describe response TLV 0x12 for UIMGetFileAttributes()
struct sUIMGetFileAttributesResponse_ResponseInIndication
{
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x01 for UIM GetFileAttributesIndication
struct sUIMGetFileAttributesIndication_OriginalToken
{
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x10 for UIM GetFileAttributesIndication
struct sUIMGetFileAttributesIndication_CardResult
{
   UINT8 mSW1;
   UINT8 mSW2;
};

// Structure to describe indication TLV 0x11 for UIM GetFileAttributesIndication
struct sUIMGetFileAttributesIndication_FileAttributes
{
   UINT16 mFileSize;
   UINT16 mFileID;
   eQMIUIMFileTypes mFileType;
   UINT16 mRecordSize;
   UINT16 mRecordCount;
   eQMIUIMSecurityAttributes mReadSecurityAttributes;
   bool mReadPIN1:1;
   bool mReadPIN2:1;
   bool mReadUPIN:1;
   bool mReadADM:1;

   // Padding out 4 bits
   UINT8 mReserved1:4;

   eQMIUIMSecurityAttributes mWriteSecurityAttributes;
   bool mWritePIN1:1;
   bool mWritePIN2:1;
   bool mWriteUPIN:1;
   bool mWriteADM:1;

   // Padding out 4 bits
   UINT8 mReserved2:4;

   eQMIUIMSecurityAttributes mIncreaseSecurityAttributes;
   bool mIncreasePIN1:1;
   bool mIncreasePIN2:1;
   bool mIncreaseUPIN:1;
   bool mIncreaseADM:1;

   // Padding out 4 bits
   UINT8 mReserved3:4;

   eQMIUIMSecurityAttributes mDeactivateSecurityAttributes;
   bool mDeactivatePIN1:1;
   bool mDeactivatePIN2:1;
   bool mDeactivateUPIN:1;
   bool mDeactivateADM:1;

   // Padding out 4 bits
   UINT8 mReserved4:4;

   eQMIUIMSecurityAttributes mActivateSecurityAttributes;
   bool mActivatePIN1:1;
   bool mActivatePIN2:1;
   bool mActivateUPIN:1;
   bool mActivateADM:1;

   // Padding out 4 bits
   UINT8 mReserved5:4;

   UINT16 mValueLength;

   // This array must be the size specified by mValueLength
   // UINT8 mValue[1];
};

// Structure to describe request TLV 0x01 for UIMSetPINProtection()
struct sUIMSetPINProtectionRequest_SessionInfo
{
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe request TLV 0x10 for UIMSetPINProtection()
struct sUIMSetPINProtectionRequest_KeyReferenceID
{
   eQMIUIMKeyReferenceID mKeyReferenceID;
};

// Structure to describe request TLV 0x11 for UIMSetPINProtection()
struct sUIMSetPINProtectionRequest_ResponseInIndication
{
   UINT32 mIndicationToken;
};

// Structure to describe response TLV 0x10 for UIMSetPINProtection()
struct sUIMSetPINProtectionResponse_Retries
{
   UINT8 mRemainingVerifyRetries;
   UINT8 mRemainingUnblockRetries;
};

// Structure to describe response TLV 0x11 for UIMSetPINProtection()
struct sUIMSetPINProtectionResponse_EncryptedPIN1
{
   UINT8 mPINLength;

   // This array must be the size specified by mPINLength
   // UINT8 mValue[1];
};

// Structure to describe response TLV 0x12 for UIMSetPINProtection()
struct sUIMSetPINProtectionResponse_ResponseInIndication
{
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x01 for UIM SetPINProtectionIndication
struct sUIMSetPINProtectionIndication_OriginalToken
{
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x10 for UIM SetPINProtectionIndication
struct sUIMSetPINProtectionIndication_Retries
{
   UINT8 mRemainingVerifyRetries;
   UINT8 mRemainingUnblockRetries;
};

// Structure to describe indication TLV 0x11 for UIM SetPINProtectionIndication
struct sUIMSetPINProtectionIndication_EncryptedPIN1
{
   UINT8 mPINLength;

   // This array must be the size specified by mPINLength
   // UINT8 mValue[1];
};

// Structure to describe request TLV 0x01 for UIMVerifyPIN()
struct sUIMVerifyPINRequest_SessionInfo
{
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe request TLV 0x10 for UIMVerifyPIN()
struct sUIMVerifyPINRequest_EncryptedPIN1
{
   UINT8 mPINLength;

   // This array must be the size specified by mPINLength
   // UINT8 mValue[1];
};

// Structure to describe request TLV 0x11 for UIMVerifyPIN()
struct sUIMVerifyPINRequest_KeyReferenceID
{
   eQMIUIMKeyReferenceID mKeyReferenceID;
};

// Structure to describe request TLV 0x12 for UIMVerifyPIN()
struct sUIMVerifyPINRequest_ResponseInIndication
{
   UINT32 mIndicationToken;
};

// Structure to describe response TLV 0x10 for UIMVerifyPIN()
struct sUIMVerifyPINResponse_Retries
{
   UINT8 mRemainingVerifyRetries;
   UINT8 mRemainingUnblockRetries;
};

// Structure to describe response TLV 0x11 for UIMVerifyPIN()
struct sUIMVerifyPINResponse_EncryptedPIN1
{
   UINT8 mPINLength;

   // This array must be the size specified by mPINLength
   // UINT8 mValue[1];
};

// Structure to describe response TLV 0x12 for UIMVerifyPIN()
struct sUIMVerifyPINResponse_ResponseInIndication
{
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x01 for UIM VerifyPINIndication
struct sUIMVerifyPINIndication_OriginalToken
{
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x10 for UIM VerifyPINIndication
struct sUIMVerifyPINIndication_Retries
{
   UINT8 mRemainingVerifyRetries;
   UINT8 mRemainingUnblockRetries;
};

// Structure to describe indication TLV 0x11 for UIM VerifyPINIndication
struct sUIMVerifyPINIndication_EncryptedPIN1
{
   UINT8 mPINLength;

   // This array must be the size specified by mPINLength
   // UINT8 mValue[1];
};

// Structure to describe request TLV 0x01 for UIMUnblockPIN()
struct sUIMUnblockPINRequest_SessionInfo
{
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe request TLV 0x10 for UIMUnblockPIN()
struct sUIMUnblockPINRequest_KeyReferenceID
{
   eQMIUIMKeyReferenceID mKeyReferenceID;
};

// Structure to describe request TLV 0x11 for UIMUnblockPIN()
struct sUIMUnblockPINRequest_ResponseInIndication
{
   UINT32 mIndicationToken;
};

// Structure to describe response TLV 0x10 for UIMUnblockPIN()
struct sUIMUnblockPINResponse_Retries
{
   UINT8 mRemainingVerifyRetries;
   UINT8 mRemainingUnblockRetries;
};

// Structure to describe response TLV 0x11 for UIMUnblockPIN()
struct sUIMUnblockPINResponse_EncryptedPIN1
{
   UINT8 mPINLength;

   // This array must be the size specified by mPINLength
   // UINT8 mValue[1];
};

// Structure to describe response TLV 0x12 for UIMUnblockPIN()
struct sUIMUnblockPINResponse_ResponseInIndication
{
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x01 for UIM UnblockPINIndication
struct sUIMUnblockPINIndication_OriginalToken
{
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x10 for UIM UnblockPINIndication
struct sUIMUnblockPINIndication_Retries
{
   UINT8 mRemainingVerifyRetries;
   UINT8 mRemainingUnblockRetries;
};

// Structure to describe indication TLV 0x11 for UIM UnblockPINIndication
struct sUIMUnblockPINIndication_EncryptedPIN1
{
   UINT8 mPINLength;

   // This array must be the size specified by mPINLength
   // UINT8 mValue[1];
};

// Structure to describe request TLV 0x01 for UIMChangePIN()
struct sUIMChangePINRequest_SessionInfo
{
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe request TLV 0x10 for UIMChangePIN()
struct sUIMChangePINRequest_KeyReferenceID
{
   eQMIUIMKeyReferenceID mKeyReferenceID;
};

// Structure to describe request TLV 0x11 for UIMChangePIN()
struct sUIMChangePINRequest_ResponseInIndication
{
   UINT32 mIndicationToken;
};

// Structure to describe response TLV 0x10 for UIMChangePIN()
struct sUIMChangePINResponse_Retries
{
   UINT8 mRemainingVerifyRetries;
   UINT8 mRemainingUnblockRetries;
};

// Structure to describe response TLV 0x11 for UIMChangePIN()
struct sUIMChangePINResponse_EncryptedPIN1
{
   UINT8 mPINLength;

   // This array must be the size specified by mPINLength
   // UINT8 mValue[1];
};

// Structure to describe response TLV 0x12 for UIMChangePIN()
struct sUIMChangePINResponse_ResponseInIndication
{
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x01 for UIM ChangePINIndication
struct sUIMChangePINIndication_OriginalToken
{
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x10 for UIM ChangePINIndication
struct sUIMChangePINIndication_Retries
{
   UINT8 mRemainingVerifyRetries;
   UINT8 mRemainingUnblockRetries;
};

// Structure to describe indication TLV 0x11 for UIM ChangePINIndication
struct sUIMChangePINIndication_EncryptedPIN1
{
   UINT8 mPINLength;

   // This array must be the size specified by mPINLength
   // UINT8 mValue[1];
};

// Structure to describe request TLV 0x01 for UIMDepersonalization()
struct sUIMDepersonalizationRequest_Info
{
   eQMIUIMPersonalizationFeatures mFeature;
   eQMIUIMCKSessionOperations mOperation;
   UINT8 mCKLength;

   // This array must be the size specified by mCKLength
   // char mCKValue[1];
};

// Structure to describe response TLV 0x10 for UIMDepersonalization()
struct sUIMDepersonalizationResponse_Retries
{
   UINT8 mRemainingVerifyRetries;
   UINT8 mRemainingUnblockRetries;
};

// Structure to describe request TLV 0x01 for UIMRefreshRegister()
struct sUIMRefreshRegisterRequest_SessionInfo
{
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe request TLV 0x01 for UIMRefreshOK()
struct sUIMRefreshOKRequest_SessionInfo
{
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe request TLV 0x01 for UIMRefreshComplete()
struct sUIMRefreshCompleteRequest_SessionInfo
{
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe request TLV 0x01 for UIMGetLastRefreshEvent()
struct sUIMGetLastRefreshEventRequest_SessionInfo
{
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe response TLV 0x10 for UIMGetLastRefreshEvent()
struct sUIMGetLastRefreshEventResponse_RefreshEvent1
{
   eQMIUIMRefreshStages mStage;
   eQMIUIMRefreshModes mMode;
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

struct sUIMGetLastRefreshEventResponse_RefreshEvent2
{
   UINT16 mFileCount;

   struct sFile
   {
      UINT16 mFileID;
      UINT8 mPathLength;
   
      // This array must be the size specified by mPathLength
      // UINT16 mPath[1];
   };

   // This array must be the size specified by mFileCount
   // sFile mFiles[1];
};

struct sUIMGetLastRefreshEventResponse_RefreshEvent
{
   sUIMGetLastRefreshEventResponse_RefreshEvent1 mUIMGetLastRefreshEventResponse_RefreshEvent1;
   sUIMGetLastRefreshEventResponse_RefreshEvent2 mUIMGetLastRefreshEventResponse_RefreshEvent2;
};

// Structure to describe request TLV 0x01 for UIMEventRegistration()
struct sUIMEventRegistrationRequest_Mask
{
   bool mCardStatus:1;
   bool mSAPConnection:1;

   // Padding out 30 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[3];
};

// Structure to describe response TLV 0x10 for UIMEventRegistration()
struct sUIMEventRegistrationResponse_Mask
{
   bool mCardStatus:1;
   bool mSAPConnection:1;

   // Padding out 30 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[3];
};

// Structure to describe response TLV 0x10 for UIMGetCardStatus()
struct sUIMGetCardStatusResponse_Status1
{
   UINT8 mPrimaryGWSlot;
   UINT8 mPrimaryGWApplication;
   UINT8 mPrimary1XSlot;
   UINT8 mPrimary1XApplication;
   UINT8 mSecondaryGWSlot;
   UINT8 mSecondaryGWApplication;
   UINT8 mSecondary1XSlot;
   UINT8 mSecondary1XApplication;
   UINT8 mSlotsAvailable;
   eQMIUIMCardStates mCardState;
   eQMIUIMPINStates mUPINState;
   UINT8 mRemainingUPINVerifyRetries;
   UINT8 mRemainingUPINUnblockRetries;
   eQMIUIMCardErrorCodes mCardErrorCode;
   UINT8 mApplicationsAvailable;
   eQMIUIMApplicationTypes mApplicationType;
   eQMIUIMApplicationStates mApplicationState;
   eQMIUIMPersonalizationStates mPersonalizationState;
   eQMIUIMPersonalizationFeatures mPersonalizationFeature;
   UINT8 mRemainingPersonalizationVerifyRetries;
   UINT8 mRemainingPersonalizationUnblockRetries;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

struct sUIMGetCardStatusResponse_Status2
{
   INT8 mUPINReplacesPIN1;
   eQMIUIMPINStates mPIN1State;
   UINT8 mRemainingPIN1VerifyRetries;
   UINT8 mRemainingPIN1UnblockRetries;
   eQMIUIMPINStates mPIN2State;
   UINT8 mRemainingPIN2VerifyRetries;
   UINT8 mRemainingPIN2UnblockRetries;
};

struct sUIMGetCardStatusResponse_Status
{
   sUIMGetCardStatusResponse_Status1 mUIMGetCardStatusResponse_Status1;
   sUIMGetCardStatusResponse_Status2 mUIMGetCardStatusResponse_Status2;
};

// Structure to describe response TLV 0x11 for UIMGetCardStatus()
struct sUIMGetCardStatusResponse_HotSwapStatus
{
   UINT8 mHotSwapLength;

   // This array must be the size specified by mHotSwapLength
   // eQMIUIMHotSwap mHotSwap[1];
};

// Structure to describe response TLV 0x12 for UIMGetCardStatus()
struct sUIMGetCardStatusResponse_ValidCardStatus
{
   UINT8 mCardStatusValidCount;

   // This array must be the size specified by mCardStatusValidCount
   // INT8 mCardStatusValid[1];
};

// Structure to describe request TLV 0x01 for UIMPowerDown()
struct sUIMPowerDownRequest_Slot
{
   eQMIUIMSlots mSlot;
};

// Structure to describe request TLV 0x01 for UIMPowerUp()
struct sUIMPowerUpRequest_Slot
{
   eQMIUIMSlots mSlot;
};

// Structure to describe request TLV 0x10 for UIMPowerUp()
struct sUIMPowerUpRequest_IgnoreHotSwapSwitch
{
   INT8 mIgnoreHotSwapSwitch;
};

// Structure to describe indication TLV 0x10 for UIM CardStatusIndication
struct sUIMCardStatusIndication_Status1
{
   UINT8 mPrimaryGWSlot;
   UINT8 mPrimaryGWApplication;
   UINT8 mPrimary1XSlot;
   UINT8 mPrimary1XApplication;
   UINT8 mSecondaryGWSlot;
   UINT8 mSecondaryGWApplication;
   UINT8 mSecondary1XSlot;
   UINT8 mSecondary1XApplication;
   UINT8 mSlotsAvailable;
   eQMIUIMCardStates mCardState;
   eQMIUIMPINStates mUPINState;
   UINT8 mRemainingUPINVerifyRetries;
   UINT8 mRemainingUPINUnblockRetries;
   eQMIUIMCardErrorCodes mCardErrorCode;
   UINT8 mApplicationsAvailable;
   eQMIUIMApplicationTypes mApplicationType;
   eQMIUIMApplicationStates mApplicationState;
   eQMIUIMPersonalizationStates mPersonalizationState;
   eQMIUIMPersonalizationFeatures mPersonalizationFeature;
   UINT8 mRemainingPersonalizationVerifyRetries;
   UINT8 mRemainingPersonalizationUnblockRetries;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

struct sUIMCardStatusIndication_Status2
{
   INT8 mUPINReplacesPIN1;
   eQMIUIMPINStates mPIN1State;
   UINT8 mRemainingPIN1VerifyRetries;
   UINT8 mRemainingPIN1UnblockRetries;
   eQMIUIMPINStates mPIN2State;
   UINT8 mRemainingPIN2VerifyRetries;
   UINT8 mRemainingPIN2UnblockRetries;
};

struct sUIMCardStatusIndication_Status
{
   sUIMCardStatusIndication_Status1 mUIMCardStatusIndication_Status1;
   sUIMCardStatusIndication_Status2 mUIMCardStatusIndication_Status2;
};

// Structure to describe indication TLV 0x10 for UIM RefreshIndication
struct sUIMRefreshIndication_RefreshEvent1
{
   eQMIUIMRefreshStages mStage;
   eQMIUIMRefreshModes mMode;
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

struct sUIMRefreshIndication_RefreshEvent2
{
   UINT16 mFileCount;

   struct sFile
   {
      UINT16 mFileID;
      UINT8 mPathLength;
   
      // This array must be the size specified by mPathLength
      // UINT16 mPath[1];
   };

   // This array must be the size specified by mFileCount
   // sFile mFiles[1];
};

struct sUIMRefreshIndication_RefreshEvent
{
   sUIMRefreshIndication_RefreshEvent1 mUIMRefreshIndication_RefreshEvent1;
   sUIMRefreshIndication_RefreshEvent2 mUIMRefreshIndication_RefreshEvent2;
};

// Structure to describe request TLV 0x01 for UIMAuthenticate()
struct sUIMAuthenticateRequest_SessionInfo
{
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe request TLV 0x10 for UIMAuthenticate()
struct sUIMAuthenticateRequest_ResponseInIndication
{
   UINT32 mIndicationToken;
};

// Structure to describe response TLV 0x10 for UIMAuthenticate()
struct sUIMAuthenticateResponse_CardResult
{
   UINT8 mSW1;
   UINT8 mSW2;
};

// Structure to describe response TLV 0x11 for UIMAuthenticate()
struct sUIMAuthenticateResponse_Data
{
   UINT16 mContentLength;

   // This array must be the size specified by mContentLength
   // UINT8 mContent[1];
};

// Structure to describe response TLV 0x12 for UIMAuthenticate()
struct sUIMAuthenticateResponse_ResponseInIndication
{
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x01 for UIM AuthenticateIndication
struct sUIMAuthenticateIndication_OriginalToken
{
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x10 for UIM AuthenticateIndication
struct sUIMAuthenticateIndication_CardResult
{
   UINT8 mSW1;
   UINT8 mSW2;
};

// Structure to describe indication TLV 0x11 for UIM AuthenticateIndication
struct sUIMAuthenticateIndication_Data
{
   UINT16 mContentLength;

   // This array must be the size specified by mContentLength
   // UINT8 mContent[1];
};

// Structure to describe request TLV 0x01 for UIMCloseSession()
struct sUIMCloseSessionRequest_SessionInfo
{
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe request TLV 0x01 for UIMGetServiceStatus()
struct sUIMGetServiceStatusRequest_SessionInfo
{
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe response TLV 0x10 for UIMGetServiceStatus()
struct sUIMGetServiceStatusResponse_FDNStatus
{
   eQMIUIMFDNStatusValues mFDNStatus;
};

// Structure to describe response TLV 0x11 for UIMGetServiceStatus()
struct sUIMGetServiceStatusResponse_HiddenKeyStatus
{
   eQMIUIMHiddenKeyStatusValues mHiddenKeyStatus;
};

// Structure to describe response TLV 0x12 for UIMGetServiceStatus()
struct sUIMGetServiceStatusResponse_Index
{
   UINT8 mEFDIRIndex;
};

// Structure to describe response TLV 0x13 for UIMGetServiceStatus()
struct sUIMGetServiceStatusResponse_ESNStatus
{
   INT8 mESNChanged;
};

// Structure to describe request TLV 0x01 for UIMSetServiceStatus()
struct sUIMSetServiceStatusRequest_SessionInfo
{
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe request TLV 0x10 for UIMSetServiceStatus()
struct sUIMSetServiceStatusRequest_FDNStatus
{
   INT8 mEnableFDN;
};

// Structure to describe request TLV 0x01 for UIMChangeProvisioningSession()
struct sUIMChangeProvisioningSessionRequest_SessionChange
{
   eQMIUIMSessionTypes mSessionType;
   eQMIUIMCKSessionOperations mOperation;
};

// Structure to describe request TLV 0x10 for UIMChangeProvisioningSession()
struct sUIMChangeProvisioningSessionRequest_AppInfo
{
   eQMIUIMSlots mSlot;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe request TLV 0x01 for UIMGetLabel()
struct sUIMGetLabelRequest_AppInfo
{
   eQMIUIMSlots mSlot;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe response TLV 0x10 for UIMGetLabel()
struct sUIMGetLabelResponse_AppLabel
{
   UINT8 mLabelLength;

   // This array must be the size specified by mLabelLength
   // char mLabelValue[1];
};

// Structure to describe request TLV 0x10 for UIMGetConfiguration()
struct sUIMGetConfigurationRequest_Mask
{
   bool mAutomaticSelection:1;
   bool mPersonalizationStatus:1;
   bool mHaltSubscription:1;

   // Padding out 29 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2[3];
};

// Structure to describe response TLV 0x10 for UIMGetConfiguration()
struct sUIMGetConfigurationResponse_AutoSelection
{
   INT8 mAutomaticProvisioningOn;
};

// Structure to describe response TLV 0x11 for UIMGetConfiguration()
struct sUIMGetConfigurationResponse_Personalization
{
   UINT8 mFeatureCount;

   struct sFeature
   {
      eQMIUIMPersonalizationFeatures mPersonalizationFeature;
      UINT8 mRemainingVerifyRetries;
      UINT8 mRemainingUnblockRetries;
   };

   // This array must be the size specified by mFeatureCount
   // sFeature mFeatures[1];
};

// Structure to describe response TLV 0x12 for UIMGetConfiguration()
struct sUIMGetConfigurationResponse_Subscription
{
   INT8 mPublishSubscription;
};

// Structure to describe request TLV 0x01 for UIMSendADPU()
struct sUIMSendADPURequest_Slot
{
   eQMIUIMSlots mSlot;
};

// Structure to describe request TLV 0x10 for UIMSendADPU()
struct sUIMSendADPURequest_LogicalChannel
{
   UINT8 mChannelID;
};

// Structure to describe request TLV 0x11 for UIMSendADPU()
struct sUIMSendADPURequest_IntermediateProcedureBytes
{
   eQMIUIMAPDUResponseStatus mIntermediateProcedureBytes;
};

// Structure to describe response TLV 0x10 for UIMSendADPU()
struct sUIMSendADPUResponse_APDUResponse
{
   UINT16 mDataLength;

   // This array must be the size specified by mDataLength
   // UINT8 mData[1];
};

// Structure to describe response TLV 0x11 for UIMSendADPU()
struct sUIMSendADPUResponse_LongAPDUResponse
{
   UINT16 mTotalLength;
   UINT32 mIndicationToken;
};

// Structure to describe indication TLV 0x01 for UIM SendADPUIndication
struct sUIMSendADPUIndication_APDUResponseChunk
{
   UINT32 mIndicationToken;
   UINT16 mTotalLength;
   UINT16 mOffset;
   UINT16 mDataLength;

   // This array must be the size specified by mDataLength
   // UINT8 mData[1];
};

// Structure to describe request TLV 0x01 for UIMSAPConnection()
struct sUIMSAPConnectionRequest_Slot
{
   eQMIUIMConnectOperations mOperation;
   eQMIUIMSlots mSlot;
};

// Structure to describe request TLV 0x10 for UIMSAPConnection()
struct sUIMSAPConnectionRequest_DisconnectMode
{
   eQMIUIMDisonnectModes mDisconnectMode;
};

// Structure to describe request TLV 0x11 for UIMSAPConnection()
struct sUIMSAPConnectionRequest_IntermediateGetResponse
{
   INT8 mReturnIntermediateGetResponse;
};

// Structure to describe response TLV 0x10 for UIMSAPConnection()
struct sUIMSAPConnectionResponse_State
{
   eQMIUIMSAPStates mSAPState;
};

// Structure to describe request TLV 0x01 for UIMSAP()
struct sUIMSAPRequestRequest_Request
{
   eQMIUIMSAPRequests mSAPRequest;
   eQMIUIMSlots mSlot;
};

// Structure to describe request TLV 0x10 for UIMSAP()
struct sUIMSAPRequestRequest_APDU
{
   UINT16 mDataLength;

   // This array must be the size specified by mDataLength
   // UINT8 mData[1];
};

// Structure to describe response TLV 0x10 for UIMSAPRequest()
struct sUIMSAPRequestResponse_ATR
{
   UINT8 mDataLength;

   // This array must be the size specified by mDataLength
   // UINT8 mData[1];
};

// Structure to describe response TLV 0x11 for UIMSAPRequest()
struct sUIMSAPRequestResponse_APDU
{
   UINT16 mDataLength;

   // This array must be the size specified by mDataLength
   // UINT8 mData[1];
};

// Structure to describe response TLV 0x12 for UIMSAPRequest()
struct sUIMSAPRequestResponse_CardReaderStatus
{
   UINT8 mDataLength;

   // This array must be the size specified by mDataLength
   // UINT8 mData[1];
};

// Structure to describe indication TLV 0x10 for UIM SAPConnectionIndication
struct sUIMSAPConnectionIndication_CardStatus
{
   eQMIUIMSAPStates mSAPState;
   eQMIUIMSlots mSlot;
};

// Structure to describe request TLV 0x01 for UIMLogicalChannel()
struct sUIMLogicalChannelRequest_Slot
{
   eQMIUIMSlots mSlot;
};

// Structure to describe request TLV 0x10 for UIMLogicalChannel()
struct sUIMLogicalChannelRequest_AID
{
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe request TLV 0x11 for UIMLogicalChannel()
struct sUIMLogicalChannelRequest_LogicalChannel
{
   UINT8 mChannelID;
};

// Structure to describe request TLV 0x12 for UIMLogicalChannel()
struct sUIMLogicalChannelRequest_FileControlInformation
{
   eQMIUIMFileControlInformation mFileControlInformation;
};

// Structure to describe response TLV 0x10 for UIMLogicalChannel()
struct sUIMLogicalChannelResponse_LogicalChannel
{
   UINT8 mChannelID;
};

// Structure to describe response TLV 0x11 for UIMLogicalChannel()
struct sUIMLogicalChannelResponse_CardResult
{
   UINT8 mSW1;
   UINT8 mSW2;
};

// Structure to describe response TLV 0x12 for UIMLogicalChannel()
struct sUIMLogicalChannelResponse_ResponseToSelectCommand
{
   UINT8 mResponseLength;

   // This array must be the size specified by mResponseLength
   // UINT8 mResponse[1];
};

// Structure to describe request TLV 0x01 for UIMSubscriptionOK()
struct sUIMSubscriptionOKRequest_SessionInfo
{
   eQMIUIMSessionTypes mSessionType;
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe request TLV 0x10 for UIMSubscriptionOK()
struct sUIMSubscriptionOKRequest_EncryptedIMSI
{
   UINT8 mEncryptedIMSILength;

   // This array must be the size specified by mEncryptedIMSILength
   // UINT8 mEncryptedIMSI[1];
};

// Structure to describe request TLV 0x01 for UIMGetATR()
struct sUIMGetATRRequest_Slot
{
   eQMIUIMSlots mSlot;
};

// Structure to describe response TLV 0x10 for UIMGetATR()
struct sUIMGetATRResponse_ATRValue
{
   UINT8 mATRValueLength;

   // This array must be the size specified by mATRValueLength
   // UINT8 mATRValue[1];
};

// Structure to describe request TLV 0x01 for UIMOpenLogicalChannel()
struct sUIMOpenLogicalChannelRequest_Slot
{
   eQMIUIMSlots mSlot;
};

// Structure to describe request TLV 0x10 for UIMOpenLogicalChannel()
struct sUIMOpenLogicalChannelRequest_AID
{
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe request TLV 0x11 for UIMOpenLogicalChannel()
struct sUIMOpenLogicalChannelRequest_FileControlInformation
{
   eQMIUIMFileControlInformation mFileControlInformation;
};

// Structure to describe response TLV 0x10 for UIMOpenLogicalChannel()
struct sUIMOpenLogicalChannelResponse_LogicalChannel
{
   UINT8 mChannelID;
};

// Structure to describe response TLV 0x11 for UIMOpenLogicalChannel()
struct sUIMOpenLogicalChannelResponse_CardResult
{
   UINT8 mSW1;
   UINT8 mSW2;
};

// Structure to describe response TLV 0x12 for UIMOpenLogicalChannel()
struct sUIMOpenLogicalChannelResponse_ResponseToSelectCommand
{
   UINT8 mResponseLength;

   // This array must be the size specified by mResponseLength
   // UINT8 mResponse[1];
};

// Structure to describe indication TLV 0x01 for UIM SessionClosedIndication
struct sUIMSessionClosedIndication_Slot
{
   eQMIUIMSlots mSlot;
};

// Structure to describe indication TLV 0x10 for UIM SessionClosedIndication
struct sUIMSessionClosedIndication_AID
{
   UINT8 mAIDLength;

   // This array must be the size specified by mAIDLength
   // UINT8 mAID[1];
};

// Structure to describe indication TLV 0x11 for UIM SessionClosedIndication
struct sUIMSessionClosedIndication_LogicalChannel
{
   UINT8 mChannelID;
};

// Structure to describe request TLV 0x01 for PBMSetIndicationRegistrationState()
struct sPBMSetIndicationRegistrationStateRequest_Mask
{
   bool mRecordUpdate:1;
   bool mPhonebookReady:1;
   bool mEmergencyNumberList:1;
   bool mHiddenRecordStatus:1;
   bool mAASUpdate:1;
   bool mGASUpdate:1;

   // Padding out 26 bits
   UINT8 mReserved1:2;
   UINT8 mReserved2[3];
};

// Structure to describe response TLV 0x10 for PBMSetIndicationRegistrationState()
struct sPBMSetIndicationRegistrationStateResponse_Mask
{
   bool mRecordUpdate:1;
   bool mPhonebookReady:1;
   bool mEmergencyNumberList:1;
   bool mHiddenRecordStatus:1;
   bool mAASUpdate:1;
   bool mGASUpdate:1;

   // Padding out 26 bits
   UINT8 mReserved1:2;
   UINT8 mReserved2[3];
};

// Structure to describe request TLV 0x01 for PBMGetCapabilities()
struct sPBMGetCapabilitiesRequest_Info
{
   eQMIPBMSessionTypes mSessionType;
   eQMIPBMPhonebookTypes mPhonebookType;
};

// Structure to describe response TLV 0x10 for PBMGetCapabilities()
struct sPBMGetCapabilitiesResponse_Basic
{
   eQMIPBMSessionTypes mSessionType;
   eQMIPBMPhonebookTypes mPhonebookType;
   UINT16 mRecordsUsed;
   UINT16 mMaximumRecords;
   UINT8 mMaximumNumberLength;
   UINT8 mMaximumNameLength;
};

// Structure to describe response TLV 0x11 for PBMGetCapabilities()
struct sPBMGetCapabilitiesResponse_Group
{
   UINT8 mMaximumGroupsPossible;
   UINT8 mMaximumGroupTagLength;
};

// Structure to describe response TLV 0x12 for PBMGetCapabilities()
struct sPBMGetCapabilitiesResponse_AdditionalNumber
{
   UINT8 mMaximumAdditionalNumbersPossible;
   UINT8 mMaximumAdditionalNumberLength;
   UINT8 mMaximumAdditionalNumberTagLength;
};

// Structure to describe response TLV 0x13 for PBMGetCapabilities()
struct sPBMGetCapabilitiesResponse_Email
{
   UINT8 mMaximumEmailsPossible;
   UINT8 mMaximumEmailAddressLength;
};

// Structure to describe response TLV 0x14 for PBMGetCapabilities()
struct sPBMGetCapabilitiesResponse_SecondName
{
   UINT8 mMaximumSecondNameLength;
};

// Structure to describe response TLV 0x15 for PBMGetCapabilities()
struct sPBMGetCapabilitiesResponse_HiddenRecords
{
   INT8 mHiddenEntrySupported;
};

// Structure to describe response TLV 0x16 for PBMGetCapabilities()
struct sPBMGetCapabilitiesResponse_GAS
{
   UINT8 mMaximumGASStringLength;
};

// Structure to describe response TLV 0x17 for PBMGetCapabilities()
struct sPBMGetCapabilitiesResponse_AAS
{
   UINT8 mMaximumAASStringLength;
};

// Structure to describe response TLV 0x18 for PBMGetCapabilities()
struct sPBMGetCapabilitiesResponse_Protection
{
   eQMIPBMProtectionMethods mProtectionMethod;
};

// Structure to describe response TLV 0x10 for PBMGetAllCapabilities()
struct sPBMGetAllCapabilitiesResponse_Basic
{
   UINT8 mNumberOfSessions;

   struct sSession
   {
      eQMIPBMSessionTypes mSessionType;
      UINT8 mNumberOfPhonebooks;
   
      struct sPhonebook
      {
         eQMIPBMPhonebookTypes mPhonebookType;
         UINT16 mRecordsUsed;
         UINT16 mMaximumRecords;
         UINT8 mMaximumNumberLength;
         UINT8 mMaximumNameLength;
      };
   
      // This array must be the size specified by mNumberOfPhonebooks
      // sPhonebook mPhonebooks[1];
   };

   // This array must be the size specified by mNumberOfSessions
   // sSession mSessions[1];
};

// Structure to describe response TLV 0x11 for PBMGetAllCapabilities()
struct sPBMGetAllCapabilitiesResponse_Group
{
   UINT8 mNumberOfSessions;

   struct sSession
   {
      eQMIPBMSessionTypes mSessionType;
      UINT8 mMaximumGroupsPossible;
      UINT8 mMaximumGroupTagLength;
   };

   // This array must be the size specified by mNumberOfSessions
   // sSession mSessions[1];
};

// Structure to describe response TLV 0x12 for PBMGetAllCapabilities()
struct sPBMGetAllCapabilitiesResponse_AdditionalNumber
{
   UINT8 mNumberOfSessions;

   struct sSession
   {
      eQMIPBMSessionTypes mSessionType;
      UINT8 mMaximumAdditionalNumbersPossible;
      UINT8 mMaximumAdditionalNumberLength;
      UINT8 mMaximumAdditionalNumberTagLength;
   };

   // This array must be the size specified by mNumberOfSessions
   // sSession mSessions[1];
};

// Structure to describe response TLV 0x13 for PBMGetAllCapabilities()
struct sPBMGetAllCapabilitiesResponse_Email
{
   UINT8 mNumberOfSessions;

   struct sSession
   {
      eQMIPBMSessionTypes mSessionType;
      UINT8 mMaximumEmailsPossible;
      UINT8 mMaximumEmailAddressLength;
   };

   // This array must be the size specified by mNumberOfSessions
   // sSession mSessions[1];
};

// Structure to describe response TLV 0x14 for PBMGetAllCapabilities()
struct sPBMGetAllCapabilitiesResponse_SecondName
{
   UINT8 mNumberOfSessions;

   struct sSession
   {
      eQMIPBMSessionTypes mSessionType;
      UINT8 mMaximumSecondNameLength;
   };

   // This array must be the size specified by mNumberOfSessions
   // sSession mSessions[1];
};

// Structure to describe response TLV 0x15 for PBMGetAllCapabilities()
struct sPBMGetAllCapabilitiesResponse_HiddenRecords
{
   UINT8 mNumberOfSessions;

   struct sSession
   {
      eQMIPBMSessionTypes mSessionType;
      INT8 mHiddenEntrySupported;
   };

   // This array must be the size specified by mNumberOfSessions
   // sSession mSessions[1];
};

// Structure to describe response TLV 0x16 for PBMGetAllCapabilities()
struct sPBMGetAllCapabilitiesResponse_GAS
{
   UINT8 mNumberOfSessions;

   struct sSession
   {
      eQMIPBMSessionTypes mSessionType;
      UINT8 mMaximumRecords;
      UINT8 mRecordsUsed;
      UINT8 mMaximumGASStringLength;
   };

   // This array must be the size specified by mNumberOfSessions
   // sSession mSessions[1];
};

// Structure to describe response TLV 0x17 for PBMGetAllCapabilities()
struct sPBMGetAllCapabilitiesResponse_AAS
{
   UINT8 mNumberOfSessions;

   struct sSession
   {
      eQMIPBMSessionTypes mSessionType;
      UINT8 mMaximumRecords;
      UINT8 mRecordsUsed;
      UINT8 mMaximumAASStringLength;
   };

   // This array must be the size specified by mNumberOfSessions
   // sSession mSessions[1];
};

// Structure to describe request TLV 0x01 for PBMReadRecords()
struct sPBMReadRecordsRequest_Info
{
   eQMIPBMSessionTypes mSessionType;
   eQMIPBMPhonebookTypes mPhonebookType;
   UINT16 mStartingRecordID;
   UINT16 mEndingRecordID;
};

// Structure to describe response TLV 0x10 for PBMReadRecords()
struct sPBMReadRecordsResponse_RecordsRead
{
   UINT16 mNumberOfRecords;
};

// Structure to describe indication TLV 0x01 for PBM ReadRecordsIndication
struct sPBMReadRecordsIndication_Basic
{
   UINT16 mSequenceNumber;
   eQMIPBMSessionTypes mSessionType;
   eQMIPBMPhonebookTypes mPhonebookType;
   UINT8 mNumberOfRecords;
   
   struct sRecord1
   {
      UINT16 mRecordID;
      eQMIPBMNumberTypes mNumberType;
      eQMIPBMNumberPlans mNumberPlan;
      UINT8 mNumberLength;
   
      // This array must be the size specified by mNumberLength
      // char mNumber[1];
   };
   
   struct sRecord2
   {
      UINT8 mNameLength;
   
      // This array must be the size (in BYTEs) specified by mNameLength
      // wchar_t mName[1];
   };
   
   struct sRecord
   {
      sRecord1 mRecord1;
      sRecord2 mRecord2;
   };
   
   // This array must be the size specified by mNumberOfRecords
   // sRecord mRecords[1];
};

// Structure to describe indication TLV 0x10 for PBM ReadRecordsIndication
struct sPBMReadRecordsIndication_SecondName
{
   UINT8 mNumberOfRecords;

   struct sRecord
   {
      UINT16 mRecordID;
      UINT8 mSecondNameLength;
   
      // This array must be the size (in BYTEs) specified by mSecondNameLength
      // wchar_t mSecondName[1];
   };

   // This array must be the size specified by mNumberOfRecords
   // sRecord mRecords[1];
};

// Structure to describe indication TLV 0x11 for PBM ReadRecordsIndication
struct sPBMReadRecordsIndication_AdditionalNumber
{
   UINT8 mNumberOfRecords;

   struct sRecord
   {
      UINT16 mRecordID;
      UINT8 mAdditionalNumberCount;
      
      struct sAdditionalNumber1
      {
         eQMIPBMNumberTypes mNumberType;
         eQMIPBMNumberPlans mNumberPlan;
         UINT8 mNumberLength;
      
         // This array must be the size specified by mNumberLength
         // char mNumber[1];
      };
      
      struct sAdditionalNumber2
      {
         UINT8 mTagID;
      };
      
      struct sAdditionalNumber
      {
         sAdditionalNumber1 mAdditionalNumber1;
         sAdditionalNumber2 mAdditionalNumber2;
      };
      
      // This array must be the size specified by mAdditionalNumberCount
      // sAdditionalNumber mAdditionalNumbers[1];
   };

   // This array must be the size specified by mNumberOfRecords
   // sRecord mRecords[1];
};

// Structure to describe indication TLV 0x12 for PBM ReadRecordsIndication
struct sPBMReadRecordsIndication_Group
{
   UINT8 mNumberOfRecords;

   struct sRecord
   {
      UINT16 mRecordID;
      UINT8 mGroupCount;
   
      // This array must be the size specified by mGroupCount
      // UINT8 mGroupID[1];
   };

   // This array must be the size specified by mNumberOfRecords
   // sRecord mRecords[1];
};

// Structure to describe indication TLV 0x13 for PBM ReadRecordsIndication
struct sPBMReadRecordsIndication_Email
{
   UINT8 mNumberOfRecords;

   struct sRecord
   {
      UINT16 mRecordID;
      UINT8 mEmailCount;
   
      struct sEmail
      {
         UINT8 mAddressLength;
      
         // This array must be the size (in BYTEs) specified by mAddressLength
         // wchar_t mAddress[1];
      };
   
      // This array must be the size specified by mEmailCount
      // sEmail mEmails[1];
   };

   // This array must be the size specified by mNumberOfRecords
   // sRecord mRecords[1];
};

// Structure to describe indication TLV 0x14 for PBM ReadRecordsIndication
struct sPBMReadRecordsIndication_Hidden
{
   UINT8 mNumberOfRecords;

   struct sRecord
   {
      UINT16 mRecordID;
      INT8 mHidden;
   };

   // This array must be the size specified by mNumberOfRecords
   // sRecord mRecords[1];
};

// Structure to describe request TLV 0x01 for PBMWriteRecord()
struct sPBMWriteRecordRequest_Info1
{
   eQMIPBMSessionTypes mSessionType;
   eQMIPBMPhonebookTypes mPhonebookType;
   UINT16 mRecordID;
   eQMIPBMNumberTypes mNumberType;
   eQMIPBMNumberPlans mNumberPlan;
   UINT8 mNumberLength;

   // This array must be the size specified by mNumberLength
   // char mNumber[1];
};

struct sPBMWriteRecordRequest_Info2
{
   UINT8 mNameLength;

   // This array must be the size (in BYTEs) specified by mNameLength
   // wchar_t mName[1];
};

struct sPBMWriteRecordRequest_Info
{
   sPBMWriteRecordRequest_Info1 mPBMWriteRecordRequest_Info1;
   sPBMWriteRecordRequest_Info2 mPBMWriteRecordRequest_Info2;
};

// Structure to describe request TLV 0x10 for PBMWriteRecord()
struct sPBMWriteRecordRequest_SecondName
{
   UINT8 mNameLength;

   // This array must be the size (in BYTEs) specified by mNameLength
   // wchar_t mName[1];
};

// Structure to describe request TLV 0x11 for PBMWriteRecord()
struct sPBMWriteRecordRequest_AdditionalNumber
{
   UINT8 mAdditionalNumberCount;
   
   struct sAdditionalNumber1
   {
      eQMIPBMNumberTypes mNumberType;
      eQMIPBMNumberPlans mNumberPlan;
      UINT8 mNumberLength;
   
      // This array must be the size specified by mNumberLength
      // char mNumber[1];
   };
   
   struct sAdditionalNumber2
   {
      UINT8 mTagID;
   };
   
   struct sAdditionalNumber
   {
      sAdditionalNumber1 mAdditionalNumber1;
      sAdditionalNumber2 mAdditionalNumber2;
   };
   
   // This array must be the size specified by mAdditionalNumberCount
   // sAdditionalNumber mAdditionalNumbers[1];
};

// Structure to describe request TLV 0x12 for PBMWriteRecord()
struct sPBMWriteRecordRequest_Group
{
   UINT8 mGroupCount;

   // This array must be the size specified by mGroupCount
   // UINT8 mGroupID[1];
};

// Structure to describe request TLV 0x13 for PBMWriteRecord()
struct sPBMWriteRecordRequest_Email
{
   UINT8 mEmailCount;

   struct sEmail
   {
      UINT8 mAddressLength;
   
      // This array must be the size (in BYTEs) specified by mAddressLength
      // wchar_t mAddress[1];
   };

   // This array must be the size specified by mEmailCount
   // sEmail mEmails[1];
};

// Structure to describe request TLV 0x14 for PBMWriteRecord()
struct sPBMWriteRecordRequest_Hidden
{
   INT8 mHidden;
};

// Structure to describe response TLV 0x10 for PBMWriteRecord()
struct sPBMWriteRecordResponse_Info
{
   UINT16 mRecordID;
};

// Structure to describe request TLV 0x01 for PBMDeleteRecord()
struct sPBMDeleteRecordRequest_Info
{
   eQMIPBMSessionTypes mSessionType;
   eQMIPBMPhonebookTypes mPhonebookType;
   UINT16 mRecordID;
};

// Structure to describe response TLV 0x10 for PBMDeleteRecord()
struct sPBMDeleteRecordResponse_Info
{
   UINT16 mRecordID;
};

// Structure to describe request TLV 0x01 for PBMDeleteAllRecords()
struct sPBMDeleteAllRecordsRequest_Info
{
   eQMIPBMSessionTypes mSessionType;
   eQMIPBMPhonebookTypes mPhonebookType;
};

// Structure to describe request TLV 0x01 for PBMSearchRecords()
struct sPBMSearchRecordsRequest_Info
{
   eQMIPBMSessionTypes mSessionType;
   eQMIPBMPhonebookTypes mPhonebookType;
};

// Structure to describe request TLV 0x10 for PBMSearchRecords()
struct sPBMSearchRecordsRequest_Number
{
   UINT8 mNumberLength;

   // This array must be the size specified by mNumberLength
   // char mNumber[1];
};

// Structure to describe request TLV 0x11 for PBMSearchRecords()
struct sPBMSearchRecordsRequest_Name
{
   UINT8 mNameLength;

   // This array must be the size (in BYTEs) specified by mNameLength
   // wchar_t mName[1];
};

// Structure to describe response TLV 0x10 for PBMSearchRecords()
struct sPBMSearchRecordsResponse_List
{
   UINT16 mNumberOfRecordIDs;

   // This array must be the size specified by mNumberOfRecordIDs
   // UINT16 mRecordID[1];
};

// Structure to describe indication TLV 0x01 for PBM RecordUpdateIndication
struct sPBMRecordUpdateIndication_Info
{
   eQMIPBMSessionTypes mSessionType;
   eQMIPBMPhonebookTypes mPhonebookType;
   eQMIPBMOperations mOperation;
   UINT16 mRecordID;
};

// Structure to describe indication TLV 0x01 for PBM RefreshIndication
struct sPBMRefreshIndication_Info
{
   eQMIPBMSessionTypes mSessionType;
   eQMIPBMPhonebookTypes mPhonebookType;
   eQMIPBMRefreshStatus mStatus;
};

// Structure to describe indication TLV 0x01 for PBM ReadyIndication
struct sPBMReadyIndication_Info
{
   eQMIPBMSessionTypes mSessionType;
   eQMIPBMPhonebookTypes mPhonebookType;
};

// Structure to describe indication TLV 0x01 for PBM EmergencyListIndication
struct sPBMEmergencyListIndication_HardCodedNumbers
{
   UINT8 mNumberCount;

   struct sNumber
   {
      UINT8 mEmergencyNumberLength;
   
      // This array must be the size specified by mEmergencyNumberLength
      // char mEmergencyNumber[1];
   };

   // This array must be the size specified by mNumberCount
   // sNumber mNumbers[1];
};

// Structure to describe indication TLV 0x10 for PBM EmergencyListIndication
struct sPBMEmergencyListIndication_NVNumbers
{
   UINT8 mNumberCount;

   struct sNumber
   {
      UINT8 mEmergencyNumberLength;
   
      // This array must be the size specified by mEmergencyNumberLength
      // char mEmergencyNumber[1];
   };

   // This array must be the size specified by mNumberCount
   // sNumber mNumbers[1];
};

// Structure to describe indication TLV 0x11 for PBM EmergencyListIndication
struct sPBMEmergencyListIndication_CardNumbers
{
   UINT8 mNumberOfSessions;

   struct sSession
   {
      eQMIPBMSessionTypes mSessionType;
      UINT8 mNumberCount;
   
      struct sNumber
      {
         bool mPolice:1;
         bool mAmbulance:1;
         bool mFireBrigade:1;
         bool mMarineGuard:1;
         bool mMountainRescue:1;
         bool mManualECall:1;
         bool mAutomaticECall:1;
         bool mSpare:1;
         UINT8 mEmergencyNumberLength;
      
         // This array must be the size specified by mEmergencyNumberLength
         // char mEmergencyNumber[1];
      };
   
      // This array must be the size specified by mNumberCount
      // sNumber mNumbers[1];
   };

   // This array must be the size specified by mNumberOfSessions
   // sSession mSessions[1];
};

// Structure to describe indication TLV 0x12 for PBM EmergencyListIndication
struct sPBMEmergencyListIndication_NetworkNumbers
{
   UINT8 mNumberOfSessions;

   struct sSession
   {
      eQMIPBMSessionTypes mSessionType;
      UINT8 mNumberCount;
   
      struct sNumber
      {
         bool mPolice:1;
         bool mAmbulance:1;
         bool mFireBrigade:1;
         bool mMarineGuard:1;
         bool mMountainRescue:1;
         bool mManualECall:1;
         bool mAutomaticECall:1;
         bool mSpare:1;
         UINT8 mEmergencyNumberLength;
      
         // This array must be the size specified by mEmergencyNumberLength
         // char mEmergencyNumber[1];
      };
   
      // This array must be the size specified by mNumberCount
      // sNumber mNumbers[1];
   };

   // This array must be the size specified by mNumberOfSessions
   // sSession mSessions[1];
};

// Structure to describe indication TLV 0x01 for PBM AllReadyIndication
struct sPBMAllReadyIndication_Info
{
   UINT8 mNumberOfSessions;

   struct sSession
   {
      eQMIPBMSessionTypes mSessionType;
      bool mAbbreviatedDialingNumber:1;
      bool mFixedDialingNumber:1;
      bool mMobileSubscriberIntegratedServicesDigitalNetwork:1;
      bool mMailBoxDialingNumber:1;
      bool mServiceDialingNumber:1;
      bool mBarredDialingNumber:1;
      bool mLastNumberDialed:1;
      bool mMailBoxNumber:1;
   
      // Padding out 8 bits
      UINT8 mReserved1;
   };

   // This array must be the size specified by mNumberOfSessions
   // sSession mSessions[1];
};

// Structure to describe response TLV 0x10 for PBMGetEmergencyList()
struct sPBMGetEmergencyListResponse_HardCodedNumbers
{
   UINT8 mNumberCount;

   struct sNumber
   {
      UINT8 mEmergencyNumberLength;
   
      // This array must be the size specified by mEmergencyNumberLength
      // char mEmergencyNumber[1];
   };

   // This array must be the size specified by mNumberCount
   // sNumber mNumbers[1];
};

// Structure to describe response TLV 0x11 for PBMGetEmergencyList()
struct sPBMGetEmergencyListResponse_NVNumbers
{
   UINT8 mNumberCount;

   struct sNumber
   {
      UINT8 mEmergencyNumberLength;
   
      // This array must be the size specified by mEmergencyNumberLength
      // char mEmergencyNumber[1];
   };

   // This array must be the size specified by mNumberCount
   // sNumber mNumbers[1];
};

// Structure to describe response TLV 0x12 for PBMGetEmergencyList()
struct sPBMGetEmergencyListResponse_CardNumbers
{
   UINT8 mNumberOfSessions;

   struct sSession
   {
      eQMIPBMSessionTypes mSessionType;
      UINT8 mNumberCount;
   
      struct sNumber
      {
         bool mPolice:1;
         bool mAmbulance:1;
         bool mFireBrigade:1;
         bool mMarineGuard:1;
         bool mMountainRescue:1;
         bool mManualECall:1;
         bool mAutomaticECall:1;
         bool mSpare:1;
         UINT8 mEmergencyNumberLength;
      
         // This array must be the size specified by mEmergencyNumberLength
         // char mEmergencyNumber[1];
      };
   
      // This array must be the size specified by mNumberCount
      // sNumber mNumbers[1];
   };

   // This array must be the size specified by mNumberOfSessions
   // sSession mSessions[1];
};

// Structure to describe response TLV 0x13 for PBMGetEmergencyList()
struct sPBMGetEmergencyListResponse_NetworkNumbers
{
   UINT8 mNumberOfSessions;

   struct sSession
   {
      eQMIPBMSessionTypes mSessionType;
      UINT8 mNumberCount;
   
      struct sNumber
      {
         bool mPolice:1;
         bool mAmbulance:1;
         bool mFireBrigade:1;
         bool mMarineGuard:1;
         bool mMountainRescue:1;
         bool mManualECall:1;
         bool mAutomaticECall:1;
         bool mSpare:1;
         UINT8 mEmergencyNumberLength;
      
         // This array must be the size specified by mEmergencyNumberLength
         // char mEmergencyNumber[1];
      };
   
      // This array must be the size specified by mNumberCount
      // sNumber mNumbers[1];
   };

   // This array must be the size specified by mNumberOfSessions
   // sSession mSessions[1];
};

// Structure to describe response TLV 0x10 for PBMGetAllGroups()
struct sPBMGetAllGroupsResponse_Groups
{
   UINT8 mNumberOfSessions;

   struct sSession
   {
      eQMIPBMSessionTypes mSessionType;
      UINT8 mGroupCount;
   
      struct sGroup
      {
         UINT8 mGroupID;
         UINT8 mGroupNameLength;
      
         // This array must be the size (in BYTEs) specified by mGroupNameLength
         // wchar_t mGroupName[1];
      };
   
      // This array must be the size specified by mGroupCount
      // sGroup mGroups[1];
   };

   // This array must be the size specified by mNumberOfSessions
   // sSession mSessions[1];
};

// Structure to describe request TLV 0x01 for PBMSetGroupInfo()
struct sPBMSetGroupInfoRequest_Info
{
   eQMIPBMSessionTypes mSessionType;
   eQMIPBMOperations mOperation;
   UINT8 mGroupID;
   UINT8 mGroupNameLength;

   // This array must be the size (in BYTEs) specified by mGroupNameLength
   // wchar_t mGroupName[1];
};

// Structure to describe response TLV 0x10 for PBMSetGroupInfo()
struct sPBMSetGroupInfoResponse_ID
{
   eQMIPBMSessionTypes mSessionType;
   UINT8 mGroupID;
};

// Structure to describe request TLV 0x01 for PBMGetState()
struct sPBMGetStateRequest_Info
{
   eQMIPBMSessionTypes mSessionType;
   eQMIPBMPhonebookTypes mPhonebookType;
};

// Structure to describe response TLV 0x10 for PBMGetStateInfo()
struct sPBMGetStateInfoResponse_State
{
   eQMIPBMSessionTypes mSessionType;
   eQMIPBMPhonebookTypes mPhonebookType;
   eQMIPBMStates mState;
};

// Structure to describe request TLV 0x01 for PBMReadAllHiddenRecords()
struct sPBMReadAllHiddenRecordsRequest_Info
{
   eQMIPBMSessionTypes mSessionType;
};

// Structure to describe response TLV 0x10 for PBMReadAllHiddenRecords()
struct sPBMReadAllHiddenRecordsResponse_Records
{
   UINT16 mNumberOfRecords;
};

// Structure to describe indication TLV 0x01 for PBM HiddenRecordStatusIndication
struct sPBMHiddenRecordStatusIndication_Status
{
   eQMIPBMSessionTypes mSessionType;
   INT8 mHiddenRecordsValid;
};

// Structure to describe request TLV 0x01 for PBMGetNextEmptyRecordID()
struct sPBMGetNextEmptyRecordIDRequest_Info
{
   eQMIPBMSessionTypes mSessionType;
   eQMIPBMPhonebookTypes mPhonebookType;
   UINT16 mRecordID;
};

// Structure to describe response TLV 0x10 for PBMGetNextEmptyRecordID()
struct sPBMGetNextEmptyRecordIDResponse_Info
{
   UINT16 mRecordID;
};

// Structure to describe request TLV 0x01 for PBMGetNextRecordID()
struct sPBMGetNextRecordIDRequest_Info
{
   eQMIPBMSessionTypes mSessionType;
   eQMIPBMPhonebookTypes mPhonebookType;
   UINT16 mRecordID;
};

// Structure to describe response TLV 0x10 for PBMGetNextRecordID()
struct sPBMGetNextRecordIDResponse_Info
{
   UINT16 mRecordID;
};

// Structure to describe response TLV 0x10 for PBMGetAASList()
struct sPBMGetAASListResponse_List
{
   UINT8 mNumberOfSessions;

   struct sSession
   {
      eQMIPBMSessionTypes mSessionType;
      UINT8 mAASCount;
   
      struct sAAS
      {
         UINT8 mAASID;
         UINT8 mAlphaStringLength;
      
         // This array must be the size (in BYTEs) specified by mAlphaStringLength
         // wchar_t mAlphaString[1];
      };
   
      // This array must be the size specified by mAASCount
      // sAAS mAASs[1];
   };

   // This array must be the size specified by mNumberOfSessions
   // sSession mSessions[1];
};

// Structure to describe request TLV 0x10 for PBMSetAAS()
struct sPBMSetAASRequest_Info
{
   eQMIPBMSessionTypes mSessionType;
   eQMIPBMAASOperations mOperation;
   UINT8 mAASID;
   UINT8 mAlphaStringLength;

   // This array must be the size (in BYTEs) specified by mAlphaStringLength
   // wchar_t mAlphaString[1];
};

// Structure to describe response TLV 0x10 for PBMSetAAS()
struct sPBMSetAASResponse_Info
{
   eQMIPBMSessionTypes mSessionType;
   UINT8 mAASID;
};

// Structure to describe indication TLV 0x01 for PBM AASChangeIndication
struct sPBMAASChangeIndication_Info
{
   eQMIPBMSessionTypes mSessionType;
   eQMIPBMOperations mOperation;
   UINT8 mAASID;
   UINT8 mAlphaStringLength;

   // This array must be the size (in BYTEs) specified by mAlphaStringLength
   // wchar_t mAlphaString[1];
};

// Structure to describe indication TLV 0x01 for PBM GASChangeIndication
struct sPBMGASChangeIndication_Info
{
   eQMIPBMSessionTypes mSessionType;
   eQMIPBMOperations mOperation;
   UINT8 mGASID;
   UINT8 mGroupNameLength;

   // This array must be the size (in BYTEs) specified by mGroupNameLength
   // wchar_t mGroupName[1];
};

// Structure to describe request TLV 0x10 for PBMBindSubscription()
struct sPBMBindSubscriptionRequest_Type
{
   eQMIPBMSubscriptionTypes mSubscriptionType;
};

// Structure to describe request TLV 0x10 for PBMGetSubscription()
struct sPBMGetSubscriptionRequest_Type
{
   eQMIPBMSubscriptionTypes mSubscriptionType;
};

// Structure to describe request TLV 0x01 for LOCClientRevision()
struct sLOCClientRevisionRequest_Revision
{
   UINT32 mRevision;
};

// Structure to describe request TLV 0x01 for LOCRegisterEvents()
struct sLOCRegisterEventsRequest_EventRegistrationMask
{
   bool mPositionReport:1;
   bool mGNSSSatelliteInfo:1;
   bool mNMEA:1;
   bool mNINotifyVerifyRequest:1;
   bool mInjectTimeRequest:1;
   bool mInjectPredictedOrbitsRequest:1;
   bool mInjectPositionRequest:1;
   bool mEngineState:1;
   bool mFixSessionState:1;
   bool mWiFiRequest:1;
   bool mSensorStreamingReadyStatus:1;
   bool mTimeSyncRequest:1;
   bool mSetSPIStreamingReport:1;
   bool mLocationServerConnectionRequest:1;
   bool mNIGeofenceNotification:1;
   bool mGeofenceGeneralAlert:1;
   bool mGeofenceBreachNotification:1;

   // Padding out 47 bits
   UINT8 mReserved1:7;
   UINT8 mReserved2[5];
};

// Structure to describe request TLV 0x01 for LOCStart()
struct sLOCStartRequest_SessionID
{
   UINT8 mSessionID;
};

// Structure to describe request TLV 0x10 for LOCStart()
struct sLOCStartRequest_FixRecurrenceType
{
   eQMILOCFixRecurrenceType mFixRecurrenceType;
};

// Structure to describe request TLV 0x11 for LOCStart()
struct sLOCStartRequest_HorizontalAccuracy
{
   eQMILOCHorizontalAccuracy mHorizontalAccuracy;
};

// Structure to describe request TLV 0x12 for LOCStart()
struct sLOCStartRequest_EnableIntermediateReports
{
   eQMILOCIntermediateReportState mEnableIntermediateReports;
};

// Structure to describe request TLV 0x13 for LOCStart()
struct sLOCStartRequest_MinimumIntervalBetweenPositionReports
{
   UINT32 mMinimumTimeIntervalMilliseconds;
};

// Structure to describe request TLV 0x14 for LOCStart()
struct sLOCStartRequest_ApplicationID1
{
   UINT8 mApplicationProviderLength;

   // This array must be the size specified by mApplicationProviderLength
   // char mApplicationProvider[1];
};

struct sLOCStartRequest_ApplicationID2
{
   UINT8 mApplicationNameLength;

   // This array must be the size specified by mApplicationNameLength
   // char mApplicationName[1];
};

struct sLOCStartRequest_ApplicationID3
{
   INT8 mApplicationVersionValid;
   UINT8 mApplicationVersionLength;

   // This array must be the size specified by mApplicationVersionLength
   // char mApplicationVersion[1];
};

struct sLOCStartRequest_ApplicationID
{
   sLOCStartRequest_ApplicationID1 mLOCStartRequest_ApplicationID1;
   sLOCStartRequest_ApplicationID2 mLOCStartRequest_ApplicationID2;
   sLOCStartRequest_ApplicationID3 mLOCStartRequest_ApplicationID3;
};

// Structure to describe request TLV 0x01 for LOCStop()
struct sLOCStopRequest_SessionID
{
   UINT8 mSessionID;
};

// Structure to describe indication TLV 0x01 for LOC PositionReportIndication
struct sLOCPositionReportIndication_SessionStatus
{
   eQMILOCSessionStatus mSessionStatus;
};

// Structure to describe indication TLV 0x10 for LOC PositionReportIndication
struct sLOCPositionReportIndication_Latitude
{
   double mLatitudeDegrees;
};

// Structure to describe indication TLV 0x11 for LOC PositionReportIndication
struct sLOCPositionReportIndication_Longitude
{
   double mLongitudeDegrees;
};

// Structure to describe indication TLV 0x12 for LOC PositionReportIndication
struct sLOCPositionReportIndication_HorizontalUncertaintyCircular
{
   float mHorizontalUncertaintyCircularMeters;
};

// Structure to describe indication TLV 0x13 for LOC PositionReportIndication
struct sLOCPositionReportIndication_HorizontalUncertaintyEllipticalMinor
{
   float mHorizontalUncertaintyEllipticalMinorMeters;
};

// Structure to describe indication TLV 0x14 for LOC PositionReportIndication
struct sLOCPositionReportIndication_HorizontalUncertaintyEllipticalMajor
{
   float mHorizontalUncertaintyEllipticalMajorMeters;
};

// Structure to describe indication TLV 0x15 for LOC PositionReportIndication
struct sLOCPositionReportIndication_HorizontalUncertaintyEllipticalAzimuth
{
   float mHorizontalUncertaintyEllipticalAzimuthDecimalDegrees;
};

// Structure to describe indication TLV 0x16 for LOC PositionReportIndication
struct sLOCPositionReportIndication_HorizontalConfidence
{
   UINT8 mHorizontalConfidencePercent;
};

// Structure to describe indication TLV 0x17 for LOC PositionReportIndication
struct sLOCPositionReportIndication_HorizontalReliability
{
   eQMILOCReliability mHorizontalReliability;
};

// Structure to describe indication TLV 0x18 for LOC PositionReportIndication
struct sLOCPositionReportIndication_HorizontalSpeed
{
   float mHorizontalSpeedMetersSecond;
};

// Structure to describe indication TLV 0x19 for LOC PositionReportIndication
struct sLOCPositionReportIndication_SpeedUncertainty
{
   float mSpeedUncertaintyMetersSecond;
};

// Structure to describe indication TLV 0x1A for LOC PositionReportIndication
struct sLOCPositionReportIndication_AltitudeFromEllipsoid
{
   float mAltitudeFromEllipsoidMeters;
};

// Structure to describe indication TLV 0x1B for LOC PositionReportIndication
struct sLOCPositionReportIndication_AltitudeFromSeaLevel
{
   float mAltitudeFromSeaLevelMeters;
};

// Structure to describe indication TLV 0x1C for LOC PositionReportIndication
struct sLOCPositionReportIndication_VerticalUncertainty
{
   float mVerticalUncertaintyMeters;
};

// Structure to describe indication TLV 0x1D for LOC PositionReportIndication
struct sLOCPositionReportIndication_VerticalConfidence
{
   UINT8 mVerticalConfidencePercent;
};

// Structure to describe indication TLV 0x1E for LOC PositionReportIndication
struct sLOCPositionReportIndication_VerticalReliability
{
   eQMILOCReliability mVerticalReliability;
};

// Structure to describe indication TLV 0x1F for LOC PositionReportIndication
struct sLOCPositionReportIndication_VerticalSpeed
{
   float mVerticalSpeedMetersSecond;
};

// Structure to describe indication TLV 0x20 for LOC PositionReportIndication
struct sLOCPositionReportIndication_Heading
{
   float mHeadingDegrees;
};

// Structure to describe indication TLV 0x21 for LOC PositionReportIndication
struct sLOCPositionReportIndication_HeadingUncertainty
{
   float mHeadingUncertaintyDegrees;
};

// Structure to describe indication TLV 0x22 for LOC PositionReportIndication
struct sLOCPositionReportIndication_MagneticDeviation
{
   float mMagneticDeviation;
};

// Structure to describe indication TLV 0x23 for LOC PositionReportIndication
struct sLOCPositionReportIndication_TechnologyUsed
{
   bool mSatellite:1;
   bool mCellular:1;
   bool mWiFi:1;
   bool mSensors:1;
   bool mReferenceLocation:1;

   // Padding out 27 bits
   UINT8 mReserved1:3;
   UINT8 mReserved2[3];
};

// Structure to describe indication TLV 0x24 for LOC PositionReportIndication
struct sLOCPositionReportIndication_DilutionOfPrecision
{
   float mPositionDilutionOfPrecision;
   float mHorizontalDilutionOfPrecision;
   float mVerticalDilutionOfPrecision;
};

// Structure to describe indication TLV 0x25 for LOC PositionReportIndication
struct sLOCPositionReportIndication_UTCTimestamp
{
   UINT64 mUTCTimestampMilliseconds;
};

// Structure to describe indication TLV 0x26 for LOC PositionReportIndication
struct sLOCPositionReportIndication_LeapSeconds
{
   UINT8 mLeapSeconds;
};

// Structure to describe indication TLV 0x27 for LOC PositionReportIndication
struct sLOCPositionReportIndication_GPSTime
{
   UINT16 mGPSWeeks;
   UINT32 mGPSTimeOfWeekMilliseconds;
};

// Structure to describe indication TLV 0x28 for LOC PositionReportIndication
struct sLOCPositionReportIndication_TimeUncertainty
{
   float mTimeUncertaintyMilliseconds;
};

// Structure to describe indication TLV 0x29 for LOC PositionReportIndication
struct sLOCPositionReportIndication_TimeSource
{
   eQMILOCTimeSource mTimeSource;
};

// Structure to describe indication TLV 0x2A for LOC PositionReportIndication
struct sLOCPositionReportIndication_SensorDataUsage
{
   bool mAccelerometerUsed:1;
   bool mGyroUsed:1;

   // Padding out 30 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[3];

   bool mAidedHeading:1;
   bool mAidedSpeed:1;
   bool mAidedPosition:1;
   bool mAidedVelocity:1;

   // Padding out 28 bits
   UINT8 mReserved3:4;
   UINT8 mReserved4[3];
};

// Structure to describe indication TLV 0x2B for LOC PositionReportIndication
struct sLOCPositionReportIndication_SessionFixCount
{
   UINT32 mSessionFixCount;
};

// Structure to describe indication TLV 0x2C for LOC PositionReportIndication
struct sLOCPositionReportIndication_SatellitesUsed
{
   UINT8 mSatellitesUsedCount;

   // This array must be the size specified by mSatellitesUsedCount
   // UINT16 mSatellitesUsed[1];
};

// Structure to describe indication TLV 0x01 for LOC GNSSSatelliteInfoIndication
struct sLOCGNSSSatelliteInfoIndication_AltitudeSource
{
   eQMILOCAltitudeAssumed mAltitudeAssumed;
};

// Structure to describe indication TLV 0x10 for LOC GNSSSatelliteInfoIndication
struct sLOCGNSSSatelliteInfoIndication_SatelliteInfo
{
   UINT8 mSatelliteInfoCount;
   bool mValidSystem:1;
   bool mValidGNSSSatelliteID:1;
   bool mValidHealthStatus:1;
   bool mValidProcessStatus:1;
   bool mValidSatelliteInfoMask:1;
   bool mValidElevation:1;
   bool mValidAzimuth:1;
   bool mValidSignalToNoiseRatio:1;

   // Padding out 24 bits
   UINT8 mReserved1[3];

   eQMILOCSystem mSystem;
   UINT16 mGNSSSatelliteID;
   eQMILOCHealthStatus mHealthStatus;
   eQMILOCSatelliteStatus mSatelliteStatus;
   bool mHasEphemeris:1;
   bool mHasAlmanac:1;

   // Padding out 6 bits
   UINT8 mReserved2:6;

   float mElevationDegrees;
   float mAzimuthDegrees;
   float mSignalToNoiseRatiodBHz;
};

// Structure to describe indication TLV 0x01 for LOC NMEAIndication
struct sLOCNMEAIndication_NMEAString
{
   // String is variable length, but must be size of the container
   // char mNMEAString[1];
};

// Structure to describe request TLV 0x01 for LOCNetworkInitiated()
struct sLOCNetworkInitiatedRequestIndication_NotificationType
{
   eQMILOCNotificationType mNotificationType;
};

// Structure to describe request TLV 0x10 for LOCNetworkInitiated()
struct sLOCNetworkInitiatedRequestIndication_VxRequest1
{
   INT8 mPositionQoSIncluded;
   UINT8 mPositionQoSTimeoutSeconds;
   UINT32 mMaxNumberOfFixes;
   UINT32 mTimeBetweenFixesSeconds;
   eQMILOCPositionMode mPosistionMode;
   eQMILOCEncodingScheme mEncodingScheme;
   UINT8 mRequestorIDLength;

   // This array must be the size specified by mRequestorIDLength
   // UINT8 mRequestorID[1];
};

struct sLOCNetworkInitiatedRequestIndication_VxRequest2
{
   UINT16 mUserResponseTimerSeconds;
};

struct sLOCNetworkInitiatedRequestIndication_VxRequest
{
   sLOCNetworkInitiatedRequestIndication_VxRequest1 mLOCNetworkInitiatedRequestIndication_VxRequest1;
   sLOCNetworkInitiatedRequestIndication_VxRequest2 mLOCNetworkInitiatedRequestIndication_VxRequest2;
};

// Structure to describe request TLV 0x11 for LOCNetworkInitiated()
struct sLOCNetworkInitiatedRequestIndication_SUPLRequest1
{
   bool mValidServerInfo:1;
   bool mValidSessionID:1;
   bool mValidHash:1;
   bool mValidPositionMethod:1;
   bool mValidDataCodingScheme:1;
   bool mValidRequestorID:1;
   bool mValidClientName:1;
   bool mValidQualityOfPosition:1;
   bool mValidUserResponseTimer:1;

   // Padding out 23 bits
   UINT8 mReserved1:7;
   UINT8 mReserved2[2];

   bool mIPv4:1;
   bool mIPv6:1;
   bool mURL:1;

   // Padding out 5 bits
   UINT8 mReserved3:5;

   UINT32 mIPv4Address;
   UINT16 mIPv4Port;
   UINT8 mIPv6Address[16];
   UINT32 mIPv6Port;
   UINT8 mURLAddressLength;

   // This array must be the size specified by mURLAddressLength
   // char mURLAddress[1];
};

struct sLOCNetworkInitiatedRequestIndication_SUPLRequest2
{
   UINT8 mSUPLSessionID[4];
   UINT8 mSUPLHash[8];
   eQMILOCPosition mPositionMethod;
   eQMILOCDataCodingScheme mDataCodingScheme;
   eQMILOCFormatType mRequestorIDFormatType;
   UINT8 mRequestorIDFormattedStringLength;

   // This array must be the size specified by mRequestorIDFormattedStringLength
   // UINT8 mRequestorIDFormattedString[1];
};

struct sLOCNetworkInitiatedRequestIndication_SUPLRequest3
{
   eQMILOCFormatType mClientNameFormatType;
   UINT8 mClientNameFormattedStringLength;

   // This array must be the size specified by mClientNameFormattedStringLength
   // UINT8 mClientNameFormattedString[1];
};

struct sLOCNetworkInitiatedRequestIndication_SUPLRequest4
{
   bool mQoPHorizontalAccelerationValid:1;
   bool mQoPVerticalAccelerationValid:1;
   bool mQoPMaximumAge:1;
   bool mQoPDelayValid:1;

   // Padding out 4 bits
   UINT8 mReserved4:4;

   UINT8 mHorizontalAccuracyMeters;
   UINT8 mVerticalAccuracyMeters;
   UINT16 mMaximumLocationAgeSeconds;
   UINT8 mDelaySeconds;
   UINT16 mUserResponseTimerSeconds;
};

struct sLOCNetworkInitiatedRequestIndication_SUPLRequest
{
   sLOCNetworkInitiatedRequestIndication_SUPLRequest1 mLOCNetworkInitiatedRequestIndication_SUPLRequest1;
   sLOCNetworkInitiatedRequestIndication_SUPLRequest2 mLOCNetworkInitiatedRequestIndication_SUPLRequest2;
   sLOCNetworkInitiatedRequestIndication_SUPLRequest3 mLOCNetworkInitiatedRequestIndication_SUPLRequest3;
   sLOCNetworkInitiatedRequestIndication_SUPLRequest4 mLOCNetworkInitiatedRequestIndication_SUPLRequest4;
};

// Structure to describe request TLV 0x12 for LOCNetworkInitiated()
struct sLOCNetworkInitiatedRequestIndication_UMTSControlPlaneRequest1
{
   bool mValidInvokeID:1;
   bool mValidDataCodingScheme:1;
   bool mValidNotificationText:1;
   bool mValidClientAddress:1;
   bool mValidLocationType:1;
   bool mValidRequestorID:1;
   bool mValidCodewordString:1;
   bool mValidServiceTypeMask:1;
   bool mValidUserResponseTImer:1;

   // Padding out 7 bits
   UINT8 mReserved1:7;

   UINT8 mInvokeID;
   eQMILOCDataCodingScheme mDataCodingScheme;
   UINT8 mNotificationTextLength;

   // This array must be the size specified by mNotificationTextLength
   // UINT8 mNotificationText[1];
};

struct sLOCNetworkInitiatedRequestIndication_UMTSControlPlaneRequest2
{
   UINT8 mClientAddressLength;

   // This array must be the size specified by mClientAddressLength
   // UINT8 mClientAddress[1];
};

struct sLOCNetworkInitiatedRequestIndication_UMTSControlPlaneRequest3
{
   eQMILOCLocationType mLocationType;
   eQMILOCDataCodingScheme mRequestorIDDataCodingScheme;
   UINT8 mRequestorIDCodedStingLength;

   // This array must be the size specified by mRequestorIDCodedStingLength
   // UINT8 mRequestorIDCodedString[1];
};

struct sLOCNetworkInitiatedRequestIndication_UMTSControlPlaneRequest4
{
   eQMILOCDataCodingScheme mCodewordStringDataCodingScheme;
   UINT8 mCodewordStringCodedStringLength;

   // This array must be the size specified by mCodewordStringCodedStringLength
   // UINT8 mCodewordStringCodedString[1];
};

struct sLOCNetworkInitiatedRequestIndication_UMTSControlPlaneRequest5
{
   UINT8 mServiceTypeID;
   UINT16 mUserResponseTimerSeconds;
};

struct sLOCNetworkInitiatedRequestIndication_UMTSControlPlaneRequest
{
   sLOCNetworkInitiatedRequestIndication_UMTSControlPlaneRequest1 mLOCNetworkInitiatedRequestIndication_UMTSControlPlaneRequest1;
   sLOCNetworkInitiatedRequestIndication_UMTSControlPlaneRequest2 mLOCNetworkInitiatedRequestIndication_UMTSControlPlaneRequest2;
   sLOCNetworkInitiatedRequestIndication_UMTSControlPlaneRequest3 mLOCNetworkInitiatedRequestIndication_UMTSControlPlaneRequest3;
   sLOCNetworkInitiatedRequestIndication_UMTSControlPlaneRequest4 mLOCNetworkInitiatedRequestIndication_UMTSControlPlaneRequest4;
   sLOCNetworkInitiatedRequestIndication_UMTSControlPlaneRequest5 mLOCNetworkInitiatedRequestIndication_UMTSControlPlaneRequest5;
};

// Structure to describe request TLV 0x13 for LOCNetworkInitiated()
struct sLOCNetworkInitiatedRequestIndication_ServiceInteractionRequest
{
   INT8 mPositionQoSIncluded;
   UINT8 mPositionQoSTimeoutSeconds;
   UINT32 mMaxNumberOfFixes;
   UINT32 mTimeBetweenFixesSeconds;
   eQMILOCPositionMode mPosistionMode;
   eQMILOCEncodingScheme mEncodingScheme;
   UINT8 mRequestorIDLength;
   UINT8 mRequestorID;
   UINT16 mUserResponseTimerSeconds;
   eQMILOCServiceInteractionType mServiceInteractionType;
};

// Structure to describe request TLV 0x14 for LOCNetworkInitiated()
struct sLOCNetworkInitiatedRequestIndication_SUPLVersion2Extension
{
   bool mWLAN:1;
   bool mGSM:1;
   bool mWCDMA:1;
   bool mCDMA:1;
   bool mHRDP:1;
   bool mUMB:1;
   bool mLTE:1;
   bool mWIMAX:1;
   bool mHISTORIC:1;
   bool mNONSVRV:1;

   // Padding out 6 bits
   UINT8 mReserved1:6;

   eQMILOCTriggerType mTriggerType;
   bool mGPS:1;
   bool mGLONASS:1;
   bool mGALILEO:1;
   bool mSBAS:1;
   bool mQZSS:1;
   bool mMODERNGPS:1;

   // Padding out 10 bits
   UINT8 mReserved2:2;
   UINT8 mReserved3;
};

// Structure to describe indication TLV 0x10 for LOC InjectTimeIndication
struct sLOCInjectTimeIndication_TimeServerInfo
{
   UINT32 mDelayThresholdMilliseconds;
   UINT8 mServerListLength;

   struct sServer
   {
      UINT8 mServerURLLength;
   
      // This array must be the size specified by mServerURLLength
      // char mServerURL[1];
   };

   // This array must be the size specified by mServerListLength
   // sServer mServers[1];
};

// Structure to describe indication TLV 0x01 for LOC InjectPredictedOrbitsIndication
struct sLOCInjectPredictedOrbitsIndication_AllowedSize
{
   UINT32 mMaximumFileSizeBytes;
   UINT32 mMaximumPartSizeBytes;
};

// Structure to describe indication TLV 0x10 for LOC InjectPredictedOrbitsIndication
struct sLOCInjectPredictedOrbitsIndication_ServerList
{
   UINT8 mServerListLength;

   struct sServer
   {
      UINT8 mServerURLLength;
   
      // This array must be the size specified by mServerURLLength
      // char mServerURL[1];
   };

   // This array must be the size specified by mServerListLength
   // sServer mServers[1];
};

// Structure to describe indication TLV 0x01 for LOC InjectPositionIndication
struct sLOCInjectPositionIndication_Latitude
{
   double mLatitudeDegrees;
};

// Structure to describe indication TLV 0x03 for LOC InjectPositionIndication
struct sLOCInjectPositionIndication_HorizontalUncertaintyCircular
{
   float mHorizontalUncertaintyCircularMeters;
};

// Structure to describe indication TLV 0x04 for LOC InjectPositionIndication
struct sLOCInjectPositionIndication_UTCTimestamp
{
   UINT64 mUTCTimestampMilliseconds;
};

// Structure to describe indication TLV 0x01 for LOC EngineStateIndication
struct sLOCEngineStateIndication_EngineState
{
   eQMILOCEngineState mEngineState;
};

// Structure to describe indication TLV 0x01 for LOC FixSessionStateIndication
struct sLOCFixSessionStateIndication_SessionState
{
   eQMILOCSessionState mSessionState;
};

// Structure to describe indication TLV 0x10 for LOC FixSessionStateIndication
struct sLOCFixSessionStateIndication_SessionID
{
   UINT8 mSessionID;
};

// Structure to describe request TLV 0x01 for LOCWiFi()
struct sLOCWiFiRequestIndication_RequestType
{
   eQMILOCRequestType mRequestType;
};

// Structure to describe request TLV 0x10 for LOCWiFi()
struct sLOCWiFiRequestIndication_TimeBetweenFixes
{
   UINT16 mTimeBetweenFixesMilliseconds;
};

// Structure to describe indication TLV 0x10 for LOC SensorStreamingReadyStatusIndication
struct sLOCSensorStreamingReadyStatusIndication_AccelerometerReady
{
   INT8 mReadyForInjection;
   UINT16 mSamplesPerBatch;
   UINT16 mBatchesPerSecond;
};

// Structure to describe indication TLV 0x11 for LOC SensorStreamingReadyStatusIndication
struct sLOCSensorStreamingReadyStatusIndication_GyrometerReady
{
   INT8 mReadyForInjection;
   UINT16 mSamplesPerBatch;
   UINT16 mBatchesPerSecond;
};

// Structure to describe request TLV 0x01 for LOCTimeSync()
struct sLOCTimeSyncRequestIndication_ReferenceCounter
{
   UINT32 mReferenceCounter;
};

// Structure to describe indication TLV 0x01 for LOC SetSPIStreamingReportIndication
struct sLOCSetSPIStreamingReportIndication_SPIRequests
{
   INT8 mEnableSPIRequests;
};

// Structure to describe request TLV 0x01 for LOCLocationServerConnection()
struct sLOCLocationServerConnectionRequestIndication_ConnectionHandle
{
   UINT32 mConnectionHandle;
};

// Structure to describe request TLV 0x03 for LOCLocationServerConnection()
struct sLOCLocationServerConnectionRequestIndication_WWANType
{
   eQMILOCWWANType mWWANType;
};

// Structure to describe indication TLV 0x10 for LOC GetServiceRevisionIndication
struct sLOCGetServiceRevisionIndication_GNSSMeasurementEngineFirmwareVersion
{
   // String is variable length, but must be size of the container
   // char mFirmwareVersion[1];
};

// Structure to describe indication TLV 0x11 for LOC GetServiceRevisionIndication
struct sLOCGetServiceRevisionIndication_GNSSHostedSoftwareVersion
{
   // String is variable length, but must be size of the container
   // char mSoftwareVersion[1];
};

// Structure to describe indication TLV 0x12 for LOC GetServiceRevisionIndication
struct sLOCGetServiceRevisionIndication_GNSSSoftwareVersion
{
   // String is variable length, but must be size of the container
   // char mSoftwareVersion[1];
};

// Structure to describe indication TLV 0x10 for LOC GetFixCriteriaIndication
struct sLOCGetFixCriteriaIndication_HorizontalAccuracy
{
   eQMILOCHorizontalAccuracy mHorizontalAccuracy;
};

// Structure to describe indication TLV 0x11 for LOC GetFixCriteriaIndication
struct sLOCGetFixCriteriaIndication_IntermediateFixes
{
   eQMILOCIntermediateReportState mEnableIntermediateReports;
};

// Structure to describe indication TLV 0x12 for LOC GetFixCriteriaIndication
struct sLOCGetFixCriteriaIndication_MinimumIntervalBetweenFixes
{
   UINT32 mMinimumTimeIntervalMilliseconds;
};

// Structure to describe indication TLV 0x13 for LOC GetFixCriteriaIndication
struct sLOCGetFixCriteriaIndication_ApplicationID1
{
   UINT8 mApplicationProviderLength;

   // This array must be the size specified by mApplicationProviderLength
   // char mApplicationProvider[1];
};

struct sLOCGetFixCriteriaIndication_ApplicationID2
{
   UINT8 mApplicationNameLength;

   // This array must be the size specified by mApplicationNameLength
   // char mApplicationName[1];
};

struct sLOCGetFixCriteriaIndication_ApplicationID3
{
   INT8 mApplicationVersionValid;
   UINT8 mApplicationVersionLength;

   // This array must be the size specified by mApplicationVersionLength
   // char mApplicationVersion[1];
};

struct sLOCGetFixCriteriaIndication_ApplicationID
{
   sLOCGetFixCriteriaIndication_ApplicationID1 mLOCGetFixCriteriaIndication_ApplicationID1;
   sLOCGetFixCriteriaIndication_ApplicationID2 mLOCGetFixCriteriaIndication_ApplicationID2;
   sLOCGetFixCriteriaIndication_ApplicationID3 mLOCGetFixCriteriaIndication_ApplicationID3;
};

// Structure to describe request TLV 0x01 for LOCProvideNIUserResponse()
struct sLOCProvideNIUserResponseRequest_UserResponse
{
   eQMILOCUserResponse mUserResponse;
};

// Structure to describe request TLV 0x10 for LOCProvideNIUserResponse()
struct sLOCProvideNIUserResponseRequest_VxRequest1
{
   INT8 mPositionQoSIncluded;
   UINT8 mPositionQoSTimeoutSeconds;
   UINT32 mMaxNumberOfFixes;
   UINT32 mTimeBetweenFixesSeconds;
   eQMILOCPositionMode mPosistionMode;
   eQMILOCEncodingScheme mEncodingScheme;
   UINT8 mRequestorIDLength;

   // This array must be the size specified by mRequestorIDLength
   // UINT8 mRequestorID[1];
};

struct sLOCProvideNIUserResponseRequest_VxRequest2
{
   UINT16 mUserResponseTimerSeconds;
};

struct sLOCProvideNIUserResponseRequest_VxRequest
{
   sLOCProvideNIUserResponseRequest_VxRequest1 mLOCProvideNIUserResponseRequest_VxRequest1;
   sLOCProvideNIUserResponseRequest_VxRequest2 mLOCProvideNIUserResponseRequest_VxRequest2;
};

// Structure to describe request TLV 0x11 for LOCProvideNIUserResponse()
struct sLOCProvideNIUserResponseRequest_SUPLRequest1
{
   bool mValidServerInfo:1;
   bool mValidSessionID:1;
   bool mValidHash:1;
   bool mValidPositionMethod:1;
   bool mValidDataCodingScheme:1;
   bool mValidRequestorID:1;
   bool mValidClientName:1;
   bool mValidQualityOfPosition:1;
   bool mValidUserResponseTimer:1;

   // Padding out 23 bits
   UINT8 mReserved1:7;
   UINT8 mReserved2[2];

   bool mIPv4:1;
   bool mIPv6:1;
   bool mURL:1;

   // Padding out 5 bits
   UINT8 mReserved3:5;

   UINT32 mIPv4Address;
   UINT16 mIPv4Port;
   UINT8 mIPv6Address[16];
   UINT32 mIPv6Port;
   UINT8 mURLAddressLength;

   // This array must be the size specified by mURLAddressLength
   // char mURLAddress[1];
};

struct sLOCProvideNIUserResponseRequest_SUPLRequest2
{
   UINT8 mSUPLSessionID[4];
   UINT8 mSUPLHash[8];
   eQMILOCPosition mPositionMethod;
   eQMILOCDataCodingScheme mDataCodingScheme;
   eQMILOCFormatType mRequestorIDFormatType;
   UINT8 mRequestorIDFormattedStringLength;

   // This array must be the size specified by mRequestorIDFormattedStringLength
   // UINT8 mRequestorIDFormattedString[1];
};

struct sLOCProvideNIUserResponseRequest_SUPLRequest3
{
   eQMILOCFormatType mClientNameFormatType;
   UINT8 mClientNameFormattedStringLength;

   // This array must be the size specified by mClientNameFormattedStringLength
   // UINT8 mClientNameFormattedString[1];
};

struct sLOCProvideNIUserResponseRequest_SUPLRequest4
{
   bool mQoPHorizontalAccelerationValid:1;
   bool mQoPVerticalAccelerationValid:1;
   bool mQoPMaximumAge:1;
   bool mQoPDelayValid:1;

   // Padding out 4 bits
   UINT8 mReserved4:4;

   UINT8 mHorizontalAccuracyMeters;
   UINT8 mVerticalAccuracyMeters;
   UINT16 mMaximumLocationAgeSeconds;
   UINT8 mDelaySeconds;
   UINT16 mUserResponseTimerSeconds;
};

struct sLOCProvideNIUserResponseRequest_SUPLRequest
{
   sLOCProvideNIUserResponseRequest_SUPLRequest1 mLOCProvideNIUserResponseRequest_SUPLRequest1;
   sLOCProvideNIUserResponseRequest_SUPLRequest2 mLOCProvideNIUserResponseRequest_SUPLRequest2;
   sLOCProvideNIUserResponseRequest_SUPLRequest3 mLOCProvideNIUserResponseRequest_SUPLRequest3;
   sLOCProvideNIUserResponseRequest_SUPLRequest4 mLOCProvideNIUserResponseRequest_SUPLRequest4;
};

// Structure to describe request TLV 0x12 for LOCProvideNIUserResponse()
struct sLOCProvideNIUserResponseRequest_UMTSControlPlaneRequest1
{
   bool mValidInvokeID:1;
   bool mValidDataCodingScheme:1;
   bool mValidNotificationText:1;
   bool mValidClientAddress:1;
   bool mValidLocationType:1;
   bool mValidRequestorID:1;
   bool mValidCodewordString:1;
   bool mValidServiceTypeMask:1;
   bool mValidUserResponseTImer:1;

   // Padding out 7 bits
   UINT8 mReserved1:7;

   UINT8 mInvokeID;
   eQMILOCDataCodingScheme mDataCodingScheme;
   UINT8 mNotificationTextLength;

   // This array must be the size specified by mNotificationTextLength
   // UINT8 mNotificationText[1];
};

struct sLOCProvideNIUserResponseRequest_UMTSControlPlaneRequest2
{
   UINT8 mClientAddressLength;

   // This array must be the size specified by mClientAddressLength
   // UINT8 mClientAddress[1];
};

struct sLOCProvideNIUserResponseRequest_UMTSControlPlaneRequest3
{
   eQMILOCLocationType mLocationType;
   eQMILOCDataCodingScheme mRequestorIDDataCodingScheme;
   UINT8 mRequestorIDCodedStingLength;

   // This array must be the size specified by mRequestorIDCodedStingLength
   // UINT8 mRequestorIDCodedString[1];
};

struct sLOCProvideNIUserResponseRequest_UMTSControlPlaneRequest4
{
   eQMILOCDataCodingScheme mCodewordStringDataCodingScheme;
   UINT8 mCodewordStringCodedStringLength;

   // This array must be the size specified by mCodewordStringCodedStringLength
   // UINT8 mCodewordStringCodedString[1];
};

struct sLOCProvideNIUserResponseRequest_UMTSControlPlaneRequest5
{
   UINT8 mServiceTypeID;
   UINT16 mUserResponseTimerSeconds;
};

struct sLOCProvideNIUserResponseRequest_UMTSControlPlaneRequest
{
   sLOCProvideNIUserResponseRequest_UMTSControlPlaneRequest1 mLOCProvideNIUserResponseRequest_UMTSControlPlaneRequest1;
   sLOCProvideNIUserResponseRequest_UMTSControlPlaneRequest2 mLOCProvideNIUserResponseRequest_UMTSControlPlaneRequest2;
   sLOCProvideNIUserResponseRequest_UMTSControlPlaneRequest3 mLOCProvideNIUserResponseRequest_UMTSControlPlaneRequest3;
   sLOCProvideNIUserResponseRequest_UMTSControlPlaneRequest4 mLOCProvideNIUserResponseRequest_UMTSControlPlaneRequest4;
   sLOCProvideNIUserResponseRequest_UMTSControlPlaneRequest5 mLOCProvideNIUserResponseRequest_UMTSControlPlaneRequest5;
};

// Structure to describe request TLV 0x13 for LOCProvideNIUserResponse()
struct sLOCProvideNIUserResponseRequest_ServiceInteractionRequest
{
   INT8 mPositionQoSIncluded;
   UINT8 mPositionQoSTimeoutSeconds;
   UINT32 mMaxNumberOfFixes;
   UINT32 mTimeBetweenFixesSeconds;
   eQMILOCPositionMode mPosistionMode;
   eQMILOCEncodingScheme mEncodingScheme;
   UINT8 mRequestorIDLength;
   UINT8 mRequestorID;
   UINT16 mUserResponseTimerSeconds;
   eQMILOCServiceInteractionType mServiceInteractionType;
};

// Structure to describe request TLV 0x14 for LOCProvideNIUserResponse()
struct sLOCProvideNIUserResponseRequest_SUPLVersion2Extension
{
   bool mWLAN:1;
   bool mGSM:1;
   bool mWCDMA:1;
   bool mCDMA:1;
   bool mHRDP:1;
   bool mUMB:1;
   bool mLTE:1;
   bool mWIMAX:1;
   bool mHISTORIC:1;
   bool mNONSVRV:1;

   // Padding out 6 bits
   UINT8 mReserved1:6;

   eQMILOCTriggerType mTriggerType;
   bool mGPS:1;
   bool mGLONASS:1;
   bool mGALILEO:1;
   bool mSBAS:1;
   bool mQZSS:1;
   bool mMODERNGPS:1;

   // Padding out 10 bits
   UINT8 mReserved2:2;
   UINT8 mReserved3;
};

// Structure to describe request TLV 0x01 for LOCInjectPredictedOrbitsData()
struct sLOCInjectPredictedOrbitsDataRequest_TotalSize
{
   UINT32 mTotalSize;
};

// Structure to describe request TLV 0x03 for LOCInjectPredictedOrbitsData()
struct sLOCInjectPredictedOrbitsDataRequest_PartNumber
{
   UINT16 mPartNumber;
};

// Structure to describe request TLV 0x04 for LOCInjectPredictedOrbitsData()
struct sLOCInjectPredictedOrbitsDataRequest_PartData
{
   UINT16 mPartDataLength;

   // This array must be the size specified by mPartDataLength
   // UINT8 mPartData[1];
};

// Structure to describe request TLV 0x10 for LOCInjectPredictedOrbitsData()
struct sLOCInjectPredictedOrbitsDataRequest_FormatType
{
   eQMILOCOrbitsFormatType mOrbitsFormatType;
};

// Structure to describe indication TLV 0x10 for LOC InjectPredictedOrbitsDataIndication
struct sLOCInjectPredictedOrbitsDataIndication_PartNumber
{
   UINT16 mPartNumber;
};

// Structure to describe indication TLV 0x10 for LOC GetPredictedOrbitsDataSourceIndication
struct sLOCGetPredictedOrbitsDataSourceIndication_AllowedSizes
{
   UINT32 mMaximumFileSizeBytes;
   UINT32 mMaximumPartSizeBytes;
};

// Structure to describe indication TLV 0x11 for LOC GetPredictedOrbitsDataSourceIndication
struct sLOCGetPredictedOrbitsDataSourceIndication_ServerList
{
   UINT8 mServerListLength;

   struct sServer
   {
      UINT8 mServerURLLength;
   
      // This array must be the size specified by mServerURLLength
      // char mServerURL[1];
   };

   // This array must be the size specified by mServerListLength
   // sServer mServers[1];
};

// Structure to describe indication TLV 0x10 for LOC GetPredictedOrbitsDataValidityIndication
struct sLOCGetPredictedOrbitsDataValidityIndication_ValidityInfo
{
   UINT64 mStartTimeInUTC;
   UINT16 mDurationHours;
};

// Structure to describe request TLV 0x01 for LOCInjectUTCTime()
struct sLOCInjectUTCTimeRequest_UTCTime
{
   UINT64 mUTCTimestampMilliseconds;
};

// Structure to describe request TLV 0x10 for LOCInjectPosition()
struct sLOCInjectPositionRequest_Latitude
{
   double mLatitudeDegrees;
};

// Structure to describe request TLV 0x11 for LOCInjectPosition()
struct sLOCInjectPositionRequest_Longitude
{
   double mLongitudeDegrees;
};

// Structure to describe request TLV 0x12 for LOCInjectPosition()
struct sLOCInjectPositionRequest_HorizontalUncertaintyCircular
{
   float mHorizontalUncertaintyCircularMeters;
};

// Structure to describe request TLV 0x13 for LOCInjectPosition()
struct sLOCInjectPositionRequest_HorizontalConfidence
{
   UINT8 mHorizontalConfidencePercent;
};

// Structure to describe request TLV 0x14 for LOCInjectPosition()
struct sLOCInjectPositionRequest_HorizontalReliability
{
   eQMILOCReliability mHorizontalReliability;
};

// Structure to describe request TLV 0x15 for LOCInjectPosition()
struct sLOCInjectPositionRequest_AltitudeFromEllipsoid
{
   float mAltitudeFromEllipsoidMeters;
};

// Structure to describe request TLV 0x16 for LOCInjectPosition()
struct sLOCInjectPositionRequest_AltitudeFromSeaLevel
{
   float mAltitudeFromSeaLevelMeters;
};

// Structure to describe request TLV 0x17 for LOCInjectPosition()
struct sLOCInjectPositionRequest_VerticalUncertainty
{
   float mVerticalUncertaintyMeters;
};

// Structure to describe request TLV 0x18 for LOCInjectPosition()
struct sLOCInjectPositionRequest_VerticalConfidence
{
   UINT8 mVerticalConfidencePercent;
};

// Structure to describe request TLV 0x19 for LOCInjectPosition()
struct sLOCInjectPositionRequest_VerticalReliability
{
   eQMILOCReliability mVerticalReliability;
};

// Structure to describe request TLV 0x1A for LOCInjectPosition()
struct sLOCInjectPositionRequest_AltitudeSource
{
   eQMILOCAltitudeSource mAltitudeSource;
   eQMILOCLinkage mLinkage;
   eQMILOCCoverage mCoverage;
};

// Structure to describe request TLV 0x1B for LOCInjectPosition()
struct sLOCInjectPositionRequest_UTCTimestamp
{
   UINT64 mUTCTimestampMilliseconds;
};

// Structure to describe request TLV 0x1C for LOCInjectPosition()
struct sLOCInjectPositionRequest_PositionAge
{
   UINT32 mAgeTimestampMilliseconds;
};

// Structure to describe request TLV 0x1D for LOCInjectPosition()
struct sLOCInjectPositionRequest_PositionSource
{
   eQMILOCPositionSource mPositionSource;
};

// Structure to describe request TLV 0x01 for LOCSetEngineLock()
struct sLOCSetEngineLockRequest_LockType
{
   eQMILOCLockType mLockType;
};

// Structure to describe indication TLV 0x10 for LOC GetEngineLockIndication
struct sLOCGetEngineLockIndication_LockType
{
   eQMILOCLockType mLockType;
};

// Structure to describe request TLV 0x01 for LOCSetSBASConfig()
struct sLOCSetSBASConfigRequest_SBASConfig
{
   INT8 mSBASEnabled;
};

// Structure to describe response TLV 0x10 for LOCGetSBASConfig()
struct sLOCGetSBASConfigResponse_SBASConfig
{
   INT8 mSBASEnabled;
};

// Structure to describe request TLV 0x01 for LOCSetNMEATypes()
struct sLOCSetNMEATypesRequest_SentenceTypes
{
   bool mGGASentence:1;
   bool mRMCSentence:1;
   bool mGSVSentence:1;
   bool mGSASentence:1;
   bool mVTGSentence:1;
   bool mPQXFISentence:1;
   bool mPSTISSentence:1;

   // Padding out 25 bits
   UINT8 mReserved1:1;
   UINT8 mReserved2[3];
};

// Structure to describe indication TLV 0x10 for LOC GetNMEATypesIndication
struct sLOCGetNMEATypesIndication_SentenceType
{
   bool mGGASentence:1;
   bool mRMCSentence:1;
   bool mGSVSentence:1;
   bool mGSASentence:1;
   bool mVTGSentence:1;
   bool mPQXFISentence:1;
   bool mPSTISSentence:1;

   // Padding out 25 bits
   UINT8 mReserved1:1;
   UINT8 mReserved2[3];
};

// Structure to describe request TLV 0x01 for LOCSetLowPowerMode()
struct sLOCSetLowPowerModeRequest_EnableLPM
{
   INT8 mEnableLowPowerMode;
};

// Structure to describe indication TLV 0x10 for LOC GetLowPowerModeIndication
struct sLOCGetLowPowerModeIndication_EnableLPM
{
   INT8 mEnableLowPowerMode;
};

// Structure to describe request TLV 0x01 for LOCSetLocationServer()
struct sLOCSetLocationServerRequest_ServerType
{
   eQMILOCLocationServerType mServerType;
};

// Structure to describe request TLV 0x10 for LOCSetLocationServer()
struct sLOCSetLocationServerRequest_IPv4Address
{
   UINT32 mIPv4Address;
   UINT16 mIPv4Port;
};

// Structure to describe request TLV 0x11 for LOCSetLocationServer()
struct sLOCSetLocationServerRequest_IPv6Address
{
   UINT8 mIPv6Address[16];
   UINT32 mIPv6Port;
};

// Structure to describe request TLV 0x12 for LOCSetLocationServer()
struct sLOCSetLocationServerRequest_URLAddress
{
   // String is variable length, but must be size of the container
   // char mURLAddress[1];
};

// Structure to describe request TLV 0x01 for LOCGetLocationServer()
struct sLOCGetLocationServerRequest_ServerType
{
   eQMILOCLocationServerType mServerType;
};

// Structure to describe request TLV 0x10 for LOCGetLocationServer()
struct sLOCGetLocationServerRequest_AddressType
{
   bool mIPv4:1;
   bool mIPv6:1;
   bool mURL:1;

   // Padding out 5 bits
   UINT8 mReserved1:5;
};

// Structure to describe indication TLV 0x10 for LOC GetLocationServerIndication
struct sLOCGetLocationServerIndication_IPv4Address
{
   UINT32 mIPv4Address;
   UINT16 mIPv4Port;
};

// Structure to describe indication TLV 0x11 for LOC GetLocationServerIndication
struct sLOCGetLocationServerIndication_IPv6Address
{
   UINT8 mIPv6Address[16];
   UINT32 mIPv6Port;
};

// Structure to describe indication TLV 0x12 for LOC GetLocationServerIndication
struct sLOCGetLocationServerIndication_URLAddress
{
   // String is variable length, but must be size of the container
   // char mURLAddress[1];
};

// Structure to describe request TLV 0x01 for LOCDeleteAssistData()
struct sLOCDeleteAssistDataRequest_DeleteAll
{
   INT8 mDeleteAll;
};

// Structure to describe request TLV 0x10 for LOCDeleteAssistData()
struct sLOCDeleteAssistDataRequest_DeleteSatelliteInfo
{
   UINT8 mSatelliteInfoCount;

   struct sSatelliteInfo
   {
      UINT16 mGNSSSatelliteID;
      eQMILOCSystem mSystem;
      bool mDeleteEphemeris:1;
      bool mDeleteAlmanac:1;
   
      // Padding out 6 bits
      UINT8 mReserved1:6;
   };

   // This array must be the size specified by mSatelliteInfoCount
   // sSatelliteInfo mSatelliteInfos[1];
};

// Structure to describe request TLV 0x11 for LOCDeleteAssistData()
struct sLOCDeleteAssistDataRequest_DeleteGNSData
{
   bool mDeleteGPSSatelliteDirectory:1;
   bool mDeleteGPSSatelliteSteering:1;
   bool mDeleteGPSTime:1;
   bool mDeleteGPSAlmanacCorrection:1;
   bool mDeleteGLOSatelliteDirectory:1;
   bool mDeleteGLOSatelliteSteering:1;
   bool mDeleteGLOTime:1;
   bool mDeleteGLOAlmanacCorrection:1;
   bool mDeleteSBASSatelliteDirectory:1;
   bool mDeleteSBASSatelliteSteering:1;
   bool mDeletePosition:1;
   bool mDeleteTime:1;
   bool mDeleteIONO:1;
   bool mDeleteUTCTimestamp:1;
   bool mDeleteHealth:1;
   bool mDeleteSAData:1;
   bool mDeleteRTI:1;
   bool mDeleteSatelliteNoExist:1;
   bool mDeleteFrequencyBiasEstimate:1;

   // Padding out 45 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2[5];
};

// Structure to describe request TLV 0x12 for LOCDeleteAssistData()
struct sLOCDeleteAssistDataRequest_DeleteCellDatabase
{
   bool mDeletePosition:1;
   bool mDeleteLatestGPSPosition:1;
   bool mDeleteOTAPosition:1;
   bool mDeleteEXTReferencePosition:1;
   bool mDeleteTimeTag:1;
   bool mDeleteCellID:1;
   bool mDeleteCachedCellID:1;
   bool mDeleteLastServerCell:1;
   bool mDeleteCurrentServerCell:1;
   bool mDeleteNeighborInfo:1;

   // Padding out 22 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[2];
};

// Structure to describe request TLV 0x13 for LOCDeleteAssistData()
struct sLOCDeleteAssistDataRequest_DeleteClockInfo
{
   bool mDeleteTimeEstimate:1;
   bool mDeleteFrequencyEstimate:1;
   bool mDeleteWeekNumber:1;
   bool mDeleteRTCTime:1;
   bool mDeleteTimeTransfer:1;
   bool mDeleteGPSTimeEstimate:1;
   bool mDeleteGLOTimeEstimate:1;
   bool mDeleteGLODayNumber:1;
   bool mDeleteGLOYearNumber:1;
   bool mDeleteGLORFGroupDelay:1;
   bool mDeleteDisableTT:1;

   // Padding out 21 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2[2];
};

// Structure to describe request TLV 0x01 for LOCSetXTRATSessionControl()
struct sLOCSetXTRATSessionControlRequest_EnableXTRAT
{
   INT8 mEnableXTRAT;
};

// Structure to describe indication TLV 0x10 for LOC GetXTRATSessionControlIndication
struct sLOCGetXTRATSessionControlIndication_EnableXTRAT
{
   INT8 mEnableXTRAT;
};

// Structure to describe request TLV 0x10 for LOCInjectWiFiPosition()
struct sLOCInjectWiFiPositionRequest_Time
{
   UINT32 mWiFiPositionTime;
};

// Structure to describe request TLV 0x11 for LOCInjectWiFiPosition()
struct sLOCInjectWiFiPositionRequest_WiFiPosition
{
   double mLatitudeDegrees;
   double mLongitudeDegrees;
   UINT16 mHEPEMeters;
   UINT8 mNumberOfAccessPointsUsed;
   eQMILOCWiFiFixErrorCode mFixErrorCode;
};

// Structure to describe request TLV 0x12 for LOCInjectWiFiPosition()
struct sLOCInjectWiFiPositionRequest_AccessPointInformation
{
   UINT8 mNumberOfAccessPoints;

   struct sAccessPointInfo
   {
      UINT8 mMACAddress[6];
      INT32 mRSSIdBm;
      UINT16 mChannel;
      bool mBeingUsed:1;
      bool mHiddenSSID:1;
      bool mPrivate:1;
      bool mInfrastructureMode:1;
   
      // Padding out 4 bits
      UINT8 mReserved1:4;
   };

   // This array must be the size specified by mNumberOfAccessPoints
   // sAccessPointInfo mAccessPointInfos[1];
};

// Structure to describe request TLV 0x13 for LOCInjectWiFiPosition()
struct sLOCInjectWiFiPositionRequest_HorizontalReliability
{
   eQMILOCReliability mHorizontalReliability;
};

// Structure to describe request TLV 0x01 for LOCProvideWiFiStatus()
struct sLOCProvideWiFiStatusRequest_WiFiStatus
{
   eQMILOCWiFiStatus mWiFiStatus;
};

// Structure to describe indication TLV 0x10 for LOC GetRegisteredEventsIndication
struct sLOCGetRegisteredEventsIndication_RegistrationMask
{
   bool mPositionReport:1;
   bool mGNSSSatelliteInfo:1;
   bool mNMEA:1;
   bool mNINotifyVerifyRequest:1;
   bool mInjectTimeRequest:1;
   bool mInjectPredictedOrbitsRequest:1;
   bool mInjectPositionRequest:1;
   bool mEngineState:1;
   bool mFixSessionState:1;
   bool mWiFiRequest:1;
   bool mSensorStreamingReadyStatus:1;
   bool mTimeSyncRequest:1;
   bool mSetSPIStreamingReport:1;
   bool mLocationServerConnectionRequest:1;
   bool mNIGeofenceNotification:1;
   bool mGeofenceGeneralAlert:1;
   bool mGeofenceBreachNotification:1;

   // Padding out 47 bits
   UINT8 mReserved1:7;
   UINT8 mReserved2[5];
};

// Structure to describe request TLV 0x01 for LOCSetOperationMode()
struct sLOCSetOperationModeRequest_OperationMode
{
   eQMILOCOperationMode mOperationMode;
};

// Structure to describe indication TLV 0x10 for LOC GetOperationModeIndication
struct sLOCGetOperationModeIndication_OperationMode
{
   eQMILOCOperationMode mOperationMode;
};

// Structure to describe request TLV 0x01 for LOCSetSPIStatus()
struct sLOCSetSPIStatusRequest_StationaryStatus
{
   INT8 mDeviceIsStationary;
};

// Structure to describe request TLV 0x10 for LOCSetSPIStatus()
struct sLOCSetSPIStatusRequest_Confidence
{
   UINT8 mStationaryConfidence;
};

// Structure to describe request TLV 0x10 for LOCInjectSensorData()
struct sLOCInjectSensorDataRequest_OpaqueIdentifier
{
   UINT32 mOpaqueIdentifier;
};

// Structure to describe request TLV 0x11 for LOCInjectSensorData()
struct sLOCInjectSensorDataRequest_AccelerometerData
{
   UINT32 mTimeOfFirstSampleMilliseconds;
   bool mSignReversal:1;
   bool mSensorTimeIsModemTime:1;

   // Padding out 6 bits
   UINT8 mReserved1:6;

   UINT8 mSensorDataLength;

   struct sSensorData
   {
      UINT16 mTimeOffsetMilliseconds;
      float mXAxis;
      float mYAxis;
      float mZAxis;
   };

   // This array must be the size specified by mSensorDataLength
   // sSensorData mSensorDatas[1];
};

// Structure to describe request TLV 0x12 for LOCInjectSensorData()
struct sLOCInjectSensorDataRequest_GyrometerData
{
   UINT32 mTimeOfFirstSampleMilliseconds;
   bool mSignReversal:1;
   bool mSensorTimeIsModemTime:1;

   // Padding out 6 bits
   UINT8 mReserved1:6;

   UINT8 mSensorDataLength;

   struct sSensorData
   {
      UINT16 mTimeOffsetMilliseconds;
      float mXAxis;
      float mYAxis;
      float mZAxis;
   };

   // This array must be the size specified by mSensorDataLength
   // sSensorData mSensorDatas[1];
};

// Structure to describe indication TLV 0x10 for LOC InjectSensorDataIndication
struct sLOCInjectSensorDataIndication_OpaqueIdentifier
{
   UINT32 mOpaqueIdentifier;
};

// Structure to describe indication TLV 0x11 for LOC InjectSensorDataIndication
struct sLOCInjectSensorDataIndication_AccelerometerSampleAccepted
{
   INT8 mAccelerometerSampleAccepted;
};

// Structure to describe indication TLV 0x12 for LOC InjectSensorDataIndication
struct sLOCInjectSensorDataIndication_GyrometerSamplesAccepted
{
   INT8 mGyrometerSamplesAccepted;
};

// Structure to describe request TLV 0x01 for LOCInjectTimeSyncData()
struct sLOCInjectTimeSyncDataRequest_ReferenceCounter
{
   UINT32 mReferenceCounter;
};

// Structure to describe request TLV 0x03 for LOCInjectTimeSyncData()
struct sLOCInjectTimeSyncDataRequest_SensorTransmitTime
{
   UINT32 mProcessTXTimeMilliseconds;
};

// Structure to describe request TLV 0x01 for LOCSetCradleMountConfig()
struct sLOCSetCradleMountConfigRequest_State
{
   eQMILOCCradleMountState mCradleMountState;
};

// Structure to describe request TLV 0x10 for LOCSetCradleMountConfig()
struct sLOCSetCradleMountConfigRequest_Confidence
{
   UINT8 mCradleMountConfidence;
};

// Structure to describe indication TLV 0x10 for LOC GetCradleMountConfigIndication
struct sLOCGetCradleMountConfigIndication_State
{
   eQMILOCCradleMountState mCradleMountState;
};

// Structure to describe indication TLV 0x11 for LOC GetCradleMountConfigIndication
struct sLOCGetCradleMountConfigIndication_Confidence
{
   UINT8 mCradleMountConfidence;
};

// Structure to describe request TLV 0x01 for LOCSetExternalPowerConfig()
struct sLOCSetExternalPowerConfigRequest_PowerState
{
   eQMILOCPowerState mPowerState;
};

// Structure to describe indication TLV 0x10 for LOC GetExternalPowerConfigIndication
struct sLOCGetExternalPowerConfigIndication_PowerState
{
   eQMILOCPowerState mPowerState;
};

// Structure to describe request TLV 0x01 for LOCProvideConnectionStatus()
struct sLOCProvideConnectionStatusRequest_ConnectionHandle
{
   UINT32 mConnectionHandle;
};

// Structure to describe request TLV 0x03 for LOCProvideConnectionStatus()
struct sLOCProvideConnectionStatusRequest_ConnectionStatus
{
   eQMILOCConnectionStatus mConnectionStatus;
};

// Structure to describe request TLV 0x10 for LOCProvideConnectionStatus()
struct sLOCProvideConnectionStatusRequest_APNProfile
{
   eQMILOCPDNType mPDNType;
   UINT8 mAPNNameLength;

   // This array must be the size specified by mAPNNameLength
   // char mAPNName[1];
};

// Structure to describe request TLV 0x10 for LOCSetProtocolConfigParameters()
struct sLOCSetProtocolConfigParametersRequest_SUPLSecurity
{
   INT8 mSUPLSecurityEnabled;
};

// Structure to describe request TLV 0x11 for LOCSetProtocolConfigParameters()
struct sLOCSetProtocolConfigParametersRequest_VXVersion
{
   eQMILOCVXVersion mVXVersion;
};

// Structure to describe request TLV 0x12 for LOCSetProtocolConfigParameters()
struct sLOCSetProtocolConfigParametersRequest_SUPLVersion
{
   eQMILOCSUPLVersion mSUPLVersion;
};

// Structure to describe request TLV 0x13 for LOCSetProtocolConfigParameters()
struct sLOCSetProtocolConfigParametersRequest_LPPConfiguration
{
   bool mEnableUserPlane:1;
   bool mEnableControlPlane:1;

   // Padding out 30 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[3];
};

// Structure to describe indication TLV 0x10 for LOC SetProtocolConfigParametersIndication
struct sLOCSetProtocolConfigParametersIndication_FailedParameters
{
   bool mSUPLSecurity:1;
   bool mVXVersion:1;
   bool mSUPLVersion:1;

   // Padding out 61 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2[7];
};

// Structure to describe request TLV 0x01 for LOCGetProtocolConfigParameters()
struct sLOCGetProtocolConfigParametersRequest_ConfigParameters
{
   bool mSUPLSecurity:1;
   bool mVXVersion:1;
   bool mSUPLVersion:1;

   // Padding out 61 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2[7];
};

// Structure to describe indication TLV 0x10 for LOC GetProtocolConfigParametersIndication
struct sLOCGetProtocolConfigParametersIndication_SUPLSecurity
{
   INT8 mSUPLSecurityEnabled;
};

// Structure to describe indication TLV 0x11 for LOC GetProtocolConfigParametersIndication
struct sLOCGetProtocolConfigParametersIndication_VXVersion
{
   eQMILOCVXVersion mVXVersion;
};

// Structure to describe indication TLV 0x12 for LOC GetProtocolConfigParametersIndication
struct sLOCGetProtocolConfigParametersIndication_SUPLVersion
{
   eQMILOCSUPLVersion mSUPLVersion;
};

// Structure to describe indication TLV 0x13 for LOC GetProtocolConfigParametersIndication
struct sLOCGetProtocolConfigParametersIndication_LPPConfiguration
{
   bool mEnableUserPlane:1;
   bool mEnableControlPlane:1;

   // Padding out 30 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[3];
};

// Structure to describe request TLV 0x10 for LOCSetSensorControlConfig()
struct sLOCSetSensorControlConfigRequest_SensorUsage
{
   eQMILOCSensorUsage mSensorUsage;
};

// Structure to describe indication TLV 0x10 for LOC GetSensorControlConfigIndication
struct sLOCGetSensorControlConfigIndication_SensorUsage
{
   eQMILOCSensorUsage mSensorUsage;
};

// Structure to describe request TLV 0x10 for LOCSetSensorProperties()
struct sLOCSetSensorPropertiesRequest_GyroBiasVariance
{
   float mGyroBiasVariance;
};

// Structure to describe request TLV 0x11 for LOCSetSensorProperties()
struct sLOCSetSensorPropertiesRequest_VelocityRWSD
{
   float mVelocityRandomWalkSpectralDensity;
};

// Structure to describe request TLV 0x12 for LOCSetSensorProperties()
struct sLOCSetSensorPropertiesRequest_AccelerationRWSD
{
   float mAccelerationRandomWalkSpectralDensity;
};

// Structure to describe request TLV 0x13 for LOCSetSensorProperties()
struct sLOCSetSensorPropertiesRequest_AngleRWSD
{
   float mAngleRandomWalkSpectralDensity;
};

// Structure to describe request TLV 0x14 for LOCSetSensorProperties()
struct sLOCSetSensorPropertiesRequest_RateRWSD
{
   float mRateRandomWalkSpectralDensity;
};

// Structure to describe indication TLV 0x10 for LOC SetSensorPropertiesIndication
struct sLOCSetSensorPropertiesIndication_Failures
{
   bool mGyroBiasVariance:1;
   bool mVelocityRandomWalkSpectralDensity:1;
   bool mAccelerationRandomWalkSpectralDensity:1;
   bool mAngleRandomWalkSpectralDensity:1;
   bool mRateRandomWalkSpectralDensity:1;

   // Padding out 27 bits
   UINT8 mReserved1:3;
   UINT8 mReserved2[3];
};

// Structure to describe request TLV 0x01 for LOCGetSensorProperties()
struct sLOCGetSensorPropertiesRequest_Properties
{
   bool mGyroBiasVariance:1;
   bool mVelocityRandomWalkSpectralDensity:1;
   bool mAccelerationRandomWalkSpectralDensity:1;
   bool mAngleRandomWalkSpectralDensity:1;
   bool mRateRandomWalkSpectralDensity:1;

   // Padding out 27 bits
   UINT8 mReserved1:3;
   UINT8 mReserved2[3];
};

// Structure to describe indication TLV 0x10 for LOC GetSensorPropertiesIndication
struct sLOCGetSensorPropertiesIndication_GyroBiasVariance
{
   float mGyroBiasVariance;
};

// Structure to describe indication TLV 0x11 for LOC GetSensorPropertiesIndication
struct sLOCGetSensorPropertiesIndication_VelocityRWSD
{
   float mVelocityRandomWalkSpectralDensity;
};

// Structure to describe indication TLV 0x12 for LOC GetSensorPropertiesIndication
struct sLOCGetSensorPropertiesIndication_AccelerationRWSD
{
   float mAccelerationRandomWalkSpectralDensity;
};

// Structure to describe indication TLV 0x13 for LOC GetSensorPropertiesIndication
struct sLOCGetSensorPropertiesIndication_AngleRWSD
{
   float mAngleRandomWalkSpectralDensity;
};

// Structure to describe indication TLV 0x14 for LOC GetSensorPropertiesIndication
struct sLOCGetSensorPropertiesIndication_RateRWSD
{
   float mRateRandomWalkSpectralDensity;
};

// Structure to describe request TLV 0x10 for LOCSetSensorPerformanceConfig()
struct sLOCSetSensorPerformanceConfigRequest_ControlMode
{
   eQMILOCControlMode mControlMode;
};

// Structure to describe request TLV 0x11 for LOCSetSensorPerformanceConfig()
struct sLOCSetSensorPerformanceConfigRequest_AccelerometerSampling
{
   UINT16 mSamplesPerBatch;
   UINT16 mBatchesPerSecond;
};

// Structure to describe request TLV 0x12 for LOCSetSensorPerformanceConfig()
struct sLOCSetSensorPerformanceConfigRequest_GyrometerSampling
{
   UINT16 mSamplesPerBatch;
   UINT16 mBatchesPerSecond;
};

// Structure to describe request TLV 0x13 for LOCSetSensorPerformanceConfig()
struct sLOCSetSensorPerformanceConfigRequest_AlgorithmConfig
{
   bool mDisableINSPositioningFilter:1;

   // Padding out 31 bits
   UINT8 mReserved1:7;
   UINT8 mReserved2[3];
};

// Structure to describe indication TLV 0x10 for LOC SetSensorPerformanceConfigIndication
struct sLOCSetSensorPerformanceConfigIndication_FailedConfiguration
{
   bool mPerformanceMode:1;
   bool mAccelerometerSampling:1;
   bool mGyrometerSampling:1;
   bool mAlgorithmConfig:1;

   // Padding out 28 bits
   UINT8 mReserved1:4;
   UINT8 mReserved2[3];
};

// Structure to describe indication TLV 0x10 for LOC GetSensorPerformanceConfigIndication
struct sLOCGetSensorPerformanceConfigIndication_ControlMode
{
   eQMILOCControlMode mControlMode;
};

// Structure to describe indication TLV 0x11 for LOC GetSensorPerformanceConfigIndication
struct sLOCGetSensorPerformanceConfigIndication_AccelerometerSampling
{
   UINT16 mSamplesPerBatch;
   UINT16 mBatchesPerSecond;
};

// Structure to describe indication TLV 0x12 for LOC GetSensorPerformanceConfigIndication
struct sLOCGetSensorPerformanceConfigIndication_GyrometerSampling
{
   UINT16 mSamplesPerBatch;
   UINT16 mBatchesPerSecond;
};

// Structure to describe indication TLV 0x13 for LOC GetSensorPerformanceConfigIndication
struct sLOCGetSensorPerformanceConfigIndication_AlgorithmConfig
{
   bool mDisableINSPositioningFilter:1;

   // Padding out 31 bits
   UINT8 mReserved1:7;
   UINT8 mReserved2[3];
};

// Structure to describe request TLV 0x01 for LOCInjectSUPLCertificate()
struct sLOCInjectSUPLCertificateRequest_ID
{
   UINT8 mSUPLCertificateID;
};

// Structure to describe request TLV 0x10 for LOCDeleteSUPLCertificate()
struct sLOCDeleteSUPLCertificateRequest_ID
{
   UINT8 mSUPLCertificateID;
};

// Structure to describe request TLV 0x10 for LOCSetPositionEngineConfig()
struct sLOCSetPositionEngineConfigRequest_InjectedPosition
{
   INT8 mUseInjectedPositionInCalculations;
};

// Structure to describe request TLV 0x11 for LOCSetPositionEngineConfig()
struct sLOCSetPositionEngineConfigRequest_FilterSVUsage
{
   INT8 mFilterUsageOfSVs;
};

// Structure to describe request TLV 0x12 for LOCSetPositionEngineConfig()
struct sLOCSetPositionEngineConfigRequest_StoreAssistData
{
   INT8 mStoreAssistanceData;
};

// Structure to describe indication TLV 0x10 for LOC SetPositionEngineConfigIndication
struct sLOCSetPositionEngineConfigIndication_FailedParameters
{
   bool mInjectedPosition:1;
   bool mFilterSVUsage:1;
   bool mStoreAssistData:1;

   // Padding out 29 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2[3];
};

// Structure to describe request TLV 0x01 for LOCGetPositionEngineConfig()
struct sLOCGetPositionEngineConfigRequest_Parameters
{
   bool mInjectedPosition:1;
   bool mFilterSVUsage:1;
   bool mStoreAssistData:1;

   // Padding out 29 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2[3];
};

// Structure to describe indication TLV 0x10 for LOC GetPositionEngineConfigIndication
struct sLOCGetPositionEngineConfigIndication_InjectedPosition
{
   INT8 mUseInjectedPositionInCalculations;
};

// Structure to describe indication TLV 0x11 for LOC GetPositionEngineConfigIndication
struct sLOCGetPositionEngineConfigIndication_FilterSVUsage
{
   INT8 mFilterUsageOfSVs;
};

// Structure to describe indication TLV 0x12 for LOC GetPositionEngineConfigIndication
struct sLOCGetPositionEngineConfigIndication_StoreAssistData
{
   INT8 mStoreAssistanceData;
};

// Structure to describe indication TLV 0x01 for LOC NetworkInitiatedGeofenceIndication
struct sLOCNetworkInitiatedGeofenceIndication_GeofenceID
{
   UINT32 mGeofenceID;
};

// Structure to describe indication TLV 0x01 for LOC EventGeofenceGeneralAlertIndication
struct sLOCEventGeofenceGeneralAlertIndication_GeofenceGeneralAlert
{
   eQMILOCGeofenceGeneralAlert mGeofenceGeneralAlert;
};

// Structure to describe indication TLV 0x01 for LOC EventGeofenceBreachIndication
struct sLOCEventGeofenceBreachIndication_GeofenceID
{
   UINT32 mGeofenceID;
};

// Structure to describe indication TLV 0x10 for LOC EventGeofenceBreachIndication
struct sLOCEventGeofenceBreachIndication_GeofencePosition
{
   UINT64 mUTCTimestampMilliseconds;
   double mLatitudeDegrees;
   double mLongitudeDegrees;
   float mHorizontalUncertaintyEllipticalMinorMeters;
   float mHorizontalUncertaintyEllipticalMajorMeters;
   float mHorizontalUncertaintyEllipticalAzimuthDecimalDegrees;
   INT8 mHorizontalSpeedValid;
   float mHorizontalSpeedMetersSecond;
   INT8 mAltitudeEllipsoidValid;
   float mAltitudeFromEllipsoidMeters;
   INT8 mVerticalUncertaintyValid;
   float mVerticalUncertaintyMeters;
   INT8 mVerticalSpeedValid;
   float mVerticalSpeedMetersSecond;
   INT8 mHeadingValid;
   float mHeadingDegrees;
};

// Structure to describe request TLV 0x01 for LOCAddCircularGeofence()
struct sLOCAddCircularGeofenceRequest_TransactionID
{
   UINT32 mTransactionID;
};

// Structure to describe request TLV 0x03 for LOCAddCircularGeofence()
struct sLOCAddCircularGeofenceRequest_BreachEventMask
{
   bool mEnteringGeofence:1;
   bool mLeavingGeofence:1;

   // Padding out 6 bits
   UINT8 mReserved1:6;
};

// Structure to describe request TLV 0x04 for LOCAddCircularGeofence()
struct sLOCAddCircularGeofenceRequest_IncludePositionInBreachEvent
{
   INT8 mIncludePositionInBreachEvent;
};

// Structure to describe request TLV 0x10 for LOCAddCircularGeofence()
struct sLOCAddCircularGeofenceRequest_Responsiveness
{
   eQMILOCResponsiveness mResponsiveness;
};

// Structure to describe request TLV 0x11 for LOCAddCircularGeofence()
struct sLOCAddCircularGeofenceRequest_Confidence
{
   eQMILOCConfidence mConfidence;
};

// Structure to describe indication TLV 0x01 for LOC AddCircularGeofenceIndication
struct sLOCAddCircularGeofenceIndication_GeofenceStatus
{
   eQMILOCGeofenceStatus mGeofenceStatus;
};

// Structure to describe indication TLV 0x10 for LOC AddCircularGeofenceIndication
struct sLOCAddCircularGeofenceIndication_TransactionID
{
   UINT32 mTransactionID;
};

// Structure to describe indication TLV 0x11 for LOC AddCircularGeofenceIndication
struct sLOCAddCircularGeofenceIndication_GeofenceID
{
   UINT32 mGeofenceID;
};

// Structure to describe request TLV 0x01 for LOCDeleteGeofence()
struct sLOCDeleteGeofenceRequest_GeofenceID
{
   UINT32 mGeofenceID;
};

// Structure to describe indication TLV 0x01 for LOC DeleteGeofenceIndication
struct sLOCDeleteGeofenceIndication_GeofenceStatus
{
   eQMILOCGeofenceStatus mGeofenceStatus;
};

// Structure to describe indication TLV 0x10 for LOC DeleteGeofenceIndication
struct sLOCDeleteGeofenceIndication_GeofenceID
{
   UINT32 mGeofenceID;
};

// Structure to describe indication TLV 0x11 for LOC DeleteGeofenceIndication
struct sLOCDeleteGeofenceIndication_TransactionID
{
   UINT32 mTransactionID;
};

// Structure to describe request TLV 0x01 for LOCQueryGeofence()
struct sLOCQueryGeofenceRequest_GeofenceID
{
   UINT32 mGeofenceID;
};

// Structure to describe indication TLV 0x01 for LOC QueryGeofenceIndication
struct sLOCQueryGeofenceIndication_Status
{
   eQMILOCGeofenceStatus mGeofenceStatus;
};

// Structure to describe indication TLV 0x10 for LOC QueryGeofenceIndication
struct sLOCQueryGeofenceIndication_GeofenceID
{
   UINT32 mGeofenceID;
};

// Structure to describe indication TLV 0x11 for LOC QueryGeofenceIndication
struct sLOCQueryGeofenceIndication_TransactionID
{
   UINT32 mTransactionID;
};

// Structure to describe indication TLV 0x12 for LOC QueryGeofenceIndication
struct sLOCQueryGeofenceIndication_Origin
{
   eQMILOCGeofenceOrigin mGeofenceOrigin;
};

// Structure to describe indication TLV 0x13 for LOC QueryGeofenceIndication
struct sLOCQueryGeofenceIndication_PositionFromGeofence
{
   eQMILOCPositionFromGeofence mPositionFromGeofence;
};

// Structure to describe indication TLV 0x14 for LOC QueryGeofenceIndication
struct sLOCQueryGeofenceIndication_Parameters
{
   double mLatitudeDegrees;
   double mLongitudeDegrees;
   UINT32 mRadiusMeters;
};

// Structure to describe indication TLV 0x15 for LOC QueryGeofenceIndication
struct sLOCQueryGeofenceIndication_State
{
   eQMILOCGeofenceState mGeofenceState;
};

// Structure to describe request TLV 0x01 for LOCEditGeofence()
struct sLOCEditGeofenceRequest_GeofenceID
{
   UINT32 mGeofenceID;
};

// Structure to describe request TLV 0x10 for LOCEditGeofence()
struct sLOCEditGeofenceRequest_State
{
   eQMILOCGeofenceState mGeofenceState;
};

// Structure to describe request TLV 0x11 for LOCEditGeofence()
struct sLOCEditGeofenceRequest_BreachEventMask
{
   bool mEnteringGeofence:1;
   bool mLeavingGeofence:1;

   // Padding out 6 bits
   UINT8 mReserved1:6;
};

// Structure to describe indication TLV 0x01 for LOC EditGeofenceIndication
struct sLOCEditGeofenceIndication_Status
{
   eQMILOCGeofenceStatus mGeofenceStatus;
};

// Structure to describe indication TLV 0x10 for LOC EditGeofenceIndication
struct sLOCEditGeofenceIndication_GeofenceID
{
   UINT32 mGeofenceID;
};

// Structure to describe indication TLV 0x11 for LOC EditGeofenceIndication
struct sLOCEditGeofenceIndication_TransactionID
{
   UINT32 mTransactionID;
};

// Structure to describe indication TLV 0x12 for LOC EditGeofenceIndication
struct sLOCEditGeofenceIndication_FailedParameters
{
   bool mGeofenceState:1;
   bool mBreachMask:1;

   // Padding out 30 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[3];
};

// Structure to describe request TLV 0x01 for LOCGetBestAvailablePosition()
struct sLOCGetBestAvailablePositionRequest_TransactionID
{
   UINT32 mTransactionID;
};

// Structure to describe indication TLV 0x01 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_Status
{
   eQMILOCGeofenceStatus mGeofenceStatus;
};

// Structure to describe indication TLV 0x10 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_TransactionID
{
   UINT32 mTransactionID;
};

// Structure to describe indication TLV 0x11 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_Latitude
{
   double mLatitudeDegrees;
};

// Structure to describe indication TLV 0x12 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_Longitude
{
   double mLongitudeDegrees;
};

// Structure to describe indication TLV 0x13 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_HorizontalUncertaintyCircular
{
   float mHorizontalUncertaintyCircularMeters;
};

// Structure to describe indication TLV 0x14 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_AltitudeFromEllipsoid
{
   float mAltitudeFromEllipsoidMeters;
};

// Structure to describe indication TLV 0x15 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_VerticalUncertainty
{
   float mVerticalUncertaintyMeters;
};

// Structure to describe indication TLV 0x16 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_UTCTimestamp
{
   UINT64 mUTCTimestampMilliseconds;
};

// Structure to describe indication TLV 0x17 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_TimeUncertainty
{
   float mTimeUncertaintyMilliseconds;
};

// Structure to describe indication TLV 0x18 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_HorizontalUncertaintyEllipticalMinor
{
   float mHorizontalUncertaintyEllipticalMinorMeters;
};

// Structure to describe indication TLV 0x19 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_HorizontalUncertaintyEllipticalMajor
{
   float mHorizontalUncertaintyEllipticalMajorMeters;
};

// Structure to describe indication TLV 0x1A for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_HorizontalUncertaintyEllipticalAzimuth
{
   float mHorizontalUncertaintyEllipticalAzimuthDecimalDegrees;
};

// Structure to describe indication TLV 0x1B for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_HorizontalConfidenceCircular
{
   UINT8 mHorizontalConfidencePercent;
};

// Structure to describe indication TLV 0x1C for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_HorizontalConfidenceElliptical
{
   UINT8 mHorizontalConfidencePercent;
};

// Structure to describe indication TLV 0x1D for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_HorizontalReliablility
{
   eQMILOCReliability mHorizontalReliability;
};

// Structure to describe indication TLV 0x1E for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_HorizontalSpeed
{
   float mHorizontalSpeedMetersSecond;
};

// Structure to describe indication TLV 0x1F for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_HorizontalSpeedUncertainty
{
   float mSpeedUncertaintyMetersSecond;
};

// Structure to describe indication TLV 0x20 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_AltitudeFromSeaLevel
{
   float mAltitudeFromSeaLevelMeters;
};

// Structure to describe indication TLV 0x21 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_VerticalConfidence
{
   UINT8 mVerticalConfidencePercent;
};

// Structure to describe indication TLV 0x22 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_VerticalReliability
{
   eQMILOCReliability mVerticalReliability;
};

// Structure to describe indication TLV 0x23 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_VerticalSpeed
{
   float mVerticalSpeedMetersSecond;
};

// Structure to describe indication TLV 0x24 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_VerticalSpeedUncertainty
{
   float mSpeedUncertaintyMetersSecond;
};

// Structure to describe indication TLV 0x25 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_Heading
{
   float mHeadingDegrees;
};

// Structure to describe indication TLV 0x26 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_HeadingUncertainty
{
   float mHeadingUncertaintyDegrees;
};

// Structure to describe indication TLV 0x27 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_MagneticDeviation
{
   float mMagneticDeviation;
};

// Structure to describe indication TLV 0x28 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_TechnologyUsed
{
   bool mSatellite:1;
   bool mCellular:1;
   bool mWiFi:1;
   bool mSensors:1;
   bool mReferenceLocation:1;

   // Padding out 27 bits
   UINT8 mReserved1:3;
   UINT8 mReserved2[3];
};

// Structure to describe indication TLV 0x29 for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_DilutionOfPrecision
{
   float mPositionDilutionOfPrecision;
   float mHorizontalDilutionOfPrecision;
   float mVerticalDilutionOfPrecision;
};

// Structure to describe indication TLV 0x2A for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_GPSTime
{
   UINT16 mGPSWeeks;
   UINT32 mGPSTimeOfWeekMilliseconds;
};

// Structure to describe indication TLV 0x2B for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_TimeSource
{
   eQMILOCTimeSource mTimeSource;
};

// Structure to describe indication TLV 0x2C for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_SensorDataUsage
{
   bool mAccelerometerUsed:1;
   bool mGyroUsed:1;

   // Padding out 30 bits
   UINT8 mReserved1:6;
   UINT8 mReserved2[3];

   bool mAidedHeading:1;
   bool mAidedSpeed:1;
   bool mAidedPosition:1;
   bool mAidedVelocity:1;

   // Padding out 28 bits
   UINT8 mReserved3:4;
   UINT8 mReserved4[3];
};

// Structure to describe indication TLV 0x2D for LOC GetBestAvailablePositionIndication
struct sLOCGetBestAvailablePositionIndication_SatellitesUsed
{
   UINT8 mSatellitesUsedCount;

   // This array must be the size specified by mSatellitesUsedCount
   // UINT16 mSatellitesUsed[1];
};

// Structure to describe request TLV 0x10 for CATSetEventReport()
struct sCATSetEventReportRequest_ReportMask
{
   bool mDisplayText:1;
   bool mGetInkey:1;
   bool mGetInput:1;
   bool mSetupMenu:1;
   bool mSelectItem:1;
   bool mSendSMSAlphaIdentifier:1;
   bool mSetupEventUserActivity:1;
   bool mSetupEventIdleScreenNotify:1;
   bool mSetupEventLanguageSelNotify:1;
   bool mSetupIdleModeText:1;
   bool mLanguageNotification:1;
   bool mRefresh:1;
   bool mEndProactiveSession:1;
   bool mPlayTone:1;
   bool mSetupCall:1;
   bool mSendDTMF:1;
   bool mLaunchBrowser:1;
   bool mSendSS:1;
   bool mSendUSSD:1;
   bool mProvideLocalInformationLanguage:1;
   bool mBearerIndependentProtocol:1;
   bool mSetupEventBrowserTermination:1;
   bool mProvideLocalInformationTime:1;
   bool mActivate:1;

   // Padding out 1 bits
   UINT8 mReserved1:1;

   bool mSetupEventHCIConnectivity:1;

   // Padding out 6 bits
   UINT8 mReserved2:6;
};

// Structure to describe request TLV 0x11 for CATSetEventReport()
struct sCATSetEventReportRequest_DecodeReportMask
{
   bool mDisplayText:1;
   bool mGetInkey:1;
   bool mGetInput:1;
   bool mSetupMenu:1;
   bool mSelectItem:1;
   bool mSendSMSAlphaIdentifier:1;
   bool mSetupEventUserActivity:1;
   bool mSetupEventIdleScreenNotify:1;
   bool mSetupEventLanguageSelNotify:1;
   bool mSetupIdleModeText:1;
   bool mLanguageNotification:1;

   // Padding out 1 bits
   UINT8 mReserved1:1;

   bool mEndProactiveSession:1;
   bool mPlayTone:1;
   bool mSetupCall:1;
   bool mSendDTMF:1;
   bool mLaunchBrowser:1;
   bool mSendSS:1;
   bool mSendUSSD:1;
   bool mProvideLocalInformationLanguage:1;
   bool mBearerIndependentProtocol:1;

   // Padding out 2 bits
   UINT8 mReserved2:2;

   bool mSCWSEvent:1;
   bool mActivate:1;
   bool mSetupEventHCIConnectivity:1;

   // Padding out 6 bits
   UINT8 mReserved3:6;
};

// Structure to describe request TLV 0x12 for CATSetEventReport()
struct sCATSetEventReportRequest_Slot
{
   bool mSlot1:1;
   bool mSlot2:1;

   // Padding out 6 bits
   UINT8 mReserved1:6;
};

// Structure to describe response TLV 0x10 for CATSetEventReport()
struct sCATSetEventReportResponse_RegStatusMask
{
   bool mDisplayText:1;
   bool mGetInkey:1;
   bool mGetInput:1;
   bool mSetupMenu:1;
   bool mSelectItem:1;
   bool mSendSMSAlphaIdentifier:1;
   bool mSetupEventUserActivity:1;
   bool mSetupEventIdleScreenNotify:1;
   bool mSetupEventLanguageSelNotify:1;
   bool mSetupIdleModeText:1;
   bool mLanguageNotification:1;
   bool mRefresh:1;
   bool mEndProactiveSession:1;
   bool mPlayTone:1;
   bool mSetupCall:1;
   bool mSendDTMF:1;
   bool mLaunchBrowser:1;
   bool mSendSS:1;
   bool mSendUSSD:1;
   bool mProvideLocalInformationLanguage:1;
   bool mBearerIndependentProtocol:1;
   bool mSetupEventBrowserTermination:1;
   bool mProvideLocalInformationTime:1;
   bool mActivate:1;

   // Padding out 1 bits
   UINT8 mReserved1:1;

   bool mSetupEventHCIConnectivity:1;

   // Padding out 6 bits
   UINT8 mReserved2:6;
};

// Structure to describe response TLV 0x11 for CATSetEventReport()
struct sCATSetEventReportResponse_DecodedRegStatusMask
{
   bool mDisplayText:1;
   bool mGetInkey:1;
   bool mGetInput:1;
   bool mSetupMenu:1;
   bool mSelectItem:1;
   bool mSendSMSAlphaIdentifier:1;
   bool mSetupEventUserActivity:1;
   bool mSetupEventIdleScreenNotify:1;
   bool mSetupEventLanguageSelNotify:1;
   bool mSetupIdleModeText:1;
   bool mLanguageNotification:1;

   // Padding out 1 bits
   UINT8 mReserved1:1;

   bool mEndProactiveSession:1;
   bool mPlayTone:1;
   bool mSetupCall:1;
   bool mSendDTMF:1;
   bool mLaunchBrowser:1;
   bool mSendSS:1;
   bool mSendUSSD:1;
   bool mProvideLocalInformationLanguage:1;
   bool mBearerIndependentProtocol:1;

   // Padding out 2 bits
   UINT8 mReserved2:2;

   bool mSCWSEvent:1;
   bool mActivate:1;
   bool mSetupEventHCIConnectivity:1;

   // Padding out 6 bits
   UINT8 mReserved3:6;
};

// Structure to describe indication TLV 0x10 for CAT EventReport
struct sCATEventReportIndication_DisplayTextEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mDisplayTextCommand[1];
};

// Structure to describe indication TLV 0x11 for CAT EventReport
struct sCATEventReportIndication_GetInkeyEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mGetInkeyCommand[1];
};

// Structure to describe indication TLV 0x12 for CAT EventReport
struct sCATEventReportIndication_GetInputEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mGetInputCommand[1];
};

// Structure to describe indication TLV 0x13 for CAT EventReport
struct sCATEventReportIndication_SetupMenuEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSetupMenuCommand[1];
};

// Structure to describe indication TLV 0x14 for CAT EventReport
struct sCATEventReportIndication_SelectItemEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSelectItemCommand[1];
};

// Structure to describe indication TLV 0x15 for CAT EventReport
struct sCATEventReportIndication_AlphaIDAvailable
{
   eQMICATAlphaIDCommandType mAlphaIDCommandType;
   UINT16 mAlphaIDLength;

   // This array must be the size specified by mAlphaIDLength
   // UINT8 mAlphaID[1];
};

// Structure to describe indication TLV 0x16 for CAT EventReport
struct sCATEventReportIndication_SetupEventList
{
   bool mUserActivityNotify:1;
   bool mIdleScreenAvailable:1;
   bool mLanguageSelectionNotify:1;

   // Padding out 29 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2[3];
};

// Structure to describe indication TLV 0x17 for CAT EventReport
struct sCATEventReportIndication_SetupIdleModeTextEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSetupIdleModeTextCommand[1];
};

// Structure to describe indication TLV 0x18 for CAT EventReport
struct sCATEventReportIndication_LanguageNotificationEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mLanguageNotificationCommand[1];
};

// Structure to describe indication TLV 0x19 for CAT EventReport
struct sCATEventReportIndication_RefreshEvent
{
   UINT16 mRefreshMode;
   eQMICATRefreshStage mRefreshStage;
};

// Structure to describe indication TLV 0x1A for CAT EventReport
struct sCATEventReportIndication_EndProactiveSession
{
   eQMICATProactiveSessionEndType mProactiveSessionEndType;
};

// Structure to describe indication TLV 0x1B for CAT EventReport
struct sCATEventReportIndication_DecodedHeaderID
{
   eQMICATCommandID mCommandID;
   UINT32 mReferenceID;
   UINT8 mCommandNumber;
};

// Structure to describe indication TLV 0x1C for CAT EventReport
struct sCATEventReportIndication_TextString
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe indication TLV 0x1D for CAT EventReport
struct sCATEventReportIndication_HighPriority
{
   eQMICATHighPriority mHighPriority;
};

// Structure to describe indication TLV 0x1E for CAT EventReport
struct sCATEventReportIndication_UserControl
{
   eQMICATUserControl mUserControl;
};

// Structure to describe indication TLV 0x1F for CAT EventReport
struct sCATEventReportIndication_Icon
{
   eQMICATIconQualifier mIconQualifier;
   UINT8 mHeight;
   UINT8 mWidth;
   eQMICATImageCodingScheme mImageCodingScheme;
   UINT8 mRecordNumber;
   UINT16 mIconDataLength;

   // This array must be the size specified by mIconDataLength
   // UINT8 mIconData[1];
};

// Structure to describe indication TLV 0x20 for CAT EventReport
struct sCATEventReportIndication_Duration
{
   eQMICATTimeUnits mUnits;
   UINT8 mInterval;
};

// Structure to describe indication TLV 0x21 for CAT EventReport
struct sCATEventReportIndication_ResponseFormat
{
   eQMICATResponseFormat mResponseFormat;
};

// Structure to describe indication TLV 0x22 for CAT EventReport
struct sCATEventReportIndication_HelpAvailable
{
   eQMICATHelpAvailable mHelpAvailable;
};

// Structure to describe indication TLV 0x23 for CAT EventReport
struct sCATEventReportIndication_ResponsePackingFormat
{
   eQMICATResponsePackingFormat mResponsePackingFormat;
};

// Structure to describe indication TLV 0x24 for CAT EventReport
struct sCATEventReportIndication_ResponseLength
{
   UINT8 mMaximumUserInput;
   UINT8 mMinimumUserInput;
};

// Structure to describe indication TLV 0x25 for CAT EventReport
struct sCATEventReportIndication_ShowUserInput
{
   eQMICATShowUserInput mShowUserInput;
};

// Structure to describe indication TLV 0x26 for CAT EventReport
struct sCATEventReportIndication_Tone
{
   eQMICATTone mTone;
};

// Structure to describe indication TLV 0x27 for CAT EventReport
struct sCATEventReportIndication_SoftkeySelection
{
   eQMICATSoftkeySelection mSoftkeySelection;
};

// Structure to describe indication TLV 0x28 for CAT EventReport
struct sCATEventReportIndication_Items
{
   UINT8 mItemsLength;

   struct sItem
   {
      UINT8 mItemID;
      UINT8 mItemTextLength;
   
      // This array must be the size specified by mItemTextLength
      // UINT8 mItemText[1];
   };

   // This array must be the size specified by mItemsLength
   // sItem mItems[1];
};

// Structure to describe indication TLV 0x29 for CAT EventReport
struct sCATEventReportIndication_DefaultItem
{
   UINT8 mDefaultItem;
};

// Structure to describe indication TLV 0x2A for CAT EventReport
struct sCATEventReportIndication_NextActionIdentifier
{
   UINT8 mActionsLength;

   // This array must be the size specified by mActionsLength
   // eQMICATNextAction mNextAction[1];
};

// Structure to describe indication TLV 0x2B for CAT EventReport
struct sCATEventReportIndication_IconIDList
{
   eQMICATDisplayIconOnly mDisplayIconOnly;
   UINT8 mItemsLength;

   struct sItem
   {
      eQMICATIconQualifier mIconQualifier;
      UINT8 mHeight;
      UINT8 mWidth;
      eQMICATImageCodingScheme mImageCodingScheme;
      UINT8 mRecordNumber;
      UINT16 mIconDataLength;
   
      // This array must be the size specified by mIconDataLength
      // UINT8 mIconData[1];
   };

   // This array must be the size specified by mItemsLength
   // sItem mItems[1];
};

// Structure to describe indication TLV 0x2C for CAT EventReport
struct sCATEventReportIndication_Presentation
{
   eQMICATPresentation mPresentation;
};

// Structure to describe indication TLV 0x2D for CAT EventReport
struct sCATEventReportIndication_PackingRequired
{
   eQMICATPackingRequired mPackingRequired;
};

// Structure to describe indication TLV 0x2E for CAT EventReport
struct sCATEventReportIndication_SMSTPDU
{
   UINT8 mSMSTPDUDataLength;

   // This array must be the size specified by mSMSTPDUDataLength
   // UINT8 mSMSTPDUData[1];
};

// Structure to describe indication TLV 0x2F for CAT EventReport
struct sCATEventReportIndication_IsCDMASMS
{
   eQMICATIsCDMASMS mIsCDMASMS;
};

// Structure to describe indication TLV 0x30 for CAT EventReport
struct sCATEventReportIndication_Address
{
   eQMICATAddressTON mAddressTON;
   eQMICATAddressNPI mAddressNPI;
   UINT8 mAddressDataLength;

   // This array must be the size specified by mAddressDataLength
   // char mAddressData[1];
};

// Structure to describe indication TLV 0x31 for CAT EventReport
struct sCATEventReportIndication_CallSetupRequirement
{
   eQMICATCallSetupRequirement mCallSetupRequirement;
};

// Structure to describe indication TLV 0x32 for CAT EventReport
struct sCATEventReportIndication_Redial
{
   eQMICATRedialNecessary mRedialNecessary;
   eQMICATTimeUnits mUnits;
   UINT8 mInterval;
};

// Structure to describe indication TLV 0x33 for CAT EventReport
struct sCATEventReportIndication_Subaddress
{
   UINT8 mSubaddressDataLength;

   struct sSubaddressData
   {
      UINT8 mSubaddressData1:4;
      UINT8 mSubaddressData2:4;
   };

   // This array must be the size specified by mSubaddressDataLength
   // sSubaddressData mSubaddressDatas[1];
};

// Structure to describe indication TLV 0x34 for CAT EventReport
struct sCATEventReportIndication_CapabilitiesConfiguration
{
   UINT8 mCapabilitesConfigurationLength;

   // This array must be the size specified by mCapabilitesConfigurationLength
   // UINT8 mCapabilitiesConfiguration[1];
};

// Structure to describe indication TLV 0x35 for CAT EventReport
struct sCATEventReportIndication_DTMF
{
   UINT8 mDTMFDataLength;

   struct sDTMFData
   {
      UINT8 mDTMFData1:4;
      UINT8 mDTMFData2:4;
   };

   // This array must be the size specified by mDTMFDataLength
   // sDTMFData mDTMFDatas[1];
};

// Structure to describe indication TLV 0x36 for CAT EventReport
struct sCATEventReportIndication_SpecificLanguageNotification
{
   eQMICATSpecificLanguageNotfication mSpecificLanguageNotification;
};

// Structure to describe indication TLV 0x37 for CAT EventReport
struct sCATEventReportIndication_Language
{
   char mLanguage[2];
};

// Structure to describe indication TLV 0x38 for CAT EventReport
struct sCATEventReportIndication_LaunchMode
{
   eQMICATLaunchMode mLaunchMode;
};

// Structure to describe indication TLV 0x39 for CAT EventReport
struct sCATEventReportIndication_URL
{
   UINT8 mURLDataLength;

   // This array must be the size specified by mURLDataLength
   // char mURLData[1];
};

// Structure to describe indication TLV 0x3A for CAT EventReport
struct sCATEventReportIndication_BrowserID
{
   UINT8 mBrowserID;
};

// Structure to describe indication TLV 0x3B for CAT EventReport
struct sCATEventReportIndication_BearerList
{
   UINT8 mBearerListLength;

   // This array must be the size specified by mBearerListLength
   // eQMICATBearer mBearerList[1];
};

// Structure to describe indication TLV 0x3C for CAT EventReport
struct sCATEventReportIndication_ProvisioningFile
{
   UINT32 mNumberOfProvisioningFiles;

   struct sFile
   {
      UINT8 mPathLength;
   
      // This array must be the size specified by mPathLength
      // char mPath[1];
   };

   // This array must be the size specified by mNumberOfProvisioningFiles
   // sFile mFiles[1];
};

// Structure to describe indication TLV 0x3D for CAT EventReport
struct sCATEventReportIndication_USSDString
{
   eQMICATUSSDDataCodingScheme mOriginalDataCodingScheme;
   eQMICATUSSDDataCodingScheme mDataCodingScheme;
   UINT8 mUSSDTextLength;

   // This array must be the size specified by mUSSDTextLength
   // UINT8 mUSSDText[1];
};

// Structure to describe indication TLV 0x3E for CAT EventReport
struct sCATEventReportIndication_DefaultText
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe indication TLV 0x3F for CAT EventReport
struct sCATEventReportIndication_ImmediateResponseRequired
{
   eQMICATImmediateResponse mImmediateResponse;
};

// Structure to describe indication TLV 0x40 for CAT EventReport
struct sCATEventReportIndication_UserConfirmationAlpha
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe indication TLV 0x41 for CAT EventReport
struct sCATEventReportIndication_SetupCallDisplayAlpha
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe indication TLV 0x42 for CAT EventReport
struct sCATEventReportIndication_UserConfirmationIcon
{
   eQMICATIconQualifier mIconQualifier;
   UINT8 mHeight;
   UINT8 mWidth;
   eQMICATImageCodingScheme mImageCodingScheme;
   UINT8 mRecordNumber;
   UINT16 mIconDataLength;

   // This array must be the size specified by mIconDataLength
   // UINT8 mIconData[1];
};

// Structure to describe indication TLV 0x43 for CAT EventReport
struct sCATEventReportIndication_SetupCallDisplayIcon
{
   eQMICATIconQualifier mIconQualifier;
   UINT8 mHeight;
   UINT8 mWidth;
   eQMICATImageCodingScheme mImageCodingScheme;
   UINT8 mRecordNumber;
   UINT16 mIconDataLength;

   // This array must be the size specified by mIconDataLength
   // UINT8 mIconData[1];
};

// Structure to describe indication TLV 0x44 for CAT EventReport
struct sCATEventReportIndication_GatewayProxy
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe indication TLV 0x45 for CAT EventReport
struct sCATEventReportIndication_Alpha
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe indication TLV 0x46 for CAT EventReport
struct sCATEventReportIndication_NotificationRequired
{
   eQMICATNotificationRequired mNotificationRequired;
};

// Structure to describe indication TLV 0x47 for CAT EventReport
struct sCATEventReportIndication_PlayToneEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mPlayToneCommand[1];
};

// Structure to describe indication TLV 0x48 for CAT EventReport
struct sCATEventReportIndication_SetupCallEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSetupCallCommand[1];
};

// Structure to describe indication TLV 0x49 for CAT EventReport
struct sCATEventReportIndication_SendDTMFEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendDTMFCommand[1];
};

// Structure to describe indication TLV 0x4A for CAT EventReport
struct sCATEventReportIndication_LaunchBrowserEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mLaunchBrowserCommand[1];
};

// Structure to describe indication TLV 0x4B for CAT EventReport
struct sCATEventReportIndication_SendSMSEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendSMSCommand[1];
};

// Structure to describe indication TLV 0x4C for CAT EventReport
struct sCATEventReportIndication_SendSSEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendSSCommand[1];
};

// Structure to describe indication TLV 0x4D for CAT EventReport
struct sCATEventReportIndication_SendUSSDEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendUSSDCommand[1];
};

// Structure to describe indication TLV 0x4E for CAT EventReport
struct sCATEventReportIndication_ProvideLocalInformationEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mProvideLocalInformationCommand[1];
};

// Structure to describe indication TLV 0x4F for CAT EventReport
struct sCATEventReportIndication_SetupRawEventList
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSetupEventListCommand[1];
};

// Structure to describe indication TLV 0x50 for CAT EventReport
struct sCATEventReportIndication_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe indication TLV 0x51 for CAT EventReport
struct sCATEventReportIndication_OpenChannelEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mOpenChannelCommand[1];
};

// Structure to describe indication TLV 0x52 for CAT EventReport
struct sCATEventReportIndication_CloseChannelEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mCloseChannelCommand[1];
};

// Structure to describe indication TLV 0x53 for CAT EventReport
struct sCATEventReportIndication_SendDataEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendDataCommand[1];
};

// Structure to describe indication TLV 0x54 for CAT EventReport
struct sCATEventReportIndication_ReceiveDataEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mReceiveDataCommand[1];
};

// Structure to describe indication TLV 0x55 for CAT EventReport
struct sCATEventReportIndication_OnDemmandLinkEstablish
{
   eQMICATOnDemandLinkEstablish mOnDemandLinkEstablish;
};

// Structure to describe indication TLV 0x56 for CAT EventReport
struct sCATEventReportIndication_CSDBearerDescription
{
   UINT8 mSpeed;
   eQMICATCSDBearerName mCSDBearerName;
   eQMICATConnectionElement mConnectionElement;
};

// Structure to describe indication TLV 0x57 for CAT EventReport
struct sCATEventReportIndication_GPRSBearerDescription
{
   UINT8 mPrecedenceClass;
   UINT8 mDelayClass;
   UINT8 mReliabilityClass;
   UINT8 mPeakThroughput;
   UINT8 mMeanThroughput;
   eQMICATPacketDataProtocol mPacketDataProtocol;
};

// Structure to describe indication TLV 0x58 for CAT EventReport
struct sCATEventReportIndication_EUTRANExternalParameterBearerDescription
{
   eQMICATTrafficClass mTrafficClass;
   UINT16 mMaxUploadBitrate;
   UINT16 mMaxDownloadBitrate;
   UINT16 mGuaranteedUploadBitrate;
   UINT16 mGuaranteedDownloadBitrate;
   eQMICATDeliveryOrder mDeliveryOrder;
   UINT8 mMaxSDUSize;
   UINT8 mMaxSDUErrorRatio;
   UINT8 mResidualBitErrorRatio;
   eQMICATDeliverErrorSDU mDeliverErrorSDU;
   UINT8 mTransferDelay;
   UINT8 mTrafficHandlingPRI;
   eQMICATPDPType mPDPType;
};

// Structure to describe indication TLV 0x59 for CAT EventReport
struct sCATEventReportIndication_EUTRANExternalMappedUTRANBearerDescription
{
   UINT8 mQCI;
   UINT8 mMaxUploadBitrate;
   UINT8 mMaxDownloadBitrate;
   UINT8 mGuaranteedUploadBitrate;
   UINT8 mGuaranteedDownloadBitrate;
   UINT8 mMaximumUploadBitrateExt;
   UINT8 mMaximumDownloadBitrateExt;
   UINT8 mGuaranteedUploadBitrateExt;
   UINT8 mGuaranteedDownloadBitrateExt;
   eQMICATPDPType mPDPType;
};

// Structure to describe indication TLV 0x5A for CAT EventReport
struct sCATEventReportIndication_BufferSize
{
   UINT16 mBufferSize;
};

// Structure to describe indication TLV 0x5B for CAT EventReport
struct sCATEventReportIndication_NetworkAccessName
{
   UINT8 mNetworkAccessNameLength;

   // This array must be the size specified by mNetworkAccessNameLength
   // UINT8 mNetworkAccessName[1];
};

// Structure to describe indication TLV 0x5C for CAT EventReport
struct sCATEventReportIndication_OtherAddress
{
   eQMICATAddressType mAddressType;
   UINT8 mAddressDataLength;

   // This array must be the size specified by mAddressDataLength
   // char mAddressData[1];
};

// Structure to describe indication TLV 0x5D for CAT EventReport
struct sCATEventReportIndication_UserLogin
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe indication TLV 0x5E for CAT EventReport
struct sCATEventReportIndication_UserPassword
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe indication TLV 0x5F for CAT EventReport
struct sCATEventReportIndication_TransportLevel
{
   eQMICATTransportProtocol mTransportProtocol;
   UINT16 mPortNumber;
};

// Structure to describe indication TLV 0x60 for CAT EventReport
struct sCATEventReportIndication_DataDestinationAddress
{
   eQMICATAddressType mAddressType;
   UINT8 mAddressDataLength;

   // This array must be the size specified by mAddressDataLength
   // char mAddressData[1];
};

// Structure to describe indication TLV 0x61 for CAT EventReport
struct sCATEventReportIndication_ChannelDataLength
{
   UINT8 mChannelDataLength;
};

// Structure to describe indication TLV 0x62 for CAT EventReport
struct sCATEventReportIndication_SendDataImmediately
{
   eQMICATSendDataImmediately mSendDataImmediately;
};

// Structure to describe indication TLV 0x63 for CAT EventReport
struct sCATEventReportIndication_ChannelData
{
   UINT16 mChannelDataLength;

   // This array must be the size specified by mChannelDataLength
   // UINT8 mChannelData[1];
};

// Structure to describe indication TLV 0x64 for CAT EventReport
struct sCATEventReportIndication_ChannelID
{
   UINT8 mChannelID;
};

// Structure to describe indication TLV 0x65 for CAT EventReport
struct sCATEventReportIndication_ItemsWithDCS
{
   UINT8 mItemsLength;

   struct sItem
   {
      UINT8 mItemID;
      eQMICATDataCodingScheme mDataCodingScheme;
      UINT8 mItemTextLength;
   
      // This array must be the size specified by mItemTextLength
      // UINT8 mItemText[1];
   };

   // This array must be the size specified by mItemsLength
   // sItem mItems[1];
};

// Structure to describe indication TLV 0x66 for CAT EventReport
struct sCATEventReportIndication_Activate
{
   UINT32 mReferenceID;
   UINT16 mActivateLength;

   // This array must be the size specified by mActivateLength
   // UINT8 mActivate[1];
};

// Structure to describe indication TLV 0x67 for CAT EventReport
struct sCATEventReportIndication_ActivateTarget
{
   eQMICATActivateTargets mActivateTarget;
};

// Structure to describe response TLV 0x01 for CATGetServiceState()
struct sCATGetServiceStateResponse_CATServiceState
{
   bool mCommonDisplayText:1;
   bool mCommonGetInkey:1;
   bool mCommonGetInput:1;
   bool mCommonSetupMenu:1;
   bool mCommonSelectItem:1;
   bool mCommonSendSMSAlphaIdentifier:1;
   bool mCommonSetupEventUserActivity:1;
   bool mCommonSetupEventIdleScreenNotify:1;
   bool mCommonSetupEventLanguageSelNotify:1;
   bool mCommonSetupIdleModeText:1;
   bool mCommonLanguageNotification:1;
   bool mCommonRefresh:1;
   bool mCommonEndProactiveSession:1;
   bool mCommonPlayTone:1;
   bool mCommonSetupCall:1;
   bool mCommonSendDTMF:1;
   bool mCommonLaunchBrowser:1;
   bool mCommonSendSS:1;
   bool mCommonSendUSSD:1;
   bool mCommonProvideLocalInformationLanguage:1;
   bool mCommonBearerIndependentProtocol:1;
   bool mCommonSetupEventBrowserTermination:1;
   bool mCommonProvideLocalInformationTime:1;
   bool mCommonActivate:1;

   // Padding out 1 bits
   UINT8 mReserved1:1;

   bool mCommonSetupEventHCIConnectivity:1;

   // Padding out 6 bits
   UINT8 mReserved2:6;

   bool mControlDisplayText:1;
   bool mControlGetInkey:1;
   bool mControlGetInput:1;
   bool mControlSetupMenu:1;
   bool mControlSelectItem:1;
   bool mControlSendSMSAlphaIdentifier:1;
   bool mControlSetupEventUserActivity:1;
   bool mControlSetupEventIdleScreenNotify:1;
   bool mControlSetupEventLanguageSelNotify:1;
   bool mControlSetupIdleModeText:1;
   bool mControlLanguageNotification:1;
   bool mControlRefresh:1;
   bool mControlEndProactiveSession:1;
   bool mControlPlayTone:1;
   bool mControlSetupCall:1;
   bool mControlSendDTMF:1;
   bool mControlLaunchBrowser:1;
   bool mControlSendSS:1;
   bool mControlSendUSSD:1;
   bool mControlProvideLocalInformationLanguage:1;
   bool mControlBearerIndependentProtocol:1;
   bool mControlSetupEventBrowserTermination:1;
   bool mControlProvideLocalInformationTime:1;
   bool mControlActivate:1;

   // Padding out 1 bits
   UINT8 mReserved3:1;

   bool mControlSetupEventHCIConnectivity:1;

   // Padding out 6 bits
   UINT8 mReserved4:6;
};

// Structure to describe response TLV 0x10 for CATGetServiceState()
struct sCATGetServiceStateResponse_DecodedCATServiceState
{
   bool mCommonDisplayText:1;
   bool mCommonGetInkey:1;
   bool mCommonGetInput:1;
   bool mCommonSetupMenu:1;
   bool mCommonSelectItem:1;
   bool mCommonSendSMSAlphaIdentifier:1;
   bool mCommonSetupEventUserActivity:1;
   bool mCommonSetupEventIdleScreenNotify:1;
   bool mCommonSetupEventLanguageSelNotify:1;
   bool mCommonSetupIdleModeText:1;
   bool mCommonLanguageNotification:1;

   // Padding out 1 bits
   UINT8 mReserved1:1;

   bool mCommonEndProactiveSession:1;
   bool mCommonPlayTone:1;
   bool mCommonSetupCall:1;
   bool mCommonSendDTMF:1;
   bool mCommonLaunchBrowser:1;
   bool mCommonSendSS:1;
   bool mCommonSendUSSD:1;
   bool mCommonProvideLocalInformationLanguage:1;
   bool mCommonBearerIndependentProtocol:1;

   // Padding out 2 bits
   UINT8 mReserved2:2;

   bool mCommonSCWSEvent:1;
   bool mCommonActivate:1;
   bool mCommonSetupEventHCIConnectivity:1;

   // Padding out 6 bits
   UINT8 mReserved3:6;

   bool mControlDisplayText:1;
   bool mControlGetInkey:1;
   bool mControlGetInput:1;
   bool mControlSetupMenu:1;
   bool mControlSelectItem:1;
   bool mControlSendSMSAlphaIdentifier:1;
   bool mControlSetupEventUserActivity:1;
   bool mControlSetupEventIdleScreenNotify:1;
   bool mControlSetupEventLanguageSelNotify:1;
   bool mControlSetupIdleModeText:1;
   bool mControlLanguageNotification:1;

   // Padding out 1 bits
   UINT8 mReserved4:1;

   bool mControlEndProactiveSession:1;
   bool mControlPlayTone:1;
   bool mControlSetupCall:1;
   bool mControlSendDTMF:1;
   bool mControlLaunchBrowser:1;
   bool mControlSendSS:1;
   bool mControlSendUSSD:1;
   bool mControlProvideLocalInformationLanguage:1;
   bool mControlBearerIndependentProtocol:1;

   // Padding out 2 bits
   UINT8 mReserved5:2;

   bool mControlSCWSEvent:1;
   bool mControlActivate:1;
   bool mControlSetupEventHCIConnectivity:1;

   // Padding out 6 bits
   UINT8 mReserved6:6;
};

// Structure to describe request TLV 0x01 for CATSendTerminalResponse()
struct sCATSendTerminalResponseRequest_TerminalResponseType
{
   UINT32 mReferenceID;
   UINT16 mTerminalResponseLength;

   // This array must be the size specified by mTerminalResponseLength
   // UINT8 mTerminalResponse[1];
};

// Structure to describe request TLV 0x10 for CATSendTerminalResponse()
struct sCATSendTerminalResponseRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe response TLV 0x10 for CATSendTerminal()
struct sCATSendTerminalResponseResponse_TRResponse
{
   UINT8 mSW1;
   UINT8 mSW2;
   UINT8 mTerminalResponseLength;

   // This array must be the size specified by mTerminalResponseLength
   // UINT8 mTerminalResponseData[1];
};

// Structure to describe request TLV 0x01 for CATEnvelopeCommand()
struct sCATEnvelopeCommandRequest_EnvelopeCommand
{
   eQMICATEnvelopeCommandType mEnvelopeCommandType;
   UINT16 mEnvelopeLength;

   // This array must be the size specified by mEnvelopeLength
   // UINT8 mEnvelopeData[1];
};

// Structure to describe request TLV 0x10 for CATEnvelopeCommand()
struct sCATEnvelopeCommandRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe response TLV 0x10 for CATEnvelopeCommand()
struct sCATEnvelopeCommandResponse_RawResponse
{
   UINT8 mSW1;
   UINT8 mSW2;
   UINT8 mEnvelopeResponseLength;

   // This array must be the size specified by mEnvelopeResponseLength
   // UINT8 mEnvelopeResponseData[1];
};

// Structure to describe request TLV 0x01 for CATGetEventReport()
struct sCATGetEventReportRequest_CommandInput
{
   UINT32 mCommandID;
   eQMICATCommandFormat mCommandFormat;
};

// Structure to describe response TLV 0x10 for CATGetEventReport()
struct sCATGetEventReportResponse_DisplayTextEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mDisplayTextCommand[1];
};

// Structure to describe response TLV 0x11 for CATGetEventReport()
struct sCATGetEventReportResponse_GetInkeyEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mGetInkeyCommand[1];
};

// Structure to describe response TLV 0x12 for CATGetEventReport()
struct sCATGetEventReportResponse_GetInputEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mGetInputCommand[1];
};

// Structure to describe response TLV 0x13 for CATGetEventReport()
struct sCATGetEventReportResponse_SetupMenuEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSetupMenuCommand[1];
};

// Structure to describe response TLV 0x14 for CATGetEventReport()
struct sCATGetEventReportResponse_SelectItemEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSelectItemCommand[1];
};

// Structure to describe response TLV 0x15 for CATGetEventReport()
struct sCATGetEventReportResponse_AlphaIDAvailable
{
   eQMICATAlphaIDCommandType mAlphaIDCommandType;
   UINT16 mAlphaIDLength;

   // This array must be the size specified by mAlphaIDLength
   // UINT8 mAlphaID[1];
};

// Structure to describe response TLV 0x16 for CATGetEventReport()
struct sCATGetEventReportResponse_SetupEventList
{
   bool mUserActivityNotify:1;
   bool mIdleScreenAvailable:1;
   bool mLanguageSelectionNotify:1;

   // Padding out 29 bits
   UINT8 mReserved1:5;
   UINT8 mReserved2[3];
};

// Structure to describe response TLV 0x17 for CATGetEventReport()
struct sCATGetEventReportResponse_SetupIdleModeTextEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSetupIdleModeTextCommand[1];
};

// Structure to describe response TLV 0x18 for CATGetEventReport()
struct sCATGetEventReportResponse_LanguageNotificationEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mLanguageNotificationCommand[1];
};

// Structure to describe response TLV 0x19 for CATGetEventReport()
struct sCATGetEventReportResponse_RefreshEvent
{
   UINT16 mRefreshMode;
   eQMICATRefreshStage mRefreshStage;
};

// Structure to describe response TLV 0x1A for CATGetEventReport()
struct sCATGetEventReportResponse_EndProactiveSession
{
   eQMICATProactiveSessionEndType mProactiveSessionEndType;
};

// Structure to describe response TLV 0x1B for CATGetEventReport()
struct sCATGetEventReportResponse_DecodedHeaderID
{
   eQMICATCommandID mCommandID;
   UINT32 mReferenceID;
   UINT8 mCommandNumber;
};

// Structure to describe response TLV 0x1C for CATGetEventReport()
struct sCATGetEventReportResponse_TextString
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x1D for CATGetEventReport()
struct sCATGetEventReportResponse_HighPriority
{
   eQMICATHighPriority mHighPriority;
};

// Structure to describe response TLV 0x1E for CATGetEventReport()
struct sCATGetEventReportResponse_UserControl
{
   eQMICATUserControl mUserControl;
};

// Structure to describe response TLV 0x1F for CATGetEventReport()
struct sCATGetEventReportResponse_Icon
{
   eQMICATIconQualifier mIconQualifier;
   UINT8 mHeight;
   UINT8 mWidth;
   eQMICATImageCodingScheme mImageCodingScheme;
   UINT8 mRecordNumber;
   UINT16 mIconDataLength;

   // This array must be the size specified by mIconDataLength
   // UINT8 mIconData[1];
};

// Structure to describe response TLV 0x20 for CATGetEventReport()
struct sCATGetEventReportResponse_Duration
{
   eQMICATTimeUnits mUnits;
   UINT8 mInterval;
};

// Structure to describe response TLV 0x21 for CATGetEventReport()
struct sCATGetEventReportResponse_ResponseFormat
{
   eQMICATResponseFormat mResponseFormat;
};

// Structure to describe response TLV 0x22 for CATGetEventReport()
struct sCATGetEventReportResponse_HelpAvailable
{
   eQMICATHelpAvailable mHelpAvailable;
};

// Structure to describe response TLV 0x23 for CATGetEventReport()
struct sCATGetEventReportResponse_ResponsePackingFormat
{
   eQMICATResponsePackingFormat mResponsePackingFormat;
};

// Structure to describe response TLV 0x24 for CATGetEventReport()
struct sCATGetEventReportResponse_ResponseLength
{
   UINT8 mMaximumUserInput;
   UINT8 mMinimumUserInput;
};

// Structure to describe response TLV 0x25 for CATGetEventReport()
struct sCATGetEventReportResponse_ShowUserInput
{
   eQMICATShowUserInput mShowUserInput;
};

// Structure to describe response TLV 0x26 for CATGetEventReport()
struct sCATGetEventReportResponse_Tone
{
   eQMICATTone mTone;
};

// Structure to describe response TLV 0x27 for CATGetEventReport()
struct sCATGetEventReportResponse_SoftkeySelection
{
   eQMICATSoftkeySelection mSoftkeySelection;
};

// Structure to describe response TLV 0x28 for CATGetEventReport()
struct sCATGetEventReportResponse_Items
{
   UINT8 mItemsLength;

   struct sItem
   {
      UINT8 mItemID;
      UINT8 mItemTextLength;
   
      // This array must be the size specified by mItemTextLength
      // UINT8 mItemText[1];
   };

   // This array must be the size specified by mItemsLength
   // sItem mItems[1];
};

// Structure to describe response TLV 0x29 for CATGetEventReport()
struct sCATGetEventReportResponse_DefaultItems
{
   UINT8 mDefaultItem;
};

// Structure to describe response TLV 0x2A for CATGetEventReport()
struct sCATGetEventReportResponse_NextActionIdentifier
{
   UINT8 mActionsLength;

   // This array must be the size specified by mActionsLength
   // eQMICATNextAction mNextAction[1];
};

// Structure to describe response TLV 0x2B for CATGetEventReport()
struct sCATGetEventReportResponse_IconIDList
{
   eQMICATDisplayIconOnly mDisplayIconOnly;
   UINT8 mItemsLength;

   struct sItem
   {
      eQMICATIconQualifier mIconQualifier;
      UINT8 mHeight;
      UINT8 mWidth;
      eQMICATImageCodingScheme mImageCodingScheme;
      UINT8 mRecordNumber;
      UINT16 mIconDataLength;
   
      // This array must be the size specified by mIconDataLength
      // UINT8 mIconData[1];
   };

   // This array must be the size specified by mItemsLength
   // sItem mItems[1];
};

// Structure to describe response TLV 0x2C for CATGetEventReport()
struct sCATGetEventReportResponse_Presentation
{
   eQMICATPresentation mPresentation;
};

// Structure to describe response TLV 0x2D for CATGetEventReport()
struct sCATGetEventReportResponse_PackingRequired
{
   eQMICATPackingRequired mPackingRequired;
};

// Structure to describe response TLV 0x2E for CATGetEventReport()
struct sCATGetEventReportResponse_SMSTPDU
{
   UINT8 mSMSTPDUDataLength;

   // This array must be the size specified by mSMSTPDUDataLength
   // UINT8 mSMSTPDUData[1];
};

// Structure to describe response TLV 0x2F for CATGetEventReport()
struct sCATGetEventReportResponse_IsCDMASMS
{
   eQMICATIsCDMASMS mIsCDMASMS;
};

// Structure to describe response TLV 0x30 for CATGetEventReport()
struct sCATGetEventReportResponse_Address
{
   eQMICATAddressTON mAddressTON;
   eQMICATAddressNPI mAddressNPI;
   UINT8 mAddressDataLength;

   // This array must be the size specified by mAddressDataLength
   // char mAddressData[1];
};

// Structure to describe response TLV 0x31 for CATGetEventReport()
struct sCATGetEventReportResponse_CallSetupRequirement
{
   eQMICATCallSetupRequirement mCallSetupRequirement;
};

// Structure to describe response TLV 0x32 for CATGetEventReport()
struct sCATGetEventReportResponse_Redial
{
   eQMICATRedialNecessary mRedialNecessary;
   eQMICATTimeUnits mUnits;
   UINT8 mInterval;
};

// Structure to describe response TLV 0x33 for CATGetEventReport()
struct sCATGetEventReportResponse_Subaddress
{
   UINT8 mSubaddressDataLength;

   struct sSubaddressData
   {
      UINT8 mSubaddressData1:4;
      UINT8 mSubaddressData2:4;
   };

   // This array must be the size specified by mSubaddressDataLength
   // sSubaddressData mSubaddressDatas[1];
};

// Structure to describe response TLV 0x34 for CATGetEventReport()
struct sCATGetEventReportResponse_CapabilityConfiguration
{
   UINT8 mCapabilitesConfigurationLength;

   // This array must be the size specified by mCapabilitesConfigurationLength
   // UINT8 mCapabilitiesConfiguration[1];
};

// Structure to describe response TLV 0x35 for CATGetEventReport()
struct sCATGetEventReportResponse_DTMF
{
   UINT8 mDTMFDataLength;

   struct sDTMFData
   {
      UINT8 mDTMFData1:4;
      UINT8 mDTMFData2:4;
   };

   // This array must be the size specified by mDTMFDataLength
   // sDTMFData mDTMFDatas[1];
};

// Structure to describe response TLV 0x36 for CATGetEventReport()
struct sCATGetEventReportResponse_SpecificLanguageNotification
{
   eQMICATSpecificLanguageNotfication mSpecificLanguageNotification;
};

// Structure to describe response TLV 0x37 for CATGetEventReport()
struct sCATGetEventReportResponse_Language
{
   char mLanguage[2];
};

// Structure to describe response TLV 0x38 for CATGetEventReport()
struct sCATGetEventReportResponse_LaunchMode
{
   eQMICATLaunchMode mLaunchMode;
};

// Structure to describe response TLV 0x39 for CATGetEventReport()
struct sCATGetEventReportResponse_URL
{
   UINT8 mURLDataLength;

   // This array must be the size specified by mURLDataLength
   // char mURLData[1];
};

// Structure to describe response TLV 0x3A for CATGetEventReport()
struct sCATGetEventReportResponse_BrowserID
{
   UINT8 mBrowserID;
};

// Structure to describe response TLV 0x3B for CATGetEventReport()
struct sCATGetEventReportResponse_BearerList
{
   UINT8 mBearerListLength;

   // This array must be the size specified by mBearerListLength
   // eQMICATBearer mBearerList[1];
};

// Structure to describe response TLV 0x3C for CATGetEventReport()
struct sCATGetEventReportResponse_ProvisioningFiles
{
   UINT32 mNumberOfProvisioningFiles;

   struct sFile
   {
      UINT8 mPathLength;
   
      // This array must be the size specified by mPathLength
      // char mPath[1];
   };

   // This array must be the size specified by mNumberOfProvisioningFiles
   // sFile mFiles[1];
};

// Structure to describe response TLV 0x3D for CATGetEventReport()
struct sCATGetEventReportResponse_USSDString
{
   eQMICATUSSDDataCodingScheme mOriginalDataCodingScheme;
   eQMICATUSSDDataCodingScheme mDataCodingScheme;
   UINT8 mUSSDTextLength;

   // This array must be the size specified by mUSSDTextLength
   // UINT8 mUSSDText[1];
};

// Structure to describe response TLV 0x3E for CATGetEventReport()
struct sCATGetEventReportResponse_DefaultText
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x3F for CATGetEventReport()
struct sCATGetEventReportResponse_ImmediateResponseRequest
{
   eQMICATImmediateResponse mImmediateResponse;
};

// Structure to describe response TLV 0x40 for CATGetEventReport()
struct sCATGetEventReportResponse_UserConfirmationAlpha
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x41 for CATGetEventReport()
struct sCATGetEventReportResponse_SetupCallDisplayAlpha
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x42 for CATGetEventReport()
struct sCATGetEventReportResponse_UserConfirmationIcon
{
   eQMICATIconQualifier mIconQualifier;
   UINT8 mHeight;
   UINT8 mWidth;
   eQMICATImageCodingScheme mImageCodingScheme;
   UINT8 mRecordNumber;
   UINT16 mIconDataLength;

   // This array must be the size specified by mIconDataLength
   // UINT8 mIconData[1];
};

// Structure to describe response TLV 0x43 for CATGetEventReport()
struct sCATGetEventReportResponse_SetupCallDisplayIcon
{
   eQMICATIconQualifier mIconQualifier;
   UINT8 mHeight;
   UINT8 mWidth;
   eQMICATImageCodingScheme mImageCodingScheme;
   UINT8 mRecordNumber;
   UINT16 mIconDataLength;

   // This array must be the size specified by mIconDataLength
   // UINT8 mIconData[1];
};

// Structure to describe response TLV 0x44 for CATGetEventReport()
struct sCATGetEventReportResponse_GatewayProxy
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x45 for CATGetEventReport()
struct sCATGetEventReportResponse_Alpha
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x46 for CATGetEventReport()
struct sCATGetEventReportResponse_NotificationRequired
{
   eQMICATNotificationRequired mNotificationRequired;
};

// Structure to describe response TLV 0x47 for CATGetEventReport()
struct sCATGetEventReportResponse_PlayToneEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mPlayToneCommand[1];
};

// Structure to describe response TLV 0x48 for CATGetEventReport()
struct sCATGetEventReportResponse_SetupCallEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSetupCallCommand[1];
};

// Structure to describe response TLV 0x49 for CATGetEventReport()
struct sCATGetEventReportResponse_SendDTMFEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendDTMFCommand[1];
};

// Structure to describe response TLV 0x4A for CATGetEventReport()
struct sCATGetEventReportResponse_LaunchBrowserEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mLaunchBrowserCommand[1];
};

// Structure to describe response TLV 0x4B for CATGetEventReport()
struct sCATGetEventReportResponse_SendSMSEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendSMSCommand[1];
};

// Structure to describe response TLV 0x4C for CATGetEventReport()
struct sCATGetEventReportResponse_SendSSEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendSSCommand[1];
};

// Structure to describe response TLV 0x4D for CATGetEventReport()
struct sCATGetEventReportResponse_SendUSSDEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendUSSDCommand[1];
};

// Structure to describe response TLV 0x4E for CATGetEventReport()
struct sCATGetEventReportResponse_ProvideLocalInformationEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mProvideLocalInformationCommand[1];
};

// Structure to describe response TLV 0x4F for CATGetEventReport()
struct sCATGetEventReportResponse_SetupEventListRawEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSetupEventListCommand[1];
};

// Structure to describe response TLV 0x50 for CATGetEventReport()
struct sCATGetEventReportResponse_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe response TLV 0x51 for CATGetEventReport()
struct sCATGetEventReportResponse_OpenChannelEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mOpenChannelCommand[1];
};

// Structure to describe response TLV 0x52 for CATGetEventReport()
struct sCATGetEventReportResponse_CloseChannelEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mCloseChannelCommand[1];
};

// Structure to describe response TLV 0x53 for CATGetEventReport()
struct sCATGetEventReportResponse_SendDataEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mSendDataCommand[1];
};

// Structure to describe response TLV 0x54 for CATGetEventReport()
struct sCATGetEventReportResponse_ReceiveDataEvent
{
   UINT32 mReferenceID;
   UINT16 mCommandLength;

   // This array must be the size specified by mCommandLength
   // UINT8 mReceiveDataCommand[1];
};

// Structure to describe response TLV 0x55 for CATGetEventReport()
struct sCATGetEventReportResponse_OnDemandLinkEstablish
{
   eQMICATOnDemandLinkEstablish mOnDemandLinkEstablish;
};

// Structure to describe response TLV 0x56 for CATGetEventReport()
struct sCATGetEventReportResponse_CSDBearerDescription
{
   UINT8 mSpeed;
   eQMICATCSDBearerName mCSDBearerName;
   eQMICATConnectionElement mConnectionElement;
};

// Structure to describe response TLV 0x57 for CATGetEventReport()
struct sCATGetEventReportResponse_GPRSBearerDescription
{
   UINT8 mPrecedenceClass;
   UINT8 mDelayClass;
   UINT8 mReliabilityClass;
   UINT8 mPeakThroughput;
   UINT8 mMeanThroughput;
   eQMICATPacketDataProtocol mPacketDataProtocol;
};

// Structure to describe response TLV 0x58 for CATGetEventReport()
struct sCATGetEventReportResponse_EUTRANExternalParameterBearerDescription
{
   eQMICATTrafficClass mTrafficClass;
   UINT16 mMaxUploadBitrate;
   UINT16 mMaxDownloadBitrate;
   UINT16 mGuaranteedUploadBitrate;
   UINT16 mGuaranteedDownloadBitrate;
   eQMICATDeliveryOrder mDeliveryOrder;
   UINT8 mMaxSDUSize;
   UINT8 mMaxSDUErrorRatio;
   UINT8 mResidualBitErrorRatio;
   eQMICATDeliverErrorSDU mDeliverErrorSDU;
   UINT8 mTransferDelay;
   UINT8 mTrafficHandlingPRI;
   eQMICATPDPType mPDPType;
};

// Structure to describe response TLV 0x59 for CATGetEventReport()
struct sCATGetEventReportResponse_EUTRANExternalMappedUTRANBearerDescription
{
   UINT8 mQCI;
   UINT8 mMaxUploadBitrate;
   UINT8 mMaxDownloadBitrate;
   UINT8 mGuaranteedUploadBitrate;
   UINT8 mGuaranteedDownloadBitrate;
   UINT8 mMaximumUploadBitrateExt;
   UINT8 mMaximumDownloadBitrateExt;
   UINT8 mGuaranteedUploadBitrateExt;
   UINT8 mGuaranteedDownloadBitrateExt;
   eQMICATPDPType mPDPType;
};

// Structure to describe response TLV 0x5A for CATGetEventReport()
struct sCATGetEventReportResponse_BufferSize
{
   UINT16 mBufferSize;
};

// Structure to describe response TLV 0x5B for CATGetEventReport()
struct sCATGetEventReportResponse_NetworkAccessName
{
   UINT8 mNetworkAccessNameLength;

   // This array must be the size specified by mNetworkAccessNameLength
   // UINT8 mNetworkAccessName[1];
};

// Structure to describe response TLV 0x5C for CATGetEventReport()
struct sCATGetEventReportResponse_OtherAddress
{
   eQMICATAddressType mAddressType;
   UINT8 mAddressDataLength;

   // This array must be the size specified by mAddressDataLength
   // char mAddressData[1];
};

// Structure to describe response TLV 0x5D for CATGetEventReport()
struct sCATGetEventReportResponse_UserLogin
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x5E for CATGetEventReport()
struct sCATGetEventReportResponse_UserPassword
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x5F for CATGetEventReport()
struct sCATGetEventReportResponse_TransportLevel
{
   eQMICATTransportProtocol mTransportProtocol;
   UINT16 mPortNumber;
};

// Structure to describe response TLV 0x60 for CATGetEventReport()
struct sCATGetEventReportResponse_DataDestinationAddress
{
   eQMICATAddressType mAddressType;
   UINT8 mAddressDataLength;

   // This array must be the size specified by mAddressDataLength
   // char mAddressData[1];
};

// Structure to describe response TLV 0x61 for CATGetEventReport()
struct sCATGetEventReportResponse_ChannelDataLength
{
   UINT8 mChannelDataLength;
};

// Structure to describe response TLV 0x62 for CATGetEventReport()
struct sCATGetEventReportResponse_SendDataImmediately
{
   eQMICATSendDataImmediately mSendDataImmediately;
};

// Structure to describe response TLV 0x63 for CATGetEventReport()
struct sCATGetEventReportResponse_ChannelData
{
   UINT16 mChannelDataLength;

   // This array must be the size specified by mChannelDataLength
   // UINT8 mChannelData[1];
};

// Structure to describe response TLV 0x64 for CATGetEventReport()
struct sCATGetEventReportResponse_ChannelID
{
   UINT8 mChannelID;
};

// Structure to describe response TLV 0x65 for CATGetEventReport()
struct sCATGetEventReportResponse_ItemsWithDCS
{
   UINT8 mItemsLength;

   struct sItem
   {
      UINT8 mItemID;
      eQMICATDataCodingScheme mDataCodingScheme;
      UINT8 mItemTextLength;
   
      // This array must be the size specified by mItemTextLength
      // UINT8 mItemText[1];
   };

   // This array must be the size specified by mItemsLength
   // sItem mItems[1];
};

// Structure to describe response TLV 0x66 for CATGetEventReport()
struct sCATGetEventReportResponse_Activate
{
   UINT32 mReferenceID;
   UINT16 mActivateLength;

   // This array must be the size specified by mActivateLength
   // UINT8 mActivate[1];
};

// Structure to describe response TLV 0x67 for CATGetEventReport()
struct sCATGetEventReportResponse_ActivateTarget
{
   eQMICATActivateTargets mActivateTarget;
};

// Structure to describe request TLV 0x01 for CATSendDecodedTerminalResponse()
struct sCATSendDecodedTerminalResponseRequest_TerminalResponse
{
   UINT32 mReferenceID;
   UINT8 mCommandNumber;
   eQMICATResponseCommand mResponseCommand;
   UINT8 mGeneralResult;
   UINT8 mAdditionalInformationLength;

   // This array must be the size specified by mAdditionalInformationLength
   // UINT8 mTerminalResponseAdditionalInformation[1];
};

// Structure to describe request TLV 0x10 for CATSendDecodedTerminalResponse()
struct sCATSendDecodedTerminalResponseRequest_TextString
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe request TLV 0x11 for CATSendDecodedTerminalResponse()
struct sCATSendDecodedTerminalResponseRequest_ItemIdentifier
{
   UINT8 mItemIdentifier;
};

// Structure to describe request TLV 0x12 for CATSendDecodedTerminalResponse()
struct sCATSendDecodedTerminalResponseRequest_GetInkeyExtraInfo
{
   eQMICATTimeUnits mUnits;
   UINT8 mInterval;
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe request TLV 0x13 for CATSendDecodedTerminalResponse()
struct sCATSendDecodedTerminalResponseRequest_LanguageInfo
{
   char mLanguage[2];
};

// Structure to describe request TLV 0x14 for CATSendDecodedTerminalResponse()
struct sCATSendDecodedTerminalResponseRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe request TLV 0x15 for CATSendDecodedTerminalResponse()
struct sCATSendDecodedTerminalResponseRequest_GetInkeyYesInput
{
   eQMICATTimeUnits mUnits;
   UINT8 mInterval;
   INT8 mGetInkeyYesInput;
};

// Structure to describe response TLV 0x10 for CATSendDecodedTerminal()
struct sCATSendDecodedTerminalResponseResponse_TRResponse
{
   UINT8 mSW1;
   UINT8 mSW2;
   UINT8 mTerminalResponseLength;

   // This array must be the size specified by mTerminalResponseLength
   // UINT8 mTerminalResponseData[1];
};

// Structure to describe request TLV 0x01 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandRequest_EnvelopeCommand
{
   eQMICATDecodedEnvelopeCommand mEnvelopeCommand;
};

// Structure to describe request TLV 0x10 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandRequest_ItemIdentifier
{
   UINT8 mItemIdentifier;
};

// Structure to describe request TLV 0x11 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandRequest_HelpRequest
{
   eQMICATHelpRequest mHelpRequest;
};

// Structure to describe request TLV 0x12 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandRequest_Language
{
   char mLanguage[2];
};

// Structure to describe request TLV 0x13 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe request TLV 0x14 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandRequest_Address
{
   eQMICATAddressTON mAddressTON;
   eQMICATAddressNPI mAddressNPI;
   UINT8 mAddressDataLength;

   // This array must be the size specified by mAddressDataLength
   // char mAddressData[1];
};

// Structure to describe request TLV 0x15 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandRequest_Subaddress
{
   UINT8 mSubaddressDataLength;

   struct sSubaddressData
   {
      UINT8 mSubaddressData1:4;
      UINT8 mSubaddressData2:4;
   };

   // This array must be the size specified by mSubaddressDataLength
   // sSubaddressData mSubaddressDatas[1];
};

// Structure to describe request TLV 0x16 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandRequest_CapabilityConfigParam1
{
   UINT8 mCapabilitesConfigurationLength;

   // This array must be the size specified by mCapabilitesConfigurationLength
   // UINT8 mCapabilitiesConfiguration[1];
};

// Structure to describe request TLV 0x17 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandRequest_CapabilityConfigParam2
{
   UINT8 mCapabilitesConfigurationLength;

   // This array must be the size specified by mCapabilitesConfigurationLength
   // UINT8 mCapabilitiesConfiguration[1];
};

// Structure to describe request TLV 0x18 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandRequest_USSDString
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe request TLV 0x19 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandRequest_PDPContextActivation
{
   UINT8 mPDPContextActivationDataLength;

   // This array must be the size specified by mPDPContextActivationDataLength
   // UINT8 mPDPContextActivationData[1];
};

// Structure to describe request TLV 0x1A for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandRequest_EPSPDNConnectActivation
{
   UINT8 mEPSPDNConnectActivationDataLength;

   // This array must be the size specified by mEPSPDNConnectActivationDataLength
   // UINT8 mEPSPDNConnectActivationData[1];
};

// Structure to describe request TLV 0x1B for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandRequest_BrowserTerminationCause
{
   eQMICATBrowserTerminationCauses mBrowserTerminationCause;
};

// Structure to describe response TLV 0x10 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandResponse_CallControlResult
{
   eQMICATCallControlResult mCallControlResult;
};

// Structure to describe response TLV 0x11 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandResponse_Address
{
   eQMICATAddressTON mAddressTON;
   eQMICATAddressNPI mAddressNPI;
   UINT8 mAddressDataLength;

   // This array must be the size specified by mAddressDataLength
   // char mAddressData[1];
};

// Structure to describe response TLV 0x12 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandResponse_Subaddress
{
   UINT8 mSubaddressDataLength;

   struct sSubaddressData
   {
      UINT8 mSubaddressData1:4;
      UINT8 mSubaddressData2:4;
   };

   // This array must be the size specified by mSubaddressDataLength
   // sSubaddressData mSubaddressDatas[1];
};

// Structure to describe response TLV 0x13 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandResponse_CapabilityConfigParam1
{
   UINT8 mCapabilitesConfigurationLength;

   // This array must be the size specified by mCapabilitesConfigurationLength
   // UINT8 mCapabilitiesConfiguration[1];
};

// Structure to describe response TLV 0x14 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandResponse_CapabilityConfigParam2
{
   UINT8 mCapabilitesConfigurationLength;

   // This array must be the size specified by mCapabilitesConfigurationLength
   // UINT8 mCapabilitiesConfiguration[1];
};

// Structure to describe response TLV 0x15 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandResponse_USSDString
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x16 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandResponse_PDPContextActivation
{
   UINT8 mPDPContextActivationDataLength;

   // This array must be the size specified by mPDPContextActivationDataLength
   // UINT8 mPDPContextActivationData[1];
};

// Structure to describe response TLV 0x17 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandResponse_EPSPDNConnectActivation
{
   UINT8 mEPSPDNConnectActivationDataLength;

   // This array must be the size specified by mEPSPDNConnectActivationDataLength
   // UINT8 mEPSPDNConnectActivationData[1];
};

// Structure to describe response TLV 0x18 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandResponse_Alpha
{
   eQMICATDataCodingScheme mDataCodingScheme;
   UINT8 mTextDataLength;

   // This array must be the size specified by mTextDataLength
   // UINT8 mTextData[1];
};

// Structure to describe response TLV 0x19 for CATSendDecodedEnvelopeCommand()
struct sCATSendDecodedEnvelopeCommandResponse_BCRepeatIndicator
{
   eQMICATBearerCapabilityRepeatIndicator mBearerCapabilityRepeatIndicator;
};

// Structure to describe request TLV 0x10 for CATEventConfirmation()
struct sCATEventConfirmationRequest_UserConfirmed
{
   eQMICATUserConfirmed mUserConfirmed;
};

// Structure to describe request TLV 0x11 for CATEventConfirmation()
struct sCATEventConfirmationRequest_IconIsDisplayed
{
   eQMICATIconIsDisplayed mIconIsDisplayed;
};

// Structure to describe request TLV 0x12 for CATEventConfirmation()
struct sCATEventConfirmationRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe request TLV 0x01 for CATSCWSOpenChannel()
struct sCATSCWSOpenChannelRequest_ChannelStatus
{
   UINT32 mChannelID;
   eQMICATChannelState mChannelState;
};

// Structure to describe request TLV 0x10 for CATSCWSOpenChannel()
struct sCATSCWSOpenChannelRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe indication TLV 0x10 for CAT SCWSOpenChannelIndication
struct sCATSCWSOpenChannelIndication_OpenChannelInformation
{
   UINT32 mChannelID;
   UINT16 mPortNumber;
   UINT16 mBufferSize;
};

// Structure to describe indication TLV 0x11 for CAT SCWSOpenChannelIndication
struct sCATSCWSOpenChannelIndication_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe request TLV 0x01 for CATSCWSCloseChannel()
struct sCATSCWSCloseChannelRequest_ChannelStatus
{
   UINT32 mChannelID;
   eQMICATChannelState mChannelState;
};

// Structure to describe request TLV 0x10 for CATSCWSCloseChannel()
struct sCATSCWSCloseChannelRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe indication TLV 0x10 for CAT SCWSCloseChannelIndication
struct sCATSCWSCloseChannelIndication_CloseChannelInfo
{
   UINT32 mChannelID;
   eQMICATChannelState mChannelState;
};

// Structure to describe response TLV 0x11 for CATSCWSCloseChannel()
struct sCATSCWSCloseChannelResponse_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe request TLV 0x01 for CATSCWSSendData()
struct sCATSCWSSendDataRequest_ChannelStatus
{
   UINT32 mChannelID;
   eQMICATSendDataResult mDataSendResult;
};

// Structure to describe request TLV 0x10 for CATSCWSSendData()
struct sCATSCWSSendDataRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe indication TLV 0x10 for CAT SCWSSendDataIndication
struct sCATSCWSSendDataIndication_SendDataInfo
{
   UINT32 mChannelID;
   UINT8 mTotalPackets;
   UINT8 mCurrentPacket;
   UINT16 mDataLength;

   // This array must be the size specified by mDataLength
   // UINT8 mData[1];
};

// Structure to describe indication TLV 0x11 for CAT SCWSSendDataIndication
struct sCATSCWSSendDataIndication_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe request TLV 0x01 for CATSCWSDataAvailable()
struct sCATSCWSDataAvailableRequest_RemainingData
{
   UINT32 mChannelID;
   UINT16 mDataLength;

   // This array must be the size specified by mDataLength
   // UINT8 mData[1];
};

// Structure to describe request TLV 0x10 for CATSCWSDataAvailable()
struct sCATSCWSDataAvailableRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe request TLV 0x01 for CATSCWSChannelStatus()
struct sCATSCWSChannelStatusRequest_ChannelStatus
{
   UINT32 mChannelID;
   eQMICATChannelState mChannelState;
};

// Structure to describe request TLV 0x10 for CATSCWSChannelStatus()
struct sCATSCWSChannelStatusRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe request TLV 0x10 for CATGetTerminalProfile()
struct sCATGetTerminalProfileRequest_Slot
{
   eQMICATSlot mSlot;
};

// Structure to describe response TLV 0x10 for CATGetTerminalProfile()
struct sCATGetTerminalProfileResponse_RawData
{
   UINT8 mTerminalProfileLength;

   // This array must be the size specified by mTerminalProfileLength
   // UINT8 mTerminalProfileData[1];
};

// Structure to describe request TLV 0x01 for CATSetConfiguration()
struct sCATSetConfigurationRequest_Mode
{
   eQMICATConfigModes mConfigMode;
};

// Structure to describe request TLV 0x10 for CATSetConfiguration()
struct sCATSetConfigurationRequest_CustomData
{
   UINT8 mTerminalProfileLength;

   // This array must be the size specified by mTerminalProfileLength
   // UINT8 mTerminalProfileData[1];
};

// Structure to describe response TLV 0x10 for CATGetConfiguration()
struct sCATGetConfigurationResponse_Mode
{
   eQMICATConfigModes mConfigMode;
};

// Structure to describe response TLV 0x11 for CATGetConfiguration()
struct sCATGetConfigurationResponse_CustomData
{
   UINT8 mTerminalProfileLength;

   // This array must be the size specified by mTerminalProfileLength
   // UINT8 mTerminalProfileData[1];
};

// Structure to describe response TLV 0x10 for RMSGetSMSWake()
struct sRMSGetSMSWakeResponse_State
{
   INT8 mSMSWakeEnabled;
};

// Structure to describe request TLV 0x11 for RMSGetSMSWake()
struct sRMSGetSMSWakeRequest_Mask
{
   UINT32 mMask;
};

// Structure to describe request TLV 0x10 for RMSSetSMSWake()
struct sRMSSetSMSWakeRequest_State
{
   INT8 mSMSWakeEnabled;
};

// Structure to describe request TLV 0x11 for RMSSetSMSWake()
struct sRMSSetSMSWakeRequest_Mask
{
   UINT32 mMask;
};

// Structure to describe request TLV 0x10 for OMASetEventReport()
struct sOMASetEventReportRequest_NIA
{
   INT8 mReportNetworkInitiatedAlerts;
};

// Structure to describe request TLV 0x11 for OMASetEventReport()
struct sOMASetEventReportRequest_Status
{
   INT8 mReportSessionStatus;
};

// Structure to describe indication TLV 0x10 for OMA EventReport
struct sOMAEventReportIndication_NIA
{
   eQMIOMASessionTypes mSessionType;
   UINT16 mSessionID;
};

// Structure to describe indication TLV 0x11 for OMA EventReport
struct sOMAEventReportIndication_Status
{
   eQMIOMASessionStates mSessionState;
};

// Structure to describe indication TLV 0x12 for OMA EventReport
struct sOMAEventReportIndication_Failure
{
   eQMIOMASessionFailureReasons mSessionFailure;
};

// Structure to describe request TLV 0x10 for OMAStartSession()
struct sOMAStartSessionRequest_Type
{
   eQMIOMASessionTypes mSessionType;
};

// Structure to describe response TLV 0x10 for OMAGetSessionInfo()
struct sOMAGetSessionInfoResponse_Info
{
   eQMIOMASessionStates mSessionState;
   eQMIOMASessionTypes mSessionType;
};

// Structure to describe response TLV 0x11 for OMAGetSessionInfo()
struct sOMAGetSessionInfoResponse_Failure
{
   eQMIOMASessionFailureReasons mSessionFailure;
};

// Structure to describe response TLV 0x12 for OMAGetSessionInfo()
struct sOMAGetSessionInfoResponse_Retry
{
   UINT8 mRetryCount;
   UINT16 mRetryPauseTimer;
   UINT16 mRemainingTime;
};

// Structure to describe response TLV 0x13 for OMAGetSessionInfo()
struct sOMAGetSessionInfoResponse_NIA
{
   eQMIOMASessionTypes mSessionType;
   UINT16 mSessionID;
};

// Structure to describe request TLV 0x10 for OMASendSelection()
struct sOMASendSelectionRequest_Type
{
   eQMIOMASelections mSelection;
   UINT16 mSessionID;
};

// Structure to describe response TLV 0x10 for OMAGetFeatures()
struct sOMAGetFeaturesResponse_Provisioning
{
   INT8 mDeviceProvisioningServiceUpdateEnabled;
};

// Structure to describe response TLV 0x11 for OMAGetFeatures()
struct sOMAGetFeaturesResponse_PRLUpdate
{
   INT8 mPRLServiceUpdateEnabled;
};

// Structure to describe response TLV 0x12 for OMAGetFeatures()
struct sOMAGetFeaturesResponse_HFAFeature
{
   INT8 mHFAFeatureEnabled;
};

// Structure to describe response TLV 0x13 for OMAGetFeatures()
struct sOMAGetFeaturesResponse_HFADoneState
{
   eQMIOMAHFADoneStates mHFAFeatureDoneState;
};

// Structure to describe request TLV 0x10 for OMASetFeatures()
struct sOMASetFeaturesRequest_Provisioning
{
   INT8 mDeviceProvisioningServiceUpdateEnabled;
};

// Structure to describe request TLV 0x11 for OMASetFeatures()
struct sOMASetFeaturesRequest_PRLUpdate
{
   INT8 mPRLServiceUpdateEnabled;
};

// Structure to describe request TLV 0x12 for OMASetFeatures()
struct sOMASetFeaturesRequest_HFAFeature
{
   INT8 mHFAFeatureEnabled;
};


#pragma pack( pop )

#
# Makefile for the Intel IXP400 Access Library
#

obj-$(CONFIG_IXP400_LIB) += ixp400.o
ifeq ($(CONFIG_IXP400_LIB),m)
obj-m += ixp400th.o
endif

IXP400_DIR := drivers/ixp400
IX_XSCALE_SW := $(IXP400_DIR)/ixp400_xscale_sw
OSAL_DIR := $(IX_XSCALE_SW)/../ixp_osal
IX_DEVICE :=ixp42X
IX_PLATFORM:=ixdp42x
IX_TARGET:=linuxbe
IX_LINUXVER:=2.6

#ifeq ($(CPU_IXP46X),y)
#EXTRA_CFLAGS += -D__ixp46X
#EXTRA_CFLAGS += -DIX_DEVICE=ixp46X
#EXTRA_CFLAGS += -DIX_PLATFORM=ixdp46x
#else
EXTRA_CFLAGS += -D__ixp42X
EXTRA_CFLAGS += -DIX_DEVICE=ixp42X
EXTRA_CFLAGS += -DIX_PLATFORM=ixdp42x
#endif

EXTRA_CFLAGS += \
	-D__linux \
	-DIX_TARGET=linuxbe \
	-DIX_UTOPIAMODE=0 \
	-DIX_PLATFORM=ixdp42x \
	-DIX_DEVICE=ixp42X \
	-DIX_LINUXVER=2.6 \
	-DIX_MPHYSINGLEPORT=0 \
	-DIX_NPEDL_READ_MICROCODE_FROM_FILE \
	-DIX_ACC_DRAM_PHYS_OFFSET=0 \
	-D__LINUX_ARM_ARCH__=5 \
	-DCPU=33 \
	-DXSCALE=33 \
	-DSIMSPARCSOLARIS=0 \
	-DNDEBUG \
	-I$(IXP400_DIR) \
	-I$(IX_XSCALE_SW)/src/include \
	-I$(OSAL_DIR)/ \
	-I$(OSAL_DIR)/os/linux/include/ \
	-I$(OSAL_DIR)/os/linux/include/core/  \
	-I$(OSAL_DIR)/os/linux/include/modules/ \
	-I$(OSAL_DIR)/os/linux/include/modules/ioMem/ \
	-I$(OSAL_DIR)/os/linux/include/modules/core/ \
	-I$(OSAL_DIR)/os/linux/include/modules/ddk/ \
	-I$(OSAL_DIR)/os/linux/include/modules/bufferMgt/ \
	-I$(OSAL_DIR)/os/linux/include/platforms/ \
	-I$(OSAL_DIR)/os/linux/include/platforms/ixp400/
ifeq ($(CPU_IXP46X),y)
EXTRA_CFLAGS += \
	-I$(OSAL_DIR)/os/linux/include/platforms/ixp400/ixp465/
else
EXTRA_CFLAGS += \
	-I$(OSAL_DIR)/os/linux/include/platforms/ixp400/ixp425/
endif
EXTRA_CFLAGS += \
	-I$(OSAL_DIR)/include/ \
	-I$(OSAL_DIR)/include/modules/ \
	-I$(OSAL_DIR)/include/modules/bufferMgt/ \
	-I$(OSAL_DIR)/include/modules/ioMem/ \
	-I$(OSAL_DIR)/include/modules/core/ \
	-I$(OSAL_DIR)/include/platforms/ \
	-I$(OSAL_DIR)/include/platforms/ixp400/
ifeq ($(CPU_IXP46X),y)
EXTRA_CFLAGS += \
	-I$(OSAL_DIR)/include/platforms/ixp400/ixp465/
else
EXTRA_CFLAGS += \
	-I$(OSAL_DIR)/include/platforms/ixp400/ixp425/
endif
EXTRA_CFLAGS += \
	-I$(IX_XSCALE_SW)/src/include \
	-I$(IX_XSCALE_SW)/src/errHdlAcc/include \
	-I$(IX_XSCALE_SW)/src/ethAcc/include \
	-I$(IX_XSCALE_SW)/src/hssAcc/include \
	-I$(IX_XSCALE_SW)/src/ethDB/include \
	-I$(IX_XSCALE_SW)/src/featureCtrl/include \
	-I$(IX_XSCALE_SW)/src/npeMh/include \
	-I$(IX_XSCALE_SW)/src/npeDl/include \
	-I$(IX_XSCALE_SW)/src/qmgr \
	-I$(IX_XSCALE_SW)/src/	\
	-I$(IX_XSCALE_SW)/src/ethDB
	


ixp400-objs += \
	ixp_osal/os/linux/src/core/IxOsalOsThread.o \
	ixp_osal/os/linux/src/core/IxOsalOsCacheMMU.o \
	ixp_osal/os/linux/src/core/IxOsalOsServices.o \
	ixp_osal/os/linux/src/core/IxOsalOsSemaphore.o  \
	ixp_osal/os/linux/src/core/IxOsalOsMsgQ.o \
	ixp_osal/os/linux/src/core/IxOsalOsSymbols.o \
	ixp_osal/os/linux/src/modules/bufferMgt/IxOsalOsBufferMgtSymbols.o \
	ixp_osal/os/linux/src/modules/ddk/IxOsalOsDdkClk.o  \
	ixp_osal/os/linux/src/modules/ddk/IxOsalOsDdkIrq.o \
	ixp_osal/os/linux/src/modules/ddk/IxOsalOsDdkPci.o  \
	ixp_osal/os/linux/src/modules/ddk/IxOsalOsDdkSymbols.o	\
	ixp_osal/os/linux/src/modules/ioMem/IxOsalOsIoMem.o  \
	ixp_osal/os/linux/src/modules/ioMem/IxOsalOsIoMemSymbols.o \
	ixp_osal/os/linux/src/platforms/ixp400/IxOsalOsIxp400.o \
	ixp_osal/os/linux/src/platforms/ixp400/IxOsalOsIxp400Symbols.o
	
ixp400-objs += \
	ixp_osal/src/core/IxOsalTime.o \
	ixp_osal/src/modules/ioMem/IxOsalIoMem.o \
	ixp_osal/src/modules/bufferMgt/IxOsalBufferMgt.o
ixp400-objs += \
	ixp400_xscale_sw/src/oslinux/IxLinuxInit.o
ixp400-objs += \
	ixp400_xscale_sw/src/featureCtrl/IxFeatureCtrl.o \
	ixp400_xscale_sw/src/featureCtrl/IxFeatureCtrlSymbols.o
ixp400-objs += \
	ixp400_xscale_sw/src/qmgr/IxQMgrQCfg.o \
	ixp400_xscale_sw/src/qmgr/IxQMgrDispatcher.o \
	ixp400_xscale_sw/src/qmgr/IxQMgrInit.o \
	ixp400_xscale_sw/src/qmgr/IxQMgrHwQIfIxp400.o \
	ixp400_xscale_sw/src/qmgr/IxQMgrQAccess.o \
	ixp400_xscale_sw/src/qmgr/IxQMgrSymbols.o

ifeq ($(CPU_IXP46X),y)

ixp400-objs += \
	ixp400_xscale_sw/src/errHdlAcc/IxErrHdlAcc.o \
	ixp400_xscale_sw/src/errHdlAcc/IxErrHdlAccControl.o \
	ixp400_xscale_sw/src/errHdlAcc/IxErrHdlAccEthNPE.o \
	ixp400_xscale_sw/src/errHdlAcc/IxErrHdlAccSymbols.o 
endif

ixp400-objs += \
	ixp400_xscale_sw/src/ethAcc/IxEthAcc.o \
	ixp400_xscale_sw/src/ethAcc/IxEthAccCommon.o \
	ixp400_xscale_sw/src/ethAcc/IxEthAccControlInterface.o \
	ixp400_xscale_sw/src/ethAcc/IxEthAccDataPlane.o \
	ixp400_xscale_sw/src/ethAcc/IxEthAccMac.o \
	ixp400_xscale_sw/src/ethAcc/IxEthAccMii.o \
	ixp400_xscale_sw/src/ethAcc/IxEthAccSymbols.o

ixp400-objs += \
	ixp400_xscale_sw/src/hssAcc/IxHssAccCCM.o \
	ixp400_xscale_sw/src/hssAcc/IxHssAccChanRx.o \
	ixp400_xscale_sw/src/hssAcc/IxHssAccCommon.o \
	ixp400_xscale_sw/src/hssAcc/IxHssAccPCM.o \
	ixp400_xscale_sw/src/hssAcc/IxHssAccPDM.o \
	ixp400_xscale_sw/src/hssAcc/IxHssAccPktRx.o \
	ixp400_xscale_sw/src/hssAcc/IxHssAccPktTx.o \
	ixp400_xscale_sw/src/hssAcc/IxHssAccService.o \
	ixp400_xscale_sw/src/hssAcc/IxHssAccSymbols.o 	

ixp400-objs += \
	ixp400_xscale_sw/src/ethDB/IxEthDBAPI.o \
	ixp400_xscale_sw/src/ethDB/IxEthDBAPISupport.o \
	ixp400_xscale_sw/src/ethDB/IxEthDBCore.o \
	ixp400_xscale_sw/src/ethDB/IxEthDBEvents.o \
	ixp400_xscale_sw/src/ethDB/IxEthDBFeatures.o \
	ixp400_xscale_sw/src/ethDB/IxEthDBFirewall.o \
	ixp400_xscale_sw/src/ethDB/IxEthDBHashtable.o \
	ixp400_xscale_sw/src/ethDB/IxEthDBLearning.o \
	ixp400_xscale_sw/src/ethDB/IxEthDBMem.o \
	ixp400_xscale_sw/src/ethDB/IxEthDBNPEAdaptor.o \
	ixp400_xscale_sw/src/ethDB/IxEthDBPortUpdate.o \
	ixp400_xscale_sw/src/ethDB/IxEthDBReports.o \
	ixp400_xscale_sw/src/ethDB/IxEthDBSearch.o \
	ixp400_xscale_sw/src/ethDB/IxEthDBSpanningTree.o \
	ixp400_xscale_sw/src/ethDB/IxEthDBUtil.o \
	ixp400_xscale_sw/src/ethDB/IxEthDBVlan.o \
	ixp400_xscale_sw/src/ethDB/IxEthDBWiFi.o \
	ixp400_xscale_sw/src/ethDB/IxEthDBSymbols.o
ixp400-objs += \
	ixp400_xscale_sw/src/ethMii/IxEthMii.o \
	ixp400_xscale_sw/src/ethMii/IxEthMiiSymbols.o
ixp400-objs += \
	ixp400_xscale_sw/src/npeMh/IxNpeMh.o \
	ixp400_xscale_sw/src/npeMh/IxNpeMhConfig.o \
	ixp400_xscale_sw/src/npeMh/IxNpeMhReceive.o \
	ixp400_xscale_sw/src/npeMh/IxNpeMhSend.o \
	ixp400_xscale_sw/src/npeMh/IxNpeMhSolicitedCbMgr.o \
	ixp400_xscale_sw/src/npeMh/IxNpeMhUnsolicitedCbMgr.o \
	ixp400_xscale_sw/src/npeMh/IxNpeMhSymbols.o
ixp400-objs += \
	ixp400_xscale_sw/src/npeDl/IxNpeDl.o \
	ixp400_xscale_sw/src/npeDl/IxNpeDlImageMgr.o \
	ixp400_xscale_sw/src/npeDl/IxNpeDlNpeMgr.o \
	ixp400_xscale_sw/src/npeDl/IxNpeDlNpeMgrUtils.o \
	ixp400_xscale_sw/src/npeDl/IxNpeDlSymbols.o \
	ixp400_xscale_sw/src/npeDl/IxNpeMicrocode.o

ixp400-objs +=  ixp400_xscale_sw/src/cryptoAcc/IxCryptoAccUtilities.o		\
		 ixp400_xscale_sw/src/cryptoAcc/IxCryptoAcc.o			        \
		 ixp400_xscale_sw/src/cryptoAcc/IxCryptoAccDescMgmt.o		    \
		 ixp400_xscale_sw/src/cryptoAcc/IxCryptoAccCCDMgmt.o		    \
		 ixp400_xscale_sw/src/cryptoAcc/IxCryptoAccQAccess.o		    \
		 ixp400_xscale_sw/src/cryptoAcc/IxCryptoAccCryptoRegister.o	\
		 ixp400_xscale_sw/src/cryptoAcc/IxCryptoAccCryptoPerform.o
		 
ifeq ($(CPU_IXP46X),y)
ixp400-objs += ixp400_xscale_sw/src/cryptoAcc/IxCryptoAccPkeCommon.o     \
		 ixp400_xscale_sw/src/cryptoAcc/IxCryptoAccPkeEau.o                \
		 ixp400_xscale_sw/src/cryptoAcc/IxCryptoAccPkeHash.o
endif
ixp400-objs += ixp400_xscale_sw/src/cryptoAcc/IxCryptoAccXScaleWep.o
ixp400-objs += ixp400_xscale_sw/src/cryptoAcc/IxCryptoAccSymbols.o

drivers/ixp400/ixp400_xscale_sw/src/cryptoAcc/IxCryptoAccXScaleWep.o: drivers/ixp400/ixp400_xscale_sw/src/cryptoAcc/IxCryptoAccXScaleWep.s
	$(CC) $(CFLAGS)	$(EXTRA_CFLAGS) -P -x assembler-with-cpp  	$< -c -o $@

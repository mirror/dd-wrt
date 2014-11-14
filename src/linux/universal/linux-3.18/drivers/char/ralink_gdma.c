/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36, Taiyuan St., Jhubei City,
 * Hsinchu County 302,
 * Taiwan, R.O.C.
 *
 * (c) Copyright, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************
 *
 */
 
#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/addrspace.h>
#include <asm/rt2880/surfboardint.h>

#include "ralink_gdma.h"

/*
 * Ch0 : Pcm0_Rx0 | Pcm0_Rx0 | ALL
 * Ch1 : Pcm0_Rx1 | Pcm0_Rx1 | ALL
 * Ch2 : Pcm0_Tx0 | Pcm0_Tx0 | ALL
 * Ch3 : Pcm0_Tx1 | Pcm0_Tx1 | ALL
 * Ch4 : Pcm1_Rx0 | I2S_Tx0  | ALL
 * Ch5 : Pcm1_Rx1 | I2S_Tx1  | ALL
 * Ch6 : Pcm1_Tx0 |  ALL     | ALL
 * Ch7 : Pcm1_Tx1 |  ALL     | ALL
 *
 */

spinlock_t  gdma_lock;
spinlock_t  gdma_int_lock;
void (*GdmaTxDoneCallback[MAX_GDMA_CHANNEL])(uint32_t);
void (*GdmaUnMaskIntCallback[MAX_GDMA_CHANNEL])(uint32_t);


/**
 * @brief Get free GDMA channel
 *
 * @param  ChNum   GDMA channel number
 * @retval 1  	   channel is available
 * @retval 0  	   channels are all busy
 */
int _GdmaGetFreeCh(uint32_t *ChNum)
{
    unsigned long flags;
    uint32_t Data=0;
    uint32_t Ch=0;

    spin_lock_irqsave(&gdma_lock, flags);

#if defined (CONFIG_GDMA_PCM_ONLY)
    for(Ch=MAX_GDMA_CHANNEL; Ch<MAX_GDMA_CHANNEL;Ch++)  //no channel
#elif defined (CONFIG_GDMA_PCM_I2S_OTHERS)
    for(Ch=6; Ch<MAX_GDMA_CHANNEL;Ch++)  //last 2 channels
#elif defined (CONFIG_GDMA_EVERYBODY)
    for(Ch=0; Ch<MAX_GDMA_CHANNEL;Ch++)  //all channel
#elif defined (CONFIG_GDMA_DEBUG)
    static uint32_t Ch_RR=0;
    for(Ch=(Ch_RR++)%MAX_GDMA_CHANNEL; Ch<MAX_GDMA_CHANNEL;Ch++)  //round robin
#endif
    {
	Data=GDMA_READ_REG(GDMA_CTRL_REG(Ch));

	/* hardware will reset this bit if transaction is done.
	 * It means channel is free */
	if((Data & (0x01<<CH_EBL_OFFSET))==0) { 
	    *ChNum = Ch;
	    spin_unlock_irqrestore(&gdma_lock, flags);
	    return 1; //Channel is free
	}
    }

    spin_unlock_irqrestore(&gdma_lock, flags);
    return 0; // Channels are all busy

}

/**
 * @brief Set channel is masked
 *
 * When channel is masked, the GDMA transaction will stop. 
 * When GDMA controller comes back from another channel (chain feature)
 *
 * >> Channel Mask=0: It's strange, and turns on related bit in GDMA interrupt
 * status register (16:23 Unmasked)
 *
 * >> Channel Mask=1: It'll start GDMA transation, and clear this bit. 
 *
 * @param  ChNum   	GDMA channel number
 * @retval 1  	   	success
 * @retval 0  	   	fail
 */
int GdmaMaskChannel(uint32_t ChNum)
{
    uint32_t Data=0;

    Data=GDMA_READ_REG(GDMA_CTRL_REG1(ChNum));
    Data |= ( 0x01 << CH_MASK_OFFSET); 
    GDMA_WRITE_REG(GDMA_CTRL_REG1(ChNum), Data);
    GDMA_PRINT("%s: Write %0X to %X\n", __FUNCTION__, Data, GDMA_CTRL_REG1(ChNum));

    return 1;
}

/**
 * @brief Set channel is unmasked
 *
 * You can unmask the channel to start GDMA transaction. 
 *
 * When GDMA controller comes back from another channel (chain feature)
 *
 * >> Channel Mask=0: It's strange, and turns on related bit in GDMA interrupt
 * status register (16:23 Unmasked)
 *
 * >> Channel Mask=1: It'll start GDMA transation, and clear this bit. 
 *
 * @param  ChNum   	GDMA channel number
 * @retval 1  	   	success
 * @retval 0  	   	fail
 */
int GdmaUnMaskChannel(uint32_t ChNum)
{
    uint32_t Data=0;

    Data=GDMA_READ_REG(GDMA_CTRL_REG1(ChNum));
    Data &= ~( 0x01 << CH_MASK_OFFSET); 
    GDMA_WRITE_REG(GDMA_CTRL_REG1(ChNum), Data);
    GDMA_PRINT("%s: Write %0X to %X\n", __FUNCTION__, Data, GDMA_CTRL_REG1(ChNum));

    return 1;
}

/**
 * @brief Insert new GDMA entry to start GDMA transaction
 *
 * @param  ChNum   	GDMA channel number
 * @retval 1  	   	success
 * @retval 0  	   	fail
 */
int GdmaReqQuickIns(uint32_t ChNum)
{
    uint32_t Data=0;

    //Mask Channel
    Data = GDMA_READ_REG(GDMA_CTRL_REG1(ChNum));
    Data |= ( 0x1 << CH_MASK_OFFSET); 
    GDMA_WRITE_REG(GDMA_CTRL_REG1(ChNum), Data);

    //Channel Enable
    Data = GDMA_READ_REG(GDMA_CTRL_REG(ChNum));
    Data |= (0x01<<CH_EBL_OFFSET); 
    GDMA_WRITE_REG(GDMA_CTRL_REG(ChNum), Data);

    return 1;

}

int _GdmaReqEntryIns(GdmaReqEntry *NewEntry)
{
    uint32_t Data=0;

    GDMA_PRINT("== << GDMA Control Reg (Channel=%d) >> ===\n", NewEntry->ChNum);
    GDMA_PRINT(" Channel Source Addr = %x \n", NewEntry->Src);
    GDMA_PRINT(" Channel Dest Addr = %x \n", NewEntry->Dst);
    GDMA_PRINT(" Transfer Count=%d\n", NewEntry->TransCount);
    GDMA_PRINT(" Source DMA Req= DMA_REQ%d\n", NewEntry->SrcReqNum);
    GDMA_PRINT(" Dest DMA Req= DMA_REQ%d\n", NewEntry->DstReqNum);
    GDMA_PRINT(" Source Burst Mode=%s\n", NewEntry->SrcBurstMode ? "Fix" : "Inc");
    GDMA_PRINT(" Dest Burst Mode=%s\n", NewEntry->DstBurstMode ? "Fix" : "Inc");
    GDMA_PRINT(" Burst Size=%s\n", NewEntry->BurstSize ==0 ? "1 transfer" : \
	    NewEntry->BurstSize ==1 ? "2 transfer" :\
	    NewEntry->BurstSize ==2 ? "4 transfer" :\
	    NewEntry->BurstSize ==3 ? "8 transfer" :\
	    NewEntry->BurstSize ==4 ? "16 transfer" :\
	    "Error");
    GDMA_PRINT(" Hardware/Software Mode = %s\n", NewEntry->SoftMode ?
	    "Soft" : "Hw");
    GDMA_PRINT("== << GDMA Control Reg1 (Channel=%d) >> =\n", NewEntry->ChNum);
    GDMA_PRINT("Channel Unmasked Int=%s\n", NewEntry->ChUnMaskIntEbl ? 
	    "Enable" : "Disable");
    GDMA_PRINT("Next Unmasked =%d\n", NewEntry->NextUnMaskCh);
    GDMA_PRINT("Ch Mask=%d\n", NewEntry->ChMask);
    GDMA_PRINT("========================================\n");

    GDMA_WRITE_REG(GDMA_SRC_REG(NewEntry->ChNum), NewEntry->Src);
    GDMA_PRINT("SrcAddr: Write %0X to %X\n", \
	    NewEntry->Src, GDMA_SRC_REG(NewEntry->ChNum));

    GDMA_WRITE_REG(GDMA_DST_REG(NewEntry->ChNum), NewEntry->Dst);
    GDMA_PRINT("DstAddr: Write %0X to %X\n", \
	    NewEntry->Dst, GDMA_DST_REG(NewEntry->ChNum));

    Data = ( NewEntry->ChUnMaskIntEbl << CH_UNMASK_INTEBL_OFFSET); 
    Data |= ( (NewEntry->NextUnMaskCh&0x7) << NEXT_UNMASK_CH_OFFSET); 
    Data |= ( NewEntry->ChMask << CH_MASK_OFFSET); 

    if(NewEntry->UnMaskIntCallback!=NULL) {
	Data |= (0x01<<CH_UNMASK_INTEBL_OFFSET); 
	GdmaUnMaskIntCallback[NewEntry->ChNum] = NewEntry->UnMaskIntCallback;
    }

    GDMA_WRITE_REG(GDMA_CTRL_REG1(NewEntry->ChNum), Data);
    GDMA_PRINT("CTRL1: Write %0X to %X\n", Data, GDMA_CTRL_REG1(NewEntry->ChNum));

    Data = ((NewEntry->TransCount) << TRANS_CNT_OFFSET); 
    Data |= (NewEntry->SrcReqNum << SRC_DMA_REQ_OFFSET); 
    Data |= (NewEntry->DstReqNum << DST_DMA_REQ_OFFSET); 
    Data |= (NewEntry->SrcBurstMode << SRC_BRST_MODE_OFFSET); 
    Data |= (NewEntry->DstBurstMode << DST_BRST_MODE_OFFSET); 
    Data |= (NewEntry->BurstSize << BRST_SIZE_OFFSET); 

    if(NewEntry->TxDoneCallback!=NULL) {
	Data |= (0x01<<INT_EBL_OFFSET); 
	GdmaTxDoneCallback[NewEntry->ChNum] = NewEntry->TxDoneCallback;
    }

    if(NewEntry->SoftMode) {
	Data |= (0x01<<MODE_SEL_OFFSET); 
    }


    Data |= (0x01<<CH_EBL_OFFSET); 
    GDMA_WRITE_REG(GDMA_CTRL_REG(NewEntry->ChNum), Data);
    GDMA_PRINT("CTRL: Write %0X to %X\n", Data, GDMA_CTRL_REG(NewEntry->ChNum));

    //if there is no interrupt handler, this function will 
    //return 1 until GDMA done.
    if(NewEntry->TxDoneCallback==NULL) { 
	//wait for GDMA processing done
	while((GDMA_READ_REG(RALINK_GDMAISTS) & 
		    (0x1<<NewEntry->ChNum))==0); 
	//write 1 clear
	GDMA_WRITE_REG(RALINK_GDMAISTS, 1<< NewEntry->ChNum); 
    }

    return 1;

}


/**
 * @brief Start GDMA transaction for sending data to I2S
 *
 * @param  *Src   	source address
 * @param  *Dst    	destination address
 * @param  TxNo    	I2S Tx number 
 * @param  TransCount  	data length
 * @param  *TxDoneCallback  callback function when transcation is done
 * @param  *UnMaskIntCallback  callback func when ch mask field is incorrect
 * @retval 1  	   	success
 * @retval 0  	   	fail
 */
int GdmaI2sTx(
	uint32_t Src, 
	uint32_t Dst, 
	uint8_t TxNo,
	uint16_t TransCount,
	void (*TxDoneCallback)(uint32_t data),
	void (*UnMaskIntCallback)(uint32_t data)
	)
{
    GdmaReqEntry Entry;

    Entry.Src= (Src & 0x1FFFFFFF);
    Entry.Dst= (Dst & 0x1FFFFFFF);
    Entry.TransCount = TransCount;
    Entry.SrcBurstMode=INC_MODE;
    Entry.DstBurstMode=FIX_MODE;
    Entry.BurstSize=BUSTER_SIZE_4B; 
    Entry.SrcReqNum=DMA_MEM_REQ;
    Entry.DstReqNum=DMA_I2S_REQ;
    Entry.TxDoneCallback=TxDoneCallback;
    Entry.UnMaskIntCallback=UnMaskIntCallback;
    Entry.SoftMode=0;
    Entry.ChUnMaskIntEbl=0;
    Entry.ChMask=1;

    if(TxNo==0) { //TX0
	//enable chain feature
	Entry.ChNum=GDMA_I2S_TX0;
	Entry.NextUnMaskCh=GDMA_I2S_TX1;
    }else if(TxNo==1) { //TX1
	//enable chain feature
	Entry.ChNum=GDMA_I2S_TX1;
	Entry.NextUnMaskCh=GDMA_I2S_TX0;
    }else {
	GDMA_PRINT("I2S Tx Number %x is invalid\n", TxNo);
	return 0;
    }

    return _GdmaReqEntryIns(&Entry);

}


/**
 * @brief Start GDMA transaction for receiving data from PCM
 *
 * @param  *Src   	source address
 * @param  *Dst    	destination address
 * @param  TransCount   data length
 * @param  PcmNo    	PCM channel
 * @param  RxNo    	PCM Rx number 
 * @param  *TxDoneCallback  callback function when transcation is done
 * @param  *UnMaskIntCallback  callback func when ch mask field is incorrect
 * @retval 1  	   	success
 * @retval 0  	   	fail
 */
int GdmaPcmRx(
	uint32_t Src, 
	uint32_t Dst, 
	uint8_t PcmNo,
	uint8_t RxNo,
	uint16_t TransCount, 
	void (*TxDoneCallback)(uint32_t data),
	void (*UnMaskIntCallback)(uint32_t data)
	)
{
    GdmaReqEntry Entry;

    Entry.Src= (Src & 0x1FFFFFFF);
    Entry.Dst= (Dst & 0x1FFFFFFF);
    Entry.TransCount = TransCount;
    Entry.SrcBurstMode=FIX_MODE;
    Entry.DstBurstMode=INC_MODE;
    Entry.BurstSize=BUSTER_SIZE_4B; 
    Entry.DstReqNum=DMA_MEM_REQ; 
    Entry.TxDoneCallback=TxDoneCallback;
    Entry.UnMaskIntCallback=UnMaskIntCallback;
    Entry.SoftMode=0;
    Entry.ChUnMaskIntEbl=0;
    Entry.ChMask=1;

    if(PcmNo==0){//PCM0
	Entry.SrcReqNum=DMA_PCM_RX0_REQ;
	if(RxNo==0) { //RX0
	    //enable chain feature
	    Entry.ChNum=GDMA_PCM0_RX0;
	    Entry.NextUnMaskCh=GDMA_PCM0_RX1;
	}else if(RxNo==1) { //RX1
	    //enable chain feature
	    Entry.ChNum=GDMA_PCM0_RX1;
	    Entry.NextUnMaskCh=GDMA_PCM0_RX0;
	}else {
	    GDMA_PRINT("PCM Rx Number %x is invalid\n", RxNo);
	    return 0;
	}
    }else if(PcmNo==1) {//PCM1
        Entry.SrcReqNum=DMA_PCM_RX1_REQ;
	if(RxNo==0) { //RX0
	    //enable chain feature
	    Entry.ChNum=GDMA_PCM1_RX0;
	    Entry.NextUnMaskCh=GDMA_PCM1_RX1;
	}else if(RxNo==1) { //RX1
	    //enable chain feature
	    Entry.ChNum=GDMA_PCM1_RX1;
	    Entry.NextUnMaskCh=GDMA_PCM1_RX0;
	}else {
	    GDMA_PRINT("PCM Rx Number %x is invalid\n", RxNo);
	    return 0;
	}
    }else {
	GDMA_PRINT("PCM Channel %x is invalid\n", PcmNo);
	return 0;
    }


    return _GdmaReqEntryIns(&Entry);

}

/**
 * @brief Start GDMA transaction for sending data to PCM
 *
 * @param  *Src		    source address
 * @param  *Dst		    destination address
 * @param  TransCount	    data length
 * @param  PcmNo	    PCM channel
 * @param  TxNo		    PCM Tx number 
 * @param  *TxDoneCallback  callback func when transcation is done
 * @param  *UnMaskIntCallback  callback func when ch mask field is incorrect
 * @retval 1		    success
 * @retval 0		    fail
 */
int GdmaPcmTx(
	uint32_t Src, 
	uint32_t Dst, 
	uint8_t PcmNo,
	uint8_t TxNo,
	uint16_t TransCount, 
	void (*TxDoneCallback)(uint32_t data),
	void (*UnMaskIntCallback)(uint32_t data)
	)
{
    GdmaReqEntry Entry;

    Entry.Src= (Src & 0x1FFFFFFF);
    Entry.Dst= (Dst & 0x1FFFFFFF);
    Entry.TransCount = TransCount;
    Entry.SrcBurstMode=INC_MODE;
    Entry.DstBurstMode=FIX_MODE;
    Entry.BurstSize=BUSTER_SIZE_4B; 
    Entry.SrcReqNum=DMA_MEM_REQ; 
    Entry.TxDoneCallback=TxDoneCallback;
    Entry.UnMaskIntCallback=UnMaskIntCallback;
    Entry.SoftMode=0; //Hardware Mode
    Entry.ChUnMaskIntEbl=0;
    Entry.ChMask=1;

    if(PcmNo==0){//PCM0
	Entry.DstReqNum=DMA_PCM_TX0_REQ;
	if(TxNo==0) { //TX0
	    //enable chain feature
	    Entry.ChNum=GDMA_PCM0_TX0;
	    Entry.NextUnMaskCh=GDMA_PCM0_TX1;
	}else if(TxNo==1) { //TX1
	    //enable chain feature
	    Entry.ChNum=GDMA_PCM0_TX1;
	    Entry.NextUnMaskCh=GDMA_PCM0_TX0;
	}else {
	    GDMA_PRINT("PCM Tx Number %x is invalid\n", TxNo);
	    return 0;
	}
    }else if(PcmNo==1) {//PCM1
	Entry.DstReqNum=DMA_PCM_TX1_REQ;
	if(TxNo==0) { //TX0
	    //enable chain feature
	    Entry.ChNum=GDMA_PCM1_TX0;
	    Entry.NextUnMaskCh=GDMA_PCM1_TX1;
	}else if(TxNo==1) { //TX1
	    //enable chain feature
	    Entry.ChNum=GDMA_PCM1_TX1;
	    Entry.NextUnMaskCh=GDMA_PCM1_TX0;
	}else {
	    GDMA_PRINT("PCM Tx Number %x is invalid\n", TxNo);
	    return 0;
	}
    }else {
	GDMA_PRINT("PCM Channel %x is invalid\n", PcmNo);
	return 0;
    }

    return _GdmaReqEntryIns(&Entry);

}

/**
 * @brief Start GDMA transaction for memory to memory copy
 *
 * @param  *Src		    source address
 * @param  *Dst		    destination address
 * @param  TransCount	    data length
 * @param  *TxDoneCallback  callback function when transcation is done
 * @retval 1		    success
 * @retval 0		    fail
 */
int GdmaMem2Mem(
	uint32_t Src, 
	uint32_t Dst, 
	uint16_t TransCount,
	void (*TxDoneCallback)(uint32_t data)
	)

{
    GdmaReqEntry Entry;

    Entry.Src= (Src & 0x1FFFFFFF);
    Entry.Dst= (Dst & 0x1FFFFFFF);
    Entry.TransCount = TransCount;
    Entry.SrcBurstMode=INC_MODE;
    Entry.DstBurstMode=INC_MODE;
    Entry.BurstSize=BUSTER_SIZE_32B; 
    Entry.SrcReqNum=DMA_MEM_REQ; 
    Entry.DstReqNum=DMA_MEM_REQ; 
    Entry.TxDoneCallback=TxDoneCallback;
    Entry.UnMaskIntCallback=NULL;
    Entry.SoftMode=1;
    Entry.ChUnMaskIntEbl=0;
    Entry.ChMask=0;

    //No reserved channel for Memory to Memory GDMA,
    //get free channel on demand
    if(!_GdmaGetFreeCh(&Entry.ChNum)) {
	GDMA_PRINT("GDMA Channels are all busy\n");
	return 0;
    }


    //set next channel to their own channel 
    //to disable chain feature
    Entry.NextUnMaskCh= Entry.ChNum;
    
    //set next channel to another channel
    //to enable chain feature
    //Entry.NextUnMaskCh= (Entry.ChNum+1) % MAX_GDMA_CHANNEL;

    return _GdmaReqEntryIns(&Entry);

}

/**
 * @brief GDMA interrupt handler 
 *
 * When GDMA transcation is done, call related handler 
 * to do the remain job.
 *
 */
irqreturn_t GdmaIrqHandler(
	int irq, 
	void *irqaction
	)
{

    u32 Ch=0;
    u32 flags;
    u32 GdmaStatus=GDMA_READ_REG(RALINK_GDMAISTS);

    GDMA_PRINT("Rcv Gdma Interrupt=%x\n",GdmaStatus);

    spin_lock_irqsave(&gdma_int_lock, flags);
    
    //UnMask error
    for(Ch=0;Ch<MAX_GDMA_CHANNEL;Ch++) {
	
	if(GdmaStatus & (0x1 << (Ch+UMASK_INT_STATUS_OFFSET)) ) {
	    if(GdmaUnMaskIntCallback[Ch] != NULL) {
		//write 1 clear
		GDMA_WRITE_REG(RALINK_GDMAISTS, 
			1<< (Ch + UMASK_INT_STATUS_OFFSET)); 

		GdmaUnMaskIntCallback[Ch](Ch); 
	    }
	}
     }	
     
     //processing done
     for(Ch=0;Ch<MAX_GDMA_CHANNEL;Ch++) {
	if(GdmaStatus & (0x1<<Ch)) {
	    if(GdmaTxDoneCallback[Ch] != NULL) {
		//write 1 clear
		GDMA_WRITE_REG(RALINK_GDMAISTS, 
			1<< (Ch + TX_DONE_INT_STATUS_OFFSET)); 

		GdmaTxDoneCallback[Ch](Ch); 
	    }
	}
      
	
    }
    spin_unlock_irqrestore(&gdma_int_lock, flags);

    return IRQ_HANDLED;

}

static int RalinkGdmaInit(void)
{

    uint32_t Ret=0;
    GDMA_PRINT("Enable Ralink GDMA Controller Module\n");

    Ret = request_irq(SURFBOARDINT_DMA, GdmaIrqHandler, \
	    SA_INTERRUPT, "Ralink_DMA", NULL);

    if(Ret){
	GDMA_PRINT("IRQ %d is not free.\n", SURFBOARDINT_DMA);
	return 1;
    }

    //Enable GDMA interrupt
    GDMA_WRITE_REG(RALINK_REG_INTENA, RALINK_INTCTL_DMA);

    //Channel0~Channel7 are round-robin
    GDMA_WRITE_REG(RALINK_GDMAGCT, 0x01);

    return 0;
}

static void __exit RalinkGdmaExit(void)
{

    GDMA_PRINT("Disable Ralink GDMA Controller Module\n");

    //Disable GDMA interrupt
    GDMA_WRITE_REG(RALINK_REG_INTDIS, RALINK_INTCTL_DMA);

    free_irq(SURFBOARDINT_DMA, NULL);
}

module_init(RalinkGdmaInit);
module_exit(RalinkGdmaExit);

EXPORT_SYMBOL(GdmaI2sTx);
EXPORT_SYMBOL(GdmaPcmRx);
EXPORT_SYMBOL(GdmaPcmTx);
EXPORT_SYMBOL(GdmaMem2Mem);
EXPORT_SYMBOL(GdmaReqQuickIns);
EXPORT_SYMBOL(GdmaMaskChannel);
EXPORT_SYMBOL(GdmaUnMaskChannel);


MODULE_DESCRIPTION("Ralink SoC GDMA Controller API Module");
MODULE_AUTHOR("Steven Liu <steven_liu@ralinktech.com.tw>");
MODULE_LICENSE("GPL");
MODULE_VERSION(MOD_VERSION);

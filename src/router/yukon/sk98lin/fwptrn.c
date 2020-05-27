/******************************************************************************
 *
 * Name:    fwptrn.c
 * Project: firmware common modules
 * Version: $Revision: #4 $
 * Date:    $Date: 2010/11/04 $
 * Purpose: firmware pattern function
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	LICENSE:
 *	(C)Copyright Marvell.
 *	
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *	
 *	The information in this file is provided "AS IS" without warranty.
 *	/LICENSE
 *
 ******************************************************************************/

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"

static SK_I8 FwWriteIpmiPattern( SK_AC *pAC, SK_IOC IoC, SK_U8 port);
static SK_I8 FwWritePatternRamEx(SK_AC *pAC,
SK_IOC IoC, SK_U8 Port,
SK_U8 PatternId ,
SK_U8 Length ,
SK_U8 *pMask ,
SK_U8 *pPattern );
static SK_I8 FwWritePatternRam(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8   Port,
SK_U8   PatternId1,
SK_U8   PatternId2,
SK_U8   Length1,
SK_U8   Length2,
SK_U8   *pMask1,
SK_U8   *pPattern1,
SK_U8   *pMask2,
SK_U8   *pPattern2);

/*  ARP pattern 40 byte (5 bytes in mask) */
/*  this pattern length corresponds with YLCI_MACRXFIFOTHRES */
/*  Pattern mask for ARP Frames */
#ifdef ASF_ONLY_ARP_REQUEST
static SK_U8 ARP_FRAME_PATTERN[] =
{
    /* MAC Header - 14 bytes */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /*Dest MAC Addr */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /*Src MAC Addr  */
    0x08, 0x06,                              /*Frame Type    */
    /* ARP Header - 28 bytes */
    0x00, 0x01,                              /* hard type    */
    0x08, 0x00,                              /* prot type    */
    0x06,                                    /* hard size    */
    0x04,                                    /* prot size    */
    0x00, 0x01,                              /* op = request */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /* senders mac  */
    0x00, 0x00, 0x00, 0x00,                  /* senders ip   */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /* target mac   */
    0x00, 0x00};
static SK_U8 ARP_PATTERN_MASK[] = { 0x00, 0xF0, 0x3F, 0x00, 0x00 };
#else
static SK_U8 ARP_FRAME_PATTERN[] =
{
    /* MAC Header - 14 bytes */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /*Dest MAC Addr */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /*Src MAC Addr  */
    0x08, 0x06,                              /*Frame Type    */
    /* ARP Header - 28 bytes */
    0x00, 0x01,                              /* hard type    */
    0x08, 0x00,                              /* prot type    */
    0x06,                                    /* hard size    */
    0x04,                                    /* prot size    */
    0x00, 0x00,                              /* op = request */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /* senders mac  */
    0x00, 0x00, 0x00, 0x00,                  /* senders ip   */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /* target mac   */
    0x00, 0x00};
static SK_U8 ARP_PATTERN_MASK[] = { 0x00, 0xF0, 0x00, 0x00, 0x00 };
#endif

/*  RSP pattern - 40 bytes (this makes 5 bytes in RSP_PATTERN_MASK) */
/*  this pattern length corresponds with YLCI_MACRXFIFOTHRES */
static SK_U8 RSP_FRAME_PATTERN[] =
{   /* MAC Header (14 bytes) */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /*Dest MAC Addr*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /*Src MAC Addr */
    0x08, 0x00,                             /*Frame Type   */
    /* IP Header (20 bytes) */
    0x45, 0x00, 0x00, 0x00,                 /* Version & Header Length */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x11, 0x00, 0x00,                 /* Protocol */
    0x00, 0x00, 0x00, 0x00,                 /*Src IP address*/
    0x00, 0x00, 0x00, 0x00,                 /*My IP address*/
    /* part of UDP Header (6 bytes) */
    0x00, 0x00,                             /* src port   */
    0x02, 0x98,                             /* dest. port */
    0x00, 0x00};                            /* length     */

/*  Pattern mask for RSP Frames */
static SK_U8 RSP_PATTERN_MASK[] = { 0x00, 0x70, 0x80, 0x00, 0x30 };

/*  RMCP pattern (unsecure port) */
/*  this pattern length corresponds with YLCI_MACRXFIFOTHRES */
static SK_U8 RMCP_FRAME_PATTERN[] =
{   /* MAC Header */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /*Dest MAC Addr*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /*Src MAC Addr */
    0x08, 0x00,                              /*Frame Type   */
    /* IP Header */
    0x45, 0x00, 0x00, 0x00,                  /* Version & Header Length */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x11, 0x00, 0x00,                  /* Protocol */
    0x00, 0x00, 0x00, 0x00,                  /*Src IP address*/
    0x00, 0x00, 0x00, 0x00,                  /*My IP address*/
    /* UDP Header */
    0x00, 0x00,                             /* src port */
    0x02, 0x6f,                             /* unsecure dest. port */
    0x00, 0x00};

/*  Pattern mask for RMCP Frames */
static SK_U8 RMCP_PATTERN_MASK[] = { 0x00, 0x70, 0x80, 0x00, 0x30 };

/*  ICMP pattern */
static SK_U8 ICMP_FRAME_PATTERN[] =
{   /* MAC Header */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /*Dest MAC Addr*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      /*Src MAC Addr */
    0x08, 0x00,                              /*Frame Type   */
    /* IP Header */
    0x45, 0x00, 0x00, 0x00,                  /* Version & Header Length */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00,                  /* Protocol */
    0x00, 0x00, 0x00, 0x00,                  /*Src IP address*/
    0x00, 0x00, 0x00, 0x00,                  /*My IP address*/
    /* ICMP Header */
    0x00, 0x00,                          
    0x00, 0x00,                      
    0x00, 0x00};

/*  Pattern mask for ICMP Frames */
static SK_U8 ICMP_PATTERN_MASK[] = 
{ 0x00, 0x70, 0x80, 0x00, 0x00 };

/*****************************************************************************
*
* FwWriteIpmiPattern
*
* Description:  write all 3 pattern for IPMI to pattern ram
*
* Notes:        none
*
* Context:      none
*
* Returns:      1:  SUCCESS
*               0:  ERROR
*
*/

static SK_I8 FwWriteIpmiPattern(
	SK_AC   *pAC,
	SK_IOC  IoC,
	SK_U8   port)
{
	SK_I8   RetVal;
	SK_U8   i, j;
	SK_U32  idx;
	SK_U32  PattRamCluster[ASF_YEC_PATTRAM_CLUSTER_WORDS];
	SK_U8   PattSrcByte[2];     /*  2 pattern bytes to read/write in one cluster */
	SK_U32  TmpVal32;
	SK_U32  mask;
	SK_U8   pattern_no;

	RetVal = 1;     /*  success */

	if (RetVal == 1) {

		for (i = 0; (i < ASF_YEC_PATTRAM_CLUSTER_SIZE) && (RetVal == 1); i++) {
			/*  pattern ram is organized into cluster (i is cluster index) */
			/*  _after_ writing a whole cluster (= 128bit), the pattern will be written into ram! */

			/*  read a cluster */
			for (j=0; j < ASF_YEC_PATTRAM_CLUSTER_WORDS; j++) {
				PattRamCluster[j] = 0;
			}

			/* ----------------------- */
			/*  read source pattern 6 */
			/* ----------------------- */
			pattern_no = 6;
			for (j=0; j < 2; j++) {
				/*  we have 2 pattern bytes to read/write for one cluster */
				/*  i is cluster index */
				/*  j is byte index */
				idx = 2*i+j;
				if (idx < 40) {
					/*  we can read from our pattern pointer */
					PattSrcByte[j] = RMCP_FRAME_PATTERN[idx];

					/*  read from our enable mask pointer */
					TmpVal32 = RMCP_PATTERN_MASK[idx/8];
					mask     = 0x01 << (idx % 8);

					if ( (TmpVal32 & mask) != 0 ) {
						/*  enable byte */
						PattRamCluster[j*2+1] |= (BIT_24 << pattern_no);
					}
					else {
						/*  disable byte */
						PattRamCluster[j*2+1] &= ~(BIT_24 << pattern_no);
					}
				}
				else {
					/*  fill up with zeros */
					PattSrcByte[j] = 0;
					/*  disable byte */
					PattRamCluster[j*2+1] &= ~(BIT_24 << pattern_no);
				}
			}

			/*  upper words are interesting here */
			idx = 1;

			/*  first byte */
			mask = 0x000000ff;
			j    = pattern_no % 4;
			PattRamCluster[idx] &= ~(mask << 8 * j);    /*  delete byte in word */
			mask = PattSrcByte[0];
			PattRamCluster[idx] |= (mask << 8 * j);     /*  write pattern byte */

			/*  second byte */
			mask = 0x000000ff;
			PattRamCluster[idx+2] &= ~(mask << 8 * j);  /*  delete byte in word */
			mask = PattSrcByte[1];
			PattRamCluster[idx+2] |= (mask << 8 * j);   /*  write pattern byte */

			/* ----------------------- */
			/*  read source pattern 5 */
			/* ----------------------- */
			pattern_no = 5;
			for (j=0; j < 2; j++) {
				/*  we have 2 pattern bytes to read/write for one cluster */
				/*  i is cluster index */
				/*  j is byte index */
				idx = 2*i+j;
				if (idx < 40) {
					/*  we can read from our pattern pointer */
					PattSrcByte[j] = ARP_FRAME_PATTERN[idx];

					/*  read from our enable mask pointer */
					TmpVal32 = ARP_PATTERN_MASK[idx/8];
					mask     = 0x01 << (idx % 8);

					if ( (TmpVal32 & mask) != 0 ) {
						/*  enable byte */
						PattRamCluster[j*2+1] |= (BIT_24 << pattern_no);
					}
					else {
						/*  disable byte */
						PattRamCluster[j*2+1] &= ~(BIT_24 << pattern_no);
					}
				}
				else {
					/*  fill up with zeros */
					PattSrcByte[j] = 0;
					/*  disable byte */
					PattRamCluster[j*2+1] &= ~(BIT_24 << pattern_no);
				}
			}

			/*  upper words are interesting here */
			idx = 1;

			/*  first byte */
			mask = 0x000000ff;
			j    = pattern_no % 4;
			PattRamCluster[idx] &= ~(mask << 8 * j);    /*  delete byte in word */
			mask = PattSrcByte[0];
			PattRamCluster[idx] |= (mask << 8 * j);     /*  write pattern byte */

			/*  second byte */
			mask = 0x000000ff;
			PattRamCluster[idx+2] &= ~(mask << 8 * j);  /*  delete byte in word */
			mask = PattSrcByte[1];
			PattRamCluster[idx+2] |= (mask << 8 * j);   /*  write pattern byte */

			/* ----------------------- */
			/*  read source pattern 4 */
			/* ----------------------- */
			pattern_no = 4;
			for (j=0; j < 2; j++) {
				/*  we have 2 pattern bytes to read/write for one cluster */
				/*  i is cluster index */
				/*  j is byte index */
				idx = 2*i+j;
				if (idx < 40) {
					/*  we can read from our pattern pointer */
					PattSrcByte[j] = RSP_FRAME_PATTERN[idx];

					/*  read from our enable mask pointer */
					TmpVal32 = RSP_PATTERN_MASK[idx/8];
					mask     = 0x01 << (idx % 8);

					if ( (TmpVal32 & mask) != 0 ) {
						/*  enable byte */
						PattRamCluster[j*2+1] |= (BIT_24 << pattern_no);
					}
					else {
						/*  disable byte */
						PattRamCluster[j*2+1] &= ~(BIT_24 << pattern_no);
					}
				}
				else {
					/*  fill up with zeros */
					PattSrcByte[j] = 0;
					/*  disable byte */
					PattRamCluster[j*2+1] &= ~(BIT_24 << pattern_no);
				}
			}

			/*  upper words are interesting here */
			idx = 1;

			/*  first byte */
			mask = 0x000000ff;
			j    = pattern_no % 4;
			PattRamCluster[idx] &= ~(mask << 8 * j);    /*  delete byte in word */
			mask = PattSrcByte[0];
			PattRamCluster[idx] |= (mask << 8 * j);     /*  write pattern byte */

			/*  second byte */
			mask = 0x000000ff;
			PattRamCluster[idx+2] &= ~(mask << 8 * j);  /*  delete byte in word */
			mask = PattSrcByte[1];
			PattRamCluster[idx+2] |= (mask << 8 * j);   /*  write pattern byte */

			/*  write a cluster */
			/*  after writing the last cluster word, the hardware will trigger writing all cluster words */
			for (j=0; j < ASF_YEC_PATTRAM_CLUSTER_WORDS; j++) {
				idx = ASF_YEC_PATTRAM_CLUSTER_WORDS*ASF_YEC_PATTRAM_CLUSTER_BYTES*i + ASF_YEC_PATTRAM_CLUSTER_BYTES*j;

				if( port == 0 )  {
					SK_OUT32( IoC, WOL_PATT_RAM_1+idx, PattRamCluster[j] );
				}
				else  {
					SK_OUT32( IoC, WOL_PATT_RAM_2+idx, PattRamCluster[j] );
				}
			}
		}
	}

	return (RetVal);
}

void FwCheckPattern(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8   port )  {

	SK_U16 val16;

	/*   WAR: sometimes, for some reasons the ARP and ICMP pattern remain */
	/*        enabled after disable/enable the driver or rebooting the system. */
	if( pAC->FwApp.ChipMode == SK_GEASF_CHIP_SU )  {
		/*  set ASF match enable register (incomming packets will redirect to ASF queue) */
		SK_IN16(IoC, ASF_YEX_PATTERN_ENA1+(port*0x80), &val16);
		if( (val16 & (0x01 << ASF_DASH_PATTERN_NUM_ARP)) || 
			 (val16 & (0x01 << ASF_DASH_PATTERN_NUM_ICMP))  )  {
			pAC->FwApp.ToPatternCheck++;
			if( pAC->FwApp.ToPatternCheck >= 15 )  {
				if(pAC->FwApp.FwState & BIT_1) { /*  Bit 1 set means port disabled */
					pAC->FwApp.ToPatternCheck = 0; /*  avoid disable pattern in this case */
				} else {
					FwDisablePattern( pAC, IoC, port, ASF_DASH_PATTERN_NUM_ARP );		
					FwDisablePattern( pAC, IoC, port, ASF_DASH_PATTERN_NUM_ICMP );		
				}
			}
		}
		else
			pAC->FwApp.ToPatternCheck = 0;
	}

}

void FwSetUpPattern(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8   port )  {

	SK_U8   TmpVal8;
	SK_U16  TmpVal16;
	SK_U32  TmpVal32;

	if( port > 1 )
		return;

	switch (pAC->FwApp.OpMode) {
		case SK_GEASF_MODE_ASF:
			/*  see "problems.txt" note #344 */
			/*  set MAC Rx fifo flush threshold register */
			/*  (please refer also to note #yuk_hw01) */
			SK_IN32(IoC, ASF_YEC_MAC_FIFO_FLUSHTHRES1+(port*0x80), &TmpVal32);
			TmpVal32 &= ~0x7f;  /*  delete bit 6:0 */
			TmpVal32 |= ASF_YLCI_MACRXFIFOTHRES;
			SK_OUT32(IoC, ASF_YEC_MAC_FIFO_FLUSHTHRES1+(port*0x80), TmpVal32);

			/*  disable Wake Up Frame Unit before write to pattern ram */
			SK_IN16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), &TmpVal16);
			TmpVal16 &= ~0x03;   /*  clear bit 0, bit1 */
			TmpVal16 |=  0x01;   /*  set bit 0 */
			SK_OUT16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), TmpVal16);

			/*   Set new pattern for ASF */
			/*   This will override the driver WOL pattern, but the driver is going to set */
			/*   the WOL pattern again before it "shut down" or "stand by" */

			if (pAC->FwApp.ChipMode == SK_GEASF_CHIP_EX ) {
					/*  write ARP, RMCP and RSS patterns */
					FwWritePatternRamEx(pAC, IoC, port, 7, 40, ARP_PATTERN_MASK, ARP_FRAME_PATTERN );
					FwWritePatternRamEx(pAC, IoC, port, 6, 40, RSP_PATTERN_MASK, RSP_FRAME_PATTERN );
					FwWritePatternRamEx(pAC, IoC, port, 8, 40, RMCP_PATTERN_MASK, RMCP_FRAME_PATTERN );
					if (pAC->FwApp.Mib.Ena) {

						FwEnablePattern( pAC, IoC, port, ASF_PATTERN_ID_RMCP );
						FwEnablePattern( pAC, IoC, port, ASF_PATTERN_ID_RSP );
					}

/* We need not disable asf pattern, they are already disabled if asf disabled
 * if shutdown or standby before parameters from FW are read
 * the patterns are allways disabled #651
 * 
 *
 *					else {
 *						FwDisablePattern( pAC, IoC, port, ASF_PATTERN_ID_RMCP );		
 *						FwDisablePattern( pAC, IoC, port, ASF_PATTERN_ID_RSP );       
 *					}
 */
			}
			else  {
				if( pAC->FwApp.Mib.RspEnable )  {
					/*  security is on - write ARP and RSP pattern */
					FwWritePatternRam(pAC, IoC, port, 5, 6, 40, 40,
										ARP_PATTERN_MASK, ARP_FRAME_PATTERN,
										RSP_PATTERN_MASK, RSP_FRAME_PATTERN);
			}

/* We need not disable asf pattern, they are already disabled if asf disabled
 * if shutdown or standby before parameters from FW are read
 * the patterns are allways disabled #651
 * 
 *
 *				
 *	            else  {
 *	                // security is off - write ARP and RMCP pattern
 * 	                AsfWritePatternRam(pAC, IoC, port, 5, 6, 40, 40,
 *	                                   ARP_PATTERN_MASK, ARP_FRAME_PATTERN,
 *	                                   RMCP_PATTERN_MASK, RMCP_FRAME_PATTERN);
 *				}
 */
				/*  set pattern length register */
				SK_IN32(IoC, ASF_YEC_PATTERN_LENGTH_R1_H+(port*0x80), &TmpVal32);
				TmpVal32 &= ~(0x0000007f << 8*(5%4));
				TmpVal32 |= (40-1) << 8*(5%4);     /*  write length-1 to pattern length register */
				SK_OUT32(IoC, ASF_YEC_PATTERN_LENGTH_R1_H+(port*0x80), TmpVal32);

				/*  set pattern length register */
				SK_IN32(IoC, ASF_YEC_PATTERN_LENGTH_R1_H+(port*0x80), &TmpVal32);
				TmpVal32 &= ~(0x0000007f << 8*(6%4));
				TmpVal32 |= (40-1) << 8*(6%4);     /*  write length-1 to pattern length register */
				SK_OUT32(IoC, ASF_YEC_PATTERN_LENGTH_R1_H+(port*0x80), TmpVal32);

				/*  set ASF match enable register (incomming packets will redirect to ASF queue) */
				SK_IN8(IoC, ASF_YEC_PATTERN_MATCHENA1+(port*0x80), &TmpVal8);
				TmpVal8 |= 0x40;    /*  pattern 6 enable */
				SK_OUT8(IoC, ASF_YEC_PATTERN_MATCHENA1+(port*0x80), TmpVal8);

				/*  enable pattern pattno */
				SK_IN8(IoC, ASF_YEC_PATTERN_ENA1+(port*0x80), &TmpVal8);
				TmpVal8 |= 0x40;    /*  pattern 6 enable */
				SK_OUT8(IoC, ASF_YEC_PATTERN_ENA1+(port*0x80), TmpVal8);
			}

			/*  enable Wake Up Frame Unit */
			SK_IN16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), &TmpVal16);
			TmpVal16 &= ~0x03;   /*  delete bit 0 and 1 */
			TmpVal16 |=  0x02;   /*  set bit 1 */
			SK_OUT16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), TmpVal16);
			break;

		case SK_GEASF_MODE_IPMI:
			/*   currently not supported with Yukon Extreme */
			if( pAC->FwApp.ChipMode == SK_GEASF_CHIP_EX )
				break;
			
			/*  disable Wake Up Frame Unit before write to pattern ram */
			SK_IN16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), &TmpVal16);
			TmpVal16 &= ~0x03;   /*  clear bit 0, bit1 */
			TmpVal16 |=  0x01;   /*  set bit 0 */
			SK_OUT16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), TmpVal16);

			/*  write all 3 pattern (RMCP, RSP and ARP) */
			FwWriteIpmiPattern(pAC, IoC, port);

			/*  set pattern length register */
			SK_IN32(IoC, ASF_YEC_PATTERN_LENGTH_R1_H+(port*0x80), &TmpVal32);
			TmpVal32 &= 0xff000000;     /*  delete length for pattern 4, 5 and 6 */
			TmpVal32 |= 0x00272727;     /*  set new length for pattern 4, 5 and 6 */
			SK_OUT32(IoC, ASF_YEC_PATTERN_LENGTH_R1_H+(port*0x80), TmpVal32);

			/*  set ASF match enable register (incomming packets will redirect to ASF queue) */
			SK_IN8(IoC, ASF_YEC_PATTERN_MATCHENA1+(port*0x80), &TmpVal8);
			TmpVal8 = 0x50;    /*  enable pattern 4 and 6 (do not enable arp pattern) */
			SK_OUT8(IoC, ASF_YEC_PATTERN_MATCHENA1+(port*0x80), TmpVal8);

			/*  enable pattern pattno */
			SK_IN8(IoC, ASF_YEC_PATTERN_ENA1+(port*0x80), &TmpVal8);
			TmpVal8 = 0x50;    /*  enable pattern 4 and 6 (do not enable arp pattern) */
			SK_OUT8(IoC, ASF_YEC_PATTERN_ENA1+(port*0x80), TmpVal8);

			/*  enable Wake Up Frame Unit */
			SK_IN16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), &TmpVal16);
			TmpVal16 &= ~0x03;   /*  delete bit 0 and 1 */
			TmpVal16 |=  0x02;   /*  set bit 1 */
			SK_OUT16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), TmpVal16);
			break;

		case SK_GEASF_MODE_DASH:
		case SK_GEASF_MODE_DASH_ASF:
			/*   currently only supported on Yukon Extreme */
			if( pAC->FwApp.ChipMode != SK_GEASF_CHIP_SU )
				break;

			/*  see "problems.txt" note #344 */
			/*  set MAC Rx fifo flush threshold register */
			/*  (please refer also to note #yuk_hw01) */
			SK_IN32(IoC, ASF_YEC_MAC_FIFO_FLUSHTHRES1+(port*0x80), &TmpVal32);
			TmpVal32 &= ~0x7f;  /*  delete bit 6:0 */
			TmpVal32 |= ASF_YLCI_MACRXFIFOTHRES;
			SK_OUT32(IoC, ASF_YEC_MAC_FIFO_FLUSHTHRES1+(port*0x80), TmpVal32);
			
			/*  disable Wake Up Frame Unit before write to pattern ram */
			SK_IN16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), &TmpVal16);
			TmpVal16 &= ~0x03;   /*  clear bit 0, bit1 */
			TmpVal16 |=  0x01;   /*  set bit 0 */
			SK_OUT16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), TmpVal16);

			/*  write all 6 pattern  */
			FwWritePatternRamEx(pAC, IoC, port, ASF_DASH_PATTERN_NUM_ARP,  40, ARP_PATTERN_MASK, ARP_FRAME_PATTERN );
			FwWritePatternRamEx(pAC, IoC, port, ASF_DASH_PATTERN_NUM_ICMP, 40, ICMP_PATTERN_MASK, ICMP_FRAME_PATTERN );

			/*  all UDP and TCP protocoll pattern will be setup by firmware */
			/* FwWritePatternRamEx(pAC, IoC, port, ASF_DASH_PATTERN_NUM_RMCP, 40, RMCP_PATTERN_MASK, RMCP_FRAME_PATTERN ); */
			/* FwWritePatternRamEx(pAC, IoC, port, ASF_DASH_PATTERN_NUM_RSP,  40, RSP_PATTERN_MASK, RSP_FRAME_PATTERN ); */
			/* FwWritePatternRamEx(pAC, IoC, port, ASF_DASH_PATTERN_NUM_TCP1, 40, TCP_PATTERN_MASK, TCP_FRAME_PATTERN1 ); */
			/* FwWritePatternRamEx(pAC, IoC, port, ASF_DASH_PATTERN_NUM_TCP2, 40, TCP_PATTERN_MASK, TCP_FRAME_PATTERN2 ); */
			
			/* FwEnablePattern( pAC, IoC, port, ASF_DASH_PATTERN_NUM_TCP1 );		 */
			/* FwEnablePattern( pAC, IoC, port, ASF_DASH_PATTERN_NUM_TCP2 );		 */
			/* FwEnablePattern( pAC, IoC, port, ASF_DASH_PATTERN_NUM_RMCP );		 */
			/* FwEnablePattern( pAC, IoC, port, ASF_DASH_PATTERN_NUM_RSP );		 */
			
			FwDisablePattern( pAC, IoC, port, ASF_DASH_PATTERN_NUM_ARP );		
			FwDisablePattern( pAC, IoC, port, ASF_DASH_PATTERN_NUM_ICMP );		

			/*  enable Wake Up Frame Unit */
			SK_IN16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), &TmpVal16);
			TmpVal16 &= ~0x03;   /*  delete bit 0 and 1 */
			TmpVal16 |=  0x02;   /*  set bit 1 */
			SK_OUT16(IoC, ASF_YEC_PATTERN_CTRL1+(port*0x80), TmpVal16);
			break;
			
		default:
			break;
	}

	return;
}

/*****************************************************************************
*
* FwWritePatternRam
*
* Description:  write to pattern ram
*
* Notes:        none
*
* Context:      none
*
* Returns:      1:  SUCCESS
*               0:  ERROR
*
*/

static SK_I8 FwWritePatternRam(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8   Port,                       /* for future use   */
SK_U8   PatternId1,                  /* 0..6             */
SK_U8   PatternId2,                  /* 0..6             */
SK_U8   Length1,                     /* 1..128 bytes     */
SK_U8   Length2,                     /* 1..128 bytes     */
SK_U8   *pMask1,
SK_U8   *pPattern1,
SK_U8   *pMask2,
SK_U8   *pPattern2)

{
	SK_U8   i, j;
	SK_I8   RetVal;
	SK_U32  idx;
	SK_U32  PattRamCluster[ASF_YEC_PATTRAM_CLUSTER_WORDS];
	SK_U8   PattSrcByte1[2];     /*  2 pattern bytes to read/write in one cluster */
	SK_U8   PattSrcByte2[2];     /*  2 pattern bytes to read/write in one cluster */
	SK_U32  TmpVal32;
	SK_U32  mask;

	RetVal = 1;  /*   success */

	/*  pattern size up to 128 bytes, pattern id can be 0...6 */
	if ( (Length1 <= SK_POW_PATTERN_LENGTH) && (PatternId1 < SK_NUM_WOL_PATTERN) ) {

		for (i = 0; (i < ASF_YEC_PATTRAM_CLUSTER_SIZE) && (RetVal == 1); i++) {
			/*  pattern ram is organized into cluster (i is cluster index) */
			/*  _after_ writing a whole cluster (= 128bit), the pattern will be written into ram! */

			/*  read a cluster */
			for (j=0; j < ASF_YEC_PATTRAM_CLUSTER_WORDS; j++) {
				PattRamCluster[j] = 0;
			}
			/*  read source pattern 1 */
			for (j=0; j < 2; j++) {
				/*  we have 2 pattern bytes to read/write for one cluster */
				/*  i is cluster index */
				/*  j is byte index */
				idx = 2*i+j;
				if ( idx < Length1 ) {
					/*  we can read from our pattern pointer */
					PattSrcByte1[j] = pPattern1[idx];

					/*  read from our enable mask pointer */
					TmpVal32 = pMask1[idx/8];
					mask     = 0x01 << (idx % 8);

					if ( (TmpVal32 & mask) != 0 ) {
						/*  enable byte */
						PattRamCluster[j*2+1] |= (BIT_24 << PatternId1);
					}
					else {
						/*  disable byte */
						PattRamCluster[j*2+1] &= ~(BIT_24 << PatternId1);
					}
				}
				else {
					/*  fill up with zeros */
					PattSrcByte1[j] = 0;
					/*  disable byte */
					PattRamCluster[j*2+1] &= ~(BIT_24 << PatternId1);
				}
			}
			/*  read source pattern 2 */
			for (j=0; j < 2; j++) {
				/*  we have 2 pattern bytes to read/write for one cluster */
				/*  i is cluster index */
				/*  j is byte index */
				idx = 2*i+j;
				if ( idx < Length2 ) {
					/*  we can read from our pattern pointer */
					PattSrcByte2[j] = pPattern2[idx];

					/*  read from our enable mask pointer */
					TmpVal32 = pMask2[idx/8];
					mask     = 0x01 << (idx % 8);

					if ( (TmpVal32 & mask) != 0 ) {
						/*  enable byte */
						PattRamCluster[j*2+1] |= (BIT_24 << PatternId2);
					}
					else {
						/*  disable byte */
						PattRamCluster[j*2+1] &= ~(BIT_24 << PatternId2);
					}
				}
				else {
					/*  fill up with zeros */
					PattSrcByte2[j] = 0;
					/*  disable byte */
					PattRamCluster[j*2+1] &= ~(BIT_24 << PatternId2);
				}
			}
			/*  set our pattern into PattRamCluster[] */
			if (PatternId1 >= 4) {
				/*  upper words are interesting here */
				idx = 1;
			}
			else {
				/*  lower words are interesting here */
				idx = 0;
			}
			/*  first byte */
			mask = 0x000000ff;
			j    = PatternId1 % 4;
			PattRamCluster[idx] &= ~(mask << 8 * j);    /*  delete byte in word */
			mask = PattSrcByte1[0];
			PattRamCluster[idx] |= (mask << 8 * j);     /*  write pattern byte */
			/*  second byte */
			mask = 0x000000ff;
			PattRamCluster[idx+2] &= ~(mask << 8 * j);  /*  delete byte in word */
			mask = PattSrcByte1[1];
			PattRamCluster[idx+2] |= (mask << 8 * j);   /*  write pattern byte */

			/*  set our pattern into PattRamCluster[] */
			if (PatternId2 >= 4) {
				/*  upper words are interesting here */
				idx = 1;
			}
			else {
				/*  lower words are interesting here */
				idx = 0;
			}
			/*  first byte */
			mask = 0x000000ff;
			j    = PatternId2 % 4;
			PattRamCluster[idx] &= ~(mask << 8 * j);    /*  delete byte in word */
			mask = PattSrcByte2[0];
			PattRamCluster[idx] |= (mask << 8 * j);     /*  write pattern byte */
			/*  second byte */
			mask = 0x000000ff;
			PattRamCluster[idx+2] &= ~(mask << 8 * j);  /*  delete byte in word */
			mask = PattSrcByte2[1];
			PattRamCluster[idx+2] |= (mask << 8 * j);   /*  write pattern byte */

			/*  write a cluster */
			/*  after writing the last cluster word, the hardware will trigger writing all cluster words */
			for (j=0; j < ASF_YEC_PATTRAM_CLUSTER_WORDS; j++) {
				idx = ASF_YEC_PATTRAM_CLUSTER_WORDS*ASF_YEC_PATTRAM_CLUSTER_BYTES*i + ASF_YEC_PATTRAM_CLUSTER_BYTES*j;
				if( Port == 0 )  {
					SK_OUT32( IoC, WOL_PATT_RAM_1+idx, PattRamCluster[j] );
				}
				else  {
					SK_OUT32( IoC, WOL_PATT_RAM_2+idx, PattRamCluster[j] );
				}
			}
		}
	}
	else {
		RetVal = 0;  /*   error */
	}

	return(RetVal);
}

/*****************************************************************************
*
* FwWritePatternRamEx
*
* Description:  write to pattern ram
*
* Notes:        none
*
* Context:      none
*
* Returns:      1:  SUCCESS
*               0:  ERROR
*
*/

static SK_I8 FwWritePatternRamEx(
SK_AC   *pAC,
SK_IOC  IoC,
SK_U8   Port,                        /* for future use   */
SK_U8   PatternId ,                  /* 0..9             */
SK_U8   Length ,                     /* 1..128 bytes     */
SK_U8   *pMask ,
SK_U8   *pPattern )

{
	SK_U8   i, j;
	SK_I8   RetVal;
	SK_U32  idx;
	SK_U32  PattRamCluster[ASF_YEC_PATTRAM_CLUSTER_WORDS];
	SK_U8   PattSrcByte[2];     /*  2 pattern bytes to read/write in one cluster */
	SK_U32  TmpVal32;
	SK_U32  mask;
	SK_U8   TmpPatternId;
	SK_U8   bank;
	SK_U16  AsfYexPatternLengthReg;
	
	RetVal = 1;  /*   success */

	/*  pattern size up to 128 bytes, pattern id can be 0...6 */
	if ( Length <= SK_POW_PATTERN_LENGTH ) {
		if ( PatternId < SK_NUM_WOL_PATTERN_EX ) {
			
			if( PatternId < SK_NUM_WOL_PATTERN )  {
				TmpPatternId = PatternId;
				bank = 0;
			}
			else  {
				TmpPatternId = PatternId - SK_NUM_WOL_PATTERN;
				bank = 1;
			}

			/*   select pattern bank */
			SK_IN32( IoC, PAT_CSR, &TmpVal32 );
			if( bank == 0 )
				TmpVal32 &= ~(PAT_CSR_BSEL);
			else
				TmpVal32 |= PAT_CSR_BSEL;
			SK_OUT32( IoC, PAT_CSR, TmpVal32 );

			for (i = 0; (i < ASF_YEC_PATTRAM_CLUSTER_SIZE) && (RetVal == 1); i++) {
				/*  pattern ram is organized into cluster (i is cluster index) */
				/*  _after_ writing a whole cluster (= 128bit), the pattern will be written into ram! */

				/*  read a cluster */
				for (j=0; j < ASF_YEC_PATTRAM_CLUSTER_WORDS; j++) {
					idx = ASF_YEC_PATTRAM_CLUSTER_WORDS*ASF_YEC_PATTRAM_CLUSTER_BYTES*i + ASF_YEC_PATTRAM_CLUSTER_BYTES*j;
					if( Port == 0 )  {
						SK_IN32( IoC, WOL_PATT_RAM_1+idx, &PattRamCluster[j] );
					}
					else  {
						SK_IN32( IoC, WOL_PATT_RAM_2+idx, &PattRamCluster[j] );
					}
				}
				/*  read source pattern  */
				for (j=0; j < 2; j++) {
					/*  we have 2 pattern bytes to read/write for one cluster */
					/*  i is cluster index */
					/*  j is byte index */
					idx = 2*i+j;
					if ( idx < Length ) {
						/*  we can read from our pattern pointer */
						PattSrcByte[j] = pPattern[idx];

						/*  read from our enable mask pointer */
						TmpVal32 = pMask[idx/8];
						mask     = 0x01 << (idx % 8);

						if ( (TmpVal32 & mask) != 0 ) {
							/*  enable byte */
							PattRamCluster[j*2+1] |= (BIT_24 << TmpPatternId);
						}
						else {
							/*  disable byte */
							PattRamCluster[j*2+1] &= ~(BIT_24 << TmpPatternId);
						}
					}
					else {
						/*  fill up with zeros */
						PattSrcByte[j] = 0;
						/*  disable byte */
						PattRamCluster[j*2+1] &= ~(BIT_24 << TmpPatternId);
					}
				}
				/*  set our pattern into PattRamCluster[] */
				if (TmpPatternId >= 4) {
					/*  upper words are interesting here */
					idx = 1;
				}
				else {
					/*  lower words are interesting here */
					idx = 0;
				}
				/*  first byte */
				mask = 0x000000ff;
				j    = TmpPatternId % 4;
				PattRamCluster[idx] &= ~(mask << 8 * j);    /*  delete byte in word */
				mask = PattSrcByte[0];
				PattRamCluster[idx] |= (mask << 8 * j);     /*  write pattern byte */
				/*  second byte */
				mask = 0x000000ff;
				PattRamCluster[idx+2] &= ~(mask << 8 * j);  /*  delete byte in word */
				mask = PattSrcByte[1];
				PattRamCluster[idx+2] |= (mask << 8 * j);   /*  write pattern byte */

				/*  write a cluster */
				/*  after writing the last cluster word, the hardware will trigger writing all cluster words */
				for (j=0; j < ASF_YEC_PATTRAM_CLUSTER_WORDS; j++) {
					idx = ASF_YEC_PATTRAM_CLUSTER_WORDS*ASF_YEC_PATTRAM_CLUSTER_BYTES*i + ASF_YEC_PATTRAM_CLUSTER_BYTES*j;
					if( Port == 0 )  {
						SK_OUT32( IoC, WOL_PATT_RAM_1+idx, PattRamCluster[j] );
					}
					else  {
						SK_OUT32( IoC, WOL_PATT_RAM_2+idx, PattRamCluster[j] );
					}
				}
			}  /*   	for (i = 0; (i < ASF_YEC_PATTRAM_CLUSTER_SIZE) && (RetVal == 1); i++) */

			/*   Set pattern length register */
			AsfYexPatternLengthReg = ASF_YEX_PATTERN_LENGTH_R1_L+(Port*0x80)+( (PatternId/4)*sizeof(SK_U32) );
			SK_IN32(IoC, AsfYexPatternLengthReg, &TmpVal32);		
			TmpVal32 &= ~(0x0000007f << 8*(PatternId%4));			
			TmpVal32 |= (Length-1) << 8*(PatternId%4);     /*  write length-1 to pattern length register */
			SK_OUT32(IoC, AsfYexPatternLengthReg, TmpVal32);
			
		}  /*   if ( PatternId < SK_NUM_WOL_PATTERN_EX )  */
		else  {
			RetVal = 0;  /*   error */
		}
	}  /*    if ( Length1 <= SK_POW_PATTERN_LENGTH ) */
	else {
		RetVal = 0;  /*   error */
	}
	return(RetVal);
}

SK_I8 FwEnablePattern (
	SK_AC   *pAC,
	SK_IOC  IoC,
	SK_U8   port,
	SK_U8   pattno ) {

	if (port > 1) {
		return (1);
	}

	SK_DBG_MSG(	pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT | SK_DBGCAT_CTRL,
				("*** FwEnablePattern Port:%d Patt#:%d OpMode:%d\n", port, pattno, pAC->FwApp.OpMode));

	if( (pAC->FwApp.ChipMode == SK_GEASF_CHIP_EX) ||
		(pAC->FwApp.ChipMode == SK_GEASF_CHIP_SU))  {
		SK_U16   val16;
		
		if( (pAC->FwApp.OpMode != SK_GEASF_MODE_DASH) &&
			(pAC->FwApp.OpMode != SK_GEASF_MODE_DASH_ASF) )
			pattno += 6;
		
		/*  set ASF match enable register (incomming packets will redirect to ASF queue) */
		SK_IN16(IoC, ASF_YEX_PATTERN_MATCHENA1+(port*0x80), &val16);
		val16 |= (0x01 << pattno);
		SK_OUT16(IoC, ASF_YEX_PATTERN_MATCHENA1+(port*0x80), val16);

		/*  enable pattern pattno */
		SK_IN16(IoC, ASF_YEX_PATTERN_ENA1+(port*0x80), &val16);
		val16 |= (0x01 << pattno);
		SK_OUT16(IoC, ASF_YEX_PATTERN_ENA1+(port*0x80), val16);
	}
	else  {
		SK_U8   val8;
		pattno += 4;
		/*  set ASF match enable register (incomming packets will redirect to ASF queue) */
		SK_IN8(IoC, ASF_YEC_PATTERN_MATCHENA1+(port*0x80), &val8);
		val8 |= (0x01 << pattno);
		SK_OUT8(IoC, ASF_YEC_PATTERN_MATCHENA1+(port*0x80), val8);

		/*  enable pattern pattno */
		SK_IN8(IoC, ASF_YEC_PATTERN_ENA1+(port*0x80), &val8);
		val8 |= (0x01 << pattno);
		SK_OUT8(IoC, ASF_YEC_PATTERN_ENA1+(port*0x80), val8);
	}
	return ( 1 );
}

SK_I8 FwDisablePattern (
	SK_AC   *pAC,
	SK_IOC  IoC,
	SK_U8   port,
	SK_U8   pattno ) {

	if (port > 1) {
		return (1);
	}

	SK_DBG_MSG(	pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT | SK_DBGCAT_CTRL,
				("*** FwDisablePattern Port:%d Patt#:%d OpMode:%d\n", port, pattno, pAC->FwApp.OpMode));

	if( (pAC->FwApp.ChipMode == SK_GEASF_CHIP_EX) ||
		(pAC->FwApp.ChipMode == SK_GEASF_CHIP_SU))  {
		SK_U16   val16;

		if( (pAC->FwApp.OpMode != SK_GEASF_MODE_DASH) &&
			(pAC->FwApp.OpMode != SK_GEASF_MODE_DASH_ASF) )
			pattno += 6;

		/*  set ASF match disable register (incomming packets will redirect to ASF queue) */
		SK_IN16(IoC, ASF_YEX_PATTERN_MATCHENA1+(port*0x80), &val16);
		val16 &= ~(0x01 << pattno);
		SK_OUT16(IoC, ASF_YEX_PATTERN_MATCHENA1+(port*0x80), val16);

		/*  disable pattern pattno */
		SK_IN16(IoC, ASF_YEX_PATTERN_ENA1+(port*0x80), &val16);
		val16 &= ~(0x01 << pattno);
		SK_OUT16(IoC, ASF_YEX_PATTERN_ENA1+(port*0x80), val16);
	}
	else  {
		SK_U8   val8;
		pattno += 4;
		/*  set ASF match disable register (incomming packets will redirect to ASF queue) */
		SK_IN8(IoC, ASF_YEC_PATTERN_MATCHENA1+(port*0x80), &val8);
		val8 &= ~(0x01 << pattno);
		SK_OUT8(IoC, ASF_YEC_PATTERN_MATCHENA1+(port*0x80), val8);

		/*  disable pattern pattno */
		SK_IN8(IoC, ASF_YEC_PATTERN_ENA1+(port*0x80), &val8);
		val8 &= ~(0x01 << pattno);
		SK_OUT8(IoC, ASF_YEC_PATTERN_ENA1+(port*0x80), val8);
	}
	return ( 1 );
}


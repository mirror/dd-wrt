/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File under the following licensing terms. 
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer. 

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution. 

    *   Neither the name of Marvell nor the names of its contributors may be 
        used to endorse or promote products derived from this software without 
        specific prior written permission. 
    
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/
/*******************************************************************/
/* Revision History						   */
/* Revision 2.1 was revised for speed-up			   */
/* Revision 3.0 was revised for speed-up			   */
/* Description : The following source code shows an example for    */
/* generating of 3bytes per 256bytes, based on hamming code ecc    */ 
/*******************************************************************/

/* #include <stdio.h> */
/* #include <stdlib.h>  */ /* for malloc, random treat functions */ 
/* #include <sys/time.h>*/ /* gettimeofday will be used as seed  */

#include "ecc3.h"

/* Data */
uchar   ecc_gen[ECC_SIZE];
uchar   write_data[BLOCK_SIZE];
char    changed_string[80];
/* Implementation */

/* little function to subotage data ... flips random bit in the buffer */
/*void flip_random_bit (uchar *data, uint len) {

  uint byte_error_index = rand() % len;
  uchar bit_error_index = rand() % DATA_SIZE;
  printf ("changing d[%d] from <0x%x> ",
	  byte_error_index, data[byte_error_index]);
  data[byte_error_index] ^= (1 << bit_error_index);
  printf ("to <0x%x>\n", data[byte_error_index]);
  sprintf (changed_string, " byte number %d, bit %d", byte_error_index, bit_error_index);
  }*/

void genEcc3(uchar* write_data, uchar* ecc_gen) {

  uint  i, j;
  uchar	paritr[256], tmp = 0, tmp2 = 0;
  uchar data_table0[16] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};
  uchar data_table1[16] = {1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1};
  uchar sum = 0, paritc = 0;
  uchar	parit0c = 0, parit1c = 0, parit2c = 0, parit3c = 0;
  uchar	parit4c = 0, parit5c = 0, parit6c = 0, parit7c = 0;
  uchar	parit1_1, parit1_2;
  uchar parit2_1, parit2_2;
  uchar parit4_1, parit4_2;
  uchar	parit8_1 = 0, parit8_2 = 0;
  uchar parit16_1 = 0, parit16_2 = 0;
  uchar parit32_1 = 0, parit32_2 = 0;
  uchar	parit64_1 = 0, parit64_2 = 0;
  uchar parit128_1 = 0, parit128_2 = 0;
  uchar parit256_1 = 0, parit256_2 = 0;
  uchar	parit512_1 = 0, parit512_2 = 0;
  uchar parit1024_1 = 0, parit1024_2 = 0;

  for( i = 0; i < 256; i++) {
      paritc = paritc ^ (*(write_data+i));

      tmp = (*(write_data+i) & 0xf0) >> 4;
      tmp2 = *(write_data+i) & 0x0f;
      switch(tmp)
	{
	case 0:
	  *(paritr + i) = *(data_table0 + tmp2);
	  break;
	case 1:
	  *(paritr + i) = *(data_table1 + tmp2);
	  break;
	case 2:
	  *(paritr + i) = *(data_table1 + tmp2);
	  break;
	case 3:
	  *(paritr + i) = *(data_table0 + tmp2);
	  break;
	case 4:
	  *(paritr + i) = *(data_table1 + tmp2);
	  break;
	case 5:
	  *(paritr + i) = *(data_table0 + tmp2);
	  break;
	case 6:
	  *(paritr + i) = *(data_table0 + tmp2);
	  break;
	case 7:
	  *(paritr + i) = *(data_table1 + tmp2);
	  break;
	case 8:
	  *(paritr + i) = *(data_table1 + tmp2);
	  break;
	case 9:
	  *(paritr + i) = *(data_table0 + tmp2);
	  break;
	case 10:
	  *(paritr + i) = *(data_table0 + tmp2);
	  break;
	case 11:
	  *(paritr + i) = *(data_table1 + tmp2);
	  break;
	case 12:
	  *(paritr + i) = *(data_table0 + tmp2);
	  break;
	case 13:
	  *(paritr + i) = *(data_table1 + tmp2);
	  break;
	case 14:
	  *(paritr + i) = *(data_table1 + tmp2);
	  break;
	case 15:
	  *(paritr + i) = *(data_table0 + tmp2);
	  break;
	}

    }

  parit0c = ((paritc & 0x01) ? 1 : 0);
  parit1c = ((paritc & 0x02) ? 1 : 0);
  parit2c = ((paritc & 0x04) ? 1 : 0);
  parit3c = ((paritc & 0x08) ? 1 : 0);
  parit4c = ((paritc & 0x10) ? 1 : 0);
  parit5c = ((paritc & 0x20) ? 1 : 0);
  parit6c = ((paritc & 0x40) ? 1 : 0);
  parit7c = ((paritc & 0x80) ? 1 : 0);

  parit1_2 = parit6c ^ parit4c ^ parit2c ^ parit0c;
  parit1_1 = parit7c ^ parit5c ^ parit3c ^ parit1c;
  parit2_2 = parit5c ^ parit4c ^ parit1c ^ parit0c;
  parit2_1 = parit7c ^ parit6c ^ parit3c ^ parit2c;
  parit4_2 = parit3c ^ parit2c ^ parit1c ^ parit0c;
  parit4_1 = parit7c ^ parit6c ^ parit5c ^ parit4c;

  for( i = 0 ; i < 256;i++)	sum=sum ^ (*(paritr+i));

  for ( i = 0; i < 256; i = i+2 )
    parit8_2 = parit8_2 ^ (*(paritr + i));
  for ( i = 0; i < 256; i = i+4 )
    {
      parit16_2 = parit16_2 ^ (*(paritr + i));
      parit16_2 = parit16_2 ^ (*(paritr + i + 1));
    }
  for ( i = 0; i < 256; i = i+8 )
    {
      for ( j = 0; j <= 3; j++ )
	parit32_2 = parit32_2 ^ (*(paritr + i + j));
    }
  for ( i = 0; i < 256; i = i+16 )
    {
      for ( j = 0; j <= 7; j++ )
	parit64_2 = parit64_2 ^ (*(paritr + i + j));
    }
  for ( i = 0; i < 256; i = i+32 )
    {
      for ( j = 0; j <= 15; j++ )
	parit128_2 = parit128_2 ^ (*(paritr + i + j));
    }
  for ( i = 0; i < 256; i = i+64 )
    {
      for ( j = 0; j <= 31; j++ )
	parit256_2 = parit256_2 ^ (*(paritr + i + j));
    }
  for ( i = 0; i < 256; i = i+128 )
    {
      for ( j = 0; j <= 63; j++ )
	parit512_2 = parit512_2 ^ (*(paritr + i + j));
    }
  for ( i = 0; i < 256; i = i+256 )
    {
      for ( j = 0; j <= 127; j++ )
	parit1024_2 = parit1024_2 ^ (*(paritr + i + j));
    }

  if(sum==0){
    parit1024_1=parit1024_2;
    parit512_1=parit512_2;
    parit256_1=parit256_2;
    parit128_1=parit128_2;
    parit64_1=parit64_2;
    parit32_1=parit32_2;
    parit16_1=parit16_2;
    parit8_1=parit8_2;
  }
  else{
    parit1024_1 = (parit1024_2 ? 0 : 1);
    parit512_1  = (parit512_2  ? 0 : 1);
    parit256_1  = (parit256_2  ? 0 : 1);
    parit128_1  = (parit128_2  ? 0 : 1);
    parit64_1   = (parit64_2   ? 0 : 1);
    parit32_1   = (parit32_2   ? 0 : 1);
    parit16_1   = (parit16_2   ? 0 : 1);
    parit8_1    = (parit8_2    ? 0 : 1);
  }

  parit1_2 <<= 2;
  parit1_1 <<= 3;
  parit2_2 <<= 4;
  parit2_1 <<= 5;
  parit4_2 <<= 6;
  parit4_1 <<= 7;
  parit128_1 <<= 1;
  parit256_2 <<= 2;
  parit256_1 <<= 3;
  parit512_2 <<= 4;
  parit512_1 <<= 5;
  parit1024_2 <<= 6;
  parit1024_1 <<= 7;
  parit8_1 <<= 1;
  parit16_2 <<= 2;

  parit16_1 <<= 3;
  parit32_2 <<= 4;
  parit32_1 <<= 5;
  parit64_2 <<= 6;
  parit64_1 <<= 7;

  *(ecc_gen + 0) = ~( parit64_1|parit64_2|parit32_1|parit32_2|parit16_1|parit16_2|parit8_1|parit8_2 );
  *(ecc_gen + 1) = ~( parit1024_1|parit1024_2|parit512_1|parit512_2|parit256_1|parit256_2|parit128_1|parit128_2 );
  *(ecc_gen + 2) = ~( parit4_1|parit4_2|parit2_1|parit2_2|parit1_1|parit1_2|0x00|0x00 );
}

ECC_RET_VAL fixEcc3 (uchar* page_data, uchar* ecc_orig) {

  uint i;
  uchar tmp0_bit[8],tmp1_bit[8],tmp2_bit[8];
  uchar comp0_bit[8],comp1_bit[8],comp2_bit[8];
  uchar tmp[3];
  uchar ecc_bit[24];
  uchar ecc_sum=0;
  uchar ecc_value,find_bit=0;
  uint  find_byte=0;
  uchar ecc_calculated[ECC_SIZE];
  uchar saved_orig_ecc[ECC_SIZE];

  /* saving the original ecc unharmed */
  genEcc3 (page_data, ecc_calculated);
  /* since we can't use memcpy ... */
  for(i = 0 ; i < ECC_SIZE ; i++) {
    saved_orig_ecc[i] = ecc_orig[i];
  }

  if ((ecc_orig[0] == ecc_calculated[0]) &&
      (ecc_orig[1] == ecc_calculated[1]) &&
      (ecc_orig[2] == ecc_calculated[2])) {
    return ECC_PASS;
  }
  for(i = 0; i <= 2; i++) {
    *(ecc_orig+i) =~ (*(ecc_orig+i));
    *(ecc_calculated+i) =~ (*(ecc_calculated+i));
  }
  tmp0_bit[0]= *ecc_orig%2;
  *ecc_orig=*ecc_orig/2;
  tmp0_bit[1]= *ecc_orig%2;
  *ecc_orig=*ecc_orig/2;
  tmp0_bit[2]= *ecc_orig%2;
  *ecc_orig=*ecc_orig/2;
  tmp0_bit[3]= *ecc_orig%2;
  *ecc_orig=*ecc_orig/2;
  tmp0_bit[4]= *ecc_orig%2;
  *ecc_orig=*ecc_orig/2;
  tmp0_bit[5]= *ecc_orig%2;
  *ecc_orig=*ecc_orig/2;
  tmp0_bit[6]= *ecc_orig%2;
  *ecc_orig=*ecc_orig/2;
  tmp0_bit[7]= *ecc_orig%2;

  tmp1_bit[0]= *(ecc_orig+1)%2;
  *(ecc_orig+1)=*(ecc_orig+1)/2;
  tmp1_bit[1]= *(ecc_orig+1)%2;
  *(ecc_orig+1)=*(ecc_orig+1)/2;
  tmp1_bit[2]= *(ecc_orig+1)%2;
  *(ecc_orig+1)=*(ecc_orig+1)/2;
  tmp1_bit[3]= *(ecc_orig+1)%2;
  *(ecc_orig+1)=*(ecc_orig+1)/2;
  tmp1_bit[4]= *(ecc_orig+1)%2;
  *(ecc_orig+1)=*(ecc_orig+1)/2;
  tmp1_bit[5]= *(ecc_orig+1)%2;
  *(ecc_orig+1)=*(ecc_orig+1)/2;
  tmp1_bit[6]= *(ecc_orig+1)%2;
  *(ecc_orig+1)=*(ecc_orig+1)/2;
  tmp1_bit[7]= *(ecc_orig+1)%2;

  tmp2_bit[0]= *(ecc_orig+2)%2;
  *(ecc_orig+2)=*(ecc_orig+2)/2;
  tmp2_bit[1]= *(ecc_orig+2)%2;
  *(ecc_orig+2)=*(ecc_orig+2)/2;
  tmp2_bit[2]= *(ecc_orig+2)%2;
  *(ecc_orig+2)=*(ecc_orig+2)/2;
  tmp2_bit[3]= *(ecc_orig+2)%2;
  *(ecc_orig+2)=*(ecc_orig+2)/2;
  tmp2_bit[4]= *(ecc_orig+2)%2;
  *(ecc_orig+2)=*(ecc_orig+2)/2;
  tmp2_bit[5]= *(ecc_orig+2)%2;
  *(ecc_orig+2)=*(ecc_orig+2)/2;
  tmp2_bit[6]= *(ecc_orig+2)%2;
  *(ecc_orig+2)=*(ecc_orig+2)/2;
  tmp2_bit[7]= *(ecc_orig+2)%2;

  comp0_bit[0]= *ecc_calculated%2;
  *ecc_calculated=*ecc_calculated/2;
  comp0_bit[1]= *ecc_calculated%2;
  *ecc_calculated=*ecc_calculated/2;
  comp0_bit[2]= *ecc_calculated%2;
  *ecc_calculated=*ecc_calculated/2;
  comp0_bit[3]= *ecc_calculated%2;
  *ecc_calculated=*ecc_calculated/2;
  comp0_bit[4]= *ecc_calculated%2;
  *ecc_calculated=*ecc_calculated/2;
  comp0_bit[5]= *ecc_calculated%2;
  *ecc_calculated=*ecc_calculated/2;
  comp0_bit[6]= *ecc_calculated%2;
  *ecc_calculated=*ecc_calculated/2;
  comp0_bit[7]= *ecc_calculated%2;

  comp1_bit[0]= *(ecc_calculated+1)%2;
  *(ecc_calculated+1)=*(ecc_calculated+1)/2;
  comp1_bit[1]= *(ecc_calculated+1)%2;
  *(ecc_calculated+1)=*(ecc_calculated+1)/2;
  comp1_bit[2]= *(ecc_calculated+1)%2;
  *(ecc_calculated+1)=*(ecc_calculated+1)/2;
  comp1_bit[3]= *(ecc_calculated+1)%2;
  *(ecc_calculated+1)=*(ecc_calculated+1)/2;
  comp1_bit[4]= *(ecc_calculated+1)%2;
  *(ecc_calculated+1)=*(ecc_calculated+1)/2;
  comp1_bit[5]= *(ecc_calculated+1)%2;
  *(ecc_calculated+1)=*(ecc_calculated+1)/2;
  comp1_bit[6]= *(ecc_calculated+1)%2;
  *(ecc_calculated+1)=*(ecc_calculated+1)/2;
  comp1_bit[7]= *(ecc_calculated+1)%2;

  comp2_bit[0]= *(ecc_calculated+2)%2;
  *(ecc_calculated+2)=*(ecc_calculated+2)/2;
  comp2_bit[1]= *(ecc_calculated+2)%2;
  *(ecc_calculated+2)=*(ecc_calculated+2)/2;
  comp2_bit[2]= *(ecc_calculated+2)%2;
  *(ecc_calculated+2)=*(ecc_calculated+2)/2;
  comp2_bit[3]= *(ecc_calculated+2)%2;
  *(ecc_calculated+2)=*(ecc_calculated+2)/2;
  comp2_bit[4]= *(ecc_calculated+2)%2;
  *(ecc_calculated+2)=*(ecc_calculated+2)/2;
  comp2_bit[5]= *(ecc_calculated+2)%2;
  *(ecc_calculated+2)=*(ecc_calculated+2)/2;
  comp2_bit[6]= *(ecc_calculated+2)%2;
  *(ecc_calculated+2)=*(ecc_calculated+2)/2;
  comp2_bit[7]= *(ecc_calculated+2)%2;

  for(i=0;i<6;i++)	ecc_bit[i]=tmp2_bit[i+2]^comp2_bit[i+2];
  for(i=0;i<8;i++)	ecc_bit[i+6]=tmp0_bit[i]^comp0_bit[i];
  for(i=0;i<8;i++)	ecc_bit[i+14] =tmp1_bit[i]^comp1_bit[i];
  ecc_bit[22]=tmp2_bit[0]^comp2_bit[0];
  ecc_bit[23]=tmp2_bit[1]^comp2_bit[1];

  for(i=0;i<24;i++)	ecc_sum+=ecc_bit[i];
  if(ecc_sum==11){
    find_byte=(ecc_bit[23]<<8)+(ecc_bit[21]<<7)+(ecc_bit[19]<<6)+(ecc_bit[17]<<5)+(ecc_bit[15]<<4)+(ecc_bit[13]<<3)+(ecc_bit[11]<<2)+(ecc_bit[9]<<1)+ecc_bit[7];
    find_bit= (ecc_bit[5]<<2) +(ecc_bit[3]<<1)+ecc_bit[1];
    
    ecc_value = (page_data[find_byte]>>find_bit)%2;
    /* ilanp: i can't agree with that */
    /*     if(ecc_value==0) ecc_value=1; */
    /*     else		    ecc_value=0; */
    
    /*     *tmp=page_data[find_byte]; */
    /*     tmp0_bit[0]= *tmp%2; */
    /*     *tmp=*tmp/2; */
    /*     tmp0_bit[1]= *tmp%2; */
    /*     *tmp=*tmp/2; */
    /*     tmp0_bit[2]= *tmp%2; */
    /*     *tmp=*tmp/2; */
    /*     tmp0_bit[3]= *tmp%2; */
    /*     *tmp=*tmp/2; */
    /*     tmp0_bit[4]= *tmp%2; */
    /*     *tmp=*tmp/2; */
    /*     tmp0_bit[5]= *tmp%2; */
    /*     *tmp=*tmp/2; */
    /*     tmp0_bit[6]= *tmp%2; */
    /*     *tmp=*tmp/2; */
    /*     tmp0_bit[7]= *tmp%2; */
    
    /*     tmp0_bit[find_bit]=ecc_value; */
    
    /*     *tmp=(tmp0_bit[7]<<7)+(tmp0_bit[6]<<6)+(tmp0_bit[5]<<5)+(tmp0_bit[4]<<4)+ */
    /*       (tmp0_bit[3]<<3)+(tmp0_bit[2]<<2)+(tmp0_bit[1]<<1)+tmp0_bit[0]; */
    
    /* ilanp: fliping the bit : */
    *tmp=page_data[find_byte];
    *tmp ^= (1 << find_bit);
    
    /*    printf("\nOriginal page_data[%d] = %x",find_byte,*tmp);
    printf("\nError    page_data[%d] = %x (%d -> %d)\n",
    find_byte,page_data[find_byte],ecc_value,(ecc_value+1)%2);*/
    page_data[find_byte]=*tmp; /* ilanp: correcting the error in data which we've found */
    /*printf ("ilanp: page_data[%d] = 0x%x;\n", find_byte, page_data[find_byte]);*/

    /* double check that we've returned to the original ecc - and not done
       some silly mistake - thinking there was one error and fixing it while
       there were two, or more errors.
    */
    genEcc3 (page_data, ecc_calculated);
    if ((saved_orig_ecc[0] == ecc_calculated[0]) &&
	(saved_orig_ecc[1] == ecc_calculated[1]) &&
	(saved_orig_ecc[2] == ecc_calculated[2])) {
      /*printf ("fixed - alright\n");*/
      return ECC_FIX;
    } else {
      /*printf ("fixed - but incorrectly !!!!!!\n");*/
      /* returning the byte to be what is was - in any case */
      page_data[find_byte] ^= (1 << find_bit);
      return ECC_FAIL_DATA;
    }
  }
  else if(ecc_sum==1){
    /*printf("Ecc Write Area itself 1 bit error occurs..\n");*/
    return ECC_FAIL;
  }
  else{
    return ECC_FAIL_DATA;
  }
}


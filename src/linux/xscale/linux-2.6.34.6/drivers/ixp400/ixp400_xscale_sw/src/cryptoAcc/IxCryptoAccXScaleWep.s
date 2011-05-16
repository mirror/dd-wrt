/*
 * IxCryptoAccXscaleWep.s
 *
 * 3-October-2003
 *
 * Assembly-coded ARC4 and combined AR4/CRC routines.
 *
 * IXP400 SW Release Crypto version 2.4
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

#include "IxOsalEndianess.h"

#ifdef __vxworks

#define _ASMLANGUAGE
#include <arch/arm/arm.h>

#endif

#ifdef __linux

//#include <asm/elf.h>
//#include <asm/procinfo.h>
//#include <asm/arch/hardware.h>
#define __ASSEMBLY__

#include <linux/version.h>
#include <linux/linkage.h>
#if KERNEL_VERSION(2,6,0) <= LINUX_VERSION_CODE
#include <asm/assembler.h>
#else
#include <asm/proc-armv/assembler.h>
#endif

#define _ARM_FUNCTION(a)    ENTRY(a)

#endif


    .balign 4
    .global Enable_BTB
    .global ARC4_InitSbox
    .global ARC4_Crypt
    .global ARC4_EncryptWithCRC
    .global ARC4_DecryptWithCRC

/*===========================================================================
 * Endianness Macros
 * 
 * Note:  Becasue it is impossible to use the '#' character in a macro
 * definition, the second (h) argument is used to supply a '#' character
 * in the expanded expression, e.g. PutB0(r0,#).
 *===========================================================================
 */
#ifdef __LITTLE_ENDIAN

#ifdef VX_VERSION55
#define GetB0(r,h)      r
#define GetB1(r,h)      r, LSR h##8
#define GetB2(r,h)      r, LSR h##16
#define GetB3(r,h)      r, LSR h##24

#define PutB0(r,h)      r
#define PutB1(r,h)      r, LSL h##8
#define PutB2(r,h)      r, LSL h##16
#define PutB3(r,h)      r, LSL h##24

#else
#define GetB0(r,h)      r
#define GetB1(r,h)      r, LSR h/**/8
#define GetB2(r,h)      r, LSR h/**/16
#define GetB3(r,h)      r, LSR h/**/24

#define PutB0(r,h)      r
#define PutB1(r,h)      r, LSL h/**/8
#define PutB2(r,h)      r, LSL h/**/16
#define PutB3(r,h)      r, LSL h/**/24
#endif /* VX_VERSION55 */

#else

#ifdef VX_VERSION55
#define GetB0(r,h)      r, LSR h##24
#define GetB1(r,h)      r, LSR h##16
#define GetB2(r,h)      r, LSR h##8
#define GetB3(r,h)      r

#define PutB0(r,h)      r, LSL h##24
#define PutB1(r,h)      r, LSL h##16
#define PutB2(r,h)      r, LSL h##8
#define PutB3(r,h)      r

#else
#define GetB0(r,h)      r, LSR h/**/24
#define GetB1(r,h)      r, LSR h/**/16
#define GetB2(r,h)      r, LSR h/**/8
#define GetB3(r,h)      r

#define PutB0(r,h)      r, LSL h/**/24
#define PutB1(r,h)      r, LSL h/**/16
#define PutB2(r,h)      r, LSL h/**/8
#define PutB3(r,h)      r
#endif /* VX_VERSION55 */

#endif

/*===========================================================================
 * unsigned long Enable_BTB ()
 *
 * Enables the Branch Target Buffer, so that looping overhead is minimized.
 * Returns the old value of CP15, register 1.
 *===========================================================================
 */
_ARM_FUNCTION(Enable_BTB)
    mrc     p15, 0x0, r0, c1, c1
    orr     r1, r0, #0x0800
    bic     r1, r1, #0xFF000000
    bic     r1, r1, #0x00FF0000
    bic     r1, r1, #0x0000C000
    mcr     p15, 0x0, r1, c1, c1
    mov     pc, lr

/*===========================================================================
 * void ARC4_InitSbox (
 *    UINT8*   pSbox,
 *    UINT8*   pKeyBuffer)
 *
 * Performs the first, common step of ARC4 encryption or decryption, i.e. the
 * S-box initialization, using the 128-bit key in pKeyBuffer.  If a 64-bit
 * key is to be used it must be duplicated (i.e. extended to 128 bits) before
 * calling ARC4_InitSbox.  A temporary buffer for the S-box must be supplied
 * in pSbox.  The following size and alignment restrictions apply to the
 * arguments:
 *
 *   Argument        Size                Alignment
 *                   (bytes)             (bytes)
 *   --------------------------------------------------------
 *   pSbox           256                 512
 *   pKeyBuffer      16                  32
 *===========================================================================
 */
_ARM_FUNCTION(ARC4_InitSbox)

    /*-----------------------------------------------------------------------
     * Upon entry:
     *   r0  base of S-box (must be 512-byte aligned)
     *   r1  base of Key buffer (must be 16 bytes long and 32-byte aligned)
     *-----------------------------------------------------------------------
     */
    stmfd   sp!, {r4, r5, r6}

    /*-----------------------------------------------------------------------
     * ARC4 initialization (pre-initialize S-box)
     *
     * Register allocation:
     *   r2  base of S-box
     *   r4  initializer word
     *   r5  initializer increment
     *
     * for i = 0 to 255
     *    S[i] = i
     *
     * Average cycle count = 6 + 9*(256/16) + 4 + 4 = 158
     *-----------------------------------------------------------------------
     */
#ifdef __LITTLE_ENDIAN

    mov     r2, r0
    mov     r4, #0x03000000         /* r4 <= 0x03000000 */
    orr     r4, r4, #0x00000100     /* r4 <= 0x03000100 */
    add     r5, r4, r4, ROR #16     /* r5 <= 0x04000400 */
    orr     r4, r4, #0x00020000     /* r4 <= 0x03020100 */
    orr     r5, r5, r5, ROR #8      /* r5 <= 0x04040404 */

#else

    mov     r2, r0
    mov     r4, #0x00000003         /* r4 <= 0x00000003 */
    orr     r4, r4, #0x00010000     /* r4 <= 0x00010003 */
    add     r5, r4, r4, ROR #16     /* r5 <= 0x00040004 */
    orr     r4, r4, #0x00000200     /* r4 <= 0x00010203 */
    orr     r5, r5, r5, ROR #8      /* r5 <= 0x04040404 */

#endif

.ARC4_Sbox_Fill_Loop:
    str     r4, [r2], #4
    add     r4, r4, r5
    str     r4, [r2], #4
    add     r4, r4, r5
    str     r4, [r2], #4
    add     r4, r4, r5
    str     r4, [r2], #4
    adds    r4, r4, r5              /* Set C flag if done with 256 bytes */
    bcc     .ARC4_Sbox_Fill_Loop

   /*-----------------------------------------------------------------------
    * ARC4 initialization (permute with key)
    *
    * Register allocation:
    *   r0  base of S-box
    *   r1  base of Key buffer (auto-incrementing)
    *   r2  address of S[i] (i.e. &S[i])
    *   r3  index j
    *   r4  S[i]
    *   r5  K[i] through K[i+3]
    *   r6  S[j]
    *
    * Algorithm (per iteration):
    *   b = j + S[i]
    *   c = b + K[i]
    *   j = c mod 256
    *   swap S[i] and S[j]
    *   i = i + 1
    *
    * Note:  There is no need to do key extension, since the input key must
    * be 16 bytes long.  The key pointer is simply "wrapped" every 16 bytes.
    *
    * Average cycle count:  3 + 32*(256/4) + 4 + 4 = 2059
    *-----------------------------------------------------------------------
    */
    mov     r2, r0                  /* i <= 0 */
    mov     r3, #0                  /* j <= 0 */

.ARC4_Sbox_Init_Loop:
                                    /*   r2     r3     r4     r5     r6   */
                                    /* ---------------------------------- */
    ldrb    r4, [r2], #4            /*   i0     0     S[i0]   .      .    */
    ldr     r5, [r1], #4            /*   .      .      .   K[i0-i3]  .    */
    bic     r1, r1, #0x10           /*   .      .      .      .      .    */
    add     r3, r3, r4              /*   .      b0     <>     .      .    */
    add     r3, r3, GetB0(r5,#)     /*   .      c0     .      <>     .    */
    and     r3, r3, #0xFF           /*   .      j0     .      .      .    */
    ldrb    r6, [r0, r3]            /*   .      .      .      .     S[j0] */
    strb    r4, [r0, r3]            /*   .      .      @@     .      .    */
    ldrb    r4, [r2, #-3]           /*   i1     .     S[i1]   .      .    */
    strb    r6, [r2, #-4]           /*   .      .      .      .      @@   */
    add     r3, r3, GetB1(r5,#)     /*   .      c1'    .      <>     .    */
    add     r3, r3, r4              /*   .      b1'    <>     .      .    */
    and     r3, r3, #0xFF           /*   .      j1     .      .      .    */
    ldrb    r6, [r0, r3]            /*   .      .      .      .     S[j1] */
    strb    r4, [r0, r3]            /*   .      .      @@     .      .    */
    ldrb    r4, [r2, #-2]           /*   i2     .     S[i2]   .      .    */
    strb    r6, [r2, #-3]           /*   .      .      .      .      @@   */
    add     r3, r3, GetB2(r5,#)     /*   .      c2'    .      <>     .    */
    add     r3, r3, r4              /*   .      b2'    <>     .      .    */
    and     r3, r3, #0xFF           /*   .      j2     .      .      .    */
    ldrb    r6, [r0, r3]            /*   .      .      .      .     S[j2] */
    strb    r4, [r0, r3]            /*   .      .      @@     .      .    */
    ldrb    r4, [r2, #-1]           /*   i3     .     S[i3]   .      .    */
    strb    r6, [r2, #-2]           /*   .      .      .      .      @@   */
    add     r3, r3, GetB3(r5,#)     /*   .      c3'    .      <>     .    */
    add     r3, r3, r4              /*   .      b3'    <>     .      .    */
    and     r3, r3, #0xFF           /*   .      j3     .      .      .    */
    ldrb    r6, [r0, r3]            /*   .      .      .      .     S[j3] */
    strb    r4, [r0, r3]            /*   .      .      @@     .      .    */
    tst     r2, #0x100              /*   .      .      .      .      .    */
    strb    r6, [r2, #-1]           /*   .      .      .      .      @@   */
    beq     .ARC4_Sbox_Init_Loop

    ldmfd   sp!, {r4, r5, r6}
    mov     pc, lr


/*===========================================================================
 * void ARC4_Crypt (
 *    UINT8*       pSbox,
 *    MBUF*        pMbuf,
 *    ARC4_STATE*  pState)
 *
 * Performs ARC4 encryption/decryption on the buffer pData (of dataLength
 * bytes) using the (already initialized) S-box in pSbox.  The following
 * size and alignment restrictions apply to the arguments:
 *
 *   Argument        Size                Alignment
 *                   (bytes)             (bytes)
 *   --------------------------------------------------------
 *   pSbox           256                 512
 *   pMbuf->mdata    >2 (*)              1 (i.e. arbitrary)
 *
 * (*) The mbuf data cluster may be 1 byte in length if mdata is of the form
 * 4n or 4n+3.  The mbuf data cluster may be 2 bytes in legnth if mdata is 
 * of the form 4n, 4n+2, or 4n+3.
 *
 * The ARC4_STATE structure is defined as follows :
 *
 * typedef struct {
 *    UINT8*      pSi        // &S[i] (where "i" is RC$ index i)
 *    UINT32      j          // ARC4 index j
 * } ARC4_STATE;
 *
 * Prior to the first call to ARC4_Crypt, an ARC4_STATE structure must be 
 * initialized with pSi set to pSbox+1 and j set to 0.
 *===========================================================================
 */
_ARM_FUNCTION(ARC4_Crypt)

   /*-----------------------------------------------------------------------
    * Upon entry:
    *   r0  base of S-box (must be 512-byte aligned)
    *   r1  base of plaintext/ciphertext buffer (must be word aligned, but
    *       preferably 32-byte aligned for best cache performance)
    *   r2  length of plaintext/ciphertext buffer
    *-----------------------------------------------------------------------
    */

    stmfd   sp!, {r2, r4, r5, r6, r7, r8, r9}

   /*-----------------------------------------------------------------------
    * RC 4 iteration
    *
    * Register allocation:
    *   r0  base of S-box
    *   r1  plaintext/ciphertext pointer
    *   r2  buffer length
    *   r3  address of S[i] (i.e. &S[i])
    *   r4  index j
    *   r5  S[i]
    *   r6  S[i+1] (must be read after S[i] is written)
    *   r7  S[j]
    *   r8  t = S[i]+S[j] mod 256, or S[t]
    *   r9  plaintext/ciphertext data word
    *
    * Algorithm (per iteration):
    *   m = i + 1
    *   i = m mod 256
    *   n = j + S[i]
    *   j = n mod 256
    *   swap S[i] and S[j]
    *   s = S[i] + S[j]
    *   t = s mod 256
    *   K = S[t]
    *
    * Average cycle count = 2 + 46*(buflen/4) + 4 + 4 = 10 + 11.5 * buflen
    *-----------------------------------------------------------------------
    */

    ldr     r4, [r2, #4]            /* r4 <= j                         */
    ldr     r3, [r2, #0]            /* r3 <= &S[i]                     */
    ldr     r2, [r1, #12]           /* r2 <= mbuf data length (mlen)   */
    ldr     r1, [r1, #8]            /* r1 <= mbuf data pointer (mdata) */

   /*-----------------------------------------------------------------------
    * Handle any odd leading data bytes, if any, in order to get the data
    * pointer into a word-aligned position.
    *-----------------------------------------------------------------------
    */

.ARC4_Crypt_Lead:

    ands    r6, r1, #0x3            /* get (data pointer) mod 4 */
    beq     .ARC4_Crypt_Main

   /* Note: may have to add some logic to handle the case of a very short
    * buffer, i.e. less than 3 bytes, so that it cannot be guaranteed that 
    * there is a word-aligned boundary anywhere within (or at either end 
    * of) the buffer.
    */

    ldrb    r5, [r3, #0]            /* load S[i]                           */
    pld     [r1, #4]                /* pre-load data starting at next word */
    rsb     r6, r6, #4              /* compute bytes left to fill out word */

.ARC4_Crypt_Lead_Loop:

    sub     r2, r2, #1              /* Update mlen */
    add     r4, r4, r5
    and     r4, r4, #0xFF           /* j = (j + S[i]) mod 256              */
    ldrb    r7, [r0, r4]            /* load S[j]                           */
    strb    r5, [r0, r4]            /* store S[i] to &S[j]                 */
    ldrb    r9, [r1, #0]            /* load data[n]                        */
    add     r8, r5, r7
    and     r8, r8, #0xFF           /* t = (S[i] + S[j]) mod 256           */
    strb    r7, [r3], #1            /* store s[j] to &S[i], i++            */
    ldrb    r8, [r0, r8]            /* load S[t]                           */
    bic     r3, r3, #0x100          /* handle rollover on &S[i]            */
    subs    r6, r6, #1              /* decrement byte count (to fill word) */
    eor     r9, r9, r8              /* data[n] ^= S[t]                     */
    strb    r9, [r1], #1            /* store data[n], n++                  */
    ldrb    r5, [r3, #0]            /* load next S[i]                      */
    bne     .ARC4_Crypt_Lead_Loop

.ARC4_Crypt_Main:

   /*-----------------------------------------------------------------------
    * Pre-decrement mlen by 4 so that the loop logic works correctly.
    *-----------------------------------------------------------------------
    */

    subs    r2, r2, #4
    bcc     .ARC4_Crypt_Trail

   /*-----------------------------------------------------------------------
    * Mask &S[i] to determine which loop to drop into.  Each loop is 48
    * instructions, and each instruction is 4 bytes.  Note that 2 dummy
    * instruction must be added since PC actually points 2 instructions
    * into the future (due to pipelining).
    *-----------------------------------------------------------------------
    */

    and     r6, r3, #0x03
    add     r6, r6, r6, LSL #1      /* r6 <= r3i * 3      */
    add     pc, pc, r6, LSL #6      /* pc += r3i * 48 * 4 */

    nop                             /* dummy instruction  */

   /*-----------------------------------------------------------------------
    * This loop handles the case of (pre-incremented) i = 0 mod 4 upon
    * entry, i.e. the "rollover" logic must be applied after accessing
    * the 4th (and final) S[i] each time through the loop.
    *-----------------------------------------------------------------------
    */

.ARC4_Crypt_Main_LoopI0:
                                    /*   r3     r4     r5     r6     r7     r8 */
                                    /* ----------------------------------------*/
    ldrb    r5, [r3, #0]            /*   i1     j0    S[i1]   .      .      .  */
    ldr     r9, [r1, #0]            /*   .      .      .      .      .      .  */
    pld     [r1, #32]               /*   .      .      .      .      .      .  */
    add     r4, r4, r5              /*   .      n1     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j1     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j1]   .  */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    ldrb    r6, [r3, #1]            /*   i2     .      .     S[i2]   .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s1 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t1 */
    add     r4, r4, r6              /*   .      n2     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j2     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j2]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K1 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    eor     r9, r9, PutB0(r8,#)     /*   .      .      .      .      .      <> */
    ldrb    r5, [r3, #0]            /*   i3     .     S[i3]   .      .      .  */
    add     r8, r6, r7              /*   .      .      .      <>     <>     s2 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t2 */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K2 */
    add     r4, r4, r5              /*   .      n3     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j3     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j3]   .  */
    eor     r9, r9, PutB1(r8,#)     /*   .      .      .      .      .      <> */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    ldrb    r6, [r3, #0]            /*   i4     .      .     S[i4]   .      .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s3 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t3 */
    add     r4, r4, r6              /*   .      n4     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j4     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j4]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K3 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    add     r5, r6, r7              /*   .      .      s4     <>     <>     .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    bic     r3, r3, #0x100          /*   .      .      .      .      .      .  */
    and     r5, r5, #0xFF           /*   .      .      t4     .      .      .  */
    ldrb    r5, [r0, r5]            /*   .      .      K4     .      .      .  */
    eor     r9, r9, PutB2(r8,#)     /*   .      .      .      .      .      <> */
    subs    r2, r2, #4              /* Decrement buffer length.      .      .  */
    eor     r9, r9, PutB3(r5,#)     /*   .      .      <>     .      .      .  */
    str     r9, [r1], #4
    bcs     .ARC4_Crypt_Main_LoopI0 /* Loop if not finished                    */

    b       .ARC4_Crypt_Trail
    nop                             /* dummy instruction                       */

   /*-----------------------------------------------------------------------
    * This loop handles the case of (pre-incremented) i = 1 mod 4 upon
    * entry, i.e. the "rollover" logic must be applied after accessing
    * the 3rd S[i] each time through the loop.
    *-----------------------------------------------------------------------
    */

.ARC4_Crypt_Main_LoopI1:
                                    /*   r3     r4     r5     r6     r7     r8 */
                                    /* ----------------------------------------*/
    ldrb    r5, [r3, #0]            /*   i1     j0    S[i1]   .      .      .  */
    ldr     r9, [r1, #0]            /*   .      .      .      .      .      .  */
    pld     [r1, #32]               /*   .      .      .      .      .      .  */
    add     r4, r4, r5              /*   .      n1     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j1     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j1]   .  */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    ldrb    r6, [r3, #1]            /*   i2     .      .     S[i2]   .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s1 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t1 */
    add     r4, r4, r6              /*   .      n2     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j2     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j2]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K1 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    eor     r9, r9, PutB0(r8,#)     /*   .      .      .      .      .      <> */
    ldrb    r5, [r3, #0]            /*   i3     .     S[i3]   .      .      .  */
    add     r8, r6, r7              /*   .      .      .      <>     <>     s2 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t2 */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K2 */
    add     r4, r4, r5              /*   .      n3     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j3     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j3]   .  */
    eor     r9, r9, PutB1(r8,#)     /*   .      .      .      .      .      <> */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    bic     r3, r3, #0x100          /*   .      .      .      .      .      .  */
    ldrb    r6, [r3, #0]            /*   i4     .      .     S[i4]   .      .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s3 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t3 */
    add     r4, r4, r6              /*   .      n4     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j4     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j4]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K3 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    add     r5, r6, r7              /*   .      .      s4     <>     <>     .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    and     r5, r5, #0xFF           /*   .      .      t4     .      .      .  */
    ldrb    r5, [r0, r5]            /*   .      .      K4     .      .      .  */
    eor     r9, r9, PutB2(r8,#)     /*   .      .      .      .      .      <> */
    subs    r2, r2, #4              /* Decrement buffer length.      .      .  */
    eor     r9, r9, PutB3(r5,#)     /*   .      .      <>     .      .      .  */
    str     r9, [r1], #4
    bcs     .ARC4_Crypt_Main_LoopI1 /* Loop if not finished                    */

    b       .ARC4_Crypt_Trail
    nop                             /* dummy instruction                       */

   /*-----------------------------------------------------------------------
    * This loop handles the case of (pre-incremented) i = 2 mod 4 upon
    * entry, i.e. the "rollover" logic must be applied after accessing
    * the 2nd S[i] each time through the loop.
    *-----------------------------------------------------------------------
    */

.ARC4_Crypt_Main_LoopI2:
                                    /*   r3     r4     r5     r6     r7     r8 */
                                    /* ----------------------------------------*/
    ldrb    r5, [r3, #0]            /*   i1     j0    S[i1]   .      .      .  */
    ldr     r9, [r1, #0]            /*   .      .      .      .      .      .  */
    pld     [r1, #32]               /*   .      .      .      .      .      .  */
    add     r4, r4, r5              /*   .      n1     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j1     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j1]   .  */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    ldrb    r6, [r3, #1]            /*   i2     .      .     S[i2]   .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s1 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t1 */
    add     r4, r4, r6              /*   .      n2     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j2     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j2]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K1 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    eor     r9, r9, PutB0(r8,#)     /*   .      .      .      .      .      <> */
    bic     r3, r3, #0x100          /*   .      .      .      .      .      .  */
    ldrb    r5, [r3, #0]            /*   i3     .     S[i3]   .      .      .  */
    add     r8, r6, r7              /*   .      .      .      <>     <>     s2 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t2 */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K2 */
    add     r4, r4, r5              /*   .      n3     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j3     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j3]   .  */
    eor     r9, r9, PutB1(r8,#)     /*   .      .      .      .      .      <> */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    ldrb    r6, [r3, #0]            /*   i4     .      .     S[i4]   .      .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s3 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t3 */
    add     r4, r4, r6              /*   .      n4     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j4     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j4]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K3 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    add     r5, r6, r7              /*   .      .      s4     <>     <>     .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    and     r5, r5, #0xFF           /*   .      .      t4     .      .      .  */
    ldrb    r5, [r0, r5]            /*   .      .      K4     .      .      .  */
    eor     r9, r9, PutB2(r8,#)     /*   .      .      .      .      .      <> */
    subs    r2, r2, #4              /* Decrement buffer length.      .      .  */
    eor     r9, r9, PutB3(r5,#)     /*   .      .      <>     .      .      .  */
    str     r9, [r1], #4
    bcs     .ARC4_Crypt_Main_LoopI2 /* Loop if not finished                    */

    b       .ARC4_Crypt_Trail
    nop                             /* dummy instruction                       */

   /*-----------------------------------------------------------------------
    * This loop handles the case of (pre-incremented) i = 3 mod 4 upon
    * entry, i.e. the "rollover" logic must be applied after accessing
    * the 1st S[i] each time through the loop.
    *-----------------------------------------------------------------------
    */

                                    /*   r3     r4     r5     r6     r7     r8 */
                                    /* ----------------------------------------*/
    ldrb    r5, [r3, #0]            /*   i1     j0    S[i1]   .      .      .  */
.ARC4_Crypt_Main_LoopI3:
    ldr     r9, [r1, #0]            /*   .      .      .      .      .      .  */
    pld     [r1, #32]               /*   .      .      .      .      .      .  */
    add     r4, r4, r5              /*   .      n1     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j1     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j1]   .  */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    subs    r2, r2, #4              /* Decrement buffer length.      .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    bic     r3, r3, #0x100          /*   .      .      .      .      .      .  */
    ldrb    r6, [r3, #0]            /*   i2     .      .     S[i2]   .      .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s1 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t1 */
    add     r4, r4, r6              /*   .      n2     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j2     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j2]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K1 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    eor     r9, r9, PutB0(r8,#)     /*   .      .      .      .      .      <> */
    ldrb    r5, [r3, #0]            /*   i3     .     S[i3]   .      .      .  */
    add     r8, r6, r7              /*   .      .      .      <>     <>     s2 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t2 */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K2 */
    add     r4, r4, r5              /*   .      n3     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j3     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j3]   .  */
    eor     r9, r9, PutB1(r8,#)     /*   .      .      .      .      .      <> */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    ldrb    r6, [r3, #0]            /*   i4     .      .     S[i4]   .      .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s3 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t3 */
    add     r4, r4, r6              /*   .      n4     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j4     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j4]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K3 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    add     r6, r6, r7              /*   .      .      .      s4     <>     .  */
    and     r6, r6, #0xFF           /*   .      .      .      t4     .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    ldrb    r6, [r0, r6]            /*   .      .      .      K4     .      .  */
    eor     r9, r9, PutB2(r8,#)     /*   .      .      .      .      .      <> */
    ldrb    r5, [r3, #0]            /*   i1     j0    S[i1]   .      .      .  */
    eor     r9, r9, PutB3(r6,#)     /*   .      .      <>     .      .      .  */
    str     r9, [r1], #4
    bcs     .ARC4_Crypt_Main_LoopI3 /* Loop if not finished                    */

   /*-----------------------------------------------------------------------
    * Handle any odd traling data bytes, if any.
    *-----------------------------------------------------------------------
    */

.ARC4_Crypt_Trail:

    ands    r2, r2, #0x03
    beq     .ARC4_Crypt_Done
    ldrb    r5, [r3, #0]            /* load S[i] */
    /* delay slot X 2 */

.ARC4_Crypt_Trail_Loop:

    add     r4, r4, r5
    and     r4, r4, #0xFF           /* j = (j + S[i]) mod 256    */
    ldrb    r7, [r0, r4]            /* load S[j]                 */
    strb    r5, [r0, r4]            /* store S[i] to &S[j]       */
    ldrb    r9, [r1, #0]            /* load data[n]              */
    add     r8, r5, r7
    and     r8, r8, #0xFF           /* t = (S[i] + S[j]) mod 256 */
    strb    r7, [r3], #1            /* store s[j] to &S[i], i++  */
    ldrb    r8, [r0, r8]            /* load S[t]                 */
    subs    r2, r2, #1              /* decrement byte count      */
    bic     r3, r3, #0x100          /* handle rollover on &S[i]  */
    eor     r9, r9, r8              /* data[n] ^= S[t]           */
    ldrb    r5, [r3, #0]            /* load next S[i]            */
    strb    r9, [r1], #1            /* store data[n], n++        */
    bne     .ARC4_Crypt_Trail_Loop

   /*-----------------------------------------------------------------------
    * Pop the registers and write out the update &S[i] and j.
    *-----------------------------------------------------------------------
    */

.ARC4_Crypt_Done:

    mov     r1, r4                  /* save j      */
    ldmfd   sp!, {r2, r4, r5, r6, r7, r8, r9}
    str     r1, [r2, #4]            /* store j     */
    str     r3, [r2, #0]            /* store &S[i] */
    mov     pc, lr


/*===========================================================================
 * void ARC4_EncryptWithCRC (
 *    UINT8*       pSbox,
 *    MBUF*        pMbuf,
 *    ARC4_STATE*  pState)
 *
 * Performs CRC-32 computation, followed by ARC4 encryption, on the buffer
 * pData (of dataLength bytes) using the (already initialized) S-box in
 * pSbox.  Regardless of the endianness of the data, the CRC is always big
 * endian (i.e. network byte order).  Note that this function DOES NOT
 * append and encrypt the CRC itself.  The caller must append the (negated)
 * CRC itself and then call ARC4_Crypt to encrypt it.  The following size
 * and alignment restrictions apply to the arguments:
 *
 *   Argument        Size                Alignment
 *                   (bytes)             (bytes)
 *   --------------------------------------------------------
 *   pSbox           256                 512
 *   pMbuf->mdata    >2 (*)              1 (i.e. arbitrary)
 *
 * (*) The mbuf data cluster may be 1 byte in length if mdata is of the form
 * 4n or 4n+3.  The mbuf data cluster may be 2 bytes in legnth if mdata is 
 * of the form 4n, 4n+2, or 4n+3.
 *
 * The ARC4_STATE structure is defined as follows :
 *
 * typedef struct {
 *    UINT8*      pSi;       // &S[i] (where "i" is RC$ index i)
 *    UINT32      j;         // ARC4 index j
 *    UINT8*      crcTable;  // CRC lookup table
 *    UNIT32      CRC;       // 32-bit CRC value
 * } ARC4_STATE;
 *
 * Prior to the first call to ARC4_EncryptWithCRC, an ARC4_STATE structure
 * must be initialized with pSi set to pSbox+1, j set to 0, and CRC set to
 * 0xFFFFFFFF.
 *===========================================================================
 */

_ARM_FUNCTION(ARC4_EncryptWithCRC)

   /*-----------------------------------------------------------------------
    * Upon entry:
    *   r0  base of S-box (must be 512-byte aligned)
    *   r1  base of plaintext/ciphertext buffer (must be word aligned, but
    *       preferably 32-byte aligned for best cache performance)
    *   r2  length of plaintext/ciphertext buffer
    *-----------------------------------------------------------------------
    */

    stmfd   sp!, {r2, r4, r5, r6, r7, r8, r9, r10, r11}

   /*-----------------------------------------------------------------------
    * RC 4 iteration
    *
    * Register allocation:
    *   r0  base of S-box
    *   r1  plaintext/ciphertext pointer
    *   r2  buffer length
    *   r3  address of S[i] (i.e. &S[i])
    *   r4  index j
    *   r5  S[i]
    *   r6  S[i+1] (must be read after S[i] is written)
    *   r7  S[j]
    *   r8  t = S[i]+S[j] mod 256, or S[t]
    *   r9  plaintext/ciphertext data word
    *
    * Algorithm (per iteration):
    *   m = i + 1
    *   i = m mod 256
    *   n = j + S[i]
    *   j = n mod 256
    *   swap S[i] and S[j]
    *   s = S[i] + S[j]
    *   t = s mod 256
    *   K = S[t]
    *
    * Average cycle count = 2 + 46*(buflen/4) + 4 + 4 = 10 + 11.5 * buflen
    *-----------------------------------------------------------------------
    */

    ldr     r12, [r2, #12]          /* r12 <= CRC                      */
    ldr     r10, [r2, #8]           /* r10 <= &crcTable[0]             */
    ldr     r4, [r2, #4]            /* r4 <= j                         */
    ldr     r3, [r2, #0]            /* r3 <= &S[i]                     */
    ldr     r2, [r1, #12]           /* r2 <= mbuf data length (mlen)   */
    ldr     r1, [r1, #8]            /* r1 <= mbuf data pointer (mdata) */

   /*-----------------------------------------------------------------------
    * Handle any odd leading data bytes, if any, in order to get the data
    * pointer into a word-aligned position.
    *-----------------------------------------------------------------------
    */

.ARC4_EnCRC_Lead:

    ands    r6, r1, #0x3            /* get (data pointer) mod 4 */
    beq     .ARC4_EnCRC_Main

   /* Note: may have to add some logic to handle the case of a very short
    * buffer, i.e. less than 3 bytes, so that it cannot be guaranteed that 
    * there is a word-aligned boundary anywhere within (or at either end 
    * of) the buffer.
    */

    pld     [r1, #4]                /* pre-load data starting at next word */
    ldrb    r5, [r3, #0]            /* load S[i]                           */
    rsb     r6, r6, #4              /* compute bytes left to fill out word */
    sub     r2, r2, r6              /* Update mlen                         */

.ARC4_EnCRC_Lead_Loop:

    ldrb    r9, [r1, #0]            /* load data[n]                        */
    add     r4, r4, r5
    and     r4, r4, #0xFF           /* j = (j + S[i]) mod 256              */
    ldrb    r7, [r0, r4]            /* load S[j]                           */
    strb    r5, [r0, r4]            /* store S[i] to &S[j]                 */
    eor     r11, r12, r9            /* u = CRC ^ data[n]                   */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n]) & 0xFF          */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n])&0xFF]    */
    add     r8, r5, r7
    and     r8, r8, #0xFF           /* t = (S[i] + S[j]) mod 256           */
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                */
    strb    r7, [r3], #1            /* store s[j] to &S[i], i++            */
    ldrb    r8, [r0, r8]            /* load S[t]                           */
    bic     r3, r3, #0x100          /* handle rollover on &S[i]            */
    subs    r6, r6, #1              /* decrement byte count (to fill word) */
    eor     r9, r9, r8              /* data[n] ^= S[t]                     */
    strb    r9, [r1], #1            /* store data[n], n++                  */
    ldrb    r5, [r3, #0]            /* load next S[i]                      */
    bne     .ARC4_EnCRC_Lead_Loop

.ARC4_EnCRC_Main:

   /*-----------------------------------------------------------------------
    * Pre-decrement mlen by 4 so that the loop logic works correctly.
    *-----------------------------------------------------------------------
    */

    subs    r2, r2, #4
    bcc     .ARC4_EnCRC_Trail

   /*-----------------------------------------------------------------------
    * Mask &S[i] to determine which loop to drop into.  Each loop is 64
    * instructions, and each instruction is 4 bytes.  Note that 2 dummy
    * instruction must be added since PC actually points 2 instructions
    * into the future (due to pipelining).
    *-----------------------------------------------------------------------
    */

    and     r6, r3, #0x03
    add     pc, pc, r6, LSL #8      /* pc += r3i * 64 * 4 */
    nop                             /* dummy instruction  */

   /*-----------------------------------------------------------------------
    * This loop handles the case of (pre-incremented) i = 0 mod 4 upon
    * entry, i.e. the "rollover" logic must be applied after accessing
    * the 4th (and final) S[i] each time through the loop.
    *-----------------------------------------------------------------------
    */

.ARC4_EnCRC_Main_LoopI0:
                                    /*   r3     r4     r5     r6     r7     r8 */
                                    /* ----------------------------------------*/
    ldrb    r5, [r3, #0]            /*   i1     j0    S[i1]   .      .      .  */
    ldr     r9, [r1, #0]            /*   .      .      .      .      .      .  */
    pld     [r1, #32]               /*   .      .      .      .      .      .  */
    add     r4, r4, r5              /*   .      n1     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j1     .      .      .      .  */
                                    /* ----------------------------------------*/
    eor     r11, r12, GetB0(r9,#)   /* u = CRC ^ data[n]                       */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n]) & 0xFF              */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n])&0xFF]        */
                                    /* ----------------------------------------*/
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j1]   .  */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB1(r9,#)   /* u = CRC ^ data[n+1]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+1]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+1])&0xFF]      */
                                    /* ----------------------------------------*/
    ldrb    r6, [r3, #1]            /*   i2     .      .     S[i2]   .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB2(r9,#)   /* u = CRC ^ data[n+2]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+2]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+2])&0xFF]      */
                                    /* ----------------------------------------*/
    add     r8, r5, r7              /*   .      .      <>     .      <>     s1 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t1 */
    add     r4, r4, r6              /*   .      n2     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j2     .      .      .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB3(r9,#)   /* u = CRC ^ data[n+3]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+3]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+3])&0xFF]      */
                                    /* ----------------------------------------*/
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j2]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K1 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
                                    /* ----------------------------------------*/
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    eor     r9, r9, PutB0(r8,#)     /*   .      .      .      .      .      <> */
    ldrb    r5, [r3, #0]            /*   i3     .     S[i3]   .      .      .  */
    add     r8, r6, r7              /*   .      .      .      <>     <>     s2 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t2 */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K2 */
    add     r4, r4, r5              /*   .      n3     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j3     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j3]   .  */
    eor     r9, r9, PutB1(r8,#)     /*   .      .      .      .      .      <> */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    ldrb    r6, [r3, #0]            /*   i4     .      .     S[i4]   .      .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s3 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t3 */
    add     r4, r4, r6              /*   .      n4     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j4     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j4]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K3 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    add     r5, r6, r7              /*   .      .      s4     <>     <>     .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    bic     r3, r3, #0x100          /*   .      .      .      .      .      .  */
    and     r5, r5, #0xFF           /*   .      .      t4     .      .      .  */
    ldrb    r5, [r0, r5]            /*   .      .      K4     .      .      .  */
    eor     r9, r9, PutB2(r8,#)     /*   .      .      .      .      .      <> */
    subs    r2, r2, #4              /* Decrement buffer length.      .      .  */
    eor     r9, r9, PutB3(r5,#)     /*   .      .      <>     .      .      .  */
    str     r9, [r1], #4
    bcs     .ARC4_EnCRC_Main_LoopI0 /* Loop if not finished                    */
    b       .ARC4_EnCRC_Trail
    nop                             /* dummy instruction                       */

   /*-----------------------------------------------------------------------
    * This loop handles the case of (pre-incremented) i = 1 mod 4 upon
    * entry, i.e. the "rollover" logic must be applied after accessing
    * the 3rd S[i] each time through the loop.
    *-----------------------------------------------------------------------
    */

.ARC4_EnCRC_Main_LoopI1:
                                    /*   r3     r4     r5     r6     r7     r8 */
                                    /* ----------------------------------------*/
    ldrb    r5, [r3, #0]            /*   i1     j0    S[i1]   .      .      .  */
    ldr     r9, [r1, #0]            /*   .      .      .      .      .      .  */
    pld     [r1, #32]               /*   .      .      .      .      .      .  */
    add     r4, r4, r5              /*   .      n1     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j1     .      .      .      .  */
                                    /* ----------------------------------------*/
    eor     r11, r12, GetB0(r9,#)   /* u = CRC ^ data[n]                       */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n]) & 0xFF              */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n])&0xFF]        */
                                    /* ----------------------------------------*/
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j1]   .  */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB1(r9,#)   /* u = CRC ^ data[n+1]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+1]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+1])&0xFF]      */
                                    /* ----------------------------------------*/
    ldrb    r6, [r3, #1]            /*   i2     .      .     S[i2]   .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB2(r9,#)   /* u = CRC ^ data[n+2]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+2]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+2])&0xFF]      */
                                    /* ----------------------------------------*/
    add     r8, r5, r7              /*   .      .      <>     .      <>     s1 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t1 */
    add     r4, r4, r6              /*   .      n2     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j2     .      .      .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB3(r9,#)   /* u = CRC ^ data[n+3]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+3]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+3])&0xFF]      */
                                    /* ----------------------------------------*/
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j2]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K1 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
                                    /* ----------------------------------------*/
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    eor     r9, r9, PutB0(r8,#)     /*   .      .      .      .      .      <> */
    ldrb    r5, [r3, #0]            /*   i3     .     S[i3]   .      .      .  */
    add     r8, r6, r7              /*   .      .      .      <>     <>     s2 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t2 */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K2 */
    add     r4, r4, r5              /*   .      n3     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j3     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j3]   .  */
    eor     r9, r9, PutB1(r8,#)     /*   .      .      .      .      .      <> */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    bic     r3, r3, #0x100          /*   .      .      .      .      .      .  */
    ldrb    r6, [r3, #0]            /*   i4     .      .     S[i4]   .      .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s3 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t3 */
    add     r4, r4, r6              /*   .      n4     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j4     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j4]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K3 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    add     r5, r6, r7              /*   .      .      s4     <>     <>     .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    and     r5, r5, #0xFF           /*   .      .      t4     .      .      .  */
    ldrb    r5, [r0, r5]            /*   .      .      K4     .      .      .  */
    eor     r9, r9, PutB2(r8,#)     /*   .      .      .      .      .      <> */
    subs    r2, r2, #4              /* Decrement buffer length.      .      .  */
    eor     r9, r9, PutB3(r5,#)     /*   .      .      <>     .      .      .  */
    str     r9, [r1], #4
    bcs     .ARC4_EnCRC_Main_LoopI1 /* Loop if not finished                    */
    b       .ARC4_EnCRC_Trail
    nop                             /* dummy instruction                       */

   /*-----------------------------------------------------------------------
    * This loop handles the case of (pre-incremented) i = 2 mod 4 upon
    * entry, i.e. the "rollover" logic must be applied after accessing
    * the 2nd S[i] each time through the loop.
    *-----------------------------------------------------------------------
    */

.ARC4_EnCRC_Main_LoopI2:
                                    /*   r3     r4     r5     r6     r7     r8 */
                                    /* ----------------------------------------*/
    ldrb    r5, [r3, #0]            /*   i1     j0    S[i1]   .      .      .  */
    ldr     r9, [r1, #0]            /*   .      .      .      .      .      .  */
    pld     [r1, #32]               /*   .      .      .      .      .      .  */
    add     r4, r4, r5              /*   .      n1     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j1     .      .      .      .  */
                                    /* ----------------------------------------*/
    eor     r11, r12, GetB0(r9,#)   /* u = CRC ^ data[n]                       */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n]) & 0xFF              */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n])&0xFF]        */
                                    /* ----------------------------------------*/
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j1]   .  */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB1(r9,#)   /* u = CRC ^ data[n+1]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+1]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+1])&0xFF]      */
                                    /* ----------------------------------------*/
    ldrb    r6, [r3, #1]            /*   i2     .      .     S[i2]   .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB2(r9,#)   /* u = CRC ^ data[n+2]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+2]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+2])&0xFF]      */
                                    /* ----------------------------------------*/
    add     r8, r5, r7              /*   .      .      <>     .      <>     s1 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t1 */
    add     r4, r4, r6              /*   .      n2     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j2     .      .      .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB3(r9,#)   /* u = CRC ^ data[n+3]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+3]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+3])&0xFF]      */
                                    /* ----------------------------------------*/
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j2]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K1 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
                                    /* ----------------------------------------*/
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    eor     r9, r9, PutB0(r8,#)     /*   .      .      .      .      .      <> */
    bic     r3, r3, #0x100          /*   .      .      .      .      .      .  */
    ldrb    r5, [r3, #0]            /*   i3     .     S[i3]   .      .      .  */
    add     r8, r6, r7              /*   .      .      .      <>     <>     s2 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t2 */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K2 */
    add     r4, r4, r5              /*   .      n3     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j3     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j3]   .  */
    eor     r9, r9, PutB1(r8,#)     /*   .      .      .      .      .      <> */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    ldrb    r6, [r3, #0]            /*   i4     .      .     S[i4]   .      .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s3 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t3 */
    add     r4, r4, r6              /*   .      n4     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j4     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j4]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K3 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    add     r5, r6, r7              /*   .      .      s4     <>     <>     .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    and     r5, r5, #0xFF           /*   .      .      t4     .      .      .  */
    ldrb    r5, [r0, r5]            /*   .      .      K4     .      .      .  */
    eor     r9, r9, PutB2(r8,#)     /*   .      .      .      .      .      <> */
    subs    r2, r2, #4              /* Decrement buffer length.      .      .  */
    eor     r9, r9, PutB3(r5,#)     /*   .      .      <>     .      .      .  */
    str     r9, [r1], #4
    bcs     .ARC4_EnCRC_Main_LoopI2 /* Loop if not finished                    */
    b       .ARC4_EnCRC_Trail
    nop                             /* dummy instruction                       */

   /*-----------------------------------------------------------------------
    * This loop handles the case of (pre-incremented) i = 3 mod 4 upon
    * entry, i.e. the "rollover" logic must be applied after accessing
    * the 1st S[i] each time through the loop.
    *-----------------------------------------------------------------------
    */

.ARC4_EnCRC_Main_LoopI3:
                                    /*   r3     r4     r5     r6     r7     r8 */
                                    /* ----------------------------------------*/
    ldrb    r5, [r3, #0]            /*   i1     j0    S[i1]   .      .      .  */
    ldr     r9, [r1, #0]            /*   .      .      .      .      .      .  */
    pld     [r1, #32]               /*   .      .      .      .      .      .  */
    add     r4, r4, r5              /*   .      n1     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j1     .      .      .      .  */
                                    /* ----------------------------------------*/
    eor     r11, r12, GetB0(r9,#)   /* u = CRC ^ data[n]                       */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n]) & 0xFF              */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n])&0xFF]        */
                                    /* ----------------------------------------*/
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j1]   .  */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB1(r9,#)   /* u = CRC ^ data[n+1]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+1]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+1])&0xFF]      */
                                    /* ----------------------------------------*/
    subs    r2, r2, #4              /* Decrement buffer length.      .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB2(r9,#)   /* u = CRC ^ data[n+2]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+2]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+2])&0xFF]      */
                                    /* ----------------------------------------*/
    bic     r3, r3, #0x100          /*   .      .      .      .      .      .  */
    ldrb    r6, [r3, #0]            /*   i2     .      .     S[i2]   .      .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s1 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t1 */
    add     r4, r4, r6              /*   .      n2     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j2     .      .      .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB3(r9,#)   /* u = CRC ^ data[n+3]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+3]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+3])&0xFF]      */
                                    /* ----------------------------------------*/
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j2]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K1 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    eor     r9, r9, PutB0(r8,#)     /*   .      .      .      .      .      <> */
    ldrb    r5, [r3, #0]            /*   i3     .     S[i3]   .      .      .  */
    add     r8, r6, r7              /*   .      .      .      <>     <>     s2 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t2 */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K2 */
    add     r4, r4, r5              /*   .      n3     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j3     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j3]   .  */
    eor     r9, r9, PutB1(r8,#)     /*   .      .      .      .      .      <> */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    ldrb    r6, [r3, #0]            /*   i4     .      .     S[i4]   .      .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s3 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t3 */
    add     r4, r4, r6              /*   .      n4     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j4     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j4]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K3 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    add     r5, r6, r7              /*   .      .      s4     <>     <>     .  */
    and     r5, r5, #0xFF           /*   .      .      t4     .      .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    ldrb    r5, [r0, r5]            /*   .      .      K4     .      .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
                                    /* ----------------------------------------*/
    eor     r9, r9, PutB2(r8,#)     /*   .      .      .      .      .      <> */
    eor     r9, r9, PutB3(r5,#)     /*   .      .      <>     .      .      .  */
    str     r9, [r1], #4
    bcs     .ARC4_EnCRC_Main_LoopI3 /* Loop if not finished                    */


   /*-----------------------------------------------------------------------
    * Handle any odd traling data bytes, if any.
    *-----------------------------------------------------------------------
    */

.ARC4_EnCRC_Trail:

    ands    r2, r2, #0x03
    beq     .ARC4_EnCRC_Done

.ARC4_EnCRC_Trail_Loop:

    ldrb    r5, [r3, #0]            /* load S[i]                        */
    ldrb    r9, [r1, #0]            /* load data[n]                     */
    subs    r2, r2, #1              /* decrement byte count             */
    add     r4, r4, r5
    and     r4, r4, #0xFF           /* j = (j + S[i]) mod 256           */
    ldrb    r7, [r0, r4]            /* load S[j]                        */
    strb    r5, [r0, r4]            /* store S[i] to &S[j]              */
    eor     r11, r12, r9            /* u = CRC ^ data[n]                */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n]) & 0xFF       */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n])&0xFF] */
    add     r8, r5, r7
    and     r8, r8, #0xFF           /* t = (S[i] + S[j]) mod 256        */
    strb    r7, [r3], #1            /* store s[j] to &S[i], i++         */
    ldrb    r8, [r0, r8]            /* load S[t]                        */
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)             */
    bic     r3, r3, #0x100          /* handle rollover on &S[i]         */
    eor     r9, r9, r8              /* data[n] ^= S[t]                  */
    strb    r9, [r1], #1            /* store data[n], n++               */
    bne     .ARC4_EnCRC_Trail_Loop

   /*-----------------------------------------------------------------------
    * Pop the registers and write out the update &S[i] and j.
    *-----------------------------------------------------------------------
    */

.ARC4_EnCRC_Done:

    mov     r0, r4                  /* save j           */
    ldmfd   sp!, {r2, r4, r5, r6, r7, r8, r9, r10, r11}
    str     r3, [r2, #0]            /* store &S[i]      */
    str     r0, [r2, #4]            /* store j          */
    str     r12, [r2, #12]          /* store CRC        */
    mov     pc, lr


/*===========================================================================
 * void ARC4_DecryptWithCRC (
 *    UINT8*       pSbox,
 *    MBUF*        pMbuf,
 *    ARC4_STATE*  pState)
 *
 * Performs ARC4 decryption, followed by CRC-32 computation, on the buffer
 * pData (of dataLength bytes) using the (already initialized) S-box in
 * pSbox.  Regardless of the endianness of the data, the CRC is always big
 * endian (i.e. network byte order).  Note that this function does not
 * decrypt the CRC and compare it to the computed CRC.  The application
 * must call ARC4_Crypt to decrypt the CRC and do the comparison itself.
 * The following size and alignment restrictions apply to the arguments:
 *
 *   Argument        Size                Alignment
 *                   (bytes)             (bytes)
 *   --------------------------------------------------------
 *   pSbox           256                 512
 *   pMbuf->mdata    >2 (*)              1 (i.e. arbitrary)
 *
 * (*) The mbuf data cluster may be 1 byte in length if mdata is of the form
 * 4n or 4n+3.  The mbuf data cluster may be 2 bytes in legnth if mdata is 
 * of the form 4n, 4n+2, or 4n+3.
 *
 * The ARC4_STATE structure is defined as follows :
 *
 * typedef struct {
 *    UINT8*      pSi;       // &S[i] (where "i" is RC$ index i)
 *    UINT32      j;         // ARC4 index j
 *    UINT8*      crcTable;  // CRC lookup table
 *    UNIT32      CRC;       // 32-bit CRC value
 * } ARC4_STATE;
 *
 * Prior to the first call to ARC4_EncryptWithCRC, an ARC4_STATE structure
 * must be initialized with pSi set to pSbox+1, j set to 0, and CRC set to
 * 0xFFFFFFFF.
 *===========================================================================
 */

_ARM_FUNCTION(ARC4_DecryptWithCRC)

   /*-----------------------------------------------------------------------
    * Upon entry:
    *   r0  base of S-box (must be 512-byte aligned)
    *   r1  base of plaintext/ciphertext buffer (must be word aligned, but
    *       preferably 32-byte aligned for best cache performance)
    *   r2  length of plaintext/ciphertext buffer
    *-----------------------------------------------------------------------
    */

    stmfd   sp!, {r2, r4, r5, r6, r7, r8, r9, r10, r11}

   /*-----------------------------------------------------------------------
    * RC 4 iteration
    *
    * Register allocation:
    *   r0  base of S-box
    *   r1  plaintext/ciphertext pointer
    *   r2  buffer length
    *   r3  address of S[i] (i.e. &S[i])
    *   r4  index j
    *   r5  S[i]
    *   r6  S[i+1] (must be read after S[i] is written)
    *   r7  S[j]
    *   r8  t = S[i]+S[j] mod 256, or S[t]
    *   r9  plaintext/ciphertext data word
    *
    * Algorithm (per iteration):
    *   m = i + 1
    *   i = m mod 256
    *   n = j + S[i]
    *   j = n mod 256
    *   swap S[i] and S[j]
    *   s = S[i] + S[j]
    *   t = s mod 256
    *   K = S[t]
    *
    * Average cycle count = 2 + 46*(buflen/4) + 4 + 4 = 10 + 11.5 * buflen
    *-----------------------------------------------------------------------
    */

    ldr     r12, [r2, #12]          /* r12 <= CRC                      */
    ldr     r10, [r2, #8]           /* r10 <= &crcTable[0]             */
    ldr     r4, [r2, #4]            /* r4 <= j                         */
    ldr     r3, [r2, #0]            /* r3 <= &S[i]                     */
    ldr     r2, [r1, #12]           /* r2 <= mbuf data length (mlen)   */
    ldr     r1, [r1, #8]            /* r1 <= mbuf data pointer (mdata) */

   /*-----------------------------------------------------------------------
    * Handle any odd leading data bytes, if any, in order to get the data
    * pointer into a word-aligned position.
    *-----------------------------------------------------------------------
    */

.ARC4_DeCRC_Lead:

    ands    r6, r1, #0x3            /* get (data pointer) mod 4 */
    beq     .ARC4_DeCRC_Main

   /* Note: may have to add some logic to handle the case of a very short
    * buffer, i.e. less than 3 bytes, so that it cannot be guaranteed that 
    * there is a word-aligned boundary anywhere within (or at either end 
    * of) the buffer.
    */

    pld     [r1, #4]                /* pre-load data starting at next word */
    ldrb    r5, [r3, #0]            /* load S[i]                           */
    rsb     r6, r6, #4              /* compute bytes left to fill out word */
    sub     r2, r2, r6              /* Update mlen                         */

.ARC4_DeCRC_Lead_Loop:

    add     r4, r4, r5
    and     r4, r4, #0xFF           /* j = (j + S[i]) mod 256              */
    ldrb    r7, [r0, r4]            /* load S[j]                           */
    strb    r5, [r0, r4]            /* store S[i] to &S[j]                 */
    ldrb    r9, [r1, #0]            /* load data[n]                        */
    add     r8, r5, r7
    and     r8, r8, #0xFF           /* t = (S[i] + S[j]) mod 256           */
    strb    r7, [r3], #1            /* store s[j] to &S[i], i++            */
    ldrb    r8, [r0, r8]            /* load S[t]                           */
    bic     r3, r3, #0x100          /* handle rollover on &S[i]            */
    subs    r6, r6, #1              /* decrement byte count (to fill word) */
    eor     r9, r9, r8              /* data[n] ^= S[t]                     */
    eor     r11, r12, r9            /* u = CRC ^ data[n]                   */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n]) & 0xFF          */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n])&0xFF]    */
    strb    r9, [r1], #1            /* store data[n], n++                  */
    ldrb    r5, [r3, #0]            /* load next S[i]                      */
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                */
    bne     .ARC4_DeCRC_Lead_Loop

.ARC4_DeCRC_Main:

   /*-----------------------------------------------------------------------
    * Pre-decrement mlen by 4 so that the loop logic works correctly.
    *-----------------------------------------------------------------------
    */

    subs    r2, r2, #4
    bcc     .ARC4_DeCRC_Trail

   /*-----------------------------------------------------------------------
    * Mask &S[i] to determine which loop to drop into.  Each loop is 64
    * instructions, and each instruction is 4 bytes.  Note that 2 dummy
    * instruction must be added since PC actually points 2 instructions
    * into the future (due to pipelining).
    *-----------------------------------------------------------------------
    */

    and     r6, r3, #0x03
    add     pc, pc, r6, LSL #8      /* pc += r3i * 64 * 4 */
    nop                             /* dummy instruction  */

   /*-----------------------------------------------------------------------
    * This loop handles the case of (pre-incremented) i = 0 mod 4 upon
    * entry, i.e. the "rollover" logic must be applied after accessing
    * the 4th (and final) S[i] each time through the loop.
    *-----------------------------------------------------------------------
    */

.ARC4_DeCRC_Main_LoopI0:
                                    /*   r3     r4     r5     r6     r7     r8 */
                                    /* ----------------------------------------*/
    ldrb    r5, [r3, #0]            /*   i1     j0    S[i1]   .      .      .  */
    ldr     r9, [r1, #0]            /*   .      .      .      .      .      .  */
    pld     [r1, #32]               /*   .      .      .      .      .      .  */
    add     r4, r4, r5              /*   .      n1     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j1     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j1]   .  */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    ldrb    r6, [r3, #1]            /*   i2     .      .     S[i2]   .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s1 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t1 */
    add     r4, r4, r6              /*   .      n2     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j2     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j2]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K1 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    eor     r9, r9, PutB0(r8,#)     /*   .      .      .      .      .      <> */
                                    /* ----------------------------------------*/
    eor     r11, r12, GetB0(r9,#)   /* u = CRC ^ data[n]                       */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n]) & 0xFF              */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n])&0xFF]        */
                                    /* ----------------------------------------*/
    ldrb    r5, [r3, #0]            /*   i3     .     S[i3]   .      .      .  */
    add     r8, r6, r7              /*   .      .      .      <>     <>     s2 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t2 */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K2 */
    add     r4, r4, r5              /*   .      n3     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j3     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j3]   .  */
    eor     r9, r9, PutB1(r8,#)     /*   .      .      .      .      .      <> */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB1(r9,#)   /* u = CRC ^ data[n+1]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+1]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+1])&0xFF]      */
                                    /* ----------------------------------------*/
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    ldrb    r6, [r3, #0]            /*   i4     .      .     S[i4]   .      .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s3 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t3 */
    add     r4, r4, r6              /*   .      n4     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j4     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j4]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K3 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    add     r5, r6, r7              /*   .      .      s4     <>     <>     .  */
    eor     r9, r9, PutB2(r8,#)     /*   .      .      .      .      .      <> */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB2(r9,#)   /* u = CRC ^ data[n+2]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+2]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+2])&0xFF]      */
                                    /* ----------------------------------------*/
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    and     r5, r5, #0xFF           /*   .      .      t4     .      .      .  */
    ldrb    r5, [r0, r5]            /*   .      .      K4     .      .      .  */
    bic     r3, r3, #0x100          /*   .      .      .      .      .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
                                    /* ----------------------------------------*/
    eor     r9, r9, PutB3(r5,#)     /*   .      .      <>     .      .      .  */
                                    /* ----------------------------------------*/
    eor     r11, r12, GetB3(r9,#)   /* u = CRC ^ data[n+3]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+3]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+3])&0xFF]      */
                                    /* ----------------------------------------*/
    str     r9, [r1], #4            /*   .      .      .      .      .      .  */
    subs    r2, r2, #4              /* Decrement buffer length.      .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
                                    /* ----------------------------------------*/
    bcs     .ARC4_DeCRC_Main_LoopI0 /* Loop if not finished                    */
    b       .ARC4_DeCRC_Trail
    nop                             /* dummy instruction                       */

   /*-----------------------------------------------------------------------
    * This loop handles the case of (pre-incremented) i = 1 mod 4 upon
    * entry, i.e. the "rollover" logic must be applied after accessing
    * the 3rd S[i] each time through the loop.
    *-----------------------------------------------------------------------
    */

                                    /*   r3     r4     r5     r6     r7     r8 */
                                    /* ----------------------------------------*/
    ldrb    r5, [r3, #0]            /*   i1     j0    S[i1]   .      .      .  */
.ARC4_DeCRC_Main_LoopI1:
    ldr     r9, [r1, #0]            /*   .      .      .      .      .      .  */
    pld     [r1, #32]               /*   .      .      .      .      .      .  */
    add     r4, r4, r5              /*   .      n1     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j1     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j1]   .  */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    ldrb    r6, [r3, #1]            /*   i2     .      .     S[i2]   .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s1 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t1 */
    add     r4, r4, r6              /*   .      n2     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j2     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j2]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K1 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    eor     r9, r9, PutB0(r8,#)     /*   .      .      .      .      .      <> */
                                    /* ----------------------------------------*/
    eor     r11, r12, GetB0(r9,#)   /* u = CRC ^ data[n]                       */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n]) & 0xFF              */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n])&0xFF]        */
                                    /* ----------------------------------------*/
    ldrb    r5, [r3, #0]            /*   i3     .     S[i3]   .      .      .  */
    add     r8, r6, r7              /*   .      .      .      <>     <>     s2 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t2 */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K2 */
    add     r4, r4, r5              /*   .      n3     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j3     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j3]   .  */
    eor     r9, r9, PutB1(r8,#)     /*   .      .      .      .      .      <> */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB1(r9,#)   /* u = CRC ^ data[n+1]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+1]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+1])&0xFF]      */
                                    /* ----------------------------------------*/
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    bic     r3, r3, #0x100          /*   .      .      .      .      .      .  */
    ldrb    r6, [r3, #0]            /*   i4     .      .     S[i4]   .      .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s3 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t3 */
    add     r4, r4, r6              /*   .      n4     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j4     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j4]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K3 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    add     r5, r6, r7              /*   .      .      s4     <>     <>     .  */
    eor     r9, r9, PutB2(r8,#)     /*   .      .      .      .      .      <> */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB2(r9,#)   /* u = CRC ^ data[n+2]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+2]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+2])&0xFF]      */
                                    /* ----------------------------------------*/
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    and     r5, r5, #0xFF           /*   .      .      t4     .      .      .  */
    ldrb    r5, [r0, r5]            /*   .      .      K4     .      .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
                                    /* ----------------------------------------*/
    subs    r2, r2, #4              /* Decrement buffer length.      .      .  */
    eor     r9, r9, PutB3(r5,#)     /*   .      .      <>     .      .      .  */
                                    /* ----------------------------------------*/
    eor     r11, r12, GetB3(r9,#)   /* u = CRC ^ data[n+3]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+3]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+3])&0xFF]      */
                                    /* ----------------------------------------*/
    str     r9, [r1], #4            /*   .      .      .      .      .      .  */
    ldrb    r5, [r3, #0]            /*   i1     j0    S[i1]   .      .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
                                    /* ----------------------------------------*/
    bcs     .ARC4_DeCRC_Main_LoopI1 /* Loop if not finished                    */
    b       .ARC4_DeCRC_Trail

   /*-----------------------------------------------------------------------
    * This loop handles the case of (pre-incremented) i = 2 mod 4 upon
    * entry, i.e. the "rollover" logic must be applied after accessing
    * the 2nd S[i] each time through the loop.
    *-----------------------------------------------------------------------
    */

                                    /*   r3     r4     r5     r6     r7     r8 */
                                    /* ----------------------------------------*/
    ldrb    r5, [r3, #0]            /*   i1     j0    S[i1]   .      .      .  */
.ARC4_DeCRC_Main_LoopI2:
    ldr     r9, [r1, #0]            /*   .      .      .      .      .      .  */
    pld     [r1, #32]               /*   .      .      .      .      .      .  */
    add     r4, r4, r5              /*   .      n1     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j1     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j1]   .  */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    ldrb    r6, [r3, #1]            /*   i2     .      .     S[i2]   .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s1 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t1 */
    add     r4, r4, r6              /*   .      n2     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j2     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j2]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K1 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    eor     r9, r9, PutB0(r8,#)     /*   .      .      .      .      .      <> */
                                    /* ----------------------------------------*/
    eor     r11, r12, GetB0(r9,#)   /* u = CRC ^ data[n]                       */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n]) & 0xFF              */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n])&0xFF]        */
                                    /* ----------------------------------------*/
    bic     r3, r3, #0x100          /*   .      .      .      .      .      .  */
    ldrb    r5, [r3, #0]            /*   i3     .     S[i3]   .      .      .  */
    add     r8, r6, r7              /*   .      .      .      <>     <>     s2 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t2 */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K2 */
    add     r4, r4, r5              /*   .      n3     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j3     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j3]   .  */
    eor     r9, r9, PutB1(r8,#)     /*   .      .      .      .      .      <> */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB1(r9,#)   /* u = CRC ^ data[n+1]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+1]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+1])&0xFF]      */
                                    /* ----------------------------------------*/
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    ldrb    r6, [r3, #0]            /*   i4     .      .     S[i4]   .      .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s3 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t3 */
    add     r4, r4, r6              /*   .      n4     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j4     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j4]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K3 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    add     r5, r6, r7              /*   .      .      s4     <>     <>     .  */
    eor     r9, r9, PutB2(r8,#)     /*   .      .      .      .      .      <> */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB2(r9,#)   /* u = CRC ^ data[n+2]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+2]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+2])&0xFF]      */
                                    /* ----------------------------------------*/
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    and     r5, r5, #0xFF           /*   .      .      t4     .      .      .  */
    ldrb    r5, [r0, r5]            /*   .      .      K4     .      .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
                                    /* ----------------------------------------*/
    subs    r2, r2, #4              /* Decrement buffer length.      .      .  */
    eor     r9, r9, PutB3(r5,#)     /*   .      .      <>     .      .      .  */
                                    /* ----------------------------------------*/
    eor     r11, r12, GetB3(r9,#)   /* u = CRC ^ data[n+3]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+3]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+3])&0xFF]      */
                                    /* ----------------------------------------*/
    str     r9, [r1], #4            /*   .      .      .      .      .      .  */
    ldrb    r5, [r3, #0]            /*   i1     j0    S[i1]   .      .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
                                    /* ----------------------------------------*/
    bcs     .ARC4_DeCRC_Main_LoopI2 /* Loop if not finished                    */
    b       .ARC4_DeCRC_Trail

   /*-----------------------------------------------------------------------
    * This loop handles the case of (pre-incremented) i = 3 mod 4 upon
    * entry, i.e. the "rollover" logic must be applied after accessing
    * the 1st S[i] each time through the loop.
    *-----------------------------------------------------------------------
    */

                                    /*   r3     r4     r5     r6     r7     r8 */
                                    /* ----------------------------------------*/
    ldrb    r5, [r3, #0]            /*   i1     j0    S[i1]   .      .      .  */
    /* delay slot */
.ARC4_DeCRC_Main_LoopI3:
    pld     [r1, #32]               /*   .      .      .      .      .      .  */
    add     r4, r4, r5              /*   .      n1     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j1     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j1]   .  */
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    ldr     r9, [r1, #0]            /*   .      .      .      .      .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    bic     r3, r3, #0x100          /*   .      .      .      .      .      .  */
    ldrb    r6, [r3, #0]            /*   i2     .      .     S[i2]   .      .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s1 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t1 */
    add     r4, r4, r6              /*   .      n2     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j2     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j2]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K1 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    eor     r9, r9, PutB0(r8,#)     /*   .      .      .      .      .      <> */
                                    /* ----------------------------------------*/
    eor     r11, r12, GetB0(r9,#)   /* u = CRC ^ data[n]                       */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n]) & 0xFF              */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n])&0xFF]        */
                                    /* ----------------------------------------*/
    ldrb    r5, [r3, #0]            /*   i3     .     S[i3]   .      .      .  */
    add     r8, r6, r7              /*   .      .      .      <>     <>     s2 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t2 */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K2 */
    add     r4, r4, r5              /*   .      n3     <>     .      .      .  */
    and     r4, r4, #0xFF           /*   .      j3     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j3]   .  */
    eor     r9, r9, PutB1(r8,#)     /*   .      .      .      .      .      <> */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB1(r9,#)   /* u = CRC ^ data[n+1]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+1]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+1])&0xFF]      */
                                    /* ----------------------------------------*/
    strb    r5, [r0, r4]            /*   .      .      @@     .      .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    ldrb    r6, [r3, #0]            /*   i4     .      .     S[i4]   .      .  */
    add     r8, r5, r7              /*   .      .      <>     .      <>     s3 */
    and     r8, r8, #0xFF           /*   .      .      .      .      .      t3 */
    add     r4, r4, r6              /*   .      n4     .      <>     .      .  */
    and     r4, r4, #0xFF           /*   .      j4     .      .      .      .  */
    ldrb    r7, [r0, r4]            /*   .      .      .      .     S[j4]   .  */
    ldrb    r8, [r0, r8]            /*   .      .      .      .      .      K3 */
    strb    r6, [r0, r4]            /*   .      .      .      @@     .      .  */
    add     r5, r6, r7              /*   .      .      s4     <>     <>     .  */
    eor     r9, r9, PutB2(r8,#)     /*   .      .      .      .      .      <> */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
    eor     r11, r12, GetB2(r9,#)   /* u = CRC ^ data[n+2]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+2]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+2])&0xFF]      */
                                    /* ----------------------------------------*/
    and     r5, r5, #0xFF           /*   .      .      t4     .      .      .  */
    strb    r7, [r3], #1            /*   .      .      .      .      @@     .  */
    ldrb    r5, [r0, r5]            /*   .      .      K4     .      .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
                                    /* ----------------------------------------*/
    subs    r2, r2, #4              /* Decrement buffer length.      .      .  */
    eor     r9, r9, PutB3(r5,#)     /*   .      .      <>     .      .      .  */
                                    /* ----------------------------------------*/
    eor     r11, r12, GetB3(r9,#)   /* u = CRC ^ data[n+3]                     */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n+3]) & 0xFF            */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n+3])&0xFF]      */
                                    /* ----------------------------------------*/
    str     r9, [r1], #4            /*   .      .      .      .      .      .  */
    ldrb    r5, [r3, #0]            /*   i1     j0    S[i1]   .      .      .  */
                                    /* ----------------------------------------*/
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)                    */
                                    /* ----------------------------------------*/
    bcs     .ARC4_DeCRC_Main_LoopI3 /* Loop if not finished                    */


   /*-----------------------------------------------------------------------
    * Handle any odd traling data bytes, if any.
    *-----------------------------------------------------------------------
    */

.ARC4_DeCRC_Trail:

    ands    r2, r2, #0x03
    beq     .ARC4_DeCRC_Done
    ldrb    r5, [r3, #0]            /* load S[i] */
    /* delay slot X 2 */

.ARC4_DeCRC_Trail_Loop:

    add     r4, r4, r5
    and     r4, r4, #0xFF           /* j = (j + S[i]) mod 256           */
    ldrb    r7, [r0, r4]            /* load S[j]                        */
    strb    r5, [r0, r4]            /* store S[i] to &S[j]              */
    ldrb    r9, [r1, #0]            /* load data[n]                     */
    strb    r7, [r3], #1            /* store s[j] to &S[i], i++         */
    add     r8, r5, r7
    and     r8, r8, #0xFF           /* t = (S[i] + S[j]) mod 256        */
    ldrb    r8, [r0, r8]            /* load S[t]                        */
    bic     r3, r3, #0x100          /* handle rollover on &S[i]         */
    subs    r2, r2, #1              /* decrement byte count             */
    eor     r9, r9, r8              /* data[n] ^= S[t]                  */
    eor     r11, r12, r9            /* u = CRC ^ data[n]                */
    and     r11, r11, #0xFF         /* u = (CRC ^ data[n]) & 0xFF       */
    ldr     r11, [r10, r11, LSL #2] /* u = crcTable[(CRC^data[n])&0xFF] */
    strb    r9, [r1], #1            /* store data[n], n++               */
    ldrb    r5, [r3, #0]            /* load next S[i]                   */
    eor     r12, r11, r12, LSR #8   /* CRC = u ^ (CRC >> 8)             */
    bne     .ARC4_DeCRC_Trail_Loop

   /*-----------------------------------------------------------------------
    * Pop the registers and write out the update &S[i] and j.
    *-----------------------------------------------------------------------
    */

.ARC4_DeCRC_Done:

    mov     r0, r4                  /* save j           */
    ldmfd   sp!, {r2, r4, r5, r6, r7, r8, r9, r10, r11}
    str     r3, [r2, #0]            /* store &S[i]      */
    str     r0, [r2, #4]            /* store j          */
    str     r12, [r2, #12]          /* store CRC        */
    mov     pc, lr






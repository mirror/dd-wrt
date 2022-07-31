/*
 * Copyright 2004-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#if defined(__aarch64__)

.text
.align	6
.type	.LK512,%object
.LK512:
.quad	0x428a2f98d728ae22,0x7137449123ef65cd
.quad	0xb5c0fbcfec4d3b2f,0xe9b5dba58189dbbc
.quad	0x3956c25bf348b538,0x59f111f1b605d019
.quad	0x923f82a4af194f9b,0xab1c5ed5da6d8118
.quad	0xd807aa98a3030242,0x12835b0145706fbe
.quad	0x243185be4ee4b28c,0x550c7dc3d5ffb4e2
.quad	0x72be5d74f27b896f,0x80deb1fe3b1696b1
.quad	0x9bdc06a725c71235,0xc19bf174cf692694
.quad	0xe49b69c19ef14ad2,0xefbe4786384f25e3
.quad	0x0fc19dc68b8cd5b5,0x240ca1cc77ac9c65
.quad	0x2de92c6f592b0275,0x4a7484aa6ea6e483
.quad	0x5cb0a9dcbd41fbd4,0x76f988da831153b5
.quad	0x983e5152ee66dfab,0xa831c66d2db43210
.quad	0xb00327c898fb213f,0xbf597fc7beef0ee4
.quad	0xc6e00bf33da88fc2,0xd5a79147930aa725
.quad	0x06ca6351e003826f,0x142929670a0e6e70
.quad	0x27b70a8546d22ffc,0x2e1b21385c26c926
.quad	0x4d2c6dfc5ac42aed,0x53380d139d95b3df
.quad	0x650a73548baf63de,0x766a0abb3c77b2a8
.quad	0x81c2c92e47edaee6,0x92722c851482353b
.quad	0xa2bfe8a14cf10364,0xa81a664bbc423001
.quad	0xc24b8b70d0f89791,0xc76c51a30654be30
.quad	0xd192e819d6ef5218,0xd69906245565a910
.quad	0xf40e35855771202a,0x106aa07032bbd1b8
.quad	0x19a4c116b8d2d0c8,0x1e376c085141ab53
.quad	0x2748774cdf8eeb99,0x34b0bcb5e19b48a8
.quad	0x391c0cb3c5c95a63,0x4ed8aa4ae3418acb
.quad	0x5b9cca4f7763e373,0x682e6ff3d6b2b8a3
.quad	0x748f82ee5defb2fc,0x78a5636f43172f60
.quad	0x84c87814a1f0ab72,0x8cc702081a6439ec
.quad	0x90befffa23631e28,0xa4506cebde82bde9
.quad	0xbef9a3f7b2c67915,0xc67178f2e372532b
.quad	0xca273eceea26619c,0xd186b8c721c0c207
.quad	0xeada7dd6cde0eb1e,0xf57d4f7fee6ed178
.quad	0x06f067aa72176fba,0x0a637dc5a2c898a6
.quad	0x113f9804bef90dae,0x1b710b35131c471b
.quad	0x28db77f523047d84,0x32caab7b40c72493
.quad	0x3c9ebe0a15c9bebc,0x431d67c49c100d4c
.quad	0x4cc5d4becb3e42b6,0x597f299cfc657e2a
.quad	0x5fcb6fab3ad6faec,0x6c44198c4a475817
.quad	0	// terminator
.size	.LK512,.-.LK512
.byte	83,72,65,53,49,50,32,98,108,111,99,107,32,116,114,97,110,115,102,111,114,109,32,102,111,114,32,65,82,77,118,56,44,32,67,82,89,80,84,79,71,65,77,83,32,98,121,32,60,97,112,112,114,111,64,111,112,101,110,115,115,108,46,111,114,103,62,0
.align	2
.align	2

.globl sha512_block_armv8_ce
.type	sha512_block_armv8_ce,%function
.align	6
sha512_block_armv8_ce:
.Lv8_entry:
	stp	x29,x30,[sp,#-16]!
	add	x29,sp,#0

	ld1	{v16.16b,v17.16b,v18.16b,v19.16b},[x1],#64	// load input
	ld1	{v20.16b,v21.16b,v22.16b,v23.16b},[x1],#64

	ld1	{v0.2d,v1.2d,v2.2d,v3.2d},[x0]		// load context
	adr	x3,.LK512

	rev64	v16.16b,v16.16b
	rev64	v17.16b,v17.16b
	rev64	v18.16b,v18.16b
	rev64	v19.16b,v19.16b
	rev64	v20.16b,v20.16b
	rev64	v21.16b,v21.16b
	rev64	v22.16b,v22.16b
	rev64	v23.16b,v23.16b
	b	.Loop_hw

.align	4
.Loop_hw:
	ld1	{v24.2d},[x3],#16
	subs	x2,x2,#1
	sub	x4,x1,#128
	orr	v26.16b,v0.16b,v0.16b			// offload
	orr	v27.16b,v1.16b,v1.16b
	orr	v28.16b,v2.16b,v2.16b
	orr	v29.16b,v3.16b,v3.16b
	csel	x1,x1,x4,ne			// conditional rewind
	add	v24.2d,v24.2d,v16.2d
	ld1	{v25.2d},[x3],#16
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v2.16b,v3.16b,#8
	ext	v6.16b,v1.16b,v2.16b,#8
	add	v3.2d,v3.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xcec08230	//sha512su0 v16.16b,v17.16b
	ext	v7.16b,v20.16b,v21.16b,#8
.inst	0xce6680a3	//sha512h v3.16b,v5.16b,v6.16b
.inst	0xce678af0	//sha512su1 v16.16b,v23.16b,v7.16b
	add	v4.2d,v1.2d,v3.2d		// "D + T1"
.inst	0xce608423	//sha512h2 v3.16b,v1.16b,v0.16b
	add	v25.2d,v25.2d,v17.2d
	ld1	{v24.2d},[x3],#16
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v4.16b,v2.16b,#8
	ext	v6.16b,v0.16b,v4.16b,#8
	add	v2.2d,v2.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xcec08251	//sha512su0 v17.16b,v18.16b
	ext	v7.16b,v21.16b,v22.16b,#8
.inst	0xce6680a2	//sha512h v2.16b,v5.16b,v6.16b
.inst	0xce678a11	//sha512su1 v17.16b,v16.16b,v7.16b
	add	v1.2d,v0.2d,v2.2d		// "D + T1"
.inst	0xce638402	//sha512h2 v2.16b,v0.16b,v3.16b
	add	v24.2d,v24.2d,v18.2d
	ld1	{v25.2d},[x3],#16
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v1.16b,v4.16b,#8
	ext	v6.16b,v3.16b,v1.16b,#8
	add	v4.2d,v4.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xcec08272	//sha512su0 v18.16b,v19.16b
	ext	v7.16b,v22.16b,v23.16b,#8
.inst	0xce6680a4	//sha512h v4.16b,v5.16b,v6.16b
.inst	0xce678a32	//sha512su1 v18.16b,v17.16b,v7.16b
	add	v0.2d,v3.2d,v4.2d		// "D + T1"
.inst	0xce628464	//sha512h2 v4.16b,v3.16b,v2.16b
	add	v25.2d,v25.2d,v19.2d
	ld1	{v24.2d},[x3],#16
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v0.16b,v1.16b,#8
	ext	v6.16b,v2.16b,v0.16b,#8
	add	v1.2d,v1.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xcec08293	//sha512su0 v19.16b,v20.16b
	ext	v7.16b,v23.16b,v16.16b,#8
.inst	0xce6680a1	//sha512h v1.16b,v5.16b,v6.16b
.inst	0xce678a53	//sha512su1 v19.16b,v18.16b,v7.16b
	add	v3.2d,v2.2d,v1.2d		// "D + T1"
.inst	0xce648441	//sha512h2 v1.16b,v2.16b,v4.16b
	add	v24.2d,v24.2d,v20.2d
	ld1	{v25.2d},[x3],#16
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v3.16b,v0.16b,#8
	ext	v6.16b,v4.16b,v3.16b,#8
	add	v0.2d,v0.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xcec082b4	//sha512su0 v20.16b,v21.16b
	ext	v7.16b,v16.16b,v17.16b,#8
.inst	0xce6680a0	//sha512h v0.16b,v5.16b,v6.16b
.inst	0xce678a74	//sha512su1 v20.16b,v19.16b,v7.16b
	add	v2.2d,v4.2d,v0.2d		// "D + T1"
.inst	0xce618480	//sha512h2 v0.16b,v4.16b,v1.16b
	add	v25.2d,v25.2d,v21.2d
	ld1	{v24.2d},[x3],#16
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v2.16b,v3.16b,#8
	ext	v6.16b,v1.16b,v2.16b,#8
	add	v3.2d,v3.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xcec082d5	//sha512su0 v21.16b,v22.16b
	ext	v7.16b,v17.16b,v18.16b,#8
.inst	0xce6680a3	//sha512h v3.16b,v5.16b,v6.16b
.inst	0xce678a95	//sha512su1 v21.16b,v20.16b,v7.16b
	add	v4.2d,v1.2d,v3.2d		// "D + T1"
.inst	0xce608423	//sha512h2 v3.16b,v1.16b,v0.16b
	add	v24.2d,v24.2d,v22.2d
	ld1	{v25.2d},[x3],#16
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v4.16b,v2.16b,#8
	ext	v6.16b,v0.16b,v4.16b,#8
	add	v2.2d,v2.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xcec082f6	//sha512su0 v22.16b,v23.16b
	ext	v7.16b,v18.16b,v19.16b,#8
.inst	0xce6680a2	//sha512h v2.16b,v5.16b,v6.16b
.inst	0xce678ab6	//sha512su1 v22.16b,v21.16b,v7.16b
	add	v1.2d,v0.2d,v2.2d		// "D + T1"
.inst	0xce638402	//sha512h2 v2.16b,v0.16b,v3.16b
	add	v25.2d,v25.2d,v23.2d
	ld1	{v24.2d},[x3],#16
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v1.16b,v4.16b,#8
	ext	v6.16b,v3.16b,v1.16b,#8
	add	v4.2d,v4.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xcec08217	//sha512su0 v23.16b,v16.16b
	ext	v7.16b,v19.16b,v20.16b,#8
.inst	0xce6680a4	//sha512h v4.16b,v5.16b,v6.16b
.inst	0xce678ad7	//sha512su1 v23.16b,v22.16b,v7.16b
	add	v0.2d,v3.2d,v4.2d		// "D + T1"
.inst	0xce628464	//sha512h2 v4.16b,v3.16b,v2.16b
	add	v24.2d,v24.2d,v16.2d
	ld1	{v25.2d},[x3],#16
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v0.16b,v1.16b,#8
	ext	v6.16b,v2.16b,v0.16b,#8
	add	v1.2d,v1.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xcec08230	//sha512su0 v16.16b,v17.16b
	ext	v7.16b,v20.16b,v21.16b,#8
.inst	0xce6680a1	//sha512h v1.16b,v5.16b,v6.16b
.inst	0xce678af0	//sha512su1 v16.16b,v23.16b,v7.16b
	add	v3.2d,v2.2d,v1.2d		// "D + T1"
.inst	0xce648441	//sha512h2 v1.16b,v2.16b,v4.16b
	add	v25.2d,v25.2d,v17.2d
	ld1	{v24.2d},[x3],#16
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v3.16b,v0.16b,#8
	ext	v6.16b,v4.16b,v3.16b,#8
	add	v0.2d,v0.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xcec08251	//sha512su0 v17.16b,v18.16b
	ext	v7.16b,v21.16b,v22.16b,#8
.inst	0xce6680a0	//sha512h v0.16b,v5.16b,v6.16b
.inst	0xce678a11	//sha512su1 v17.16b,v16.16b,v7.16b
	add	v2.2d,v4.2d,v0.2d		// "D + T1"
.inst	0xce618480	//sha512h2 v0.16b,v4.16b,v1.16b
	add	v24.2d,v24.2d,v18.2d
	ld1	{v25.2d},[x3],#16
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v2.16b,v3.16b,#8
	ext	v6.16b,v1.16b,v2.16b,#8
	add	v3.2d,v3.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xcec08272	//sha512su0 v18.16b,v19.16b
	ext	v7.16b,v22.16b,v23.16b,#8
.inst	0xce6680a3	//sha512h v3.16b,v5.16b,v6.16b
.inst	0xce678a32	//sha512su1 v18.16b,v17.16b,v7.16b
	add	v4.2d,v1.2d,v3.2d		// "D + T1"
.inst	0xce608423	//sha512h2 v3.16b,v1.16b,v0.16b
	add	v25.2d,v25.2d,v19.2d
	ld1	{v24.2d},[x3],#16
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v4.16b,v2.16b,#8
	ext	v6.16b,v0.16b,v4.16b,#8
	add	v2.2d,v2.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xcec08293	//sha512su0 v19.16b,v20.16b
	ext	v7.16b,v23.16b,v16.16b,#8
.inst	0xce6680a2	//sha512h v2.16b,v5.16b,v6.16b
.inst	0xce678a53	//sha512su1 v19.16b,v18.16b,v7.16b
	add	v1.2d,v0.2d,v2.2d		// "D + T1"
.inst	0xce638402	//sha512h2 v2.16b,v0.16b,v3.16b
	add	v24.2d,v24.2d,v20.2d
	ld1	{v25.2d},[x3],#16
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v1.16b,v4.16b,#8
	ext	v6.16b,v3.16b,v1.16b,#8
	add	v4.2d,v4.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xcec082b4	//sha512su0 v20.16b,v21.16b
	ext	v7.16b,v16.16b,v17.16b,#8
.inst	0xce6680a4	//sha512h v4.16b,v5.16b,v6.16b
.inst	0xce678a74	//sha512su1 v20.16b,v19.16b,v7.16b
	add	v0.2d,v3.2d,v4.2d		// "D + T1"
.inst	0xce628464	//sha512h2 v4.16b,v3.16b,v2.16b
	add	v25.2d,v25.2d,v21.2d
	ld1	{v24.2d},[x3],#16
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v0.16b,v1.16b,#8
	ext	v6.16b,v2.16b,v0.16b,#8
	add	v1.2d,v1.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xcec082d5	//sha512su0 v21.16b,v22.16b
	ext	v7.16b,v17.16b,v18.16b,#8
.inst	0xce6680a1	//sha512h v1.16b,v5.16b,v6.16b
.inst	0xce678a95	//sha512su1 v21.16b,v20.16b,v7.16b
	add	v3.2d,v2.2d,v1.2d		// "D + T1"
.inst	0xce648441	//sha512h2 v1.16b,v2.16b,v4.16b
	add	v24.2d,v24.2d,v22.2d
	ld1	{v25.2d},[x3],#16
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v3.16b,v0.16b,#8
	ext	v6.16b,v4.16b,v3.16b,#8
	add	v0.2d,v0.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xcec082f6	//sha512su0 v22.16b,v23.16b
	ext	v7.16b,v18.16b,v19.16b,#8
.inst	0xce6680a0	//sha512h v0.16b,v5.16b,v6.16b
.inst	0xce678ab6	//sha512su1 v22.16b,v21.16b,v7.16b
	add	v2.2d,v4.2d,v0.2d		// "D + T1"
.inst	0xce618480	//sha512h2 v0.16b,v4.16b,v1.16b
	add	v25.2d,v25.2d,v23.2d
	ld1	{v24.2d},[x3],#16
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v2.16b,v3.16b,#8
	ext	v6.16b,v1.16b,v2.16b,#8
	add	v3.2d,v3.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xcec08217	//sha512su0 v23.16b,v16.16b
	ext	v7.16b,v19.16b,v20.16b,#8
.inst	0xce6680a3	//sha512h v3.16b,v5.16b,v6.16b
.inst	0xce678ad7	//sha512su1 v23.16b,v22.16b,v7.16b
	add	v4.2d,v1.2d,v3.2d		// "D + T1"
.inst	0xce608423	//sha512h2 v3.16b,v1.16b,v0.16b
	add	v24.2d,v24.2d,v16.2d
	ld1	{v25.2d},[x3],#16
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v4.16b,v2.16b,#8
	ext	v6.16b,v0.16b,v4.16b,#8
	add	v2.2d,v2.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xcec08230	//sha512su0 v16.16b,v17.16b
	ext	v7.16b,v20.16b,v21.16b,#8
.inst	0xce6680a2	//sha512h v2.16b,v5.16b,v6.16b
.inst	0xce678af0	//sha512su1 v16.16b,v23.16b,v7.16b
	add	v1.2d,v0.2d,v2.2d		// "D + T1"
.inst	0xce638402	//sha512h2 v2.16b,v0.16b,v3.16b
	add	v25.2d,v25.2d,v17.2d
	ld1	{v24.2d},[x3],#16
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v1.16b,v4.16b,#8
	ext	v6.16b,v3.16b,v1.16b,#8
	add	v4.2d,v4.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xcec08251	//sha512su0 v17.16b,v18.16b
	ext	v7.16b,v21.16b,v22.16b,#8
.inst	0xce6680a4	//sha512h v4.16b,v5.16b,v6.16b
.inst	0xce678a11	//sha512su1 v17.16b,v16.16b,v7.16b
	add	v0.2d,v3.2d,v4.2d		// "D + T1"
.inst	0xce628464	//sha512h2 v4.16b,v3.16b,v2.16b
	add	v24.2d,v24.2d,v18.2d
	ld1	{v25.2d},[x3],#16
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v0.16b,v1.16b,#8
	ext	v6.16b,v2.16b,v0.16b,#8
	add	v1.2d,v1.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xcec08272	//sha512su0 v18.16b,v19.16b
	ext	v7.16b,v22.16b,v23.16b,#8
.inst	0xce6680a1	//sha512h v1.16b,v5.16b,v6.16b
.inst	0xce678a32	//sha512su1 v18.16b,v17.16b,v7.16b
	add	v3.2d,v2.2d,v1.2d		// "D + T1"
.inst	0xce648441	//sha512h2 v1.16b,v2.16b,v4.16b
	add	v25.2d,v25.2d,v19.2d
	ld1	{v24.2d},[x3],#16
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v3.16b,v0.16b,#8
	ext	v6.16b,v4.16b,v3.16b,#8
	add	v0.2d,v0.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xcec08293	//sha512su0 v19.16b,v20.16b
	ext	v7.16b,v23.16b,v16.16b,#8
.inst	0xce6680a0	//sha512h v0.16b,v5.16b,v6.16b
.inst	0xce678a53	//sha512su1 v19.16b,v18.16b,v7.16b
	add	v2.2d,v4.2d,v0.2d		// "D + T1"
.inst	0xce618480	//sha512h2 v0.16b,v4.16b,v1.16b
	add	v24.2d,v24.2d,v20.2d
	ld1	{v25.2d},[x3],#16
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v2.16b,v3.16b,#8
	ext	v6.16b,v1.16b,v2.16b,#8
	add	v3.2d,v3.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xcec082b4	//sha512su0 v20.16b,v21.16b
	ext	v7.16b,v16.16b,v17.16b,#8
.inst	0xce6680a3	//sha512h v3.16b,v5.16b,v6.16b
.inst	0xce678a74	//sha512su1 v20.16b,v19.16b,v7.16b
	add	v4.2d,v1.2d,v3.2d		// "D + T1"
.inst	0xce608423	//sha512h2 v3.16b,v1.16b,v0.16b
	add	v25.2d,v25.2d,v21.2d
	ld1	{v24.2d},[x3],#16
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v4.16b,v2.16b,#8
	ext	v6.16b,v0.16b,v4.16b,#8
	add	v2.2d,v2.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xcec082d5	//sha512su0 v21.16b,v22.16b
	ext	v7.16b,v17.16b,v18.16b,#8
.inst	0xce6680a2	//sha512h v2.16b,v5.16b,v6.16b
.inst	0xce678a95	//sha512su1 v21.16b,v20.16b,v7.16b
	add	v1.2d,v0.2d,v2.2d		// "D + T1"
.inst	0xce638402	//sha512h2 v2.16b,v0.16b,v3.16b
	add	v24.2d,v24.2d,v22.2d
	ld1	{v25.2d},[x3],#16
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v1.16b,v4.16b,#8
	ext	v6.16b,v3.16b,v1.16b,#8
	add	v4.2d,v4.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xcec082f6	//sha512su0 v22.16b,v23.16b
	ext	v7.16b,v18.16b,v19.16b,#8
.inst	0xce6680a4	//sha512h v4.16b,v5.16b,v6.16b
.inst	0xce678ab6	//sha512su1 v22.16b,v21.16b,v7.16b
	add	v0.2d,v3.2d,v4.2d		// "D + T1"
.inst	0xce628464	//sha512h2 v4.16b,v3.16b,v2.16b
	add	v25.2d,v25.2d,v23.2d
	ld1	{v24.2d},[x3],#16
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v0.16b,v1.16b,#8
	ext	v6.16b,v2.16b,v0.16b,#8
	add	v1.2d,v1.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xcec08217	//sha512su0 v23.16b,v16.16b
	ext	v7.16b,v19.16b,v20.16b,#8
.inst	0xce6680a1	//sha512h v1.16b,v5.16b,v6.16b
.inst	0xce678ad7	//sha512su1 v23.16b,v22.16b,v7.16b
	add	v3.2d,v2.2d,v1.2d		// "D + T1"
.inst	0xce648441	//sha512h2 v1.16b,v2.16b,v4.16b
	add	v24.2d,v24.2d,v16.2d
	ld1	{v25.2d},[x3],#16
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v3.16b,v0.16b,#8
	ext	v6.16b,v4.16b,v3.16b,#8
	add	v0.2d,v0.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xcec08230	//sha512su0 v16.16b,v17.16b
	ext	v7.16b,v20.16b,v21.16b,#8
.inst	0xce6680a0	//sha512h v0.16b,v5.16b,v6.16b
.inst	0xce678af0	//sha512su1 v16.16b,v23.16b,v7.16b
	add	v2.2d,v4.2d,v0.2d		// "D + T1"
.inst	0xce618480	//sha512h2 v0.16b,v4.16b,v1.16b
	add	v25.2d,v25.2d,v17.2d
	ld1	{v24.2d},[x3],#16
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v2.16b,v3.16b,#8
	ext	v6.16b,v1.16b,v2.16b,#8
	add	v3.2d,v3.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xcec08251	//sha512su0 v17.16b,v18.16b
	ext	v7.16b,v21.16b,v22.16b,#8
.inst	0xce6680a3	//sha512h v3.16b,v5.16b,v6.16b
.inst	0xce678a11	//sha512su1 v17.16b,v16.16b,v7.16b
	add	v4.2d,v1.2d,v3.2d		// "D + T1"
.inst	0xce608423	//sha512h2 v3.16b,v1.16b,v0.16b
	add	v24.2d,v24.2d,v18.2d
	ld1	{v25.2d},[x3],#16
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v4.16b,v2.16b,#8
	ext	v6.16b,v0.16b,v4.16b,#8
	add	v2.2d,v2.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xcec08272	//sha512su0 v18.16b,v19.16b
	ext	v7.16b,v22.16b,v23.16b,#8
.inst	0xce6680a2	//sha512h v2.16b,v5.16b,v6.16b
.inst	0xce678a32	//sha512su1 v18.16b,v17.16b,v7.16b
	add	v1.2d,v0.2d,v2.2d		// "D + T1"
.inst	0xce638402	//sha512h2 v2.16b,v0.16b,v3.16b
	add	v25.2d,v25.2d,v19.2d
	ld1	{v24.2d},[x3],#16
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v1.16b,v4.16b,#8
	ext	v6.16b,v3.16b,v1.16b,#8
	add	v4.2d,v4.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xcec08293	//sha512su0 v19.16b,v20.16b
	ext	v7.16b,v23.16b,v16.16b,#8
.inst	0xce6680a4	//sha512h v4.16b,v5.16b,v6.16b
.inst	0xce678a53	//sha512su1 v19.16b,v18.16b,v7.16b
	add	v0.2d,v3.2d,v4.2d		// "D + T1"
.inst	0xce628464	//sha512h2 v4.16b,v3.16b,v2.16b
	add	v24.2d,v24.2d,v20.2d
	ld1	{v25.2d},[x3],#16
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v0.16b,v1.16b,#8
	ext	v6.16b,v2.16b,v0.16b,#8
	add	v1.2d,v1.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xcec082b4	//sha512su0 v20.16b,v21.16b
	ext	v7.16b,v16.16b,v17.16b,#8
.inst	0xce6680a1	//sha512h v1.16b,v5.16b,v6.16b
.inst	0xce678a74	//sha512su1 v20.16b,v19.16b,v7.16b
	add	v3.2d,v2.2d,v1.2d		// "D + T1"
.inst	0xce648441	//sha512h2 v1.16b,v2.16b,v4.16b
	add	v25.2d,v25.2d,v21.2d
	ld1	{v24.2d},[x3],#16
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v3.16b,v0.16b,#8
	ext	v6.16b,v4.16b,v3.16b,#8
	add	v0.2d,v0.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xcec082d5	//sha512su0 v21.16b,v22.16b
	ext	v7.16b,v17.16b,v18.16b,#8
.inst	0xce6680a0	//sha512h v0.16b,v5.16b,v6.16b
.inst	0xce678a95	//sha512su1 v21.16b,v20.16b,v7.16b
	add	v2.2d,v4.2d,v0.2d		// "D + T1"
.inst	0xce618480	//sha512h2 v0.16b,v4.16b,v1.16b
	add	v24.2d,v24.2d,v22.2d
	ld1	{v25.2d},[x3],#16
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v2.16b,v3.16b,#8
	ext	v6.16b,v1.16b,v2.16b,#8
	add	v3.2d,v3.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xcec082f6	//sha512su0 v22.16b,v23.16b
	ext	v7.16b,v18.16b,v19.16b,#8
.inst	0xce6680a3	//sha512h v3.16b,v5.16b,v6.16b
.inst	0xce678ab6	//sha512su1 v22.16b,v21.16b,v7.16b
	add	v4.2d,v1.2d,v3.2d		// "D + T1"
.inst	0xce608423	//sha512h2 v3.16b,v1.16b,v0.16b
	add	v25.2d,v25.2d,v23.2d
	ld1	{v24.2d},[x3],#16
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v4.16b,v2.16b,#8
	ext	v6.16b,v0.16b,v4.16b,#8
	add	v2.2d,v2.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xcec08217	//sha512su0 v23.16b,v16.16b
	ext	v7.16b,v19.16b,v20.16b,#8
.inst	0xce6680a2	//sha512h v2.16b,v5.16b,v6.16b
.inst	0xce678ad7	//sha512su1 v23.16b,v22.16b,v7.16b
	add	v1.2d,v0.2d,v2.2d		// "D + T1"
.inst	0xce638402	//sha512h2 v2.16b,v0.16b,v3.16b
	ld1	{v25.2d},[x3],#16
	add	v24.2d,v24.2d,v16.2d
	ld1	{v16.16b},[x1],#16		// load next input
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v1.16b,v4.16b,#8
	ext	v6.16b,v3.16b,v1.16b,#8
	add	v4.2d,v4.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xce6680a4	//sha512h v4.16b,v5.16b,v6.16b
	rev64	v16.16b,v16.16b
	add	v0.2d,v3.2d,v4.2d		// "D + T1"
.inst	0xce628464	//sha512h2 v4.16b,v3.16b,v2.16b
	ld1	{v24.2d},[x3],#16
	add	v25.2d,v25.2d,v17.2d
	ld1	{v17.16b},[x1],#16		// load next input
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v0.16b,v1.16b,#8
	ext	v6.16b,v2.16b,v0.16b,#8
	add	v1.2d,v1.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xce6680a1	//sha512h v1.16b,v5.16b,v6.16b
	rev64	v17.16b,v17.16b
	add	v3.2d,v2.2d,v1.2d		// "D + T1"
.inst	0xce648441	//sha512h2 v1.16b,v2.16b,v4.16b
	ld1	{v25.2d},[x3],#16
	add	v24.2d,v24.2d,v18.2d
	ld1	{v18.16b},[x1],#16		// load next input
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v3.16b,v0.16b,#8
	ext	v6.16b,v4.16b,v3.16b,#8
	add	v0.2d,v0.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xce6680a0	//sha512h v0.16b,v5.16b,v6.16b
	rev64	v18.16b,v18.16b
	add	v2.2d,v4.2d,v0.2d		// "D + T1"
.inst	0xce618480	//sha512h2 v0.16b,v4.16b,v1.16b
	ld1	{v24.2d},[x3],#16
	add	v25.2d,v25.2d,v19.2d
	ld1	{v19.16b},[x1],#16		// load next input
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v2.16b,v3.16b,#8
	ext	v6.16b,v1.16b,v2.16b,#8
	add	v3.2d,v3.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xce6680a3	//sha512h v3.16b,v5.16b,v6.16b
	rev64	v19.16b,v19.16b
	add	v4.2d,v1.2d,v3.2d		// "D + T1"
.inst	0xce608423	//sha512h2 v3.16b,v1.16b,v0.16b
	ld1	{v25.2d},[x3],#16
	add	v24.2d,v24.2d,v20.2d
	ld1	{v20.16b},[x1],#16		// load next input
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v4.16b,v2.16b,#8
	ext	v6.16b,v0.16b,v4.16b,#8
	add	v2.2d,v2.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xce6680a2	//sha512h v2.16b,v5.16b,v6.16b
	rev64	v20.16b,v20.16b
	add	v1.2d,v0.2d,v2.2d		// "D + T1"
.inst	0xce638402	//sha512h2 v2.16b,v0.16b,v3.16b
	ld1	{v24.2d},[x3],#16
	add	v25.2d,v25.2d,v21.2d
	ld1	{v21.16b},[x1],#16		// load next input
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v1.16b,v4.16b,#8
	ext	v6.16b,v3.16b,v1.16b,#8
	add	v4.2d,v4.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xce6680a4	//sha512h v4.16b,v5.16b,v6.16b
	rev64	v21.16b,v21.16b
	add	v0.2d,v3.2d,v4.2d		// "D + T1"
.inst	0xce628464	//sha512h2 v4.16b,v3.16b,v2.16b
	ld1	{v25.2d},[x3],#16
	add	v24.2d,v24.2d,v22.2d
	ld1	{v22.16b},[x1],#16		// load next input
	ext	v24.16b,v24.16b,v24.16b,#8
	ext	v5.16b,v0.16b,v1.16b,#8
	ext	v6.16b,v2.16b,v0.16b,#8
	add	v1.2d,v1.2d,v24.2d			// "T1 + H + K512[i]"
.inst	0xce6680a1	//sha512h v1.16b,v5.16b,v6.16b
	rev64	v22.16b,v22.16b
	add	v3.2d,v2.2d,v1.2d		// "D + T1"
.inst	0xce648441	//sha512h2 v1.16b,v2.16b,v4.16b
	sub	x3,x3,#80*8	// rewind
	add	v25.2d,v25.2d,v23.2d
	ld1	{v23.16b},[x1],#16		// load next input
	ext	v25.16b,v25.16b,v25.16b,#8
	ext	v5.16b,v3.16b,v0.16b,#8
	ext	v6.16b,v4.16b,v3.16b,#8
	add	v0.2d,v0.2d,v25.2d			// "T1 + H + K512[i]"
.inst	0xce6680a0	//sha512h v0.16b,v5.16b,v6.16b
	rev64	v23.16b,v23.16b
	add	v2.2d,v4.2d,v0.2d		// "D + T1"
.inst	0xce618480	//sha512h2 v0.16b,v4.16b,v1.16b
	add	v0.2d,v0.2d,v26.2d			// accumulate
	add	v1.2d,v1.2d,v27.2d
	add	v2.2d,v2.2d,v28.2d
	add	v3.2d,v3.2d,v29.2d

	cbnz	x2,.Loop_hw

	st1	{v0.2d,v1.2d,v2.2d,v3.2d},[x0]		// store context

	ldr	x29,[sp],#16
	ret
.size	sha512_block_armv8_ce,.-sha512_block_armv8_ce
#endif

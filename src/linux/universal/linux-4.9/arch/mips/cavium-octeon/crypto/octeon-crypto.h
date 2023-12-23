/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2012-2013 Cavium Inc., All Rights Reserved.
 *
 * MD5/SHA1/SHA256/SHA512 instruction definitions added by
 * Aaro Koskinen <aaro.koskinen@iki.fi>.
 *
 */
#ifndef __LINUX_OCTEON_CRYPTO_H
#define __LINUX_OCTEON_CRYPTO_H

#include <linux/sched.h>
#include <asm/mipsregs.h>

#define OCTEON_CR_OPCODE_PRIORITY 300

extern unsigned long octeon_crypto_enable(struct octeon_cop2_state *state);
extern void octeon_crypto_disable(struct octeon_cop2_state *state,
				  unsigned long flags);

#if 0
#define CVMX_MT_AES_ENC_CBC0(val)   asm volatile ("dmtc2 %[rt],0x0108"                   :                 : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_AES_ENC_CBC1(val)   asm volatile ("dmtc2 %[rt],0x3109"                   :                 : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_AES_ENC0(val)       asm volatile ("dmtc2 %[rt],0x010a"                   :                 : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_AES_ENC1(val)       asm volatile ("dmtc2 %[rt],0x310b"                   :                 : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_AES_DEC_CBC0(val)   asm volatile ("dmtc2 %[rt],0x010c"                   :                 : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_AES_DEC_CBC1(val)   asm volatile ("dmtc2 %[rt],0x310d"                   :                 : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_AES_DEC0(val)       asm volatile ("dmtc2 %[rt],0x010e"                   :                 : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_AES_DEC1(val)       asm volatile ("dmtc2 %[rt],0x310f"                   :                 : [rt] "d" (cvmx_cpu_to_be64(val)))
/* pos can be 0-3 */
#define CVMX_MT_AES_KEY(val,pos)    asm volatile ("dmtc2 %[rt],0x0104+" CVMX_TMP_STR(pos) :                 : [rt] "d" (cvmx_cpu_to_be64(val)))
/* pos can be 0-1 */
#define CVMX_MT_AES_IV(val,pos)     asm volatile ("dmtc2 %[rt],0x0102+" CVMX_TMP_STR(pos) :                 : [rt] "d" ((val)))
#define CVMX_MT_AES_KEYLENGTH(val)  asm volatile ("dmtc2 %[rt],0x0110"                   :                 : [rt] "d" ((val)))	/* write the keylen */
/* pos can be 0-1 */
#define CVMX_MT_AES_RESULT(val,pos) asm volatile ("dmtc2 %[rt],0x0100+" CVMX_TMP_STR(pos) :                 : [rt] "d" (cvmx_cpu_to_be64(val)))

/* pos can be 0-1 */
#define CVMX_MF_AES_RESULT(val,pos) asm volatile ("dmfc2 %[rt],0x0100+" CVMX_TMP_STR(pos) : [rt] "=d" ((val)) :               )
/* pos can be 0-1 */
#define CVMX_MF_AES_IV(val,pos)     asm volatile ("dmfc2 %[rt],0x0102+" CVMX_TMP_STR(pos) : [rt] "=d" ((val)) :               )
/* pos can be 0-3 */
#define CVMX_MF_AES_KEY(val,pos)    asm volatile ("dmfc2 %[rt],0x0104+" CVMX_TMP_STR(pos) : [rt] "=d" ((val)) :               )
#define CVMX_MF_AES_KEYLENGTH(val)  asm volatile ("dmfc2 %[rt],0x0110"                   : [rt] "=d" ((val)) :               )	/* read the keylen */
#define CVMX_MF_AES_DAT0(val)       asm volatile ("dmfc2 %[rt],0x0111"                   : [rt] "=d" ((val)) :               )	/* first piece of input data */
#endif

#define read_octeon_64bit_aes_result(index)		\
({							\
	u64 __value;					\
							\
	__asm__ __volatile__ (				\
	"dmfc2 %[rt],0x0100+" STR(index)		\
	: [rt] "=d" (__value)				\
	: );						\
							\
	be64_to_cpu(__value);				\
})



#define write_octeon_64bit_aes_enc0(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x010a"		\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

#define write_octeon_64bit_aes_enc1(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x310b"		\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)


#define write_octeon_64bit_aes_dec0(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x010e"		\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

#define write_octeon_64bit_aes_dec1(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x310f"		\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

#define write_octeon_64bit_aes_key(value, index)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x0104+" STR(index)		\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

#define write_octeon_64bit_aes_iv(value, index)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x0102+" STR(index)		\
	:						\
	: [rt] "d" (value));		\
} while (0)

#define write_octeon_64bit_aes_keylength(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x0110"				\
	:						\
	: [rt] "d" (value));		\
} while (0)


/*
 * Macros needed to implement MD5/SHA1/SHA256:
 */
/*
 * The index can be 0-1 (MD5) or 0-2 (SHA1), 0-3 (SHA256).
 */
#define write_octeon_64bit_hash_dword(value, index)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x0048+" STR(index)		\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

/*
 * The index can be 0-1 (MD5) or 0-2 (SHA1), 0-3 (SHA256).
 */
#define read_octeon_64bit_hash_dword(index)		\
({							\
	u64 __value;					\
							\
	__asm__ __volatile__ (				\
	"dmfc2 %[rt],0x0048+" STR(index)		\
	: [rt] "=d" (__value)				\
	: );						\
							\
	__value;					\
})

/*
 * The index can be 0-6.
 */
#define write_octeon_64bit_block_dword(value, index)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x0040+" STR(index)		\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

/*
 * The value is the final block dword (64-bit).
 */
#define octeon_md5_start(value)				\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x4047"				\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

/*
 * The value is the final block dword (64-bit).
 */
#define octeon_sha1_start(value)			\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x4057"				\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

/*
 * The value is the final block dword (64-bit).
 */
#define octeon_sha256_start(value)			\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x404f"				\
	:						\
	: [rt] "d" (cpu_to_be64(value)));				\
} while (0)

/*
 * Macros needed to implement SHA512:
 */

/*
 * The index can be 0-7.
 */
#define write_octeon_64bit_hash_sha512(value, index)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x0250+" STR(index)		\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

/*
 * The index can be 0-7.
 */
#define read_octeon_64bit_hash_sha512(index)		\
({							\
	u64 __value;					\
							\
	__asm__ __volatile__ (				\
	"dmfc2 %[rt],0x0250+" STR(index)		\
	: [rt] "=d" (__value)				\
	: );						\
							\
	__value;				\
})

/*
 * The index can be 0-14.
 */
#define write_octeon_64bit_block_sha512(value, index)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x0240+" STR(index)		\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

/*
 * The value is the final block word (64-bit).
 */
#define octeon_sha512_start(value)			\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x424f"				\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

/*
 * The value is the final block dword (64-bit).
 */
#define octeon_sha1_start(value)			\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x4057"				\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

/*
 * The value is the final block dword (64-bit).
 */
#define octeon_sha256_start(value)			\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x404f"				\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

/*
 * Macros needed to implement SHA512:
 */

/*
 * The index can be 0-7.
 */
#define write_octeon_64bit_hash_sha512(value, index)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x0250+" STR(index)		\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

/*
 * The index can be 0-7.
 */
#define read_octeon_64bit_hash_sha512(index)		\
({							\
	u64 __value;					\
							\
	__asm__ __volatile__ (				\
	"dmfc2 %[rt],0x0250+" STR(index)		\
	: [rt] "=d" (__value)				\
	: );						\
							\
	__value;				\
})

/*
 * The index can be 0-14.
 */
#define write_octeon_64bit_block_sha512(value, index)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x0240+" STR(index)		\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

/*
 * The value is the final block word (64-bit).
 */
#define octeon_sha512_start(value)			\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x424f"				\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

#endif /* __LINUX_OCTEON_CRYPTO_H */

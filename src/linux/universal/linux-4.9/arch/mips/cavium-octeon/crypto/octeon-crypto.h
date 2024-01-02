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

#define octeon_prefetch_prefx(X, address, offset) asm volatile ("pref %[type], %[off](%[rbase])" : : [rbase] "d" (address), [off] "I" (offset), [type] "n" (X))
#define octeon_prefetch_pref0(address, offset) octeon_prefetch_prefx(0, address, offset)
#define octeon_prefetch_pref1(address, offset) octeon_prefetch_prefx(1, address, offset)
#define octeon_prefetch_pref6(address, offset) octeon_prefetch_prefx(6, address, offset)
#define octeon_prefetch_pref7(address, offset) octeon_prefetch_prefx(7, address, offset)

#define octeon_prefetch(address, offset) octeon_prefetch_pref0(address, offset)
#define CVMX_SYNCI(address, offset) asm volatile ("synci " CVMX_TMP_STR(offset) "(%[rbase])" : : [rbase] "d" (address) )
#define octeon_prefetch0(address) octeon_prefetch(address, 0)
#define octeon_prefetch128(address) octeon_prefetch(address, 128)




#define octeon_storeuna_int64(data, address, offset) \
	{ char *__a = (char *)(address); \
	  asm ("usd %[rsrc], " STR(offset) "(%[rbase])" : \
	       "=m"(__a[offset + 0]), "=m"(__a[offset + 1]), "=m"(__a[offset + 2]), "=m"(__a[offset + 3]), \
	       "=m"(__a[offset + 4]), "=m"(__a[offset + 5]), "=m"(__a[offset + 6]), "=m"(__a[offset + 7]) : \
	       [rsrc] "d" (data), [rbase] "d" (__a)); }


#define octeon_loaduna_int64(result, address, offset) \
	{ char *__a = (char *)(address); \
	  asm ("uld %[rdest], " STR(offset) "(%[rbase])" : [rdest] "=d" (result) : \
	       [rbase] "d" (__a), "m"(__a[offset + 0]), "m"(__a[offset + 1]), "m"(__a[offset + 2]), "m"(__a[offset + 3]), \
	       "m"(__a[offset + 4]), "m"(__a[offset + 5]), "m"(__a[offset + 6]), "m"(__a[offset + 7])); }


#define write_octeon_64bit_aes_result_wr_data(in1, in2, out1, out2)	\
do {							\
	__asm__ __volatile__ (		\
            ".set noreorder    \n" \
            "dmfc2 %[r1],0x0100\n" \
            "dmfc2 %[r2],0x0101\n" \
            "dmtc2 %[r3],0x010a\n" \
            "dmtc2 %[r4],0x310b\n" \
            ".set reorder      \n" \
            : [r1] "=&d"(in1) , [r2] "=&d"(in2) \
            : [r3] "d"(out1),  [r4] "d"(out2)); \
} while (0)

#define CVMX_MT_CRC_POLYNOMIAL(val)         asm volatile ("dmtc2 %[rt],0x4200" : : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_CRC_IV(val)                 asm volatile ("dmtc2 %[rt],0x0201" : : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_CRC_LEN(val)                asm volatile ("dmtc2 %[rt],0x1202" : : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_CRC_BYTE(val)               asm volatile ("dmtc2 %[rt],0x0204" : : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_CRC_HALF(val)               asm volatile ("dmtc2 %[rt],0x0205" : : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_CRC_WORD(val)               asm volatile ("dmtc2 %[rt],0x0206" : : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_CRC_DWORD(val)              asm volatile ("dmtc2 %[rt],0x1207" : : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_CRC_VAR(val)                asm volatile ("dmtc2 %[rt],0x1208" : : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_CRC_POLYNOMIAL_REFLECT(val) asm volatile ("dmtc2 %[rt],0x4210" : : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_CRC_IV_REFLECT(val)         asm volatile ("dmtc2 %[rt],0x0211" : : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_CRC_BYTE_REFLECT(val)       asm volatile ("dmtc2 %[rt],0x0214" : : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_CRC_HALF_REFLECT(val)       asm volatile ("dmtc2 %[rt],0x0215" : : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_CRC_WORD_REFLECT(val)       asm volatile ("dmtc2 %[rt],0x0216" : : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_CRC_DWORD_REFLECT(val)      asm volatile ("dmtc2 %[rt],0x1217" : : [rt] "d" (cvmx_cpu_to_be64(val)))
#define CVMX_MT_CRC_VAR_REFLECT(val)        asm volatile ("dmtc2 %[rt],0x1218" : : [rt] "d" (cvmx_cpu_to_be64(val)))

#define CVMX_MF_CRC_POLYNOMIAL(val)         asm volatile ("dmfc2 %[rt],0x0200" : [rt] "=d" (cvmx_be64_to_cpu(val)) : )
#define CVMX_MF_CRC_IV(val)                 asm volatile ("dmfc2 %[rt],0x0201" : [rt] "=d" (cvmx_be64_to_cpu(val)) : )
#define CVMX_MF_CRC_IV_REFLECT(val)         asm volatile ("dmfc2 %[rt],0x0203" : [rt] "=d" (cvmx_be64_to_cpu(val)) : )
#define CVMX_MF_CRC_LEN(val)                asm volatile ("dmfc2 %[rt],0x0202" : [rt] "=d" (cvmx_be64_to_cpu(val)) : )

#define write_octeon_64bit_crc_polynominal(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x4200"				\
	:						\
	: [rt] "d" (value));		\
} while (0)

#define write_octeon_64bit_crc_iv(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x0201"				\
	:						\
	: [rt] "d" (value));		\
} while (0)

#define write_octeon_64bit_crc_len(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x1202"				\
	:						\
	: [rt] "d" (value));		\
} while (0)

#define write_octeon_64bit_crc_byte(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x0204"				\
	:						\
	: [rt] "d" (value));		\
} while (0)

#define write_octeon_64bit_crc_half(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x0205"				\
	:						\
	: [rt] "d" (value));		\
} while (0)

#define write_octeon_64bit_crc_word(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x0206"				\
	:						\
	: [rt] "d" (value));		\
} while (0)

#define write_octeon_64bit_crc_dword(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x1207"				\
	:						\
	: [rt] "d" (value));		\
} while (0)

#define write_octeon_64bit_crc_var(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x1208"				\
	:						\
	: [rt] "d" (value));		\
} while (0)

#define read_octeon_64bit_crc_iv()		\
({							\
	u64 __value;					\
							\
	__asm__ __volatile__ (				\
	"dmfc2 %[rt],0x0201"		\
	: [rt] "=d" (__value)				\
	: );						\
							\
	__value;					\
})


#define write_octeon_64bit_gfm_xormul1_reflect(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x405d"				\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

#define write_octeon_64bit_gfm_xor0_reflect(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x005c"				\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)


#define write_octeon_64bit_gfm_mul_reflect(value, index)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x0058+" STR(index)		\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)


#define write_octeon_64bit_gfm_xormul1(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x425d"				\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)


#define write_octeon_64bit_gfm_xor0(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x025c"				\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)


#define write_octeon_64bit_gfm_resinp(value, index)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x025a+" STR(index)		\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

#define read_octeon_64bit_gfm_resinp(index)		\
({							\
	u64 __value;					\
							\
	__asm__ __volatile__ (				\
	"dmfc2 %[rt],0x025a+" STR(index)		\
	: [rt] "=d" (__value)				\
	: );						\
							\
	be64_to_cpu(__value);				\
})


#define write_octeon_64bit_gfm_mul(value, index)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x0258+" STR(index)		\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

#define write_octeon_64bit_gfm_poly(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x025e"				\
	:						\
	: [rt] "d" (value));		\
} while (0)


#define write_octeon_64bit_aes_iv(value, index)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x0102+" STR(index)		\
	:						\
	: [rt] "d" (cpu_to_be64(value)));				\
} while (0)


#define write_octeon_64bit_aes_enc_cbc0(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x0108"		\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

#define write_octeon_64bit_aes_enc_cbc1(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x3109"		\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

#define write_octeon_64bit_aes_dec_cbc0(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x010c"		\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)

#define write_octeon_64bit_aes_dec_cbc1(value)	\
do {							\
	__asm__ __volatile__ (				\
	"dmtc2 %[rt],0x310d"		\
	:						\
	: [rt] "d" (cpu_to_be64(value)));		\
} while (0)



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

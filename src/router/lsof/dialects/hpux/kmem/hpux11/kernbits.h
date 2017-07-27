/*
 * kernbits.h for HP-UX 10.30 and above
 *
 * This header file defines the basic kernel word size type for lsof, using
 * the Configure-generated -DHPUXKERNBITS=<32|64> definition.
 *
 * V. Abell
 * February, 1998
 */

#if	!defined(LSOF_KERNBITS_H)
#define	LSOF_KERNBITS_H

# if	!defined(HPUXKERNBITS) || HPUXKERNBITS<64
typedef	uint32_t	KA_T;
#define	KA_T_FMT_X	"%#lx"
# else	/* defined(HPUXKERNBITS) && HPUXKERNBITS>=64 */
typedef	uint64_t	KA_T;
#define	KA_T_FMT_X	"%#llx"
# endif	/* !defined(HPUXKERNBITS) || HPUXKERNBITS<64 */

#endif	/* !defined(LSOF_KERNBITS_H) */

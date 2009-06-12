/* bootstd.h:  Bootloader system call interface
 *
 * (c) 1999, Rt-Control, Inc.
 *
 *  ARM7TDMI support by O.Zhadan,  2001
 *  ( used undefined instruction trap )
 */

#ifndef __BOOTSTD_H__
#define __BOOTSTD_H__

#define NR_BSC 22            	/* last used bootloader system call */

#define __BN_reset        	0  /* reset and start the bootloader */
#define __BN_test         	1  /* tests the system call interface */
#define __BN_exec         	2  /* executes a bootloader image */
#define __BN_exit         	3  /* terminates a bootloader image */
#define __BN_program      	4  /* program FLASH from a chain */
#define __BN_erase        	5  /* erase sector(s) of FLASH */
#define __BN_open         	6
#define __BN_write        	7
#define __BN_read         	8
#define __BN_close        	9
#define __BN_mmap         	10 /* map a file descriptor into memory */
#define __BN_munmap       	11 /* remove a file to memory mapping */
#define __BN_gethwaddr    	12 /* get the hardware address of my interfaces */
#define __BN_getserialnum 	13 /* get the serial number of this board */
#define __BN_getbenv      	14 /* get a bootloader envvar */
#define __BN_setbenv      	15 /* get a bootloader envvar */
#define __BN_setpmask     	16 /* set the protection mask */
#define __BN_readbenv      	17 /* read environment variables */
#define __BN_flash_chattr_range	18
#define __BN_flash_erase_range	19
#define __BN_flash_write_range	20
#define __BN_ramload            21 /* load kernel into RAM and exec */

/* Calling conventions compatible to (uC)linux
 * We use simmilar macros to call into the bootloader as for uClinux
 */

#define __bsc_return(type, res) \
do { \
   if ((unsigned int)(res) >= (unsigned int)(-64)) { \
      /* let errno be a function, preserve res in %r0 */ \
      int __err = -(res); \
      errno = __err; \
      res = -1; \
   } \
   return (type)(res); \
} while (0)

#define _bsc0(type,name) \
type name(void) \
{ \
   register unsigned int __res __asm__ ("%sl") = __BN_##name; \
   __asm__ __volatile__ (".word 0xe65a4f10; mov sl, r0" \
                         : "=g" (__res) \
                         : "0"  (__res) \
                         : "%r0"); \
   __bsc_return(type,__res); \
}

#define _bsc1(type,name,atype,a) \
type name(atype a) \
{ \
   register unsigned int __res __asm__ ("%sl") = __BN_##name; \
   __asm__ __volatile__ (".word 0xe65a4f10; mov sl, r0" \
                         : "=g" (__res) \
                         : "0"  (__res) \
                         : "%r0"); \
   __bsc_return(type,__res); \
}

#define _bsc2(type,name,atype,a,btype,b) \
type name(atype a, btype b) \
{ \
   register unsigned int __res __asm__ ("%sl") = __BN_##name; \
   __asm__ __volatile__ (".word 0xe65a4f10; mov sl, r0" \
                         : "=g" (__res) \
                         : "0"  (__res) \
                         : "%r0"); \
   __bsc_return(type,__res); \
}

#define _bsc3(type,name,atype,a,btype,b,ctype,c) \
type name(atype a, btype b, ctype c) \
{ \
   register unsigned int __res __asm__ ("%sl") = __BN_##name; \
   __asm__ __volatile__ (".word 0xe65a4f10; mov sl, r0" \
                         : "=g" (__res) \
                         : "0"  (__res) \
                         : "%r0"); \
   __bsc_return(type,__res); \
}

#define _bsc4(type,name,atype,a,btype,b,ctype,c,dtype,d) \
type name(atype a, btype b, ctype c, dtype d) \
{ \
   register unsigned int __res __asm__ ("%sl") = __BN_##name; \
   __asm__ __volatile__ (".word 0xe65a4f10; mov sl, r0" \
                         : "=g" (__res) \
                         : "0"  (__res) \
                         : "%r0"); \
   __bsc_return(type,__res); \
}

#define _bsc5(type,name,atype,a,btype,b,ctype,c,dtype,d,etype,e) \
type name(atype a, btype b, ctype c, dtype d, etype e) \
{ \
   register unsigned int __e   __asm__ ("%r4") = e; \
   register unsigned int __res __asm__ ("%sl") = __BN_##name; \
   __asm__ __volatile__ (".word 0xe65A4F10; mov sl, r0" \
                         : "=g" (__res) \
                         : "0"  (__res) \
                         : "%r0"); \
   __bsc_return(type,__res); \
}

#endif /* __BOOTSTD_H__ */

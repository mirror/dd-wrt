/* Changes made by Tony Kou    Lineo  Inc.  May 2001	*/

#ifndef __ASMFRIO_ELF_H
#define __ASMFRIO_ELF_H

/*
 * ELF register definitions..
 */

#include <linux/config.h>
#include <asm/ptrace.h>
#include <asm/user.h>

typedef unsigned long elf_greg_t;

#define ELF_NGREG (sizeof(struct user_regs_struct) / sizeof(elf_greg_t))
typedef elf_greg_t elf_gregset_t[ELF_NGREG];


/*
 * This is used to ensure we don't load something for the wrong architecture.
 */
#define elf_check_arch(x) ((x)->e_machine == EM_FRIO)

/* EM_FRIO defined in linux/elf.h	*/

/*
 * These are used to set parameters in the core dumps.
 */
#define ELF_CLASS	ELFCLASS32
#define ELF_DATA	ELFDATA2LSB
#define ELF_ARCH	EM_FRIO

/* For SVR4/m68k the function pointer to be registered with `atexit' is
   passed in %a1.  Although my copy of the ABI has no such statement, it
   is actually used on ASV.  */
#define ELF_PLAT_INIT(_r)	_r->p1 = 0	/*   ???    tonyko */

#define USE_ELF_CORE_DUMP
#define ELF_EXEC_PAGESIZE	4096

/* This is the location that an ET_DYN program is loaded if exec'ed.  Typical
   use of this is to invoke "./ld.so someprog" to test out a new version of
   the loader.  We need to make sure that it is out of the way of the program
   that it will "exec", and that there is sufficient room for the brk.  */

#define ELF_ET_DYN_BASE         0xD0000000UL

#define ELF_CORE_COPY_REGS(pr_reg, regs)	\
        memcpy((char *) &pr_reg, (char *)regs,  \
               sizeof(struct pt_regs));

/* This yields a mask that user programs can use to figure out what
   instruction set this cpu supports.  */

#define ELF_HWCAP	(0)

/* This yields a string that ld.so will use to load implementation
   specific libraries for optimization.  This is more specific in
   intent than poking at uname or /proc/cpuinfo.  */

#define ELF_PLATFORM  (NULL)

#ifdef __KERNEL__
#define SET_PERSONALITY(ex, ibcs2) set_personality((ibcs2)?PER_SVR4:PER_LINUX)
#endif

#endif

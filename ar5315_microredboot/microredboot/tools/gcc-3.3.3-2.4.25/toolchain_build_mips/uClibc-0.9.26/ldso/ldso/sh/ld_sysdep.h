/*
 * Various assmbly language/system dependent  hacks that are required
 * so that we can minimize the amount of platform specific code.
 */

/* 
 * Define this if the system uses RELOCA.
 */
#define ELF_USES_RELOCA

/*
 * Get a pointer to the argv array.  On many platforms this can be just
 * the address if the first argument, on other platforms we need to
 * do something a little more subtle here.
 */
#define GET_ARGV(ARGVP, ARGS) ARGVP = ((unsigned long*)   ARGS)

/*
 * Initialization sequence for a GOT.
 */
#define INIT_GOT(GOT_BASE,MODULE) \
{				\
  GOT_BASE[2] = (unsigned long) _dl_linux_resolve; \
  GOT_BASE[1] = (unsigned long) (MODULE); \
}

/*
 * Here is a macro to perform a relocation.  This is only used when
 * bootstrapping the dynamic loader.  RELP is the relocation that we
 * are performing, REL is the pointer to the address we are relocating.
 * SYMBOL is the symbol involved in the relocation, and LOAD is the
 * load address.
 */
#define PERFORM_BOOTSTRAP_RELOC(RELP,REL,SYMBOL,LOAD,SYMTAB)	\
	switch(ELF32_R_TYPE((RELP)->r_info)){			\
	case R_SH_REL32:					\
		*(REL)  = (SYMBOL) + (RELP)->r_addend		\
			    - (unsigned long)(REL);		\
		break;						\
	case R_SH_DIR32:					\
	case R_SH_GLOB_DAT:					\
	case R_SH_JMP_SLOT:					\
		*(REL)  = (SYMBOL) + (RELP)->r_addend;		\
		break;						\
	case R_SH_RELATIVE:					\
		*(REL)  = (LOAD) + (RELP)->r_addend;		\
		break;						\
	case R_SH_NONE:						\
		break;						\
	default:						\
		SEND_STDERR("BOOTSTRAP_RELOC: unhandled reloc type "); \
		SEND_NUMBER_STDERR(ELF32_R_TYPE((RELP)->r_info), 1); \
		SEND_STDERR("REL, SYMBOL, LOAD: ");		\
		SEND_ADDRESS_STDERR(REL, 0);			\
		SEND_STDERR(", ");				\
		SEND_ADDRESS_STDERR(SYMBOL, 0);			\
		SEND_STDERR(", ");				\
		SEND_ADDRESS_STDERR(LOAD, 1);			\
		_dl_exit(1);					\
	}


/*
 * Transfer control to the user's application, once the dynamic loader
 * is done.  This routine has to exit the current function, then 
 * call the _dl_elf_main function.
 */

#define START()   return _dl_elf_main;



/* Here we define the magic numbers that this dynamic loader should accept */

#define MAGIC1 EM_SH
#undef  MAGIC2
/* Used for error messages */
#define ELF_TARGET "sh"

struct elf_resolve;
extern unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);

static __inline__ unsigned int
_dl_urem(unsigned int n, unsigned int base)
{
  int res;
  
	__asm__ (""\
		"mov	#0, r0\n\t" \
		"div0u\n\t" \
		"" \
		"! get one bit from the msb of the numerator into the T\n\t" \
		"! bit and divide it by whats in %2.  Put the answer bit\n\t" \
		"! into the T bit so it can come out again at the bottom\n\t" \
		""				\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		""				\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		""				\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
 		""				\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1 ; div1 %2, r0\n\t"	\
		"rotcl	%1\n\t"
		: "=r" (res)
		: "0" (n), "r" (base)
		: "r0","cc");

	return n - (base * res);
}

#define do_rem(result, n, base)     ((result) = _dl_urem((n), (base)))

/* 4096 bytes alignment */
#define PAGE_ALIGN 0xfffff000
#define ADDR_ALIGN 0xfff
#define OFFS_ALIGN 0x7ffff000

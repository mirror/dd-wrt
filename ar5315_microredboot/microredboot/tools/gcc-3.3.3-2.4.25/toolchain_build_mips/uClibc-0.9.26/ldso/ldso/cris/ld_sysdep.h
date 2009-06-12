/* CRIS can never use Elf32_Rel relocations. */
#define ELF_USES_RELOCA

/*
 * Get a pointer to the argv array.  On many platforms this can be just
 * the address if the first argument, on other platforms we need to
 * do something a little more subtle here.
 */
#define GET_ARGV(ARGVP, ARGS) ARGVP = ((unsigned long *) ARGS)

/*
 * Initialization sequence for a GOT.
 */
#define INIT_GOT(GOT_BASE,MODULE)				\
{								\
	GOT_BASE[1] = (unsigned long) MODULE; 			\
	GOT_BASE[2] = (unsigned long) _dl_linux_resolve; 	\
}

/*
 * Here is a macro to perform a relocation.  This is only used when
 * bootstrapping the dynamic loader.  RELP is the relocation that we
 * are performing, REL is the pointer to the address we are relocating.
 * SYMBOL is the symbol involved in the relocation, and LOAD is the
 * load address.
 */
#define PERFORM_BOOTSTRAP_RELOC(RELP, REL, SYMBOL, LOAD, SYMTAB)	\
	switch (ELF32_R_TYPE((RELP)->r_info)) {				\
		case R_CRIS_GLOB_DAT:					\
		case R_CRIS_JUMP_SLOT:					\
		case R_CRIS_32:						\
			*REL = SYMBOL;					\
			break;						\
		case R_CRIS_16_PCREL:					\
			*(short *) *REL = SYMBOL + (RELP)->r_addend - *REL - 2;	\
			break;						\
		case R_CRIS_32_PCREL:					\
			*REL = SYMBOL + (RELP)->r_addend - *REL - 4;	\
			break;						\
		case R_CRIS_NONE:					\
			break;						\
		case R_CRIS_RELATIVE:					\
			*REL = (unsigned long) LOAD + (RELP)->r_addend;	\
			break;						\
		default:						\
			_dl_exit(1);					\
			break;						\
	}

/*
 * Transfer control to the user's application once the dynamic loader
 * is done. This routine has to exit the current function, then call
 * _dl_elf_main.
 */
#define START() __asm__ volatile ("moveq 0,$r8\n\t" \
				  "move $r8,$srp\n\t" \
				  "move.d %1,$sp\n\t" \
				  "jump %0\n\t" \
				  : : "r" (_dl_elf_main), "r" (args))

/* Defined some magic numbers that this ld.so should accept. */
#define MAGIC1 EM_CRIS
#undef MAGIC2
#define ELF_TARGET "CRIS"

struct elf_resolve;
extern unsigned long _dl_linux_resolver(struct elf_resolve *tpnt, int reloc_entry);

/* Cheap modulo implementation, taken from arm/ld_sysdep.h. */
static inline unsigned long
cris_mod(unsigned long m, unsigned long p)
{
	unsigned long i, t, inc;

	i = p;
	t = 0;

	while (!(i & (1 << 31))) {
		i <<= 1;
		t++;
	}

	t--;

	for (inc = t; inc > 2; inc--) {
		i = p << inc;

		if (i & (1 << 31))
			break;

		while (m >= i) {
			m -= i;
			i <<= 1;
			if (i & (1 << 31))
				break;
			if (i < p)
				break;
		}
	}

	while (m >= p)
		m -= p;

	return m;
}

#define do_rem(result, n, base) result = cris_mod(n, base);

/* 8192 bytes alignment */
#define PAGE_ALIGN 0xffffe000
#define ADDR_ALIGN 0x1fff
#define OFFS_ALIGN 0xffffe000

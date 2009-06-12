/*
 * Various assmbly language/system dependent  hacks that are required
 * so that we can minimize the amount of platform specific code.
 */

/*
 * Define this if the system uses RELOCA.
 */
#undef ELF_USES_RELOCA

/*
 * Get a pointer to the argv array.  On many platforms this can be just
 * the address if the first argument, on other platforms we need to
 * do something a little more subtle here.
 */
#define GET_ARGV(ARGVP, ARGS) ARGVP = ((unsigned long*) & ARGS)

/*
 * Initialization sequence for a GOT.
 */
#define INIT_GOT(GOT_BASE,MODULE)				\
do {								\
  GOT_BASE[2] = (unsigned long) _dl_linux_resolve;		\
  GOT_BASE[1] = (unsigned long) MODULE;				\
} while(0)

/*
 * Here is a macro to perform a relocation.  This is only used when
 * bootstrapping the dynamic loader.  RELP is the relocation that we
 * are performing, REL is the pointer to the address we are relocating.
 * SYMBOL is the symbol involved in the relocation, and LOAD is the
 * load address.
 */
#define PERFORM_BOOTSTRAP_RELOC(RELP,REL,SYMBOL,LOAD,SYMTAB)	\
	switch(ELF32_R_TYPE((RELP)->r_info)){			\
	case R_386_32:						\
	  *REL += SYMBOL;					\
	  break;						\
	case R_386_PC32:					\
	  *REL += SYMBOL - (unsigned long) REL;			\
	  break;						\
	case R_386_GLOB_DAT:					\
	case R_386_JMP_SLOT:					\
	  *REL = SYMBOL;					\
	  break;						\
	case R_386_RELATIVE:					\
	  *REL += (unsigned long) LOAD;				\
	  break;						\
	default:						\
	  _dl_exit(1);						\
	}


/*
 * Transfer control to the user's application, once the dynamic loader
 * is done.  This routine has to exit the current function, then 
 * call the _dl_elf_main function.
 */
#define START()							\
	__asm__ volatile ("leave\n\t"				\
		    "jmp *%%eax\n\t"				\
		    : "=a" (status) :	"a" (_dl_elf_main))



/* Here we define the magic numbers that this dynamic loader should accept */

#define MAGIC1 EM_386
#undef  MAGIC2
/* Used for error messages */
#define ELF_TARGET "386"

struct elf_resolve;
extern unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);

#define do_rem(result, n, base)  result = (n % base)

/* 4096 bytes alignment */
#define PAGE_ALIGN 0xfffff000
#define ADDR_ALIGN 0xfff
#define OFFS_ALIGN 0x7ffff000

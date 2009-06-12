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
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long*) ARGS)+1)

/*
 * Initialization sequence for a GOT.
 */
#define INIT_GOT(GOT_BASE,MODULE)  _dl_init_got(GOT_BASE,MODULE)

/* Stuff for the PLT.  */
#define PLT_INITIAL_ENTRY_WORDS 18
#define PLT_LONGBRANCH_ENTRY_WORDS 0
#define PLT_TRAMPOLINE_ENTRY_WORDS 6
#define PLT_DOUBLE_SIZE (1<<13)
#define PLT_ENTRY_START_WORDS(entry_number) \
  (PLT_INITIAL_ENTRY_WORDS + (entry_number)*2				\
   + ((entry_number) > PLT_DOUBLE_SIZE					\
      ? ((entry_number) - PLT_DOUBLE_SIZE)*2				\
      : 0))
#define PLT_DATA_START_WORDS(num_entries) PLT_ENTRY_START_WORDS(num_entries)

/* Macros to build PowerPC opcode words.  */
#define OPCODE_ADDI(rd,ra,simm) \
  (0x38000000 | (rd) << 21 | (ra) << 16 | ((simm) & 0xffff))
#define OPCODE_ADDIS(rd,ra,simm) \
  (0x3c000000 | (rd) << 21 | (ra) << 16 | ((simm) & 0xffff))
#define OPCODE_ADD(rd,ra,rb) \
  (0x7c000214 | (rd) << 21 | (ra) << 16 | (rb) << 11)
#define OPCODE_B(target) (0x48000000 | ((target) & 0x03fffffc))
#define OPCODE_BA(target) (0x48000002 | ((target) & 0x03fffffc))
#define OPCODE_BCTR() 0x4e800420
#define OPCODE_LWZ(rd,d,ra) \
  (0x80000000 | (rd) << 21 | (ra) << 16 | ((d) & 0xffff))
#define OPCODE_LWZU(rd,d,ra) \
  (0x84000000 | (rd) << 21 | (ra) << 16 | ((d) & 0xffff))
#define OPCODE_MTCTR(rd) (0x7C0903A6 | (rd) << 21)
#define OPCODE_RLWINM(ra,rs,sh,mb,me) \
  (0x54000000 | (rs) << 21 | (ra) << 16 | (sh) << 11 | (mb) << 6 | (me) << 1)

#define OPCODE_LI(rd,simm)    OPCODE_ADDI(rd,0,simm)
#define OPCODE_ADDIS_HI(rd,ra,value) \
  OPCODE_ADDIS(rd,ra,((value) + 0x8000) >> 16)
#define OPCODE_LIS_HI(rd,value) OPCODE_ADDIS_HI(rd,0,value)
#define OPCODE_SLWI(ra,rs,sh) OPCODE_RLWINM(ra,rs,sh,0,31-sh)


#define PPC_DCBST(where) asm volatile ("dcbst 0,%0" : : "r"(where) : "memory")
#define PPC_SYNC asm volatile ("sync" : : : "memory")
#define PPC_ISYNC asm volatile ("sync; isync" : : : "memory")
#define PPC_ICBI(where) asm volatile ("icbi 0,%0" : : "r"(where) : "memory")
#define PPC_DIE asm volatile ("tweq 0,0")

/*
 * Here is a macro to perform a relocation.  This is only used when
 * bootstrapping the dynamic loader.  RELP is the relocation that we
 * are performing, REL is the pointer to the address we are relocating.
 * SYMBOL is the symbol involved in the relocation, and LOAD is the
 * load address.
 */
// finaladdr = LOAD ?
#define PERFORM_BOOTSTRAP_RELOC(RELP,REL,SYMBOL,LOAD,SYMTAB) \
	{int type=ELF32_R_TYPE((RELP)->r_info);		\
	if(type==R_PPC_NONE){				\
	}else if(type==R_PPC_ADDR32){			\
		*REL += (SYMBOL);			\
	}else if(type==R_PPC_RELATIVE){			\
		*REL = (Elf32_Word)(LOAD) + (RELP)->r_addend;		\
	}else if(type==R_PPC_REL24){			\
		Elf32_Sword delta = (Elf32_Word)(SYMBOL) - (Elf32_Word)(REL);	\
		*REL &= 0xfc000003;			\
		*REL |= (delta & 0x03fffffc);		\
	}else if(type==R_PPC_JMP_SLOT){			\
		Elf32_Sword delta = (Elf32_Word)(SYMBOL) - (Elf32_Word)(REL);	\
		/*if (delta << 6 >> 6 != delta)_dl_exit(99);*/	\
		*REL = OPCODE_B(delta);			\
	}else{						\
	  _dl_exit(100+ELF32_R_TYPE((RELP)->r_info));	\
	}						\
	if(type!=R_PPC_NONE){				\
		PPC_DCBST(REL); PPC_SYNC; PPC_ICBI(REL);\
	}						\
	}

/*
 * Transfer control to the user's application, once the dynamic loader
 * is done.  This routine has to exit the current function, then 
 * call the _dl_elf_main function.
 */

/* hgb@ifi.uio.no:
 * Adding a clobber list consisting of r0 for %1.  addi on PowerPC
 * takes a register as the second argument, but if the register is
 * r0, the value 0 is used instead.  If r0 is used here, the stack
 * pointer (r1) will be zeroed, and the dynamically linked
 * application will seg.fault immediatly when receiving control.
 */
#define START()		\
	__asm__ volatile ( \
		    "addi 1,%1,0\n\t" \
		    "mtlr %0\n\t" \
		    "blrl\n\t"	\
		    : : "r" (_dl_elf_main), "r" (args) \
		    : "r0")


/* Here we define the magic numbers that this dynamic loader should accept */

#define MAGIC1 EM_PPC
#undef  MAGIC2
/* Used for error messages */
#define ELF_TARGET "powerpc"

struct elf_resolve;
extern unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);
void _dl_init_got(unsigned long *lpnt,struct elf_resolve *tpnt);


#define do_rem(result, n, base)  result = (n % base)

/* 4096 bytes alignment */
#define PAGE_ALIGN 0xfffff000
#define ADDR_ALIGN 0xfff
#define OFFS_ALIGN 0x7ffff000

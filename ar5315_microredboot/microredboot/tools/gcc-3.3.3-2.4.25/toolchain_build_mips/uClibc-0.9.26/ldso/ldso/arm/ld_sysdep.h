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
#define GET_ARGV(ARGVP, ARGS) ARGVP = ((unsigned long*)   ARGS)

/*
 * Initialization sequence for a GOT.
 */
#define INIT_GOT(GOT_BASE,MODULE) \
{				\
  GOT_BASE[2] = (unsigned long) _dl_linux_resolve; \
  GOT_BASE[1] = (unsigned long) MODULE; \
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
	case R_ARM_ABS32:					\
	  *REL += SYMBOL;					\
	  break;						\
        case R_ARM_PC24:					\
	    { long newvalue, topbits;				\
	    unsigned long addend = *REL & 0x00ffffff;		\
	    if (addend & 0x00800000) addend |= 0xff000000;	\
	    newvalue=SYMBOL-(unsigned long)REL+(addend<<2);	\
	    topbits = newvalue & 0xfe000000;			\
	    if (topbits!=0xfe000000&&topbits!=0x00000000){	\
	    newvalue = fix_bad_pc24(REL, SYMBOL)		\
		-(unsigned long)REL+(addend<<2);		\
	    topbits = newvalue & 0xfe000000;			\
	    if (topbits!=0xfe000000&&topbits!=0x00000000){	\
	    SEND_STDERR("R_ARM_PC24 relocation out of range\n");\
	    _dl_exit(1); } }					\
	    newvalue>>=2;					\
	    SYMBOL=(*REL&0xff000000)|(newvalue & 0x00ffffff);	\
	    *REL=SYMBOL;					\
	    }							\
	  break;						\
	case R_ARM_GLOB_DAT:					\
	case R_ARM_JUMP_SLOT:					\
	  *REL = SYMBOL;					\
	  break;						\
        case R_ARM_RELATIVE:					\
	  *REL += (unsigned long) LOAD;				\
	  break;						\
        case R_ARM_NONE:					\
	  break;						\
	default:						\
	  SEND_STDERR("Aiieeee!");				\
	  _dl_exit(1);						\
	}


/*
 * Transfer control to the user's application, once the dynamic loader
 * is done.  This routine has to exit the current function, then 
 * call the _dl_elf_main function.
 */

#define START()   return _dl_elf_main;      



/* Here we define the magic numbers that this dynamic loader should accept */

#define MAGIC1 EM_ARM
#undef  MAGIC2
/* Used for error messages */
#define ELF_TARGET "ARM"

struct elf_resolve;
unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);

static inline unsigned long arm_modulus(unsigned long m, unsigned long p) {
	unsigned long i,t,inc;
        i=p; t=0;
        while(!(i&(1<<31))) {
                i<<=1;
                t++;
        }
        t--;
        for(inc=t;inc>2;inc--) {
                i=p<<inc;
                if(i&(1<<31))
                        break;
                while(m>=i) {
                        m-=i;
                        i<<=1;
                        if(i&(1<<31))
                                break;
                        if(i<p)
                                break;
                }
        }
        while(m>=p) {
                m-=p;
        }
        return m;
}

#define do_rem(result, n, base)  result=arm_modulus(n,base);

/* 4096 bytes alignment */
#define PAGE_ALIGN 0xfffff000
#define ADDR_ALIGN 0xfff
#define OFFS_ALIGN 0x7ffff000

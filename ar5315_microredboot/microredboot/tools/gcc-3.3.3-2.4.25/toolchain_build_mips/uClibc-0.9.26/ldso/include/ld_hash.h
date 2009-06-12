#ifndef _LD_HASH_H_
#define _LD_HASH_H_

#ifndef RTLD_NEXT
#define RTLD_NEXT	((void*)-1)
#endif

struct dyn_elf{
  unsigned long flags;
  struct elf_resolve * dyn;
  struct dyn_elf * next_handle;  /* Used by dlopen et al. */
  struct dyn_elf * next;
  struct dyn_elf * prev;
};

struct elf_resolve{
  /* These entries must be in this order to be compatible with the interface used
     by gdb to obtain the list of symbols. */
  ElfW(Addr) loadaddr;		/* Base address shared object is loaded at.  */
  char *libname;		/* Absolute file name object was found in.  */
  ElfW(Dyn) *dynamic_addr;	/* Dynamic section of the shared object.  */
  struct elf_resolve * next;
  struct elf_resolve * prev;
  /* Nothing after this address is used by gdb. */
  enum {elf_lib, elf_executable,program_interpreter, loaded_file} libtype;
  struct dyn_elf * symbol_scope;
  unsigned short usage_count;
  unsigned short int init_flag;
  unsigned int nbucket;
  unsigned long * elf_buckets;
  /*
   * These are only used with ELF style shared libraries
   */
  unsigned long nchain;
  unsigned long * chains;
  unsigned long dynamic_info[24];

  unsigned long dynamic_size;
  unsigned long n_phent;
  Elf32_Phdr * ppnt;

#if defined(__mips__)
  /* Needed for MIPS relocation */
  unsigned long mips_gotsym;
  unsigned long mips_local_gotno;
  unsigned long mips_symtabno;
#endif

#ifdef __powerpc__
  /* this is used to store the address of relocation data words, so
   * we don't have to calculate it every time, which requires a divide */
  unsigned long data_words;
#endif
};

#define COPY_RELOCS_DONE    1
#define RELOCS_DONE         2
#define JMP_RELOCS_DONE     4
#define INIT_FUNCS_CALLED   8

extern struct dyn_elf     * _dl_symbol_tables;
extern struct elf_resolve * _dl_loaded_modules;
extern struct dyn_elf 	  * _dl_handles;

extern struct elf_resolve * _dl_check_hashed_files(const char * libname);
extern struct elf_resolve * _dl_add_elf_hash_table(const char * libname, 
	char * loadaddr, unsigned long * dynamic_info, 
	unsigned long dynamic_addr, unsigned long dynamic_size);

enum caller_type{symbolrel=0,copyrel=1,resolver=2};
extern char * _dl_find_hash(const char * name, struct dyn_elf * rpnt1, 
	struct elf_resolve * f_tpnt, enum caller_type);

extern int _dl_linux_dynamic_link(void);

extern char * _dl_library_path;
extern char * _dl_not_lazy;
extern unsigned long _dl_elf_hash(const char * name);

static inline int _dl_symbol(char * name)
{
  if(name[0] != '_' || name[1] != 'd' || name[2] != 'l' || name[3] != '_')
    return 0;
  return 1;
}


#define LD_ERROR_NOFILE 1
#define LD_ERROR_NOZERO 2
#define LD_ERROR_NOTELF 3
#define LD_ERROR_NOTMAGIC 4
#define LD_ERROR_NOTDYN 5
#define LD_ERROR_MMAP_FAILED 6
#define LD_ERROR_NODYNAMIC 7
#define LD_WRONG_RELOCS 8
#define LD_BAD_HANDLE 9
#define LD_NO_SYMBOL 10



#endif /* _LD_HASH_H_ */



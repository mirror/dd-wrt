#ifndef LINUXELF_H
#define LINUXELF_H

#include <ld_sysdep.h> /* before elf.h to get ELF_USES_RELOCA right */
#include <elf.h>
#include <link.h>

#ifdef DEBUG
#  define LDSO_CONF  "../util/ld.so.conf"
#  define LDSO_CACHE "../util/ld.so.cache"
#  define LDSO_PRELOAD "../util/ld.so.preload"
#else
#  define LDSO_CONF  UCLIBC_RUNTIME_PREFIX "etc/ld.so.conf"
#  define LDSO_CACHE UCLIBC_RUNTIME_PREFIX "etc/ld.so.cache"
#  define LDSO_PRELOAD UCLIBC_RUNTIME_PREFIX "etc/ld.so.preload"
#endif


#define LIB_ANY	     -1
#define LIB_DLL       0
#define LIB_ELF       1
#define LIB_ELF64     0x80
#define LIB_ELF_LIBC5 2
#define LIB_ELF_LIBC6 3
#define LIB_ELF_LIBC0 4

/* Forward declarations for stuff defined in ld_hash.h */
struct dyn_elf;
struct elf_resolve;


/* Definitions and prototypes for cache stuff */
#ifdef USE_CACHE
extern int _dl_map_cache(void);
extern int _dl_unmap_cache(void);

#define LDSO_CACHE_MAGIC "ld.so-"
#define LDSO_CACHE_MAGIC_LEN (sizeof LDSO_CACHE_MAGIC -1)
#define LDSO_CACHE_VER "1.7.0"
#define LDSO_CACHE_VER_LEN (sizeof LDSO_CACHE_VER -1)

typedef struct {
	char magic   [LDSO_CACHE_MAGIC_LEN];
	char version [LDSO_CACHE_VER_LEN];
	int nlibs;
} header_t;

typedef struct {
	int flags;
	int sooffset;
	int liboffset;
} libentry_t;

#else
static inline void _dl_map_cache(void) { }
static inline void _dl_unmap_cache(void) { }
#endif	


/* Function prototypes for non-static stuff in readelflib1.c */
int _dl_copy_fixups(struct dyn_elf * tpnt);
extern int _dl_parse_copy_information(struct dyn_elf *rpnt,
	unsigned long rel_addr, unsigned long rel_size, int type);
extern void _dl_parse_lazy_relocation_information(struct elf_resolve *tpnt,
	unsigned long rel_addr, unsigned long rel_size, int type);
extern int _dl_parse_relocation_information(struct elf_resolve *tpnt,
	unsigned long rel_addr, unsigned long rel_size, int type);
extern struct elf_resolve * _dl_load_shared_library(int secure, 
	struct dyn_elf **rpnt, struct elf_resolve *tpnt, char *full_libname);
extern struct elf_resolve * _dl_load_elf_shared_library(int secure, 
	struct dyn_elf **rpnt, char *libname);
extern struct elf_resolve *_dl_check_if_named_library_is_loaded(const char *full_libname);
extern int _dl_linux_resolve(void);


/*
 * Datatype of a relocation on this platform
 */
#ifdef ELF_USES_RELOCA
# define ELF_RELOC	ElfW(Rela)
#else
# define ELF_RELOC	ElfW(Rel)
#endif


/* Convert between the Linux flags for page protections and the
   ones specified in the ELF standard. */
#define LXFLAGS(X) ( (((X) & PF_R) ? PROT_READ : 0) | \
		    (((X) & PF_W) ? PROT_WRITE : 0) | \
		    (((X) & PF_X) ? PROT_EXEC : 0))


#endif	/* LINUXELF_H */

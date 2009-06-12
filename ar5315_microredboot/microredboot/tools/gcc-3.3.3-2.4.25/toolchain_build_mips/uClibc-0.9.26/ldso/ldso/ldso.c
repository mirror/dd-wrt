/* vi: set sw=4 ts=4: */
/* Program to load an ELF binary on a linux system, and run it
 * after resolving ELF shared library symbols
 *
 * Copyright (c) 1994-2000 Eric Youngdale, Peter MacDonald, 
 *				David Engel, Hongjiu Lu and Mitch D'Souza
 * Copyright (C) 2001-2002, Erik Andersen
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the above contributors may not be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

// Support a list of library preloads in /etc/ld.so.preload
//#define SUPPORT_LDSO_PRELOAD_FILE


/* Disclaimer:  I have never seen any AT&T source code for SVr4, nor have
   I ever taken any courses on internals.  This program was developed using
   information available through the book "UNIX SYSTEM V RELEASE 4,
   Programmers guide: Ansi C and Programming Support Tools", which did
   a more than adequate job of explaining everything required to get this
   working. */

/*
 * The main trick with this program is that initially, we ourselves are
 * not dynamicly linked.  This means that we cannot access any global
 * variables or call any functions.  No globals initially, since the
 * Global Offset Table (GOT) is initialized by the linker assuming a
 * virtual address of 0, and no function calls initially since the
 * Procedure Linkage Table (PLT) is not yet initialized.
 *
 * There are additional initial restrictions - we cannot use large
 * switch statements, since the compiler generates tables of addresses
 * and jumps through them.  We can use inline functions, because these
 * do not transfer control to a new address, but they must be static so
 * that they are not exported from the modules.  We cannot use normal
 * syscall stubs, because these all reference the errno global variable
 * which is not yet initialized.  We can use all of the local stack
 * variables that we want.
 *
 * Life is further complicated by the fact that initially we do not
 * want to do a complete dynamic linking.  We want to allow the user to
 * supply new functions to override symbols (i.e. weak symbols and/or
 * LD_PRELOAD).  So initially, we only perform relocations for
 * variables that start with "_dl_" since ANSI specifies that the user
 * is not supposed to redefine any of these variables.
 *
 * Fortunately, the linker itself leaves a few clues lying around, and
 * when the kernel starts the image, there are a few further clues.
 * First of all, there is Auxiliary Vector Table information sitting on
 * which is provided to us by the kernel, and which includes
 * information about the load address that the program interpreter was
 * loaded at, the number of sections, the address the application was
 * loaded at and so forth.  Here this information is stored in the
 * array auxvt.  For details see linux/fs/binfmt_elf.c where it calls
 * NEW_AUX_ENT() a bunch of time....
 *
 * Next, we need to find the GOT.  On most arches there is a register
 * pointing to the GOT, but just in case (and for new ports) I've added
 * some (slow) C code to locate the GOT for you. 
 *
 * This code was originally written for SVr4, and there the kernel
 * would load all text pages R/O, so they needed to call mprotect a
 * zillion times to mark all text pages as writable so dynamic linking
 * would succeed.  Then when they were done, they would change the
 * protections for all the pages back again.  Well, under Linux
 * everything is loaded writable (since Linux does copy on write
 * anyways) so all the mprotect stuff has been disabled.
 *
 * Initially, we do not have access to _dl_malloc since we can't yet
 * make function calls, so we mmap one page to use as scratch space.
 * Later on, when we can call _dl_malloc we reuse this this memory.
 * This is also beneficial, since we do not want to use the same memory
 * pool as malloc anyway - esp if the user redefines malloc to do
 * something funky.
 *
 * Our first task is to perform a minimal linking so that we can call
 * other portions of the dynamic linker.  Once we have done this, we
 * then build the list of modules that the application requires, using
 * LD_LIBRARY_PATH if this is not a suid program (/usr/lib otherwise).
 * Once this is done, we can do the dynamic linking as required, and we
 * must omit the things we did to get the dynamic linker up and running
 * in the first place.  After we have done this, we just have a few
 * housekeeping chores and we can transfer control to the user's
 * application.
 */

#include "ldso.h"


#define ALLOW_ZERO_PLTGOT

/*  Some arches may need to override this in boot1_arch.h */
#define	    ELFMAGIC	ELFMAG

/* This is a poor man's malloc, used prior to resolving our internal poor man's malloc */
#define LD_MALLOC(SIZE) ((void *) (malloc_buffer += SIZE, malloc_buffer - SIZE)) ;  REALIGN();
/*
 * Make sure that the malloc buffer is aligned on 4 byte boundary.  For 64 bit
 * platforms we may need to increase this to 8, but this is good enough for
 * now.  This is typically called after LD_MALLOC.
 */
#define REALIGN() malloc_buffer = (char *) (((unsigned long) malloc_buffer + 3) & ~(3))

char *_dl_library_path = 0;		/* Where we look for libraries */
char *_dl_preload = 0;			/* Things to be loaded before the libs. */
char *_dl_ldsopath = 0;
static int _dl_be_lazy = RTLD_LAZY;
#ifdef __SUPPORT_LD_DEBUG__
char *_dl_debug  = 0;
char *_dl_debug_symbols = 0;
char *_dl_debug_move    = 0;
char *_dl_debug_reloc   = 0;
char *_dl_debug_detail  = 0;
char *_dl_debug_nofixups  = 0;
char *_dl_debug_bindings  = 0;
int   _dl_debug_file = 2;
#else
#define _dl_debug_file 2
#endif
static char *_dl_malloc_addr, *_dl_mmap_zero;

static char *_dl_trace_loaded_objects = 0;
static int (*_dl_elf_main) (int, char **, char **);
struct r_debug *_dl_debug_addr = NULL;
unsigned long *_dl_brkp;
unsigned long *_dl_envp;
int _dl_fixup(struct elf_resolve *tpnt, int lazy);
void _dl_debug_state(void);
char *_dl_get_last_path_component(char *path);
static void _dl_get_ready_to_run(struct elf_resolve *tpnt, struct elf_resolve *app_tpnt, 
		unsigned long load_addr, unsigned long *hash_addr, Elf32_auxv_t auxvt[AT_EGID + 1], 
		char **envp, struct r_debug *debug_addr);

#include "boot1_arch.h"
#include "_dl_progname.h"				/* Pull in the value of _dl_progname */

/* When we enter this piece of code, the program stack looks like this:
        argc            argument counter (integer)
        argv[0]         program name (pointer)
        argv[1...N]     program args (pointers)
        argv[argc-1]    end of args (integer)
		NULL
        env[0...N]      environment variables (pointers)
        NULL
		auxvt[0...N]   Auxiliary Vector Table elements (mixed types)
*/

#ifdef __SUPPORT_LD_DEBUG_EARLY__
/* Debugging is especially tricky on PowerPC, since string literals
 * require relocations.  Thus, you can't use _dl_dprintf() for
 * anything until the bootstrap relocations are finished. */
static inline void hexprint(unsigned long x)
{
	int i;
	char c;

	for (i = 0; i < 8; i++) {
		c = ((x >> 28) + '0');
		if (c > '9')
			c += 'a' - '9' - 1;
		_dl_write(1, &c, 1);
		x <<= 4;
	}
	c = '\n';
	_dl_write(1, &c, 1);
}
#endif

LD_BOOT(unsigned long args) __attribute__ ((unused));

LD_BOOT(unsigned long args)
{
	unsigned int argc;
	char **argv, **envp;
	unsigned long load_addr;
	unsigned long *got;
	unsigned long *aux_dat;
	int goof = 0;
	ElfW(Ehdr) *header;
	struct elf_resolve *tpnt;
	struct elf_resolve *app_tpnt;
	Elf32_auxv_t auxvt[AT_EGID + 1];
	unsigned char *malloc_buffer, *mmap_zero;
	Elf32_Dyn *dpnt;
	unsigned long *hash_addr;
	struct r_debug *debug_addr = NULL;
	int indx;
	int status;


	/* WARNING! -- we cannot make _any_ funtion calls until we have
	 * taken care of fixing up our own relocations.  Making static
	 * inline calls is ok, but _no_ function calls.  Not yet
	 * anyways. */

	/* First obtain the information on the stack that tells us more about
	   what binary is loaded, where it is loaded, etc, etc */
	GET_ARGV(aux_dat, args);
#if defined (__arm__) || defined (__mips__) || defined (__cris__)
	aux_dat += 1;
#endif
	argc = *(aux_dat - 1);
	argv = (char **) aux_dat;
	aux_dat += argc;			/* Skip over the argv pointers */
	aux_dat++;					/* Skip over NULL at end of argv */
	envp = (char **) aux_dat;
	while (*aux_dat)
		aux_dat++;				/* Skip over the envp pointers */
	aux_dat++;					/* Skip over NULL at end of envp */

	/* Place -1 here as a checkpoint.  We later check if it was changed
	 * when we read in the auxvt */
	auxvt[AT_UID].a_type = -1;

	/* The junk on the stack immediately following the environment is  
	 * the Auxiliary Vector Table.  Read out the elements of the auxvt,
	 * sort and store them in auxvt for later use. */
	while (*aux_dat) {
		Elf32_auxv_t *auxv_entry = (Elf32_auxv_t *) aux_dat;

		if (auxv_entry->a_type <= AT_EGID) {
			_dl_memcpy(&(auxvt[auxv_entry->a_type]), auxv_entry, sizeof(Elf32_auxv_t));
		}
		aux_dat += 2;
	}

	/* locate the ELF header.   We need this done as soon as possible 
	 * (esp since SEND_STDERR() needs this on some platforms... */
	load_addr = auxvt[AT_BASE].a_un.a_val;
	header = (ElfW(Ehdr) *) auxvt[AT_BASE].a_un.a_ptr;

	/* Check the ELF header to make sure everything looks ok.  */
	if (!header || header->e_ident[EI_CLASS] != ELFCLASS32 ||
		header->e_ident[EI_VERSION] != EV_CURRENT
#if !defined(__powerpc__) && !defined(__mips__) && !defined(__sh__)
		|| _dl_strncmp((void *) header, ELFMAGIC, SELFMAG) != 0
#else
	        || header->e_ident[EI_MAG0] != ELFMAG0
	        || header->e_ident[EI_MAG1] != ELFMAG1
	        || header->e_ident[EI_MAG2] != ELFMAG2
	        || header->e_ident[EI_MAG3] != ELFMAG3
#endif
		) {
		SEND_STDERR("Invalid ELF header\n");
		_dl_exit(0);
	}
#ifdef __SUPPORT_LD_DEBUG_EARLY__
	SEND_STDERR("ELF header=");
	SEND_ADDRESS_STDERR(load_addr, 1);
#endif


	/* Locate the global offset table.  Since this code must be PIC  
	 * we can take advantage of the magic offset register, if we
	 * happen to know what that is for this architecture.  If not,
	 * we can always read stuff out of the ELF file to find it... */
#if defined(__i386__)
  __asm__("\tmovl %%ebx,%0\n\t":"=a"(got));
#elif defined(__m68k__)
  __asm__("movel %%a5,%0":"=g"(got))
#elif defined(__sparc__)
  __asm__("\tmov %%l7,%0\n\t":"=r"(got))
#elif defined(__arm__)
  __asm__("\tmov %0, r10\n\t":"=r"(got));
#elif defined(__powerpc__)
  __asm__("\tbl _GLOBAL_OFFSET_TABLE_-4@local\n\t":"=l"(got));
#elif defined(__mips__)
  __asm__("\tmove %0, $28\n\tsubu %0,%0,0x7ff0\n\t":"=r"(got));
#elif defined(__sh__) && !defined(__SH5__)
  __asm__(
"       mov.l    1f, %0\n"
"       mova     1f, r0\n"
"       bra      2f\n"
"       add r0,  %0\n"
"       .balign  4\n"
"1:     .long    _GLOBAL_OFFSET_TABLE_\n"
"2:" : "=r" (got) : : "r0");
#elif defined(__cris__)
  __asm__("\tmove.d $pc,%0\n\tsub.d .:GOTOFF,%0\n\t":"=r"(got));
#else
	/* Do things the slow way in C */
	{
		unsigned long tx_reloc;
		Elf32_Dyn *dynamic = NULL;
		Elf32_Shdr *shdr;
		Elf32_Phdr *pt_load;

#ifdef __SUPPORT_LD_DEBUG_EARLY__
		SEND_STDERR("Finding the GOT using C code to read the ELF file\n");
#endif
		/* Find where the dynamic linking information section is hiding */
		shdr = (Elf32_Shdr *) (header->e_shoff + (char *) header);
		for (indx = header->e_shnum; --indx >= 0; ++shdr) {
			if (shdr->sh_type == SHT_DYNAMIC) {
				goto found_dynamic;
			}
		}
		SEND_STDERR("missing dynamic linking information section \n");
		_dl_exit(0);

	  found_dynamic:
		dynamic = (Elf32_Dyn *) (shdr->sh_offset + (char *) header);

		/* Find where PT_LOAD is hiding */
		pt_load = (Elf32_Phdr *) (header->e_phoff + (char *) header);
		for (indx = header->e_phnum; --indx >= 0; ++pt_load) {
			if (pt_load->p_type == PT_LOAD) {
				goto found_pt_load;
			}
		}
		SEND_STDERR("missing loadable program segment\n");
		_dl_exit(0);

	  found_pt_load:
		/* Now (finally) find where DT_PLTGOT is hiding */
		tx_reloc = pt_load->p_vaddr - pt_load->p_offset;
		for (; DT_NULL != dynamic->d_tag; ++dynamic) {
			if (dynamic->d_tag == DT_PLTGOT) {
				goto found_got;
			}
		}
		SEND_STDERR("missing global offset table\n");
		_dl_exit(0);

	  found_got:
		got = (unsigned long *) (dynamic->d_un.d_val - tx_reloc +
				(char *) header);
	}
#endif

	/* Now, finally, fix up the location of the dynamic stuff */
	dpnt = (Elf32_Dyn *) (*got + load_addr);
#ifdef __SUPPORT_LD_DEBUG_EARLY__
	SEND_STDERR("First Dynamic section entry=");
	SEND_ADDRESS_STDERR(dpnt, 1);
#endif


	/* Call mmap to get a page of writable memory that can be used 
	 * for _dl_malloc throughout the shared lib loader. */
	mmap_zero = malloc_buffer = _dl_mmap((void *) 0, PAGE_SIZE, 
			PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (_dl_mmap_check_error(mmap_zero)) {
		SEND_STDERR("dl_boot: mmap of a spare page failed!\n");
		_dl_exit(13);
	}

	tpnt = LD_MALLOC(sizeof(struct elf_resolve));
	_dl_memset(tpnt, 0, sizeof(struct elf_resolve));
	app_tpnt = LD_MALLOC(sizeof(struct elf_resolve));
	_dl_memset(app_tpnt, 0, sizeof(struct elf_resolve));

#ifdef __UCLIBC_PIE_SUPPORT__
	/* Find the runtime load address of the main executable, this may be
         * different from what the ELF header says for ET_DYN/PIE executables.
	 */
	{
		ElfW(Phdr) *ppnt;
		int i;

		ppnt = (ElfW(Phdr) *) auxvt[AT_PHDR].a_un.a_ptr;
		for (i = 0; i < auxvt[AT_PHNUM].a_un.a_val; i++, ppnt++)
			if (ppnt->p_type == PT_PHDR) {
				app_tpnt->loadaddr = (ElfW(Addr)) (auxvt[AT_PHDR].a_un.a_val - ppnt->p_vaddr);
				break;
			}
	}

#ifdef __SUPPORT_LD_DEBUG_EARLY__
	SEND_STDERR("app_tpnt->loadaddr=");
	SEND_ADDRESS_STDERR(app_tpnt->loadaddr, 1);
#endif
#endif

	/*
	 * This is used by gdb to locate the chain of shared libraries that are currently loaded.
	 */
	debug_addr = LD_MALLOC(sizeof(struct r_debug));
	_dl_memset(debug_addr, 0, sizeof(struct r_debug));

	/* OK, that was easy.  Next scan the DYNAMIC section of the image.
	   We are only doing ourself right now - we will have to do the rest later */
#ifdef __SUPPORT_LD_DEBUG_EARLY__
	SEND_STDERR("scanning DYNAMIC section\n");
#endif
	while (dpnt->d_tag) {
#if defined(__mips__)
		if (dpnt->d_tag == DT_MIPS_GOTSYM)
			tpnt->mips_gotsym = (unsigned long) dpnt->d_un.d_val;
		if (dpnt->d_tag == DT_MIPS_LOCAL_GOTNO)
			tpnt->mips_local_gotno = (unsigned long) dpnt->d_un.d_val;
		if (dpnt->d_tag == DT_MIPS_SYMTABNO)
			tpnt->mips_symtabno = (unsigned long) dpnt->d_un.d_val;
#endif
		if (dpnt->d_tag < 24) {
			tpnt->dynamic_info[dpnt->d_tag] = dpnt->d_un.d_val;
			if (dpnt->d_tag == DT_TEXTREL) {
				tpnt->dynamic_info[DT_TEXTREL] = 1;
			}
		}
		dpnt++;
	}

	{
		ElfW(Phdr) *ppnt;
		int i;

		ppnt = (ElfW(Phdr) *) auxvt[AT_PHDR].a_un.a_ptr;
		for (i = 0; i < auxvt[AT_PHNUM].a_un.a_val; i++, ppnt++)
			if (ppnt->p_type == PT_DYNAMIC) {
#ifndef __UCLIBC_PIE_SUPPORT__
				dpnt = (Elf32_Dyn *) ppnt->p_vaddr;
#else
				dpnt = (Elf32_Dyn *) (ppnt->p_vaddr + app_tpnt->loadaddr);
#endif
				while (dpnt->d_tag) {
#if defined(__mips__)
					if (dpnt->d_tag == DT_MIPS_GOTSYM)
						app_tpnt->mips_gotsym =
							(unsigned long) dpnt->d_un.d_val;
					if (dpnt->d_tag == DT_MIPS_LOCAL_GOTNO)
						app_tpnt->mips_local_gotno =
							(unsigned long) dpnt->d_un.d_val;
					if (dpnt->d_tag == DT_MIPS_SYMTABNO)
						app_tpnt->mips_symtabno =
							(unsigned long) dpnt->d_un.d_val;
					if (dpnt->d_tag > DT_JMPREL) {
						dpnt++;
						continue;
					}
					app_tpnt->dynamic_info[dpnt->d_tag] = dpnt->d_un.d_val;

#warning "Debugging threads on mips won't work till someone fixes this..."
#if 0
					if (dpnt->d_tag == DT_DEBUG) {
						dpnt->d_un.d_val = (unsigned long) debug_addr;
					}
#endif

#else
					if (dpnt->d_tag > DT_JMPREL) {
						dpnt++;
						continue;
					}
					app_tpnt->dynamic_info[dpnt->d_tag] = dpnt->d_un.d_val;
					if (dpnt->d_tag == DT_DEBUG) {
						dpnt->d_un.d_val = (unsigned long) debug_addr;
					}
#endif
					if (dpnt->d_tag == DT_TEXTREL)
						app_tpnt->dynamic_info[DT_TEXTREL] = 1;
					dpnt++;
				}
			}
	}

#ifdef __SUPPORT_LD_DEBUG_EARLY__
	SEND_STDERR("done scanning DYNAMIC section\n");
#endif

	/* Get some more of the information that we will need to dynamicly link
	   this module to itself */

	hash_addr = (unsigned long *) (tpnt->dynamic_info[DT_HASH] + load_addr);
	tpnt->nbucket = *hash_addr++;
	tpnt->nchain = *hash_addr++;
	tpnt->elf_buckets = hash_addr;
	hash_addr += tpnt->nbucket;

#ifdef __SUPPORT_LD_DEBUG_EARLY__
	SEND_STDERR("done grabbing link information\n");
#endif

#ifndef FORCE_SHAREABLE_TEXT_SEGMENTS
	/* Ugly, ugly.  We need to call mprotect to change the protection of
	   the text pages so that we can do the dynamic linking.  We can set the
	   protection back again once we are done */

	{
		ElfW(Phdr) *ppnt;
		int i;

#ifdef __SUPPORT_LD_DEBUG_EARLY__
		SEND_STDERR("calling mprotect on the shared library/dynamic linker\n");
#endif

		/* First cover the shared library/dynamic linker. */
		if (tpnt->dynamic_info[DT_TEXTREL]) {
			header = (ElfW(Ehdr) *) auxvt[AT_BASE].a_un.a_ptr;
			ppnt = (ElfW(Phdr) *) ((int)auxvt[AT_BASE].a_un.a_ptr + 
					header->e_phoff);
			for (i = 0; i < header->e_phnum; i++, ppnt++) {
				if (ppnt->p_type == PT_LOAD && !(ppnt->p_flags & PF_W)) {
					_dl_mprotect((void *) (load_addr + (ppnt->p_vaddr & PAGE_ALIGN)), 
							(ppnt->p_vaddr & ADDR_ALIGN) + (unsigned long) ppnt->p_filesz, 
							PROT_READ | PROT_WRITE | PROT_EXEC);
				}
			}
		}

#ifdef __SUPPORT_LD_DEBUG_EARLY__
		SEND_STDERR("calling mprotect on the application program\n");
#endif
		/* Now cover the application program. */
		if (app_tpnt->dynamic_info[DT_TEXTREL]) {
			ppnt = (ElfW(Phdr) *) auxvt[AT_PHDR].a_un.a_ptr;
			for (i = 0; i < auxvt[AT_PHNUM].a_un.a_val; i++, ppnt++) {
				if (ppnt->p_type == PT_LOAD && !(ppnt->p_flags & PF_W))
#ifndef __UCLIBC_PIE_SUPPORT__
					_dl_mprotect((void *) (ppnt->p_vaddr & PAGE_ALIGN),
								 (ppnt->p_vaddr & ADDR_ALIGN) +
#else
					_dl_mprotect((void *) ((ppnt->p_vaddr + app_tpnt->loadaddr) & PAGE_ALIGN),
								 ((ppnt->p_vaddr + app_tpnt->loadaddr) & ADDR_ALIGN) +
#endif
								 (unsigned long) ppnt->p_filesz,
								 PROT_READ | PROT_WRITE | PROT_EXEC);
			}
		}
	}
#endif
	
#if defined(__mips__)
#ifdef __SUPPORT_LD_DEBUG_EARLY__
	SEND_STDERR("About to do MIPS specific GOT bootstrap\n");
#endif
	/* For MIPS we have to do stuff to the GOT before we do relocations.  */
	PERFORM_BOOTSTRAP_GOT(got);
#endif

	/* OK, now do the relocations.  We do not do a lazy binding here, so
	   that once we are done, we have considerably more flexibility. */
#ifdef __SUPPORT_LD_DEBUG_EARLY__
	SEND_STDERR("About to do library loader relocations\n");
#endif

	goof = 0;
	for (indx = 0; indx < 2; indx++) {
		unsigned int i;
		ELF_RELOC *rpnt;
		unsigned long *reloc_addr;
		unsigned long symbol_addr;
		int symtab_index;
		unsigned long rel_addr, rel_size;


#ifdef ELF_USES_RELOCA
		rel_addr = (indx ? tpnt->dynamic_info[DT_JMPREL] : tpnt->
			 dynamic_info[DT_RELA]);
		rel_size = (indx ? tpnt->dynamic_info[DT_PLTRELSZ] : tpnt->
			 dynamic_info[DT_RELASZ]);
#else
		rel_addr = (indx ? tpnt->dynamic_info[DT_JMPREL] : tpnt->
			 dynamic_info[DT_REL]);
		rel_size = (indx ? tpnt->dynamic_info[DT_PLTRELSZ] : tpnt->
			 dynamic_info[DT_RELSZ]);
#endif

		if (!rel_addr)
			continue;

		/* Now parse the relocation information */
		rpnt = (ELF_RELOC *) (rel_addr + load_addr);
		for (i = 0; i < rel_size; i += sizeof(ELF_RELOC), rpnt++) {
			reloc_addr = (unsigned long *) (load_addr + (unsigned long) rpnt->r_offset);
			symtab_index = ELF32_R_SYM(rpnt->r_info);
			symbol_addr = 0;
			if (symtab_index) {
				char *strtab;
				Elf32_Sym *symtab;

				symtab = (Elf32_Sym *) (tpnt->dynamic_info[DT_SYMTAB] + load_addr);
				strtab = (char *) (tpnt->dynamic_info[DT_STRTAB] + load_addr);

				/* We only do a partial dynamic linking right now.  The user
				   is not supposed to redefine any symbols that start with
				   a '_', so we can do this with confidence. */
				if (!_dl_symbol(strtab + symtab[symtab_index].st_name))
					continue;
				symbol_addr = load_addr + symtab[symtab_index].st_value;

				if (!symbol_addr) {
					/* This will segfault - you cannot call a function until
					 * we have finished the relocations.
					 */
					SEND_STDERR("ELF dynamic loader - unable to self-bootstrap - symbol ");
					SEND_STDERR(strtab + symtab[symtab_index].st_name);
					SEND_STDERR(" undefined.\n");
					goof++;
				}
#ifdef __SUPPORT_LD_DEBUG_EARLY__
				SEND_STDERR("About to fixup symbol: ");
				SEND_STDERR(strtab + symtab[symtab_index].st_name);
				SEND_STDERR("\n");
#endif  
				PERFORM_BOOTSTRAP_RELOC(rpnt, reloc_addr, symbol_addr, load_addr, &symtab[symtab_index]);
			} else {
				/*
				 * Use this machine-specific macro to perform the actual relocation.
				 */
				PERFORM_BOOTSTRAP_RELOC(rpnt, reloc_addr, symbol_addr, load_addr, NULL);
			}
		}
	}

	if (goof) {
		_dl_exit(14);
	}
#ifdef __SUPPORT_LD_DEBUG_EARLY__
	/* Wahoo!!! */
	_dl_dprintf(_dl_debug_file, "Done relocating library loader, so we can now\n\tuse globals and make function calls!\n");
#endif

	if (argv[0]) {
		_dl_progname = argv[0];
	}

	/* Start to build the tables of the modules that are required for
	 * this beast to run.  We start with the basic executable, and then
	 * go from there.  Eventually we will run across ourself, and we
	 * will need to properly deal with that as well. */

	/* Make it so _dl_malloc can use the page of memory we have already
	 * allocated, so we shouldn't need to grab any more memory */
	_dl_malloc_addr = malloc_buffer;
	_dl_mmap_zero = mmap_zero;



	/* Now we have done the mandatory linking of some things.  We are now
	   free to start using global variables, since these things have all been
	   fixed up by now.  Still no function calls outside of this library ,
	   since the dynamic resolver is not yet ready. */
	_dl_get_ready_to_run(tpnt, app_tpnt, load_addr, hash_addr, auxvt, envp, debug_addr);


	/* Notify the debugger that all objects are now mapped in.  */
	_dl_debug_addr->r_state = RT_CONSISTENT;
	_dl_debug_state();


	/* OK we are done here.  Turn out the lights, and lock up. */
	_dl_elf_main = (int (*)(int, char **, char **)) auxvt[AT_ENTRY].a_un.a_fcn;

	/*
	 * Transfer control to the application.
	 */
	status = 0;					/* Used on x86, but not on other arches */
#if defined (__SUPPORT_LD_DEBUG__)
	if(_dl_debug) _dl_dprintf(_dl_debug_file,"\ntransfering control: %s\n\n", _dl_progname);	
#endif    
	START();
}

#if defined (__SUPPORT_LD_DEBUG__)
static void debug_fini (int status, void *arg)
{
	(void)status;
	_dl_dprintf(_dl_debug_file,"\ncalling fini: %s\n\n", (const char*)arg);
}
#endif    

static void _dl_get_ready_to_run(struct elf_resolve *tpnt, struct elf_resolve *app_tpnt, 
		unsigned long load_addr, unsigned long *hash_addr, Elf32_auxv_t auxvt[AT_EGID + 1], 
		char **envp, struct r_debug *debug_addr)
{
	ElfW(Phdr) *ppnt;
	char *lpntstr;
	int i, _dl_secure, goof = 0;
	struct dyn_elf *rpnt;
	struct elf_resolve *tcurr;
	struct elf_resolve *tpnt1;
	unsigned long brk_addr, *lpnt;
	int (*_dl_atexit) (void *);
#if defined (__SUPPORT_LD_DEBUG__)
	int (*_dl_on_exit) (void (*FUNCTION)(int STATUS, void *ARG),void*);
#endif

	/* Now we have done the mandatory linking of some things.  We are now
	   free to start using global variables, since these things have all been
	   fixed up by now.  Still no function calls outside of this library ,
	   since the dynamic resolver is not yet ready. */
	lpnt = (unsigned long *) (tpnt->dynamic_info[DT_PLTGOT] + load_addr);

	tpnt->chains = hash_addr;
	tpnt->next = 0;
	tpnt->libname = 0;
	tpnt->libtype = program_interpreter;
	tpnt->loadaddr = (ElfW(Addr)) load_addr;

#ifdef ALLOW_ZERO_PLTGOT
	if (tpnt->dynamic_info[DT_PLTGOT])
#endif
	{
		INIT_GOT(lpnt, tpnt);
#ifdef __SUPPORT_LD_DEBUG_EARLY__
		_dl_dprintf(_dl_debug_file, "GOT found at %x\n", lpnt);
#endif
	}

	/* OK, this was a big step, now we need to scan all of the user images
	   and load them properly. */

	{
		ElfW(Ehdr) *epnt;
		ElfW(Phdr) *myppnt;
		int j;

		epnt = (ElfW(Ehdr) *) auxvt[AT_BASE].a_un.a_ptr;
		tpnt->n_phent = epnt->e_phnum;
		tpnt->ppnt = myppnt = (ElfW(Phdr) *) (load_addr + epnt->e_phoff);
		for (j = 0; j < epnt->e_phnum; j++, myppnt++) {
			if (myppnt->p_type == PT_DYNAMIC) {
				tpnt->dynamic_addr = (ElfW(Dyn) *)myppnt->p_vaddr + load_addr;
				tpnt->dynamic_size = myppnt->p_filesz;
			}
		}
	}

	brk_addr = 0;
	rpnt = NULL;

	/* At this point we are now free to examine the user application,
	   and figure out which libraries are supposed to be called.  Until
	   we have this list, we will not be completely ready for dynamic linking */

	ppnt = (ElfW(Phdr) *) auxvt[AT_PHDR].a_un.a_ptr;
	for (i = 0; i < auxvt[AT_PHNUM].a_un.a_val; i++, ppnt++) {
		if (ppnt->p_type == PT_LOAD) {
#ifndef __UCLIBC_PIE_SUPPORT__
			if (ppnt->p_vaddr + ppnt->p_memsz > brk_addr)
				brk_addr = ppnt->p_vaddr + ppnt->p_memsz;
#else
			if (ppnt->p_vaddr + app_tpnt->loadaddr + ppnt->p_memsz > brk_addr)
				brk_addr = ppnt->p_vaddr + app_tpnt->loadaddr + ppnt->p_memsz;
#endif
		}
		if (ppnt->p_type == PT_DYNAMIC) {
#ifndef ALLOW_ZERO_PLTGOT
			/* make sure it's really there. */
			if (app_tpnt->dynamic_info[DT_PLTGOT] == 0)
				continue;
#endif
			/* OK, we have what we need - slip this one into the list. */
#ifndef __UCLIBC_PIE_SUPPORT__
			app_tpnt = _dl_add_elf_hash_table("", 0, 
					app_tpnt->dynamic_info, ppnt->p_vaddr, ppnt->p_filesz);
#else
			app_tpnt = _dl_add_elf_hash_table("", (char *)app_tpnt->loadaddr,
					app_tpnt->dynamic_info, ppnt->p_vaddr + app_tpnt->loadaddr, ppnt->p_filesz);
#endif
			_dl_loaded_modules->libtype = elf_executable;
			_dl_loaded_modules->ppnt = (ElfW(Phdr) *) auxvt[AT_PHDR].a_un.a_ptr;
			_dl_loaded_modules->n_phent = auxvt[AT_PHNUM].a_un.a_val;
			_dl_symbol_tables = rpnt = (struct dyn_elf *) _dl_malloc(sizeof(struct dyn_elf));
			_dl_memset(rpnt, 0, sizeof(struct dyn_elf));
			rpnt->dyn = _dl_loaded_modules;
			app_tpnt->usage_count++;
			app_tpnt->symbol_scope = _dl_symbol_tables;
#ifndef __UCLIBC_PIE_SUPPORT__
			lpnt = (unsigned long *) (app_tpnt->dynamic_info[DT_PLTGOT]);
#else
			lpnt = (unsigned long *) (app_tpnt->dynamic_info[DT_PLTGOT] + app_tpnt->loadaddr);
#endif
#ifdef ALLOW_ZERO_PLTGOT
			if (lpnt)
#endif
				INIT_GOT(lpnt, _dl_loaded_modules);
		}

		/* OK, fill this in - we did not have this before */
		if (ppnt->p_type == PT_INTERP) {	
			int readsize = 0;
			char *pnt, *pnt1, buf[1024];
			tpnt->libname = _dl_strdup((char *) ppnt->p_offset +
					(auxvt[AT_PHDR].a_un.a_val & PAGE_ALIGN));
			
			/* Determine if the shared lib loader is a symlink */
			_dl_memset(buf, 0, sizeof(buf));
			readsize = _dl_readlink(tpnt->libname, buf, sizeof(buf));
			if (readsize > 0 && readsize < (int)(sizeof(buf)-1)) {
				pnt1 = _dl_strrchr(buf, '/');
				if (pnt1 && buf != pnt1) {
#ifdef __SUPPORT_LD_DEBUG_EARLY__
					_dl_dprintf(_dl_debug_file, "changing tpnt->libname from '%s' to '%s'\n", tpnt->libname, buf);
#endif
					tpnt->libname = _dl_strdup(buf);
				}
			}

			/* Store the path where the shared lib loader was found for 
			 * later use */
			pnt = _dl_strdup(tpnt->libname);
			pnt1 = _dl_strrchr(pnt, '/');
			if (pnt != pnt1) {
				*pnt1 = '\0';
				_dl_ldsopath = pnt;
			} else {
				_dl_ldsopath = tpnt->libname;
			}
#ifdef __SUPPORT_LD_DEBUG_EARLY__
			_dl_dprintf(_dl_debug_file, "Lib Loader:\t(%x) %s\n", tpnt->loadaddr, tpnt->libname);
#endif
		}
	}


	/* Now we need to figure out what kind of options are selected.
	   Note that for SUID programs we ignore the settings in LD_LIBRARY_PATH */
	{
		if (_dl_getenv("LD_BIND_NOW", envp))
			_dl_be_lazy = 0;

		if ((auxvt[AT_UID].a_un.a_val == -1 && _dl_suid_ok()) ||
				(auxvt[AT_UID].a_un.a_val != -1 && 
				 auxvt[AT_UID].a_un.a_val == auxvt[AT_EUID].a_un.a_val
				 && auxvt[AT_GID].a_un.a_val== auxvt[AT_EGID].a_un.a_val)) {
			_dl_secure = 0;
			_dl_preload = _dl_getenv("LD_PRELOAD", envp);
			_dl_library_path = _dl_getenv("LD_LIBRARY_PATH", envp);
		} else {
			_dl_secure = 1;
			_dl_preload = _dl_getenv("LD_PRELOAD", envp);
			_dl_unsetenv("LD_AOUT_PRELOAD", envp);
			_dl_unsetenv("LD_LIBRARY_PATH", envp);
			_dl_unsetenv("LD_AOUT_LIBRARY_PATH", envp);
			_dl_library_path = NULL;
		}
	}

#ifdef __SUPPORT_LD_DEBUG__
	_dl_debug    = _dl_getenv("LD_DEBUG", envp);
	if (_dl_debug)
	{
	  if (_dl_strstr(_dl_debug, "all")) {
	  	_dl_debug_detail = _dl_debug_move = _dl_debug_symbols
			= _dl_debug_reloc = _dl_debug_bindings = _dl_debug_nofixups = _dl_strstr(_dl_debug, "all");
	  }
	  else {
	  	_dl_debug_detail   = _dl_strstr(_dl_debug, "detail");
	  	_dl_debug_move     = _dl_strstr(_dl_debug, "move");
	  	_dl_debug_symbols  = _dl_strstr(_dl_debug, "sym");
	  	_dl_debug_reloc    = _dl_strstr(_dl_debug, "reloc");
	  	_dl_debug_nofixups = _dl_strstr(_dl_debug, "nofix");
	  	_dl_debug_bindings = _dl_strstr(_dl_debug, "bind");
	  }
	}
	{
	  const char *dl_debug_output;
	  
	  dl_debug_output = _dl_getenv("LD_DEBUG_OUTPUT", envp);

	  if (dl_debug_output)
	  {
	    char tmp[22], *tmp1, *filename;
	    int len1, len2;
	    
	    _dl_memset(tmp, 0, sizeof(tmp));
	    tmp1=_dl_simple_ltoa( tmp, (unsigned long)_dl_getpid());

	    len1 = _dl_strlen(dl_debug_output);
	    len2 = _dl_strlen(tmp1);

	    filename = _dl_malloc(len1+len2+2);

	    if (filename)
	    {
	      _dl_strcpy (filename, dl_debug_output);
	      filename[len1] = '.';
	      _dl_strcpy (&filename[len1+1], tmp1);

	      _dl_debug_file= _dl_open (filename, O_WRONLY|O_CREAT);
	      if (_dl_debug_file<0)
	      {
		_dl_debug_file = 2;
		_dl_dprintf (2, "can't open file: '%s'\n",filename);
	      }
	    }
	  }
	}
	
	
#endif	
	_dl_trace_loaded_objects = _dl_getenv("LD_TRACE_LOADED_OBJECTS", envp);
#ifndef __LDSO_LDD_SUPPORT__
	if (_dl_trace_loaded_objects) {
		_dl_dprintf(_dl_debug_file, "Use the ldd provided by uClibc\n");
		_dl_exit(1);
	}
#endif

	/*
	 * OK, fix one more thing - set up debug_addr so it will point
	 * to our chain.  Later we may need to fill in more fields, but this
	 * should be enough for now.
	 */
	debug_addr->r_map = (struct link_map *) _dl_loaded_modules;
	debug_addr->r_version = 1;
	debug_addr->r_ldbase = load_addr;
	debug_addr->r_brk = (unsigned long) &_dl_debug_state;
	_dl_debug_addr = debug_addr;

	/* Notify the debugger we are in a consistant state */
	_dl_debug_addr->r_state = RT_CONSISTENT;
	_dl_debug_state();

	/* OK, we now have the application in the list, and we have some
	   basic stuff in place.  Now search through the list for other shared
	   libraries that should be loaded, and insert them on the list in the
	   correct order. */

	_dl_map_cache();


	if (_dl_preload) 
	{
		char c, *str, *str2;

		str = _dl_preload;
		while (*str == ':' || *str == ' ' || *str == '\t')
			str++;
		while (*str) 
		{
			str2 = str;
			while (*str2 && *str2 != ':' && *str2 != ' ' && *str2 != '\t')
				str2++;
			c = *str2;
			*str2 = '\0';
			if (!_dl_secure || _dl_strchr(str, '/') == NULL) 
			{
				if ((tpnt1 = _dl_check_if_named_library_is_loaded(str))) 
				{
					continue;
				}
#if defined (__SUPPORT_LD_DEBUG__)
				if(_dl_debug) _dl_dprintf(_dl_debug_file, "\tfile='%s';  needed by '%s'\n", 
						str, _dl_progname);
#endif
				tpnt1 = _dl_load_shared_library(_dl_secure, &rpnt, NULL, str);
				if (!tpnt1) {
#ifdef __LDSO_LDD_SUPPORT__
					if (_dl_trace_loaded_objects)
						_dl_dprintf(1, "\t%s => not found\n", str);
					else 
#endif
					{
						_dl_dprintf(2, "%s: can't load " "library '%s'\n", _dl_progname, str);
						_dl_exit(15);
					}
				} else {
#ifdef __SUPPORT_LD_DEBUG_EARLY__
					_dl_dprintf(_dl_debug_file, "Loading:\t(%x) %s\n", tpnt1->loadaddr, tpnt1->libname);
#endif
#ifdef __LDSO_LDD_SUPPORT__
					if (_dl_trace_loaded_objects && tpnt1->usage_count==1) {
						/* this is a real hack to make ldd not print 
						 * the library itself when run on a library. */
						if (_dl_strcmp(_dl_progname, str) != 0)
							_dl_dprintf(1, "\t%s => %s (%x)\n", str, tpnt1->libname, 
									(unsigned) tpnt1->loadaddr);
					}
#endif
				}
			}
			*str2 = c;
			str = str2;
			while (*str == ':' || *str == ' ' || *str == '\t')
				str++;
		}
	}

#ifdef SUPPORT_LDSO_PRELOAD_FILE
	{
		int fd;
		struct stat st;
		char *preload;
		if (!_dl_stat(LDSO_PRELOAD, &st) && st.st_size > 0) {
			if ((fd = _dl_open(LDSO_PRELOAD, O_RDONLY)) < 0) {
				_dl_dprintf(2, "%s: can't open file '%s'\n", 
						_dl_progname, LDSO_PRELOAD);
			} else {
				preload = (caddr_t) _dl_mmap(0, st.st_size + 1, 
						PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
				_dl_close(fd);
				if (preload == (caddr_t) - 1) {
					_dl_dprintf(2, "%s: can't map file '%s'\n", 
							_dl_progname, LDSO_PRELOAD);
				} else {
					char c, *cp, *cp2;

					/* convert all separators and comments to spaces */
					for (cp = preload; *cp; /*nada */ ) {
						if (*cp == ':' || *cp == '\t' || *cp == '\n') {
							*cp++ = ' ';
						} else if (*cp == '#') {
							do
								*cp++ = ' ';
							while (*cp != '\n' && *cp != '\0');
						} else {
							cp++;
						}
					}

					/* find start of first library */
					for (cp = preload; *cp && *cp == ' '; cp++)
						/*nada */ ;

					while (*cp) {
						/* find end of library */
						for (cp2 = cp; *cp && *cp != ' '; cp++)
							/*nada */ ;
						c = *cp;
						*cp = '\0';

						if ((tpnt1 = _dl_check_if_named_library_is_loaded(cp2))) 
						{
							continue;
						}
#if defined (__SUPPORT_LD_DEBUG__)
						if(_dl_debug) _dl_dprintf(_dl_debug_file, "\tfile='%s';  needed by '%s'\n", 
								cp2, _dl_progname);
#endif
						tpnt1 = _dl_load_shared_library(0, &rpnt, NULL, cp2);
						if (!tpnt1) {
#ifdef __LDSO_LDD_SUPPORT__
							if (_dl_trace_loaded_objects)
								_dl_dprintf(1, "\t%s => not found\n", cp2);
							else 
#endif
							{
								_dl_dprintf(2, "%s: can't load library '%s'\n", _dl_progname, cp2);
								_dl_exit(15);
							}
						} else {
#ifdef __SUPPORT_LD_DEBUG_EARLY__
							_dl_dprintf(_dl_debug_file, "Loading:\t(%x) %s\n", tpnt1->loadaddr, tpnt1->libname);
#endif
#ifdef __LDSO_LDD_SUPPORT__
							if (_dl_trace_loaded_objects && tpnt1->usage_count==1) {
								_dl_dprintf(1, "\t%s => %s (%x)\n", cp2, 
										tpnt1->libname, (unsigned) tpnt1->loadaddr);
							}
#endif
						}

						/* find start of next library */
						*cp = c;
						for ( /*nada */ ; *cp && *cp == ' '; cp++)
							/*nada */ ;
					}

					_dl_munmap(preload, st.st_size + 1);
				}
			}
		}
	}
#endif

	for (tcurr = _dl_loaded_modules; tcurr; tcurr = tcurr->next) 
	{
		Elf32_Dyn *dpnt;
		for (dpnt = (Elf32_Dyn *) tcurr->dynamic_addr; dpnt->d_tag; dpnt++) 
		{
			if (dpnt->d_tag == DT_NEEDED) 
			{
				char *name;
				lpntstr = (char*) (tcurr->loadaddr + tcurr->dynamic_info[DT_STRTAB] + dpnt->d_un.d_val);
				name = _dl_get_last_path_component(lpntstr);

				if ((tpnt1 = _dl_check_if_named_library_is_loaded(name))) 
				{
					continue;
				}
#if defined (__SUPPORT_LD_DEBUG__)
				if(_dl_debug) _dl_dprintf(_dl_debug_file, "\tfile='%s';  needed by '%s'\n", 
						lpntstr, _dl_progname);
#endif
				if (!(tpnt1 = _dl_load_shared_library(0, &rpnt, tcurr, lpntstr)))
				{
#ifdef __LDSO_LDD_SUPPORT__
					if (_dl_trace_loaded_objects) {
						_dl_dprintf(1, "\t%s => not found\n", lpntstr);
						continue;
					} else 
#endif
					{
						_dl_dprintf(2, "%s: can't load library '%s'\n", _dl_progname, lpntstr);
						_dl_exit(16);
					}
				} else {
#ifdef __SUPPORT_LD_DEBUG_EARLY__
					_dl_dprintf(_dl_debug_file, "Loading:\t(%x) %s\n", tpnt1->loadaddr, tpnt1->libname);
#endif
#ifdef __LDSO_LDD_SUPPORT__
					if (_dl_trace_loaded_objects && tpnt1->usage_count==1) {
						_dl_dprintf(1, "\t%s => %s (%x)\n", lpntstr, tpnt1->libname, 
								(unsigned) tpnt1->loadaddr);
					}
#endif
				}
			}
		}
	}


	_dl_unmap_cache();

	/*
	 * If the program interpreter is not in the module chain, add it.  This will
	 * be required for dlopen to be able to access the internal functions in the 
	 * dynamic linker.
	 */
	if (tpnt) {
		tcurr = _dl_loaded_modules;
		if (tcurr)
			while (tcurr->next)
				tcurr = tcurr->next;
		tpnt->next = NULL;
		tpnt->usage_count++;

		if (tcurr) {
			tcurr->next = tpnt;
			tpnt->prev = tcurr;
		} else {
			_dl_loaded_modules = tpnt;
			tpnt->prev = NULL;
		}
		if (rpnt) {
			rpnt->next = (struct dyn_elf *) _dl_malloc(sizeof(struct dyn_elf));
			_dl_memset(rpnt->next, 0, sizeof(struct dyn_elf));
			rpnt->next->prev = rpnt;
			rpnt = rpnt->next;
		} else {
			rpnt = (struct dyn_elf *) _dl_malloc(sizeof(struct dyn_elf));
			_dl_memset(rpnt, 0, sizeof(struct dyn_elf));
		}
		rpnt->dyn = tpnt;
		tpnt = NULL;
	}

#ifdef __LDSO_LDD_SUPPORT__
	/* End of the line for ldd.... */
	if (_dl_trace_loaded_objects) {
		_dl_dprintf(1, "\t%s => %s (%x)\n", rpnt->dyn->libname + (_dl_strlen(_dl_ldsopath)) + 1, 
				rpnt->dyn->libname, rpnt->dyn->loadaddr);  
		_dl_exit(0);
	}
#endif


#ifdef __mips__
	/*
	 * Relocation of the GOT entries for MIPS have to be done
	 * after all the libraries have been loaded.
	 */
	_dl_perform_mips_global_got_relocations(_dl_loaded_modules);
#endif

#ifdef __SUPPORT_LD_DEBUG_EARLY__
	_dl_dprintf(_dl_debug_file, "Beginning relocation fixups\n");
#endif
	/*
	 * OK, now all of the kids are tucked into bed in their proper addresses.
	 * Now we go through and look for REL and RELA records that indicate fixups
	 * to the GOT tables.  We need to do this in reverse order so that COPY
	 * directives work correctly */
	goof = _dl_loaded_modules ? _dl_fixup(_dl_loaded_modules, _dl_be_lazy) : 0;


	/* Some flavors of SVr4 do not generate the R_*_COPY directive,
	   and we have to manually search for entries that require fixups. 
	   Solaris gets this one right, from what I understand.  */

#ifdef __SUPPORT_LD_DEBUG_EARLY__
	_dl_dprintf(_dl_debug_file, "Beginning copy fixups\n");
#endif
	if (_dl_symbol_tables)
		goof += _dl_copy_fixups(_dl_symbol_tables);

	/* OK, at this point things are pretty much ready to run.  Now we
	   need to touch up a few items that are required, and then
	   we can let the user application have at it.  Note that
	   the dynamic linker itself is not guaranteed to be fully
	   dynamicly linked if we are using ld.so.1, so we have to look
	   up each symbol individually. */


	_dl_brkp = (unsigned long *) (intptr_t) _dl_find_hash("___brk_addr", NULL, NULL, symbolrel);
	
	if (_dl_brkp) {
		*_dl_brkp = brk_addr;
	}
	_dl_envp = (unsigned long *) (intptr_t) _dl_find_hash("__environ", NULL, NULL, symbolrel);

	if (_dl_envp) {
		*_dl_envp = (unsigned long) envp;
	}

#ifndef FORCE_SHAREABLE_TEXT_SEGMENTS
	{
		unsigned int j;
		ElfW(Phdr) *myppnt;

		/* We had to set the protections of all pages to R/W for dynamic linking.
		   Set text pages back to R/O */
		for (tpnt = _dl_loaded_modules; tpnt; tpnt = tpnt->next) {
			for (myppnt = tpnt->ppnt, j = 0; j < tpnt->n_phent; j++, myppnt++) {
				if (myppnt->p_type == PT_LOAD && !(myppnt->p_flags & PF_W) && tpnt->dynamic_info[DT_TEXTREL]) {
					_dl_mprotect((void *) (tpnt->loadaddr + (myppnt->p_vaddr & PAGE_ALIGN)), 
							(myppnt->p_vaddr & ADDR_ALIGN) + (unsigned long) myppnt->p_filesz, LXFLAGS(myppnt->p_flags));
				}
			}
		}

	}
#endif
	_dl_atexit = (int (*)(void *)) (intptr_t) _dl_find_hash("atexit", NULL, NULL, symbolrel);
#if defined (__SUPPORT_LD_DEBUG__)
	_dl_on_exit = (int (*)(void (*)(int, void *),void*)) 
		(intptr_t) _dl_find_hash("on_exit", NULL, NULL, symbolrel);
#endif

	/* Notify the debugger we have added some objects. */
	_dl_debug_addr->r_state = RT_ADD;
	_dl_debug_state();

	for (rpnt = _dl_symbol_tables; rpnt!=NULL&& rpnt->next!=NULL; rpnt=rpnt->next)
	  ;
	  
	for (;rpnt!=NULL; rpnt=rpnt->prev)
	{
  	        tpnt = rpnt->dyn;

	        if (tpnt->libtype == program_interpreter)
			continue;

		/* Apparently crt0/1 for the application is responsible for handling this.
		 * We only need to run the init/fini for shared libraries
		 */
	        if (tpnt->libtype == elf_executable)
			break;      /* at this point all shared libs are initialized !! */

		if (tpnt->init_flag & INIT_FUNCS_CALLED)
			continue;
		tpnt->init_flag |= INIT_FUNCS_CALLED;

		if (tpnt->dynamic_info[DT_INIT]) {
			void (*dl_elf_func) (void);
			dl_elf_func = (void (*)(void)) (intptr_t) (tpnt->loadaddr + tpnt->dynamic_info[DT_INIT]);
#if defined (__SUPPORT_LD_DEBUG__)
			if(_dl_debug) _dl_dprintf(_dl_debug_file,"\ncalling init: %s\n\n", tpnt->libname);	
#endif    
			(*dl_elf_func) ();
		}
		if (_dl_atexit && tpnt->dynamic_info[DT_FINI]) {
			void (*dl_elf_func) (void);
			dl_elf_func = (void (*)(void)) (intptr_t) (tpnt->loadaddr + tpnt->dynamic_info[DT_FINI]);
			(*_dl_atexit) (dl_elf_func);
#if defined (__SUPPORT_LD_DEBUG__)
			if(_dl_debug && _dl_on_exit)
			{
				(*_dl_on_exit)(debug_fini, tpnt->libname);
			}
#endif
		}
#if defined (__SUPPORT_LD_DEBUG__)
		else {
			if (!_dl_atexit)
				_dl_dprintf(_dl_debug_file, "%s: The address of atexit () is 0x0.\n", tpnt->libname);
#if 0
			if (!tpnt->dynamic_info[DT_FINI])
				_dl_dprintf(_dl_debug_file, "%s: Invalid .fini section.\n", tpnt->libname);
#endif
		}
#endif
	}
}

/*
 * This stub function is used by some debuggers.  The idea is that they
 * can set an internal breakpoint on it, so that we are notified when the
 * address mapping is changed in some way.
 */
void _dl_debug_state(void)
{
}

char *_dl_getenv(const char *symbol, char **envp)
{
	char *pnt;
	const char *pnt1;

	while ((pnt = *envp++)) {
		pnt1 = symbol;
		while (*pnt && *pnt == *pnt1)
			pnt1++, pnt++;
		if (!*pnt || *pnt != '=' || *pnt1)
			continue;
		return pnt + 1;
	}
	return 0;
}

void _dl_unsetenv(const char *symbol, char **envp)
{
	char *pnt;
	const char *pnt1;
	char **newenvp = envp;

	for (pnt = *envp; pnt; pnt = *++envp) {
		pnt1 = symbol;
		while (*pnt && *pnt == *pnt1)
			pnt1++, pnt++;
		if (!*pnt || *pnt != '=' || *pnt1)
			*newenvp++ = *envp;
	}
	*newenvp++ = *envp;
	return;
}

#include "hash.c"
#include "readelflib1.c"

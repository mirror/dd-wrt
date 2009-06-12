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


/* Various symbol table handling functions, including symbol lookup */

/*
 * This is the start of the linked list that describes all of the files present
 * in the system with pointers to all of the symbol, string, and hash tables, 
 * as well as all of the other good stuff in the binary.
 */

struct elf_resolve *_dl_loaded_modules = NULL;

/*
 * This is the list of modules that are loaded when the image is first
 * started.  As we add more via dlopen, they get added into other
 * chains.
 */
struct dyn_elf *_dl_symbol_tables = NULL;

/*
 * This is the list of modules that are loaded via dlopen.  We may need
 * to search these for RTLD_GLOBAL files.
 */
struct dyn_elf *_dl_handles = NULL;


/*
 * This is the hash function that is used by the ELF linker to generate
 * the hash table that each executable and library is required to
 * have.  We need it to decode the hash table.
 */

unsigned long _dl_elf_hash(const char *name)
{
	unsigned long hash = 0;
	unsigned long tmp;

	while (*name) {
		hash = (hash << 4) + *name++;
		if ((tmp = hash & 0xf0000000))
			hash ^= tmp >> 24;
		hash &= ~tmp;
	};
	return hash;
}

/*
 * Check to see if a library has already been added to the hash chain.
 */
struct elf_resolve *_dl_check_hashed_files(const char *libname)
{
	struct elf_resolve *tpnt;
	int len = _dl_strlen(libname);

	for (tpnt = _dl_loaded_modules; tpnt; tpnt = tpnt->next) {
		if (_dl_strncmp(tpnt->libname, libname, len) == 0 &&
			(tpnt->libname[len] == '\0' || tpnt->libname[len] == '.'))
			return tpnt;
	}

	return NULL;
}

/*
 * We call this function when we have just read an ELF library or executable.
 * We add the relevant info to the symbol chain, so that we can resolve all
 * externals properly.
 */

struct elf_resolve *_dl_add_elf_hash_table(const char *libname, 
	char *loadaddr, unsigned long *dynamic_info, unsigned long dynamic_addr, 
	unsigned long dynamic_size)
{
	unsigned long *hash_addr;
	struct elf_resolve *tpnt;
	int i;

	if (!_dl_loaded_modules) {
		tpnt = _dl_loaded_modules = 
		    (struct elf_resolve *) _dl_malloc(sizeof(struct elf_resolve));
		_dl_memset(tpnt, 0, sizeof(struct elf_resolve));
	} else {
		tpnt = _dl_loaded_modules;
		while (tpnt->next)
			tpnt = tpnt->next;
		tpnt->next = (struct elf_resolve *) _dl_malloc(sizeof(struct elf_resolve));
		_dl_memset(tpnt->next, 0, sizeof(struct elf_resolve));
		tpnt->next->prev = tpnt;
		tpnt = tpnt->next;
	};

	tpnt->next = NULL;
	tpnt->init_flag = 0;
	tpnt->libname = _dl_strdup(libname);
	tpnt->dynamic_addr = (ElfW(Dyn) *)dynamic_addr;
	tpnt->dynamic_size = dynamic_size;
	tpnt->libtype = loaded_file;

	if (dynamic_info[DT_HASH] != 0) {
		hash_addr = (unsigned long *) (intptr_t)(dynamic_info[DT_HASH] + loadaddr);
		tpnt->nbucket = *hash_addr++;
		tpnt->nchain = *hash_addr++;
		tpnt->elf_buckets = hash_addr;
		hash_addr += tpnt->nbucket;
		tpnt->chains = hash_addr;
	}
	tpnt->loadaddr = (ElfW(Addr))loadaddr;
	for (i = 0; i < 24; i++)
		tpnt->dynamic_info[i] = dynamic_info[i];
#ifdef __mips__
	{
		Elf32_Dyn *dpnt = (Elf32_Dyn *) dynamic_addr;

		while(dpnt->d_tag) {
			if (dpnt->d_tag == DT_MIPS_GOTSYM)
				tpnt->mips_gotsym = dpnt->d_un.d_val;
			if (dpnt->d_tag == DT_MIPS_LOCAL_GOTNO)
				tpnt->mips_local_gotno = dpnt->d_un.d_val;
			if (dpnt->d_tag == DT_MIPS_SYMTABNO)
				tpnt->mips_symtabno = dpnt->d_un.d_val;
			dpnt++;
		}
	}
#endif
	return tpnt;
}


/*
 * This function resolves externals, and this is either called when we process
 * relocations or when we call an entry in the PLT table for the first time.
 */

char *_dl_find_hash(const char *name, struct dyn_elf *rpnt1, 
	struct elf_resolve *f_tpnt, enum caller_type caller_type)
{
	struct elf_resolve *tpnt;
	int si;
	char *pnt;
	int pass;
	char *strtab;
	Elf32_Sym *symtab;
	unsigned long elf_hash_number, hn;
	char *weak_result;
	struct elf_resolve *first_def;
	struct dyn_elf *rpnt, first;
	char *data_result = 0;		/* nakao */

	weak_result = 0;
	elf_hash_number = _dl_elf_hash(name);

	/* A quick little hack to make sure that any symbol in the executable
	   will be preferred to one in a shared library.  This is necessary so
	   that any shared library data symbols referenced in the executable
	   will be seen at the same address by the executable, shared libraries
	   and dynamically loaded code. -Rob Ryan (robr@cmu.edu) */
	if (_dl_symbol_tables && !caller_type && rpnt1) {
		first = (*_dl_symbol_tables);
		first.next = rpnt1;
		rpnt1 = (&first);
	}

	/*
	 * The passes are so that we can first search the regular symbols
	 * for whatever module was specified, and then search anything
	 * loaded with RTLD_GLOBAL.  When pass is 1, it means we are just
	 * starting the first dlopened module, and anything above that
	 * is just the next one in the chain.
	 */
	for (pass = 0; (1 == 1); pass++) {

		/*
		 * If we are just starting to search for RTLD_GLOBAL, setup
		 * the pointer for the start of the search.
		 */
		if (pass == 1) {
			rpnt1 = _dl_handles;
		}

		/*
		 * Anything after this, we need to skip to the next module.
		 */
		else if (pass >= 2) {
			rpnt1 = rpnt1->next_handle;
		}

		/*
		 * Make sure we still have a module, and make sure that this
		 * module was loaded with RTLD_GLOBAL.
		 */
		if (pass != 0) {
			if (rpnt1 == NULL)
				break;
			if ((rpnt1->flags & RTLD_GLOBAL) == 0)
				continue;
		}

		for (rpnt = (rpnt1 ? rpnt1 : _dl_symbol_tables); rpnt; rpnt = rpnt->next) {
			tpnt = rpnt->dyn;

			/*
			 * The idea here is that if we are using dlsym, we want to
			 * first search the entire chain loaded from dlopen, and
			 * return a result from that if we found anything.  If this
			 * fails, then we continue the search into the stuff loaded
			 * when the image was activated.  For normal lookups, we start
			 * with rpnt == NULL, so we should never hit this.  
			 */
			if (tpnt->libtype == elf_executable && weak_result != 0) {
				break;
			}

			/*
			 * Avoid calling .urem here.
			 */
			do_rem(hn, elf_hash_number, tpnt->nbucket);
			symtab = (Elf32_Sym *) (intptr_t) (tpnt->dynamic_info[DT_SYMTAB] + tpnt->loadaddr);
			strtab = (char *) (tpnt->dynamic_info[DT_STRTAB] + tpnt->loadaddr);
			/*
			 * This crap is required because the first instance of a
			 * symbol on the chain will be used for all symbol references.
			 * Thus this instance must be resolved to an address that
			 * contains the actual function, 
			 */

			first_def = NULL;

			for (si = tpnt->elf_buckets[hn]; si; si = tpnt->chains[si]) {
				pnt = strtab + symtab[si].st_name;

				if (_dl_strcmp(pnt, name) == 0 &&
				    symtab[si].st_value != 0)
				{
				  if ((ELF32_ST_TYPE(symtab[si].st_info) == STT_FUNC ||
				       ELF32_ST_TYPE(symtab[si].st_info) == STT_NOTYPE ||
				       ELF32_ST_TYPE(symtab[si].st_info) == STT_OBJECT) &&
				      symtab[si].st_shndx != SHN_UNDEF) {

					/* Here we make sure that we find a module where the symbol is
					 * actually defined.
					 */

					if (f_tpnt) {
						if (!first_def)
							first_def = tpnt;
						if (first_def == f_tpnt
							&& symtab[si].st_shndx == 0)
							continue;
					}

					switch (ELF32_ST_BIND(symtab[si].st_info)) {
					case STB_GLOBAL:
						if (tpnt->libtype != elf_executable && 
							ELF32_ST_TYPE(symtab[si].st_info) 
							== STT_NOTYPE) 
						{	/* nakao */
							data_result = (char *)tpnt->loadaddr + 
							    symtab[si].st_value;	/* nakao */
							break;	/* nakao */
						} else	/* nakao */
							return (char*)tpnt->loadaddr + symtab[si].st_value;
					case STB_WEAK:
						if (!weak_result)
							weak_result = (char *)tpnt->loadaddr + symtab[si].st_value;
						break;
					default:	/* Do local symbols need to be examined? */
						break;
					}
				  }
#ifndef __mips__
				  /*
				   * References to the address of a function from an executable file and
				   * the shared objects associated with it might not resolve to the same
				   * value. To allow comparisons of function addresses we must resolve
				   * to the address of the plt entry of the executable instead of the
				   * real function address.
				   * see "TIS ELF Specification Version 1.2, Book 3, A-11 (Function
				   * Adresses) 
				   */				 
				  if (resolver != caller_type &&
				      NULL==f_tpnt && /*trick: don't  handle R_??_JMP_SLOT reloc type*/
				      tpnt->libtype == elf_executable &&
				      ELF32_ST_TYPE(symtab[si].st_info) == STT_FUNC &&
				      symtab[si].st_shndx == SHN_UNDEF)
				  {
				      return (char*)symtab[si].st_value;
				  }
#endif
				}
			}
		}
	}
	if (data_result)
		return data_result;		/* nakao */
	return weak_result;
}

/* vi: set sw=4 ts=4: */
/* Program to load an ELF binary on a linux system, and run it
 * after resolving ELF shared library symbols
 *
 * Copyright (c) 1994-2000 Eric Youngdale, Peter MacDonald, 
 *				David Engel, Hongjiu Lu and Mitch D'Souza
 * Copyright (C) 2001-2003, Erik Andersen
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


/* This file contains the helper routines to load an ELF sharable
   library into memory and add the symbol table info to the chain. */

#ifdef USE_CACHE

static caddr_t _dl_cache_addr = NULL;
static size_t _dl_cache_size = 0;

int _dl_map_cache(void)
{
	int fd;
	struct stat st;
	header_t *header;
	libentry_t *libent;
	int i, strtabsize;

	if (_dl_cache_addr == (caddr_t) - 1)
		return -1;
	else if (_dl_cache_addr != NULL)
		return 0;

	if (_dl_stat(LDSO_CACHE, &st)
		|| (fd = _dl_open(LDSO_CACHE, O_RDONLY)) < 0) {
		_dl_dprintf(2, "%s: can't open cache '%s'\n", _dl_progname, LDSO_CACHE);
		_dl_cache_addr = (caddr_t) - 1;	/* so we won't try again */
		return -1;
	}

	_dl_cache_size = st.st_size;
	_dl_cache_addr = (caddr_t) _dl_mmap(0, _dl_cache_size, PROT_READ, MAP_SHARED, fd, 0);
	_dl_close(fd);
	if (_dl_mmap_check_error(_dl_cache_addr)) {
		_dl_dprintf(2, "%s: can't map cache '%s'\n", 
			_dl_progname, LDSO_CACHE);
		return -1;
	}

	header = (header_t *) _dl_cache_addr;

	if (_dl_cache_size < sizeof(header_t) ||
		_dl_memcmp(header->magic, LDSO_CACHE_MAGIC, LDSO_CACHE_MAGIC_LEN)
		|| _dl_memcmp(header->version, LDSO_CACHE_VER, LDSO_CACHE_VER_LEN)
		|| _dl_cache_size <
		(sizeof(header_t) + header->nlibs * sizeof(libentry_t))
		|| _dl_cache_addr[_dl_cache_size - 1] != '\0') 
	{
		_dl_dprintf(2, "%s: cache '%s' is corrupt\n", _dl_progname, 
			LDSO_CACHE);
		goto fail;
	}

	strtabsize = _dl_cache_size - sizeof(header_t) -
		header->nlibs * sizeof(libentry_t);
	libent = (libentry_t *) & header[1];

	for (i = 0; i < header->nlibs; i++) {
		if (libent[i].sooffset >= strtabsize || 
			libent[i].liboffset >= strtabsize) 
		{
			_dl_dprintf(2, "%s: cache '%s' is corrupt\n", _dl_progname, LDSO_CACHE);
			goto fail;
		}
	}

	return 0;

  fail:
	_dl_munmap(_dl_cache_addr, _dl_cache_size);
	_dl_cache_addr = (caddr_t) - 1;
	return -1;
}

int _dl_unmap_cache(void)
{
	if (_dl_cache_addr == NULL || _dl_cache_addr == (caddr_t) - 1)
		return -1;

#if 1
	_dl_munmap(_dl_cache_addr, _dl_cache_size);
	_dl_cache_addr = NULL;
#endif

	return 0;
}

#endif

/* This function's behavior must exactly match that 
 * in uClibc/ldso/util/ldd.c */
static struct elf_resolve * 
search_for_named_library(const char *name, int secure, const char *path_list,
	struct dyn_elf **rpnt)
{
	int i, count = 1;
	char *path, *path_n;
	char mylibname[2050];
	struct elf_resolve *tpnt1;
	
	if (path_list==NULL)
		return NULL;

	/* We need a writable copy of this string */
	path = _dl_strdup(path_list);
	if (!path) {
		_dl_dprintf(2, "Out of memory!\n");
		_dl_exit(0);
	}
	

	/* Unlike ldd.c, don't bother to eliminate double //s */


	/* Replace colons with zeros in path_list and count them */
	for(i=_dl_strlen(path); i > 0; i--) {
		if (path[i]==':') {
			path[i]=0;
			count++;
		}
	}

	path_n = path;
	for (i = 0; i < count; i++) {
		_dl_strcpy(mylibname, path_n); 
		_dl_strcat(mylibname, "/"); 
		_dl_strcat(mylibname, name);
		if ((tpnt1 = _dl_load_elf_shared_library(secure, rpnt, mylibname)) != NULL)
		{
			return tpnt1;
		}
		path_n += (_dl_strlen(path_n) + 1);
	}
	return NULL;
}

/* Check if the named library is already loaded... */
struct elf_resolve *_dl_check_if_named_library_is_loaded(const char *full_libname)
{
	const char *pnt, *pnt1;
	struct elf_resolve *tpnt1;
	const char *libname, *libname2;
	static const char *libc = "libc.so.";
	static const char *aborted_wrong_lib = "%s: aborted attempt to load %s!\n";

	pnt = libname = full_libname;

#if defined (__SUPPORT_LD_DEBUG__)
	if(_dl_debug) 
		_dl_dprintf(_dl_debug_file, "Checking if '%s' is already loaded\n", full_libname);
#endif
	/* quick hack to ensure mylibname buffer doesn't overflow.  don't 
	   allow full_libname or any directory to be longer than 1024. */
	if (_dl_strlen(full_libname) > 1024)
		return NULL;

	/* Skip over any initial initial './' and '/' stuff to 
	 * get the short form libname with no path garbage */ 
	pnt1 = _dl_strrchr(pnt, '/');
	if (pnt1) {
		libname = pnt1 + 1;
	}

	/* Make sure they are not trying to load the wrong C library!
	 * This sometimes happens esp with shared libraries when the
	 * library path is somehow wrong! */
#define isdigit(c)  (c >= '0' && c <= '9')
	if ((_dl_strncmp(libname, libc, 8) == 0) &&  _dl_strlen(libname) >=8 &&
			isdigit(libname[8]))
	{
		/* Abort attempts to load glibc, libc5, etc */
		if ( libname[8]!='0') {
			if (!_dl_trace_loaded_objects) {
				_dl_dprintf(2, aborted_wrong_lib, libname, _dl_progname);
				_dl_exit(1);
			}
			return NULL;
		}
	}

	/* Critical step!  Weed out duplicates early to avoid
	 * function aliasing, which wastes memory, and causes
	 * really bad things to happen with weaks and globals. */
	for (tpnt1 = _dl_loaded_modules; tpnt1; tpnt1 = tpnt1->next) {

		/* Skip over any initial initial './' and '/' stuff to 
		 * get the short form libname with no path garbage */ 
		libname2 = tpnt1->libname;
		pnt1 = _dl_strrchr(libname2, '/');
		if (pnt1) {
			libname2 = pnt1 + 1;
		}

		if (_dl_strcmp(libname2, libname) == 0) {
			/* Well, that was certainly easy */
			return tpnt1;
		}
	}

	return NULL;
}
	


/*
 * Used to return error codes back to dlopen et. al.
 */

unsigned long _dl_error_number;
unsigned long _dl_internal_error_number;
extern char *_dl_ldsopath;

struct elf_resolve *_dl_load_shared_library(int secure, struct dyn_elf **rpnt,
	struct elf_resolve *tpnt, char *full_libname)
{
	char *pnt, *pnt1;
	struct elf_resolve *tpnt1;
	char *libname;

	_dl_internal_error_number = 0;
	libname = full_libname;

	/* quick hack to ensure mylibname buffer doesn't overflow.  don't 
	   allow full_libname or any directory to be longer than 1024. */
	if (_dl_strlen(full_libname) > 1024)
		goto goof;

	/* Skip over any initial initial './' and '/' stuff to 
	 * get the short form libname with no path garbage */ 
	pnt1 = _dl_strrchr(libname, '/');
	if (pnt1) {
		libname = pnt1 + 1;
	}

	/* Critical step!  Weed out duplicates early to avoid
	 * function aliasing, which wastes memory, and causes
	 * really bad things to happen with weaks and globals. */
	if ((tpnt1=_dl_check_if_named_library_is_loaded(libname))!=NULL)
		return tpnt1;

#if defined (__SUPPORT_LD_DEBUG__)
	if(_dl_debug) _dl_dprintf(_dl_debug_file, "\tfind library='%s'; searching\n", libname);
#endif
	/* If the filename has any '/', try it straight and leave it at that.
	   For IBCS2 compatibility under linux, we substitute the string 
	   /usr/i486-sysv4/lib for /usr/lib in library names. */

	if (libname != full_libname) {
#if defined (__SUPPORT_LD_DEBUG__)
		if(_dl_debug) _dl_dprintf(_dl_debug_file, "\ttrying file='%s'\n", full_libname);
#endif
		tpnt1 = _dl_load_elf_shared_library(secure, rpnt, full_libname);
		if (tpnt1) {
			return tpnt1;
		}
		//goto goof;
	}

	/*
	 * The ABI specifies that RPATH is searched before LD_*_PATH or
	 * the default path of /usr/lib.  Check in rpath directories.
	 */
	for (tpnt = _dl_loaded_modules; tpnt; tpnt = tpnt->next) {
		if (tpnt->libtype == elf_executable) {
			pnt = (char *) tpnt->dynamic_info[DT_RPATH];
			if (pnt) {
				pnt += (unsigned long) tpnt->loadaddr + tpnt->dynamic_info[DT_STRTAB];
#if defined (__SUPPORT_LD_DEBUG__)
				if(_dl_debug) _dl_dprintf(_dl_debug_file, "\tsearching RPATH='%s'\n", pnt);
#endif
				if ((tpnt1 = search_for_named_library(libname, secure, pnt, rpnt)) != NULL) 
				{
				    return tpnt1;
				}
			}
		}
	}

	/* Check in LD_{ELF_}LIBRARY_PATH, if specified and allowed */
	if (_dl_library_path) {
#if defined (__SUPPORT_LD_DEBUG__)
		if(_dl_debug) _dl_dprintf(_dl_debug_file, "\tsearching LD_LIBRARY_PATH='%s'\n", _dl_library_path);
#endif
	    if ((tpnt1 = search_for_named_library(libname, secure, _dl_library_path, rpnt)) != NULL) 
	    {
		return tpnt1;
	    }
	}

	/*
	 * Where should the cache be searched?  There is no such concept in the
	 * ABI, so we have some flexibility here.  For now, search it before
	 * the hard coded paths that follow (i.e before /lib and /usr/lib).
	 */
#ifdef USE_CACHE
	if (_dl_cache_addr != NULL && _dl_cache_addr != (caddr_t) - 1) {
		int i;
		header_t *header = (header_t *) _dl_cache_addr;
		libentry_t *libent = (libentry_t *) & header[1];
		char *strs = (char *) &libent[header->nlibs];

#if defined (__SUPPORT_LD_DEBUG__)
		if(_dl_debug) _dl_dprintf(_dl_debug_file, "\tsearching cache='%s'\n", LDSO_CACHE);
#endif
		for (i = 0; i < header->nlibs; i++) {
			if ((libent[i].flags == LIB_ELF ||
				 libent[i].flags == LIB_ELF_LIBC5) &&
				_dl_strcmp(libname, strs + libent[i].sooffset) == 0 &&
				(tpnt1 = _dl_load_elf_shared_library(secure, 
				     rpnt, strs + libent[i].liboffset)))
				return tpnt1;
		}
	}
#endif

	/* Look for libraries wherever the shared library loader
	 * was installed */
#if defined (__SUPPORT_LD_DEBUG__)
		if(_dl_debug) _dl_dprintf(_dl_debug_file, "\tsearching ldso dir='%s'\n", _dl_ldsopath);
#endif
	if ((tpnt1 = search_for_named_library(libname, secure, _dl_ldsopath, rpnt)) != NULL) 
	{
	    return tpnt1;
	}


	/* Lastly, search the standard list of paths for the library.
	   This list must exactly match the list in uClibc/ldso/util/ldd.c */
#if defined (__SUPPORT_LD_DEBUG__)
	if(_dl_debug) _dl_dprintf(_dl_debug_file, "\tsearching full lib path list\n");
#endif
	if ((tpnt1 = search_for_named_library(libname, secure, 
			UCLIBC_RUNTIME_PREFIX "usr/X11R6/lib:"
			UCLIBC_RUNTIME_PREFIX "usr/lib:"
			UCLIBC_RUNTIME_PREFIX "lib:"
			"/usr/lib:"
			"/lib", rpnt)
		    ) != NULL) 
	{
	    return tpnt1;
	}

goof:
	/* Well, we shot our wad on that one.  All we can do now is punt */
	if (_dl_internal_error_number)
		_dl_error_number = _dl_internal_error_number;
	else
		_dl_error_number = LD_ERROR_NOFILE;
#if defined (__SUPPORT_LD_DEBUG__)
	if(_dl_debug) _dl_dprintf(2, "Bummer: could not find '%s'!\n", libname);
#endif
	return NULL;
}


/*
 * Read one ELF library into memory, mmap it into the correct locations and
 * add the symbol info to the symbol chain.  Perform any relocations that
 * are required.
 */

struct elf_resolve *_dl_load_elf_shared_library(int secure,
	struct dyn_elf **rpnt, char *libname)
{
	ElfW(Ehdr) *epnt;
	unsigned long dynamic_addr = 0;
	unsigned long dynamic_size = 0;
	Elf32_Dyn *dpnt;
	struct elf_resolve *tpnt;
	ElfW(Phdr) *ppnt;
	char *status, *header;
	unsigned long dynamic_info[24];
	unsigned long *lpnt;
	unsigned long libaddr;
	unsigned long minvma = 0xffffffff, maxvma = 0;
	int i, flags, piclib, infile;

	/* If this file is already loaded, skip this step */
	tpnt = _dl_check_hashed_files(libname);
	if (tpnt) {
		if (*rpnt) {
			(*rpnt)->next = (struct dyn_elf *) _dl_malloc(sizeof(struct dyn_elf));
			_dl_memset((*rpnt)->next, 0, sizeof(struct dyn_elf));
			(*rpnt)->next->prev = (*rpnt);
			*rpnt = (*rpnt)->next;
			(*rpnt)->dyn = tpnt;
			tpnt->symbol_scope = _dl_symbol_tables;
		}
		tpnt->usage_count++;
		tpnt->libtype = elf_lib;
#if defined (__SUPPORT_LD_DEBUG__)
		if(_dl_debug) _dl_dprintf(2, "file='%s';  already loaded\n", libname);
#endif
		return tpnt;
	}

	/* If we are in secure mode (i.e. a setu/gid binary using LD_PRELOAD),
	   we don't load the library if it isn't setuid. */

	if (secure) {
		struct stat st;

		if (_dl_stat(libname, &st) || !(st.st_mode & S_ISUID))
			return NULL;
	}

	libaddr = 0;
	infile = _dl_open(libname, O_RDONLY);
	if (infile < 0) {
#if 0
		/*
		 * NO!  When we open shared libraries we may search several paths.
		 * it is inappropriate to generate an error here.
		 */
		_dl_dprintf(2, "%s: can't open '%s'\n", _dl_progname, libname);
#endif
		_dl_internal_error_number = LD_ERROR_NOFILE;
		return NULL;
	}

	 header = _dl_mmap((void *) 0, PAGE_SIZE, PROT_READ | PROT_WRITE,
	 	MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (_dl_mmap_check_error(header)) {
		_dl_dprintf(2, "%s: can't map '%s'\n", _dl_progname, libname);
		_dl_internal_error_number = LD_ERROR_MMAP_FAILED;
		_dl_close(infile);
		return NULL;
	};
        
	_dl_read(infile, header, PAGE_SIZE);
	epnt = (ElfW(Ehdr) *) (intptr_t) header;
	if (epnt->e_ident[0] != 0x7f ||
		epnt->e_ident[1] != 'E' || 
		epnt->e_ident[2] != 'L' || 
		epnt->e_ident[3] != 'F') 
	{
		_dl_dprintf(2, "%s: '%s' is not an ELF file\n", _dl_progname,
					 libname);
		_dl_internal_error_number = LD_ERROR_NOTELF;
		_dl_close(infile);
		_dl_munmap(header, PAGE_SIZE);
		return NULL;
	};

	if ((epnt->e_type != ET_DYN) || (epnt->e_machine != MAGIC1 
#ifdef MAGIC2
		    && epnt->e_machine != MAGIC2
#endif
		)) 
	{
		_dl_internal_error_number = 
		    (epnt->e_type != ET_DYN ? LD_ERROR_NOTDYN : LD_ERROR_NOTMAGIC);
		_dl_dprintf(2, "%s: '%s' is not an ELF executable for " ELF_TARGET 
			"\n", _dl_progname, libname);
		_dl_close(infile);
		_dl_munmap(header, PAGE_SIZE);
		return NULL;
	};

	ppnt = (ElfW(Phdr) *)(intptr_t) & header[epnt->e_phoff];

	piclib = 1;
	for (i = 0; i < epnt->e_phnum; i++) {

		if (ppnt->p_type == PT_DYNAMIC) {
			if (dynamic_addr)
				_dl_dprintf(2, "%s: '%s' has more than one dynamic section\n", 
					_dl_progname, libname);
			dynamic_addr = ppnt->p_vaddr;
			dynamic_size = ppnt->p_filesz;
		};

		if (ppnt->p_type == PT_LOAD) {
			/* See if this is a PIC library. */
			if (i == 0 && ppnt->p_vaddr > 0x1000000) {
				piclib = 0;
				minvma = ppnt->p_vaddr;
			}
			if (piclib && ppnt->p_vaddr < minvma) {
				minvma = ppnt->p_vaddr;
			}
			if (((unsigned long) ppnt->p_vaddr + ppnt->p_memsz) > maxvma) {
				maxvma = ppnt->p_vaddr + ppnt->p_memsz;
			}
		}
		ppnt++;
	};

	maxvma = (maxvma + ADDR_ALIGN) & ~ADDR_ALIGN;
	minvma = minvma & ~0xffffU;

	flags = MAP_PRIVATE /*| MAP_DENYWRITE */ ;
	if (!piclib)
		flags |= MAP_FIXED;

	status = (char *) _dl_mmap((char *) (piclib ? 0 : minvma), 
		maxvma - minvma, PROT_NONE, flags | MAP_ANONYMOUS, -1, 0);
	if (_dl_mmap_check_error(status)) {
		_dl_dprintf(2, "%s: can't map %s\n", _dl_progname, libname);
		_dl_internal_error_number = LD_ERROR_MMAP_FAILED;
		_dl_close(infile);
		_dl_munmap(header, PAGE_SIZE);
		return NULL;
	};
	libaddr = (unsigned long) status;
	flags |= MAP_FIXED;

	/* Get the memory to store the library */
	ppnt = (ElfW(Phdr) *)(intptr_t) & header[epnt->e_phoff];

	for (i = 0; i < epnt->e_phnum; i++) {
		if (ppnt->p_type == PT_LOAD) {

			/* See if this is a PIC library. */
			if (i == 0 && ppnt->p_vaddr > 0x1000000) {
				piclib = 0;
				/* flags |= MAP_FIXED; */
			}



			if (ppnt->p_flags & PF_W) {
				unsigned long map_size;
				char *cpnt;

				status = (char *) _dl_mmap((char *) ((piclib ? libaddr : 0) + 
					(ppnt->p_vaddr & PAGE_ALIGN)), (ppnt->p_vaddr & ADDR_ALIGN) 
					+ ppnt->p_filesz, LXFLAGS(ppnt->p_flags), flags, infile, 
					ppnt->p_offset & OFFS_ALIGN);

				if (_dl_mmap_check_error(status)) {
					_dl_dprintf(2, "%s: can't map '%s'\n", 
						_dl_progname, libname);
					_dl_internal_error_number = LD_ERROR_MMAP_FAILED;
					_dl_munmap((char *) libaddr, maxvma - minvma);
					_dl_close(infile);
					_dl_munmap(header, PAGE_SIZE);
					return NULL;
				};

				/* Pad the last page with zeroes. */
				cpnt = (char *) (status + (ppnt->p_vaddr & ADDR_ALIGN) +
							  ppnt->p_filesz);
				while (((unsigned long) cpnt) & ADDR_ALIGN)
					*cpnt++ = 0;

				/* I am not quite sure if this is completely
				 * correct to do or not, but the basic way that
				 * we handle bss segments is that we mmap
				 * /dev/zero if there are any pages left over
				 * that are not mapped as part of the file */

				map_size = (ppnt->p_vaddr + ppnt->p_filesz + ADDR_ALIGN) & PAGE_ALIGN;

				if (map_size < ppnt->p_vaddr + ppnt->p_memsz)
					status = (char *) _dl_mmap((char *) map_size + 
						(piclib ? libaddr : 0), 
						ppnt->p_vaddr + ppnt->p_memsz - map_size, 
						LXFLAGS(ppnt->p_flags), flags | MAP_ANONYMOUS, -1, 0);
			} else
				status = (char *) _dl_mmap((char *) (ppnt->p_vaddr & PAGE_ALIGN) 
					+ (piclib ? libaddr : 0), (ppnt->p_vaddr & ADDR_ALIGN) + 
					ppnt->p_filesz, LXFLAGS(ppnt->p_flags), flags, 
					infile, ppnt->p_offset & OFFS_ALIGN);
			if (_dl_mmap_check_error(status)) {
				_dl_dprintf(2, "%s: can't map '%s'\n", _dl_progname, libname);
				_dl_internal_error_number = LD_ERROR_MMAP_FAILED;
				_dl_munmap((char *) libaddr, maxvma - minvma);
				_dl_close(infile);
				_dl_munmap(header, PAGE_SIZE);
				return NULL;
			};

			/* if(libaddr == 0 && piclib) {
			   libaddr = (unsigned long) status;
			   flags |= MAP_FIXED;
			   }; */
		};
		ppnt++;
	};
	_dl_close(infile);

	/* For a non-PIC library, the addresses are all absolute */
	if (piclib) {
		dynamic_addr += (unsigned long) libaddr;
	}

	/* 
	 * OK, the ELF library is now loaded into VM in the correct locations
	 * The next step is to go through and do the dynamic linking (if needed).
	 */

	/* Start by scanning the dynamic section to get all of the pointers */

	if (!dynamic_addr) {
		_dl_internal_error_number = LD_ERROR_NODYNAMIC;
		_dl_dprintf(2, "%s: '%s' is missing a dynamic section\n", 
			_dl_progname, libname);
			_dl_munmap(header, PAGE_SIZE);
		return NULL;
	}

	dpnt = (Elf32_Dyn *) dynamic_addr;

	dynamic_size = dynamic_size / sizeof(Elf32_Dyn);
	_dl_memset(dynamic_info, 0, sizeof(dynamic_info));

#if defined(__mips__)
	{
		
		int indx = 1;
		Elf32_Dyn *dpnt = (Elf32_Dyn *) dynamic_addr;

		while(dpnt->d_tag) {
			dpnt++;
			indx++;
		}
		dynamic_size = indx;
	}
#endif

	{
		unsigned long indx;

		for (indx = 0; indx < dynamic_size; indx++) 
		{
			if (dpnt->d_tag > DT_JMPREL) {
				dpnt++;
				continue;
			}
			dynamic_info[dpnt->d_tag] = dpnt->d_un.d_val;
			if (dpnt->d_tag == DT_TEXTREL)
				dynamic_info[DT_TEXTREL] = 1;
			dpnt++;
		};
	}

	/* If the TEXTREL is set, this means that we need to make the pages
	   writable before we perform relocations.  Do this now. They get set
	   back again later. */

	if (dynamic_info[DT_TEXTREL]) {
#ifndef FORCE_SHAREABLE_TEXT_SEGMENTS
		ppnt = (ElfW(Phdr) *)(intptr_t) & header[epnt->e_phoff];
		for (i = 0; i < epnt->e_phnum; i++, ppnt++) {
			if (ppnt->p_type == PT_LOAD && !(ppnt->p_flags & PF_W))
				_dl_mprotect((void *) ((piclib ? libaddr : 0) + 
					    (ppnt->p_vaddr & PAGE_ALIGN)), 
					(ppnt->p_vaddr & ADDR_ALIGN) + (unsigned long) ppnt->p_filesz, 
					PROT_READ | PROT_WRITE | PROT_EXEC);
		}
#else
		_dl_dprintf(_dl_debug_file, "Can't modify %s's text section. Use GCC option -fPIC for shared objects, please.\n",libname);
		_dl_exit(1);
#endif		
	}

	tpnt = _dl_add_elf_hash_table(libname, (char *) libaddr, dynamic_info, 
		dynamic_addr, dynamic_size);

	tpnt->ppnt = (ElfW(Phdr) *)(intptr_t) (tpnt->loadaddr + epnt->e_phoff);
	tpnt->n_phent = epnt->e_phnum;

	/*
	 * Add this object into the symbol chain
	 */
	if (*rpnt) {
		(*rpnt)->next = (struct dyn_elf *) _dl_malloc(sizeof(struct dyn_elf));
		_dl_memset((*rpnt)->next, 0, sizeof(struct dyn_elf));
		(*rpnt)->next->prev = (*rpnt);
		*rpnt = (*rpnt)->next;
		(*rpnt)->dyn = tpnt;
		tpnt->symbol_scope = _dl_symbol_tables;
	}
	tpnt->usage_count++;
	tpnt->libtype = elf_lib;

	/*
	 * OK, the next thing we need to do is to insert the dynamic linker into
	 * the proper entry in the GOT so that the PLT symbols can be properly
	 * resolved. 
	 */

	lpnt = (unsigned long *) dynamic_info[DT_PLTGOT];

	if (lpnt) {
		lpnt = (unsigned long *) (dynamic_info[DT_PLTGOT] +
			((int) libaddr));
		INIT_GOT(lpnt, tpnt);
	};

#if defined (__SUPPORT_LD_DEBUG__)
	if(_dl_debug) {
		_dl_dprintf(2, "\n\tfile='%s';  generating link map\n", libname);
		_dl_dprintf(2, "\t\tdynamic: %x  base: %x   size: %x\n", 
				dynamic_addr, libaddr, dynamic_size);
		_dl_dprintf(2, "\t\t  entry: %x  phdr: %x  phnum: %d\n\n", 
				epnt->e_entry + libaddr, tpnt->ppnt, tpnt->n_phent);

	}
#endif
	_dl_munmap(header, PAGE_SIZE);

	return tpnt;
}

/* Ugly, ugly.  Some versions of the SVr4 linker fail to generate COPY
   relocations for global variables that are present both in the image and
   the shared library.  Go through and do it manually.  If the images
   are guaranteed to be generated by a trustworthy linker, then this
   step can be skipped. */

int _dl_copy_fixups(struct dyn_elf *rpnt)
{
	int goof = 0;
	struct elf_resolve *tpnt;

	if (rpnt->next)
		goof += _dl_copy_fixups(rpnt->next);
	else
		return 0;

	tpnt = rpnt->dyn;

	if (tpnt->init_flag & COPY_RELOCS_DONE)
		return goof;
	tpnt->init_flag |= COPY_RELOCS_DONE;

#if defined (__SUPPORT_LD_DEBUG__)
	if(_dl_debug) _dl_dprintf(_dl_debug_file,"\nrelocation copy fixups: %s", tpnt->libname);	
#endif    

#ifdef ELF_USES_RELOCA
	goof += _dl_parse_copy_information(rpnt, 
		tpnt->dynamic_info[DT_RELA], tpnt->dynamic_info[DT_RELASZ], 0);

#else
	goof += _dl_parse_copy_information(rpnt, tpnt->dynamic_info[DT_REL], 
		tpnt->dynamic_info[DT_RELSZ], 0);

#endif
#if defined (__SUPPORT_LD_DEBUG__)
	if(_dl_debug) _dl_dprintf(_dl_debug_file,"\nrelocation copy fixups: %s; finished\n\n", tpnt->libname);	
#endif    
	return goof;
}

/* Minimal printf which handles only %s, %d, and %x */
void _dl_dprintf(int fd, const char *fmt, ...)
{
	int num;
	va_list args;
	char *start, *ptr, *string;
	static char *buf;

	buf = _dl_mmap((void *) 0, PAGE_SIZE, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (_dl_mmap_check_error(buf)) {
			_dl_write(fd, "mmap of a spare page failed!\n", 29); 
			_dl_exit(20);
	}

	start = ptr = buf;

	if (!fmt)
		return;

	if (_dl_strlen(fmt) >= (PAGE_SIZE - 1)) {
		_dl_write(fd, "overflow\n", 11);
		_dl_exit(20);
	}

	_dl_strcpy(buf, fmt);
	va_start(args, fmt);

	while (start) {
		while (*ptr != '%' && *ptr) {
			ptr++;
		}

		if (*ptr == '%') {
			*ptr++ = '\0';
			_dl_write(fd, start, _dl_strlen(start));

			switch (*ptr++) {
			case 's':
				string = va_arg(args, char *);

				if (!string)
					_dl_write(fd, "(null)", 6);
				else
					_dl_write(fd, string, _dl_strlen(string));
				break;

			case 'i':
			case 'd':
			{
				char tmp[22];
				num = va_arg(args, int);

				string = _dl_simple_ltoa(tmp, num);
				_dl_write(fd, string, _dl_strlen(string));
				break;
			}
			case 'x':
			case 'X':
			{
				char tmp[22];
				num = va_arg(args, int);

				string = _dl_simple_ltoahex(tmp, num);
				_dl_write(fd, string, _dl_strlen(string));
				break;
			}
			default:
				_dl_write(fd, "(null)", 6);
				break;
			}

			start = ptr;
		} else {
			_dl_write(fd, start, _dl_strlen(start));
			start = NULL;
		}
	}
	_dl_munmap(buf, PAGE_SIZE);
	return;
}

char *_dl_strdup(const char *string)
{
	char *retval;
	int len;

	len = _dl_strlen(string);
	retval = _dl_malloc(len + 1);
	_dl_strcpy(retval, string);
	return retval;
}

void *(*_dl_malloc_function) (size_t size) = NULL;
void *_dl_malloc(int size)
{
	void *retval;

#if 0
#ifdef __SUPPORT_LD_DEBUG_EARLY__
	_dl_dprintf(2, "malloc: request for %d bytes\n", size);
#endif
#endif

	if (_dl_malloc_function)
		return (*_dl_malloc_function) (size);

	if (_dl_malloc_addr - _dl_mmap_zero + size > PAGE_SIZE) {
#ifdef __SUPPORT_LD_DEBUG_EARLY__
		_dl_dprintf(2, "malloc: mmapping more memory\n");
#endif
		_dl_mmap_zero = _dl_malloc_addr = _dl_mmap((void *) 0, size, 
				PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
		if (_dl_mmap_check_error(_dl_mmap_zero)) {
			_dl_dprintf(2, "%s: mmap of a spare page failed!\n", _dl_progname);
			_dl_exit(20);
		}
	}
	retval = _dl_malloc_addr;
	_dl_malloc_addr += size;

	/*
	 * Align memory to 4 byte boundary.  Some platforms require this, others
	 * simply get better performance.
	 */
	_dl_malloc_addr = (char *) (((unsigned long) _dl_malloc_addr + 3) & ~(3));
	return retval;
}

int _dl_fixup(struct elf_resolve *tpnt, int flag)
{
	int goof = 0;

	if (tpnt->next)
		goof += _dl_fixup(tpnt->next, flag);
#if defined (__SUPPORT_LD_DEBUG__)
	if(_dl_debug) _dl_dprintf(_dl_debug_file,"\nrelocation processing: %s", tpnt->libname);	
#endif    
	
	if (tpnt->dynamic_info[DT_REL]) {
#ifdef ELF_USES_RELOCA
#if defined (__SUPPORT_LD_DEBUG__)
		if(_dl_debug) _dl_dprintf(2, "%s: can't handle REL relocation records\n", _dl_progname);
#endif    
		goof++;
		return goof;
#else
		if (tpnt->init_flag & RELOCS_DONE)
			return goof;
		tpnt->init_flag |= RELOCS_DONE;
		goof += _dl_parse_relocation_information(tpnt, 
				tpnt->dynamic_info[DT_REL], 
				tpnt->dynamic_info[DT_RELSZ], 0);
#endif
	}
	if (tpnt->dynamic_info[DT_RELA]) {
#ifndef ELF_USES_RELOCA
#if defined (__SUPPORT_LD_DEBUG__)
		if(_dl_debug) _dl_dprintf(2, "%s: can't handle RELA relocation records\n", _dl_progname);
#endif    
		goof++;
		return goof;
#else
		if (tpnt->init_flag & RELOCS_DONE)
			return goof;
		tpnt->init_flag |= RELOCS_DONE;
		goof += _dl_parse_relocation_information(tpnt, 
				tpnt->dynamic_info[DT_RELA], 
				tpnt->dynamic_info[DT_RELASZ], 0);
#endif
	}
	if (tpnt->dynamic_info[DT_JMPREL]) {
		if (tpnt->init_flag & JMP_RELOCS_DONE)
			return goof;
		tpnt->init_flag |= JMP_RELOCS_DONE;
		if (flag & RTLD_LAZY) {
			_dl_parse_lazy_relocation_information(tpnt, 
					tpnt->dynamic_info[DT_JMPREL], 
					tpnt->dynamic_info [DT_PLTRELSZ], 0);
		} else {
			goof += _dl_parse_relocation_information(tpnt, 
					tpnt->dynamic_info[DT_JMPREL], 
					tpnt->dynamic_info[DT_PLTRELSZ], 0);
		}
	}
#if defined (__SUPPORT_LD_DEBUG__)
	if(_dl_debug) {
		_dl_dprintf(_dl_debug_file,"\nrelocation processing: %s", tpnt->libname);     
		_dl_dprintf(_dl_debug_file,"; finished\n\n");
	}
#endif    
	return goof;
}



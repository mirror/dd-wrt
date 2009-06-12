/* vi: set sw=4 ts=4: */
/* ARM ELF shared library loader suppport
 *
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

#if defined (__SUPPORT_LD_DEBUG__)
static const char *_dl_reltypes_tab[] =
{
  [0]	"R_ARM_NONE",	    "R_ARM_PC24",	"R_ARM_ABS32",		"R_ARM_REL32",
  [4]	"R_ARM_PC13",	    "R_ARM_ABS16",	"R_ARM_ABS12",		"R_ARM_THM_ABS5",
  [8]	"R_ARM_ABS8",		"R_ARM_SBREL32","R_ARM_THM_PC22",	"R_ARM_THM_PC8",
  [12]	"R_ARM_AMP_VCALL9",	"R_ARM_SWI24",	"R_ARM_THM_SWI8",	"R_ARM_XPC25",
  [16]	"R_ARM_THM_XPC22",
  [20]	"R_ARM_COPY",		"R_ARM_GLOB_DAT","R_ARM_JUMP_SLOT",	"R_ARM_RELATIVE",
  [24]	"R_ARM_GOTOFF",		"R_ARM_GOTPC",	 "R_ARM_GOT32",		"R_ARM_PLT32",
  [32]	"R_ARM_ALU_PCREL_7_0","R_ARM_ALU_PCREL_15_8","R_ARM_ALU_PCREL_23_15","R_ARM_LDR_SBREL_11_0",
  [36]	"R_ARM_ALU_SBREL_19_12","R_ARM_ALU_SBREL_27_20",
  [100]	"R_ARM_GNU_VTENTRY","R_ARM_GNU_VTINHERIT","R_ARM_THM_PC11","R_ARM_THM_PC9",
  [249] "R_ARM_RXPC25", "R_ARM_RSBREL32", "R_ARM_THM_RPC22", "R_ARM_RREL32",
  [253] "R_ARM_RABS22", "R_ARM_RPC24", "R_ARM_RBASE",
};

static const char *
_dl_reltypes(int type)
{
  static char buf[22];  
  const char *str;
  
  if (type >= (sizeof (_dl_reltypes_tab)/sizeof(_dl_reltypes_tab[0])) ||
      NULL == (str = _dl_reltypes_tab[type]))
  {
    str =_dl_simple_ltoa( buf, (unsigned long)(type));
  }
  return str;
}

static 
void debug_sym(Elf32_Sym *symtab,char *strtab,int symtab_index)
{
  if(_dl_debug_symbols)
  {
    if(symtab_index){
      _dl_dprintf(_dl_debug_file, "\n%s\tvalue=%x\tsize=%x\tinfo=%x\tother=%x\tshndx=%x",
		  strtab + symtab[symtab_index].st_name,
		  symtab[symtab_index].st_value,
		  symtab[symtab_index].st_size,
		  symtab[symtab_index].st_info,
		  symtab[symtab_index].st_other,
		  symtab[symtab_index].st_shndx);
    }
  }
}

static void debug_reloc(Elf32_Sym *symtab,char *strtab, ELF_RELOC *rpnt)
{
  if(_dl_debug_reloc)
  {
    int symtab_index;
    const char *sym;
    symtab_index = ELF32_R_SYM(rpnt->r_info);
    sym = symtab_index ? strtab + symtab[symtab_index].st_name : "sym=0x0";
    
#ifdef ELF_USES_RELOCA
    _dl_dprintf(_dl_debug_file, "\n%s\toffset=%x\taddend=%x %s",
		_dl_reltypes(ELF32_R_TYPE(rpnt->r_info)),
		rpnt->r_offset,
		rpnt->r_addend,
		sym);
#else
    _dl_dprintf(_dl_debug_file, "\n%s\toffset=%x %s",
		_dl_reltypes(ELF32_R_TYPE(rpnt->r_info)),
		rpnt->r_offset,
		sym);
#endif
  }
}
#endif

/* Program to load an ELF binary on a linux system, and run it.
   References to symbols in sharable libraries can be resolved by either
   an ELF sharable library or a linux style of shared library. */

/* Disclaimer:  I have never seen any AT&T source code for SVr4, nor have
   I ever taken any courses on internals.  This program was developed using
   information available through the book "UNIX SYSTEM V RELEASE 4,
   Programmers guide: Ansi C and Programming Support Tools", which did
   a more than adequate job of explaining everything required to get this
   working. */

extern int _dl_linux_resolve(void);

unsigned long _dl_linux_resolver(struct elf_resolve *tpnt, int reloc_entry)
{
	int reloc_type;
	ELF_RELOC *this_reloc;
	char *strtab;
	Elf32_Sym *symtab;
	ELF_RELOC *rel_addr;
	int symtab_index;
	char *new_addr;
	char **got_addr;
	unsigned long instr_addr;

	rel_addr = (ELF_RELOC *) (tpnt->dynamic_info[DT_JMPREL] + tpnt->loadaddr);

	this_reloc = rel_addr + (reloc_entry >> 3);
	reloc_type = ELF32_R_TYPE(this_reloc->r_info);
	symtab_index = ELF32_R_SYM(this_reloc->r_info);

	symtab = (Elf32_Sym *) (tpnt->dynamic_info[DT_SYMTAB] + tpnt->loadaddr);
	strtab = (char *) (tpnt->dynamic_info[DT_STRTAB] + tpnt->loadaddr);


	if (reloc_type != R_ARM_JUMP_SLOT) {
		_dl_dprintf(2, "%s: Incorrect relocation type in jump relocations\n", 
			_dl_progname);
		_dl_exit(1);
	};

	/* Address of jump instruction to fix up */
	instr_addr = ((unsigned long) this_reloc->r_offset + 
		(unsigned long) tpnt->loadaddr);
	got_addr = (char **) instr_addr;

	/* Get the address of the GOT entry */
	new_addr = _dl_find_hash(strtab + symtab[symtab_index].st_name, 
		tpnt->symbol_scope, tpnt, resolver);
	if (!new_addr) {
		_dl_dprintf(2, "%s: can't resolve symbol '%s'\n", 
			_dl_progname, strtab + symtab[symtab_index].st_name);
		_dl_exit(1);
	};
#if defined (__SUPPORT_LD_DEBUG__)
	if ((unsigned long) got_addr < 0x40000000)
	{
		if (_dl_debug_bindings)
		{
			_dl_dprintf(_dl_debug_file, "\nresolve function: %s",
					strtab + symtab[symtab_index].st_name);
			if(_dl_debug_detail) _dl_dprintf(_dl_debug_file, 
					"\tpatch %x ==> %x @ %x", *got_addr, new_addr, got_addr);
		}
	}
	if (!_dl_debug_nofixups) {
		*got_addr = new_addr;
	}
#else
	*got_addr = new_addr;
#endif

	return (unsigned long) new_addr;
}

static int
_dl_parse(struct elf_resolve *tpnt, struct dyn_elf *scope,
	  unsigned long rel_addr, unsigned long rel_size,
	  int (*reloc_fnc) (struct elf_resolve *tpnt, struct dyn_elf *scope,
			    ELF_RELOC *rpnt, Elf32_Sym *symtab, char *strtab))
{
	int i;
	char *strtab;
	int goof = 0;
	Elf32_Sym *symtab;
	ELF_RELOC *rpnt;
	int symtab_index;
	/* Now parse the relocation information */

	rpnt = (ELF_RELOC *) (rel_addr + tpnt->loadaddr);
	rel_size = rel_size / sizeof(ELF_RELOC);

	symtab = (Elf32_Sym *) (tpnt->dynamic_info[DT_SYMTAB] + tpnt->loadaddr);
	strtab = (char *) (tpnt->dynamic_info[DT_STRTAB] + tpnt->loadaddr);

	  for (i = 0; i < rel_size; i++, rpnt++) {
	        int res;
	    
		symtab_index = ELF32_R_SYM(rpnt->r_info);
		
		/* When the dynamic linker bootstrapped itself, it resolved some symbols.
		   Make sure we do not do them again */
		if (!symtab_index && tpnt->libtype == program_interpreter)
			continue;
		if (symtab_index && tpnt->libtype == program_interpreter &&
		    _dl_symbol(strtab + symtab[symtab_index].st_name))
			continue;

#if defined (__SUPPORT_LD_DEBUG__)
		debug_sym(symtab,strtab,symtab_index);
		debug_reloc(symtab,strtab,rpnt);
#endif

		res = reloc_fnc (tpnt, scope, rpnt, symtab, strtab);

		if (res==0) continue;

		_dl_dprintf(2, "\n%s: ",_dl_progname);
		
		if (symtab_index)
		  _dl_dprintf(2, "symbol '%s': ", strtab + symtab[symtab_index].st_name);
		  
		if (res <0)
		{
		        int reloc_type = ELF32_R_TYPE(rpnt->r_info);
#if defined (__SUPPORT_LD_DEBUG__)
			_dl_dprintf(2, "can't handle reloc type %s\n ", _dl_reltypes(reloc_type));
#else
			_dl_dprintf(2, "can't handle reloc type %x\n", reloc_type);
#endif			
			_dl_exit(-res);
		}
		else if (res >0)
		{
			_dl_dprintf(2, "can't resolve symbol\n");
			goof += res;
		}
	  }
	  return goof;
}

static unsigned long
fix_bad_pc24 (unsigned long *const reloc_addr, unsigned long value)
{
  static void *fix_page;
  static unsigned int fix_offset;
  unsigned int *fix_address;
  if (! fix_page)
    {
      fix_page = _dl_mmap (NULL,  PAGE_SIZE   , PROT_READ | PROT_WRITE | PROT_EXEC,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      fix_offset = 0;
    }

  fix_address = (unsigned int *)(fix_page + fix_offset);
  fix_address[0] = 0xe51ff004;  /* ldr pc, [pc, #-4] */
  fix_address[1] = value;

  fix_offset += 8;
  if (fix_offset >= PAGE_SIZE)
    fix_page = NULL;

  return (unsigned long)fix_address;
}

static int
_dl_do_reloc (struct elf_resolve *tpnt,struct dyn_elf *scope,
	      ELF_RELOC *rpnt, Elf32_Sym *symtab, char *strtab)
{
	int reloc_type;
	int symtab_index;
	unsigned long *reloc_addr;
	unsigned long symbol_addr;
	int goof = 0;

	reloc_addr = (unsigned long *) (tpnt->loadaddr + (unsigned long) rpnt->r_offset);
	reloc_type = ELF32_R_TYPE(rpnt->r_info);
	symtab_index = ELF32_R_SYM(rpnt->r_info);
	symbol_addr = 0;

	if (symtab_index) {

		symbol_addr = (unsigned long) _dl_find_hash(strtab + symtab[symtab_index].st_name, 
				scope, (reloc_type == R_ARM_JUMP_SLOT ? tpnt : NULL), symbolrel);

		/*
		 * We want to allow undefined references to weak symbols - this might
		 * have been intentional.  We should not be linking local symbols
		 * here, so all bases should be covered.
		 */
		if (!symbol_addr && ELF32_ST_BIND(symtab[symtab_index].st_info) == STB_GLOBAL) {
			goof++;
		}
	}

#if defined (__SUPPORT_LD_DEBUG__)
	{
		unsigned long old_val = *reloc_addr;
#endif
		switch (reloc_type) {
			case R_ARM_NONE:
				break;
			case R_ARM_ABS32:
				*reloc_addr += symbol_addr;
				break;
			case R_ARM_PC24:
				{
					unsigned long addend;
					long newvalue, topbits;

					addend = *reloc_addr & 0x00ffffff;
					if (addend & 0x00800000) addend |= 0xff000000;

					newvalue = symbol_addr - (unsigned long)reloc_addr + (addend << 2);
					topbits = newvalue & 0xfe000000;
					if (topbits != 0xfe000000 && topbits != 0x00000000)
					{
						newvalue = fix_bad_pc24(reloc_addr, symbol_addr)
							- (unsigned long)reloc_addr + (addend << 2);
						topbits = newvalue & 0xfe000000;
						if (topbits != 0xfe000000 && topbits != 0x00000000)
						{
							_dl_dprintf(2,"symbol '%s': R_ARM_PC24 relocation out of range.", 
								symtab[symtab_index].st_name);
							_dl_exit(1);
						}
					}
					newvalue >>= 2;
					symbol_addr = (*reloc_addr & 0xff000000) | (newvalue & 0x00ffffff);
					*reloc_addr = symbol_addr;
					break;
				}
			case R_ARM_GLOB_DAT:
			case R_ARM_JUMP_SLOT:
				*reloc_addr = symbol_addr;
				break;
			case R_ARM_RELATIVE:
				*reloc_addr += (unsigned long) tpnt->loadaddr;
				break;
			case R_ARM_COPY:						
#if 0							
				/* Do this later */
				_dl_dprintf(2, "Doing copy for symbol ");
				if (symtab_index) _dl_dprintf(2, strtab + symtab[symtab_index].st_name);
				_dl_dprintf(2, "\n");
				_dl_memcpy((void *) symtab[symtab_index].st_value, 
						(void *) symbol_addr, symtab[symtab_index].st_size);
#endif
				break;
			default:
				return -1; /*call _dl_exit(1) */
		}
#if defined (__SUPPORT_LD_DEBUG__)
		if(_dl_debug_reloc && _dl_debug_detail)
			_dl_dprintf(_dl_debug_file, "\tpatch: %x ==> %x @ %x", old_val, *reloc_addr, reloc_addr);
	}

#endif

	return goof;
}

static int
_dl_do_lazy_reloc (struct elf_resolve *tpnt, struct dyn_elf *scope,
		   ELF_RELOC *rpnt, Elf32_Sym *symtab, char *strtab)
{
	int reloc_type;
	unsigned long *reloc_addr;

	reloc_addr = (unsigned long *) (tpnt->loadaddr + (unsigned long) rpnt->r_offset);
	reloc_type = ELF32_R_TYPE(rpnt->r_info);

#if defined (__SUPPORT_LD_DEBUG__)
	{
		unsigned long old_val = *reloc_addr;
#endif
		switch (reloc_type) {
			case R_ARM_NONE:
				break;
			case R_ARM_JUMP_SLOT:
				*reloc_addr += (unsigned long) tpnt->loadaddr;
				break;
			default:
				return -1; /*call _dl_exit(1) */
		}
#if defined (__SUPPORT_LD_DEBUG__)
		if(_dl_debug_reloc && _dl_debug_detail)
			_dl_dprintf(_dl_debug_file, "\tpatch: %x ==> %x @ %x", old_val, *reloc_addr, reloc_addr);
	}

#endif
	return 0;

}

/* This is done as a separate step, because there are cases where
   information is first copied and later initialized.  This results in
   the wrong information being copied.  Someone at Sun was complaining about
   a bug in the handling of _COPY by SVr4, and this may in fact be what he
   was talking about.  Sigh. */

/* No, there are cases where the SVr4 linker fails to emit COPY relocs
   at all */
static int
_dl_do_copy (struct elf_resolve *tpnt, struct dyn_elf *scope,
	     ELF_RELOC *rpnt, Elf32_Sym *symtab, char *strtab)
{
        int reloc_type;
	int symtab_index;
	unsigned long *reloc_addr;
	unsigned long symbol_addr;
	int goof = 0;
	  
	reloc_addr = (unsigned long *) (tpnt->loadaddr + (unsigned long) rpnt->r_offset);
	reloc_type = ELF32_R_TYPE(rpnt->r_info);
	if (reloc_type != R_ARM_COPY) 
		return 0;
	symtab_index = ELF32_R_SYM(rpnt->r_info);
	symbol_addr = 0;
		
	if (symtab_index) {

		symbol_addr = (unsigned long) _dl_find_hash(strtab + 
			symtab[symtab_index].st_name, scope, 
			NULL, copyrel);
		if (!symbol_addr) goof++;
	}
	if (!goof) {
#if defined (__SUPPORT_LD_DEBUG__)
	        if(_dl_debug_move)
		  _dl_dprintf(_dl_debug_file,"\n%s move %x bytes from %x to %x",
			     strtab + symtab[symtab_index].st_name,
			     symtab[symtab_index].st_size,
			     symbol_addr, symtab[symtab_index].st_value);
#endif
		_dl_memcpy((char *) symtab[symtab_index].st_value, 
			(char *) symbol_addr, symtab[symtab_index].st_size);
	}

	return goof;
}

void _dl_parse_lazy_relocation_information(struct elf_resolve *tpnt, 
	unsigned long rel_addr, unsigned long rel_size, int type)
{
  (void)_dl_parse(tpnt, NULL, rel_addr, rel_size, _dl_do_lazy_reloc);
}

int _dl_parse_relocation_information(struct elf_resolve *tpnt, 
	unsigned long rel_addr, unsigned long rel_size, int type)
{
  return _dl_parse(tpnt, tpnt->symbol_scope, rel_addr, rel_size, _dl_do_reloc);
}

int _dl_parse_copy_information(struct dyn_elf *xpnt, unsigned long rel_addr, 
	unsigned long rel_size, int type)
{
  return _dl_parse(xpnt->dyn, xpnt->next, rel_addr, rel_size, _dl_do_copy);
}


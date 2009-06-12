/* vi: set sw=4 ts=4: */
/* powerpc shared library loader suppport
 *
 * Copyright (C) 2001-2002,  David A. Schleef
 * Copyright (C) 2003, Erik Andersen
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
	{ "R_PPC_NONE", "R_PPC_ADDR32", "R_PPC_ADDR24", "R_PPC_ADDR16",
	"R_PPC_ADDR16_LO", "R_PPC_ADDR16_HI", "R_PPC_ADDR16_HA",
	"R_PPC_ADDR14", "R_PPC_ADDR14_BRTAKEN", "R_PPC_ADDR14_BRNTAKEN",
	"R_PPC_REL24", "R_PPC_REL14", "R_PPC_REL14_BRTAKEN",
	"R_PPC_REL14_BRNTAKEN", "R_PPC_GOT16", "R_PPC_GOT16_LO",
	"R_PPC_GOT16_HI", "R_PPC_GOT16_HA", "R_PPC_PLTREL24",
	"R_PPC_COPY", "R_PPC_GLOB_DAT", "R_PPC_JMP_SLOT", "R_PPC_RELATIVE",
	"R_PPC_LOCAL24PC", "R_PPC_UADDR32", "R_PPC_UADDR16", "R_PPC_REL32",
	"R_PPC_PLT32", "R_PPC_PLTREL32", "R_PPC_PLT16_LO", "R_PPC_PLT16_HI",
	"R_PPC_PLT16_HA", "R_PPC_SDAREL16", "R_PPC_SECTOFF",
	"R_PPC_SECTOFF_LO", "R_PPC_SECTOFF_HI", "R_PPC_SECTOFF_HA",
};

static const char *
_dl_reltypes(int type)
{
  static char buf[22];  
  const char *str;
  
  if (type >= (int)(sizeof (_dl_reltypes_tab)/sizeof(_dl_reltypes_tab[0])) ||
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
      _dl_dprintf(_dl_debug_file, "\n%s\n\tvalue=%x\tsize=%x\tinfo=%x\tother=%x\tshndx=%x",
		  strtab + symtab[symtab_index].st_name,
		  symtab[symtab_index].st_value,
		  symtab[symtab_index].st_size,
		  symtab[symtab_index].st_info,
		  symtab[symtab_index].st_other,
		  symtab[symtab_index].st_shndx);
    }
  }
}

static 
void debug_reloc(Elf32_Sym *symtab,char *strtab, ELF_RELOC *rpnt)
{
  if(_dl_debug_reloc)
  {
    int symtab_index;
    const char *sym;
    symtab_index = ELF32_R_SYM(rpnt->r_info);
    sym = symtab_index ? strtab + symtab[symtab_index].st_name : "sym=0x0";
    
  if(_dl_debug_symbols)
	  _dl_dprintf(_dl_debug_file, "\n\t");
  else
	  _dl_dprintf(_dl_debug_file, "\n%s\n\t", sym);
#ifdef ELF_USES_RELOCA
    _dl_dprintf(_dl_debug_file, "%s\toffset=%x\taddend=%x",
		_dl_reltypes(ELF32_R_TYPE(rpnt->r_info)),
		rpnt->r_offset,
		rpnt->r_addend);
#else
    _dl_dprintf(_dl_debug_file, "%s\toffset=%x\n",
		_dl_reltypes(ELF32_R_TYPE(rpnt->r_info)),
		rpnt->r_offset);
#endif
  }
}
#endif

extern int _dl_linux_resolve(void);

void _dl_init_got(unsigned long *plt,struct elf_resolve *tpnt)
{
	unsigned long target_addr = (unsigned long)_dl_linux_resolve;
	unsigned int n_plt_entries;
	unsigned long *tramp;
	unsigned long data_words;
	unsigned int rel_offset_words;

	//DPRINTF("init_got plt=%x, tpnt=%x\n", (unsigned long)plt,(unsigned long)tpnt);

	n_plt_entries = tpnt->dynamic_info[DT_PLTRELSZ] / sizeof(ELF_RELOC);
	//DPRINTF("n_plt_entries %d\n",n_plt_entries);

	rel_offset_words = PLT_DATA_START_WORDS(n_plt_entries);
	//DPRINTF("rel_offset_words %x\n",rel_offset_words);
	data_words = (unsigned long)(plt + rel_offset_words);
	//DPRINTF("data_words %x\n",data_words);

	tpnt->data_words = data_words;

	plt[PLT_LONGBRANCH_ENTRY_WORDS] = OPCODE_ADDIS_HI(11, 11, data_words);
	plt[PLT_LONGBRANCH_ENTRY_WORDS+1] = OPCODE_LWZ(11,data_words,11);

	plt[PLT_LONGBRANCH_ENTRY_WORDS+2] = OPCODE_MTCTR(11);
	plt[PLT_LONGBRANCH_ENTRY_WORDS+3] = OPCODE_BCTR();

	/* [4] */
	/* [5] */

	tramp = plt + PLT_TRAMPOLINE_ENTRY_WORDS;
	tramp[0] = OPCODE_ADDIS_HI(11,11,-data_words);
	tramp[1] = OPCODE_ADDI(11,11,-data_words);
	tramp[2] = OPCODE_SLWI(12,11,1);
	tramp[3] = OPCODE_ADD(11,12,11);
	tramp[4] = OPCODE_LI(12,target_addr);
	tramp[5] = OPCODE_ADDIS_HI(12,12,target_addr);
	tramp[6] = OPCODE_MTCTR(12);
	tramp[7] = OPCODE_LI(12,(unsigned long)tpnt);
	tramp[8] = OPCODE_ADDIS_HI(12,12,(unsigned long)tpnt);
	tramp[9] = OPCODE_BCTR();

	/* [16] unused */
	/* [17] unused */

	/* instructions were modified */
	PPC_DCBST(plt);
	PPC_DCBST(plt+4);
	PPC_DCBST(plt+8);
	PPC_DCBST(plt+12);
	PPC_DCBST(plt+16-1);
	PPC_SYNC;
	PPC_ICBI(plt);
	PPC_ICBI(plt+4); /* glibc thinks this is not needed */
	PPC_ICBI(plt+8); /* glibc thinks this is not needed */
	PPC_ICBI(plt+12); /* glibc thinks this is not needed */
	PPC_ICBI(plt+16-1);
	PPC_ISYNC;
}

unsigned long _dl_linux_resolver(struct elf_resolve *tpnt, int reloc_entry)
{
	int reloc_type;
	ELF_RELOC *this_reloc;
	char *strtab;
	Elf32_Sym *symtab;
	ELF_RELOC *rel_addr;
	int symtab_index;
	char *symname;
	unsigned long insn_addr;
	unsigned long *insns;
	unsigned long new_addr;
	unsigned long delta;

	rel_addr = (ELF_RELOC *) (tpnt->dynamic_info[DT_JMPREL] + tpnt->loadaddr);

	this_reloc = (void *)rel_addr + reloc_entry;
	reloc_type = ELF32_R_TYPE(this_reloc->r_info);
	symtab_index = ELF32_R_SYM(this_reloc->r_info);

	symtab = (Elf32_Sym *) (tpnt->dynamic_info[DT_SYMTAB] + tpnt->loadaddr);
	strtab = (char *) (tpnt->dynamic_info[DT_STRTAB] + tpnt->loadaddr);
	symname      = strtab + symtab[symtab_index].st_name;

#if defined (__SUPPORT_LD_DEBUG__)
	debug_sym(symtab,strtab,symtab_index);
	debug_reloc(symtab,strtab,this_reloc);
#endif

	if (reloc_type != R_PPC_JMP_SLOT) {
		_dl_dprintf(2, "%s: Incorrect relocation type in jump relocation\n", _dl_progname);
		_dl_exit(1);
	};

	/* Address of dump instruction to fix up */
	insn_addr = (unsigned long) tpnt->loadaddr +
		(unsigned long) this_reloc->r_offset;

#if defined (__SUPPORT_LD_DEBUG__)
	if(_dl_debug_reloc && _dl_debug_detail)
		_dl_dprintf(_dl_debug_file, "\n\tResolving symbol %s %x --> ", symname, insn_addr);
#endif

	/* Get the address of the GOT entry */
	new_addr = (unsigned long) _dl_find_hash(
		strtab + symtab[symtab_index].st_name, 
		tpnt->symbol_scope, tpnt, resolver);
	if (!new_addr) {
		_dl_dprintf(2, "%s: can't resolve symbol '%s'\n", 
			_dl_progname, symname);
		_dl_exit(1);
	};

#if defined (__SUPPORT_LD_DEBUG__)
	if(_dl_debug_reloc && _dl_debug_detail)
		_dl_dprintf(_dl_debug_file, "%x\n", new_addr);
#endif

	insns = (unsigned long *)insn_addr;
	delta = new_addr - insn_addr;

	if(delta<<6>>6 == delta){
		insns[0] = OPCODE_B(delta);
	}else if (new_addr <= 0x01fffffc || new_addr >= 0xfe000000){
		insns[0] = OPCODE_BA (new_addr);
	}else{
		/* Warning: we don't handle double-sized PLT entries */
		unsigned long plt_addr;
		unsigned long *ptr;
		int index;

		plt_addr = (unsigned long)tpnt->dynamic_info[DT_PLTGOT] + 
			(unsigned long)tpnt->loadaddr;

		delta = PLT_LONGBRANCH_ENTRY_WORDS*4 - (insn_addr-plt_addr+4);

		index = (insn_addr - plt_addr - PLT_INITIAL_ENTRY_WORDS*4)/8;

		ptr = (unsigned long *)tpnt->data_words;
		//DPRINTF("plt_addr=%x delta=%x index=%x ptr=%x\n", plt_addr, delta, index, ptr);
		insns += 1;

		ptr[index] = new_addr;
		PPC_SYNC;
		/* icache sync is not necessary, since this will be a data load */
		//PPC_DCBST(ptr+index);
		//PPC_SYNC;
		//PPC_ICBI(ptr+index);
		//PPC_ISYNC;

		insns[0] = OPCODE_B(delta);

	}

	/* instructions were modified */
	PPC_DCBST(insns);
	PPC_SYNC;
	PPC_ICBI(insns);
	PPC_ISYNC;

	return new_addr;
}

static int
_dl_parse(struct elf_resolve *tpnt, struct dyn_elf *scope,
	  unsigned long rel_addr, unsigned long rel_size,
	  int (*reloc_fnc) (struct elf_resolve *tpnt, struct dyn_elf *scope,
			    ELF_RELOC *rpnt, Elf32_Sym *symtab, char *strtab))
{
	unsigned int i;
	char *strtab;
	Elf32_Sym *symtab;
	ELF_RELOC *rpnt;
	int symtab_index;

	/* Now parse the relocation information */
	rpnt = (ELF_RELOC *)(intptr_t) (rel_addr + tpnt->loadaddr);
	rel_size = rel_size / sizeof(ELF_RELOC);

	symtab = (Elf32_Sym *)(intptr_t) (tpnt->dynamic_info[DT_SYMTAB] + tpnt->loadaddr);
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
			return res;
		}
	  }
	  return 0;
}

static int
_dl_do_lazy_reloc (struct elf_resolve *tpnt, struct dyn_elf *scope,
		   ELF_RELOC *rpnt, Elf32_Sym *symtab, char *strtab)
{
	int reloc_type;
	unsigned long reloc_addr;
#if defined (__SUPPORT_LD_DEBUG__)
	unsigned long old_val;
#endif
	(void)scope;
	(void)symtab;
	(void)strtab;

	reloc_addr = (unsigned long)tpnt->loadaddr + (unsigned long) rpnt->r_offset;
	reloc_type = ELF32_R_TYPE(rpnt->r_info);

#if defined (__SUPPORT_LD_DEBUG__)
	old_val = reloc_addr;
#endif

	switch (reloc_type) {
		case R_PPC_NONE:
			return 0;
			break;
		case R_PPC_JMP_SLOT:
			{
				int index;
				unsigned long delta;
				unsigned long *plt;
				unsigned long *insns;

				plt = (unsigned long *)(tpnt->dynamic_info[DT_PLTGOT] + tpnt->loadaddr);

				delta = (unsigned long)(plt+PLT_TRAMPOLINE_ENTRY_WORDS+2) - (reloc_addr+4);

				index = (reloc_addr - (unsigned long)(plt+PLT_INITIAL_ENTRY_WORDS)) 
						/sizeof(unsigned long);
				index /= 2;
				//DPRINTF("        index %x delta %x\n",index,delta);
				insns = (unsigned long *)reloc_addr;
				insns[0] = OPCODE_LI(11,index*4);
				insns[1] = OPCODE_B(delta);
				break;
			}
		default:
#if 0
			_dl_dprintf(2, "%s: (LAZY) can't handle reloc type ", 
					_dl_progname);
#if defined (__SUPPORT_LD_DEBUG__)
			_dl_dprintf(2, "%s ", _dl_reltypes[reloc_type]);
#endif
			if (symtab_index)
				_dl_dprintf(2, "'%s'\n", strtab + symtab[symtab_index].st_name);
#endif
			//_dl_exit(1);
			return -1;
	};

	/* instructions were modified */
	PPC_DCBST(reloc_addr);
	PPC_DCBST(reloc_addr+4);
	PPC_SYNC;
	PPC_ICBI(reloc_addr);
	PPC_ICBI(reloc_addr+4);
	PPC_ISYNC;

#if defined (__SUPPORT_LD_DEBUG__)
	if(_dl_debug_reloc && _dl_debug_detail)
		_dl_dprintf(_dl_debug_file, "\tpatched: %x ==> %x", old_val, reloc_addr);
#endif
	return 0;

}

static int
_dl_do_reloc (struct elf_resolve *tpnt,struct dyn_elf *scope,
	      ELF_RELOC *rpnt, Elf32_Sym *symtab, char *strtab)
{
	int reloc_type;
	int symtab_index;
	char *symname;
	unsigned long *reloc_addr;
	unsigned long symbol_addr;
#if defined (__SUPPORT_LD_DEBUG__)
	unsigned long old_val;
#endif

	reloc_addr   = (unsigned long *)(intptr_t) (tpnt->loadaddr + (unsigned long) rpnt->r_offset);
	reloc_type   = ELF32_R_TYPE(rpnt->r_info);
	symtab_index = ELF32_R_SYM(rpnt->r_info);
	symbol_addr  = 0;
	symname      = strtab + symtab[symtab_index].st_name;

	if (symtab_index) {

		symbol_addr = (unsigned long) _dl_find_hash(symname, scope, 
				(reloc_type == R_PPC_JMP_SLOT ? tpnt : NULL), symbolrel);

		/*
		 * We want to allow undefined references to weak symbols - this might
		 * have been intentional.  We should not be linking local symbols
		 * here, so all bases should be covered.
		 */

		if (!symbol_addr && ELF32_ST_BIND(symtab[symtab_index].st_info) == STB_GLOBAL) {
#if defined (__SUPPORT_LD_DEBUG__)
			_dl_dprintf(2, "\tglobal symbol '%s' already defined in '%s'\n",
					symname, tpnt->libname);
#endif
			return 0;
		}
	}

#if defined (__SUPPORT_LD_DEBUG__)
	old_val = *reloc_addr;
#endif
		switch (reloc_type) {
			case R_PPC_NONE:
				return 0;
				break;
			case R_PPC_REL24:
#if 0
				{
					unsigned long delta = symbol_addr - (unsigned long)reloc_addr;
					if(delta<<6>>6 != delta){
						_dl_dprintf(2,"R_PPC_REL24: Reloc out of range\n");
						_dl_exit(1);
					}
					*reloc_addr &= 0xfc000003;
					*reloc_addr |= delta&0x03fffffc;
				}
				break;
#else
				_dl_dprintf(2, "%s: symbol '%s' is type R_PPC_REL24\n\tCompile shared libraries with -fPIC!\n",
						_dl_progname, symname);
				_dl_exit(1);
#endif
			case R_PPC_RELATIVE:
				*reloc_addr = (unsigned long)tpnt->loadaddr + (unsigned long)rpnt->r_addend;
				break;
			case R_PPC_ADDR32:
				*reloc_addr += symbol_addr;
				break;
			case R_PPC_ADDR16_HA:
				/* XXX is this correct? */
				*(short *)reloc_addr += (symbol_addr+0x8000)>>16;
				break;
			case R_PPC_ADDR16_HI:
				*(short *)reloc_addr += symbol_addr>>16;
				break;
			case R_PPC_ADDR16_LO:
				*(short *)reloc_addr += symbol_addr;
				break;
			case R_PPC_JMP_SLOT:
				{
					unsigned long targ_addr = (unsigned long)*reloc_addr;
					unsigned long delta = targ_addr - (unsigned long)reloc_addr;
					if(delta<<6>>6 == delta){
						*reloc_addr = OPCODE_B(delta);
					}else if (targ_addr <= 0x01fffffc || targ_addr >= 0xfe000000){
						*reloc_addr = OPCODE_BA (targ_addr);
					}else{
						{
							int index;
							unsigned long delta2;
							unsigned long *plt, *ptr;
							plt = (unsigned long *)(tpnt->dynamic_info[DT_PLTGOT] + tpnt->loadaddr);

							delta2 = (unsigned long)(plt+PLT_LONGBRANCH_ENTRY_WORDS)
								- (unsigned long)(reloc_addr+1);

							index = ((unsigned long)reloc_addr -
									(unsigned long)(plt+PLT_INITIAL_ENTRY_WORDS))
								/sizeof(unsigned long);
							index /= 2;
							//DPRINTF("        index %x delta %x\n",index,delta2);
							ptr = (unsigned long *)tpnt->data_words;
							ptr[index] = targ_addr;
							reloc_addr[0] = OPCODE_LI(11,index*4);
							reloc_addr[1] = OPCODE_B(delta2);

							/* instructions were modified */
							PPC_DCBST(reloc_addr+1);
							PPC_SYNC;
							PPC_ICBI(reloc_addr+1);
						}
					}
					break;
				}
			case R_PPC_GLOB_DAT:
				*reloc_addr += symbol_addr;
				break;
			case R_PPC_COPY:
				// handled later
				return 0;
				break;
			default:
#if 0
				_dl_dprintf(2, "%s: can't handle reloc type ", _dl_progname);
#if defined (__SUPPORT_LD_DEBUG__)
				_dl_dprintf(2, "%s ", _dl_reltypes[reloc_type]);
#endif
				if (symtab_index)
					_dl_dprintf(2, "'%s'\n", strtab + symtab[symtab_index].st_name);
#endif
				//_dl_exit(1);
				return -1;
		};

		/* instructions were modified */
		PPC_DCBST(reloc_addr);
		PPC_SYNC;
		PPC_ICBI(reloc_addr);
		PPC_ISYNC;

#if defined (__SUPPORT_LD_DEBUG__)
	if(_dl_debug_reloc && _dl_debug_detail)
		_dl_dprintf(_dl_debug_file, "\tpatched: %x ==> %x @ %x", old_val, *reloc_addr, reloc_addr);
#endif

	return 0;
}


/* This is done as a separate step, because there are cases where
   information is first copied and later initialized.  This results in
   the wrong information being copied.  Someone at Sun was complaining about
   a bug in the handling of _COPY by SVr4, and this may in fact be what he
   was talking about.  Sigh. */
static int
_dl_do_copy (struct elf_resolve *tpnt, struct dyn_elf *scope,
	     ELF_RELOC *rpnt, Elf32_Sym *symtab, char *strtab)
{
	int reloc_type;
	int symtab_index;
	unsigned long *reloc_addr;
	unsigned long symbol_addr;
	int goof = 0;
	char *symname;
	  
	reloc_addr = (unsigned long *)(intptr_t) (tpnt->loadaddr + (unsigned long) rpnt->r_offset);
	reloc_type = ELF32_R_TYPE(rpnt->r_info);
	if (reloc_type != R_PPC_COPY) 
		return 0;
	symtab_index = ELF32_R_SYM(rpnt->r_info);
	symbol_addr = 0;
	symname      = strtab + symtab[symtab_index].st_name;
		
	if (symtab_index) {
		symbol_addr = (unsigned long) _dl_find_hash(symname, scope, NULL, copyrel);
		if (!symbol_addr) goof++;
	}
	if (!goof) {
#if defined (__SUPPORT_LD_DEBUG__)
	        if(_dl_debug_move)
		  _dl_dprintf(_dl_debug_file,"\n%s move %x bytes from %x to %x",
			     symname, symtab[symtab_index].st_size,
			     symbol_addr, symtab[symtab_index].st_value);
#endif
			_dl_memcpy((char *) reloc_addr,
					(char *) symbol_addr, symtab[symtab_index].st_size);
	}

	return goof;
}

void _dl_parse_lazy_relocation_information(struct elf_resolve *tpnt, 
	unsigned long rel_addr, unsigned long rel_size, int type)
{
	(void) type;
	(void)_dl_parse(tpnt, NULL, rel_addr, rel_size, _dl_do_lazy_reloc);
}

int _dl_parse_relocation_information(struct elf_resolve *tpnt, 
	unsigned long rel_addr, unsigned long rel_size, int type)
{
	(void) type;
	return _dl_parse(tpnt, tpnt->symbol_scope, rel_addr, rel_size, _dl_do_reloc);
}

int _dl_parse_copy_information(struct dyn_elf *xpnt, unsigned long rel_addr, 
	unsigned long rel_size, int type)
{
	(void) type;
	return _dl_parse(xpnt->dyn, xpnt->next, rel_addr, rel_size, _dl_do_copy);
}



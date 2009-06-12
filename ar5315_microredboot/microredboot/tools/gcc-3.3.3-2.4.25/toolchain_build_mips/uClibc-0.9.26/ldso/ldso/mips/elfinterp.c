/* vi: set sw=4 ts=4: */
/* mips/mipsel ELF shared library loader suppport
 *
   Copyright (C) 2002, Steven J. Hill (sjhill@realitydiluted.com)
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
		[0]		"R_MIPS_NONE",	"R_MIPS_16",	"R_MIPS_32",
		[3]		"R_MIPS_REL32",	"R_MIPS_26",	"R_MIPS_HI16",
		[6]		"R_MIPS_LO16",	"R_MIPS_GPREL16",	"R_MIPS_LITERAL",
		[9]		"R_MIPS_GOT16",	"R_MIPS_PC16",	"R_MIPS_CALL16",
		[12]	"R_MIPS_GPREL32",
		[16]	"R_MIPS_SHIFT5",	"R_MIPS_SHIFT6",	"R_MIPS_64",
		[19]	"R_MIPS_GOT_DISP",	"R_MIPS_GOT_PAGE",	"R_MIPS_GOT_OFST",
		[22]	"R_MIPS_GOT_HI16",	"R_MIPS_GOT_LO16",	"R_MIPS_SUB",
		[25]	"R_MIPS_INSERT_A",	"R_MIPS_INSERT_B",	"R_MIPS_DELETE",
		[28]	"R_MIPS_HIGHER",	"R_MIPS_HIGHEST",	"R_MIPS_CALL_HI16",
		[31]	"R_MIPS_CALL_LO16",	"R_MIPS_SCN_DISP",	"R_MIPS_REL16",
		[34]	"R_MIPS_ADD_IMMEDIATE",	"R_MIPS_PJUMP",	"R_MIPS_RELGOT",
		[37]	"R_MIPS_JALR",
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

static void debug_reloc(Elf32_Sym *symtab,char *strtab, ELF_RELOC *rpnt)
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

#define OFFSET_GP_GOT 0x7ff0

unsigned long _dl_linux_resolver(unsigned long sym_index,
	unsigned long old_gpreg)
{
	unsigned long *got = (unsigned long *) (old_gpreg - OFFSET_GP_GOT);
	struct elf_resolve *tpnt = (struct elf_resolve *) got[1];
	Elf32_Sym *sym;
	char *strtab;
	unsigned long local_gotno;
	unsigned long gotsym;
	unsigned long new_addr;
	unsigned long instr_addr;
	char **got_addr;
	char *symname;

	gotsym = tpnt->mips_gotsym;
	local_gotno = tpnt->mips_local_gotno;

	sym = ((Elf32_Sym *) (tpnt->dynamic_info[DT_SYMTAB] + tpnt->loadaddr)) + sym_index;
	strtab = (char *) (tpnt->dynamic_info[DT_STRTAB] + tpnt->loadaddr);
	symname = strtab + sym->st_name;

	new_addr = (unsigned long) _dl_find_hash(strtab + sym->st_name,
		 tpnt->symbol_scope, tpnt, resolver);
	 
	/* Address of jump instruction to fix up */
	instr_addr = (unsigned long) (got + local_gotno + sym_index - gotsym); 
	got_addr = (char **) instr_addr;
	 
#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_bindings)
	{
		_dl_dprintf(_dl_debug_file, "\nresolve function: %s", symname);
		if(_dl_debug_detail) _dl_dprintf(_dl_debug_file, 
				"\n\tpatched %x ==> %x @ %x\n", *got_addr, new_addr, got_addr);
	}
	if (!_dl_debug_nofixups) {
		*got_addr = (char*)new_addr;
	}
#else
	*got_addr = (char*)new_addr;
#endif

	return new_addr;
}

void _dl_parse_lazy_relocation_information(struct elf_resolve *tpnt, 
	unsigned long rel_addr, unsigned long rel_size, int type)
{
	/* Nothing to do */
	return;
}

int _dl_parse_copy_information(struct dyn_elf *xpnt, unsigned long rel_addr, 
	unsigned long rel_size, int type)
{
	/* Nothing to do */
	return 0;
}


int _dl_parse_relocation_information(struct elf_resolve *tpnt, 
	unsigned long rel_addr, unsigned long rel_size, int type)
{
	Elf32_Sym *symtab;
	Elf32_Rel *rpnt;
	char *strtab;
	unsigned long *got;
	unsigned long *reloc_addr=NULL, old_val=0;
	unsigned long symbol_addr;
	int i, reloc_type, symtab_index;

	/* Now parse the relocation information */
	rel_size = rel_size / sizeof(Elf32_Rel);
	rpnt = (Elf32_Rel *) (rel_addr + tpnt->loadaddr);

	symtab = (Elf32_Sym *) (tpnt->dynamic_info[DT_SYMTAB] + tpnt->loadaddr);
	strtab = (char *) (tpnt->dynamic_info[DT_STRTAB] + tpnt->loadaddr);
	got = (unsigned long *) (tpnt->dynamic_info[DT_PLTGOT] + tpnt->loadaddr);

	for (i = 0; i < rel_size; i++, rpnt++) {
		reloc_addr = (unsigned long *) (tpnt->loadaddr +
			(unsigned long) rpnt->r_offset);
		reloc_type = ELF32_R_TYPE(rpnt->r_info);
		symtab_index = ELF32_R_SYM(rpnt->r_info);
		symbol_addr = 0;

		if (!symtab_index && tpnt->libtype == program_interpreter)
			continue;

#if defined (__SUPPORT_LD_DEBUG__)
		debug_sym(symtab,strtab,symtab_index);
		debug_reloc(symtab,strtab,rpnt);
		old_val = *reloc_addr;
#endif

		switch (reloc_type) {
		case R_MIPS_REL32:
			if (symtab_index) {
				if (symtab_index < tpnt->mips_gotsym)
					*reloc_addr +=
						symtab[symtab_index].st_value +
						(unsigned long) tpnt->loadaddr;
				else {
					*reloc_addr += got[symtab_index + tpnt->mips_local_gotno -
						tpnt->mips_gotsym];
				}
			}
			else {
				*reloc_addr += (unsigned long) tpnt->loadaddr;
			}
			break;
		case R_MIPS_NONE:
			break;
		default:
			{
				int reloc_type = ELF32_R_TYPE(rpnt->r_info);
				_dl_dprintf(2, "\n%s: ",_dl_progname);

				if (symtab_index)
					_dl_dprintf(2, "symbol '%s': ", strtab + symtab[symtab_index].st_name);

#if defined (__SUPPORT_LD_DEBUG__)
				_dl_dprintf(2, "can't handle reloc type %s\n ", _dl_reltypes(reloc_type));
#else
				_dl_dprintf(2, "can't handle reloc type %x\n", reloc_type);
#endif			
				_dl_exit(1);
			}
		};

	};
#if defined (__SUPPORT_LD_DEBUG__)
	if(_dl_debug_reloc && _dl_debug_detail)
		_dl_dprintf(_dl_debug_file, "\tpatched: %x ==> %x @ %x\n", old_val, *reloc_addr, reloc_addr);
#endif

	return 0;
}

void _dl_perform_mips_global_got_relocations(struct elf_resolve *tpnt)
{
	Elf32_Sym *sym;
	char *strtab;
	unsigned long i;
	unsigned long *got_entry;

	for (; tpnt ; tpnt = tpnt->next) {

		/* We don't touch the dynamic linker */
		if (tpnt->libtype == program_interpreter)
			continue;

		/* Setup the loop variables */
		got_entry = (unsigned long *) (tpnt->loadaddr +
			tpnt->dynamic_info[DT_PLTGOT]) + tpnt->mips_local_gotno;
		sym = (Elf32_Sym *) (tpnt->dynamic_info[DT_SYMTAB] +
			(unsigned long) tpnt->loadaddr) + tpnt->mips_gotsym;
		strtab = (char *) (tpnt->dynamic_info[DT_STRTAB] +
			(unsigned long) tpnt->loadaddr);
		i = tpnt->mips_symtabno - tpnt->mips_gotsym;

		/* Relocate the global GOT entries for the object */
		while(i--) {
			if (sym->st_shndx == SHN_UNDEF) {
				if (ELF32_ST_TYPE(sym->st_info) == STT_FUNC && sym->st_value)
					*got_entry = sym->st_value + (unsigned long) tpnt->loadaddr;
				else {
					*got_entry = (unsigned long) _dl_find_hash(strtab +
						sym->st_name, tpnt->symbol_scope, NULL, copyrel);
				}
			}
			else if (sym->st_shndx == SHN_COMMON) {
				*got_entry = (unsigned long) _dl_find_hash(strtab +
					sym->st_name, tpnt->symbol_scope, NULL, copyrel);
			}
			else if (ELF32_ST_TYPE(sym->st_info) == STT_FUNC &&
				*got_entry != sym->st_value)
				*got_entry += (unsigned long) tpnt->loadaddr;
			else if (ELF32_ST_TYPE(sym->st_info) == STT_SECTION) {
				if (sym->st_other == 0)
					*got_entry += (unsigned long) tpnt->loadaddr;
			}
			else {
				*got_entry = (unsigned long) _dl_find_hash(strtab +
					sym->st_name, tpnt->symbol_scope, NULL, copyrel);
			}

			got_entry++;
			sym++;
		}
	}
}

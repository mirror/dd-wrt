/* vi: set sw=4 ts=4: */
/* m68k ELF shared library loader suppport
 *
 * Copyright (c) 1994-2000 Eric Youngdale, Peter MacDonald, 
 *				David Engel, Hongjiu Lu and Mitch D'Souza
 * Adapted to ELF/68k by Andreas Schwab.
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
static const char *_dl_reltypes[] =
{
  "R_68K_NONE",
  "R_68K_32", "R_68K_16", "R_68K_8",
  "R_68K_PC32", "R_68K_PC16", "R_68K_PC8",
  "R_68K_GOT32", "R_68K_GOT16", "R_68K_GOT8",
  "R_68K_GOT32O", "R_68K_GOT16O", "R_68K_GOT8O",
  "R_68K_PLT32", "R_68K_PLT16", "R_68K_PLT8",
  "R_68K_PLT32O", "R_68K_PLT16O", "R_68K_PLT8O",
  "R_68K_COPY", "R_68K_GLOB_DAT", "R_68K_JMP_SLOT", "R_68K_RELATIVE",
  "R_68K_NUM"
};
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


unsigned int _dl_linux_resolver (int dummy1, int dummy2, 
	struct elf_resolve *tpnt, int reloc_entry)
{
  int reloc_type;
  Elf32_Rela *this_reloc;
  char *strtab;
  Elf32_Sym *symtab;
  char *rel_addr;
  int symtab_index;
  char *new_addr;
  char **got_addr;
  unsigned int instr_addr;

  rel_addr = tpnt->loadaddr + tpnt->dynamic_info[DT_JMPREL];
  this_reloc = (Elf32_Rela *) (rel_addr + reloc_entry);
  reloc_type = ELF32_R_TYPE (this_reloc->r_info);
  symtab_index = ELF32_R_SYM (this_reloc->r_info);

  symtab = (Elf32_Sym *) (tpnt->dynamic_info[DT_SYMTAB]
				 + tpnt->loadaddr);
  strtab = (char *) (tpnt->dynamic_info[DT_STRTAB] + tpnt->loadaddr);


  if (reloc_type != R_68K_JMP_SLOT)
    {
      _dl_dprintf (2, "%s: incorrect relocation type in jump relocations\n",
		    _dl_progname);
      _dl_exit (1);
    }

  /* Address of jump instruction to fix up.  */
  instr_addr = (int) this_reloc->r_offset + (int) tpnt->loadaddr;
  got_addr = (char **) instr_addr;

#ifdef __SUPPORT_LD_DEBUG__
  if (_dl_debug_symbols) {
	  _dl_dprintf (2, "Resolving symbol %s\n", strtab + symtab[symtab_index].st_name);
  }
#endif

  /* Get the address of the GOT entry.  */
  new_addr = _dl_find_hash (strtab + symtab[symtab_index].st_name,
			    tpnt->symbol_scope, tpnt, resolver);
  if (!new_addr)
    {
      _dl_dprintf (2, "%s: can't resolve symbol '%s'\n",
		    _dl_progname, strtab + symtab[symtab_index].st_name);
      _dl_exit (1);
    }
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

  return (unsigned int) new_addr;
}

void
_dl_parse_lazy_relocation_information (struct elf_resolve *tpnt,
                       unsigned long rel_addr, unsigned long rel_size, int type)
{
  int i;
  char *strtab;
  int reloc_type;
  int symtab_index;
  Elf32_Sym *symtab;
  Elf32_Rela *rpnt;
  unsigned int *reloc_addr;

  /* Now parse the relocation information.  */
  rpnt = (Elf32_Rela *) (rel_addr + tpnt->loadaddr);
  rel_size = rel_size / sizeof (Elf32_Rela);

  symtab = (Elf32_Sym *) (tpnt->dynamic_info[DT_SYMTAB]
				 + tpnt->loadaddr);
  strtab = (char *) (tpnt->dynamic_info[DT_STRTAB] + tpnt->loadaddr);

  for (i = 0; i < rel_size; i++, rpnt++)
    {
      reloc_addr = (int *) (tpnt->loadaddr + (int) rpnt->r_offset);
      reloc_type = ELF32_R_TYPE (rpnt->r_info);
      symtab_index = ELF32_R_SYM (rpnt->r_info);

      /* When the dynamic linker bootstrapped itself, it resolved some symbols.
         Make sure we do not do them again.  */
      if (tpnt->libtype == program_interpreter
	  && (!symtab_index
	      || _dl_symbol (strtab + symtab[symtab_index].st_name)))
	continue;

      switch (reloc_type)
	{
	case R_68K_NONE:
	  break;
	case R_68K_JMP_SLOT:
	  *reloc_addr += (unsigned int) tpnt->loadaddr;
	  break;
	default:
	  _dl_dprintf (2, "%s: (LAZY) can't handle reloc type ", _dl_progname);
#if defined (__SUPPORT_LD_DEBUG__)
	  _dl_dprintf (2, "%s ", _dl_reltypes[reloc_type]);
#endif
	  if (symtab_index)
	    _dl_dprintf (2, "'%s'", strtab + symtab[symtab_index].st_name);
	  _dl_dprintf (2, "\n");
	  _dl_exit (1);
	}
    }
}

int 
_dl_parse_relocation_information (struct elf_resolve *tpnt,
                  unsigned long rel_addr, unsigned long rel_size, int type)
{
  int i;
  char *strtab;
  int reloc_type;
  int goof = 0;
  Elf32_Sym *symtab;
  Elf32_Rela *rpnt;
  unsigned int *reloc_addr;
  unsigned int symbol_addr;
  int symtab_index;
  /* Now parse the relocation information */

  rpnt = (Elf32_Rela *) (rel_addr + tpnt->loadaddr);
  rel_size = rel_size / sizeof (Elf32_Rela);

  symtab = (Elf32_Sym *) (tpnt->dynamic_info[DT_SYMTAB]
				 + tpnt->loadaddr);
  strtab = (char *) (tpnt->dynamic_info[DT_STRTAB] + tpnt->loadaddr);

  for (i = 0; i < rel_size; i++, rpnt++)
    {
      reloc_addr = (int *) (tpnt->loadaddr + (int) rpnt->r_offset);
      reloc_type = ELF32_R_TYPE (rpnt->r_info);
      symtab_index = ELF32_R_SYM (rpnt->r_info);
      symbol_addr = 0;

      if (tpnt->libtype == program_interpreter
	  && (!symtab_index
	      || _dl_symbol (strtab + symtab[symtab_index].st_name)))
	continue;

      if (symtab_index)
	{
	  symbol_addr = (unsigned int)
	    _dl_find_hash (strtab + symtab[symtab_index].st_name,
			   tpnt->symbol_scope,
			   reloc_type == R_68K_JMP_SLOT ? tpnt : NULL, symbolrel);

	  /* We want to allow undefined references to weak symbols -
	     this might have been intentional.  We should not be
	     linking local symbols here, so all bases should be
	     covered.  */
	  if (!symbol_addr
	      && ELF32_ST_BIND (symtab[symtab_index].st_info) == STB_GLOBAL)
	    {
	      _dl_dprintf (2, "%s: can't resolve symbol '%s'\n",
			    _dl_progname, strtab + symtab[symtab_index].st_name);
	      goof++;
	    }
	}
      switch (reloc_type)
	{
	case R_68K_NONE:
	  break;
	case R_68K_8:
	  *(char *) reloc_addr = symbol_addr + rpnt->r_addend;
	  break;
	case R_68K_16:
	  *(short *) reloc_addr = symbol_addr + rpnt->r_addend;
	  break;
	case R_68K_32:
	  *reloc_addr = symbol_addr + rpnt->r_addend;
	  break;
	case R_68K_PC8:
	  *(char *) reloc_addr = (symbol_addr + rpnt->r_addend
				  - (unsigned int) reloc_addr);
	  break;
	case R_68K_PC16:
	  *(short *) reloc_addr = (symbol_addr + rpnt->r_addend
				   - (unsigned int) reloc_addr);
	  break;
	case R_68K_PC32:
	  *reloc_addr = (symbol_addr + rpnt->r_addend
			 - (unsigned int) reloc_addr);
	  break;
	case R_68K_GLOB_DAT:
	case R_68K_JMP_SLOT:
	  *reloc_addr = symbol_addr;
	  break;
	case R_68K_RELATIVE:
	  *reloc_addr = ((unsigned int) tpnt->loadaddr
			 /* Compatibility kludge.  */
			 + (rpnt->r_addend ? : *reloc_addr));
	  break;
	case R_68K_COPY:
#if 0 /* Do this later.  */
	  _dl_dprintf (2, "Doing copy");
	  if (symtab_index)
	    _dl_dprintf (2, " for symbol %s",
			  strtab + symtab[symtab_index].st_name);
	  _dl_dprintf (2, "\n");
	  _dl_memcpy ((void *) symtab[symtab_index].st_value,
		      (void *) symbol_addr,
		      symtab[symtab_index].st_size);
#endif
	  break;
	default:
	  _dl_dprintf (2, "%s: can't handle reloc type ", _dl_progname);
#if defined (__SUPPORT_LD_DEBUG__)
	  _dl_dprintf (2, "%s ", _dl_reltypes[reloc_type]);
#endif
	  if (symtab_index)
	    _dl_dprintf (2, "'%s'", strtab + symtab[symtab_index].st_name);
	  _dl_dprintf (2, "\n");
	  _dl_exit (1);
	}

    }
  return goof;
}

/* This is done as a separate step, because there are cases where
   information is first copied and later initialized.  This results in
   the wrong information being copied.  Someone at Sun was complaining about
   a bug in the handling of _COPY by SVr4, and this may in fact be what he
   was talking about.  Sigh.  */

/* No, there are cases where the SVr4 linker fails to emit COPY relocs
   at all.  */

int 
_dl_parse_copy_information (struct dyn_elf *xpnt, unsigned long rel_addr,
			    unsigned long rel_size, int type)
{
  int i;
  char *strtab;
  int reloc_type;
  int goof = 0;
  Elf32_Sym *symtab;
  Elf32_Rela *rpnt;
  unsigned int *reloc_addr;
  unsigned int symbol_addr;
  struct elf_resolve *tpnt;
  int symtab_index;
  /* Now parse the relocation information */

  tpnt = xpnt->dyn;

  rpnt = (Elf32_Rela *) (rel_addr + tpnt->loadaddr);
  rel_size = rel_size / sizeof (Elf32_Rela);

  symtab = (Elf32_Sym *) (tpnt->dynamic_info[DT_SYMTAB]
				 + tpnt->loadaddr);
  strtab = (char *) (tpnt->dynamic_info[DT_STRTAB] + tpnt->loadaddr);

  for (i = 0; i < rel_size; i++, rpnt++)
    {
      reloc_addr = (int *) (tpnt->loadaddr + (int) rpnt->r_offset);
      reloc_type = ELF32_R_TYPE (rpnt->r_info);
      if (reloc_type != R_68K_COPY)
	continue;
      symtab_index = ELF32_R_SYM (rpnt->r_info);
      symbol_addr = 0;
      if (tpnt->libtype == program_interpreter
	  && (!symtab_index
	      || _dl_symbol (strtab + symtab[symtab_index].st_name)))
	continue;
      if (symtab_index)
	{
	  symbol_addr = (unsigned int)
	    _dl_find_hash (strtab + symtab[symtab_index].st_name,
			   xpnt->next, NULL, copyrel);
	  if (!symbol_addr)
	    {
	      _dl_dprintf (2, "%s: can't resolve symbol '%s'\n",
			    _dl_progname, strtab + symtab[symtab_index].st_name);
	      goof++;
	    }
	}
      if (!goof)
      _dl_memcpy ((void *) symtab[symtab_index].st_value, (void *) symbol_addr,
		  symtab[symtab_index].st_size);
    }
  return goof;
}

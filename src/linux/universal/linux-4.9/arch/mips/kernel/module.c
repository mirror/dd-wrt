/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Copyright (C) 2001 Rusty Russell.
 *  Copyright (C) 2003, 2004 Ralf Baechle (ralf@linux-mips.org)
 *  Copyright (C) 2005 Thiemo Seufer
 */

#undef DEBUG

#include <linux/extable.h>
#include <linux/moduleloader.h>
#include <linux/elf.h>
#include <linux/mm.h>
#include <linux/numa.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/jump_label.h>

#include <asm/pgtable.h>	/* MODULE_START */

struct mips_hi16 {
	struct mips_hi16 *next;
	Elf_Addr *addr;
	Elf_Addr value;
};

static LIST_HEAD(dbe_list);
static DEFINE_SPINLOCK(dbe_lock);

/*
 * Get the potential max trampolines size required of the init and
 * non-init sections. Only used if we cannot find enough contiguous
 * physically mapped memory to put the module into.
 */
static unsigned int
get_plt_size(const Elf_Ehdr *hdr, const Elf_Shdr *sechdrs,
             const char *secstrings, unsigned int symindex, bool is_init)
{
	unsigned long ret = 0;
	unsigned int i, j;
	Elf_Sym *syms;

	/* Everything marked ALLOC (this includes the exported symbols) */
	for (i = 1; i < hdr->e_shnum; ++i) {
		unsigned int info = sechdrs[i].sh_info;

		if (sechdrs[i].sh_type != SHT_REL
		    && sechdrs[i].sh_type != SHT_RELA)
			continue;

		/* Not a valid relocation section? */
		if (info >= hdr->e_shnum)
			continue;

		/* Don't bother with non-allocated sections */
		if (!(sechdrs[info].sh_flags & SHF_ALLOC))
			continue;

		/* If it's called *.init*, and we're not init, we're
                   not interested */
		if ((strstr(secstrings + sechdrs[i].sh_name, ".init") != 0)
		    != is_init)
			continue;

		syms = (Elf_Sym *) sechdrs[symindex].sh_addr;
		if (sechdrs[i].sh_type == SHT_REL) {
			Elf_Mips_Rel *rel = (void *) sechdrs[i].sh_addr;
			unsigned int size = sechdrs[i].sh_size / sizeof(*rel);

			for (j = 0; j < size; ++j) {
				Elf_Sym *sym;

				if (ELF_MIPS_R_TYPE(rel[j]) != R_MIPS_26)
					continue;

				sym = syms + ELF_MIPS_R_SYM(rel[j]);
				if (!is_init && sym->st_shndx != SHN_UNDEF)
					continue;

				ret += 4 * sizeof(int);
			}
		} else {
			Elf_Mips_Rela *rela = (void *) sechdrs[i].sh_addr;
			unsigned int size = sechdrs[i].sh_size / sizeof(*rela);

			for (j = 0; j < size; ++j) {
				Elf_Sym *sym;

				if (ELF_MIPS_R_TYPE(rela[j]) != R_MIPS_26)
					continue;

				sym = syms + ELF_MIPS_R_SYM(rela[j]);
				if (!is_init && sym->st_shndx != SHN_UNDEF)
					continue;

				ret += 4 * sizeof(int);
			}
		}
	}

	return ret;
}

#ifndef MODULE_START
static void *alloc_phys(unsigned long size)
{
	unsigned order;
	struct page *page;
	struct page *p;

	size = PAGE_ALIGN(size);
	order = get_order(size);

	page = alloc_pages(GFP_KERNEL | __GFP_NORETRY | __GFP_NOWARN |
			__GFP_THISNODE, order);
	if (!page)
		return NULL;

	split_page(page, order);

	/* mark all pages except for the last one */
	for (p = page; p + 1 < page + (size >> PAGE_SHIFT); ++p)
		set_bit(PG_owner_priv_1, &p->flags);

	for (p = page + (size >> PAGE_SHIFT); p < page + (1 << order); ++p)
		__free_page(p);

	return page_address(page);
}
#endif

static void free_phys(void *ptr)
{
	struct page *page;
	bool free;

	page = virt_to_page(ptr);
	do {
		free = test_and_clear_bit(PG_owner_priv_1, &page->flags);
		__free_page(page);
		page++;
	} while (free);
}


void *module_alloc(unsigned long size)
{
#ifdef MODULE_START
	return __vmalloc_node_range(size, 1, MODULE_START, MODULE_END,
				GFP_KERNEL, PAGE_KERNEL, 0, NUMA_NO_NODE,
				__builtin_return_address(0));
#else
	void *ptr;

	if (size == 0)
		return NULL;

	ptr = alloc_phys(size);

	/* If we failed to allocate physically contiguous memory,
	 * fall back to regular vmalloc. The module loader code will
	 * create jump tables to handle long jumps */
	if (!ptr)
		return vmalloc(size);

	return ptr;
#endif
}

static inline bool is_phys_addr(void *ptr)
{
#ifdef CONFIG_64BIT
	return (KSEGX((unsigned long)ptr) == CKSEG0);
#else
	return (KSEGX(ptr) == KSEG0);
#endif
}

/* Free memory returned from module_alloc */
void module_memfree(void *module_region)
{
	if (is_phys_addr(module_region))
		free_phys(module_region);
	else
		vfree(module_region);
}

static void *__module_alloc(int size, bool phys)
{
	void *ptr;

	if (phys)
		ptr = kmalloc(size, GFP_KERNEL);
	else
		ptr = vmalloc(size);
	return ptr;
}

static void __module_free(void *ptr)
{
	if (is_phys_addr(ptr))
		kfree(ptr);
	else
		vfree(ptr);
}

int module_frob_arch_sections(Elf_Ehdr *hdr, Elf_Shdr *sechdrs,
			      char *secstrings, struct module *mod)
{
	unsigned int symindex = 0;
	unsigned int core_size, init_size;
	int i;

	mod->arch.phys_plt_offset = 0;
	mod->arch.virt_plt_offset = 0;
	mod->arch.phys_plt_tbl = NULL;
	mod->arch.virt_plt_tbl = NULL;

	if (IS_ENABLED(CONFIG_64BIT))
		return 0;

	for (i = 1; i < hdr->e_shnum; i++)
		if (sechdrs[i].sh_type == SHT_SYMTAB)
			symindex = i;

	core_size = get_plt_size(hdr, sechdrs, secstrings, symindex, false);
	init_size = get_plt_size(hdr, sechdrs, secstrings, symindex, true);

	if ((core_size + init_size) == 0)
		return 0;

	mod->arch.phys_plt_tbl = __module_alloc(core_size + init_size, 1);
	if (!mod->arch.phys_plt_tbl)
		return -ENOMEM;

	mod->arch.virt_plt_tbl = __module_alloc(core_size + init_size, 0);
	if (!mod->arch.virt_plt_tbl) {
		__module_free(mod->arch.phys_plt_tbl);
		mod->arch.phys_plt_tbl = NULL;
		return -ENOMEM;
	}

	return 0;
}

int apply_r_mips_none(struct module *me, u32 *location, Elf_Addr v)
{
	return 0;
}

static int apply_r_mips_32_rel(struct module *me, u32 *location, Elf_Addr v)
{
	*location += v;

	return 0;
}

static Elf_Addr add_plt_entry_to(unsigned *plt_offset,
				 void *start, Elf_Addr v)
{
	unsigned *tramp = start + *plt_offset;
	*plt_offset += 4 * sizeof(int);

	/* adjust carry for addiu */
	if (v & 0x00008000)
		v += 0x10000;

	tramp[0] = 0x3c190000 | (v >> 16);      /* lui t9, hi16 */
	tramp[1] = 0x27390000 | (v & 0xffff);   /* addiu t9, t9, lo16 */
	tramp[2] = 0x03200008;                  /* jr t9 */
	tramp[3] = 0x00000000;                  /* nop */

	return (Elf_Addr) tramp;
}

static Elf_Addr add_plt_entry(struct module *me, void *location, Elf_Addr v)
{
	if (is_phys_addr(location))
		return add_plt_entry_to(&me->arch.phys_plt_offset,
				me->arch.phys_plt_tbl, v);
	else
		return add_plt_entry_to(&me->arch.virt_plt_offset,
				me->arch.virt_plt_tbl, v);

}

static int apply_r_mips_26_rel(struct module *me, u32 *location, Elf_Addr v)
{
	u32 ofs = *location & 0x03ffffff;

	if (v % 4) {
		pr_err("module %s: dangerous R_MIPS_26 REL relocation\n",
		       me->name);
		return -ENOEXEC;
	}

	if ((v & 0xf0000000) != (((unsigned long)location + 4) & 0xf0000000)) {
		v = add_plt_entry(me, location, v + (ofs << 2));
		if (!v) {
			pr_err("module %s: relocation overflow\n",
			       me->name);
			return -ENOEXEC;
		}
		ofs = 0;
	}

	*location = (*location & ~0x03ffffff) |
		    ((ofs + (v >> 2)) & 0x03ffffff);

	return 0;
}

static int apply_r_mips_hi16_rel(struct module *me, u32 *location, Elf_Addr v)
{
	struct mips_hi16 *n;

	/*
	 * We cannot relocate this one now because we don't know the value of
	 * the carry we need to add.  Save the information, and let LO16 do the
	 * actual relocation.
	 */
	n = kmalloc(sizeof *n, GFP_KERNEL);
	if (!n)
		return -ENOMEM;

	n->addr = (Elf_Addr *)location;
	n->value = v;
	n->next = me->arch.r_mips_hi16_list;
	me->arch.r_mips_hi16_list = n;

	return 0;
}

static void free_relocation_chain(struct mips_hi16 *l)
{
	struct mips_hi16 *next;

	while (l) {
		next = l->next;
		kfree(l);
		l = next;
	}
}

static int apply_r_mips_lo16_rel(struct module *me, u32 *location, Elf_Addr v)
{
	unsigned long insnlo = *location;
	struct mips_hi16 *l;
	Elf_Addr val, vallo;

	/* Sign extend the addend we extract from the lo insn.	*/
	vallo = ((insnlo & 0xffff) ^ 0x8000) - 0x8000;

	if (me->arch.r_mips_hi16_list != NULL) {
		l = me->arch.r_mips_hi16_list;
		while (l != NULL) {
			struct mips_hi16 *next;
			unsigned long insn;

			/*
			 * The value for the HI16 had best be the same.
			 */
			if (v != l->value)
				goto out_danger;

			/*
			 * Do the HI16 relocation.  Note that we actually don't
			 * need to know anything about the LO16 itself, except
			 * where to find the low 16 bits of the addend needed
			 * by the LO16.
			 */
			insn = *l->addr;
			val = ((insn & 0xffff) << 16) + vallo;
			val += v;

			/*
			 * Account for the sign extension that will happen in
			 * the low bits.
			 */
			val = ((val >> 16) + ((val & 0x8000) != 0)) & 0xffff;

			insn = (insn & ~0xffff) | val;
			*l->addr = insn;

			next = l->next;
			kfree(l);
			l = next;
		}

		me->arch.r_mips_hi16_list = NULL;
	}

	/*
	 * Ok, we're done with the HI16 relocs.	 Now deal with the LO16.
	 */
	val = v + vallo;
	insnlo = (insnlo & ~0xffff) | (val & 0xffff);
	*location = insnlo;

	return 0;

out_danger:
	free_relocation_chain(l);
	me->arch.r_mips_hi16_list = NULL;

	pr_err("module %s: dangerous R_MIPS_LO16 REL relocation\n", me->name);

	return -ENOEXEC;
}

static int apply_r_mips_pc_rel(struct module *me, u32 *location, Elf_Addr v,
			       unsigned bits)
{
	unsigned long mask = GENMASK(bits - 1, 0);
	unsigned long se_bits;
	long offset;

	if (v % 4) {
		pr_err("module %s: dangerous R_MIPS_PC%u REL relocation\n",
		       me->name, bits);
		return -ENOEXEC;
	}

	/* retrieve & sign extend implicit addend */
	offset = *location & mask;
	offset |= (offset & BIT(bits - 1)) ? ~mask : 0;

	offset += ((long)v - (long)location) >> 2;

	/* check the sign bit onwards are identical - ie. we didn't overflow */
	se_bits = (offset & BIT(bits - 1)) ? ~0ul : 0;
	if ((offset & ~mask) != (se_bits & ~mask)) {
		pr_err("module %s: relocation overflow\n", me->name);
		return -ENOEXEC;
	}

	*location = (*location & ~mask) | (offset & mask);

	return 0;
}

static int apply_r_mips_pc16_rel(struct module *me, u32 *location, Elf_Addr v)
{
	return apply_r_mips_pc_rel(me, location, v, 16);
}

static int apply_r_mips_pc21_rel(struct module *me, u32 *location, Elf_Addr v)
{
	return apply_r_mips_pc_rel(me, location, v, 21);
}

static int apply_r_mips_pc26_rel(struct module *me, u32 *location, Elf_Addr v)
{
	return apply_r_mips_pc_rel(me, location, v, 26);
}

static int (*reloc_handlers_rel[]) (struct module *me, u32 *location,
				Elf_Addr v) = {
	[R_MIPS_NONE]		= apply_r_mips_none,
	[R_MIPS_32]		= apply_r_mips_32_rel,
	[R_MIPS_26]		= apply_r_mips_26_rel,
	[R_MIPS_HI16]		= apply_r_mips_hi16_rel,
	[R_MIPS_LO16]		= apply_r_mips_lo16_rel,
	[R_MIPS_PC16]		= apply_r_mips_pc16_rel,
	[R_MIPS_PC21_S2]	= apply_r_mips_pc21_rel,
	[R_MIPS_PC26_S2]	= apply_r_mips_pc26_rel,
};

int apply_relocate(Elf_Shdr *sechdrs, const char *strtab,
		   unsigned int symindex, unsigned int relsec,
		   struct module *me)
{
	Elf_Mips_Rel *rel = (void *) sechdrs[relsec].sh_addr;
	int (*handler)(struct module *me, u32 *location, Elf_Addr v);
	Elf_Sym *sym;
	u32 *location;
	unsigned int i, type;
	Elf_Addr v;
	int res;

	pr_debug("Applying relocate section %u to %u\n", relsec,
	       sechdrs[relsec].sh_info);

	me->arch.r_mips_hi16_list = NULL;
	for (i = 0; i < sechdrs[relsec].sh_size / sizeof(*rel); i++) {
		/* This is where to make the change */
		location = (void *)sechdrs[sechdrs[relsec].sh_info].sh_addr
			+ rel[i].r_offset;
		/* This is the symbol it is referring to */
		sym = (Elf_Sym *)sechdrs[symindex].sh_addr
			+ ELF_MIPS_R_SYM(rel[i]);
		if (sym->st_value >= -MAX_ERRNO) {
			/* Ignore unresolved weak symbol */
			if (ELF_ST_BIND(sym->st_info) == STB_WEAK)
				continue;
			pr_warn("%s: Unknown symbol %s\n",
				me->name, strtab + sym->st_name);
			return -ENOENT;
		}

		type = ELF_MIPS_R_TYPE(rel[i]);

		if (type < ARRAY_SIZE(reloc_handlers_rel))
			handler = reloc_handlers_rel[type];
		else
			handler = NULL;

		if (!handler) {
			pr_err("%s: Unknown relocation type %u\n",
			       me->name, type);
			return -EINVAL;
		}

		v = sym->st_value;
		res = handler(me, location, v);
		if (res)
			return res;
	}

	/*
	 * Normally the hi16 list should be deallocated at this point.	A
	 * malformed binary however could contain a series of R_MIPS_HI16
	 * relocations not followed by a R_MIPS_LO16 relocation.  In that
	 * case, free up the list and return an error.
	 */
	if (me->arch.r_mips_hi16_list) {
		free_relocation_chain(me->arch.r_mips_hi16_list);
		me->arch.r_mips_hi16_list = NULL;

		return -ENOEXEC;
	}

	return 0;
}

/* Given an address, look for it in the module exception tables. */
const struct exception_table_entry *search_module_dbetables(unsigned long addr)
{
	unsigned long flags;
	const struct exception_table_entry *e = NULL;
	struct mod_arch_specific *dbe;

	spin_lock_irqsave(&dbe_lock, flags);
	list_for_each_entry(dbe, &dbe_list, dbe_list) {
		e = search_extable(dbe->dbe_start,
				   dbe->dbe_end - dbe->dbe_start, addr);
		if (e)
			break;
	}
	spin_unlock_irqrestore(&dbe_lock, flags);

	/* Now, if we found one, we are running inside it now, hence
	   we cannot unload the module, hence no refcnt needed. */
	return e;
}

/* Put in dbe list if necessary. */
int module_finalize(const Elf_Ehdr *hdr,
		    const Elf_Shdr *sechdrs,
		    struct module *me)
{
	const Elf_Shdr *s;
	char *secstrings = (void *)hdr + sechdrs[hdr->e_shstrndx].sh_offset;

	/* Make jump label nops. */
	jump_label_apply_nops(me);

	INIT_LIST_HEAD(&me->arch.dbe_list);
	for (s = sechdrs; s < sechdrs + hdr->e_shnum; s++) {
		if (strcmp("__dbe_table", secstrings + s->sh_name) != 0)
			continue;
		me->arch.dbe_start = (void *)s->sh_addr;
		me->arch.dbe_end = (void *)s->sh_addr + s->sh_size;
		spin_lock_irq(&dbe_lock);
		list_add(&me->arch.dbe_list, &dbe_list);
		spin_unlock_irq(&dbe_lock);
	}

	/* Get rid of the fixup trampoline if we're running the module
	 * from physically mapped address space */
	if (me->arch.phys_plt_offset == 0) {
		__module_free(me->arch.phys_plt_tbl);
		me->arch.phys_plt_tbl = NULL;
	}
	if (me->arch.virt_plt_offset == 0) {
		__module_free(me->arch.virt_plt_tbl);
		me->arch.virt_plt_tbl = NULL;
	}

	return 0;
}

void module_arch_freeing_init(struct module *mod)
{
	if (mod->state == MODULE_STATE_LIVE)
		return;

	if (mod->arch.phys_plt_tbl) {
		__module_free(mod->arch.phys_plt_tbl);
		mod->arch.phys_plt_tbl = NULL;
	}
	if (mod->arch.virt_plt_tbl) {
		__module_free(mod->arch.virt_plt_tbl);
		mod->arch.virt_plt_tbl = NULL;
	}
}

void module_arch_cleanup(struct module *mod)
{
	spin_lock_irq(&dbe_lock);
	list_del(&mod->arch.dbe_list);
	spin_unlock_irq(&dbe_lock);
}

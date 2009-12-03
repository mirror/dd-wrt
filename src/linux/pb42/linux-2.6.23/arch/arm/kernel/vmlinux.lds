

/*
 * Automatically generated C config: don't edit
 * Linux kernel version: 2.6.23
 * Thu Oct 11 15:50:07 2007
 */
/* ld script to make ARM Linux kernel
 * taken from the i386 version by Russell King
 * Written by Martin Mares <mj@atrey.karlin.mff.cuni.cz>
 */
/* Align . to a 8 byte boundary equals to maximum function alignment. */

/* .data section */
/* RODATA provided for backward compatibility.
 * All archs are supposed to use RO_DATA() */
/* .text section. Map to function alignment to avoid address changes
 * during second ld run in second ld pass when generating System.map */
/* sched.text is aling to function alignment to secure we have same
 * address even at second ld pass when generating System.map */
/* spinlock.text is aling to function alignment to secure we have same
 * address even at second ld pass when generating System.map */
  /* DWARF debug sections.
		Symbols in the DWARF debugging sections are relative to
		the beginning of the section so we begin them at 0.  */
  /* Stabs debugging sections.  */
/*
 *  linux/include/asm-arm/thread_info.h
 *
 *  Copyright (C) 2002 Russell King.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*
 * Allow us to mark functions as 'deprecated' and have gcc emit a nice
 * warning for each use, in hopes of speeding the functions removal.
 * Usage is:
 * 		int __deprecated foo(void)
 */
/*
 * Allow us to avoid 'defined but not used' warnings on functions and data,
 * as well as force them to be emitted to the assembly file.
 *
 * As of gcc 3.4, static functions that are not marked with attribute((used))
 * may be elided from the assembly file.  As of gcc 3.4, static data not so
 * marked will not be elided, but this may change in a future gcc version.
 *
 * NOTE: Because distributions shipped with a backported unit-at-a-time
 * compiler in gcc 3.3, we must define __used to be __attribute__((used))
 * for gcc >=3.3 instead of 3.4.
 *
 * In prior versions of gcc, such functions and data would be emitted, but
 * would be warned about except with attribute((unused)).
 *
 * Mark functions that are referenced only in inline assembly as __used so
 * the code is emitted even though it appears to be unreferenced.
 */
/*
 * From the GCC manual:
 *
 * Many functions have no effects except the return value and their
 * return value depends only on the parameters and/or global
 * variables.  Such a function can be subject to common subexpression
 * elimination and loop optimization just as an arithmetic operator
 * would be.
 * [...]
 */
/*
 * From the GCC manual:
 *
 * Many functions do not examine any values except their arguments,
 * and have no effects except the return value.  Basically this is
 * just slightly more strict class than the `pure' attribute above,
 * since function is not allowed to read global memory.
 *
 * Note that a function that has pointer arguments and examines the
 * data pointed to must _not_ be declared `const'.  Likewise, a
 * function that calls a non-`const' function usually must not be
 * `const'.  It does not make sense for a `const' function to return
 * `void'.
 */
/*
 * Tell gcc if a function is cold. The compiler will assume any path
 * directly leading to the call is unlikely.
 */
/*
 *  linux/include/asm-arm/fpstate.h
 *
 *  Copyright (C) 1995 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*
 * We use bit 30 of the preempt_count to indicate that kernel
 * preemption is occurring.  See include/asm-arm/hardirq.h.
 */
/*
 * thread information flags:
 *  TIF_SYSCALL_TRACE	- syscall trace active
 *  TIF_SIGPENDING	- signal pending
 *  TIF_NEED_RESCHED	- rescheduling necessary
 *  TIF_USEDFPU		- FPU was used by this task this quantum (SMP)
 *  TIF_POLLING_NRFLAG	- true if poll_idle() is polling TIF_NEED_RESCHED
 */
/*
 * Change these and you break ASM code in entry-common.S
 */
/*
 *  linux/include/asm-arm/memory.h
 *
 *  Copyright (C) 2000-2002 Russell King
 *  modification for nommu, Hyok S. Choi, 2004
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Note: this file should not be included by non-asm/.h files
 */
/*
 * Allow for constants defined here to be used from assembly code
 * by prepending the UL suffix only with actual C code compilation.
 */
/*
 * linux/include/asm-arm/arch-ixp4xx/memory.h
 *
 * Copyright (c) 2001-2004 MontaVista Software, Inc.
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* DO NOT EDIT!! - this file automatically generated
 *                 from .s file by awk -f s2h.awk
 */
/*  Size definitions
 *  Copyright (C) ARM Limited 1998. All rights reserved.
 */
/* handy sizes */
/*         END */
/*
 * Physical DRAM offset.
 */
/*
 * Virtual view <-> DMA view memory address translations
 * virt_to_bus: Used to translate the virtual address to an
 *		address suitable to be passed to set_dma_addr
 * bus_to_virt: Used to convert an address for DMA operations
 *		to an address that the kernel can use.
 *
 * These are dummies for now.
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* DO NOT EDIT!! - this file automatically generated
 *                 from .s file by awk -f s2h.awk
 */
/*  Size definitions
 *  Copyright (C) ARM Limited 1998. All rights reserved.
 */
/*         END */
/*
 * TASK_SIZE - the maximum size of a user space task.
 * TASK_UNMAPPED_BASE - the lower boundary of the mmap VM area
 */
/*
 * The maximum size of a 26-bit user space task.
 */
/*
 * Page offset: 3GB
 */
/*
 * The module space lives between the addresses given by TASK_SIZE
 * and PAGE_OFFSET - it must be within 32MB of the kernel text.
 */
/*
 * The XIP kernel gets mapped at the bottom of the module vm area.
 * Since we use sections to map it, this macro replaces the physical address
 * with its virtual address while keeping offset from the base section.
 */
/*
 * Allow 16MB-aligned ioremap pages
 */
/*
 * Size of DMA-consistent memory region.  Must be multiple of 2M,
 * between 2MB and 14MB inclusive.
 */
/*
 * Physical vs virtual RAM address space conversion.  These are
 * private definitions which should NOT be used outside memory.h
 * files.  Use virt_to_phys/phys_to_virt/__pa/__va instead.
 */
/*
 * Convert a physical address to a Page Frame Number and back
 */
OUTPUT_ARCH(arm)
ENTRY(stext)
jiffies = jiffies_64 + 4;
SECTIONS
{
 . = (0xc0000000) + 0x00008000;
 .text.head : {
  _stext = .;
  _sinittext = .;
  *(.text.head)
 }
 .init : { /* Init code and data		*/
   *(.init.text)
  _einittext = .;
  __proc_info_begin = .;
   *(.proc.info.init)
  __proc_info_end = .;
  __arch_info_begin = .;
   *(.arch.info.init)
  __arch_info_end = .;
  __tagtable_begin = .;
   *(.taglist.init)
  __tagtable_end = .;
  . = ALIGN(16);
  __setup_start = .;
   *(.init.setup)
  __setup_end = .;
  __early_begin = .;
   *(.early_param.init)
  __early_end = .;
  __initcall_start = .;
   *(.initcall0.init) *(.initcall0s.init) *(.initcall1.init) *(.initcall1s.init) *(.initcall2.init) *(.initcall2s.init) *(.initcall3.init) *(.initcall3s.init) *(.initcall4.init) *(.initcall4s.init) *(.initcall5.init) *(.initcall5s.init) *(.initcallrootfs.init) *(.initcall6.init) *(.initcall6s.init) *(.initcall7.init) *(.initcall7s.init)
  __initcall_end = .;
  __con_initcall_start = .;
   *(.con_initcall.init)
  __con_initcall_end = .;
  __security_initcall_start = .;
   *(.security_initcall.init)
  __security_initcall_end = .;
  . = ALIGN(4096);
  __per_cpu_start = .;
   *(.data.percpu)
   *(.data.percpu.shared_aligned)
  __per_cpu_end = .;
  __init_begin = _stext;
  *(.init.data)
  . = ALIGN(4096);
  __init_end = .;
 }
 /DISCARD/ : { /* Exit code and data		*/
  *(.exit.text)
  *(.exit.data)
  *(.exitcall.exit)
 }
 .text : { /* Real text segment		*/
  _text = .; /* Text and read-only data	*/
   __exception_text_start = .;
   *(.exception.text)
   __exception_text_end = .;
   . = ALIGN(8); *(.text) *(.text.init.refok)
   . = ALIGN(8); __sched_text_start = .; *(.sched.text) __sched_text_end = .;
   . = ALIGN(8); __lock_text_start = .; *(.spinlock.text) __lock_text_end = .;
   *(.fixup)
   *(.gnu.warning)
   *(.rodata)
   *(.rodata.*)
   *(.glue_7)
   *(.glue_7t)
  *(.got) /* Global offset table		*/
 }
 . = ALIGN((4096)); .rodata : AT(ADDR(.rodata) - 0) { __start_rodata = .; *(.rodata) *(.rodata.*) *(__vermagic) } .rodata1 : AT(ADDR(.rodata1) - 0) { *(.rodata1) } .pci_fixup : AT(ADDR(.pci_fixup) - 0) { __start_pci_fixups_early = .; *(.pci_fixup_early) __end_pci_fixups_early = .; __start_pci_fixups_header = .; *(.pci_fixup_header) __end_pci_fixups_header = .; __start_pci_fixups_final = .; *(.pci_fixup_final) __end_pci_fixups_final = .; __start_pci_fixups_enable = .; *(.pci_fixup_enable) __end_pci_fixups_enable = .; __start_pci_fixups_resume = .; *(.pci_fixup_resume) __end_pci_fixups_resume = .; } .rio_route : AT(ADDR(.rio_route) - 0) { __start_rio_route_ops = .; *(.rio_route_ops) __end_rio_route_ops = .; } __ksymtab : AT(ADDR(__ksymtab) - 0) { __start___ksymtab = .; *(__ksymtab) __stop___ksymtab = .; } __ksymtab_gpl : AT(ADDR(__ksymtab_gpl) - 0) { __start___ksymtab_gpl = .; *(__ksymtab_gpl) __stop___ksymtab_gpl = .; } __ksymtab_unused : AT(ADDR(__ksymtab_unused) - 0) { __start___ksymtab_unused = .; *(__ksymtab_unused) __stop___ksymtab_unused = .; } __ksymtab_unused_gpl : AT(ADDR(__ksymtab_unused_gpl) - 0) { __start___ksymtab_unused_gpl = .; *(__ksymtab_unused_gpl) __stop___ksymtab_unused_gpl = .; } __ksymtab_gpl_future : AT(ADDR(__ksymtab_gpl_future) - 0) { __start___ksymtab_gpl_future = .; *(__ksymtab_gpl_future) __stop___ksymtab_gpl_future = .; } __kcrctab : AT(ADDR(__kcrctab) - 0) { __start___kcrctab = .; *(__kcrctab) __stop___kcrctab = .; } __kcrctab_gpl : AT(ADDR(__kcrctab_gpl) - 0) { __start___kcrctab_gpl = .; *(__kcrctab_gpl) __stop___kcrctab_gpl = .; } __kcrctab_unused : AT(ADDR(__kcrctab_unused) - 0) { __start___kcrctab_unused = .; *(__kcrctab_unused) __stop___kcrctab_unused = .; } __kcrctab_unused_gpl : AT(ADDR(__kcrctab_unused_gpl) - 0) { __start___kcrctab_unused_gpl = .; *(__kcrctab_unused_gpl) __stop___kcrctab_unused_gpl = .; } __kcrctab_gpl_future : AT(ADDR(__kcrctab_gpl_future) - 0) { __start___kcrctab_gpl_future = .; *(__kcrctab_gpl_future) __stop___kcrctab_gpl_future = .; } __ksymtab_strings : AT(ADDR(__ksymtab_strings) - 0) { *(__ksymtab_strings) } __param : AT(ADDR(__param) - 0) { __start___param = .; *(__param) __stop___param = .; __end_rodata = .; } . = ALIGN((4096));
 _etext = .; /* End of text and rodata section */
 . = ALIGN(8192);
 __data_loc = .;
 .data : AT(__data_loc) {
  __data_start = .; /* address in memory */
  /*
		 * first, the init task union, aligned
		 * to an 8192 byte boundary.
		 */
  *(.data.init_task)
  . = ALIGN(4096);
  __nosave_begin = .;
  *(.data.nosave)
  . = ALIGN(4096);
  __nosave_end = .;
  /*
		 * then the cacheline aligned data
		 */
  . = ALIGN(32);
  *(.data.cacheline_aligned)
  /*
		 * The exception fixup table (might need resorting at runtime)
		 */
  . = ALIGN(32);
  __start___ex_table = .;
  *(__ex_table)
  __stop___ex_table = .;
  /*
		 * and the usual data section
		 */
  *(.data) *(.data.init.refok)
  CONSTRUCTORS
  _edata = .;
 }
 _edata_loc = __data_loc + SIZEOF(.data);
 .bss : {
  __bss_start = .; /* BSS				*/
  *(.bss)
  *(COMMON)
  _end = .;
 }
     /* Stabs debugging sections.	*/
 .stab 0 : { *(.stab) }
 .stabstr 0 : { *(.stabstr) }
 .stab.excl 0 : { *(.stab.excl) }
 .stab.exclstr 0 : { *(.stab.exclstr) }
 .stab.index 0 : { *(.stab.index) }
 .stab.indexstr 0 : { *(.stab.indexstr) }
 .comment 0 : { *(.comment) }
}
/*
 * These must never be empty
 * If you have to comment these two assert statements out, your
 * binutils is too old (for other reasons as well)
 */
ASSERT((__proc_info_end - __proc_info_begin), "missing CPU support")
ASSERT((__arch_info_end - __arch_info_begin), "no machine record defined")

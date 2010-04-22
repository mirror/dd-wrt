

/*
 * Automatically generated C config: don't edit
 * Linux kernel version: 2.6.23.17
 * Thu Apr 22 07:55:12 2010
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
/* bytes per L1 cache line */
ENTRY(_stext)
OUTPUT_ARCH(powerpc:common)
jiffies = jiffies_64 + 4;
SECTIONS
{
 /* Sections to be discarded. */
 /DISCARD/ : {
 *(.exitcall.exit)
 *(.exit.data)
 }
 . = 0xc0000000;
/*
 * Text, read only data and other permanent read-only sections
 */
 /* Text and gots */
 .text : {
  _text = .;
  . = ALIGN(8); *(.text) *(.text.init.refok)
  . = ALIGN(8); __sched_text_start = .; *(.sched.text) __sched_text_end = .;
  . = ALIGN(8); __lock_text_start = .; *(.spinlock.text) __lock_text_end = .;
  . = ALIGN(8); __kprobes_text_start = .; *(.kprobes.text) __kprobes_text_end = .;
  *(.fixup)
  *(.got1)
  __got2_start = .;
  *(.got2)
  __got2_end = .;
  . = ALIGN(4096);
  _etext = .;
  PROVIDE(etext = .);
 }
 /* Read-only data */
 . = ALIGN((4096)); .rodata : AT(ADDR(.rodata) - 0) { __start_rodata = .; *(.rodata) *(.rodata.*) *(__vermagic) } .rodata1 : AT(ADDR(.rodata1) - 0) { *(.rodata1) } .pci_fixup : AT(ADDR(.pci_fixup) - 0) { __start_pci_fixups_early = .; *(.pci_fixup_early) __end_pci_fixups_early = .; __start_pci_fixups_header = .; *(.pci_fixup_header) __end_pci_fixups_header = .; __start_pci_fixups_final = .; *(.pci_fixup_final) __end_pci_fixups_final = .; __start_pci_fixups_enable = .; *(.pci_fixup_enable) __end_pci_fixups_enable = .; __start_pci_fixups_resume = .; *(.pci_fixup_resume) __end_pci_fixups_resume = .; } .rio_route : AT(ADDR(.rio_route) - 0) { __start_rio_route_ops = .; *(.rio_route_ops) __end_rio_route_ops = .; } __ksymtab : AT(ADDR(__ksymtab) - 0) { __start___ksymtab = .; *(__ksymtab) __stop___ksymtab = .; } __ksymtab_gpl : AT(ADDR(__ksymtab_gpl) - 0) { __start___ksymtab_gpl = .; *(__ksymtab_gpl) __stop___ksymtab_gpl = .; } __ksymtab_unused : AT(ADDR(__ksymtab_unused) - 0) { __start___ksymtab_unused = .; *(__ksymtab_unused) __stop___ksymtab_unused = .; } __ksymtab_unused_gpl : AT(ADDR(__ksymtab_unused_gpl) - 0) { __start___ksymtab_unused_gpl = .; *(__ksymtab_unused_gpl) __stop___ksymtab_unused_gpl = .; } __ksymtab_gpl_future : AT(ADDR(__ksymtab_gpl_future) - 0) { __start___ksymtab_gpl_future = .; *(__ksymtab_gpl_future) __stop___ksymtab_gpl_future = .; } __kcrctab : AT(ADDR(__kcrctab) - 0) { __start___kcrctab = .; *(__kcrctab) __stop___kcrctab = .; } __kcrctab_gpl : AT(ADDR(__kcrctab_gpl) - 0) { __start___kcrctab_gpl = .; *(__kcrctab_gpl) __stop___kcrctab_gpl = .; } __kcrctab_unused : AT(ADDR(__kcrctab_unused) - 0) { __start___kcrctab_unused = .; *(__kcrctab_unused) __stop___kcrctab_unused = .; } __kcrctab_unused_gpl : AT(ADDR(__kcrctab_unused_gpl) - 0) { __start___kcrctab_unused_gpl = .; *(__kcrctab_unused_gpl) __stop___kcrctab_unused_gpl = .; } __kcrctab_gpl_future : AT(ADDR(__kcrctab_gpl_future) - 0) { __start___kcrctab_gpl_future = .; *(__kcrctab_gpl_future) __stop___kcrctab_gpl_future = .; } __ksymtab_strings : AT(ADDR(__ksymtab_strings) - 0) { *(__ksymtab_strings) } __param : AT(ADDR(__param) - 0) { __start___param = .; *(__param) __stop___param = .; __end_rodata = .; } . = ALIGN((4096));

 /* Exception & bug tables */
 __ex_table : {
  __start___ex_table = .;
  *(__ex_table)
  __stop___ex_table = .;
 }

 .notes : AT(ADDR(.notes) - 0) { __start_notes = .; *(.note.*) __stop_notes = .; }

 . = ALIGN(8); __bug_table : AT(ADDR(__bug_table) - 0) { __start___bug_table = .; *(__bug_table) __stop___bug_table = .; }

/*
 * Init sections discarded at runtime
 */
 . = ALIGN(4096);
 __init_begin = .;

 .init.text : {
  _sinittext = .;
  *(.init.text)
  _einittext = .;
 }

 /* .exit.text is discarded at runtime, not link time,
	 * to deal with references from __bug_table
	 */
 .exit.text : { *(.exit.text) }

 .init.data : {
  *(.init.data);
  __vtop_table_begin = .;
  *(.vtop_fixup);
  __vtop_table_end = .;
  __ptov_table_begin = .;
  *(.ptov_fixup);
  __ptov_table_end = .;





 }

 . = ALIGN(16);
 .init.setup : {
  __setup_start = .;
  *(.init.setup)
  __setup_end = .;
 }

 .initcall.init : {
  __initcall_start = .;
  *(.initcall0.init) *(.initcall0s.init) *(.initcall1.init) *(.initcall1s.init) *(.initcall2.init) *(.initcall2s.init) *(.initcall3.init) *(.initcall3s.init) *(.initcall4.init) *(.initcall4s.init) *(.initcall5.init) *(.initcall5s.init) *(.initcallrootfs.init) *(.initcall6.init) *(.initcall6s.init) *(.initcall7.init) *(.initcall7s.init)
  __initcall_end = .;
  }

 .con_initcall.init : {
  __con_initcall_start = .;
  *(.con_initcall.init)
  __con_initcall_end = .;
 }

 .security_initcall.init : AT(ADDR(.security_initcall.init) - 0) { __security_initcall_start = .; *(.security_initcall.init) __security_initcall_end = .; }

 . = ALIGN(8);
 __ftr_fixup : {
  __start___ftr_fixup = .;
  *(__ftr_fixup)
  __stop___ftr_fixup = .;
 }
 . = ALIGN(4096);
 .data.percpu : {
  __per_cpu_start = .;
  *(.data.percpu)
  *(.data.percpu.shared_aligned)
  __per_cpu_end = .;
 }
 . = ALIGN(8);
 .machine.desc : {
  __machine_desc_start = . ;
  *(.machine.desc)
  __machine_desc_end = . ;
 }
 /* freed after init ends here */
 . = ALIGN(4096);
 __init_end = .;
/*
 * And now the various read/write data
 */
 . = ALIGN(4096);
 _sdata = .;
 .data :
 {
  *(.data) *(.data.init.refok)
  *(.sdata)
  *(.got.plt) *(.got)
 }
 . = ALIGN(4096);
 _edata = .;
 PROVIDE(edata = .);
 /* The initial task and kernel stack */
 . = ALIGN(8192);
 .data.init_task : {
  *(.data.init_task)
 }
 . = ALIGN(4096);
 .data.page_aligned : {
  *(.data.page_aligned)
 }
 .data.cacheline_aligned : {
  *(.data.cacheline_aligned)
 }
 . = ALIGN((1 << 5));
 .data.read_mostly : {
  *(.data.read_mostly)
 }
 . = ALIGN(4096);
 __data_nosave : {
  __nosave_begin = .;
  *(.data.nosave)
  . = ALIGN(4096);
  __nosave_end = .;
 }
/*
 * And finally the bss
 */
 .bss : {
  __bss_start = .;
  *(.sbss) *(.scommon)
  *(.dynbss)
  *(.bss)
  *(COMMON)
  __bss_stop = .;
 }
 . = ALIGN(4096);
 _end = . ;
 PROVIDE(end = .);
}

OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
  . = 0x81400000;
  .text      :
  {
    _ftext = . ;
    _start_code_data = .;
    *(.text)
    *(.rodata*)
    lzma_start = .;
    kernel.o
    lzma_end = .;
    *(.data)
    *(.reginfo) /* cliff - ? */
    _end_code_data = .;
  } =0

  .bss       :
  {
    . = ALIGN(4);
    bss_start = .;
    *(.bss)
    . = ALIGN(4);
    bss_end = .;
  }

}

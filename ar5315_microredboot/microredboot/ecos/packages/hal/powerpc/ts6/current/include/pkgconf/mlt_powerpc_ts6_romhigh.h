// eCos memory layout - Fri Oct 20 10:36:41 2000

// This is a generated file - do not edit

#ifndef __ASSEMBLER__
#include <cyg/infra/cyg_type.h>
#include <stddef.h>

#endif
#define CYGMEM_REGION_ram      (0x00000000)
#define CYGMEM_REGION_ram_SIZE (0x04000000)
#define CYGMEM_REGION_ram_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)
#define CYGMEM_REGION_rom      (0xff800000)
#define CYGMEM_REGION_rom_SIZE (0x00800000)
#define CYGMEM_REGION_rom_ATTR (CYGMEM_REGION_ATTR_R)
#ifndef __ASSEMBLER__
extern char CYG_LABEL_NAME (__reserved_vsr_table) [];
#endif
#define CYGMEM_SECTION_reserved_vsr_table (CYG_LABEL_NAME (__reserved_vsr_table))
#define CYGMEM_SECTION_reserved_vsr_table_SIZE (0x200)
#ifndef __ASSEMBLER__
extern char CYG_LABEL_NAME (__heap1) [];
#endif
#define CYGMEM_SECTION_heap1 (CYG_LABEL_NAME (__heap1))

#if 0
#define CYGMEM_SECTION_heap1_SIZE (0x100000 - (size_t) CYG_LABEL_NAME (__heap1))
#else // Attempt to make more RAM usable in RedBoot ROM version
#define CYGMEM_SECTION_heap1_SIZE (0x04000000 - (size_t) CYG_LABEL_NAME (__heap1))
#endif

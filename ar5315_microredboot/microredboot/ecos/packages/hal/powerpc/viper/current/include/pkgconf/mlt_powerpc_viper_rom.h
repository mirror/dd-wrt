// eCos memory layout - Thu May 30 10:21:41 2002

// This is a generated file - do not edit

#ifndef __ASSEMBLER__
#include <cyg/infra/cyg_type.h>
#include <stddef.h>

#endif
#define CYGMEM_REGION_ram (0)
#define CYGMEM_REGION_ram_SIZE (0x800000)
#define CYGMEM_REGION_ram_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)
#define CYGMEM_REGION_rom (0xfe000000)
#define CYGMEM_REGION_rom_SIZE (0x800000)
#define CYGMEM_REGION_rom_ATTR (CYGMEM_REGION_ATTR_R)
#ifndef __ASSEMBLER__
extern char CYG_LABEL_NAME (__reserved_vectors) [];
#endif
#define CYGMEM_SECTION_reserved_vectors (CYG_LABEL_NAME (__reserved_vectors))
#define CYGMEM_SECTION_reserved_vectors_SIZE (0x3000)
#ifndef __ASSEMBLER__
extern char CYG_LABEL_NAME (__reserved_vsr_table) [];
#endif
#define CYGMEM_SECTION_reserved_vsr_table (CYG_LABEL_NAME (__reserved_vsr_table))
#define CYGMEM_SECTION_reserved_vsr_table_SIZE (0x200)
#ifndef __ASSEMBLER__
extern char CYG_LABEL_NAME (__reserved_virtual_table) [];
#endif
#define CYGMEM_SECTION_reserved_virtual_table (CYG_LABEL_NAME (__reserved_virtual_table))
#define CYGMEM_SECTION_reserved_virtual_table_SIZE (0x100)
#ifndef __ASSEMBLER__
extern char CYG_LABEL_NAME (__heap1) [];
#endif
#define CYGMEM_SECTION_heap1 (CYG_LABEL_NAME (__heap1))
#define CYGMEM_SECTION_heap1_SIZE (0x800000 - (size_t) CYG_LABEL_NAME (__heap1))

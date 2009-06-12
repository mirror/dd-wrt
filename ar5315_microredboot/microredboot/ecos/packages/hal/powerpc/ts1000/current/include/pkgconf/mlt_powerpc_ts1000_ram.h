// eCos memory layout - Thu May 30 10:27:39 2002

// This is a generated file - do not edit

#ifndef __ASSEMBLER__
#include <cyg/infra/cyg_type.h>
#include <stddef.h>

#endif
#define CYGMEM_REGION_ram (0)
#define CYGMEM_REGION_ram_SIZE (0x1000000)
#define CYGMEM_REGION_ram_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)
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
extern char CYG_LABEL_NAME (__reserved_for_rom) [];
#endif
#define CYGMEM_SECTION_reserved_for_rom (CYG_LABEL_NAME (__reserved_for_rom))
#define CYGMEM_SECTION_reserved_for_rom_SIZE (0x3cd00)
#ifndef __ASSEMBLER__
extern char CYG_LABEL_NAME (__heap1) [];
#endif
#define CYGMEM_SECTION_heap1 (CYG_LABEL_NAME (__heap1))
#define CYGMEM_SECTION_heap1_SIZE (0x1000000 - (size_t) CYG_LABEL_NAME (__heap1))

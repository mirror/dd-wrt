// eCos memory layout - Fri Oct 20 10:25:48 2000

// This is a generated file - do not edit

#ifndef __ASSEMBLER__
#include <cyg/infra/cyg_type.h>
#include <stddef.h>
#endif

#define CYGMEM_REGION_rom         (0x000000)
#define CYGMEM_REGION_rom_SIZE    (0x06ffff)
#define CYGMEM_REGION_rom_ATTR    (CYGMEM_REGION_ATTR_RO)

#define CYGMEM_REGION_ram         (0x1000000)
#define CYGMEM_REGION_ram_SIZE    (0x0fffff)
#define CYGMEM_REGION_ram_ATTR    (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)

#define CYGMEM_REGION_iram        (0x3f9800)
#define CYGMEM_REGION_iram_SIZE   (0x006800)
#define CYGMEM_REGION_iram_ATTR   (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)

#define CYGMEM_REGION_eflash      (0x2000000)
#define CYGMEM_REGION_eflash_SIZE (0x1fffff)
#define CYGMEM_REGION_eflash_ATTR (CYGMEM_REGION_ATTR_RO)

#ifndef __ASSEMBLER__
extern char CYG_LABEL_NAME (__reserved_vectors) [];
#endif
#define CYGMEM_SECTION_reserved_vectors (CYG_LABEL_NAME (__reserved_vectors))
#define CYGMEM_SECTION_reserved_vectors_SIZE (0x2000)

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
#define CYGMEM_SECTION_heap1_SIZE (0x1100000 - (size_t) CYG_LABEL_NAME (__heap1))

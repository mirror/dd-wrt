/*
 * uclinux/include/asm-armnommu/arch-S3C3410/processor.h
 *
 */
#ifndef __ASM_ARCH_PROCESSOR_H
#define __ASM_ARCH_PROCESSOR_H

#define EISA_bus 0
#define EISA_bus__is_a_macro
#define MCA_bus 0
#define MCA_bus__is_a_macro

#define TASK_SIZE (0x01a00000UL)

#define INIT_MMAP { \
  &init_mm, 0, 0x02000000, PAGE_SHARED, VM_READ | VM_WRITE | VM_EXEC \
}

#endif /* __ASM_ARCH_PROCESSOR_H */

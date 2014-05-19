#ifndef __CRASHLOG_H
#define __CRASHLOG_H

#ifdef CONFIG_CRASHLOG
void crashlog_init_bootmem(struct bootmem_data *bdata);
void crashlog_init_memblock(phys_addr_t addr, phys_addr_t size);
#else
static inline void crashlog_init_bootmem(struct bootmem_data *bdata)
{
}

static inline void crashlog_init_memblock(phys_addr_t addr, phys_addr_t size)
{
}
#endif

#endif

#ifndef __CRASHLOG_H
#define __CRASHLOG_H

#ifdef CONFIG_CRASHLOG
void __init crashlog_init_mem(struct bootmem_data *bdata);
#else
static inline void crashlog_init_mem(struct bootmem_data *bdata)
{
}
#endif

#endif

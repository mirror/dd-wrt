#ifndef _ASM_POWERPC_RB_AUX_H
#define _ASM_POWERPC_RB_AUX_H

#include <linux/init.h>

extern void __init rb_pic_init(void);
extern void __init rb_init_pci(void);
extern void rb_show_cpuinfo(struct seq_file *);
extern void rb_restart(char *cmd);
extern void change_latch(unsigned char set, unsigned char clear);

#endif

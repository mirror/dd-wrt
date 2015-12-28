#ifndef __AR231X_DEVICES_H
#define __AR231X_DEVICES_H

enum {
	/* handled by ar5312.c */
	DEV_TYPE_AR2312,
	DEV_TYPE_AR2313,
	DEV_TYPE_AR5312,

	/* handled by ar2315.c */
	DEV_TYPE_AR2315,
	DEV_TYPE_AR2316,
	DEV_TYPE_AR2317,
	DEV_TYPE_AR2318,

	DEV_TYPE_UNKNOWN
};

extern int ar231x_devtype;
extern struct ar231x_board_config ar231x_board;
extern asmlinkage void (*ar231x_irq_dispatch)(void);

extern int ar231x_find_config(u8 *flash_limit);
extern void ar231x_serial_setup(u32 mapbase, unsigned int uartclk);
extern int ar231x_add_wmac(int nr, u32 base, int irq);
extern int ar231x_add_ethernet(int nr, u32 base, int irq, void *pdata);

static inline bool is_2315(void)
{
	return (current_cpu_data.cputype == CPU_4KEC);
}

static inline bool is_5312(void)
{
	return !is_2315();
}

#endif

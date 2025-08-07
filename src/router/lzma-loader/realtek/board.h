#ifndef __BOARD_H__
#define __BOARD_H__

extern unsigned char workspace[];
extern unsigned char _code_start[];

struct rtl83xx_soc_info {
	char *name;
	unsigned int id;
	unsigned int family;
	unsigned int revision;
	unsigned int cpu;
	bool testchip;
	unsigned char *compatible;
	volatile void *sw_base;
	volatile void *icu_base;
	int cpu_port;
};

void board_init(void);
void board_putchar(int ch, void *ctx);
void board_reset(void);
void board_watchdog(void);

#endif

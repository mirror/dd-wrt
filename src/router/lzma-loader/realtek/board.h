#ifndef __BOARD_H__
#define __BOARD_H__

extern unsigned char workspace[];
extern unsigned char _code_start[];

void board_init(void);
void board_putchar(int ch, void *ctx);
void board_reset(void);
void board_watchdog(void);

#endif

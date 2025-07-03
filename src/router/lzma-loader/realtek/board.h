#ifndef __BOARD_H__
#define __BOARD_H__

void board_init(void);
void board_putchar(int ch, void *ctx);
void board_reset(void);
void board_watchdog(void);

#endif

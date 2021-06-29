#include <linux/log2.h>
#include <linux/delay.h>
#include <linux/kernel.h>

/* 
 * When doing link-time-optimization the linker resolves some symbols
 * before the optimizer has done full optimization.
 * To avoid this problem provide stubs for them.
 * This prevents these errors, but you will only see them now
 * when CONFIG_LTO is disabled. Sorry.
 */

int ____ilog2_NaN(void) { BUG_ON(1); } 
void __bad_udelay(void) { BUG_ON(1); } 
void __bad_ndelay(void) { BUG_ON(1); } 

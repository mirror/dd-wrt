#ifndef _LONG_SHIFT_H
#define _LONG_SHIFT_H

/* Slow but compileable shift operations for unsigned long data type */
unsigned long long longshift_left(unsigned long long val, int shift);
unsigned long long longshift_right(unsigned long long val, int shift);

#endif



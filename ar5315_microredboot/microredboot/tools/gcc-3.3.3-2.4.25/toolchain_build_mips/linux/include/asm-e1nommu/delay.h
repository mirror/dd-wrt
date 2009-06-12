#ifndef _HYPERSTONE_NOMMU_DELAY_H
#define _HYPERSTONE_NOMMU__DELAY_H

extern void __udelay(unsigned long usecs);
extern void __delay(unsigned long loops);

#define udelay(n) __udelay(n)

#endif  /*  _HYPERSTONE_NOMMU_DELAY_H */

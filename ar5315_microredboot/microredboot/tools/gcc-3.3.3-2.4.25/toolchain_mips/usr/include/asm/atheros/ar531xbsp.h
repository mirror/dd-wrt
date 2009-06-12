#ifndef __ASM_ATHEROS_BSP_SUPPORT_H
#define __ASM_ATHEROS_BSP_SUPPORT_H
/*
 * These are definitions and functions provided by the bsp to support the
 * AR5312 WiSoC running LSDK.  For different BSP implementations, different
 * BSP functions will be needed.
 */

extern unsigned int ar531x_sys_frequency(void);
extern const char* get_system_type(void);

#ifdef CONFIG_KGDB
extern  void kgdbInit(void);
extern int kgdbEnabled(void);
#endif

#endif /* __ASM_ATHEROS_BSP_SUPPORT_H */

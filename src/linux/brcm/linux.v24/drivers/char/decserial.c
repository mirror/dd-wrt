/*
 * sercons.c
 *      choose the right serial device at boot time
 *
 * triemer 6-SEP-1998
 *      sercons.c is designed to allow the three different kinds
 *      of serial devices under the decstation world to co-exist
 *      in the same kernel.  The idea here is to abstract
 *      the pieces of the drivers that are common to this file
 *      so that they do not clash at compile time and runtime.
 *
 * HK 16-SEP-1998 v0.002
 *      removed the PROM console as this is not a real serial
 *      device. Added support for PROM console in drivers/char/tty_io.c
 *      instead. Although it may work to enable more than one
 *      console device I strongly recommend to use only one.
 *
 *	Copyright (C) 2004  Maciej W. Rozycki
 */

#include <linux/config.h>
#include <linux/errno.h>
#include <linux/init.h>

#include <asm/dec/machtype.h>
#include <asm/dec/serial.h>

extern int register_zs_hook(unsigned int channel,
			    struct dec_serial_hook *hook);
extern int unregister_zs_hook(unsigned int channel);

extern int register_dz_hook(unsigned int channel,
			    struct dec_serial_hook *hook);
extern int unregister_dz_hook(unsigned int channel);

int register_dec_serial_hook(unsigned int channel,
			     struct dec_serial_hook *hook)
{
#ifdef CONFIG_ZS
	if (IOASIC)
		return register_zs_hook(channel, hook);
#endif
#ifdef CONFIG_DZ
	if (!IOASIC)
		return register_dz_hook(channel, hook);
#endif
	return 0;
}

int unregister_dec_serial_hook(unsigned int channel)
{
#ifdef CONFIG_ZS
	if (IOASIC)
		return unregister_zs_hook(channel);
#endif
#ifdef CONFIG_DZ
	if (!IOASIC)
		return unregister_dz_hook(channel);
#endif
	return 0;
}


extern int zs_init(void);
extern int dz_init(void);

/*
 * rs_init - starts up the serial interface -
 * handle normal case of starting up the serial interface
 */
int __init rs_init(void)
{
#ifdef CONFIG_ZS
	if (IOASIC)
		return zs_init();
#endif
#ifdef CONFIG_DZ
	if (!IOASIC)
		return dz_init();
#endif
	return -ENXIO;
}

__initcall(rs_init);


#ifdef CONFIG_SERIAL_DEC_CONSOLE

extern void zs_serial_console_init(void);
extern void dz_serial_console_init(void);

/*
 * dec_serial_console_init handles the special case of starting
 * up the console on the serial port
 */
void __init dec_serial_console_init(void)
{
#ifdef CONFIG_ZS
	if (IOASIC)
		zs_serial_console_init();
#endif
#ifdef CONFIG_DZ
	if (!IOASIC)
		dz_serial_console_init();
#endif
}

#endif

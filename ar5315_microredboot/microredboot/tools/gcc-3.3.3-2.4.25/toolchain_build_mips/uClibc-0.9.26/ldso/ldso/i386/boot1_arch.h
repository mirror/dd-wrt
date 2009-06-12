/* Any assmbly language/system dependent hacks needed to setup boot1.c so it
 * will work as expected and cope with whatever platform specific wierdness is
 * needed for this architecture.  See arm/boot1_arch.h for an example of what
 * can be done.
 */

#define LD_BOOT(X)   void _dl_boot (X)

#ifndef _BTNS_DEV_H_
#define _BTNS_DEV_H_

#define RESET_BTN (1 << 0x0)
#define STNBY_BTN (1 << 0x1)


typedef struct {
	unsigned int mask;
} BTNS_STS;


/*
 * done against open of /dev/gpp, to get a cloned descriptor.
 */
#define CIOCWAIT_P       _IOWR('c', 150, BTNS_STS)

#endif /* _BTNS_DEV_H_ */


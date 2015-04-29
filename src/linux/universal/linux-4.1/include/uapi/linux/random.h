/*
 * include/linux/random.h
 *
 * Include file for the random number generator.
 */

#ifndef _UAPI_LINUX_RANDOM_H
#define _UAPI_LINUX_RANDOM_H

#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/irqnr.h>

/* ioctl()'s for the random number generator */

/* Get the entropy count. */
#define RNDGETENTCNT	_IOR( 'R', 0x00, int )

/* Add to (or subtract from) the entropy count.  (Superuser only.) */
#define RNDADDTOENTCNT	_IOW( 'R', 0x01, int )

/* Get the contents of the entropy pool.  (Superuser only.) */
#define RNDGETPOOL	_IOR( 'R', 0x02, int [2] )

/* 
 * Write bytes into the entropy pool and add to the entropy count.
 * (Superuser only.)
 */
#define RNDADDENTROPY	_IOW( 'R', 0x03, int [2] )

/* Clear entropy count to 0.  (Superuser only.) */
#define RNDZAPENTCNT	_IO( 'R', 0x04 )

/* Clear the entropy pool and associated counters.  (Superuser only.) */
#define RNDCLEARPOOL	_IO( 'R', 0x06 )

#ifdef CONFIG_FIPS_RNG

/* Size of seed value - equal to AES blocksize */
#define AES_BLOCK_SIZE_BYTES	16
#define SEED_SIZE_BYTES			AES_BLOCK_SIZE_BYTES
/* Size of AES key */
#define KEY_SIZE_BYTES		16

/* ioctl() structure used by FIPS 140-2 Tests */
struct rand_fips_test {
	unsigned char key[KEY_SIZE_BYTES];			/* Input */
	unsigned char datetime[SEED_SIZE_BYTES];	/* Input */
	unsigned char seed[SEED_SIZE_BYTES];		/* Input */
	unsigned char result[SEED_SIZE_BYTES];		/* Output */
};

/* FIPS 140-2 RNG Variable Seed Test. (Superuser only.) */
#define RNDFIPSVST	_IOWR('R', 0x10, struct rand_fips_test)

/* FIPS 140-2 RNG Monte Carlo Test. (Superuser only.) */
#define RNDFIPSMCT	_IOWR('R', 0x11, struct rand_fips_test)

#endif /* #ifdef CONFIG_FIPS_RNG */

struct rand_pool_info {
	int	entropy_count;
	int	buf_size;
	__u32	buf[0];
};

/*
 * Flags for getrandom(2)
 *
 * GRND_NONBLOCK	Don't block and return EAGAIN instead
 * GRND_RANDOM		Use the /dev/random pool instead of /dev/urandom
 */
#define GRND_NONBLOCK	0x0001
#define GRND_RANDOM	0x0002

#endif /* _UAPI_LINUX_RANDOM_H */

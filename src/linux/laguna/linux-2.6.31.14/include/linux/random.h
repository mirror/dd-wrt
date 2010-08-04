/*
 * include/linux/random.h
 *
 * Include file for the random number generator.
 */

#ifndef _LINUX_RANDOM_H
#define _LINUX_RANDOM_H

#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/types.h> /* for __u32 in user space */
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

/* Exported functions */

#ifdef __KERNEL__

extern void rand_initialize_irq(int irq);

extern void add_input_randomness(unsigned int type, unsigned int code,
				 unsigned int value);
extern void add_interrupt_randomness(int irq);

extern void random_input_words(__u32 *buf, size_t wordcount, int ent_count);
extern int random_input_wait(void);
#define HAS_RANDOM_INPUT_WAIT 1

extern void get_random_bytes(void *buf, int nbytes);
void generate_random_uuid(unsigned char uuid_out[16]);

extern __u32 secure_ip_id(__be32 daddr);
extern u32 secure_ipv4_port_ephemeral(__be32 saddr, __be32 daddr, __be16 dport);
extern u32 secure_ipv6_port_ephemeral(const __be32 *saddr, const __be32 *daddr,
				      __be16 dport);
extern __u32 secure_tcp_sequence_number(__be32 saddr, __be32 daddr,
					__be16 sport, __be16 dport);
extern __u32 secure_tcpv6_sequence_number(__be32 *saddr, __be32 *daddr,
					  __be16 sport, __be16 dport);
extern u64 secure_dccp_sequence_number(__be32 saddr, __be32 daddr,
				       __be16 sport, __be16 dport);

#ifndef MODULE
extern const struct file_operations random_fops, urandom_fops;
#endif

unsigned int get_random_int(void);
unsigned long randomize_range(unsigned long start, unsigned long end, unsigned long len);

u32 random32(void);
void srandom32(u32 seed);

#endif /* __KERNEL___ */

#endif /* _LINUX_RANDOM_H */

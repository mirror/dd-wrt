#ifndef _CESA_DEV_H_
#define _CESA_DEV_H_

/*
 * common debug for all
 */
#if 1
#define dprintk(a...)	if (debug) { printk(a); } else
#else
#define dprintk(a...)
#endif

typedef enum {
	MULTI = 0,
	SIZE,
	SINGLE,
	AES,
	DES,
	TRI_DES,
	MD5,
	SHA1,
	MAX_CESA_TEST_TYPE	
} CESA_TEST_TYPE;

typedef struct {
	CESA_TEST_TYPE 		test;
	unsigned int	  	iter;		/* How many interation to run */
	unsigned int	  	req_size;	/* request buffer size */
	unsigned int		checkmode;	/* check mode: verify or not */
	unsigned int		session_id; 	/* relevant only for single test */
	unsigned int		data_id;   	/* relevant only for single test */
} CESA_TEST;

typedef enum {
	STATUS = 0,
	CHAN,
	QUEUE,
	SA,
	CACHE_IDX,
	SRAM,
	SAD,
	TST_REQ,
	TST_SES,
    TST_STATS,
	MAX_CESA_DEBUG_TYPE
} CESA_DEBUG_TYPE;

typedef struct {
	CESA_DEBUG_TYPE	debug;
	unsigned int	index; /* general index */
	unsigned int	mode;  /* verbos mode */
	unsigned int 	size;  /* size of buffer */
} CESA_DEBUG;


/*
 * done against open of /dev/cesa, to get a cloned descriptor.
 */
#define	CIOCDEBUG	_IOWR('c', 150, CESA_DEBUG)
#define	CIOCTEST	_IOWR('c', 151, CESA_TEST)

#endif /* _CESA_DEV_H_ */

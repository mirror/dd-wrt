#include "config.h"

#if HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#if HAVE_STDIO_H
#include <stdio.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_ASSERT_H
#include <assert.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include "../src/defines.h"
#include "../src/enums.h"

struct _tlv;
typedef struct _tlv tlv;

struct _tlv {
    char type;
    int length;
    void *value;
    tlv *next;
};

#undef debug
#define	debug(a, b)	printf

#define	MEM_TLV	sizeof(tlv)
#define	memAllocate(a)	malloc(a)
#define	memFree(a, b)	free(a)
#define xmalloc(a) malloc(a)
#define xfree(a) free(a)

#ifndef PRId64
#ifdef _SQUID_MSWIN_		/* Windows native port using MSVCRT */
#define PRId64 "I64d"
#elif SIZEOF_INT64_T > SIZEOF_LONG
#define PRId64 "lld"
#else
#define PRId64 "ld"
#endif
#endif

#if SIZEOF_INT64_T > SIZEOF_LONG && HAVE_STRTOLL
typedef int64_t squid_off_t;
#define SIZEOF_SQUID_OFF_T SIZEOF_INT64_T
#define PRINTF_OFF_T PRId64
#define strto_off_t (int64_t)strtoll
#else
typedef long squid_off_t;
#define SIZEOF_SQUID_OFF_T SIZEOF_LONG
#define PRINTF_OFF_T "ld"
#define strto_off_t strtol
#endif

static tlv **
storeSwapTLVAdd(int type, const void *ptr, size_t len, tlv ** tail)
{
    tlv *t = memAllocate(MEM_TLV);
    t->type = (char) type;
    t->length = (int) len;
    t->value = xmalloc(len);
    xmemcpy(t->value, ptr, len);
    *tail = t;
    return &t->next;		/* return new tail pointer */
}

#if UNUSED_CODE
static void
storeSwapTLVFree(tlv * n)
{
    tlv *t;
    while ((t = n) != NULL) {
	n = t->next;
	xfree(t->value);
	memFree(t, MEM_TLV);
    }
}
#endif

#if UNUSED_CODE
static char *
storeSwapMetaPack(tlv * tlv_list, int *length)
{
    int buflen = 0;
    tlv *t;
    int j = 0;
    char *buf;
    assert(length != NULL);
    buflen++;			/* STORE_META_OK */
    buflen += sizeof(int);	/* size of header to follow */
    for (t = tlv_list; t; t = t->next)
	buflen += sizeof(char) + sizeof(int) + t->length;
    buflen++;			/* STORE_META_END */
    buf = xmalloc(buflen);
    buf[j++] = (char) STORE_META_OK;
    xmemcpy(&buf[j], &buflen, sizeof(int));
    j += sizeof(int);
    for (t = tlv_list; t; t = t->next) {
	buf[j++] = (char) t->type;
	xmemcpy(&buf[j], &t->length, sizeof(int));
	j += sizeof(int);
	xmemcpy(&buf[j], t->value, t->length);
	j += t->length;
    }
    buf[j++] = (char) STORE_META_END;
    assert((int) j == buflen);
    *length = buflen;
    return buf;
}
#endif

static tlv *
storeSwapMetaUnpack(const char *buf, int *hdr_len)
{
    tlv *TLV;			/* we'll return this */
    tlv **T = &TLV;
    char type;
    int length;
    int buflen;
    int j = 0;
    assert(buf != NULL);
    assert(hdr_len != NULL);
    if (buf[j++] != (char) STORE_META_OK)
	return NULL;
    xmemcpy(&buflen, &buf[j], sizeof(int));
    j += sizeof(int);
    /*
     * sanity check on 'buflen' value.  It should be at least big
     * enough to hold one type and one length.
     */
    if (buflen <= (sizeof(char) + sizeof(int)))
	    return NULL;
    while (buflen - j > (sizeof(char) + sizeof(int))) {
	type = buf[j++];
	/* VOID is reserved, but allow some slack for new types.. */
	if (type <= STORE_META_VOID || type > STORE_META_END + 10) {
	    debug(20, 0) ("storeSwapMetaUnpack: bad type (%d)!\n", type);
	    break;
	}
	xmemcpy(&length, &buf[j], sizeof(int));
	if (length < 0 || length > (1 << 16)) {
	    debug(20, 0) ("storeSwapMetaUnpack: insane length (%d)!\n", length);
	    break;
	}
	j += sizeof(int);
	if (j + length > buflen) {
	    debug(20, 0) ("storeSwapMetaUnpack: overflow!\n");
	    debug(20, 0) ("\ttype=%d, length=%d, buflen=%d, offset=%d\n",
		type, length, buflen, (int) j);
	    break;
	}
	T = storeSwapTLVAdd(type, &buf[j], (size_t) length, T);
	j += length;
    }
    *hdr_len = buflen;
    return TLV;
}


#define	STRIPESIZE 1048576
#define	BLOCKSIZE 1024
#define BLKBITS 10

static void
parse_stripe(int stripeid, char *buf, int len, int blocksize)
{
    int j = 0;
    int bl = 0;
    tlv *t, *tlv_list;
    int64_t *l;
    int tmp;

    while (j < len) {
	l = NULL;
	bl = 0;
	tlv_list = storeSwapMetaUnpack(&buf[j], &bl);
	if (tlv_list == NULL) {
	    printf("  Object: NULL\n");
	    return;
	}
	printf("  Object: (filen %d) hdr size %d\n", j / blocksize + (stripeid * STRIPESIZE / blocksize), bl);
	for (t = tlv_list; t; t = t->next) {
	    switch (t->type) {
	    case STORE_META_URL:
		/* XXX Is this OK? Is the URL guaranteed to be \0 terminated? */
		printf("    URL: %s\n", (char *) t->value);
		break;
	    case STORE_META_OBJSIZE:
		l = t->value;
		printf("Size: %" PRINTF_OFF_T " (len %d)\n", *l, t->length);
		break;
	    }
	}
	if (l == NULL) {
	    printf("  STRIPE: Completed, got an object with no size\n");
	    return;
	}
	j = j + *l + bl;
	/* And now, the blocksize! */
	tmp = j / blocksize;
	tmp = (tmp + 1) * blocksize;
	j = tmp;
    }
}

int
main(int argc, char *argv[])
{
    int fd;
    char buf[STRIPESIZE];
    int i = 0, len;
    unsigned int numstripes = 0;
    int blocksize = BLOCKSIZE;
    int blksize_bits;

    if (argc < 4) {
	printf("Usage: %s <path to COSS datafile> <blocksize> <number of stripes>\n", argv[0]);
	exit(1);
    }
    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
	perror("open");
	exit(1);
    }

    blocksize = (unsigned int) atoi(argv[2]);
    for(blksize_bits = 0;((blocksize >> blksize_bits) > 0);blksize_bits++) {
	if( ((blocksize >> blksize_bits) > 0) &&
	  (((blocksize >> blksize_bits) << blksize_bits) != blocksize)) {
	    printf("Blocksize bits must be a power of 2\n");
	    exit(1);
	}
    }

    numstripes = (unsigned int) atoi(argv[3]);

    while ((len = read(fd, buf, STRIPESIZE)) > 0) {
	printf("STRIPE: %d (len %d)\n", i, len);
	parse_stripe(i, buf, len, blocksize);
	i++;
	if((numstripes > 0) && (i >= numstripes))
	    break;
    }
    return 0;
}

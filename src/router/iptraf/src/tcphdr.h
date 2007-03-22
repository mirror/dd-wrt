
/*
 * Reconstruction of the TCP header structure, slightly modified.  With
 * provisions for little- and big-endian architectures.
 */

#include <sys/types.h>
#include <endian.h>

struct tcphdr {
    u_int16_t source;
    u_int16_t dest;
    u_int32_t seq;
    u_int32_t ack_seq;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    u_int16_t res1:4,
        doff:4, fin:1, syn:1, rst:1, psh:1, ack:1, urg:1, res2:2;
#elif __BYTE_ORDER == __BIG_ENDIAN
    u_int16_t doff:4,
        res1:4, res2:2, urg:1, ack:1, psh:1, rst:1, syn:1, fin:1;
#endif
    u_int16_t window;
    u_int16_t check;
    u_int16_t urg_ptr;
};

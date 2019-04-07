
#include "main.c"
#include "noise.c" 
#include "device.c" 
#include "peer.c" 
#include "timers.c" 
#include "queueing.c" 
#include "send.c" 
#include "receive.c" 
#include "socket.c" 
#include "peerlookup.c" 
#include "allowedips.c" 
#include "ratelimiter.c" 
#include "cookie.c" 
#include "netlink.c"
#include "crypto/zinc/chacha20/chacha20.c" 
#include "crypto/zinc/curve25519/curve25519.c" 
#include "crypto/zinc/poly1305/poly1305.c" 
#include "crypto/zinc/chacha20poly1305.c" 
#include "crypto/zinc/blake2s/blake2s.c"
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0)
#include "compat/siphash/siphash.c"
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
#include "compat/dst_cache/dst_cache.c"
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
#include "compat/udp_tunnel/udp_tunnel.c"
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 106)
#include "compat/memneq/memneq.c"
#endif
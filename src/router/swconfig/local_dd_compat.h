#ifndef NLA_F_NESTED
#define NLA_F_NESTED (1 << 15)
#endif

#ifndef NLA_F_NET_BYTEORDER
#define NLA_F_NET_BYTEORDER (1 << 14)
#endif

#ifndef NLA_TYPE_MASK
#define NLA_TYPE_MASK ~(NLA_F_NESTED | NLA_F_NET_BYTEORDER)
#endif

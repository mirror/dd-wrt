#ifndef __DAWN_MAC_UTILS_H
#define __DAWN_MAC_UTILS_H

#include <stdint.h>
#include <string.h>

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define STR2MAC(a) &(a)[0], &(a)[1], &(a)[2], &(a)[3], &(a)[4], &(a)[5]

#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"
#define MACSTRLOWER "%02x:%02x:%02x:%02x:%02x:%02x"

#define NR_MACSTR "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c"
#define NR_MAC2STR(a) *a, *(a+1), *(a+2), *(a+3), *(a+4), *(a+5), *(a+6), *(a+7), *(a+8), *(a+9), *(a+10), *(a+11)

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

// Simplify some handling of MAC addresses
struct __attribute__((__packed__)) dawn_mac
{
        uint8_t u8[ETH_ALEN];
};

// Used as a filler where a value is required but not used functionally
extern const struct dawn_mac dawn_mac_null;

// Compare a raw MAC address to 00:00:00:00:00:00
#define mac_is_null(a1) ((a1)[0] == 0) && ((a1)[1] == 0) && ((a1)[2] == 0) && ((a1)[3] == 0) && ((a1)[4] == 0) && ((a1)[5] == 0)

// For byte arrays outside MAC structure
#define mac_is_equal(addr1, addr2) (memcmp(addr1, addr2, ETH_ALEN) == 0)

// For byte arrays inside MAC structure
#define mac_compare_bb(addr1, addr2) memcmp((addr1).u8, (addr2).u8, ETH_ALEN)
#define mac_is_equal_bb(addr1, addr2) (memcmp((addr1).u8, (addr2).u8, ETH_ALEN) == 0)

/**
 * Convert mac adress string to mac adress.
 * @param txt
 * @param addr
 * @return
 */
int hwaddr_aton(const char* txt, uint8_t* addr);

/**
 * Parse MAC from string.
 * @param s
 */
struct dawn_mac str2mac(char* s);

/**
 * Write mac to a file.
 * @param path
 * @param addr
 */
void write_mac_to_file(char* path, struct dawn_mac addr);

#endif

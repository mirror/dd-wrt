#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"
#include "mac_utils.h"

// Used as a filler where a value is required but not used functionally
const struct dawn_mac dawn_mac_null = { .u8 = {0,0,0,0,0,0} };

// source: https://elixir.bootlin.com/linux/v4.9/source/lib/hexdump.c#L28
// based on: hostapd src/utils/common.c
int hwaddr_aton(const char* txt, uint8_t* addr) {
    int i;

    for (i = 0; i < ETH_ALEN; i++) {
        int a = 0;
        char ch = *txt++;

        if ((ch >= '0') && (ch <= '9'))
            a = ch - '0';
        else if ((ch >= 'a') && (ch <= 'f'))
            a = ch - 'a' + 10;
        else if ((ch >= 'A') && (ch <= 'F'))
            a = ch - 'A' + 10;
        else
            return -1;

        ch = *txt++;
        a *= 16;

        if ((ch >= '0') && (ch <= '9'))
            a += ch - '0';
        else if ((ch >= 'a') && (ch <= 'f'))
            a += ch - 'a' + 10;
        else if ((ch >= 'A') && (ch <= 'F'))
            a += ch - 'A' + 10;
        else
            return -1;

        *addr++ = a;

        // TODO: Should NUL terminator be checked for? Is aa:bb:cc:dd:ee:ff00 valid input?
        if (i != (ETH_ALEN - 1) && *txt++ != ':')
            return -1;
    }

    return 0;
}

struct dawn_mac str2mac(char* s)
{
    // Return something testable if sscanf() fails
    struct dawn_mac tmp_mac = { .u8 = {0,0,0,0,0,0} };

    // Need to scanf to an array of ints as there is no byte format specifier
    int tmp_int_mac[ETH_ALEN];
    if (sscanf(s, MACSTR, STR2MAC(tmp_int_mac)) == 6)
    {
        for (int i = 0; i < ETH_ALEN; ++i)
            tmp_mac.u8[i] = (uint8_t)tmp_int_mac[i];
    }

    return tmp_mac;
}

void write_mac_to_file(char* path, struct dawn_mac addr) {
    FILE* f = fopen(path, "a");
    if (f == NULL)
        dawnlog_error("Error opening mac file!\n");
    else
    {   
        fprintf(f, MACSTR "\n", MAC2STR(addr.u8));
        fclose(f);
    }
}

//
// Simple (boot) FLASH test
//
//-----------------------------------------------------------------
// Copyright (C) 2003, Gary Thomas <gary@mlbassoc.com>
//-----------------------------------------------------------------

#define BOOT_FLASH 0xFFF80000   // 512K
#define SETUP1         0x5555
#define SETUP2         0x2AAA
#define SETUP_CODE1      0xAA
#define SETUP_CODE2      0x55
#define QUERY_START      0x90
#define QUERY_END        0xF0
#define MANUF_ID            0
#define PART_ID             1

static void
hang(void)
{
    while (1) ;
}

void
run_test(void)
{
    volatile unsigned char *flash = (volatile unsigned char *)BOOT_FLASH;
    unsigned char info[2];

    tty_puts("FLASH test started\n");

    // Send "query" command sequence
    flash[SETUP1] = SETUP_CODE1;
    flash[SETUP2] = SETUP_CODE2;
    flash[SETUP1] = QUERY_START;

    // Fetch query information
    info[0] = flash[MANUF_ID];
    info[1] = flash[PART_ID];

    // Send "query done" command sequence
    flash[SETUP1] = SETUP_CODE1;
    flash[SETUP2] = SETUP_CODE2;
    flash[SETUP1] = QUERY_END;

    tty_puts("FLASH query info = ");
    tty_puthex(info[0]<<8 | info[1]);
    tty_puts("\n");

    // Done
    tty_puts("Done\n");
}

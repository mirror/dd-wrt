//
// Simple SDRAM test
//
//-----------------------------------------------------------------
// Copyright (C) 2003, Gary Thomas <gary@mlbassoc.com>
//-----------------------------------------------------------------

#define DRAM_START      0x00010000
#define DRAM_END        0x00700000

#define MAX_ERRS        16
#define SPIN_TIME       0x800

//#define DO_SPIN
#define TEST_LONGS
#define TEST_SHORTS
#define TEST_BYTES
#define TEST_REFRESH
#define TEST_PARALLEL
//#define TEST_RANDOM

typedef unsigned long munge_fun(unsigned long);

void
spin(void)
{
#ifdef DO_SPIN
    static char spin_chars[] = "|/-\\|-";
    static int tick = 0;
    static int times = 0;

    if (++times == SPIN_TIME) {
        tty_putc(spin_chars[tick++]);
        tty_putc(0x08);
        if (tick >= sizeof(spin_chars)) {
            tick = 0;
        }
        times = 0;
    }
#endif
}

unsigned long
nop(unsigned long val)
{
    spin();
    return val;
}

unsigned long
toggle(unsigned long val)
{
    spin();
    return ~val;
}

unsigned long
inc_addr(unsigned long val)
{
    spin();
    return val + sizeof(unsigned long);
}

void
idle(void)
{
    int i;
    for (i = 0;  i < 1000000;  i++) ;
}

#ifdef TEST_LONGS
void
test_longs(char *title, unsigned long _orig_pat, 
           unsigned long *start_addr, unsigned long *end_addr,
           munge_fun *fun, int _delay, int loops)
{
    unsigned long *p;
    unsigned long val, pat;
    int errs, idle, delay;

    tty_puts(title);
#ifdef DO_SPIN
    tty_puts("\n");
#endif

    // Fill SDRAM with various patterns
    p = start_addr;
    pat = _orig_pat;
    while (p != end_addr) {
        *p++ = pat;
        pat = (*fun)(pat);
    }

    while (loops-- > 0) {
        tty_puts("<");
        delay = _delay;
        while (delay-- > 0) {
            idle = 0xF000000;
            while (--idle > 0) ;
        }
        tty_puts(">");

        // Test SDRAM
        p = start_addr;
        pat = _orig_pat;
        errs = 0;
        while (p != end_addr) {
            if ((val = *p++) != pat) {
                if (++errs < MAX_ERRS) {
                    tty_puts("\nFailed at ");
                    tty_puthex(p-1);
                    tty_puts(", read: ");
                    tty_puthex(val);
                    tty_puts(", expected: ");
                    tty_puthex(pat);
                }
            }
            pat = (*fun)(pat);
        }

        tty_puts(" - done");
        if (errs) {
            tty_puts(" with ");
            tty_puthex(errs);
            tty_puts(" errors");
        }
        tty_puts("\n");
    }
}
#endif

#ifdef TEST_SHORTS
void
test_shorts(char *title, unsigned short _orig_pat, 
            unsigned short *start_addr, unsigned short *end_addr,
            munge_fun *fun, int _delay, int loops)
{
    unsigned short *p;
    unsigned short val, pat;
    int errs, idle, delay;

    tty_puts(title);
#ifdef DO_SPIN
    tty_puts("\n");
#endif

    // Fill SDRAM with various patterns
    p = start_addr;
    pat = _orig_pat;
    while (p != end_addr) {
        *p++ = pat;
        pat = (*fun)(pat);
    }

    while (loops-- > 0) {
        tty_puts("<");
        delay = _delay;
        while (delay-- > 0) {
            idle = 0xF000000;
            while (--idle > 0) ;
        }
        tty_puts(">");

        // Test SDRAM
        p = start_addr;
        pat = _orig_pat;
        errs = 0;
        while (p != end_addr) {
            if ((val = *p++) != pat) {
                if (++errs < MAX_ERRS) {
                    tty_puts("\nFailed at ");
                    tty_puthex(p-1);
                    tty_puts(", read: ");
                    tty_puthex(val);
                    tty_puts(", expected: ");
                    tty_puthex(pat);
                }
            }
            pat = (*fun)(pat);
        }

        tty_puts(" - done");
        if (errs) {
            tty_puts(" with ");
            tty_puthex(errs);
            tty_puts(" errors");
        }
        tty_puts("\n");
    }
}
#endif

#ifdef TEST_BYTES
void
test_bytes(char *title, unsigned char _orig_pat, 
           unsigned char *start_addr, unsigned char *end_addr,
           munge_fun *fun, int _delay, int loops)
{
    unsigned char *p;
    unsigned char val, pat;
    int errs, idle, delay;

    tty_puts(title);
#ifdef DO_SPIN
    tty_puts("\n");
#endif

    // Fill SDRAM with various patterns
    p = start_addr;
    pat = _orig_pat;
    while (p != end_addr) {
        *p++ = pat;
        pat = (*fun)(pat);
    }

    while (loops-- > 0) {
        tty_puts("<");
        delay = _delay;
        while (delay-- > 0) {
            idle = 0xF000000;
            while (--idle > 0) ;
        }
        tty_puts(">");

        // Test SDRAM
        p = start_addr;
        pat = _orig_pat;
        errs = 0;
        while (p != end_addr) {
            if ((val = *p++) != pat) {
                if (++errs < MAX_ERRS) {
                    tty_puts("\nFailed at ");
                    tty_puthex(p-1);
                    tty_puts(", read: ");
                    tty_puthex(val);
                    tty_puts(", expected: ");
                    tty_puthex(pat);
                }
            }
            pat = (*fun)(pat);
        }

        tty_puts(" - done");
        if (errs) {
            tty_puts(" with ");
            tty_puthex(errs);
            tty_puts(" errors");
        }
        tty_puts("\n");
    }
}
#endif

#ifdef TEST_PARALLEL
void
parallel_longs(unsigned long _orig_pat, 
               unsigned long *start_buf1, 
               unsigned long *start_buf2,
               unsigned long *end_addr,
               munge_fun *fun)
{
    unsigned long *p1, *p2;
    unsigned long pat, v1, v2;
    int errs, loops;

    tty_puts("Parallel buffers\n");
    // Fill buffers
    p1 = start_buf1;
    p2 = start_buf2;
    pat = _orig_pat;
    while (p2 != end_addr) {
        *p1++ = pat;
        *p2++ = ~pat;
        pat = (*fun)(pat);
    }

    for (loops = 0;  loops < 4;  loops++) {
        // Compare buffers
        p1 = start_buf1;
        p2 = start_buf2;
        pat = _orig_pat;
        errs = 0;
        while (p2 != end_addr) {
            if (((v1 = *p1) != pat) ||
                ((v2 = *p2) != ~pat)) {
                if (++errs < MAX_ERRS) {
                    tty_puts("\nFailed at ");
                    tty_puthex(p1);
                    tty_puts(", v1: ");
                    tty_puthex(v1);
                    tty_puts(", v2: ");
                    tty_puthex(v2);
                }
            }
            p1++;  p2++;
            pat = (*fun)(pat);
        }
        p1 = start_buf1;
        p2 = start_buf2;
        pat = _orig_pat;
        errs = 0;
        while (p2 != end_addr) {
            if (((v1 = *p1) != pat) ||
                ((v2 = *p1) != pat)) {
                if (++errs < MAX_ERRS) {
                    tty_puts("\nFailed at ");
                    tty_puthex(p1);
                    tty_puts(", v1: ");
                    tty_puthex(v1);
                    tty_puts(", v2: ");
                    tty_puthex(v2);
                }
            }
            p1++;  p2++;
            pat = (*fun)(pat);
        }
        tty_puts(" - done");
        if (errs) {
            tty_puts(" with ");
            tty_puthex(errs);
            tty_puts(" errors");
        }
        tty_puts("\n");
    }
}
#endif

#ifdef TEST_RANDOM
// Not really random, but noisy

unsigned long pseudo_random_data[] = {
#if 0
    0x38810008, 0x80690000, 0x8129001C, 0x7D2903A6,
    0x4E800421, 0x2C030000, 0x4082FFE4, 0x80010024,
    0x83E1001C, 0x7C0803A6, 0x38210020, 0x4E800020,
    0x9421FFD0, 0x7C0802A6, 0x3D200000, 0x93C10028,
    0x90010034, 0x3BC948F0, 0x801E0430, 0x9361001C,
    0x2C000000, 0x93A10024, 0x93E1002C, 0x93210014,
    0x93410018, 0x93810020, 0x3BA00000, 0x3B600000,
    0x3BE00000, 0x40820318, 0x3B410008, 0x3F800000,
    0x7FD9F378, 0x813C48F0, 0x7F44D378, 0x80690000,
    0x8129001C, 0x7D2903A6, 0x4E800421, 0x2C030000,
    0x418202C8, 0x89610008, 0x3BFF0001, 0x5560063E,
    0x2C000002, 0x4182023C, 0x41810258, 0x2C000001,
    0x41820224, 0x2C1D0000, 0x4182FFBC, 0x3FC00000,
    0x813E48F0, 0x3C800000, 0x80690000, 0x8129001C,
    0x38844CF8, 0x7D2903A6, 0x4E800421, 0x2C030000,
    0x3BFE48F0, 0x3860FFFD, 0x40820030, 0x80010034 
#else
    /* 0x0010B000: */0x41810008, 0x540B063E, 0x5560063E, 0x7C030000,
    /* 0x0010B010: */0x7C601850, 0x4C820020, 0x419E0008, 0x4200FFAC,
    /* 0x0010B020: */0x38600000, 0x4E800020, 0x9421FFE0, 0x7C0802A6, 
    /* 0x0010B030: */0x3D200000, 0x93C10018, 0x90010024, 0x3BC93200, 
    /* 0x0010B040: */0x801E0034, 0x93810010, 0x93A10014, 0x93E1001C, 
    /* 0x0010B050: */0x7C7D1B78, 0x7C0903A6, 0x3860FFFF, 0x7C9F2378,
    /* 0x0010B060: */0x4E800421, 0x2C1D0002, 0x7C7C1B78, 0x41820060, 
    /* 0x0010B070: */0x3C60FFF9, 0x3863FBFC, 0x4BFFEEA9, 0x3D200000,
    /* 0x0010B080: */0x80093880, 0x3C60FFF9, 0x2C000000, 0x3863FC1C,
    /* 0x0010B090: */0x7F84E378, 0x40820030, 0x3C60FFF9, 0x3863FC20,
    /* 0x0010B0A0: */0x4BFFEE81, 0x80010024, 0x83810010, 0x83A10014, 
    /* 0x0010B0B0: */0x83C10018, 0x83E1001C, 0x7C0803A6, 0x38210020, 
    /* 0x0010B0C0: */0x4E800020, 0x4BFFEE5D, 0x4BFFFFDC, 0x807F0004, 
    /* 0x0010B0D0: */0x3C80FFF9, 0x3884FC24, 0x38A00003, 0x48001C09, 
    /* 0x0010B0E0: */0x2C030000, 0x4082001C, 0x3D200000, 0x90693880, 
    /* 0x0010B0F0: */0x38000001, 0x3D200000, 0x9009389C, 0x4BFFFFA8, 
    /* 0x0010B100: */0x807F0004, 0x38810008, 0x38A00000, 0x38C00000,
    /* 0x0010B110: */0x48001625, 0x2C030000, 0x41820050, 0x80010008, 
    /* 0x0010B120: */0x2C000000, 0x40820034, 0x801E0034, 0x38600000, 
    /* 0x0010B130: */0x7C0903A6, 0x4E800421, 0x83DE0030, 0x80610008, 
    /* 0x0010B140: */0x7FC903A6, 0x4E800421, 0x80010008, 0x7C00E000,
    /* 0x0010B150: */0x4082FFA0, 0x4BFFFF50, 0x3C60FFF9, 0x809F0004,
    /* 0x0010B160: */0x3863FC28, 0x4BFFFF60, 0x3C60FFF9, 0x809F0004,
    /* 0x0010B170: */0x3863FC4C, 0x4BFFFF50, 0x9421FFE8, 0x7C0802A6,
    /* 0x0010B180: */0x3D200000, 0x93C10010, 0x9001001C, 0x3BC93200,
    /* 0x0010B190: */0x3D200000, 0x80093880, 0x93E10014, 0x2C000000,
    /* 0x0010B1A0: */0x7C7F1B78, 0x93A1000C, 0x3860FFFF, 0x4182004C,
    /* 0x0010B1B0: */0x813E0014, 0x7FE4FB78, 0x2C090000, 0x41820030,
    /* 0x0010B1C0: */0x80690000, 0x8129000C, 0x7D2903A6, 0x4E800421,
    /* 0x0010B1D0: */0x8001001C, 0x83A1000C, 0x83C10010, 0x83E10014,
    /* 0x0010B1E0: */0x7C0803A6, 0x38210018, 0x4E800020, 0x813E0018,
    /* 0x0010B1F0: */0x7FE4FB78, 0x4BFFFFCC, 0x801E0034, 0x7C0903A6 

#endif
};

void
random_longs(unsigned long *start, unsigned long end)
{
    unsigned long *dp, *random, *random_end;
    unsigned long v1, v2;
    int errs, tries, total;

    random_end = (unsigned long *)((unsigned long)pseudo_random_data + sizeof(pseudo_random_data));

    dp = start;  random = pseudo_random_data;
    tty_putc('\r');
    tries = 0;
    while (dp != end) {
        *dp++ = *random++;
        if (random == random_end) {
            random = pseudo_random_data;
            if (++tries == 0x100) {
                tty_putc('.');            
                tries = 0;
                idle();
            }
        }
    }
    tty_putc('\r');

    dp = start;  random = pseudo_random_data;
    errs = 0;
    while (dp != end) {
        if ((v1 = *dp++) != (v2 = *random++)) {
            if (++errs < MAX_ERRS) {
                tty_puts("\nFailed at ");
                tty_puthex(dp-1);
                tty_puts(", v1: ");
                tty_puthex(v1);
                tty_puts(", v2: ");
                tty_puthex(v2);
                total = 0;
                for (tries = 0;  tries < 1024;  tries++) {
                    dp -= 2;  random -= 2;
                    if ((v1 = *dp++) != (v2 = *random++)) total++;
                    if ((v1 = *dp++) != (v2 = *random++)) total++;
                }
                tty_puts(", times: ");
                tty_puthex(total);
            }
        }
        if (random == random_end) {
            random = pseudo_random_data;
        }
    }
    if (errs) {
        tty_puts("\nTotal errors: ");
        tty_puthex(errs);
        tty_puts("\n");
    }
}
#endif

void
run_test(void)
{
    tty_puts("\nSDRAM test started\n");

#ifdef TEST_RANDOM
    tty_puts("Random data\n");
    while(1) 
    random_longs(DRAM_START, DRAM_END);
#endif

#ifdef TEST_PARALLEL
    parallel_longs(DRAM_START, 
                   (unsigned long *)DRAM_START, 
                   (unsigned long *)(DRAM_START+((DRAM_END-DRAM_START)/2)), 
                   (unsigned long *)DRAM_END, 
                   inc_addr);
    parallel_longs(0x00000000, 
                   (unsigned long *)DRAM_START, 
                   (unsigned long *)(DRAM_START+((DRAM_END-DRAM_START)/2)), 
                   (unsigned long *)DRAM_END, 
                   nop);
#endif

#ifdef TEST_LONG
    tty_puts("*** Testing LONG\n");
    test_longs("ZERO: ", 0x00000000, (unsigned long *)DRAM_START, (unsigned long *)DRAM_END, nop, 1, 1);
    test_longs("ONES: ", 0xFFFFFFFF, (unsigned long *)DRAM_START, (unsigned long *)DRAM_END, nop, 1, 1);
    test_longs("ALTS: ", 0x55AA55AA, (unsigned long *)DRAM_START, (unsigned long *)DRAM_END, toggle, 1, 1);
    test_longs("ADDR: ", DRAM_START, (unsigned long *)DRAM_START, (unsigned long *)DRAM_END, inc_addr, 1, 1);
#endif

#ifdef TEST_SHORTS
    tty_puts("*** Testing SHORT\n");
    test_shorts("ZERO: ", 0x0000, (unsigned short *)DRAM_START, (unsigned short *)DRAM_END, nop, 1, 1);
    test_shorts("ONES: ", 0xFFFF, (unsigned short *)DRAM_START, (unsigned short *)DRAM_END, nop, 1, 1);
    test_shorts("ALTS: ", 0x55AA, (unsigned short *)DRAM_START, (unsigned short *)DRAM_END, toggle, 1, 1);
    test_shorts("ADDR: ", 0, (unsigned short *)DRAM_START, (unsigned short *)DRAM_END, inc_addr, 1, 1);
#endif

#ifdef TEST_BYTES
    tty_puts("*** Testing BYTE\n");
    test_bytes("ZERO: ", 0x00, (unsigned char *)DRAM_START, (unsigned char *)DRAM_END, nop, 1, 1);
    test_bytes("ONES: ", 0xFF, (unsigned char *)DRAM_START, (unsigned char *)DRAM_END, nop, 1, 1);
    test_bytes("ALTS: ", 0xAA, (unsigned char *)DRAM_START, (unsigned char *)DRAM_END, toggle, 1, 1);
    test_bytes("ADDR: ", 0, (unsigned char *)DRAM_START, (unsigned char *)DRAM_END, inc_addr, 1, 1);
#endif

#ifdef TEST_REFRESH
    tty_puts("*** Testing REFRESH\n");
    test_longs("LONG: ", DRAM_START, (unsigned long *)DRAM_START, (unsigned long *)DRAM_END, inc_addr, 32, 32);
    test_shorts("SHORT: ", 0, (unsigned short *)DRAM_START, (unsigned short *)DRAM_END, inc_addr, 32, 32);
    test_bytes("BYTE: ", 0, (unsigned char *)DRAM_START, (unsigned char *)DRAM_END, inc_addr, 32, 32);
#endif

    // Done
    tty_puts("Done\n");
}

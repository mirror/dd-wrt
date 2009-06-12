//-------------------------------------------------------------------
//
// gps4020_download.c
//
//-------------------------------------------------------------------
//
// Copyright (C) 2003, MLB Associates.
//
// Routine to download code into main [external] SRAM and then
// execute it.
//
//-------------------------------------------------------------------

//#define DEBUG_DOWNLOAD

extern void tty_init(void);
extern unsigned char tty_getc(int chan);
extern void tty_putc(int chan, char c);
extern void tty_puts(int chan, char *c);
extern void tty_puthex(int chan, unsigned long val, int width);

typedef int bool;
#define false 0
#define true  1

// Validate a hex character
__inline__ static bool
_is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||            
            ((c >= 'a') && (c <= 'f')));
}

// Convert a single hex nibble
__inline__ static int
_from_hex(char c) 
{
    int ret = 0;

    if ((c >= '0') && (c <= '9')) {
        ret = (c - '0');
    } else if ((c >= 'a') && (c <= 'f')) {
        ret = (c - 'a' + 0x0a);
    } else if ((c >= 'A') && (c <= 'F')) {
        ret = (c - 'A' + 0x0A);
    }
    return ret;
}


//
// Process the next hex value from a string
//
unsigned long
_hex(char **_cp)
{
    unsigned long val;
    unsigned char *cp = *_cp;

    val = 0;
    while (_is_hex(*cp)) {
        val = (val << 4) | _from_hex(*cp++);
    }
    *_cp = cp;
    return val;
}

//
// Simple GDB protocol handler
// Only three commands are handled:
//   $M<loc>,<len>:<data...>#<cksum>
//     $M20000000,10:0e0000ea18f09fe518f09fe518f09fe5#59
//   $P<reg>=<val>#<cksum>
//   $c#<cksum>
// <cksum> is a simple accumulation of all of the ASCII
// characters following the "$"

static char
_getc(void)
{
    char c;
    c = tty_getc(0);
#ifdef DEBUG_DOWNLOAD
    tty_putc(1, c);
#endif
    return c;
}

void 
GDB_protocol(void)
{
    char c;
    unsigned char cksum, _cksum;
    unsigned char line[64];
    char *cp;
    bool eol, ok;
    unsigned char *loc;
    unsigned long len;
    unsigned char val;
    void (*entry_address)(void);

    while (true) {
        // Wait for a '$'
        do {
            c = _getc();
            // HACK - '>>' indicates to jump to FLASH/ROM
            if (c == '>') {
                c = _getc();
                if (c == '>') {
                    entry_address = (void *)0x60000000;
                    (*entry_address)();
                }
            }
        } while (c != '$');
        cksum = 0;
        eol = false;
        cp = line;
        while (!eol) {
            c = _getc();
            if (c != '#') {
                cksum += c;
                *cp++ = c;
                *cp = '\0';
            } else {
                c = _getc();
                _cksum = _from_hex(c) << 4;
                c = _getc();
                _cksum |= _from_hex(c);
                ok = (cksum == _cksum);
                tty_putc(0, ok ? '+' : '-');
                eol = true;
            }
        }
#ifdef DEBUG_DOWNLOAD
        tty_puts(1, ok ? " = OK\n" : " = BAD\n");
#endif
        if (ok) {
            // Empty "OK" message
            tty_puts(0, "#00");
            // Process command
            cp = line;
            switch (*cp++) {
            case 'M':
                loc = (unsigned char *)_hex(&cp);
                if (*cp++ != ',') continue;
                len = _hex(&cp);
                if (*cp++ != ':') continue;
                while (len-- > 0) {
                    val = _from_hex(*cp++) << 4;
                    val |= _from_hex(*cp++);
                    *loc++ = val;
                }
                break;
            case 'P':
                // Ignore register #
                cp += 2;
                if (*cp++ != '=') continue;
                entry_address = (void *)_hex(&cp);
                break;
            case 'c':
                (*entry_address)();
                break;
            default:
                tty_puts(0, "** unknown command: $");
                tty_puts(0, line);
            }
        }
    }
}

int
main(void)
{
    tty_init();
    // Change 0x6XXXXXXX to use external ROM
    *(volatile short *)0x4010100C |= (1<<9);
    tty_puts(0, "Ready to download >>");
#ifdef DEBUG_DOWNLOAD
    tty_puts(1, "... GDB data:\n");
#endif
    GDB_protocol();
    return 0;  // Never happens!
}

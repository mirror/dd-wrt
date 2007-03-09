//==========================================================================
//
//      shex.c
//
//      RedBoot SHEX format parser
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    dwmw2
// Contributors: dwmw2
// Date:         2002-01-11
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>
#include <xyzModem.h>

extern void redboot_getc_terminate(bool abort);

//
// Process a set of Symbol HEX records, loading the contents into memory.  
// Note: A "base" value must be provided, and the data will be relocated
// relative to that location.  Of course, this can only work for
// the first section of the data, so if there are non-contiguous
// pieces of data, they will end up relocated in the same fashion.
// Because of this, "base" probably only makes sense for a set of
// data which has only one section, e.g. a ROM image.
//

static long
_shex2(int (*getc)(void), int len, long *sum)
{
    int val, byte;
    char c1, c2;

    val = 0;
    while (len-- > 0) {
        c1 = (*getc)();
        c2 = (*getc)();
        if (_is_hex(c1) && _is_hex(c2)) {
            val <<= 8;
            byte = (_from_hex(c1)<<4) | _from_hex(c2);
            val |= byte;
            if (sum) {
                *sum += c1+c2;
            }
        } else {
            return (-1);
        }
    }
    return (val);
}


static int
_decode_compressed(int (*getc)(void), unsigned long addr, long count, long *sum, long line_nr)
{
    char *buf = (char *)addr;
    int  i, b, b1, state, oldstate;
    long repeat = 1;

    state = oldstate = '}';

    while (count-- > 0) {
	if ((b = getc()) < 0) {
	    redboot_getc_terminate(true);
	    diag_printf("Failed to read SHEX data at line %d\n", line_nr);
	    return -1;
	}
	*sum += b;

	switch (b) {
	case '~':   // shift char
	    oldstate = state;
	    // fall through
	case '{':   // escape in
	case '}':   // escape out
	    state = b;
	    break;
	case '|':   // repeat
	    if (count < 2) {
		redboot_getc_terminate(true);
		diag_printf("Bad count at line %d\n", line_nr);
		return -1;
	    }
	    count -= 2;
	    repeat = _shex2(getc, 1, sum);
	    if (repeat < 0) {
		redboot_getc_terminate(true);
		diag_printf("Bad repeat count in compressed data record at line %d\n", line_nr);
		return -1;
	    }
	    break;
	default:    // data
	    if (state != '}' && (state != '~' || oldstate != '{')) {
		// decode ascii encoded data, then copy repeat chars to buf
		if (count-- <= 0) {
		    redboot_getc_terminate(true);
		    diag_printf("Bad count at line %d\n", line_nr);
		    return -1;
		}
		if ((b1 = getc()) < 0) {
		    redboot_getc_terminate(true);
		    diag_printf("Failed to read SHEX data at line %d\n", line_nr);
		    return -1;
		}
		*sum += b1;
		if (!_is_hex(b) || !_is_hex(b1)) {
		    redboot_getc_terminate(true);
		    diag_printf("Bad ASCII encoded character at line %d\n", line_nr);
		    return -1;
		}
		b = (_from_hex(b)<<4) | _from_hex(b1);
	    }
	    for (i = 0; i < repeat; i++)
		*buf++ = b;

	    if (state == '~')
		state = oldstate;

	    repeat = 1;
	    break;
	}
    }
    return (unsigned long)buf - addr;
}

int shex_image_number = -1;

unsigned long
load_shex_image(int (*getc)(void), unsigned long base, bool base_addr_set)
{
    int  c, len;
    long line_nr = 0, count, sum, cksum;
    long megabyte_ofs = 0;
    long paragraph_ofs = 0;
    long payload_ofs = 0;
    int type;
    unsigned long addr = base;
    int end = 0;
    unsigned long highest_address;

    if (!base_addr_set) {
	redboot_getc_terminate(true);
	diag_printf("SHEX load requires a memory address\n");
	return 0;
    }

    while (!end && (c = (*getc)()) >= 0) {
	line_nr++;
	len = 0;
        // Start of line
        if (c != ':') {
	    redboot_getc_terminate(true);
            diag_printf("Invalid SHEX record at line %d, no colon, char 0x%02x instead\n", 
                   line_nr, (unsigned char)c);
            return 0;
        }

        sum = 0;
        if ((count = _shex2(getc, 1, &sum)) < 0) {
	    redboot_getc_terminate(true);
            diag_printf("Bad SHEX line length at line %d\n", line_nr);
            return 0;
        }

        if ((payload_ofs = _shex2(getc, 2, &sum)) < 0) {
	    redboot_getc_terminate(true);
            diag_printf("Bad SHEX line start address at line %d\n", line_nr);
            return 0;
	}

	type = _shex2(getc, 1, &sum);
	if (shex_image_number == -1 && type != 0xA0) {
	    redboot_getc_terminate(true);
	    diag_printf("SHEX didn't start with Header record\n");
	    return 0;
	}

	switch (type) {
	case 0xA0: /* Header record. Must come first */
	    if (count != 0x0a) {
		redboot_getc_terminate(true);
		diag_printf("SHEX Header record had wrong length (%d not 10) at line %d\n", count, line_nr);
		return 0;
	    }
	    if (payload_ofs) {
		redboot_getc_terminate(true);
		diag_printf("SHEX Header record had non-zero payload addr at line %d\n", line_nr);
		return 0;
	    }
	    if ((shex_image_number = _shex2(getc, 1, &sum)) < 0) {
		redboot_getc_terminate(true);
		diag_printf("Bad SHEX header record image number\n", line_nr);
		return 0;
	    }
	    if ((long)(highest_address = _shex2(getc, 4, &sum)) < 0) {
		redboot_getc_terminate(true);
		diag_printf("Bad SHEX header record image size at line %d\n", line_nr);
		return 0;
	    }
	    break;

	case 0xA1: /* Start of Data Record */ {
	    int filetype;

	    if (count != 0x02) {
		redboot_getc_terminate(true);
		diag_printf("SHEX Start of Data record had wrong length (%d not 2) at line %d\n", count, line_nr);
		return 0;
	    }
	    if ((filetype = _shex2(getc, 1, &sum)) < 0) {
		redboot_getc_terminate(true);
		diag_printf("Bad SHEX image type in Start of Data record\n");
		return 0;
	    }
	    if (filetype != 1) {
		redboot_getc_terminate(true);
		diag_printf("Bad SHEX image type 0x%02x in Start of Data record\n", filetype);
		return 0;
	    }
	    break;
	}

	case 0xA2: /* Comment */
	    while(count--)
		sum += getc();
	    break;

	case 0xA3: /* Megabyte Address Extension Record */
	    if (count != 0x04) {
		redboot_getc_terminate(true);
		diag_printf("SHEX Megabyte record had wrong length (%d not 4) at line %d\n", count, line_nr);
		return 0;
	    }
	    if ((megabyte_ofs =_shex2(getc, 2, &sum)) < 0) {
		redboot_getc_terminate(true);
		diag_printf("Bad SHEX Megabyte extension record\n");
		return 0;
	    }
	    megabyte_ofs *= 0x100000;
	    addr = (base + megabyte_ofs + paragraph_ofs);
	    break;
	
	case 0xA4: /* Address Break Record */
	    /* We actually ignore this - we don't enforce contiguity */
	    if (count != 0x08) {
		redboot_getc_terminate(true);
		diag_printf("SHEX Megabyte record had wrong length (%d not 4) at line %d\n", count, line_nr);
		return 0;
	    }
	    if (_shex2(getc, 4, &sum) < 0) {
		redboot_getc_terminate(true);
		diag_printf("Bad SHEX Address Break record\n");
		return 0;
	    }
	    break;
	

	case 0x02: /* Paragraph Address Extension Record */
	    if (count != 0x04 && count != 2 /* See below */) {
		redboot_getc_terminate(true);
		diag_printf("SHEX Paragraph record had wrong length (%d not 2) at line %d\n", count, line_nr);
		return 0;
	    }
	    if ((paragraph_ofs =_shex2(getc, 2, &sum)) < 0) {
		redboot_getc_terminate(true);
		diag_printf("Bad SHEX Paragraph extension record\n");
		return 0;
	    }
	    paragraph_ofs *= 16;
	    addr = (base + megabyte_ofs + paragraph_ofs);
	    if (count == 2) {
		/* Workaround for brain death - Symbol 2800 IPL doesn't
		   accept valid Paragraph records so we have to accept
		   the broken ones that it _does_ accept. They in fact
		   have an ihex checksum, not an shex checksum. We just
		   skip the checksum, not actually implement ihex cksum.
		*/
		getc();
		getc();
		goto shex_eol;
	    }
	    break;

	case 0x84: /* Binary data record */
	    if (addr != (base + megabyte_ofs + paragraph_ofs + payload_ofs)) {
		    redboot_getc_terminate(true);
		    diag_printf("SHEX file non-contiguous at line %d - record at 0x%08x not %08x\n",
				line_nr, (base + megabyte_ofs + paragraph_ofs + payload_ofs), addr );
		    return 0;
	    }

#ifdef CYGSEM_REDBOOT_VALIDATE_USER_RAM_LOADS
            if ((addr < (unsigned long)user_ram_start) || (addr > (unsigned long)user_ram_end)) {
	      // Only if there is no need to stop the download before printing
	      // output can we ask confirmation questions.
	      redboot_getc_terminate(true);
	      diag_printf("*** Warning! Attempt to load SHEX-record to address: %p\nRedBoot does not believe this is in RAM\nUse TFTP for a chance to override this.\n",(void*)addr);
            }
#endif

	    while(count--) {
		int b;
		
		if ((b = getc()) < 0) {
		    redboot_getc_terminate(true);
		    diag_printf("Failed to read SHEX data at line %d\n", line_nr);
		    return 0;
		}

		sum += b;
		*(char *)(addr++) = b;
	    }
	    break;
	

	case 0x00: /* Standard Data Record */
	    while (count >= 2) {
		long b;
		b = _shex2(getc, 1, &sum);
		if (b < 0) {
		    redboot_getc_terminate(true);
		    diag_printf("Bad ASCII encoded byte in standard record at line %d\n", line_nr);
		    return 0;
		}
		*(char *)(addr++) = b;
		count -= 2;
	    }
	    if (count) {
		redboot_getc_terminate(true);
		diag_printf("Bad count in standard record at line %d\n", line_nr);
		return 0;
	    }
	    break;

	case 0x82: /* Compressed Data Record */
	    if (addr != (base + megabyte_ofs + paragraph_ofs + payload_ofs)) {
		redboot_getc_terminate(true);
		diag_printf("SHEX file non-contiguous at line %d - record at 0x%08x not %08x\n",
			    line_nr, (base + megabyte_ofs + paragraph_ofs + payload_ofs), addr );
		return 0;
	    }
	    len = _decode_compressed(getc, addr, count, &sum, line_nr);
	    if (len < 0)
		return 0;
	    addr += len;
	    break;

	case 0x01:
	    if(count) {
		redboot_getc_terminate(true);
		diag_printf("SHEX End of File Record has payload at line %d\n", line_nr);
		return 0;
	    }
	    /* Hack. See the Paragraph Address Record code */
	    getc();
	    getc();
	    end = 1;
	    goto shex_eol;

	default:
	        redboot_getc_terminate(true);
		diag_printf("Unknown SHEX record type %02X at line %d\n", type, line_nr);
		return 0;

	}

	if ((cksum = _shex2(getc, 1, NULL)) < 0) {
	        redboot_getc_terminate(true);
		diag_printf("Bad SHEX checksum at line %d\n", line_nr);
		return 0;
	}
	if (cksum != ((-sum)&0xff)) {
	    redboot_getc_terminate(true);
	    diag_printf("Bad SHEX checksum (0x%02x not 0x%02x) at line %d\n",
			(-sum) & 0xff, cksum, line_nr);
	    return 0;
	}
    shex_eol:
	c = getc();
	if (c == 0x0d)
		c = getc();
	if (c != 0x0a) {
	    redboot_getc_terminate(true);
	    diag_printf("SHEX end-of-line missing at line %d\n", line_nr);
	    return 0;
	}
    }

    redboot_getc_terminate(true);
    diag_printf("SHEX file loaded %p-%p\n", 
		(void*)base, (void *)addr);

    // Save load base/top
    load_address = base;
    load_address_end = addr;
    entry_address = base;               // best guess

    return 0;
}


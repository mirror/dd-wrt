/*=============================================================================
//
//      calmbreaker.c
//
//      Host to CalmBreaker communication utility.
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: msalter
// Date:         2001-03-02
// Purpose:      
// Description:  Host to CalmBreaker communication utility.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <termios.h>
#include <fcntl.h>
#include <stdio.h>

int ser_fd;  // The com port
int is_risc16;

void
tty_setup(int fd)
{
    struct termios t;

    memset(&t, 0, sizeof(struct termios));

    t.c_oflag = 0;
    t.c_lflag = 0;
    
    t.c_cflag &= ~(CSIZE | PARENB);
    t.c_cflag |= CS8 | CREAD /*| CSTOPB*/;
    t.c_cflag |= CLOCAL;                   // ignore modem status lines

    t.c_iflag = IGNBRK | IGNPAR /* | ISTRIP */ ;

    t.c_lflag &= ~ICANON;                  // non-canonical mode
    t.c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHOKE);           

    t.c_cc[VMIN]  = 0;
    t.c_cc[VTIME] = 10;  // 1 second timeout

    t.c_cflag &= ~CRTSCTS;

    /* set speed */
    cfsetispeed(&t, B57600);
    cfsetospeed(&t, B57600);

    tcdrain(fd);

    if (tcsetattr(fd, TCSANOW, &t) < 0)
    {
	perror("tcssetattr");
	exit(1);
    }
}

static void
do_putc(unsigned char ch)
{
    // keep retrying until sent or an error is returned
    while (write(ser_fd, &ch, 1) == 0);
}

static int
do_getc(unsigned char *ucp)
{
    return read(ser_fd, ucp, 1);
}

static int
wait_for_ack(void)
{
    unsigned char ch;
    int i;

    // try for 5 seconds
    for (i = 0; i < 5; i++) {
	if (do_getc(&ch)) {
	    if (ch == '+')
		return 1;
	    printf("Bad ack [0x%x]\n", ch);
	}
    }
    printf("No ack\n");
    return 0;
}

// Add prefix and checksum to packet and send to MDS board.
void
send_packet_noack(unsigned char *pkt, int len)
{
    unsigned char cksum = 0;

    do_putc(':');

    while (len-- > 0) {
	cksum += *pkt;
	do_putc(*pkt++);
    }

    do_putc(0x100 - cksum);
}

// Add prefix and checksum to packet and send to MDS board.
void
send_packet(unsigned char *pkt, int len)
{
    send_packet_noack(pkt, len);
    wait_for_ack();
}



// Send a packet of code or data (max 0xff bytes)
int
send_data_packet(int is_data, unsigned addr, unsigned char *buf, int buflen)
{
    unsigned char uc, cksum = 0;
    int i;
    
    do_putc(':');
    
    // code or data?
    uc = is_data ? 0x52 : 0x51;
    cksum += uc;
    do_putc(uc);

    // code/data nwords?
    uc = buflen >> 1;
    cksum += uc;
    do_putc(uc);

    // code/data address
    uc = (unsigned char)(addr >> 24);
    cksum += uc;
    do_putc(uc);
    uc = (unsigned char)(addr >> 16);
    cksum += uc;
    do_putc(uc);
    uc = (unsigned char)(addr >> 8);
    cksum += uc;
    do_putc(uc);
    uc = (unsigned char)addr;
    cksum += uc;
    do_putc(uc);

    while (buflen-- > 0) {
	cksum += *buf;
	do_putc(*buf++);
    }

    do_putc(0x100 - cksum);

    return wait_for_ack();
}


void
send_command(unsigned char uc)
{
    send_packet(&uc, 1);
}

void
send_command_noack(unsigned char uc)
{
    send_packet_noack(&uc, 1);
}

// Simple single-byte commands
void target_reset(void) { send_command(0x20); }
void target_singlestep(void) { send_command(0x22); }
void target_singlecycle(void) { send_command(0x23); }
void target_stop(void) { send_command(0x24); }
void target_run(void) { send_command_noack(0x25); }

#define DOWNLOAD_CHUNK_SIZE 254

int
download_words(int is_data, unsigned addr, unsigned char *buf, int buflen)
{
    while (buflen >= DOWNLOAD_CHUNK_SIZE) {
	if (!send_data_packet(is_data, addr, buf, DOWNLOAD_CHUNK_SIZE)) {
	    printf("Error downloading %d bytes of %s to 0x%x\n",
		   DOWNLOAD_CHUNK_SIZE, (is_data ? "data" : "code"), addr);
	    return 0;
	}
	addr += DOWNLOAD_CHUNK_SIZE;
	buf += DOWNLOAD_CHUNK_SIZE;
	buflen -= DOWNLOAD_CHUNK_SIZE;
    }
    if (buflen && !send_data_packet(is_data, addr, buf, buflen)) {
	printf("Error downloading %d bytes of %s to 0x%x\n",
	       buflen, (is_data ? "data" : "code"), addr);
	return 0;
    }
    return 1;
}

static inline int
gethexnibble(FILE *fp)
{
    int ch;

    ch = getc(fp);
    if (ch >= '0' && ch <= '9')
	return (ch - '0');
    if (ch >= 'a' && ch <= 'f')
	return (ch - 'a' + 10);
    if (ch >= 'A' && ch <= 'F')
	return (ch - 'A' + 10);

    if (ch == EOF)
	fprintf(stderr, "Unexpected EOF\n");
    else
	fprintf(stderr, "Bad hex char\n");

    return -1;
}


static inline int
gethexbyte(FILE *fp)
{
    int nib;
    unsigned char n;

    if ((nib = gethexnibble(fp)) < 0)
	return -1;
    n = nib << 4;
    if ((nib = gethexnibble(fp)) < 0)
	return -1;
    n |= nib;
    return n;
}

static inline int
chk_cksum(FILE *fp, unsigned int cksum)
{
    int n;

    if ((n = gethexbyte(fp)) < 0)
	return -1;

    cksum = ~cksum & 0xff;

    if (cksum != n) {
	fprintf(stderr, "Bad cksum[%02x]\n", cksum);
	return -1;
    }
    return 0;
}

int
load_srec(FILE *fp, int is_data)
{
    int count, dcount, data, n, addr_bytes = 0, is_term, is_comment;
    unsigned long address, cksum;
    unsigned char data_buf[256];

    is_comment = is_term = 0;

    do {
	if ((n = getc(fp)) == EOF)
	    return 1;
    } while (n != 'S');
	    
    switch (n = gethexnibble(fp)) {
      case 0:
      case 5:
	is_comment = 1;
	break;

      case 1:
      case 2:
      case 3:
	addr_bytes = n + 1;
	break;

      case 7:
      case 8:
      case 9:
	is_term = 1;
	addr_bytes = 11 - n;
	break;

      default:
	if (n < 0)
	    return -1;
	fprintf(stderr, "Bad record type: %d\n", n);
	return -1;
    }

    if ((count = gethexbyte(fp)) < 0)
	return -1;

    cksum = count;

    --count; // don't count chksum

    if (is_comment) {
	while (count > 0) {
	    if ((n = gethexbyte(fp)) < 0)
		return -1;
	    cksum += n;
	    --count;
	}
	if (chk_cksum(fp,cksum) < 0)
	    return -1;
	return 0;
    }

    address = 0;
    while (count > 0 && addr_bytes) {
	if ((n = gethexbyte(fp)) < 0)
	    return -1;
	cksum += n;
	address = (address << 8) | n;
	--addr_bytes;
	--count;
    }

    if (is_risc16 && (address & 0x400000))
	address &= 0x3fffff;

    if (is_term) {
	if (count || addr_bytes) {
	    fprintf(stderr, "Malformed record cnt[%d] abytes[%d]\n",
		    count, addr_bytes);
	    return -1;
	}
	if (chk_cksum(fp, cksum) == 0) {
	    fprintf(stderr, "Setting start address: 0x%08x\n", address);
	    return 1;
	}
	return -1;
    }

    for (dcount = 0; dcount < count; dcount++) {
	if ((data = gethexbyte(fp)) < 0)
	    return -1;
	cksum += data;
	data_buf[dcount] = data;
    }
    
    if (chk_cksum(fp, cksum) < 0)
	return -1;

    if (dcount & 1)
	dcount++;

    if (!download_words(is_data, address, data_buf, dcount))
	return -1;
    
    return 0;
}


int
load_hex(FILE *fp, int is_data)
{
    int count, n, i;
    unsigned long address;
    unsigned char data_buf[256];
    
    do {
	if ((n = getc(fp)) == EOF)
	    return 1;
    } while (n == '\r' || n == '\n');

    if (n == '#') {
	do {
	    if ((n = getc(fp)) == EOF)
		return 1;
	} while (n != '\r' && n != '\n');
	return 0;
    }

    if (n != ':') {
	fprintf(stderr, "Unrecognized HEX line start.\n");
	return -1;
    }

    if ((count = gethexbyte(fp)) < 0)
	return -1;

    address = 0;
    for (i = 0; i < 4; i++) {
	if ((n = gethexbyte(fp)) < 0)
	    return -1;
	address = (address << 8) | n;
    }

    // skip type byte
    if ((n = gethexbyte(fp)) < 0)
	return -1;

    for (i = 0; i < count; i++) {
	if ((n = gethexbyte(fp)) < 0)
	    return -1;
	data_buf[i] = n;
    }

    // skip chksum byte
    if ((n = gethexbyte(fp)) < 0)
	return -1;

    if (count & 1)
	++count;

    if (!download_words(is_data, address, data_buf, count))
	return -1;
    
    return 0;
}


int
main(int argc, char *argv[])
{
    int i;
    int do_download = 0, is_data, is_hex;
    int do_run = 0, do_reset = 0;
    char *filename = NULL;
    char *portname = "/dev/ttyS0";
    FILE *infile;

    if (argc == 1) {
	fprintf(stderr, "Usage: mds_talk [--run] [--reset] [--srec-code | --srec-date | --hex-code | --hex-data] [-f filename] [-p serial_dev]\n");
	exit(1);
    }

    is_risc16 = 0;
    for (i = 1; i < argc; i++) {
	if (!strcmp(argv[i], "--srec-code")) {
	    do_download = 1;
	    is_data = 0;
	    is_hex = 0;
	} else if (!strcmp(argv[i], "--srec-data")) {
	    do_download = 1;
	    is_data = 1;
	    is_hex = 0;
	} else if (!strcmp(argv[i], "--hex-code")) {
	    do_download = 1;
	    is_data = 0;
	    is_hex = 1;
	} else if (!strcmp(argv[i], "--hex-data")) {
	    do_download = 1;
	    is_data = 1;
	    is_hex = 1;
	} else if (!strcmp(argv[i], "--reset"))
	    do_reset = 1;
	else if (!strcmp(argv[i], "--run"))
	    do_run = 1;
	else if (!strcmp(argv[i], "-f")) {
	    if (++i >= argc) {
		fprintf(stderr, "Missing filename\n");
		exit(1);
	    }
	    filename = argv[i];
	} else if (!strcmp(argv[i], "-p")) {
	    if (++i >= argc) {
		fprintf(stderr, "Missing serial port name\n");
		exit(1);
	    }
	    portname = argv[i];
	} else if (!strcmp(argv[i], "--risc16")) {
	    is_risc16 = 1;
	} else {
	    fprintf(stderr, "Unknown argument \"%s\"\n", argv[i]);
	    exit(1);
	}
    }

    if ((ser_fd = open(portname, O_RDWR | O_NOCTTY)) < 0) {
	fprintf(stderr, "Can't open port %s\n", portname);
	exit(1);
    }
    tty_setup(ser_fd);

    if (filename) {
	if ((infile = fopen(filename, "r")) == NULL) {
	    fprintf(stderr, "Can't open file %s\n", filename);
	    exit(1);
	}
    } else
	infile = stdin;

    if (do_reset)
	target_reset();

    if (do_download) {
	if (is_hex)
	    while (!load_hex(infile, is_data)) ;
	else
	    while (!load_srec(infile, is_data)) ;
    }

    if (do_run)
	target_run();
}

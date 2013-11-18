/* $Id: gpsflash.h 3670 2006-10-27 01:45:12Z esr $ */
/*
 * Copyright (c) 2005 Chris Kuethe <chris.kuethe@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _GPSFLASH_H_
#define _GPSFLASH_H_

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <netinet/in.h>	/* for htonl() under Linux */

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>

struct flashloader_t {
    const char *name;
    const char *flashloader;
    size_t min_loader_size, max_loader_size;
    size_t min_firmware_size, max_firmware_size;
    int (*probe)(int fd, char **version);
    int (*port_setup)(int fd, struct termios *term);
    int (*version_check)(int fd, const char *, 
			 const char *, size_t, 
			 const char *, size_t);
    int (*stage1_command)(int fd);
    int (*loader_send)(int pfd, struct termios *term, char *loader, size_t ls);
    int (*stage2_command)(int fd);
    int (*firmware_send)(int pfd, char *loader, size_t ls);
    int (*stage3_command)(int fd);
    int (*port_wrapup)(int fd, struct termios *term);
};
extern struct flashloader_t sirf_type;

int serialConfig(int, struct termios *, int);
int serialSpeed(int, struct termios *, int);
int srecord_send(int pfd, char *fw, size_t len);
int binary_send(int pfd, char *data, size_t ls);
bool expect(int pfd, const char *str, size_t len, time_t timeout);

#endif /* _GPSFLASH_H_ */

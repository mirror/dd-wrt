/*-
 * Copyright (c) 2001 Lev Walkin <vlm@lionet.info>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: rw.c,v 1.5 2004/03/06 05:31:12 vlm Exp $
 */

#include "rflow.h"
#include "opt.h"

ssize_t
read_timeout(int sockfd, void *buf, size_t nbytes, int to_ms) {
	ssize_t size = -1;
	ssize_t ss;
	int rc;
	struct pollfd pfd;

	pfd.fd = sockfd;
	pfd.events = POLLIN;

	while(1) {

		rc = poll(&pfd, 1, to_ms);

		if(rc == -1) {
			if(signoff_now)
				return -1;
			if(errno == EINTR)
				continue;
			return size;
		}

		/* poll() timeout */
		if(rc == 0)
			return size;

		ss = read(sockfd, buf, nbytes);

		if(ss == -1) {
			if(signoff_now)
				return -1;
			if(errno == EINTR)
				continue;
			return size;
		}

		if(size == -1)
			size = 0;

		/* Read timeout */
		if(!ss)
			return size;

		size += ss;
		if((ssize_t)(nbytes -= ss) <= 0)
			return size;
		buf += ss;

	}

	return size;
}

ssize_t
safe_write(int sockfd, void *buf, size_t nbytes) {
	ssize_t size = 0;
	ssize_t ss;

	while( (ss = write(sockfd, buf, nbytes)) ) {

		if(ss == -1) {
			if(signoff_now)
				return -1;
			if(errno == EINTR)
				continue;
			return size;
		}

		size += ss;
		if( (ssize_t)(nbytes -= ss) <= 0 )
			return size;
		buf += ss;
	}

	return size;
}


/*
 * Copyright (c) 2005, Bruno Randolf <bruno.randolf@4g-systems.biz>
 * Copyright (c) 2004, Andreas Tønnesen(andreto-at-olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * * Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 * * Neither the name of the UniK olsr daemon nor the names of its contributors 
 *   may be used to endorse or promote products derived from this software 
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* $Id: nameservice_msg.h,v 1.6 2005/03/17 21:41:30 br1 Exp $ */

/*
 * Dynamic linked library for UniK OLSRd
 */
 
#ifndef _NAMESEVICE_MSG
#define _NAMESEVICE_MSG


typedef enum {
	NAME_HOST = 0,
	NAME_FORWARDER = 1,
	NAME_SERVICE = 2
} NAME_TYPE;


struct name
{
	olsr_u16_t		type;
	olsr_u16_t		len;	// length of the name
	union olsr_ip_addr	ip;
	/*
	 * name is written in plain text after this struct and padded to 4 byte
	 */
};


struct namemsg
{
	olsr_u16_t version;
	olsr_u16_t nr_names;   // number of following name messages
	/*
	 * at least one struct name following
	 */
};

#endif

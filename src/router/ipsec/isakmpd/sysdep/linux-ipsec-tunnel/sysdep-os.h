/*                _______              ___       -*- linux-c -*-
 *      _________/ _____ \____________/ _ \____________________  
 * __  |____ _ _ _|_   _| __ ___  _ __ (_)_  __  ___ ___  _ __ |__  
 * \ \ / / _` | '_ \| || '__/ _ \| '_ \| \ \/ / / __/ _ \| '_ ` _ \ 
 *  \ V / (_| | | | | || | | (_) | | | | |>  < | (_| (_) | | | | | |
 *   \_/ \__,_|_| |_|_||_|  \___/|_| |_|_/_/\_(_)___\___/|_| |_| |_|
 *     | contact: secteam@vantronix.net, https://vantronix.net |               
 *     +-------------------------------------------------------+
 * 
 * isakmpd sysdeps for the linux ipsec implementation "ipsec_tunnel"
 *
 * Copyright (c) 2002 Reyk Floeter.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _SYSDEP_OS_H_
#define _SYSDEP_OS_H_

#undef HAVE_STRLCPY
#undef HAVE_STRLCAT

#include <netinet/in.h>
#include <time.h>
#include <sys/types.h>
#include "strlcpy.h"
#include "strlcat.h"

#ifndef linux
#define linux
#endif

#define uh_sport source
#define uh_dport dest
#define uh_ulen len
#define uh_sum check

#define IPV6_VERSION 0x1 /* IPV6 is currently not supported... 
			    use any value here *g* */

#define DL_LAZY RTLD_LAZY

#endif /* _SYSDEP_OS_H_ */

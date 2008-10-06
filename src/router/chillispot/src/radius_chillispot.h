/* 
 * Radius client functions.
 *
 * Copyright (c) 2006, Jens Jakobsen 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 *   Neither the names of copyright holders nor the names of its contributors
 *   may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * Copyright (C) 2005 Mondru AB.
 *
 * The contents of this file may be used under the terms of the GNU
 * General Public License Version 2, provided that the above copyright
 * notice and this permission notice is included in all copies or
 * substantial portions of the software.
 *
 */

#ifndef _RADIUS_CHILLISPOT_H
#define _RADIUS_CHILLISPOT_H

#define RADIUS_VENDOR_CHILLISPOT                      14559
#define	RADIUS_ATTR_CHILLISPOT_MAX_INPUT_OCTETS           1 /* integer */
#define	RADIUS_ATTR_CHILLISPOT_MAX_OUTPUT_OCTETS          2 /* integer */
#define	RADIUS_ATTR_CHILLISPOT_MAX_TOTAL_OCTETS           3 /* integer */
#define	RADIUS_ATTR_CHILLISPOT_BANDWIDTH_MAX_UP	          4 /* integer */
#define	RADIUS_ATTR_CHILLISPOT_BANDWIDTH_MAX_DOWN         5 /* integer */

#define	RADIUS_ATTR_CHILLISPOT_UAM_ALLOWED              100 /* integer */
#define	RADIUS_ATTR_CHILLISPOT_MAC_ALLOWED              101 /* integer */
#define	RADIUS_ATTR_CHILLISPOT_INTERVAL                 102 /* integer */

#define	RADIUS_SERVICE_TYPE_CHILLISPOT_AUTHORIZE_ONLY 0x38df0001 /* integer */

#endif	/* !_RADIUS_CHILLISPOT_H */

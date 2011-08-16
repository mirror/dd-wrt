/*
** Copyright (C) 2009-2011 Sourcefire, Inc.
**
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef SDF_CREDIT_CARD__H
#define SDF_CREDIT_CARD__H

#include <stdint.h>
#include "spp_sdf.h"

#define ISSUER_SIZE 4
#define CC_COPY_BUF_LEN 20 /* 16 digits + 3 spaces/dashes + null */
#define MIN_CC_BUF_LEN 15 /* 13 digits + 2 surrounding non-digits */

int SDFLuhnAlgorithm(char *buf, uint32_t buflen, struct _SDFConfig *config);

#endif /* SDF_CREDIT_CARD__H */

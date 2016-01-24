#ifndef TOOLS_EXTERN_H
#define TOOLS_EXTERN_H
/*
 *  Copyright (C) 1999  James Antill
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  email: james@and.org
 */

#define SWAP_TYPE(x, y, type) do { \
 type internal_local_tmp = (x); \
 (x) = (y); \
 (y) = internal_local_tmp; \
 } while (FALSE)

#define BUF_NUM_TYPE_SZ(x) ((sizeof(x) * CHAR_BIT) + 1)

#define C_RET1_P2(func, type, x, y)    ((const type *) (func (x, y)))
#define N_RET1_P2(func, type, x, y)    (func ((type *)x, y))
#define C_RET1_P3(func, type, x, y, z) ((const type *) (func (x, y, z)))
#define N_RET1_P3(func, type, x, y, z) (func ((type *)x, y, z))

#define C_strchr(x, y)     C_RET1_P2(strchr, char, x, y)
#define N_strchr(x, y)     N_RET1_P2(strchr, char, x, y)
#define C_strrchr(x, y)    C_RET1_P2(strrchr, char, x, y)
#define N_strrchr(x, y)    N_RET1_P2(strrchr, char, x, y)
#define C_memchr(x, y, z)  C_RET1_P3(memchr, void, x, y, z)
#define N_memchr(x, y, z)  N_RET1_P3(memchr, void, x, y, z)
#define C_strnchr(x, y, z) C_RET1_P3(strnchr, char, x, y, z)
#define N_strnchr(x, y, z) N_RET1_P3(strnchr, char, x, y, z)
#define C_strstr(x, y)     C_RET1_P2(strstr, char, x, y)
#define N_strstr(x, y)     N_RET1_P2(strstr, char, x, y)

#define CONST_STRLEN(x) (sizeof(x) - 1)

#define BEG_CONST_STRCMP(x, y) (strncmp(x, y, CONST_STRLEN(x)))
#define BEG_CONST_STRCASECMP(x, y) (strncasecmp(x, y, CONST_STRLEN(x)))

#endif

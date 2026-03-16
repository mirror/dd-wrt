/*
 * $Id: pac_key.c,v 1.2.2.2 2020/11/11 16:12:04 karls Exp $
 *
 * Copyright (c) 2009, 2011, 2019
 *      Inferno Nettverk A/S, Norway.  All rights reserved.
 */

#include "common.h"

static const char rcsid[] =
"$Id: pac_key.c,v 1.2.2.2 2020/11/11 16:12:04 karls Exp $";

const licensekey_t module_pac_keyv[1] = {
   {
      .key               = KEY_IPV4,
      .value.ipv4.s_addr = 16777343 /* inet_addr("127.0.0.1") */
   },
};
const int module_pac_keyc = ELEMENTS(module_pac_keyv);

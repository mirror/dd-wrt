/*
 * str_stk.c  - maintains a stack of the components of a bit string
 *        or octet string so they can be copied into a single chunk
 *
 * Copyright (C) 1992 Michael Sample and the University of British Columbia
 *
 * This library is free software; you can redistribute it and/or
 * modify it provided that this copyright/license information is retained
 * in original form.
 *
 * If you modify this file, you must clearly indicate your changes.
 *
 * This source code is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "asn_config.h"
#include "str_stk.h"

/* global for use by AsnBits and AsnOcts */

StrStk strStkG = { NULL, 128, 0, 64, 0, 0 };


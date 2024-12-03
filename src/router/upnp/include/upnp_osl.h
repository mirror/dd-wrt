/*
 * Broadcom UPnP module OS Layer include file
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_osl.h,v 1.7 2007/11/23 09:54:14 Exp $
 */

#ifndef __UPNP_OSL_H__
#define __UPNP_OSL_H__

#if defined(linux)
#include <upnp_linux_osl.h>
#elif defined(__ECOS)
#include <upnp_ecos_osl.h>
#else
#error "Unsupported OSL requested"
#endif
#include "typedefs.h"

#include <bcmnvram.h>

#endif /* __UPNP_OSL_H__ */

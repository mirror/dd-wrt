#ifndef _CONFIG_PP_H
#define _CONFIG_PP_H

#if !defined(HAVE_CONFIG_H)
#include "rtpp_version.h"
#else
#include "config.h"
#define RTPP_SW_VERSION PACKAGE_VERSION
#endif

#if defined(HAVE_ERR_H)
#undef NO_ERR_H
#else
#define NO_ERR_H 1
#endif

#endif

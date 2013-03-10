/*
   Copyright (C) 2010 bg <bg_one@mail.ru>
*/
#ifndef CHAN_DONGLE_APP_H_INCLUDED
#define CHAN_DONGLE_APP_H_INCLUDED

#ifdef BUILD_APPLICATIONS

#include "export.h"		/* EXPORT_DECL EXPORT_DEF */

EXPORT_DECL void app_register();
EXPORT_DECL void app_unregister();

#else /* BUILD_APPLICATIONS */

#define app_register()
#define app_unregister()

#endif /* BUILD_APPLICATIONS */
#endif /* CHAN_DONGLE_APP_H_INCLUDED */

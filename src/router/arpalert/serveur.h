/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: serveur.h 690 2008-03-31 18:36:43Z  $
 *
 */

#ifndef __SERVER_H
#define __SERVER_H

// run program as daemon
void daemonize(void);

// set security option (user separation, etc ...)
void separe(void);

#endif


/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: loadmodule.h 690 2008-03-31 18:36:43Z  $
 *
 */

// header for definition of this header
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/if_ether.h>

// load alert module
void module_load(void);

// load alert module
void module_unload(void);

// function call when an alert is send
void alerte_mod(struct ether_addr *mac_sender,
                struct in_addr ip_sender,
                int type,
                struct ether_addr *ref_mac,
                struct in_addr ref_ip,
                char *interface,
                char *vendor);


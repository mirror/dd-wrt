/** @internal
 * @file http_microhttpd.c
 * @brief a httpd implementation using libmicrohttpd
 * @author Copyright (C) 2015 Alexander Couzens <lynxis@fe80.eu>
 * @author Copyright (C) 2015-2023 The openNDS contributors <opennds@blue-wave.net>
 * @author Copyright (C) 2015-2023 Modifications and additions by BlueWave Projects and Services <opennds@blue-wave.net>
 */

#ifndef HTTP_MICROHTTPD_H
#define HTTP_MICROHTTPD_H

#include <stddef.h>
#include <stdio.h>
#include <string.h>

struct MHD_Connection;

/** @brief Get an IP's MAC address from the ARP cache.*/
int arp_get(char mac_addr[18], const char req_ip[]);

void start_mhd(void);
void stop_mhd(void);

enum MHD_Result libmicrohttpd_cb (void *cls,
					struct MHD_Connection *connection,
					const char *url,
					const char *method,
					const char *version,
					const char *upload_data, size_t *upload_data_size, void **ptr);


size_t unescape(void * cls, struct MHD_Connection *c, char *src);

#endif // HTTP_MICROHTTPD_H

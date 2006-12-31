/* Copyright (C) 1999 Heikki Vatiainen hessu@cs.tut.fi */

#ifndef JOIN_H
#define JOIN_H

void init_lec_params(unsigned char *mac_addr, char *elan_name,
                     unsigned char *listen_addr, int itf, char *foreId,
                     int max_frame_size, int proxy_flag, int lane_version);
int lec_configure(int lecs_method, struct sockaddr_atmsvc *manual_atm_addr,
                  struct sockaddr_atmsvc *listen_addr);
int les_connect(int lecs_method, struct sockaddr_atmsvc *manual_atm_addr,
                struct sockaddr_atmsvc *listen_addr);
int bus_connect(void);

/* Different ways to contact LECS */
#define LECS_NONE      0
#define LECS_WELLKNOWN 1
#define LECS_MANUAL    2

#endif /* JOIN_H */

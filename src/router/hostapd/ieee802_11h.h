/*
 * hostapd / IEEE 802.11h
 * Copyright 2005-2006, Devicescape Software, Inc.
 * All Rights Reserved.
 */

#ifndef IEEE802_11H_H
#define IEEE802_11H_H

#define SPECT_LOOSE_BINDING	1
#define SPECT_STRICT_BINDING	2

#define CHAN_SWITCH_MODE_NOISY	0
#define CHAN_SWITCH_MODE_QUIET	1

int hostapd_check_power_cap(struct hostapd_data *hapd, u8 *power, u8 len);

#endif /* IEEE802_11H_H */

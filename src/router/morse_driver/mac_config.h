#ifndef _MORSE_MAC_CFG_H_
#define _MORSE_MAC_CFG_H_

/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

/*
 * Enable/Disabe various WiFi Config options
 * To enable a configuration, replace the #undef with #define
 *
 * Note: As the config code is not finalized, enabling some of the configs may
 * break the code. So use with care.
 */

#undef MORSE_MAC_CONFIG_HT_CAP
#undef MORSE_MAC_CONFIG_IEEE80211_HW
#undef MORSE_MAC_CONFIX_RX_STATUS_SIG

#endif /* !_MORSE_MAC_CFG_H_ */

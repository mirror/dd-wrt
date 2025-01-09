/*
 * ledconfig.c
 *
 * Copyright (C) 2018 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#include <bcmnvram.h>
#include <utils.h>

#define GPIO_VOID 0xffff
#define GPIO_LOW 0x1000
#define GPIO_IN 0x2000 // even if gpio is written, it must be set to input

#define GPIO_MASK 0xfff
static void getledconfig(struct ledconfig *cfg)
{
	cfg->power_gpio = GPIO_VOID;
	cfg->beeper_gpio = GPIO_VOID;
	cfg->diag_gpio = GPIO_VOID;
	cfg->diag_gpio_disabled = GPIO_VOID;
	cfg->dmz_gpio = GPIO_VOID;
	cfg->connected_gpio = GPIO_VOID;
	cfg->disconnected_gpio = GPIO_VOID;
	cfg->bridge_gpio = GPIO_VOID;
	cfg->vpn_gpio = GPIO_VOID;
	cfg->ses_gpio = GPIO_VOID; // use for SES1 (Linksys), AOSS (Buffalo)
	cfg->ses2_gpio = GPIO_VOID;
	cfg->wlan_gpio = GPIO_VOID; // wlan button led R7000
	cfg->wlan0_gpio = GPIO_VOID; // use this only if wlan led is not controlled by hardware!
	cfg->wlan1_gpio = GPIO_VOID;
	cfg->wlan2_gpio = GPIO_VOID;
	cfg->usb_gpio = GPIO_VOID;
	cfg->usb_gpio1 = GPIO_VOID;
	cfg->sec_gpio = GPIO_VOID; // generic
	cfg->sec0_gpio = GPIO_VOID; // security leds, wrt600n
	cfg->sec1_gpio = GPIO_VOID;
	cfg->usb_power = GPIO_VOID;
	cfg->usb_power1 = GPIO_VOID;
	cfg->poe_gpio = GPIO_VOID;
	cfg->v1func = 0;
	cfg->connblue = nvram_matchi("connblue", 1) ? 1 : 0;

	switch (getRouterBrand()) // gpio definitions here: 0xYZ,
	// Y=0:normal, Y=1:inverted, Z:gpio
	// number (f=disabled)
	{
#ifndef HAVE_BUFFALO
	case ROUTER_BOARD_TECHNAXX3G:
		cfg->usb_gpio = GPIO_LOW | 0x9;
		cfg->diag_gpio = GPIO_LOW | 0xc;
		cfg->connected_gpio = GPIO_LOW | 0xb;
		cfg->ses_gpio = GPIO_LOW | 0xc;
		break;
#ifdef HAVE_WPE72
	case ROUTER_BOARD_NS5M:
		cfg->diag_gpio = GPIO_LOW | 0xd;
		break;
#else
	case ROUTER_BOARD_NS5M:
		cfg->poe_gpio = 0x8;
		break;
#endif
	case ROUTER_BOARD_UNIFI:
		cfg->ses_gpio = 0x1;
		cfg->sec0_gpio = 0x1;
		break;
	case ROUTER_BOARD_UNIFI_V2:
		cfg->connected_gpio = 0xd;
		break;
	case ROUTER_UBNT_NANOAC:
		cfg->poe_gpio = 0x3;
		break;
	case ROUTER_BOARD_NS2M:
		cfg->poe_gpio = 0x8;
		break;
	case ROUTER_BOARD_NS5MXW:
		cfg->poe_gpio = 0x2;
		break;
	case ROUTER_UBNT_UAPAC:
	case ROUTER_UBNT_UAPACPRO:
		cfg->ses_gpio = 0x7;
		cfg->sec0_gpio = 0x7;
		break;
	case ROUTER_BOARD_AIRROUTER:
		cfg->power_gpio = GPIO_LOW | 0xb;
		cfg->diag_gpio = 0xb;
		cfg->connected_gpio = GPIO_LOW | 0x0;
		break;
	case ROUTER_BOARD_DANUBE:
#ifdef HAVE_WMBR_G300NH
		cfg->diag_gpio = GPIO_LOW | 0x5;
		cfg->ses_gpio = GPIO_LOW | 0xe;
		cfg->sec0_gpio = GPIO_LOW | 0xe;
		cfg->connected_gpio = GPIO_LOW | 0x11;
		cfg->disconnected_gpio = GPIO_LOW | 0x12;
		cfg->power_gpio = GPIO_LOW | 0x1;
#endif
#ifdef HAVE_SX763
		//              cfg->diag_gpio = GPIO_LOW | 0x5;
		//              cfg->ses_gpio = GPIO_LOW | 0xe;
		//              cfg->sec0_gpio = GPIO_LOW | 0xe;
		cfg->connected_gpio = GPIO_LOW | 0xde;
//              cfg->disconnected_gpio = GPIO_LOW | 0x12;
//              cfg->power_gpio = GPIO_LOW | 0x1;
#endif
		break;
#ifdef HAVE_UNIWIP
	case ROUTER_BOARD_UNIWIP:
		break;
#endif
#ifdef HAVE_WDR4900
	case ROUTER_BOARD_WDR4900:
		cfg->diag_gpio = 0x0;
		cfg->usb_gpio = 0x1;
		cfg->usb_gpio1 = 0x2;
		cfg->usb_power = GPIO_LOW | 0x3;
		break;
#endif
#ifdef HAVE_WRT1900AC
	case ROUTER_WRT_1200AC:
	case ROUTER_WRT_1900ACS:

	case ROUTER_WRT_1900ACV2:
		cfg->usb_power = 0x32;
	case ROUTER_WRT_1900AC:
		cfg->power_gpio = 0x0;
		cfg->diag_gpio = GPIO_LOW | 0x0;
		cfg->connected_gpio = 0x6;
		cfg->disconnected_gpio = 0x7;
		//              cfg->usb_gpio = 0x4;
		//              cfg->usb_gpio1 = 0x5;
		cfg->ses_gpio = 0x9;
		break;
	case ROUTER_WRT_3200ACM:
	case ROUTER_WRT_32X:
		cfg->usb_power = 0x2f;
		cfg->usb_power1 = 0x2c;
		cfg->power_gpio = 0x0;
		cfg->diag_gpio = GPIO_LOW | 0x0;
		cfg->connected_gpio = 0x6;
		cfg->disconnected_gpio = 0x7;
		//              cfg->usb_gpio = 0x4;
		//              cfg->usb_gpio1 = 0x5;
		cfg->ses_gpio = 0x9;
		break;
#endif
	case ROUTER_BOARD_PB42:
#ifdef HAVE_WA901
		cfg->diag_gpio = GPIO_LOW | 0x2;
		cfg->ses_gpio = 0x4;
//              cfg->usb_gpio = GPIO_LOW | 0x1;
#elif HAVE_WR941
		cfg->diag_gpio = GPIO_LOW | 0x2;
		cfg->ses_gpio = 0x5;
//              cfg->usb_gpio = GPIO_LOW | 0x1;
#endif
#ifdef HAVE_MR3020
		cfg->connected_gpio = GPIO_LOW | 0x1b;
		cfg->diag_gpio = GPIO_LOW | 0x1a;
		cfg->usb_power = 0x8;
#elif HAVE_GL150
//              cfg->power_gpio = GPIO_LOW | 0x1b;
//              cfg->diag_gpio = 0x1b;
//              cfg->usb_power = 0x8;
#elif HAVE_WR710
		cfg->power_gpio = GPIO_LOW | 0x1b;
		cfg->diag_gpio = 0x1b;
#elif HAVE_WA701V2
		cfg->diag_gpio = GPIO_LOW | 0x1b;
		cfg->ses_gpio = 0x1;
		cfg->sec0_gpio = 0x1;
#elif HAVE_WR703
		cfg->diag_gpio = GPIO_LOW | 0x1b;
		cfg->ses_gpio = 0x1;
		cfg->sec0_gpio = 0x1;
		cfg->usb_power = 0x8;
#elif HAVE_WR842
		cfg->diag_gpio = GPIO_LOW | 0x1;
		cfg->ses_gpio = 0x0;
		cfg->usb_power = 0x6;

#elif HAVE_WR741V4
		cfg->diag_gpio = GPIO_LOW | 0x1b;
		cfg->ses_gpio = 0x1;
		cfg->sec0_gpio = 0x1;

#elif HAVE_MR3420
		cfg->diag_gpio = GPIO_LOW | 0x1;
		cfg->connected_gpio = GPIO_LOW | 0x8;
		cfg->usb_power = 0x6;
#elif HAVE_WR741
		cfg->diag_gpio = GPIO_LOW | 0x1;
		cfg->ses_gpio = 0x0;
//              cfg->usb_gpio = GPIO_LOW | 0x1;
#endif
#ifdef HAVE_WR1043
		cfg->diag_gpio = GPIO_LOW | 0x2;
		cfg->ses_gpio = 0x5;
//              cfg->usb_gpio = GPIO_LOW | 0x1;
#endif
#ifdef HAVE_WRT160NL
		cfg->power_gpio = GPIO_LOW | 0xe;
		cfg->connected_gpio = GPIO_LOW | 0x9;
		cfg->ses_gpio = GPIO_LOW | 0x8;
#endif
#ifdef HAVE_TG2521
		cfg->ses_gpio = GPIO_LOW | 0x3;
		cfg->diag_gpio = GPIO_LOW | 0x3;
		cfg->usb_power = GPIO_LOW | 0x5;
#endif
#ifdef HAVE_TEW632BRP
		cfg->diag_gpio = GPIO_LOW | 0x1;
		cfg->ses_gpio = GPIO_LOW | 0x3;
#endif
#ifdef HAVE_WP543
		cfg->diag_gpio = GPIO_LOW | 0x7;
		cfg->connected_gpio = GPIO_LOW | 0x6;
#endif
#ifdef HAVE_WP546
		cfg->beeper_gpio = 0x1;
		cfg->diag_gpio = GPIO_LOW | 0x7;
		cfg->connected_gpio = GPIO_LOW | 0x6;
#endif
#ifdef HAVE_DIR825
		cfg->power_gpio = GPIO_LOW | 0x2;
		cfg->diag_gpio = GPIO_LOW | 0x1;
		cfg->connected_gpio = GPIO_LOW | 0xb;
		cfg->disconnected_gpio = GPIO_LOW | 0x6;
		cfg->ses_gpio = GPIO_LOW | 0x4;
		cfg->usb_gpio = GPIO_LOW | 0x0;
//              cfg->wlan0_gpio = 0xff; //correct states missing
#endif
#ifdef HAVE_WNDR3700
		cfg->power_gpio = GPIO_LOW | 0x2;
		cfg->diag_gpio = GPIO_LOW | 0x1;
		cfg->connected_gpio = GPIO_LOW | 0x6;
		cfg->ses_gpio = GPIO_LOW | 0x4;
#endif
#ifdef HAVE_WZRG300NH
		cfg->diag_gpio = GPIO_LOW | 0x1;
		cfg->connected_gpio = GPIO_LOW | 0x12;
		cfg->ses_gpio = GPIO_LOW | 0x11;
		cfg->sec0_gpio = GPIO_LOW | 0x11;
#endif
#ifdef HAVE_DIR632
		cfg->power_gpio = 0x1;
		cfg->diag_gpio = GPIO_LOW | 0x0;
		cfg->connected_gpio = GPIO_LOW | 0x11;
		cfg->usb_gpio = GPIO_LOW | 0xb;
#endif
#ifdef HAVE_WZRG450
		cfg->diag_gpio = GPIO_LOW | 0xe;
		cfg->ses_gpio = GPIO_LOW | 0xd;
		cfg->sec0_gpio = GPIO_LOW | 0xd;
		cfg->usb_power = 0x10;
		cfg->connected_gpio = GPIO_LOW | 0x2e; // card 1, gpio 14
#endif
#ifdef HAVE_WZRG300NH2
		cfg->diag_gpio = GPIO_LOW | 0x10;
		cfg->ses_gpio = GPIO_LOW | 0x26; // card 1, gpio 6
		cfg->sec0_gpio = GPIO_LOW | 0x26;
		cfg->usb_power = 0xd;
		cfg->connected_gpio = GPIO_LOW | 0x27; // card 1, gpio 7
#endif
#ifdef HAVE_WZRHPAG300NH
		cfg->diag_gpio = GPIO_LOW | 0x1;
		cfg->connected_gpio = GPIO_LOW | 0x33; // card 2 gpio 3
		cfg->sec0_gpio = GPIO_LOW | 0x25;
		cfg->sec1_gpio = GPIO_LOW | 0x31;
		cfg->ses_gpio = GPIO_LOW | 0x25; // card 1 gpio 5
		cfg->ses2_gpio = GPIO_LOW | 0x31; // card 2 gpio 5
		cfg->usb_power = 0x2;
#endif
#ifdef HAVE_DIR615C1
		cfg->power_gpio = GPIO_LOW | 0x4;
		cfg->wlan0_gpio = GPIO_LOW | 0xf;
#endif
#ifdef HAVE_DIR615E
		cfg->power_gpio = 0x6;
		cfg->diag_gpio = 0x1;
		cfg->connected_gpio = GPIO_LOW | 0x11;
		cfg->disconnected_gpio = 0x7;
		cfg->ses_gpio = GPIO_LOW | 0x0;
#endif
#ifdef HAVE_DAP2230
		cfg->diag_gpio = 0xb;
		cfg->power_gpio = GPIO_LOW | 0xb;
#elif HAVE_WR940V6
		cfg->diag_gpio = 0xf;
#elif HAVE_WR940V4
		cfg->disconnected_gpio = 0xf;
		cfg->power_gpio = GPIO_LOW | 0x5;
		cfg->diag_gpio = 0x5;
#elif HAVE_WR941V6
		cfg->disconnected_gpio = 0xf;
		cfg->power_gpio = GPIO_LOW | 0x12;
		cfg->diag_gpio = 0x12;

#elif HAVE_WR841HPV3
		cfg->power_gpio = GPIO_LOW | 0x10;
		cfg->diag_gpio = 0x10;
		cfg->ses_gpio = GPIO_LOW | 0x4;
		cfg->sec0_gpio = GPIO_LOW | 0x4;
		cfg->connected_gpio = GPIO_LOW | 0xc;
		cfg->disconnected_gpio = GPIO_LOW | 0xb;
#elif HAVE_WR841V12
		cfg->power_gpio = GPIO_LOW | 0x1;
		cfg->diag_gpio = 0x1;
		cfg->ses_gpio = GPIO_LOW | 0x3;
		cfg->sec0_gpio = GPIO_LOW | 0x3;
		cfg->connected_gpio = GPIO_LOW | 0x2;
#elif HAVE_WR841V11
		cfg->power_gpio = GPIO_LOW | 0x1;
		cfg->diag_gpio = 0x1;
		cfg->ses_gpio = GPIO_LOW | 0x3;
		cfg->sec0_gpio = GPIO_LOW | 0x3;
		cfg->connected_gpio = GPIO_LOW | 0x2;
#elif HAVE_ARCHERC25
		cfg->power_gpio = GPIO_LOW | 0x11;
		cfg->diag_gpio = 0x11;
		cfg->ses_gpio = GPIO_LOW | 0x2;
		cfg->sec0_gpio = GPIO_LOW | 0x2;
		cfg->connected_gpio = 0x7d;
		cfg->disconnected_gpio = 0x7c;
#elif HAVE_WR841V9
		cfg->diag_gpio = GPIO_LOW | 0x3;
#elif HAVE_WR842V2
		cfg->connected_gpio = GPIO_LOW | 0xe;
		cfg->usb_power = GPIO_IN | 0x4;
		cfg->usb_gpio = GPIO_LOW | 0xf;
#elif HAVE_WR810N
		cfg->diag_gpio = GPIO_LOW | 0xd;
		cfg->usb_power = 0xb;
#elif HAVE_WA860RE
		cfg->diag_gpio_disabled = GPIO_LOW | 0xf;
		cfg->diag_gpio = GPIO_LOW | 0xc;
		cfg->sec_gpio = 0;
#elif HAVE_WA850RE
		cfg->diag_gpio = GPIO_LOW | 0xf;
#elif HAVE_WR841V8
		cfg->diag_gpio = GPIO_LOW | 0xf;
		cfg->connected_gpio = GPIO_LOW | 0xe;
#elif HAVE_DIR615I
		cfg->power_gpio = 0xe;
		cfg->diag_gpio = GPIO_LOW | 0xf;
		cfg->connected_gpio = GPIO_LOW | 0xc;
		cfg->disconnected_gpio = 0x16;
#endif
#ifdef HAVE_WRT400
		cfg->power_gpio = 0x1;
		cfg->diag_gpio = GPIO_LOW | 0x5;
		cfg->ses_gpio = GPIO_LOW | 0x4;
		cfg->connected_gpio = 0x7;
#endif
#ifdef HAVE_ALFAAP94
		cfg->power_gpio = 0x5;
#endif
		break;
	case ROUTER_ALLNET01:
		cfg->connected_gpio = GPIO_LOW | 0x0;
		break;
	case ROUTER_BOARD_WP54G:
		cfg->diag_gpio = GPIO_LOW | 0x2;
		cfg->connected_gpio = GPIO_LOW | 0x7;
		break;
	case ROUTER_BOARD_NP28G:
		cfg->diag_gpio = GPIO_LOW | 0x2;
		cfg->connected_gpio = GPIO_LOW | 0x6;
		break;
	case ROUTER_BOARD_GATEWORX_GW2369:
		cfg->connected_gpio = GPIO_LOW | 0x2;
		break;
	case ROUTER_BOARD_GW2388:
	case ROUTER_BOARD_GW6400:
	case ROUTER_BOARD_GW2380:
#ifdef HAVE_NEWPORT

#elif defined(HAVE_VENTANA)
		cfg->power_gpio = GPIO_LOW | 0x66;
		cfg->diag_gpio = 0x6F;
		cfg->connected_gpio = 0x66;
		cfg->disconnected_gpio = 0x67;
#else
		cfg->connected_gpio = GPIO_LOW | 0x10; // 16 is mapped to front led
#endif
		break;
	case ROUTER_BOARD_GATEWORX:
#ifdef HAVE_WG302V1
		cfg->diag_gpio = GPIO_LOW | 0x4;
		cfg->wlan0_gpio = GPIO_LOW | 0x5;
#elif HAVE_WG302
		cfg->diag_gpio = GPIO_LOW | 0x2;
		cfg->wlan0_gpio = GPIO_LOW | 0x4;
#else
		if (nvram_match("DD_BOARD", "Gateworks Cambria GW2350"))
			cfg->connected_gpio = GPIO_LOW | 0x5;
		else if (nvram_match("DD_BOARD", "Gateworks Cambria GW2358-4"))
			cfg->connected_gpio = GPIO_LOW | 0x18;
		else
			cfg->connected_gpio = 0x3;
#endif
		break;
	case ROUTER_BOARD_GATEWORX_SWAP:
		cfg->connected_gpio = 0x4;
		break;
	case ROUTER_BOARD_STORM:
		cfg->connected_gpio = 0x5;
		cfg->diag_gpio = 0x3;
		break;
	case ROUTER_LINKSYS_WRH54G:
		cfg->diag_gpio = GPIO_LOW | 0x1; // power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_WRT54G:
	case ROUTER_WRT54G_V8:
		cfg->power_gpio = 0x1;
		cfg->dmz_gpio = GPIO_LOW | 0x7;
		cfg->connected_gpio = GPIO_LOW | 0x3; // ses orange
		cfg->ses_gpio = GPIO_LOW | 0x2; // ses white
		cfg->ses2_gpio = GPIO_LOW | 0x3; // ses orange
		break;
	case ROUTER_WRT54G_V81:
		cfg->power_gpio = GPIO_LOW | 0x1;
		cfg->dmz_gpio = GPIO_LOW | 0x2;
		cfg->connected_gpio = GPIO_LOW | 0x4; // ses orange
		cfg->ses_gpio = GPIO_LOW | 0x3; // ses white
		cfg->ses2_gpio = GPIO_LOW | 0x4; // ses orange
		break;
	case ROUTER_WRT54G1X:
		cfg->connected_gpio = GPIO_LOW | 0x3;
		cfg->v1func = 1;
		break;
	case ROUTER_WRT350N:
		cfg->connected_gpio = GPIO_LOW | 0x3;
		cfg->power_gpio = 0x1;
		cfg->ses2_gpio = GPIO_LOW | 0x3; // ses orange
		cfg->sec0_gpio = GPIO_LOW | 0x9;
		cfg->usb_gpio = GPIO_LOW | 0xb;
		break;
	case ROUTER_WRT600N:
		cfg->power_gpio = GPIO_LOW | 0x2;
		cfg->diag_gpio = 0x2;
		cfg->usb_gpio = GPIO_LOW | 0x3;
		cfg->sec0_gpio = GPIO_LOW | 0x9;
		cfg->sec1_gpio = GPIO_LOW | 0xb;
		break;
	case ROUTER_LINKSYS_WRT55AG:
		cfg->connected_gpio = GPIO_LOW | 0x3;
		break;
	case ROUTER_DLINK_DIR330:
		cfg->diag_gpio = GPIO_LOW | 0x6;
		cfg->connected_gpio = GPIO_LOW | 0x0;
		cfg->usb_gpio = GPIO_LOW | 0x4;
		break;
	case ROUTER_ASUS_RTN10PLUS:
		//              cfg->diag_gpio = GPIO_LOW | 0xd;
		//              cfg->connected_gpio = GPIO_LOW | 0x8;
		//              cfg->power_gpio = GPIO_LOW | 0x9;
		break;
	case ROUTER_BOARD_DIR600B:
		cfg->diag_gpio = GPIO_LOW | 0xd;
		cfg->connected_gpio = GPIO_LOW | 0x8;
		cfg->power_gpio = GPIO_LOW | 0x9;
		break;
	case ROUTER_BOARD_DIR615D:
#ifdef HAVE_DIR615H
		cfg->diag_gpio = 0x7;
		cfg->connected_gpio = GPIO_LOW | 0xd;
		cfg->disconnected_gpio = GPIO_LOW | 0xc;
		cfg->ses_gpio = GPIO_LOW | 0xe;
		cfg->power_gpio = 0x9;
#else
		cfg->diag_gpio = GPIO_LOW | 0x8;
		cfg->connected_gpio = GPIO_LOW | 0xc;
		cfg->disconnected_gpio = GPIO_LOW | 0xe;
		cfg->ses_gpio = GPIO_LOW | 0xb;
		cfg->power_gpio = GPIO_LOW | 0x9;
#endif
		break;
		/*
		   DIR 882 
		   power LED red diag = 8 inv, green 16 inv

		 */
	case ROUTER_BOARD_W502U:
		cfg->connected_gpio = GPIO_LOW | 0xd;
		break;
	case ROUTER_BOARD_OPENRISC:
#ifndef HAVE_ERC
		// ERC: diag button is used different / wlan button is handled by a script
		cfg->diag_gpio = 0x3;
		cfg->ses_gpio = 0x5;
#endif
		break;
	case ROUTER_BOARD_WR5422:
		cfg->ses_gpio = GPIO_LOW | 0xd;
		break;
	case ROUTER_BOARD_F5D8235:
		cfg->usb_gpio = GPIO_LOW | 0x17;
		cfg->diag_gpio = GPIO_LOW | 0x9;
		cfg->disconnected_gpio = GPIO_LOW | 0x6;
		cfg->connected_gpio = GPIO_LOW | 0x5;
		cfg->ses_gpio = GPIO_LOW | 0xc;
		break;
#else
	case ROUTER_BOARD_DANUBE:
#ifdef HAVE_WMBR_G300NH
		cfg->diag_gpio = GPIO_LOW | 0x5;
		cfg->ses_gpio = GPIO_LOW | 0xe;
		cfg->sec0_gpio = GPIO_LOW | 0xe;
		cfg->connected_gpio = GPIO_LOW | 0x11;
		cfg->disconnected_gpio = GPIO_LOW | 0x12;
		cfg->power_gpio = GPIO_LOW | 0x1;
#endif
		break;
	case ROUTER_BOARD_PB42:
#ifdef HAVE_WZRG300NH
		cfg->diag_gpio = GPIO_LOW | 0x1;
		cfg->connected_gpio = GPIO_LOW | 0x12;
		cfg->ses_gpio = GPIO_LOW | 0x11;
		cfg->sec0_gpio = GPIO_LOW | 0x11;
#endif
#ifdef HAVE_WZRHPAG300NH
		cfg->diag_gpio = GPIO_LOW | 0x1;
		cfg->connected_gpio = GPIO_LOW | 0x33;
		cfg->ses_gpio = GPIO_LOW | 0x25;
		cfg->ses2_gpio = GPIO_LOW | 0x31;
		cfg->sec0_gpio = GPIO_LOW | 0x25;
		cfg->sec1_gpio = GPIO_LOW | 0x31;
		cfg->usb_power = 0x2;
#endif
#ifdef HAVE_WZRG450
		cfg->diag_gpio = GPIO_LOW | 0xe;
		cfg->ses_gpio = GPIO_LOW | 0xd;
		cfg->sec0_gpio = GPIO_LOW | 0xd;
		cfg->usb_power = 0x10;
		cfg->connected_gpio = GPIO_LOW | 0x2e; // card 1, gpio 14
#endif
#ifdef HAVE_WZRG300NH2
		cfg->diag_gpio = GPIO_LOW | 0x10;
		cfg->ses_gpio = GPIO_LOW | 0x26;
		cfg->sec0_gpio = GPIO_LOW | 0x26;
		cfg->usb_power = 0xd;
		cfg->connected_gpio = GPIO_LOW | 0x27;
#endif
		break;
#endif
	case ROUTER_BOARD_HAMEA15:
		cfg->diag_gpio = GPIO_LOW | 0x11;
		cfg->connected_gpio = GPIO_LOW | 0x14;
		//              cfg->ses_gpio = GPIO_LOW | 0xe;
		break;
	case ROUTER_BOARD_WCRGN:
		cfg->diag_gpio = GPIO_LOW | 0x7;
		cfg->connected_gpio = GPIO_LOW | 0xb;
		//              cfg->ses_gpio = GPIO_LOW | 0xe;
		break;
	case ROUTER_R6800:
		cfg->diag_gpio = GPIO_LOW | 0x8;
		cfg->power_gpio = GPIO_LOW | 0x1f1;
		cfg->diag_gpio = GPIO_LOW | 0x1f0;
		cfg->diag_gpio_disabled = GPIO_LOW | 0x1f1;
		cfg->usb_gpio = GPIO_LOW | 0x1f6;
		cfg->usb_gpio1 = GPIO_LOW | 0x1f7;
		cfg->sec_gpio = 0x11;
		cfg->wlan_gpio = GPIO_LOW | 0x5;
		break;
	case ROUTER_R6850:
		cfg->power_gpio = GPIO_LOW | 0x12;
		cfg->usb_gpio = GPIO_LOW | 0xf;
		cfg->diag_gpio = 0x12;
		cfg->diag_gpio_disabled = GPIO_LOW | 0x12;
		cfg->connected_gpio = GPIO_LOW | 0xd;
		break;
	case ROUTER_R6220:
		cfg->power_gpio = GPIO_LOW | 0x12;
		cfg->usb_gpio = GPIO_LOW | 0xf;
		cfg->diag_gpio = 0x12;
		cfg->diag_gpio_disabled = GPIO_LOW | 0x12;
		cfg->connected_gpio = GPIO_LOW | 0xd;
		cfg->sec_gpio = GPIO_LOW | 0xc;
		cfg->usb_power = 0xa;
		break;
	case ROUTER_DIR882:
		cfg->connected_gpio = GPIO_LOW | 0x3;
		cfg->disconnected_gpio = GPIO_LOW | 0x4;
		cfg->diag_gpio = GPIO_LOW | 0x8;
		cfg->power_gpio = GPIO_LOW | 0x10;
		cfg->usb_gpio = GPIO_LOW | 0xc;
		cfg->usb_gpio1 = GPIO_LOW | 0xe;
		break;
	case ROUTER_DIR860LB1:
		cfg->power_gpio = GPIO_LOW | 0xf;
		cfg->diag_gpio = GPIO_LOW | 0xd;
		cfg->diag_gpio_disabled = GPIO_LOW | 0xf;
		cfg->disconnected_gpio = GPIO_LOW | 0xe;
		cfg->connected_gpio = GPIO_LOW | 0x10;
		break;
	case ROUTER_DIR810L:
		cfg->power_gpio = 0x9;
		cfg->diag_gpio = 0xd;
		cfg->diag_gpio_disabled = 0x9;
		cfg->connected_gpio = GPIO_LOW | 0x28;
		cfg->disconnected_gpio = 0xc;
		break;
	case ROUTER_WHR300HP2:
		cfg->power_gpio = GPIO_LOW | 0x9;
		cfg->diag_gpio = GPIO_LOW | 0x7;
		cfg->diag_gpio_disabled = GPIO_LOW | 0x9;
		cfg->wlan0_gpio = GPIO_LOW | 0x8;
		cfg->sec0_gpio = GPIO_LOW | 0xa;
		cfg->ses_gpio = GPIO_LOW | 0xa;
		cfg->connected_gpio = GPIO_LOW | 0x39;
		cfg->disconnected_gpio = GPIO_LOW | 0x3b;
		break;
	case ROUTER_BOARD_WHRG300N:
		cfg->diag_gpio = GPIO_LOW | 0x7;
		cfg->connected_gpio = GPIO_LOW | 0x9;
		cfg->ses_gpio = GPIO_LOW | 0xe;
		break;
#ifdef HAVE_WNR2200
	case ROUTER_BOARD_WHRHPGN:
		cfg->power_gpio = GPIO_LOW | 0x22;
		cfg->diag_gpio = GPIO_LOW | 0x21;
		cfg->connected_gpio = GPIO_LOW | 0x7;
		cfg->usb_power = 0x24; // enable usb port
		cfg->ses_gpio = GPIO_LOW | 0x5; //correct state missing
		cfg->usb_gpio = GPIO_LOW | 0x8;
		//              cfg->sec0_gpio = GPIO_LOW | 0x4;
		break;
#elif HAVE_DW02_412H
	case ROUTER_BOARD_WHRHPGN:
		//              cfg->disconnected_gpio = 22;
		cfg->connected_gpio = GPIO_LOW | 0x16;
		//		cfg->wlan_gpio = GPIO_LOW | 0x0d;
		cfg->wlan0_gpio = GPIO_LOW | 0x0d;
		cfg->wlan1_gpio = GPIO_LOW | 0x0d;
		break;
#elif HAVE_PERU
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0xc;
		cfg->beeper_gpio = 0x4;
		break;
#elif HAVE_LIMA
	case ROUTER_BOARD_WHRHPGN:
		//              cfg->disconnected_gpio = 0xf;
		//              cfg->power_gpio = GPIO_LOW | 0x5;
		//              cfg->diag_gpio = 0x5;
		break;
#elif HAVE_RAMBUTAN
	case ROUTER_BOARD_WHRHPGN:
		//              cfg->disconnected_gpio = 0xf;
		//              cfg->power_gpio = GPIO_LOW | 0x5;
		//              cfg->diag_gpio = 0x5;
		break;
#elif HAVE_WNR2000
	case ROUTER_BOARD_WHRHPGN:
		cfg->power_gpio = GPIO_LOW | 0x23;
		cfg->diag_gpio = GPIO_LOW | 0x22;
		cfg->connected_gpio = GPIO_LOW | 0x0;
		//              cfg->ses_gpio = GPIO_LOW | 0x4;
		//              cfg->sec0_gpio = GPIO_LOW | 0x4;
		break;
#elif HAVE_WLAEAG300N
	case ROUTER_BOARD_WHRHPGN:
		cfg->power_gpio = GPIO_LOW | 0x10;
		cfg->diag_gpio = GPIO_LOW | 0x11;
		cfg->connected_gpio = GPIO_LOW | 0x6;
		cfg->ses_gpio = GPIO_LOW | 0xe;
		cfg->sec0_gpio = GPIO_LOW | 0xe;
		break;
#elif HAVE_CARAMBOLA
#ifdef HAVE_ERC
	case ROUTER_BOARD_WHRHPGN:
		cfg->vpn_gpio = GPIO_LOW | 0x1B;
		cfg->wlan0_gpio = 0x0;
		break;
#elif HAVE_FMS2111
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x13;
		cfg->beeper_gpio = 0xc;
		break;
#else
	case ROUTER_BOARD_WHRHPGN:
		//              cfg->usb_power = 0x1a;
		//              cfg->usb_gpio = 0x1;
		//              cfg->ses_gpio = GPIO_LOW | 0x1b;
		break;
#endif
#elif HAVE_HORNET
	case ROUTER_BOARD_WHRHPGN:
		cfg->usb_power = 0x1a;
		cfg->usb_gpio = 0x1;
		cfg->ses_gpio = GPIO_LOW | 0x1b;
		break;
#elif HAVE_RB2011
	case ROUTER_BOARD_WHRHPGN:
		//              cfg->diag_gpio = GPIO_LOW | 0xf;
		//              cfg->connected_gpio = GPIO_LOW | 0x12;
		//              cfg->disconnected_gpio = GPIO_LOW | 0x13;
		//              cfg->power_gpio = GPIO_LOW | 0xe;
		//              cfg->usb_power = 0x1a;
		//              cfg->usb_gpio = GPIO_LOW | 0xb;
		//              cfg->ses_gpio = GPIO_LOW | 0x1b;
		break;
#elif HAVE_WDR3500
	case ROUTER_BOARD_WHRHPGN:
		cfg->usb_gpio = GPIO_LOW | 0xb;
		cfg->usb_power = 0xf;
		cfg->diag_gpio = GPIO_LOW | 0xe;
		cfg->connected_gpio = GPIO_LOW | 0xf;
		break;
#elif HAVE_WDR4300
	case ROUTER_BOARD_WHRHPGN:
		cfg->usb_gpio = GPIO_LOW | 0xb;
		cfg->usb_gpio1 = GPIO_LOW | 0xc;
		cfg->usb_power = 0x15;
		cfg->usb_power1 = 0x16;
		cfg->diag_gpio = GPIO_LOW | 0xe;
		cfg->connected_gpio = GPIO_LOW | 0xf;
		break;
#elif HAVE_WNDR3700V4
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = GPIO_LOW | 0x2;
		cfg->power_gpio = GPIO_LOW | 0x0;
		cfg->connected_gpio = GPIO_LOW | 0x1;
		cfg->disconnected_gpio = GPIO_LOW | 0x3;
		cfg->usb_power = 0x20;
		cfg->usb_gpio = GPIO_LOW | 0xd;
		cfg->ses_gpio = GPIO_LOW | 0x10;
		break;
#elif HAVE_DAP2680
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = GPIO_LOW | 0x14; // red
		cfg->diag_gpio_disabled = GPIO_LOW | 0x13; //
		cfg->power_gpio = GPIO_LOW | 0x13; // green
		break;
#elif HAVE_DAP3662
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = GPIO_LOW | 0xe; // red
		cfg->diag_gpio_disabled = GPIO_LOW | 0x17; //
		cfg->power_gpio = GPIO_LOW | 0x17; // green
		break;
#elif HAVE_DIR862
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = GPIO_LOW | 0xe; // orange
		cfg->diag_gpio_disabled = GPIO_LOW | 0x13; //
		cfg->power_gpio = GPIO_LOW | 0x13; // green
		cfg->connected_gpio = GPIO_LOW | 0x16; // green
		cfg->disconnected_gpio = GPIO_LOW | 0x17; // orange
		break;
#elif HAVE_XD9531
	case ROUTER_BOARD_WHRHPGN:
		cfg->connected_gpio = GPIO_LOW | 0x4;
		cfg->diag_gpio = GPIO_LOW | 0xD;
		break;
#elif HAVE_CPE880
	case ROUTER_BOARD_WHRHPGN:
		cfg->connected_gpio = GPIO_LOW | 0x12;
		break;
#elif HAVE_MMS344
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = GPIO_LOW | 0xe;
		break;
#elif HAVE_WR1043V5
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x6;
		cfg->ses_gpio = GPIO_LOW | 0x1;
		cfg->sec0_gpio = GPIO_LOW | 0x1;

		break;
#elif HAVE_WR1043V4
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x6;
		cfg->ses_gpio = GPIO_LOW | 0x1;
		cfg->sec0_gpio = GPIO_LOW | 0x1;
		cfg->usb_gpio = GPIO_LOW | 0x7;
		cfg->usb_power = 0x8;

		break;
#elif HAVE_ARCHERC7V5
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x6;
		cfg->connected_gpio = GPIO_LOW | 0x15;
		cfg->disconnected_gpio = GPIO_LOW | 0x14;
		cfg->ses_gpio = GPIO_LOW | 0x1;
		cfg->sec0_gpio = GPIO_LOW | 0x1;
		cfg->usb_power = 0x13;
		cfg->usb_gpio = GPIO_LOW | 0x7;

		break;
#elif HAVE_ARCHERC7V4
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x6;
		cfg->connected_gpio = GPIO_LOW | 0x1a;
		cfg->disconnected_gpio = GPIO_LOW | 0x19;
		cfg->ses_gpio = GPIO_LOW | 0x1f;
		cfg->sec0_gpio = GPIO_LOW | 0x1f;

		//              cfg->usb_power = 0x16;
		cfg->usb_gpio = GPIO_LOW | 0x7;

		//              cfg->usb_power1 = 0x15;
		cfg->usb_gpio1 = GPIO_LOW | 0x8;

		break;
#elif HAVE_ARCHERC7
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = GPIO_LOW | 0xe;
		cfg->ses_gpio = GPIO_LOW | 0xf;
		cfg->sec0_gpio = GPIO_LOW | 0xf;

		cfg->usb_power = 0x16;
		cfg->usb_gpio = GPIO_LOW | 0x12;

		cfg->usb_power1 = 0x15;
		cfg->usb_gpio1 = GPIO_LOW | 0x13;

		break;
#elif HAVE_WR1043V2
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = GPIO_LOW | 0x13;
		//              cfg->connected_gpio = GPIO_LOW | 0x12;
		//              cfg->disconnected_gpio = GPIO_LOW | 0x13;
		//              cfg->power_gpio = GPIO_LOW | 0xe;
		cfg->usb_power = 0x15;
		cfg->usb_gpio = GPIO_LOW | 0xf;
		cfg->ses_gpio = GPIO_LOW | 0x12;
		cfg->sec0_gpio = GPIO_LOW | 0x12;
		break;
#elif HAVE_WZR450HP2
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = GPIO_LOW | 0x14;
		//              cfg->connected_gpio = GPIO_LOW | 0x12;
		//              cfg->disconnected_gpio = GPIO_LOW | 0x13;
		//              cfg->power_gpio = GPIO_LOW | 0xe;
		//              cfg->usb_power = 0x1a;
		//              cfg->usb_gpio = GPIO_LOW | 0xb;

		cfg->connected_gpio = GPIO_LOW | 0xd;
		cfg->power_gpio = GPIO_LOW | 0x13;
		cfg->ses_gpio = GPIO_LOW | 0x3;
		cfg->sec0_gpio = GPIO_LOW | 0x3;
		break;
#elif HAVE_DHP1565
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = GPIO_LOW | 0xe;
		cfg->diag_gpio_disabled = GPIO_LOW | 0x16;
		cfg->connected_gpio = GPIO_LOW | 0x12;
		cfg->disconnected_gpio = GPIO_LOW | 0x13;
		cfg->power_gpio = GPIO_LOW | 0x16;
		cfg->usb_gpio = GPIO_LOW | 0xb;
		cfg->ses_gpio = GPIO_LOW | 0xf;
		break;
#elif HAVE_E325N
	case ROUTER_BOARD_WHRHPGN:
		cfg->connected_gpio = 0x3;
		cfg->disconnected_gpio = 0x2;
		break;
#elif defined(HAVE_SR3200) || defined(HAVE_CPE890)
	case ROUTER_BOARD_WHRHPGN:
		cfg->power_gpio = GPIO_LOW | 0x1;
		cfg->diag_gpio = 0x1;
		break;
#elif HAVE_XD3200
	case ROUTER_BOARD_WHRHPGN:
		break;
#elif HAVE_E380AC
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x3;
		break;
#elif HAVE_WR615N
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = GPIO_LOW | 0x1;
		cfg->connected_gpio = GPIO_LOW | 0x2;
		cfg->disconnected_gpio = GPIO_LOW | 0x3;
		cfg->ses_gpio = GPIO_LOW | 0xc;
		cfg->sec0_gpio = GPIO_LOW | 0xc;
		break;
#elif HAVE_E355AC
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x2;
		break;
#elif HAVE_WR650AC
	case ROUTER_BOARD_WHRHPGN:
		cfg->ses_gpio = GPIO_LOW | 0x14;
		cfg->sec0_gpio = GPIO_LOW | 0x14;
		cfg->connected_gpio = GPIO_LOW | 0x4;
		cfg->diag_gpio = 0x4;
		break;
#elif HAVE_DIR869
	case ROUTER_BOARD_WHRHPGN:
		cfg->disconnected_gpio = GPIO_LOW | 0xf;
		cfg->connected_gpio = GPIO_LOW | 0x10;
		cfg->diag_gpio = 0xf;
		break;
#elif HAVE_DIR859
	case ROUTER_BOARD_WHRHPGN:
		cfg->power_gpio = GPIO_LOW | 0xf;
		cfg->connected_gpio = GPIO_LOW | 0x10;
		cfg->diag_gpio = 0xf;
		break;
#elif HAVE_JWAP606
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = GPIO_LOW | 0xb;
		cfg->connected_gpio = GPIO_LOW | 0xd;
		cfg->disconnected_gpio = GPIO_LOW | 0xd;
		cfg->power_gpio = GPIO_LOW | 0xb;
		//              cfg->usb_power = 0x1a;
		//              cfg->usb_gpio = GPIO_LOW | 0xb;
		//              cfg->ses_gpio = GPIO_LOW | 0x1b;
		break;
#elif HAVE_DIR825C1
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = GPIO_LOW | 0xf;
		cfg->connected_gpio = GPIO_LOW | 0x12;
		cfg->disconnected_gpio = GPIO_LOW | 0x13;
		cfg->power_gpio = GPIO_LOW | 0xe;
		//              cfg->usb_power = 0x1a;
		cfg->usb_gpio = GPIO_LOW | 0xb;
		//              cfg->ses_gpio = GPIO_LOW | 0x1b;
		break;
#elif HAVE_WDR2543
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = GPIO_LOW | 0x0;
		cfg->usb_gpio = GPIO_LOW | 0x8;
		break;
#elif HAVE_WASP
	case ROUTER_BOARD_WHRHPGN:
		//              cfg->usb_power = 0x1a;
		//              cfg->usb_gpio = 0x1;
		//              cfg->ses_gpio = GPIO_LOW | 0x1b;
		break;
#else
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = GPIO_LOW | 0x1;
		cfg->connected_gpio = GPIO_LOW | 0x6;
		cfg->ses_gpio = GPIO_LOW | 0x0;
		cfg->sec0_gpio = GPIO_LOW | 0x0;
		break;
#endif
	case ROUTER_BUFFALO_WBR54G:
		cfg->diag_gpio = GPIO_LOW | 0x7;
		break;
	case ROUTER_BUFFALO_WBR2G54S:
		cfg->diag_gpio = 0x1;
		cfg->ses_gpio = 0x6;
		break;
	case ROUTER_BUFFALO_WLA2G54C:
		cfg->diag_gpio = GPIO_LOW | 0x4;
		cfg->ses_gpio = GPIO_LOW | 0x3;
		break;
	case ROUTER_BUFFALO_WLAH_G54:
		cfg->diag_gpio = GPIO_LOW | 0x7;
		cfg->ses_gpio = GPIO_LOW | 0x6;
		break;
	case ROUTER_BUFFALO_WAPM_HP_AM54G54:
		cfg->diag_gpio = GPIO_LOW | 0x7;
		cfg->ses_gpio = GPIO_LOW | 0x1;
		break;
	case ROUTER_BOARD_WHRAG108:
		cfg->diag_gpio = GPIO_LOW | 0x7;
		cfg->bridge_gpio = GPIO_LOW | 0x4;
		cfg->ses_gpio = GPIO_LOW | 0x0;
		break;
	case ROUTER_BUFFALO_WHRG54S:
	case ROUTER_BUFFALO_WLI_TX4_G54HP:
		cfg->diag_gpio = GPIO_LOW | 0x7;
		if (nvram_match("DD_BOARD", "Buffalo WHR-G125")) {
			cfg->connected_gpio = GPIO_LOW | 0x1;
			cfg->sec0_gpio = GPIO_LOW | 0x6;
		} else {
			cfg->bridge_gpio = GPIO_LOW | 0x1;
			cfg->ses_gpio = GPIO_LOW | 0x6;
		}
		break;
	case ROUTER_UBNT_UNIFIAC:
		cfg->power_gpio = 0xe;
		cfg->diag_gpio = 0xf;
		break;
	case ROUTER_D1800H:
		cfg->usb_gpio = GPIO_LOW | 0x1;
		cfg->usb_power = 0x7;
		cfg->power_gpio = 0x2;
		cfg->diag_gpio = 0xd;
		cfg->diag_gpio_disabled = 0x2;
		cfg->connected_gpio = GPIO_LOW | 0xf;
		cfg->disconnected_gpio = GPIO_LOW | 0xe;
		break;
	case ROUTER_BUFFALO_WZRRSG54:
		cfg->diag_gpio = GPIO_LOW | 0x7;
		cfg->vpn_gpio = GPIO_LOW | 0x1;
		cfg->ses_gpio = GPIO_LOW | 0x6;
		break;
	case ROUTER_BUFFALO_WZRG300N:
		cfg->diag_gpio = GPIO_LOW | 0x7;
		cfg->bridge_gpio = GPIO_LOW | 0x1;
		break;
	case ROUTER_BUFFALO_WZRG144NH:
		cfg->diag_gpio = GPIO_LOW | 0x3;
		cfg->bridge_gpio = GPIO_LOW | 0x1;
		cfg->ses_gpio = GPIO_LOW | 0x2;
		break;
	case ROUTER_BUFFALO_WZR900DHP:
	case ROUTER_BUFFALO_WZR600DHP2:
		//              cfg->usb_power = 0x9;      // USB 2.0 ehci port
		cfg->usb_power1 = GPIO_LOW | 0xa; // USB 3.0 xhci port
		//              cfg->wlan0_gpio = 0x28; // wireless orange
		//              cfg->wlan1_gpio = 0x29; // wireless blue
		cfg->connected_gpio = 0x2a; // connected blue
		cfg->sec0_gpio = 0x2b;
		cfg->sec1_gpio = 0x2c;
		// 0x2b strange led orange
		// 0x2c strange led blue
		cfg->power_gpio = 0x2e;
		cfg->diag_gpio = 0x2d;
		cfg->diag_gpio_disabled = 0x2e;
		cfg->usb_gpio = 0x2f;
		break;

	case ROUTER_BUFFALO_WXR1900DHP:
		cfg->usb_power = 0xd; // USB 2.0 ehci port
		cfg->usb_power1 = 0xe; // USB 3.0 xhci port
		//              cfg->wlan0_gpio = 0x28; // wireless orange
		//              cfg->wlan1_gpio = 0x29; // wireless blue
		cfg->connected_gpio = 0x9; // connected blue
		cfg->disconnected_gpio = 0xa; // connected blue
		cfg->sec0_gpio = 0xb;
		cfg->sec1_gpio = 0xb;
		// 0x2b strange led orange
		// 0x2c strange led blue
		cfg->power_gpio = 0x6;
		cfg->diag_gpio = 0x5;
		cfg->diag_gpio_disabled = 0x6;
		break;

	case ROUTER_BUFFALO_WZR1750:
		cfg->usb_power = 0x9; // USB 2.0 ehci port
		cfg->usb_power1 = GPIO_LOW | 0xa; // USB 3.0 xhci port
		//              cfg->wlan0_gpio = 0x28; // wireless orange
		//              cfg->wlan1_gpio = 0x29; // wireless blue
		cfg->connected_gpio = 0x2a; // connected blue
		cfg->sec0_gpio = 0x2b;
		cfg->sec1_gpio = 0x2c;
		// 0x2b strange led orange
		// 0x2c strange led blue
		cfg->power_gpio = 0x2d;
		cfg->diag_gpio = 0x2e;
		cfg->diag_gpio_disabled = 0x2d;
		cfg->usb_gpio = 0x2f;
		break;
#ifndef HAVE_BUFFALO
#ifdef HAVE_DIR300
	case ROUTER_BOARD_FONERA:
		cfg->diag_gpio = 0x3;
		cfg->bridge_gpio = 0x4;
		cfg->ses_gpio = 0x1;
		break;
#endif
#ifdef HAVE_WRT54G2
	case ROUTER_BOARD_FONERA:
		cfg->bridge_gpio = 0x4;
		cfg->ses_gpio = GPIO_LOW | 0x4;
		cfg->diag_gpio = GPIO_LOW | 0x3;
		break;
#endif
#ifdef HAVE_RTG32
	case ROUTER_BOARD_FONERA:
		break;
#endif
#ifdef HAVE_BWRG1000
	case ROUTER_BOARD_LS2:
		cfg->diag_gpio = 0x7;
		break;
#endif
#ifdef HAVE_DIR400
	case ROUTER_BOARD_FONERA2200:
		cfg->diag_gpio = 0x3;
		cfg->bridge_gpio = 0x4;
		cfg->ses_gpio = 0x1;
		break;
#endif
#ifdef HAVE_WRK54G
	case ROUTER_BOARD_FONERA:
		cfg->diag_gpio = GPIO_LOW | 0x7;
		cfg->dmz_gpio = 0x5;
		break;
#endif
	case ROUTER_BOARD_TW6600:
		cfg->diag_gpio = GPIO_LOW | 0x7;
		cfg->bridge_gpio = GPIO_LOW | 0x4;
		cfg->ses_gpio = GPIO_LOW | 0x0;
		break;
	case ROUTER_MOTOROLA:
		cfg->power_gpio = 0x1;
		cfg->diag_gpio = GPIO_LOW | 0x1; // power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_RT210W:
		cfg->power_gpio = GPIO_LOW | 0x5;
		cfg->diag_gpio = 0x5; // power led blink / off to indicate factory
		// defaults
		cfg->connected_gpio = GPIO_LOW | 0x0;
		cfg->wlan0_gpio = GPIO_LOW | 0x3;
		break;
	case ROUTER_RT480W:
	case ROUTER_BELKIN_F5D7230_V2000:
	case ROUTER_BELKIN_F5D7231:
		cfg->power_gpio = GPIO_LOW | 0x5;
		cfg->diag_gpio = 0x5; // power led blink / off to indicate factory
		// defaults
		cfg->connected_gpio = GPIO_LOW | 0x0;
		break;
	case ROUTER_MICROSOFT_MN700:
		cfg->power_gpio = 0x6;
		cfg->diag_gpio = GPIO_LOW | 0x6; // power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_ASUS_WL500GD:
	case ROUTER_ASUS_WL520GUGC:
		cfg->diag_gpio = 0x0; // power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_ASUS_WL500G_PRE:
	case ROUTER_ASUS_WL700GE:
		cfg->power_gpio = GPIO_LOW | 0x1;
		cfg->diag_gpio = 0x1; // power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_ASUS_WL550GE:
		cfg->power_gpio = GPIO_LOW | 0x2;
		cfg->diag_gpio = 0x2; // power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_WRT54G3G:
	case ROUTER_WRTSL54GS:
		cfg->power_gpio = 0x1;
		cfg->dmz_gpio = GPIO_LOW | 0x0;
		cfg->connected_gpio = GPIO_LOW | 0x7; // ses orange
		cfg->ses_gpio = GPIO_LOW | 0x5; // ses white
		cfg->ses2_gpio = GPIO_LOW | 0x7; // ses orange
		break;
	case ROUTER_MOTOROLA_WE800G:
	case ROUTER_MOTOROLA_V1:
		cfg->diag_gpio = GPIO_LOW | 0x3;
		cfg->wlan0_gpio = GPIO_LOW | 0x1;
		cfg->bridge_gpio = GPIO_LOW | 0x5;
		break;
	case ROUTER_DELL_TRUEMOBILE_2300:
	case ROUTER_DELL_TRUEMOBILE_2300_V2:
		cfg->power_gpio = GPIO_LOW | 0x7;
		cfg->diag_gpio = 0x7; // power led blink / off to indicate factory
		// defaults
		cfg->wlan0_gpio = GPIO_LOW | 0x6;
		break;
	case ROUTER_NETGEAR_WNR834B:
		cfg->power_gpio = GPIO_LOW | 0x4;
		cfg->diag_gpio = GPIO_LOW | 0x5;
		cfg->wlan0_gpio = GPIO_LOW | 0x6;
		break;
	case ROUTER_SITECOM_WL105B:
		cfg->power_gpio = 0x3;
		cfg->diag_gpio = GPIO_LOW | 0x3; // power led blink / off to indicate factory
		// defaults
		cfg->wlan0_gpio = GPIO_LOW | 0x4;
		break;
	case ROUTER_WRT300N:
		cfg->power_gpio = 0x1;
		cfg->diag_gpio = GPIO_LOW | 0x1; // power led blink / off to indicate fac.def.
		break;
	case ROUTER_WRT150N:
		cfg->power_gpio = 0x1;
		cfg->diag_gpio = GPIO_LOW | 0x1; // power led blink / off to indicate fac.def.
		cfg->sec0_gpio = GPIO_LOW | 0x5;
		break;
	case ROUTER_WRT300NV11:
		cfg->ses_gpio = GPIO_LOW | 0x5;
		cfg->sec0_gpio = GPIO_LOW | 0x3;
		// cfg->diag_gpio = GPIO_LOW | 0x1; //power led blink / off to indicate fac.def.
		break;
	case ROUTER_WRT310N:
		cfg->connected_gpio = GPIO_LOW | 0x3; //ses orange
		cfg->power_gpio = 0x1;
		cfg->diag_gpio = GPIO_LOW | 0x1; // power led blink / off to indicate fac.def.
		cfg->ses_gpio = GPIO_LOW | 0x9; // ses blue
		break;
	case ROUTER_WRT310NV2:
		cfg->connected_gpio = GPIO_LOW | 0x2; // ses orange
		cfg->power_gpio = 0x1;
		cfg->diag_gpio = GPIO_LOW | 0x1; // power led blink / off to indicate fac.def.
		cfg->ses_gpio = GPIO_LOW | 0x4; // ses blue
		break;
	case ROUTER_WRT160N:
		cfg->power_gpio = 0x1;
		cfg->diag_gpio = GPIO_LOW | 0x1; // power led blink / off to indicate fac.def.
		cfg->connected_gpio = GPIO_LOW | 0x3; // ses orange
		cfg->ses_gpio = GPIO_LOW | 0x5; // ses blue
		break;
	case ROUTER_WRT160NV3:
		cfg->power_gpio = 0x1;
		cfg->diag_gpio = GPIO_LOW | 0x1; // power led blink / off to indicate fac.def.
		cfg->connected_gpio = GPIO_LOW | 0x2; // ses orange
		cfg->ses_gpio = GPIO_LOW | 0x4; // ses blue
		break;
	case ROUTER_LINKSYS_E800:
	case ROUTER_LINKSYS_E900:
	case ROUTER_LINKSYS_E1500:
	case ROUTER_LINKSYS_E1550:
		cfg->power_gpio = GPIO_LOW | 0x6;
		cfg->diag_gpio = 0x6; // power led blink / off to indicate fac.def.
		cfg->ses_gpio = GPIO_LOW | 0x8; // ses blue
		break;
	case ROUTER_LINKSYS_E1000V2:
		cfg->power_gpio = GPIO_LOW | 0x6;
		cfg->diag_gpio = 0x6; // power led blink / off to indicate fac.def.
		cfg->connected_gpio = 0x7; // ses orange
		cfg->ses_gpio = 0x8; // ses blue
		break;
	case ROUTER_LINKSYS_E2500:
		cfg->power_gpio = GPIO_LOW | 0x6;
		cfg->diag_gpio = 0x6; // power led blink / off to indicate fac.def.
		break;
	case ROUTER_LINKSYS_E3200:
		cfg->power_gpio = GPIO_LOW | 0x3;
		cfg->diag_gpio = 0x3; // power led blink / off to indicate fac.def.
		break;
	case ROUTER_LINKSYS_E4200:
		cfg->power_gpio = GPIO_LOW | 0x5; // white LED1
		cfg->diag_gpio = GPIO_LOW | 0x3; // power led blink / off to indicate fac.def.
		//              cfg->connected_gpio = GPIO_LOW | 0x3; // white LED2
		break;
	case ROUTER_LINKSYS_EA6500:
		cfg->diag_gpio = GPIO_LOW | 0x1; // white led blink / off to indicate fac.def.
		break;
	case ROUTER_LINKSYS_EA6500V2:
	case ROUTER_LINKSYS_EA6700:
	case ROUTER_LINKSYS_EA6400:
	case ROUTER_LINKSYS_EA6350:
	case ROUTER_LINKSYS_EA6900:
		cfg->usb_power = 0x9; //usb power on/off
		cfg->usb_power1 = 0xa; //usb power on/off
		cfg->diag_gpio = GPIO_LOW | 0x6; // white led blink / off to indicate fac.def.
		cfg->connected_gpio = 0x8;
		break;
	case ROUTER_LINKSYS_EA8500:
		cfg->power_gpio = GPIO_LOW | 0x0; // power led
		cfg->diag_gpio = 0x0; // power led orange
		cfg->wlan0_gpio = 0x1; // radio 0
		cfg->ses_gpio = GPIO_LOW | 0x2; // wps led
		break;
	case ROUTER_ASUS_WL500G:
		cfg->power_gpio = GPIO_LOW | 0x0;
		cfg->diag_gpio = 0x0; // power led blink /off to indicate factory
		// defaults
		break;
	case ROUTER_ASUS_WL500W:
		cfg->power_gpio = GPIO_LOW | 0x5;
		cfg->diag_gpio = 0x5; // power led blink /off to indicate factory
		// defaults
		break;
	case ROUTER_LINKSYS_WTR54GS:
		cfg->diag_gpio = 0x1;
		break;
	case ROUTER_WAP54G_V1:
		cfg->diag_gpio = GPIO_LOW | 0x3;
		cfg->wlan0_gpio = GPIO_LOW | 0x4; // LINK led
		break;
	case ROUTER_WAP54G_V3:
		cfg->ses_gpio = GPIO_LOW | 0xc;
		cfg->connected_gpio = 0x6;
		break;
	case ROUTER_NETGEAR_WNR834BV2:
		cfg->power_gpio = 0x2;
		cfg->diag_gpio = 0x3; // power led amber
		cfg->connected_gpio = 0x7; // WAN led green
		break;
	case ROUTER_NETGEAR_WNDR3300:
		cfg->power_gpio = 0x5;
		cfg->diag_gpio = GPIO_LOW | 0x5; // power led blink /off to indicate factory defaults
		cfg->connected_gpio = 0x7; // WAN led green
		break;
	case ROUTER_ASKEY_RT220XD:
		cfg->wlan0_gpio = GPIO_LOW | 0x0;
		cfg->dmz_gpio = GPIO_LOW | 0x1; // not soldered
		break;
	case ROUTER_WRT610N:
		cfg->power_gpio = 0x1;
		cfg->diag_gpio = GPIO_LOW | 0x1; // power led blink /off to indicate factory defaults
		cfg->connected_gpio = GPIO_LOW | 0x3; // ses amber
		cfg->ses_gpio = GPIO_LOW | 0x9; // ses blue
		cfg->usb_gpio = GPIO_LOW | 0x0;
		break;
	case ROUTER_WRT610NV2:
		cfg->power_gpio = 0x5;
		cfg->diag_gpio = GPIO_LOW | 0x5; // power led blink
		cfg->connected_gpio = GPIO_LOW | 0x0; // ses amber
		cfg->ses_gpio = GPIO_LOW | 0x3; // ses blue
		cfg->usb_gpio = 0x7;
		break;
	case ROUTER_USR_5461:
		cfg->usb_gpio = 0x1;
		break;
	case ROUTER_USR_5465:
		//cfg->usb_gpio = 0x2; //or 0x1 ??
		break;
	case ROUTER_NETGEAR_WGR614L:
	case ROUTER_NETGEAR_WGR614V9:
		// cfg->power_gpio = GPIO_LOW | 0x7;       // don't use - resets router
		cfg->diag_gpio = 0x6;
		cfg->connected_gpio = GPIO_LOW | 0x4;
		break;
	case ROUTER_NETGEAR_WG602_V4:
		cfg->power_gpio = GPIO_LOW | 0x1; // trick: make lan led green for 100Mbps
		break;
	case ROUTER_BELKIN_F5D7231_V2000:
		cfg->connected_gpio = GPIO_LOW | 0x4;
		cfg->diag_gpio = 0x1; // power led blink /off to indicate factory defaults
		break;
	case ROUTER_NETGEAR_WNR3500L:
	case ROUTER_NETGEAR_WNR3500LV2:
		cfg->power_gpio = 0x3; // power led green
		cfg->diag_gpio = 0x7; // power led amber
		cfg->ses_gpio = 0x1; // WPS led green
		cfg->connected_gpio = 0x2; // wan led green
		cfg->wlan1_gpio = 0x0; // radio 1 blue led
		cfg->usb_gpio = 0x14; // usb power
		break;
	case ROUTER_NETGEAR_WNDR3400:
		cfg->power_gpio = 0x3; //power led green
		cfg->diag_gpio = 0x7; // power led amber
		cfg->connected_gpio = 0x1; //wan led green
		cfg->usb_gpio = GPIO_LOW | 0x2; //usb led green
		cfg->wlan1_gpio = 0x0; // radio 1 led blue
		break;
	case ROUTER_NETGEAR_WNDR4000:
	case ROUTER_NETGEAR_R6200:
		cfg->power_gpio = 0x0; //power led green
		cfg->diag_gpio = 0x1; // power led amber
		cfg->connected_gpio = 0x2; //wan led green
		cfg->wlan0_gpio = 0x3; //radio 0 led green
		cfg->wlan1_gpio = 0x4; // radio 1 led blue
		cfg->usb_gpio = 0x5; //usb led green
		cfg->ses_gpio = GPIO_LOW | 0x6; // WPS led green - inverse
		cfg->ses2_gpio = GPIO_LOW | 0x7; // WLAN led green - inverse
		break;
	case ROUTER_DLINK_DIR860:
		cfg->usb_power = 0xa;
		cfg->connected_gpio = GPIO_LOW | 0x4;
		cfg->disconnected_gpio = GPIO_LOW | 0x3;
		cfg->power_gpio = GPIO_LOW | 0x1;
		cfg->diag_gpio = GPIO_LOW | 0x0;
		cfg->diag_gpio_disabled = GPIO_LOW | 0x1;
		break;
	case ROUTER_DLINK_DIR868:
	case ROUTER_DLINK_DIR868C:
		cfg->usb_power = 0xa;
		cfg->connected_gpio = GPIO_LOW | 0x3;
		cfg->disconnected_gpio = GPIO_LOW | 0x1;
		cfg->power_gpio = GPIO_LOW | 0x2;
		cfg->diag_gpio = GPIO_LOW | 0x0;
		break;

	case ROUTER_DLINK_DIR880:
		cfg->connected_gpio = GPIO_LOW | 0x3;
		cfg->disconnected_gpio = GPIO_LOW | 0x1;
		cfg->power_gpio = GPIO_LOW | 0x2;
		cfg->diag_gpio = GPIO_LOW | 0x0;
		cfg->diag_gpio_disabled = GPIO_LOW | 0x2;
		cfg->usb_gpio = GPIO_LOW | 0x8;
		cfg->usb_gpio1 = GPIO_LOW | 0xf;
		//              cfg->wlan0_gpio = GPIO_LOW | 0xd;
		//              cfg->wlan1_gpio = GPIO_LOW | 0xe;
		cfg->usb_power = 0x9;
		cfg->usb_power1 = 0xa;
		break;
	case ROUTER_DLINK_DIR885:
		cfg->usb_power = 0x12;
		cfg->usb_gpio = GPIO_LOW | 0x8;
		cfg->power_gpio = GPIO_LOW | 0x0;
		cfg->diag_gpio = GPIO_LOW | 0x2;
		cfg->diag_gpio_disabled = GPIO_LOW | 0x0;
		cfg->disconnected_gpio = GPIO_LOW | 0x3;
		cfg->connected_gpio = GPIO_LOW | 0x1;
		cfg->wlan0_gpio = GPIO_LOW | 0xd;
		cfg->wlan1_gpio = GPIO_LOW | 0xe;
		break;
	case ROUTER_DLINK_DIR895:
		cfg->usb_power = 0x15;
		cfg->usb_power1 = 0x12;
		cfg->usb_gpio = GPIO_LOW | 0x8;
		cfg->usb_gpio1 = GPIO_LOW | 0xf;
		cfg->power_gpio = GPIO_LOW | 0x0;
		cfg->diag_gpio = GPIO_LOW | 0x2;
		cfg->diag_gpio_disabled = GPIO_LOW | 0x0;
		cfg->disconnected_gpio = GPIO_LOW | 0x3;
		cfg->connected_gpio = GPIO_LOW | 0x1;
		cfg->wlan0_gpio = GPIO_LOW | 0xd;
		cfg->wlan1_gpio = GPIO_LOW | 0xe;
		break;
	case ROUTER_DLINK_DIR890:
		cfg->usb_power = 0x15;
		cfg->usb_power1 = 0x12;
		cfg->usb_gpio = GPIO_LOW | 0x8;
		cfg->usb_gpio1 = GPIO_LOW | 0xf;
		cfg->connected_gpio = GPIO_LOW | 0x1;
		cfg->disconnected_gpio = GPIO_LOW | 0x3;
		cfg->power_gpio = GPIO_LOW | 0x2;
		cfg->diag_gpio = 0x2;
		break;
	case ROUTER_TRENDNET_TEW828:
		cfg->usb_gpio = GPIO_LOW | 0x4;
		cfg->power_gpio = GPIO_LOW | 0x6;
		cfg->diag_gpio = 0x6;
		break;
	case ROUTER_TRENDNET_TEW812:
		// gpio !1 = 2.4 ghz led
		// gpio !2 = 5 ghz led
		// gpio !3 = power somthing
		// gpio !8 = usb led
		//
		cfg->usb_gpio = GPIO_LOW | 0x8;
		cfg->diag_gpio = GPIO_LOW | 0x3;
		cfg->wlan0_gpio = GPIO_LOW | 0x1;
		cfg->wlan1_gpio = GPIO_LOW | 0x2;
		break;
	case ROUTER_ASUS_RTN18U:
		cfg->power_gpio = GPIO_LOW | 0x0;
		//              cfg->usb_power = 0xd;      //usb power on/off
		if (nvram_match("bl_version", "3.0.0.7")) {
			cfg->usb_gpio = GPIO_LOW | 0xe;
			cfg->connected_gpio = GPIO_LOW | 0x3;
			cfg->disconnected_gpio = GPIO_LOW | 0x6;
		} else if (nvram_match("bl_version", "1.0.0.0")) {
			cfg->usb_gpio = GPIO_LOW | 0x3;
			cfg->connected_gpio = GPIO_LOW | 0x6;
			cfg->disconnected_gpio = GPIO_LOW | 0x9;
		} else {
			cfg->usb_gpio = GPIO_LOW | 0x3;
			cfg->usb_gpio1 = GPIO_LOW | 0xe;
			cfg->connected_gpio = GPIO_LOW | 0x6;
			cfg->disconnected_gpio = GPIO_LOW | 0x9;
		}
		break;
	case ROUTER_TPLINK_ARCHERC8:
		cfg->ses_gpio = 0x6;
		cfg->usb_gpio = 0x2;
		cfg->diag_gpio = GPIO_LOW | 0x4;
		//              cfg->usb_gpio1 = 0x7;
		//              cfg->disconnected_gpio = 0xf;
		cfg->connected_gpio = 0x8;
		cfg->power_gpio = 0x4;
		//              cfg->diag_gpio = 0x12;
		cfg->usb_power = 0xa; // usb 3
		cfg->usb_power1 = 0x9; // usb 2
		break;
	case ROUTER_TPLINK_ARCHERC8_V4:
		cfg->ses_gpio = 0x2;
		cfg->usb_gpio = 0x6;
		cfg->usb_gpio1 = 0x7;
		cfg->disconnected_gpio = 0xf;
		cfg->connected_gpio = 0xe;
		cfg->power_gpio = GPIO_LOW | 0x12;
		cfg->diag_gpio = 0x12;
		cfg->usb_power = 0xc; // usb 3
		cfg->usb_power1 = 0xd; // usb 2
		break;
	case ROUTER_TPLINK_ARCHERC9:
		cfg->ses_gpio = 0x2;
		cfg->usb_gpio = 0x6;
		cfg->usb_gpio1 = 0x7;
		cfg->disconnected_gpio = 0xf;
		cfg->connected_gpio = 0xe;
		cfg->power_gpio = GPIO_LOW | 0x12;
		cfg->diag_gpio = 0x12;
		cfg->usb_power = 0xc; // usb 3
		cfg->usb_power1 = 0xd; // usb 2
		break;
	case ROUTER_TPLINK_ARCHERC3150:
		cfg->ses_gpio = 0x2;
		//              cfg->usb_gpio = 0x6;
		//              cfg->usb_gpio1 = 0x7;
		//              cfg->disconnected_gpio = 0xf;
		//              cfg->connected_gpio = 0xe;
		//              cfg->power_gpio = GPIO_LOW | 0x12;
		//              cfg->diag_gpio = 0x12;
		cfg->usb_power = 0xc; // usb 3
		cfg->usb_power1 = 0xd; // usb 2
		break;
	case ROUTER_ASUS_AC67U:
	case ROUTER_ASUS_AC56U:
		cfg->wlan1_gpio = GPIO_LOW | 0x6;
		cfg->power_gpio = GPIO_LOW | 0x3;
		cfg->usb_power = 0x9; //usb power on/off
		cfg->usb_power1 = 0xa; //usb power on/off
		cfg->usb_gpio = GPIO_LOW | 0xe;
		cfg->usb_gpio1 = GPIO_LOW | 0x0;
		cfg->diag_gpio = 0x3;
		cfg->connected_gpio = GPIO_LOW | 0x1;
		cfg->disconnected_gpio = GPIO_LOW | 0x2;
		break;
	case ROUTER_ASUS_AC3200:
		cfg->usb_power = 0x9;
		cfg->power_gpio = GPIO_LOW | 0x3;
		cfg->connected_gpio = GPIO_LOW | 0x5;
		cfg->diag_gpio = 0x3;
		// wps gpio = 14
		break;
	case ROUTER_ASUS_AC1200:
		cfg->usb_power = GPIO_LOW | 0xa;
		cfg->diag_gpio = 0xa;
		cfg->diag_gpio_disabled = GPIO_LOW | 0xa;
		cfg->usb_gpio = GPIO_LOW | 0xf;
		break;
	case ROUTER_ASUS_AC88U:
	case ROUTER_ASUS_AC3100:
	case ROUTER_ASUS_AC5300:
		cfg->usb_power = 0x9;
		cfg->usb_gpio = GPIO_LOW | 0x10;
		cfg->usb_gpio1 = GPIO_LOW | 0x11;
		cfg->power_gpio = GPIO_LOW | 0x3;
		cfg->diag_gpio = 0x3;
		cfg->connected_gpio = 0x5;
		cfg->disconnected_gpio = GPIO_LOW | 0x15;
		cfg->ses_gpio = GPIO_LOW | 0x13;
		// komisches symbol gpio 21
		// quantenna reset 8 inv (off / on to reset)
		break;
	case ROUTER_ASUS_AC87U:
		cfg->usb_power = 0x9;
		cfg->power_gpio = GPIO_LOW | 0x3;
		cfg->connected_gpio = GPIO_LOW | 0x5;
		cfg->ses_gpio = GPIO_LOW | 0x1;
		// quantenna reset 8 inv (off / on to reset)
		break;
	case ROUTER_NETGEAR_EX6200:
		//cfg->power_gpio = GPIO_LOW | 0x9;   // connected red
		cfg->diag_gpio = GPIO_LOW | 0x1; // Netgear logo
		cfg->connected_gpio = GPIO_LOW | 0x8; // connected green
		cfg->wlan1_gpio = GPIO_LOW | 0xb; // radio led red 2.4G
		cfg->wlan0_gpio = GPIO_LOW | 0xd; // radio led red 5G
		cfg->usb_gpio = GPIO_LOW | 0x5; // usb led
		//cfg->usb_power = 0x0;    // usb enable
		break;
	case ROUTER_NETGEAR_AC1450:
		cfg->power_gpio = GPIO_LOW | 0x2; // power led green
		//cfg->diag_gpio = GPIO_LOW | 0x3;    // power led orange
		cfg->diag_gpio = GPIO_LOW | 0x1; // Netgear logo
		cfg->connected_gpio = GPIO_LOW | 0xa; // wan led green - hw controlled
		cfg->wlan0_gpio = GPIO_LOW | 0xb; // radio led blue
		cfg->usb_gpio = GPIO_LOW | 0x8; // usb led
		//cfg->usb_power = 0x0;    // usb enable
		break;
	case ROUTER_NETGEAR_R6250:
		cfg->power_gpio = GPIO_LOW | 0x2; // power led green
		//cfg->diag_gpio = GPIO_LOW | 0x3;    // power led orange
		cfg->diag_gpio = 0x1; // Netgear logo
		//emblem0_gpio = 0x1; // NETGEAR Emblem
		cfg->connected_gpio = GPIO_LOW | 0xf; // wan led green
		cfg->wlan0_gpio = GPIO_LOW | 0xb; // radio led blue
		cfg->usb_gpio = GPIO_LOW | 0x8; // usb led green
		//cfg->usb_power = 0x0;    // usb enable
		break;
	case ROUTER_NETGEAR_R6300:
		cfg->usb_gpio = GPIO_LOW | 0x8; //usb led
		cfg->usb_power = 0x0; //usb power on/off
		cfg->connected_gpio = GPIO_LOW | 0xf; //green led
		cfg->power_gpio = GPIO_LOW | 0x2; //power orange led
		cfg->diag_gpio = GPIO_LOW | 0x3; //power led orange
		//cfg->diag_gpio_disabled=0x9;//netgear logo led r
		//emblem0_gpio = GPIO_LOW | 0x1;   // NETGEAR Emblem l
		//emblem1_gpio = GPIO_LOW | 0x9;   // NETGEAR Emblem r
		cfg->wlan0_gpio = GPIO_LOW | 0xb; // radio led blue
		break;
	case ROUTER_NETGEAR_R6300V2:
		cfg->power_gpio = GPIO_LOW | 0x2; // power led green
		//cfg->diag_gpio = GPIO_LOW | 0x3;    // power led orange
		cfg->diag_gpio = GPIO_LOW | 0x1; // Netgear logo
		cfg->connected_gpio = GPIO_LOW | 0xa; // wan led green - hw controlled
		cfg->wlan0_gpio = GPIO_LOW | 0xb; // radio led blue
		cfg->usb_gpio = GPIO_LOW | 0x8; // usb led
		//cfg->usb_power = 0x0;    // usb enable
		break;
	case ROUTER_NETGEAR_R6400:
	case ROUTER_NETGEAR_R6400V2:
	case ROUTER_NETGEAR_R6700V3:
		cfg->power_gpio = GPIO_LOW | 0x1; //
		cfg->connected_gpio = GPIO_LOW | 0x7; //
		cfg->usb_power = 0x0; //
		cfg->diag_gpio = GPIO_LOW | 0x2; //
		cfg->wlan0_gpio = GPIO_LOW | 0x9; // radio 0
		cfg->wlan1_gpio = GPIO_LOW | 0x8; // radio 1
		cfg->ses_gpio = GPIO_LOW | 0xa; // wps led
		cfg->wlan_gpio = GPIO_LOW | 0xb; // wifi button led
		cfg->usb_gpio = GPIO_LOW | 0xc; // usb1
		cfg->usb_gpio1 = GPIO_LOW | 0xd; // usb2
		break;
	case ROUTER_NETGEAR_R7000:
		cfg->power_gpio = GPIO_LOW | 0x2; // power led
		cfg->diag_gpio = GPIO_LOW | 0x3; // power led orange
		cfg->connected_gpio = GPIO_LOW | 0x9; // wan led
		cfg->usb_power = 0x0; // usb enable
		cfg->wlan0_gpio = GPIO_LOW | 0xd; // radio 0
		cfg->wlan1_gpio = GPIO_LOW | 0xc; // radio 1
		cfg->ses_gpio = GPIO_LOW | 0xe; // wps led
		//cfg->wlan_gpio = GPIO_LOW | 0xf;    // wifi button led
		cfg->usb_gpio = GPIO_LOW | 0x11; //usb1
		cfg->usb_gpio1 = GPIO_LOW | 0x12; //usb2
		break;
	case ROUTER_NETGEAR_R7000P:
		cfg->power_gpio = GPIO_LOW | 0x2; // power led *
		cfg->diag_gpio = GPIO_LOW | 0x3; // power led orange *
		cfg->connected_gpio = GPIO_LOW | 0x8; // wan led
		//cfg->usb_power = 0x0;    // usb enable
		cfg->wlan0_gpio = GPIO_LOW | 0x9; // radio 0 *
		cfg->wlan1_gpio = GPIO_LOW | 0xa; // radio 1 *
		cfg->ses_gpio = GPIO_LOW | 0xb; // wps led * //13 is wifi
		//cfg->wlan_gpio = GPIO_LOW | 0xf;    // wifi button led
		cfg->usb_gpio = GPIO_LOW | 0xe; //usb1 *
		cfg->usb_gpio1 = GPIO_LOW | 0xf; //usb2 *
		break;
	case ROUTER_NETGEAR_R7500V2:
	case ROUTER_NETGEAR_R7500:
		cfg->power_gpio = 0x0; // power led
		cfg->diag_gpio = 0xa; // power led orange
		cfg->diag_gpio_disabled = 0x0; // power led orange
		cfg->connected_gpio = 0x7; // wan led
		cfg->usb_power = 0x10; // usb enable
		cfg->usb_power1 = 0xf; // usb enable
		cfg->wlan0_gpio = 0x1; // radio 0
		cfg->wlan1_gpio = GPIO_LOW | 0x2; // radio 1
		cfg->ses_gpio = GPIO_LOW | 0x9; // wps led
		cfg->wlan_gpio = GPIO_LOW | 0x8; // wifi button led
		cfg->usb_gpio = 0x4; //usb1
		cfg->usb_gpio1 = 0x5; //usb2
		break;
	case ROUTER_LINKSYS_EA8300:
		cfg->diag_gpio = 0x2f; // power led off
		cfg->connected_gpio = 0x31; // wan led
		cfg->sec0_gpio = 0x2e; // wps led
		cfg->sec1_gpio = 0x16; //
		cfg->power_gpio = 0x2d; // power led
		break;
	case ROUTER_ASUS_AC58U:
		cfg->power_gpio = 0x3; // power led
		cfg->diag_gpio_disabled = 0x3; // power led off
		break;
	case ROUTER_HABANERO:
#ifdef HAVE_ANTAIRA
		cfg->diag_gpio = 0xD4; //gpio 212 on i2c slave antaira-gpio
		cfg->beeper_gpio = 0xD7; //gpio 215 on i2c slave antaira-gpio
#endif
		break;
	case ROUTER_ASUS_AX89X:
		cfg->power_gpio = 0x0;
		cfg->diag_gpio = GPIO_LOW | 0x0;
		cfg->diag_gpio_disabled = 0;
		cfg->connected_gpio = 0x2;
		cfg->disconnected_gpio = 0x1;
		cfg->usb_power = 0x1e; // usb enable
		cfg->usb_power1 = 0x1f;
		break;
	case ROUTER_LINKSYS_MX4200V1:
	case ROUTER_LINKSYS_MX4200V2:
	case ROUTER_LINKSYS_MX4300:
	case ROUTER_LINKSYS_MX5500:
		cfg->power_gpio = 0x1; // power led / green
		cfg->diag_gpio = 0x0; // diag led / red
		cfg->diag_gpio_disabled = 0x1;
		cfg->connected_gpio = 0x2; // blue wan led
		break;
	case ROUTER_LINKSYS_MR5500:
		cfg->power_gpio = 0x1; // power led / green
		cfg->diag_gpio = 0x0; // diag led / red
		cfg->diag_gpio_disabled = 0x1;
		cfg->connected_gpio = 0x2; // blue wan led
		cfg->usb_gpio = GPIO_LOW | 18; //usb led
		cfg->usb_power = GPIO_LOW | 17; //usb power
		break;
	case ROUTER_LINKSYS_MR7350:
		cfg->power_gpio = 0x1; // power led / green
		cfg->diag_gpio = 0x0; // diag led / red
		cfg->diag_gpio_disabled = 0x1;
		cfg->connected_gpio = 0x2; // blue wan led
		cfg->usb_gpio = GPIO_LOW | 0x3c; //usb1
		cfg->usb_power = 0x3d; //usb2
		break;
	case ROUTER_DYNALINK_DLWRX36:
		cfg->power_gpio = 0x1a; // power led / green
		cfg->diag_gpio = 0x19; // diag led / red
		cfg->diag_gpio_disabled = 0x1a;
		break;
	case ROUTER_BUFFALO_WXR5950AX12:
		cfg->power_gpio = 34; // power led / green
		cfg->diag_gpio = 31; // diag led / red
		cfg->diag_gpio_disabled = 34;
		cfg->connected_gpio = 43; // internet white
		cfg->disconnected_gpio = 44; // internet red
		cfg->wlan0_gpio = 55; // radio 5G
		cfg->wlan1_gpio = 56; // radio 2G
		cfg->usb_power = 64; //usb
		break;
	case ROUTER_FORTINET_FAP231F:
		cfg->power_gpio = 0x12; // power led / amber
		cfg->diag_gpio = 0x16; // diag led / red
		cfg->usb_power = 74 | GPIO_LOW; //usb2
		break;
	case ROUTER_NETGEAR_R7800:
		cfg->power_gpio = 0x0; // power led
		cfg->diag_gpio = 0xa; // power led orange
		cfg->diag_gpio_disabled = 0x0; // power led orange
		cfg->connected_gpio = 0x7; // wan led
		cfg->usb_power = 0x10; // usb enable
		cfg->usb_power1 = 0xf;
		cfg->wlan0_gpio = 0x9; // radio 5G
		cfg->wlan1_gpio = 0x8; // radio 2G
		//cfg->ses_gpio = GPIO_LOW | 0x9;     // wps button led used for 2G
		//cfg->wlan_gpio = 0x8;    // wifi button led used for 5G
		cfg->usb_gpio = 0x4; //usb1
		cfg->usb_gpio1 = 0x5; //usb2
		break;
	case ROUTER_ASROCK_G10:
		cfg->diag_gpio = 0x9; // power led orange
		cfg->connected_gpio = 0x8; // wan led
		cfg->disconnected_gpio = 0x7; // wan led
		break;
	case ROUTER_NETGEAR_R9000:

		cfg->power_gpio = 0x16; // power led
		cfg->diag_gpio = GPIO_LOW | 0x16; // power led orange
		cfg->diag_gpio_disabled = 0x16; // power led orange
		cfg->connected_gpio = 0x17; // wan led
		//      cfg->usb_power = 0x10;      // usb enable
		//      cfg->usb_power1 = 0xf;
		cfg->ses_gpio = GPIO_LOW | 0x27; // wps button led used for 2G
		cfg->usb_gpio = 0x24; //usb1
		cfg->usb_gpio1 = 0x25; //usb2
		break;
	case ROUTER_TRENDNET_TEW827:
		cfg->power_gpio = GPIO_LOW | 0x35; // power led
		cfg->usb_gpio = GPIO_LOW | 0x7; // usb led
		break;
	case ROUTER_NETGEAR_R8000:
		cfg->power_gpio = GPIO_LOW | 0x2; // power led
		cfg->diag_gpio = GPIO_LOW | 0x3; // power led orange
		cfg->connected_gpio = GPIO_LOW | 0x9; // wan led green
		cfg->usb_power = 0x0; // usb enable
		cfg->wlan0_gpio = GPIO_LOW | 0xd; // radio 2G
		cfg->wlan1_gpio = GPIO_LOW | 0xc; // radio 5G-1
		cfg->wlan2_gpio = GPIO_LOW | 0x10; // radio 5G-2
		cfg->ses_gpio = GPIO_LOW | 0xe; // wps led
		cfg->wlan_gpio = GPIO_LOW | 0xf; // wifi button led
		cfg->usb_gpio = GPIO_LOW | 0x11; //usb1
		cfg->usb_gpio1 = GPIO_LOW | 0x12; //usb2
		break;
	case ROUTER_NETGEAR_R8500:
		cfg->power_gpio = GPIO_LOW | 0x2; // power led
		cfg->diag_gpio = GPIO_LOW | 0xf; //
		cfg->connected_gpio = GPIO_LOW | 0x9; // wan led white 1Gb amber 100Mb
		cfg->usb_power = 0x0; // usb enable
		cfg->wlan0_gpio = GPIO_LOW | 0xb; // radio 5G-1
		cfg->wlan1_gpio = GPIO_LOW | 0xd; // radio 2G
		cfg->wlan2_gpio = GPIO_LOW | 0xc; // radio 5G-2
		cfg->ses_gpio = GPIO_LOW | 0xe; // wps led
		cfg->wlan_gpio = 0x14; // wifi button led
		cfg->usb_gpio = GPIO_LOW | 0x11; //usb1
		cfg->usb_gpio1 = GPIO_LOW | 0x12; //usb2
		break;
	case ROUTER_NETGEAR_WNDR4500:
	case ROUTER_NETGEAR_WNDR4500V2:
		cfg->power_gpio = GPIO_LOW | 0x2; //power led green
		cfg->diag_gpio = GPIO_LOW | 0x3; // power led amber
		cfg->connected_gpio = GPIO_LOW | 0xf; //wan led green
		cfg->wlan0_gpio = GPIO_LOW | 0x9; //radio 0 led green
		cfg->wlan1_gpio = GPIO_LOW | 0xb; // radio 1 led blue
		cfg->usb_gpio = GPIO_LOW | 0x8; //usb led green
		cfg->usb_gpio1 = GPIO_LOW | 0xe; //usb1 led green
		break;
	case ROUTER_ASUS_RTN66:
	case ROUTER_ASUS_AC66U:
		cfg->power_gpio = GPIO_LOW | 0xc;
		cfg->diag_gpio = 0xc;
		cfg->usb_gpio = GPIO_LOW | 0xf;
		break;
	case ROUTER_NETGEAR_WNR2000V2:

		//cfg->power_gpio = ??;
		cfg->diag_gpio = 0x2;
		cfg->ses_gpio = 0x7; //WPS led
		cfg->connected_gpio = 0x6;
		break;
	case ROUTER_WRT320N:
		cfg->power_gpio = 0x2; //power/diag (disabled=blink)
		cfg->ses_gpio = GPIO_LOW | 0x3; // ses blue
		cfg->connected_gpio = GPIO_LOW | 0x4; //ses orange
		break;
	case ROUTER_ASUS_RTN12:
		cfg->power_gpio = GPIO_LOW | 0x2;
		cfg->diag_gpio = 0x2; // power blink
		break;
	case ROUTER_BOARD_NEPTUNE:
//              cfg->usb_gpio = GPIO_LOW | 0x8;
// GPIO_LOW | 0xc //unknown gpio label, use as diag
#ifdef HAVE_RUT500
		cfg->diag_gpio = GPIO_LOW | 0xe;
#else
		cfg->diag_gpio = GPIO_LOW | 0xc;
#endif
		break;
	case ROUTER_ASUS_RTN10U:
		cfg->ses_gpio = 0x7;
		cfg->usb_gpio = 0x8;
		break;
	case ROUTER_ASUS_RTN12B:
		cfg->connected_gpio = GPIO_LOW | 0x5;
		break;
	case ROUTER_ASUS_RTN10PLUSD1:
		cfg->ses_gpio = 0x7;
		cfg->power_gpio = GPIO_LOW | 0x6;
		cfg->diag_gpio = 0x6;
		break;
	case ROUTER_ASUS_RTN10:
	case ROUTER_ASUS_RTN16:
	case ROUTER_NETCORE_NW618:
		cfg->power_gpio = GPIO_LOW | 0x1;
		cfg->diag_gpio = 0x1; // power blink
		break;
	case ROUTER_BELKIN_F7D3301:
	case ROUTER_BELKIN_F7D3302:
	case ROUTER_BELKIN_F7D4301:
	case ROUTER_BELKIN_F7D4302:
		cfg->power_gpio = GPIO_LOW | 0xa; // green
		cfg->diag_gpio = GPIO_LOW | 0xb; // red
		cfg->ses_gpio = GPIO_LOW | 0xd; // wps orange
		break;
	case ROUTER_DYNEX_DX_NRUTER:
		cfg->power_gpio = 0x1;
		cfg->diag_gpio = GPIO_LOW | 0x1; // power blink
		cfg->connected_gpio = GPIO_LOW | 0x0;
		cfg->sec0_gpio = GPIO_LOW | 0x3;
		break;
#endif
	}
}

static struct ledconfig led_cfg;

int led_control(int type, int act)
/*
 * type: LED_POWER, LED_DIAG, LED_DMZ, LED_CONNECTED, LED_BRIDGE, LED_VPN,
 * LED_SES, LED_SES2, LED_WLAN0, LED_WLAN1, LED_WLAN2, LED_SEC0, LED_SEC1, USB_POWER, USB_POWER1
 * act: LED_ON, LED_OFF, LED_FLASH 
 * 1st hex number: 1 = inverted, 0 = normal
 */
{
#if (defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_MAGICBOX) || (defined(HAVE_RB600) && !defined(HAVE_WDR4900)) || \
     defined(HAVE_MERAKI) || defined(HAVE_LS2) || defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_LS5)) &&                \
	(!defined(HAVE_DIR300) && !defined(HAVE_WRT54G2) && !defined(HAVE_RTG32) && !defined(HAVE_DIR400) &&                     \
	 !defined(HAVE_BWRG1000))
	return 0;
#else
	int use_gpio = 0x0fff;
	static struct ledconfig *cfg = NULL;
	if (!cfg) {
		cfg = &led_cfg;
		getledconfig(cfg);
	}

	switch (type) {
	case POE_GPIO:
		use_gpio = cfg->poe_gpio;
		break;
	case LED_POWER:
		use_gpio = cfg->power_gpio;
		break;
	case BEEPER:
		use_gpio = cfg->beeper_gpio;
		break;
	case USB_POWER:
		use_gpio = cfg->usb_power;
		break;
	case USB_POWER1:
		use_gpio = cfg->usb_power1;
		break;
	case LED_DIAG:
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
		if (cfg->v1func == 1) {
			if (act == LED_ON)
				C_led(1);
			else
				C_led(0);
		}
#endif
		if (act == LED_ON)
			led_control(LED_DIAG_DISABLED, LED_OFF);
		else
			led_control(LED_DIAG_DISABLED, LED_ON);
		use_gpio = cfg->diag_gpio;
		break;
	case LED_DIAG_DISABLED:
		use_gpio = cfg->diag_gpio_disabled;
		break;
	case LED_DMZ:
		use_gpio = cfg->dmz_gpio;
		break;
	case LED_CONNECTED:
		if (act == LED_ON)
			led_control(LED_DISCONNECTED, LED_OFF);
		else
			led_control(LED_DISCONNECTED, LED_ON);
		use_gpio = cfg->connblue ? cfg->ses_gpio : cfg->connected_gpio;
		break;
	case LED_DISCONNECTED:
		use_gpio = cfg->disconnected_gpio;
		break;
	case LED_BRIDGE:
		use_gpio = cfg->bridge_gpio;
		break;
	case LED_VPN:
		use_gpio = cfg->vpn_gpio;
		break;
	case LED_SES:
		use_gpio = cfg->connblue ? cfg->connected_gpio : cfg->ses_gpio;
		break;
	case LED_SES2:
		use_gpio = cfg->ses2_gpio;
		break;
	case LED_WLAN:
		use_gpio = cfg->wlan_gpio;
		break;
	case LED_WLAN0:
		use_gpio = cfg->wlan0_gpio;
		break;
	case LED_WLAN1:
		use_gpio = cfg->wlan1_gpio;
		break;
	case LED_WLAN2:
		use_gpio = cfg->wlan2_gpio;
		break;
	case LED_USB:
		use_gpio = cfg->usb_gpio;
		break;
	case LED_USB1:
		use_gpio = cfg->usb_gpio1;
		break;
	case LED_SEC:
		use_gpio = cfg->sec_gpio;
		break;
	case LED_SEC0:
		use_gpio = cfg->sec0_gpio;
		break;
	case LED_SEC1:
		use_gpio = cfg->sec1_gpio;
		break;
	}
	if (act == GPIO_CHECK) {
		if ((use_gpio & GPIO_MASK) != GPIO_MASK)
			return 1;
		else
			return 0;
	}
	if ((use_gpio & GPIO_MASK) != GPIO_MASK) {
		int gpio_value = use_gpio & GPIO_MASK;
		int enable = (use_gpio & GPIO_LOW) == 0 ? 1 : 0;
		int disable = (use_gpio & GPIO_LOW) == 0 ? 0 : 1;
		int setin = (use_gpio & GPIO_IN) == 0 ? 0 : 1;
		switch (act) {
		case LED_ON:
			set_gpio(gpio_value, enable);
			if (setin)
				get_gpio(gpio_value);
			break;
		case LED_OFF:
			set_gpio(gpio_value, disable);
			break;
		case LED_FLASH: // will lit the led for 1 sec.
			set_gpio(gpio_value, enable);
			struct timespec tim, tim2;
			tim.tv_sec = 1;
			tim.tv_nsec = 0;
			nanosleep(&tim, &tim2);
			set_gpio(gpio_value, disable);
			break;
		}
	}
	return 1;

#endif
}

static int diag_led_4702(int type, int act)
{
#if defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_XSCALE) || defined(HAVE_LAGUNA) || defined(HAVE_MAGICBOX) ||  \
	defined(HAVE_RB600) || defined(HAVE_FONERA) || defined(HAVE_MERAKI) || defined(HAVE_LS2) || defined(HAVE_WHRAG108) || \
	defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LS5) ||          \
	defined(HAVE_FONERA) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_STORM) || defined(HAVE_ADM5120) ||  \
	defined(HAVE_RT2880) || defined(HAVE_OPENRISC)
	return 0;
#else
	if (act == START_LED) {
		switch (type) {
		case DMZ:
			writeprocsys("diag", "1");
			break;
		}
	} else {
		switch (type) {
		case DMZ:
			writeprocsys("diag", "0");
			break;
		}
	}
	return 0;
#endif
}

#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)

static int C_led_4702(int i)
{
#if defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_XSCALE) || defined(HAVE_LAGUNA) || defined(HAVE_MAGICBOX) ||  \
	defined(HAVE_RB600) || defined(HAVE_FONERA) || defined(HAVE_MERAKI) || defined(HAVE_LS2) || defined(HAVE_WHRAG108) || \
	defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LS5) ||          \
	defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_STORM) || defined(HAVE_ADM5120) || defined(HAVE_RT2880) ||  \
	defined(HAVE_OPENRISC)
	return 0;
#else
	FILE *fp;
	char string[10];
	int flg;

	bzero(string, 10);
	/*
	 * get diag before set 
	 */
	if ((fp = fopen("/proc/sys/diag", "r"))) {
		fgets(string, sizeof(string), fp);
		fclose(fp);
	} else
		perror("/proc/sys/diag");

	if (i)
		flg = atoi(string) | 0x10;
	else
		flg = atoi(string) & 0xef;

	bzero(string, 10);
	sprintf(string, "%d", flg);
	writeprocsys("diag", string);

	return 0;
#endif
}
#endif

static int diag_led_4704(int type, int act)
{
#if defined(HAVE_IPQ806X) || defined(HAVE_MVEBU) || defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_XSCALE) ||       \
	defined(HAVE_LAGUNA) || defined(HAVE_MAGICBOX) || defined(HAVE_RB600) || defined(HAVE_FONERA) || defined(HAVE_MERAKI) || \
	defined(HAVE_LS2) || defined(HAVE_WHRAG108) || defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_TW6600) ||         \
	defined(HAVE_PB42) || defined(HAVE_LS5) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_STORM) ||           \
	defined(HAVE_ADM5120) || defined(HAVE_RT2880) || defined(HAVE_OPENRISC) || defined(HAVE_ALPINE) || defined(HAVE_IPQ6018)
	return 0;
#else
	unsigned int control, in, outen, out;

#ifdef BCM94712AGR
	/*
	 * The router will crash, if we load the code into broadcom demo board. 
	 */
	return 1;
#endif
	static char hw_error = 0;
	// int brand;
	control = read_gpio("/dev/gpio/control");
	in = read_gpio("/dev/gpio/in");
	out = read_gpio("/dev/gpio/out");
	outen = read_gpio("/dev/gpio/outen");

	write_gpio("/dev/gpio/outen", (outen & 0x7c) | 0x83);
	switch (type) {
	case DIAG: // GPIO 1
		if (hw_error) {
			write_gpio("/dev/gpio/out", (out & 0x7c));
			return 1;
		}

		if (act == STOP_LED) { // stop blinking
			write_gpio("/dev/gpio/out", (out & 0x7c) | 0x83);
		} else if (act == START_LED) { // start blinking
			write_gpio("/dev/gpio/out", (out & 0x7c) | 0x81);
		} else if (act == MALFUNCTION_LED) { // start blinking
			write_gpio("/dev/gpio/out", (out & 0x7c));
			hw_error = 1;
		}
		break;
	}
	return 1;
#endif
}

static int diag_led_4712(int type, int act)
{
#if defined(HAVE_IPQ806X) || defined(HAVE_MVEBU) || defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_XSCALE) ||       \
	defined(HAVE_LAGUNA) || defined(HAVE_MAGICBOX) || defined(HAVE_RB600) || defined(HAVE_FONERA) || defined(HAVE_MERAKI) || \
	defined(HAVE_LS2) || defined(HAVE_WHRAG108) || defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_TW6600) ||         \
	defined(HAVE_PB42) || defined(HAVE_LS5) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_STORM) ||           \
	defined(HAVE_ADM5120) || defined(HAVE_RT2880) || defined(HAVE_OPENRISC) || defined(HAVE_ALPINE) || defined(HAVE_IPQ6018)
	return 0;
#else
	unsigned int control, in, outen, out, ctr_mask, out_mask;

#ifdef BCM94712AGR
	/*
	 * The router will crash, if we load the code into broadcom demo board. 
	 */
	return 1;
#endif
	control = read_gpio("/dev/gpio/control");
	in = read_gpio("/dev/gpio/in");
	out = read_gpio("/dev/gpio/out");
	outen = read_gpio("/dev/gpio/outen");

	ctr_mask = ~(1 << type);
	out_mask = (1 << type);

	write_gpio("/dev/gpio/control", control & ctr_mask);
	write_gpio("/dev/gpio/outen", outen | out_mask);

	if (act == STOP_LED) { // stop blinking
		write_gpio("/dev/gpio/out", out | out_mask);
	} else if (act == START_LED) { // start blinking
		write_gpio("/dev/gpio/out", out & ctr_mask);
	}

	return 1;
#endif
}

#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
static int C_led_4712(int i)
{
	if (i == 1)
		return diag_led(DIAG, START_LED);
	else
		return diag_led(DIAG, STOP_LED);
}

int C_led(int i)
{
	int brand = getRouterBrand();

	if (brand == ROUTER_WRT54G1X || brand == ROUTER_LINKSYS_WRT55AG)
		return C_led_4702(i);
	else if (brand == ROUTER_WRT54G)
		return C_led_4712(i);
	else
		return 0;
}

int diag_led(int type, int act)
{
	int brand = getRouterBrand();

	if (brand == ROUTER_WRT54G || brand == ROUTER_WRT54G3G || brand == ROUTER_WRT300NV11)
		return diag_led_4712(type, act);
	else if (brand == ROUTER_WRT54G1X || brand == ROUTER_LINKSYS_WRT55AG)
		return diag_led_4702(type, act);
	else if ((brand == ROUTER_WRTSL54GS || brand == ROUTER_WRT310N || brand == ROUTER_WRT350N ||
		  brand == ROUTER_BUFFALO_WZRG144NH) &&
		 type == DIAG)
		return diag_led_4704(type, act);
	else {
		if (type == DMZ) {
			if (act == START_LED)
				return led_control(LED_DMZ, LED_ON);
			if (act == STOP_LED)
				return led_control(LED_DMZ, LED_OFF);
			return 1;
		}
	}
	return 0;
}
#endif

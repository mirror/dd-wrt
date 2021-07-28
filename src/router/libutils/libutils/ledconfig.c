/*
 * ledconfig.c
 *
 * Copyright (C) 2018 Sebastian Gottschall <gottschall@dd-wrt.com>
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

static void getledconfig(struct ledconfig *cfg)
{

	cfg->power_gpio = 0xffff;
	cfg->beeper_gpio = 0xffff;
	cfg->diag_gpio = 0xffff;
	cfg->diag_gpio_disabled = 0xffff;
	cfg->dmz_gpio = 0xffff;
	cfg->connected_gpio = 0xffff;
	cfg->disconnected_gpio = 0xffff;
	cfg->bridge_gpio = 0xffff;
	cfg->vpn_gpio = 0xffff;
	cfg->ses_gpio = 0xffff;	// use for SES1 (Linksys), AOSS (Buffalo)
	cfg->ses2_gpio = 0xffff;
	cfg->wlan_gpio = 0xffff;	// wlan button led R7000
	cfg->wlan0_gpio = 0xffff;	// use this only if wlan led is not controlled by hardware!
	cfg->wlan1_gpio = 0xffff;
	cfg->wlan2_gpio = 0xffff;
	cfg->usb_gpio = 0xffff;
	cfg->usb_gpio1 = 0xffff;
	cfg->sec_gpio = 0xffff;	// generic
	cfg->sec0_gpio = 0xffff;	// security leds, wrt600n
	cfg->sec1_gpio = 0xffff;
	cfg->usb_power = 0xffff;
	cfg->usb_power1 = 0xffff;
	cfg->poe_gpio = 0xffff;
	cfg->v1func = 0;
	cfg->connblue = nvram_matchi("connblue", 1) ? 1 : 0;

	switch (getRouterBrand())	// gpio definitions here: 0xYZ,
		// Y=0:normal, Y=1:inverted, Z:gpio
		// number (f=disabled)
	{
#ifndef HAVE_BUFFALO
	case ROUTER_BOARD_TECHNAXX3G:
		cfg->usb_gpio = 0x1009;
		cfg->diag_gpio = 0x100c;
		cfg->connected_gpio = 0x100b;
		cfg->ses_gpio = 0x100c;
		break;
#ifdef HAVE_WPE72
	case ROUTER_BOARD_NS5M:
		cfg->diag_gpio = 0x100d;
		break;
#else
	case ROUTER_BOARD_NS5M:
		cfg->poe_gpio = 0x0008;
		break;
#endif
	case ROUTER_BOARD_UNIFI:
		cfg->ses_gpio = 0x0001;
		cfg->sec0_gpio = 0x0001;
		break;
	case ROUTER_BOARD_UNIFI_V2:
		cfg->connected_gpio = 0x000d;
		break;
	case ROUTER_UBNT_NANOAC:
		cfg->poe_gpio = 0x0003;
		break;
	case ROUTER_BOARD_NS2M:
		cfg->poe_gpio = 0x0008;
		break;
	case ROUTER_BOARD_NS5MXW:
		cfg->poe_gpio = 0x0002;
		break;
	case ROUTER_UBNT_UAPAC:
	case ROUTER_UBNT_UAPACPRO:
		cfg->ses_gpio = 0x0007;
		cfg->sec0_gpio = 0x0007;
		break;
	case ROUTER_BOARD_AIRROUTER:
		cfg->power_gpio = 0x100b;
		cfg->diag_gpio = 0x000b;
		cfg->connected_gpio = 0x1000;
		break;
	case ROUTER_BOARD_DANUBE:
#ifdef HAVE_WMBR_G300NH
		cfg->diag_gpio = 0x1005;
		cfg->ses_gpio = 0x100e;
		cfg->sec0_gpio = 0x100e;
		cfg->connected_gpio = 0x1011;
		cfg->disconnected_gpio = 0x1012;
		cfg->power_gpio = 0x1001;
#endif
#ifdef HAVE_SX763
//              cfg->diag_gpio = 0x1005;
//              cfg->ses_gpio = 0x100e;
//              cfg->sec0_gpio = 0x100e;
		cfg->connected_gpio = 0x10de;
//              cfg->disconnected_gpio = 0x1012;
//              cfg->power_gpio = 0x1001;
#endif
		break;
#ifdef HAVE_UNIWIP
	case ROUTER_BOARD_UNIWIP:
		break;
#endif
#ifdef HAVE_WDR4900
	case ROUTER_BOARD_WDR4900:
		cfg->diag_gpio = 0x0000;
		cfg->usb_gpio = 0x0001;
		cfg->usb_gpio1 = 0x0002;
		cfg->usb_power = 0x1003;
		break;
#endif
#ifdef HAVE_WRT1900AC
	case ROUTER_WRT_1200AC:
	case ROUTER_WRT_1900ACS:

	case ROUTER_WRT_1900ACV2:
		cfg->usb_power = 0x0032;
	case ROUTER_WRT_1900AC:
		cfg->power_gpio = 0x0000;
		cfg->diag_gpio = 0x1000;
		cfg->connected_gpio = 0x0006;
		cfg->disconnected_gpio = 0x0007;
//              cfg->usb_gpio = 0x0004;
//              cfg->usb_gpio1 = 0x0005;
		cfg->ses_gpio = 0x0009;
		break;
	case ROUTER_WRT_3200ACM:
	case ROUTER_WRT_32X:
//              cfg->usb_power = 0x002f;
		cfg->power_gpio = 0x0000;
		cfg->diag_gpio = 0x1000;
		cfg->connected_gpio = 0x0006;
		cfg->disconnected_gpio = 0x0007;
//              cfg->usb_gpio = 0x0004;
//              cfg->usb_gpio1 = 0x0005;
		cfg->ses_gpio = 0x0009;
		break;
#endif
	case ROUTER_BOARD_PB42:
#ifdef HAVE_WA901
		cfg->diag_gpio = 0x1002;
		cfg->ses_gpio = 0x0004;
//              cfg->usb_gpio = 0x1001;
#elif  HAVE_WR941
		cfg->diag_gpio = 0x1002;
		cfg->ses_gpio = 0x0005;
//              cfg->usb_gpio = 0x1001;
#endif
#ifdef HAVE_MR3020
		cfg->connected_gpio = 0x101b;
		cfg->diag_gpio = 0x101a;
		cfg->usb_power = 0x0008;
#elif HAVE_GL150
//              cfg->power_gpio = 0x101b;
//              cfg->diag_gpio = 0x001b;
//              cfg->usb_power = 0x0008;
#elif HAVE_WR710
		cfg->power_gpio = 0x101b;
		cfg->diag_gpio = 0x001b;
#elif HAVE_WA701V2
		cfg->diag_gpio = 0x101b;
		cfg->ses_gpio = 0x0001;
		cfg->sec0_gpio = 0x0001;
#elif HAVE_WR703
		cfg->diag_gpio = 0x101b;
		cfg->ses_gpio = 0x0001;
		cfg->sec0_gpio = 0x0001;
		cfg->usb_power = 0x0008;
#elif HAVE_WR842
		cfg->diag_gpio = 0x1001;
		cfg->ses_gpio = 0x0000;
		cfg->usb_power = 0x0006;

#elif HAVE_WR741V4
		cfg->diag_gpio = 0x101b;
		cfg->ses_gpio = 0x0001;
		cfg->sec0_gpio = 0x0001;

#elif HAVE_MR3420
		cfg->diag_gpio = 0x1001;
		cfg->connected_gpio = 0x1008;
		cfg->usb_power = 0x0006;
#elif HAVE_WR741
		cfg->diag_gpio = 0x1001;
		cfg->ses_gpio = 0x0000;
//              cfg->usb_gpio = 0x1001;
#endif
#ifdef HAVE_WR1043
		cfg->diag_gpio = 0x1002;
		cfg->ses_gpio = 0x0005;
//              cfg->usb_gpio = 0x1001;
#endif
#ifdef HAVE_WRT160NL
		cfg->power_gpio = 0x100e;
		cfg->connected_gpio = 0x1009;
		cfg->ses_gpio = 0x1008;
#endif
#ifdef HAVE_TG2521
		cfg->ses_gpio = 0x1003;
		cfg->diag_gpio = 0x1003;
		cfg->usb_power = 0x1005;
#endif
#ifdef HAVE_TEW632BRP
		cfg->diag_gpio = 0x1001;
		cfg->ses_gpio = 0x1003;
#endif
#ifdef HAVE_WP543
		cfg->diag_gpio = 0x1007;
		cfg->connected_gpio = 0x1006;
#endif
#ifdef HAVE_WP546
		cfg->beeper_gpio = 0x0001;
		cfg->diag_gpio = 0x1007;
		cfg->connected_gpio = 0x1006;
#endif
#ifdef HAVE_DIR825
		cfg->power_gpio = 0x1002;
		cfg->diag_gpio = 0x1001;
		cfg->connected_gpio = 0x100b;
		cfg->disconnected_gpio = 0x1006;
		cfg->ses_gpio = 0x1004;
		cfg->usb_gpio = 0x1000;
//              cfg->wlan0_gpio = 0x00ff; //correct states missing
#endif
#ifdef HAVE_WNDR3700
		cfg->power_gpio = 0x1002;
		cfg->diag_gpio = 0x1001;
		cfg->connected_gpio = 0x1006;
		cfg->ses_gpio = 0x1004;
#endif
#ifdef HAVE_WZRG300NH
		cfg->diag_gpio = 0x1001;
		cfg->connected_gpio = 0x1012;
		cfg->ses_gpio = 0x1011;
		cfg->sec0_gpio = 0x1011;
#endif
#ifdef HAVE_DIR632
		cfg->power_gpio = 0x0001;
		cfg->diag_gpio = 0x1000;
		cfg->connected_gpio = 0x1011;
		cfg->usb_gpio = 0x100b;
#endif
#ifdef HAVE_WZRG450
		cfg->diag_gpio = 0x100e;
		cfg->ses_gpio = 0x100d;
		cfg->sec0_gpio = 0x100d;
		cfg->usb_power = 0x0010;
		cfg->connected_gpio = 0x102e;	// card 1, gpio 14
#endif
#ifdef HAVE_WZRG300NH2
		cfg->diag_gpio = 0x1010;
		cfg->ses_gpio = 0x1026;	// card 1, gpio 6
		cfg->sec0_gpio = 0x1026;
		cfg->usb_power = 0x000d;
		cfg->connected_gpio = 0x1027;	// card 1, gpio 7
#endif
#ifdef HAVE_WZRHPAG300NH
		cfg->diag_gpio = 0x1001;
		cfg->connected_gpio = 0x1033;	// card 2 gpio 3
		cfg->sec0_gpio = 0x1025;
		cfg->sec1_gpio = 0x1031;
		cfg->ses_gpio = 0x1025;	// card 1 gpio 5
		cfg->ses2_gpio = 0x1031;	// card 2 gpio 5
		cfg->usb_power = 0x0002;
#endif
#ifdef HAVE_DIR615C1
		cfg->power_gpio = 0x1004;
		cfg->wlan0_gpio = 0x100f;
#endif
#ifdef HAVE_DIR615E
		cfg->power_gpio = 0x0006;
		cfg->diag_gpio = 0x0001;
		cfg->connected_gpio = 0x1011;
		cfg->disconnected_gpio = 0x0007;
		cfg->ses_gpio = 0x1000;
#endif
#ifdef HAVE_DAP2230
		cfg->diag_gpio = 0x000b;
		cfg->power_gpio = 0x100b;
#elif HAVE_LIMA
//              cfg->disconnected_gpio = 0x000f;
//              cfg->power_gpio = 0x1005;
//              cfg->diag_gpio = 0x0005;
#elif HAVE_RAMBUTAN
//              cfg->disconnected_gpio = 0x000f;
//              cfg->power_gpio = 0x1005;
//              cfg->diag_gpio = 0x0005;
#elif HAVE_WR940V6
		cfg->diag_gpio = 0x000f;
#elif HAVE_WR940V4
		cfg->disconnected_gpio = 0x000f;
		cfg->power_gpio = 0x1005;
		cfg->diag_gpio = 0x0005;

#elif HAVE_WR941V6
		cfg->disconnected_gpio = 0x000f;
		cfg->power_gpio = 0x1012;
		cfg->diag_gpio = 0x0012;

#elif HAVE_WR841V12
		cfg->power_gpio = 0x1001;
		cfg->diag_gpio = 0x0001;
		cfg->ses_gpio = 0x1003;
		cfg->sec0_gpio = 0x1003;
		cfg->connected_gpio = 0x1002;
#elif HAVE_WR841V11
		cfg->power_gpio = 0x1001;
		cfg->diag_gpio = 0x0001;
		cfg->ses_gpio = 0x1003;
		cfg->sec0_gpio = 0x1003;
		cfg->connected_gpio = 0x1002;
#elif HAVE_ARCHERC25
		cfg->power_gpio = 0x1011;
		cfg->diag_gpio = 0x0011;
		cfg->ses_gpio = 0x1002;
		cfg->sec0_gpio = 0x1002;
		cfg->connected_gpio = 0x007d;
		cfg->disconnected_gpio = 0x007c;
#elif HAVE_WR841V9
		cfg->diag_gpio = 0x1003;
#elif HAVE_WR842V2
		cfg->connected_gpio = 0x100e;
		cfg->usb_power = 0x2004;
		cfg->usb_gpio = 0x100f;
#elif HAVE_WR810N
		cfg->diag_gpio = 0x100d;
		cfg->usb_power = 0x000b;
#elif HAVE_WR841V8
		cfg->diag_gpio = 0x100f;
		cfg->connected_gpio = 0x100e;
#elif HAVE_DIR615I
		cfg->power_gpio = 0x000e;
		cfg->diag_gpio = 0x100f;
		cfg->connected_gpio = 0x100c;
		cfg->disconnected_gpio = 0x0016;
#endif
#ifdef HAVE_WRT400
		cfg->power_gpio = 0x0001;
		cfg->diag_gpio = 0x1005;
		cfg->ses_gpio = 0x1004;
		cfg->connected_gpio = 0x0007;
#endif
#ifdef HAVE_ALFAAP94
		cfg->power_gpio = 0x0005;
#endif
		break;
	case ROUTER_ALLNET01:
		cfg->connected_gpio = 0x1000;
		break;
	case ROUTER_BOARD_WP54G:
		cfg->diag_gpio = 0x1002;
		cfg->connected_gpio = 0x1007;
		break;
	case ROUTER_BOARD_NP28G:
		cfg->diag_gpio = 0x1002;
		cfg->connected_gpio = 0x1006;
		break;
	case ROUTER_BOARD_GATEWORX_GW2369:
		cfg->connected_gpio = 0x1002;
		break;
	case ROUTER_BOARD_GW2388:
	case ROUTER_BOARD_GW6400:
	case ROUTER_BOARD_GW2380:
#ifdef HAVE_NEWPORT

#elif defined(HAVE_VENTANA)
		cfg->power_gpio = 0x1066;
		cfg->diag_gpio = 0x006F;
		cfg->connected_gpio = 0x0066;
		cfg->disconnected_gpio = 0x0067;
#else
		cfg->connected_gpio = 0x1010;	// 16 is mapped to front led
#endif
		break;
	case ROUTER_BOARD_GATEWORX:
#ifdef HAVE_WG302V1
		cfg->diag_gpio = 0x1004;
		cfg->wlan0_gpio = 0x1005;
#elif HAVE_WG302
		cfg->diag_gpio = 0x1002;
		cfg->wlan0_gpio = 0x1004;
#else
		if (nvram_match("DD_BOARD", "Gateworks Cambria GW2350"))
			cfg->connected_gpio = 0x1005;
		else if (nvram_match("DD_BOARD", "Gateworks Cambria GW2358-4"))
			cfg->connected_gpio = 0x1018;
		else
			cfg->connected_gpio = 0x0003;
#endif
		break;
	case ROUTER_BOARD_GATEWORX_SWAP:
		cfg->connected_gpio = 0x0004;
		break;
	case ROUTER_BOARD_STORM:
		cfg->connected_gpio = 0x0005;
		cfg->diag_gpio = 0x0003;
		break;
	case ROUTER_LINKSYS_WRH54G:
		cfg->diag_gpio = 0x1001;	// power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_WRT54G:
	case ROUTER_WRT54G_V8:
		cfg->power_gpio = 0x0001;
		cfg->dmz_gpio = 0x1007;
		cfg->connected_gpio = 0x1003;	// ses orange
		cfg->ses_gpio = 0x1002;	// ses white
		cfg->ses2_gpio = 0x1003;	// ses orange
		break;
	case ROUTER_WRT54G_V81:
		cfg->power_gpio = 0x1001;
		cfg->dmz_gpio = 0x1002;
		cfg->connected_gpio = 0x1004;	// ses orange
		cfg->ses_gpio = 0x1003;	// ses white
		cfg->ses2_gpio = 0x1004;	// ses orange
		break;
	case ROUTER_WRT54G1X:
		cfg->connected_gpio = 0x1003;
		cfg->v1func = 1;
		break;
	case ROUTER_WRT350N:
		cfg->connected_gpio = 0x1003;
		cfg->power_gpio = 0x0001;
		cfg->ses2_gpio = 0x1003;	// ses orange
		cfg->sec0_gpio = 0x1009;
		cfg->usb_gpio = 0x100b;
		break;
	case ROUTER_WRT600N:
		cfg->power_gpio = 0x1002;
		cfg->diag_gpio = 0x0002;
		cfg->usb_gpio = 0x1003;
		cfg->sec0_gpio = 0x1009;
		cfg->sec1_gpio = 0x100b;
		break;
	case ROUTER_LINKSYS_WRT55AG:
		cfg->connected_gpio = 0x1003;
		break;
	case ROUTER_DLINK_DIR330:
		cfg->diag_gpio = 0x1006;
		cfg->connected_gpio = 0x1000;
		cfg->usb_gpio = 0x1004;
		break;
	case ROUTER_ASUS_RTN10PLUS:
//              cfg->diag_gpio = 0x100d;
//              cfg->connected_gpio = 0x1008;
//              cfg->power_gpio = 0x1009;
		break;
	case ROUTER_BOARD_DIR600B:
		cfg->diag_gpio = 0x100d;
		cfg->connected_gpio = 0x1008;
		cfg->power_gpio = 0x1009;
		break;
	case ROUTER_BOARD_DIR615D:
#ifdef HAVE_DIR615H
		cfg->diag_gpio = 0x0007;
		cfg->connected_gpio = 0x100d;
		cfg->disconnected_gpio = 0x100c;
		cfg->ses_gpio = 0x100e;
		cfg->power_gpio = 0x0009;
#else
		cfg->diag_gpio = 0x1008;
		cfg->connected_gpio = 0x100c;
		cfg->disconnected_gpio = 0x100e;
		cfg->ses_gpio = 0x100b;
		cfg->power_gpio = 0x1009;
#endif
		break;
		/*
		   DIR 882 
		   power LED red diag = 8 inv, green 16 inv

		 */
	case ROUTER_BOARD_W502U:
		cfg->connected_gpio = 0x100d;
		break;
	case ROUTER_BOARD_OPENRISC:
#ifndef HAVE_ERC
// ERC: diag button is used different / wlan button is handled by a script
		cfg->diag_gpio = 0x0003;
		cfg->ses_gpio = 0x0005;
#endif
		break;
	case ROUTER_BOARD_WR5422:
		cfg->ses_gpio = 0x100d;
		break;
	case ROUTER_BOARD_F5D8235:
		cfg->usb_gpio = 0x1017;
		cfg->diag_gpio = 0x1009;
		cfg->disconnected_gpio = 0x1006;
		cfg->connected_gpio = 0x1005;
		cfg->ses_gpio = 0x100c;
		break;
#else
	case ROUTER_BOARD_DANUBE:
#ifdef HAVE_WMBR_G300NH
		cfg->diag_gpio = 0x1005;
		cfg->ses_gpio = 0x100e;
		cfg->sec0_gpio = 0x100e;
		cfg->connected_gpio = 0x1011;
		cfg->disconnected_gpio = 0x1012;
		cfg->power_gpio = 0x1001;
#endif
		break;
	case ROUTER_BOARD_PB42:
#ifdef HAVE_WZRG300NH
		cfg->diag_gpio = 0x1001;
		cfg->connected_gpio = 0x1012;
		cfg->ses_gpio = 0x1011;
		cfg->sec0_gpio = 0x1011;
#endif
#ifdef HAVE_WZRHPAG300NH
		cfg->diag_gpio = 0x1001;
		cfg->connected_gpio = 0x1033;
		cfg->ses_gpio = 0x1025;
		cfg->ses2_gpio = 0x1031;
		cfg->sec0_gpio = 0x1025;
		cfg->sec1_gpio = 0x1031;
		cfg->usb_power = 0x0002;
#endif
#ifdef HAVE_WZRG450
		cfg->diag_gpio = 0x100e;
		cfg->ses_gpio = 0x100d;
		cfg->sec0_gpio = 0x100d;
		cfg->usb_power = 0x0010;
		cfg->connected_gpio = 0x102e;	// card 1, gpio 14
#endif
#ifdef HAVE_WZRG300NH2
		cfg->diag_gpio = 0x1010;
		cfg->ses_gpio = 0x1026;
		cfg->sec0_gpio = 0x1026;
		cfg->usb_power = 0x000d;
		cfg->connected_gpio = 0x1027;
#endif
		break;
#endif
	case ROUTER_BOARD_HAMEA15:
		cfg->diag_gpio = 0x1011;
		cfg->connected_gpio = 0x1014;
//              cfg->ses_gpio = 0x100e;
		break;
	case ROUTER_BOARD_WCRGN:
		cfg->diag_gpio = 0x1007;
		cfg->connected_gpio = 0x100b;
//              cfg->ses_gpio = 0x100e;
		break;
	case ROUTER_R6800:
		cfg->diag_gpio = 0x1008;
		cfg->power_gpio = 0x11f1;
		cfg->diag_gpio = 0x11f0;
		cfg->diag_gpio_disabled = 0x11f1;
		cfg->usb_gpio = 0x11f6;
		cfg->usb_gpio1 = 0x11f7;
		cfg->sec_gpio = 0x0011;
		cfg->wlan_gpio = 0x1005;
		break;
	case ROUTER_R6850:
		cfg->power_gpio = 0x1012;
		cfg->usb_gpio = 0x100f;
		cfg->diag_gpio = 0x0012;
		cfg->diag_gpio_disabled = 0x1012;
		cfg->connected_gpio = 0x100d;
		break;
	case ROUTER_R6220:
		cfg->power_gpio = 0x1012;
		cfg->usb_gpio = 0x100f;
		cfg->diag_gpio = 0x0012;
		cfg->diag_gpio_disabled = 0x1012;
		cfg->connected_gpio = 0x100d;
		cfg->sec_gpio = 0x100c;
		cfg->usb_power = 0x000a;
		break;
	case ROUTER_DIR882:
		cfg->connected_gpio = 0x1003;
		cfg->disconnected_gpio = 0x1004;
		cfg->diag_gpio = 0x1008;
		cfg->power_gpio = 0x1010;
		cfg->usb_gpio = 0x100c;
		cfg->usb_gpio1 = 0x100e;
		break;
	case ROUTER_DIR860LB1:
		cfg->power_gpio = 0x100f;
		cfg->diag_gpio = 0x100d;
		cfg->diag_gpio_disabled = 0x100f;
		cfg->disconnected_gpio = 0x100e;
		cfg->connected_gpio = 0x1010;
		break;
	case ROUTER_DIR810L:
		cfg->power_gpio = 0x0009;
		cfg->diag_gpio = 0x000d;
		cfg->diag_gpio_disabled = 0x0009;
		cfg->connected_gpio = 0x1028;
		cfg->disconnected_gpio = 0x000c;
		break;
	case ROUTER_WHR300HP2:
		cfg->power_gpio = 0x1009;
		cfg->diag_gpio = 0x1007;
		cfg->diag_gpio_disabled = 0x1009;
		cfg->wlan0_gpio = 0x1008;
		cfg->sec0_gpio = 0x100a;
		cfg->ses_gpio = 0x100a;
		cfg->connected_gpio = 0x1039;
		cfg->disconnected_gpio = 0x103b;
		break;
	case ROUTER_BOARD_WHRG300N:
		cfg->diag_gpio = 0x1007;
		cfg->connected_gpio = 0x1009;
		cfg->ses_gpio = 0x100e;
		break;
#ifdef HAVE_WNR2200
	case ROUTER_BOARD_WHRHPGN:
		cfg->power_gpio = 0x1022;
		cfg->diag_gpio = 0x1021;
		cfg->connected_gpio = 0x1007;
		cfg->usb_power = 0x0024;	// enable usb port 
		cfg->ses_gpio = 0x1005;	//correct state missing
		cfg->usb_gpio = 0x1008;
//              cfg->sec0_gpio = 0x1004;
		break;
#elif HAVE_PERU
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x000c;
		cfg->beeper_gpio = 0x0004;
		break;
#elif HAVE_WNR2000
	case ROUTER_BOARD_WHRHPGN:
		cfg->power_gpio = 0x1023;
		cfg->diag_gpio = 0x1022;
		cfg->connected_gpio = 0x1000;
//              cfg->ses_gpio = 0x1004;
//              cfg->sec0_gpio = 0x1004;
		break;
#elif HAVE_WLAEAG300N
	case ROUTER_BOARD_WHRHPGN:
		cfg->power_gpio = 0x1010;
		cfg->diag_gpio = 0x1011;
		cfg->connected_gpio = 0x1006;
		cfg->ses_gpio = 0x100e;
		cfg->sec0_gpio = 0x100e;
		break;
#elif HAVE_CARAMBOLA
#ifdef HAVE_ERC
	case ROUTER_BOARD_WHRHPGN:
		cfg->vpn_gpio = 0x101B;
		cfg->wlan0_gpio = 0x0000;
		break;
#elif HAVE_FMS2111
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x0013;
		cfg->beeper_gpio = 0x000c;
		break;
#else
	case ROUTER_BOARD_WHRHPGN:
//              cfg->usb_power = 0x001a;
//              cfg->usb_gpio = 0x0001;
//              cfg->ses_gpio = 0x101b;
		break;
#endif
#elif HAVE_HORNET
	case ROUTER_BOARD_WHRHPGN:
		cfg->usb_power = 0x001a;
		cfg->usb_gpio = 0x0001;
		cfg->ses_gpio = 0x101b;
		break;
#elif HAVE_RB2011
	case ROUTER_BOARD_WHRHPGN:
//              cfg->diag_gpio = 0x100f;
//              cfg->connected_gpio = 0x1012;
//              cfg->disconnected_gpio = 0x1013;
//              cfg->power_gpio = 0x100e;
//              cfg->usb_power = 0x001a;
//              cfg->usb_gpio = 0x100b;
//              cfg->ses_gpio = 0x101b;
		break;
#elif HAVE_WDR3500
	case ROUTER_BOARD_WHRHPGN:
		cfg->usb_gpio = 0x100b;
		cfg->usb_power = 0x000f;
		cfg->diag_gpio = 0x100e;
		cfg->connected_gpio = 0x100f;
		break;
#elif HAVE_WDR4300
	case ROUTER_BOARD_WHRHPGN:
		cfg->usb_gpio = 0x100b;
		cfg->usb_gpio1 = 0x100c;
		cfg->usb_power = 0x0015;
		cfg->usb_power1 = 0x0016;
		cfg->diag_gpio = 0x100e;
		cfg->connected_gpio = 0x100f;
		break;
#elif HAVE_WNDR3700V4
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x1002;
		cfg->power_gpio = 0x1000;
		cfg->connected_gpio = 0x1001;
		cfg->disconnected_gpio = 0x1003;
		cfg->usb_power = 0x0020;
		cfg->usb_gpio = 0x100d;
		cfg->ses_gpio = 0x1010;
		break;
#elif HAVE_DAP3662
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x100e;	// red
		cfg->diag_gpio_disabled = 0x1017;	//
		cfg->power_gpio = 0x1017;	// green
		break;
#elif HAVE_DIR862
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x100e;	// orange
		cfg->diag_gpio_disabled = 0x1013;	// 
		cfg->power_gpio = 0x1013;	// green
		cfg->connected_gpio = 0x1016;	// green
		cfg->disconnected_gpio = 0x1017;	// orange
		break;
#elif HAVE_XD9531
	case ROUTER_BOARD_WHRHPGN:
		cfg->connected_gpio = 0x1004;
		cfg->diag_gpio = 0x100D;
		break;
#elif HAVE_CPE880
	case ROUTER_BOARD_WHRHPGN:
		cfg->connected_gpio = 0x1012;
		break;
#elif HAVE_MMS344
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x100e;
		break;
#elif HAVE_WR1043V4
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x0006;
		cfg->ses_gpio = 0x1001;
		cfg->sec0_gpio = 0x1001;
		cfg->usb_gpio = 0x1007;
		cfg->usb_power = 0x0008;

		break;
#elif HAVE_ARCHERC7V5
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x0006;
		cfg->connected_gpio = 0x1015;
		cfg->disconnected_gpio = 0x1014;
		cfg->ses_gpio = 0x1001;
		cfg->sec0_gpio = 0x1001;
		cfg->usb_power = 0x0013;
		cfg->usb_gpio = 0x1007;

		break;
#elif HAVE_ARCHERC7V4
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x0006;
		cfg->connected_gpio = 0x101a;
		cfg->disconnected_gpio = 0x1019;
		cfg->ses_gpio = 0x101f;
		cfg->sec0_gpio = 0x101f;

//              cfg->usb_power = 0x0016;
		cfg->usb_gpio = 0x1007;

//              cfg->usb_power1 = 0x0015;
		cfg->usb_gpio1 = 0x1008;

		break;
#elif HAVE_ARCHERC7
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x100e;
		cfg->ses_gpio = 0x100f;
		cfg->sec0_gpio = 0x100f;

		cfg->usb_power = 0x0016;
		cfg->usb_gpio = 0x1012;

		cfg->usb_power1 = 0x0015;
		cfg->usb_gpio1 = 0x1013;

		break;
#elif HAVE_WR1043V2
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x1013;
//              cfg->connected_gpio = 0x1012;
//              cfg->disconnected_gpio = 0x1013;
//              cfg->power_gpio = 0x100e;
		cfg->usb_power = 0x0015;
		cfg->usb_gpio = 0x100f;
		cfg->ses_gpio = 0x1012;
		cfg->sec0_gpio = 0x1012;
		break;
#elif HAVE_WZR450HP2
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x1014;
//              cfg->connected_gpio = 0x1012;
//              cfg->disconnected_gpio = 0x1013;
//              cfg->power_gpio = 0x100e;
//              cfg->usb_power = 0x001a;
//              cfg->usb_gpio = 0x100b;

		cfg->connected_gpio = 0x100d;
		cfg->power_gpio = 0x1013;
		cfg->ses_gpio = 0x1003;
		cfg->sec0_gpio = 0x1003;
		break;
#elif HAVE_DHP1565
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x100e;
		cfg->diag_gpio_disabled = 0x1016;
		cfg->connected_gpio = 0x1012;
		cfg->disconnected_gpio = 0x1013;
		cfg->power_gpio = 0x1016;
		cfg->usb_gpio = 0x100b;
		cfg->ses_gpio = 0x100f;
		break;
#elif HAVE_E325N
	case ROUTER_BOARD_WHRHPGN:
		cfg->connected_gpio = 0x0003;
		cfg->disconnected_gpio = 0x0002;
		break;
#elif defined(HAVE_SR3200) || defined(HAVE_CPE890)
	case ROUTER_BOARD_WHRHPGN:
		cfg->power_gpio = 0x1001;
		cfg->diag_gpio = 0x0001;
		break;
#elif HAVE_XD3200
	case ROUTER_BOARD_WHRHPGN:
		break;
#elif HAVE_E380AC
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x0003;
		break;
#elif HAVE_WR615N
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x1001;
		cfg->connected_gpio = 0x1002;
		cfg->disconnected_gpio = 0x1003;
		cfg->ses_gpio = 0x100c;
		cfg->sec0_gpio = 0x100c;
		break;
#elif HAVE_E355AC
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x0002;
		break;
#elif HAVE_WR650AC
	case ROUTER_BOARD_WHRHPGN:
		cfg->ses_gpio = 0x1014;
		cfg->sec0_gpio = 0x1014;
		cfg->connected_gpio = 0x1004;
		cfg->diag_gpio = 0x0004;
		break;
#elif HAVE_DIR869
	case ROUTER_BOARD_WHRHPGN:
		cfg->disconnected_gpio = 0x100f;
		cfg->connected_gpio = 0x1010;
		cfg->diag_gpio = 0x000f;
		break;
#elif HAVE_DIR859
	case ROUTER_BOARD_WHRHPGN:
		cfg->power_gpio = 0x100f;
		cfg->connected_gpio = 0x1010;
		cfg->diag_gpio = 0x000f;
		break;
#elif HAVE_JWAP606
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x100b;
		cfg->connected_gpio = 0x100d;
		cfg->disconnected_gpio = 0x100d;
		cfg->power_gpio = 0x100b;
//              cfg->usb_power = 0x001a;
//              cfg->usb_gpio = 0x100b;
//              cfg->ses_gpio = 0x101b;
		break;
#elif HAVE_DIR825C1
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x100f;
		cfg->connected_gpio = 0x1012;
		cfg->disconnected_gpio = 0x1013;
		cfg->power_gpio = 0x100e;
//              cfg->usb_power = 0x001a;
		cfg->usb_gpio = 0x100b;
//              cfg->ses_gpio = 0x101b;
		break;
#elif HAVE_WDR2543
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x1000;
		cfg->usb_gpio = 0x1008;
		break;
#elif HAVE_WASP
	case ROUTER_BOARD_WHRHPGN:
//              cfg->usb_power = 0x001a;
//              cfg->usb_gpio = 0x0001;
//              cfg->ses_gpio = 0x101b;
		break;
#else
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x1001;
		cfg->connected_gpio = 0x1006;
		cfg->ses_gpio = 0x1000;
		cfg->sec0_gpio = 0x1000;
		break;
#endif
	case ROUTER_BUFFALO_WBR54G:
		cfg->diag_gpio = 0x1007;
		break;
	case ROUTER_BUFFALO_WBR2G54S:
		cfg->diag_gpio = 0x0001;
		cfg->ses_gpio = 0x0006;
		break;
	case ROUTER_BUFFALO_WLA2G54C:
		cfg->diag_gpio = 0x1004;
		cfg->ses_gpio = 0x1003;
		break;
	case ROUTER_BUFFALO_WLAH_G54:
		cfg->diag_gpio = 0x1007;
		cfg->ses_gpio = 0x1006;
		break;
	case ROUTER_BUFFALO_WAPM_HP_AM54G54:
		cfg->diag_gpio = 0x1007;
		cfg->ses_gpio = 0x1001;
		break;
	case ROUTER_BOARD_WHRAG108:
		cfg->diag_gpio = 0x1007;
		cfg->bridge_gpio = 0x1004;
		cfg->ses_gpio = 0x1000;
		break;
	case ROUTER_BUFFALO_WHRG54S:
	case ROUTER_BUFFALO_WLI_TX4_G54HP:
		cfg->diag_gpio = 0x1007;
		if (nvram_match("DD_BOARD", "Buffalo WHR-G125")) {
			cfg->connected_gpio = 0x1001;
			cfg->sec0_gpio = 0x1006;
		} else {
			cfg->bridge_gpio = 0x1001;
			cfg->ses_gpio = 0x1006;
		}
		break;
	case ROUTER_UBNT_UNIFIAC:
		cfg->power_gpio = 0x000e;
		cfg->diag_gpio = 0x000f;
		break;
	case ROUTER_D1800H:
		cfg->usb_gpio = 0x1001;
		cfg->usb_power = 0x0007;
		cfg->power_gpio = 0x0002;
		cfg->diag_gpio = 0x000d;
		cfg->diag_gpio_disabled = 0x0002;
		cfg->connected_gpio = 0x100f;
		cfg->disconnected_gpio = 0x100e;
		break;
	case ROUTER_BUFFALO_WZRRSG54:
		cfg->diag_gpio = 0x1007;
		cfg->vpn_gpio = 0x1001;
		cfg->ses_gpio = 0x1006;
		break;
	case ROUTER_BUFFALO_WZRG300N:
		cfg->diag_gpio = 0x1007;
		cfg->bridge_gpio = 0x1001;
		break;
	case ROUTER_BUFFALO_WZRG144NH:
		cfg->diag_gpio = 0x1003;
		cfg->bridge_gpio = 0x1001;
		cfg->ses_gpio = 0x1002;
		break;
	case ROUTER_BUFFALO_WZR900DHP:
	case ROUTER_BUFFALO_WZR600DHP2:
//              cfg->usb_power = 0x0009;      // USB 2.0 ehci port
		cfg->usb_power1 = 0x100a;	// USB 3.0 xhci port
//              cfg->wlan0_gpio = 0x0028; // wireless orange
//              cfg->wlan1_gpio = 0x0029; // wireless blue
		cfg->connected_gpio = 0x002a;	// connected blue
		cfg->sec0_gpio = 0x002b;
		cfg->sec1_gpio = 0x002c;
		// 0x2b strange led orange
		// 0x2c strange led blue
		cfg->power_gpio = 0x002e;
		cfg->diag_gpio = 0x002d;
		cfg->diag_gpio_disabled = 0x002e;
		cfg->usb_gpio = 0x002f;
		break;

	case ROUTER_BUFFALO_WXR1900DHP:
		cfg->usb_power = 0x000d;	// USB 2.0 ehci port
		cfg->usb_power1 = 0x000e;	// USB 3.0 xhci port
//              cfg->wlan0_gpio = 0x0028; // wireless orange
//              cfg->wlan1_gpio = 0x0029; // wireless blue
		cfg->connected_gpio = 0x0009;	// connected blue
		cfg->disconnected_gpio = 0x000a;	// connected blue
		cfg->sec0_gpio = 0x000b;
		cfg->sec1_gpio = 0x000b;
		// 0x2b strange led orange
		// 0x2c strange led blue
		cfg->power_gpio = 0x0006;
		cfg->diag_gpio = 0x0005;
		cfg->diag_gpio_disabled = 0x0006;
		break;

	case ROUTER_BUFFALO_WZR1750:
		cfg->usb_power = 0x0009;	// USB 2.0 ehci port
		cfg->usb_power1 = 0x100a;	// USB 3.0 xhci port
//              cfg->wlan0_gpio = 0x0028; // wireless orange
//              cfg->wlan1_gpio = 0x0029; // wireless blue
		cfg->connected_gpio = 0x002a;	// connected blue
		cfg->sec0_gpio = 0x002b;
		cfg->sec1_gpio = 0x002c;
		// 0x2b strange led orange
		// 0x2c strange led blue
		cfg->power_gpio = 0x002d;
		cfg->diag_gpio = 0x002e;
		cfg->diag_gpio_disabled = 0x002d;
		cfg->usb_gpio = 0x002f;
		break;
#ifndef HAVE_BUFFALO
#ifdef HAVE_DIR300
	case ROUTER_BOARD_FONERA:
		cfg->diag_gpio = 0x0003;
		cfg->bridge_gpio = 0x0004;
		cfg->ses_gpio = 0x0001;
		break;
#endif
#ifdef HAVE_WRT54G2
	case ROUTER_BOARD_FONERA:
		cfg->bridge_gpio = 0x0004;
		cfg->ses_gpio = 0x1004;
		cfg->diag_gpio = 0x1003;
		break;
#endif
#ifdef HAVE_RTG32
	case ROUTER_BOARD_FONERA:
		break;
#endif
#ifdef HAVE_BWRG1000
	case ROUTER_BOARD_LS2:
		cfg->diag_gpio = 0x0007;
		break;
#endif
#ifdef HAVE_DIR400
	case ROUTER_BOARD_FONERA2200:
		cfg->diag_gpio = 0x0003;
		cfg->bridge_gpio = 0x0004;
		cfg->ses_gpio = 0x0001;
		break;
#endif
#ifdef HAVE_WRK54G
	case ROUTER_BOARD_FONERA:
		cfg->diag_gpio = 0x1007;
		cfg->dmz_gpio = 0x0005;
		break;
#endif
	case ROUTER_BOARD_TW6600:
		cfg->diag_gpio = 0x1007;
		cfg->bridge_gpio = 0x1004;
		cfg->ses_gpio = 0x1000;
		break;
	case ROUTER_MOTOROLA:
		cfg->power_gpio = 0x0001;
		cfg->diag_gpio = 0x1001;	// power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_RT210W:
		cfg->power_gpio = 0x1005;
		cfg->diag_gpio = 0x0005;	// power led blink / off to indicate factory
		// defaults
		cfg->connected_gpio = 0x1000;
		cfg->wlan0_gpio = 0x1003;
		break;
	case ROUTER_RT480W:
	case ROUTER_BELKIN_F5D7230_V2000:
	case ROUTER_BELKIN_F5D7231:
		cfg->power_gpio = 0x1005;
		cfg->diag_gpio = 0x0005;	// power led blink / off to indicate factory
		// defaults
		cfg->connected_gpio = 0x1000;
		break;
	case ROUTER_MICROSOFT_MN700:
		cfg->power_gpio = 0x0006;
		cfg->diag_gpio = 0x1006;	// power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_ASUS_WL500GD:
	case ROUTER_ASUS_WL520GUGC:
		cfg->diag_gpio = 0x0000;	// power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_ASUS_WL500G_PRE:
	case ROUTER_ASUS_WL700GE:
		cfg->power_gpio = 0x1001;
		cfg->diag_gpio = 0x0001;	// power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_ASUS_WL550GE:
		cfg->power_gpio = 0x1002;
		cfg->diag_gpio = 0x0002;	// power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_WRT54G3G:
	case ROUTER_WRTSL54GS:
		cfg->power_gpio = 0x0001;
		cfg->dmz_gpio = 0x1000;
		cfg->connected_gpio = 0x1007;	// ses orange
		cfg->ses_gpio = 0x1005;	// ses white
		cfg->ses2_gpio = 0x1007;	// ses orange 
		break;
	case ROUTER_MOTOROLA_WE800G:
	case ROUTER_MOTOROLA_V1:
		cfg->diag_gpio = 0x1003;
		cfg->wlan0_gpio = 0x1001;
		cfg->bridge_gpio = 0x1005;
		break;
	case ROUTER_DELL_TRUEMOBILE_2300:
	case ROUTER_DELL_TRUEMOBILE_2300_V2:
		cfg->power_gpio = 0x1007;
		cfg->diag_gpio = 0x0007;	// power led blink / off to indicate factory
		// defaults
		cfg->wlan0_gpio = 0x1006;
		break;
	case ROUTER_NETGEAR_WNR834B:
		cfg->power_gpio = 0x1004;
		cfg->diag_gpio = 0x1005;
		cfg->wlan0_gpio = 0x1006;
		break;
	case ROUTER_SITECOM_WL105B:
		cfg->power_gpio = 0x0003;
		cfg->diag_gpio = 0x1003;	// power led blink / off to indicate factory
		// defaults
		cfg->wlan0_gpio = 0x1004;
		break;
	case ROUTER_WRT300N:
		cfg->power_gpio = 0x0001;
		cfg->diag_gpio = 0x1001;	// power led blink / off to indicate fac.def.
		break;
	case ROUTER_WRT150N:
		cfg->power_gpio = 0x0001;
		cfg->diag_gpio = 0x1001;	// power led blink / off to indicate fac.def.
		cfg->sec0_gpio = 0x1005;
		break;
	case ROUTER_WRT300NV11:
		cfg->ses_gpio = 0x1005;
		cfg->sec0_gpio = 0x1003;
		// cfg->diag_gpio = 0x101; //power led blink / off to indicate fac.def.
		break;
	case ROUTER_WRT310N:
		cfg->connected_gpio = 0x1003;	//ses orange
		cfg->power_gpio = 0x0001;
		cfg->diag_gpio = 0x1001;	// power led blink / off to indicate fac.def.
		cfg->ses_gpio = 0x1009;	// ses blue
		break;
	case ROUTER_WRT310NV2:
		cfg->connected_gpio = 0x1002;	// ses orange
		cfg->power_gpio = 0x0001;
		cfg->diag_gpio = 0x1001;	// power led blink / off to indicate fac.def.
		cfg->ses_gpio = 0x1004;	// ses blue
		break;
	case ROUTER_WRT160N:
		cfg->power_gpio = 0x0001;
		cfg->diag_gpio = 0x1001;	// power led blink / off to indicate fac.def. 
		cfg->connected_gpio = 0x1003;	// ses orange
		cfg->ses_gpio = 0x1005;	// ses blue
		break;
	case ROUTER_WRT160NV3:
		cfg->power_gpio = 0x0001;
		cfg->diag_gpio = 0x1001;	// power led blink / off to indicate fac.def. 
		cfg->connected_gpio = 0x1002;	// ses orange
		cfg->ses_gpio = 0x1004;	// ses blue
		break;
	case ROUTER_LINKSYS_E800:
	case ROUTER_LINKSYS_E900:
	case ROUTER_LINKSYS_E1500:
	case ROUTER_LINKSYS_E1550:
		cfg->power_gpio = 0x1006;
		cfg->diag_gpio = 0x0006;	// power led blink / off to indicate fac.def.
		cfg->ses_gpio = 0x1008;	// ses blue
		break;
	case ROUTER_LINKSYS_E1000V2:
		cfg->power_gpio = 0x1006;
		cfg->diag_gpio = 0x0006;	// power led blink / off to indicate fac.def. 
		cfg->connected_gpio = 0x0007;	// ses orange
		cfg->ses_gpio = 0x0008;	// ses blue
		break;
	case ROUTER_LINKSYS_E2500:
		cfg->power_gpio = 0x1006;
		cfg->diag_gpio = 0x0006;	// power led blink / off to indicate fac.def.
		break;
	case ROUTER_LINKSYS_E3200:
		cfg->power_gpio = 0x1003;
		cfg->diag_gpio = 0x0003;	// power led blink / off to indicate fac.def. 
		break;
	case ROUTER_LINKSYS_E4200:
		cfg->power_gpio = 0x1005;	// white LED1
		cfg->diag_gpio = 0x1003;	// power led blink / off to indicate fac.def. 
//              cfg->connected_gpio = 0x1003; // white LED2
		break;
	case ROUTER_LINKSYS_EA6500:
		cfg->diag_gpio = 0x1001;	// white led blink / off to indicate fac.def. 
		break;
	case ROUTER_LINKSYS_EA6500V2:
	case ROUTER_LINKSYS_EA6700:
	case ROUTER_LINKSYS_EA6400:
	case ROUTER_LINKSYS_EA6350:
	case ROUTER_LINKSYS_EA6900:
		cfg->usb_power = 0x0009;	//usb power on/off
		cfg->usb_power1 = 0x000a;	//usb power on/off
		cfg->diag_gpio = 0x1006;	// white led blink / off to indicate fac.def. 
		cfg->connected_gpio = 0x0008;
		break;
	case ROUTER_LINKSYS_EA8500:
		cfg->power_gpio = 0x1000;	// power led 
		cfg->diag_gpio = 0x0000;	// power led orange     
		cfg->wlan0_gpio = 0x0001;	// radio 0  
		cfg->ses_gpio = 0x1002;	// wps led
		break;
	case ROUTER_ASUS_WL500G:
		cfg->power_gpio = 0x1000;
		cfg->diag_gpio = 0x0000;	// power led blink /off to indicate factory
		// defaults
		break;
	case ROUTER_ASUS_WL500W:
		cfg->power_gpio = 0x1005;
		cfg->diag_gpio = 0x0005;	// power led blink /off to indicate factory
		// defaults
		break;
	case ROUTER_LINKSYS_WTR54GS:
		cfg->diag_gpio = 0x0001;
		break;
	case ROUTER_WAP54G_V1:
		cfg->diag_gpio = 0x1003;
		cfg->wlan0_gpio = 0x1004;	// LINK led
		break;
	case ROUTER_WAP54G_V3:
		cfg->ses_gpio = 0x100c;
		cfg->connected_gpio = 0x0006;
		break;
	case ROUTER_NETGEAR_WNR834BV2:
		cfg->power_gpio = 0x0002;
		cfg->diag_gpio = 0x0003;	// power led amber 
		cfg->connected_gpio = 0x0007;	// WAN led green 
		break;
	case ROUTER_NETGEAR_WNDR3300:
		cfg->power_gpio = 0x0005;
		cfg->diag_gpio = 0x1005;	// power led blink /off to indicate factory defaults
		cfg->connected_gpio = 0x0007;	// WAN led green 
		break;
	case ROUTER_ASKEY_RT220XD:
		cfg->wlan0_gpio = 0x1000;
		cfg->dmz_gpio = 0x1001;	// not soldered 
		break;
	case ROUTER_WRT610N:
		cfg->power_gpio = 0x0001;
		cfg->diag_gpio = 0x1001;	// power led blink /off to indicate factory defaults
		cfg->connected_gpio = 0x1003;	// ses amber
		cfg->ses_gpio = 0x1009;	// ses blue
		cfg->usb_gpio = 0x1000;
		break;
	case ROUTER_WRT610NV2:
		cfg->power_gpio = 0x0005;
		cfg->diag_gpio = 0x1005;	// power led blink
		cfg->connected_gpio = 0x1000;	// ses amber
		cfg->ses_gpio = 0x1003;	// ses blue
		cfg->usb_gpio = 0x0007;
		break;
	case ROUTER_USR_5461:
		cfg->usb_gpio = 0x0001;
		break;
	case ROUTER_USR_5465:
		//cfg->usb_gpio = 0x0002; //or 0x0001 ??
		break;
	case ROUTER_NETGEAR_WGR614L:
	case ROUTER_NETGEAR_WGR614V9:
		// cfg->power_gpio = 0x1007;       // don't use - resets router
		cfg->diag_gpio = 0x0006;
		cfg->connected_gpio = 0x1004;
		break;
	case ROUTER_NETGEAR_WG602_V4:
		cfg->power_gpio = 0x1001;	// trick: make lan led green for 100Mbps
		break;
	case ROUTER_BELKIN_F5D7231_V2000:
		cfg->connected_gpio = 0x1004;
		cfg->diag_gpio = 0x0001;	// power led blink /off to indicate factory defaults
		break;
	case ROUTER_NETGEAR_WNR3500L:
	case ROUTER_NETGEAR_WNR3500LV2:
		cfg->power_gpio = 0x0003;	// power led green
		cfg->diag_gpio = 0x0007;	// power led amber
		cfg->ses_gpio = 0x0001;	// WPS led green
		cfg->connected_gpio = 0x0002;	// wan led green
		cfg->wlan1_gpio = 0x0000;	// radio 1 blue led
		cfg->usb_gpio = 0x0014;	// usb power
		break;
	case ROUTER_NETGEAR_WNDR3400:
		cfg->power_gpio = 0x0003;	//power led green
		cfg->diag_gpio = 0x0007;	// power led amber
		cfg->connected_gpio = 0x0001;	//wan led green
		cfg->usb_gpio = 0x1002;	//usb led green
		cfg->wlan1_gpio = 0x0000;	// radio 1 led blue
		break;
	case ROUTER_NETGEAR_WNDR4000:
		cfg->power_gpio = 0x0000;	//power led green
		cfg->diag_gpio = 0x0001;	// power led amber
		cfg->connected_gpio = 0x0002;	//wan led green
		cfg->wlan0_gpio = 0x0003;	//radio 0 led green
		cfg->wlan1_gpio = 0x0004;	// radio 1 led blue
		cfg->usb_gpio = 0x0005;	//usb led green
		cfg->ses_gpio = 0x1006;	// WPS led green - inverse
		cfg->ses2_gpio = 0x1007;	// WLAN led green - inverse
		break;
	case ROUTER_DLINK_DIR860:
		cfg->usb_power = 0x000a;
		cfg->connected_gpio = 0x1004;
		cfg->disconnected_gpio = 0x1003;
		cfg->power_gpio = 0x1001;
		cfg->diag_gpio = 0x1000;
		cfg->diag_gpio_disabled = 0x1001;
		break;
	case ROUTER_DLINK_DIR868:
	case ROUTER_DLINK_DIR868C:
		cfg->usb_power = 0x000a;
		cfg->connected_gpio = 0x1003;
		cfg->disconnected_gpio = 0x1001;
		cfg->power_gpio = 0x1002;
		cfg->diag_gpio = 0x1000;
		break;

	case ROUTER_DLINK_DIR880:
		cfg->connected_gpio = 0x1003;
		cfg->disconnected_gpio = 0x1001;
		cfg->power_gpio = 0x1002;
		cfg->diag_gpio = 0x1000;
		cfg->diag_gpio_disabled = 0x1002;
		cfg->usb_gpio = 0x1008;
		cfg->usb_gpio1 = 0x100f;
//              cfg->wlan0_gpio = 0x100d;
//              cfg->wlan1_gpio = 0x100e;
		cfg->usb_power = 0x0009;
		cfg->usb_power1 = 0x000a;
		break;
	case ROUTER_DLINK_DIR885:
		cfg->usb_power = 0x0012;
		cfg->usb_gpio = 0x1008;
		cfg->power_gpio = 0x1000;
		cfg->diag_gpio = 0x1002;
		cfg->diag_gpio_disabled = 0x1000;
		cfg->disconnected_gpio = 0x1003;
		cfg->connected_gpio = 0x1001;
		cfg->wlan0_gpio = 0x100d;
		cfg->wlan1_gpio = 0x100e;
		break;
	case ROUTER_DLINK_DIR895:
		cfg->usb_power = 0x0015;
		cfg->usb_power1 = 0x0012;
		cfg->usb_gpio = 0x1008;
		cfg->usb_gpio1 = 0x100f;
		cfg->power_gpio = 0x1000;
		cfg->diag_gpio = 0x1002;
		cfg->diag_gpio_disabled = 0x1000;
		cfg->disconnected_gpio = 0x1003;
		cfg->connected_gpio = 0x1001;
		cfg->wlan0_gpio = 0x100d;
		cfg->wlan1_gpio = 0x100e;
		break;
	case ROUTER_DLINK_DIR890:
		cfg->usb_power = 0x0015;
		cfg->usb_power1 = 0x0012;
		cfg->usb_gpio = 0x1008;
		cfg->usb_gpio1 = 0x100f;
		cfg->connected_gpio = 0x1001;
		cfg->disconnected_gpio = 0x1003;
		cfg->power_gpio = 0x1002;
		cfg->diag_gpio = 0x0002;
		break;
	case ROUTER_TRENDNET_TEW828:
		cfg->usb_gpio = 0x1004;
		cfg->power_gpio = 0x1006;
		cfg->diag_gpio = 0x0006;
		break;
	case ROUTER_TRENDNET_TEW812:
		// gpio !1 = 2.4 ghz led
		// gpio !2 = 5 ghz led
		// gpio !3 = power somthing
		// gpio !8 = usb led
		// 
		cfg->usb_gpio = 0x1008;
		cfg->diag_gpio = 0x1003;
		cfg->wlan0_gpio = 0x1001;
		cfg->wlan1_gpio = 0x1002;
		break;
	case ROUTER_ASUS_RTN18U:
		cfg->power_gpio = 0x1000;
//              cfg->usb_power = 0x000d;      //usb power on/off
		if (nvram_match("bl_version", "3.0.0.7")) {
			cfg->usb_gpio = 0x100e;
			cfg->connected_gpio = 0x1003;
			cfg->disconnected_gpio = 0x1006;
		} else if (nvram_match("bl_version", "1.0.0.0")) {
			cfg->usb_gpio = 0x1003;
			cfg->connected_gpio = 0x1006;
			cfg->disconnected_gpio = 0x1009;
		} else {
			cfg->usb_gpio = 0x1003;
			cfg->usb_gpio1 = 0x100e;
			cfg->connected_gpio = 0x1006;
			cfg->disconnected_gpio = 0x1009;
		}
		break;
	case ROUTER_TPLINK_ARCHERC8:
		cfg->ses_gpio = 0x0006;
		cfg->usb_gpio = 0x0002;
		cfg->diag_gpio = 0x1004;
//              cfg->usb_gpio1 = 0x0007;
//              cfg->disconnected_gpio = 0x000f;
		cfg->connected_gpio = 0x0008;
		cfg->power_gpio = 0x0004;
//              cfg->diag_gpio = 0x0012;
		cfg->usb_power = 0x000a;	// usb 3
		cfg->usb_power1 = 0x0009;	// usb 2
		break;
	case ROUTER_TPLINK_ARCHERC8_V4:
		cfg->ses_gpio = 0x0002;
		cfg->usb_gpio = 0x0006;
		cfg->usb_gpio1 = 0x0007;
		cfg->disconnected_gpio = 0x000f;
		cfg->connected_gpio = 0x000e;
		cfg->power_gpio = 0x1012;
		cfg->diag_gpio = 0x0012;
		cfg->usb_power = 0x000c;	// usb 3
		cfg->usb_power1 = 0x000d;	// usb 2
		break;
	case ROUTER_TPLINK_ARCHERC9:
		cfg->ses_gpio = 0x0002;
		cfg->usb_gpio = 0x0006;
		cfg->usb_gpio1 = 0x0007;
		cfg->disconnected_gpio = 0x000f;
		cfg->connected_gpio = 0x000e;
		cfg->power_gpio = 0x1012;
		cfg->diag_gpio = 0x0012;
		cfg->usb_power = 0x000c;	// usb 3
		cfg->usb_power1 = 0x000d;	// usb 2
		break;
	case ROUTER_TPLINK_ARCHERC3150:
		cfg->ses_gpio = 0x0002;
//              cfg->usb_gpio = 0x0006;
//              cfg->usb_gpio1 = 0x0007;
//              cfg->disconnected_gpio = 0x000f;
//              cfg->connected_gpio = 0x000e;
//              cfg->power_gpio = 0x1012;
//              cfg->diag_gpio = 0x0012;
		cfg->usb_power = 0x000c;	// usb 3
		cfg->usb_power1 = 0x000d;	// usb 2
		break;
	case ROUTER_ASUS_AC67U:
	case ROUTER_ASUS_AC56U:
		cfg->wlan1_gpio = 0x1006;
		cfg->power_gpio = 0x1003;
		cfg->usb_power = 0x0009;	//usb power on/off
		cfg->usb_power1 = 0x000a;	//usb power on/off
		cfg->usb_gpio = 0x100e;
		cfg->usb_gpio1 = 0x1000;
		cfg->diag_gpio = 0x0003;
		cfg->connected_gpio = 0x1001;
		cfg->disconnected_gpio = 0x1002;
		break;
	case ROUTER_ASUS_AC3200:
		cfg->usb_power = 0x0009;
		cfg->power_gpio = 0x1003;
		cfg->connected_gpio = 0x1005;
		cfg->diag_gpio = 0x0003;
		// wps gpio = 14
		break;
	case ROUTER_ASUS_AC1200:
		cfg->usb_power = 0x100a;
		cfg->diag_gpio = 0x000a;
		cfg->diag_gpio_disabled = 0x100a;
		cfg->usb_gpio = 0x100f;
		break;
	case ROUTER_ASUS_AC88U:
	case ROUTER_ASUS_AC3100:
	case ROUTER_ASUS_AC5300:
		cfg->usb_power = 0x0009;
		cfg->usb_gpio = 0x1010;
		cfg->usb_gpio1 = 0x1011;
		cfg->power_gpio = 0x1003;
		cfg->diag_gpio = 0x0003;
		cfg->connected_gpio = 0x0005;
		cfg->disconnected_gpio = 0x1015;
		cfg->ses_gpio = 0x1013;
		// komisches symbol gpio 21
		// quantenna reset 8 inv (off / on to reset)    
		break;
	case ROUTER_ASUS_AC87U:
		cfg->usb_power = 0x0009;
		cfg->power_gpio = 0x1003;
		cfg->connected_gpio = 0x1005;
		cfg->ses_gpio = 0x1001;
		// quantenna reset 8 inv (off / on to reset)    
		break;
	case ROUTER_NETGEAR_EX6200:
		//cfg->power_gpio = 0x1009;   // connected red
		cfg->diag_gpio = 0x1001;	// Netgear logo 
		cfg->connected_gpio = 0x1008;	// connected green
		cfg->wlan1_gpio = 0x100b;	// radio led red 2.4G
		cfg->wlan0_gpio = 0x100d;	// radio led red 5G
		cfg->usb_gpio = 0x1005;	// usb led 
		//cfg->usb_power = 0x0000;    // usb enable
		break;
	case ROUTER_NETGEAR_AC1450:
		cfg->power_gpio = 0x1002;	// power led green
		//cfg->diag_gpio = 0x1003;    // power led orange
		cfg->diag_gpio = 0x1001;	// Netgear logo 
		cfg->connected_gpio = 0x100a;	// wan led green - hw controlled
		cfg->wlan0_gpio = 0x100b;	// radio led blue
		cfg->usb_gpio = 0x1008;	// usb led 
		//cfg->usb_power = 0x0000;    // usb enable
		break;
	case ROUTER_NETGEAR_R6250:
		cfg->power_gpio = 0x1002;	// power led green
		//cfg->diag_gpio = 0x1003;    // power led orange
		cfg->diag_gpio = 0x0001;	// Netgear logo
		//emblem0_gpio = 0x0001; // NETGEAR Emblem       
		cfg->connected_gpio = 0x100f;	// wan led green
		cfg->wlan0_gpio = 0x100b;	// radio led blue
		cfg->usb_gpio = 0x1008;	// usb led green
		//cfg->usb_power = 0x0000;    // usb enable
		break;
	case ROUTER_NETGEAR_R6300:
		cfg->usb_gpio = 0x1008;	//usb led
		cfg->usb_power = 0x0000;	//usb power on/off
		cfg->connected_gpio = 0x100f;	//green led
		cfg->power_gpio = 0x1002;	//power orange led
		cfg->diag_gpio = 0x1003;	//power led orange
		//cfg->diag_gpio_disabled=0x0009;//netgear logo led r
		//emblem0_gpio = 0x1001;   // NETGEAR Emblem l     
		//emblem1_gpio = 0x1009;   // NETGEAR Emblem r
		cfg->wlan0_gpio = 0x100b;	// radio led blue
		break;
	case ROUTER_NETGEAR_R6300V2:
		cfg->power_gpio = 0x1002;	// power led green
		//cfg->diag_gpio = 0x1003;    // power led orange
		cfg->diag_gpio = 0x1001;	// Netgear logo 
		cfg->connected_gpio = 0x100a;	// wan led green - hw controlled
		cfg->wlan0_gpio = 0x100b;	// radio led blue
		cfg->usb_gpio = 0x1008;	// usb led 
		//cfg->usb_power = 0x0000;    // usb enable
		break;
	case ROUTER_NETGEAR_R6400:
	case ROUTER_NETGEAR_R6400V2:
	case ROUTER_NETGEAR_R6700V3:
		cfg->power_gpio = 0x1001;	// 
		cfg->connected_gpio = 0x1007;	//
		cfg->usb_power = 0x0000;	//
		cfg->diag_gpio = 0x1002;	// 
		cfg->wlan0_gpio = 0x1009;	// radio 0 
		cfg->wlan1_gpio = 0x1008;	// radio 1 
		cfg->ses_gpio = 0x100a;	// wps led
		cfg->wlan_gpio = 0x100b;	// wifi button led
		cfg->usb_gpio = 0x100c;	// usb1 
		cfg->usb_gpio1 = 0x100d;	// usb2
		break;
	case ROUTER_NETGEAR_R7000:
		cfg->power_gpio = 0x1002;	// power led 
		cfg->diag_gpio = 0x1003;	// power led orange     
		cfg->connected_gpio = 0x1009;	// wan led
		cfg->usb_power = 0x0000;	// usb enable
		cfg->wlan0_gpio = 0x100d;	// radio 0 
		cfg->wlan1_gpio = 0x100c;	// radio 1 
		cfg->ses_gpio = 0x100e;	// wps led
		//cfg->wlan_gpio = 0x100f;    // wifi button led
		cfg->usb_gpio = 0x1011;	//usb1 
		cfg->usb_gpio1 = 0x1012;	//usb2 
		break;
	case ROUTER_NETGEAR_R7000P:
		cfg->power_gpio = 0x1002;	// power led *
		cfg->diag_gpio = 0x1003;	// power led orange *    
		cfg->connected_gpio = 0x1008;	// wan led
		//cfg->usb_power = 0x0000;    // usb enable
		cfg->wlan0_gpio = 0x1009;	// radio 0 *
		cfg->wlan1_gpio = 0x100a;	// radio 1 *
		cfg->ses_gpio = 0x100b;	// wps led * //13 is wifi
		//cfg->wlan_gpio = 0x100f;    // wifi button led
		cfg->usb_gpio = 0x100e;	//usb1 *
		cfg->usb_gpio1 = 0x100f;	//usb2 *
		break;
	case ROUTER_NETGEAR_R7500V2:
	case ROUTER_NETGEAR_R7500:
		cfg->power_gpio = 0x0000;	// power led 
		cfg->diag_gpio = 0x000a;	// power led orange     
		cfg->diag_gpio_disabled = 0x0000;	// power led orange     
		cfg->connected_gpio = 0x0007;	// wan led
		cfg->usb_power = 0x0010;	// usb enable
		cfg->usb_power1 = 0x000f;	// usb enable
		cfg->wlan0_gpio = 0x0001;	// radio 0 
		cfg->wlan1_gpio = 0x1002;	// radio 1 
		cfg->ses_gpio = 0x1009;	// wps led
		cfg->wlan_gpio = 0x1008;	// wifi button led
		cfg->usb_gpio = 0x0004;	//usb1 
		cfg->usb_gpio1 = 0x0005;	//usb2 
		break;
	case ROUTER_HABANERO:
#ifdef HAVE_ANTAIRA
		cfg->diag_gpio = 0x00D4;	//gpio 212 on i2c slave antaira-gpio
		cfg->beeper_gpio = 0x00D7;	//gpio 215 on i2c slave antaira-gpio
#endif
		break;
	case ROUTER_NETGEAR_R7800:
		cfg->power_gpio = 0x0000;	// power led 
		cfg->diag_gpio = 0x000a;	// power led orange     
		cfg->diag_gpio_disabled = 0x0000;	// power led orange     
		cfg->connected_gpio = 0x0007;	// wan led
		cfg->usb_power = 0x0010;	// usb enable
		cfg->usb_power1 = 0x000f;
		cfg->wlan0_gpio = 0x0009;	// radio 5G 
		cfg->wlan1_gpio = 0x0008;	// radio 2G
		//cfg->ses_gpio = 0x1009;     // wps button led used for 2G
		//cfg->wlan_gpio = 0x0008;    // wifi button led used for 5G
		cfg->usb_gpio = 0x0004;	//usb1 
		cfg->usb_gpio1 = 0x0005;	//usb2
		break;
	case ROUTER_ASROCK_G10:
		cfg->diag_gpio = 0x0009;	// power led orange     
		cfg->connected_gpio = 0x0008;	// wan led
		cfg->disconnected_gpio = 0x0007;	// wan led
		break;
	case ROUTER_NETGEAR_R9000:

		cfg->power_gpio = 0x0016;	// power led 
		cfg->diag_gpio = 0x1016;	// power led orange     
		cfg->diag_gpio_disabled = 0x0016;	// power led orange     
		cfg->connected_gpio = 0x0017;	// wan led
//      cfg->usb_power = 0x0010;      // usb enable
//      cfg->usb_power1 = 0x000f;
		cfg->ses_gpio = 0x1027;	// wps button led used for 2G
		cfg->usb_gpio = 0x0024;	//usb1 
		cfg->usb_gpio1 = 0x0025;	//usb2
		break;
	case ROUTER_TRENDNET_TEW827:
		cfg->power_gpio = 0x1035;	// power led 
		cfg->usb_gpio = 0x1007;	// usb led
		break;
	case ROUTER_NETGEAR_R8000:
		cfg->power_gpio = 0x1002;	// power led 
		cfg->diag_gpio = 0x1003;	// power led orange     
		cfg->connected_gpio = 0x1009;	// wan led green
		cfg->usb_power = 0x0000;	// usb enable
		cfg->wlan0_gpio = 0x100d;	// radio 2G 
		cfg->wlan1_gpio = 0x100c;	// radio 5G-1 
		cfg->wlan2_gpio = 0x1010;	// radio 5G-2
		cfg->ses_gpio = 0x100e;	// wps led
		cfg->wlan_gpio = 0x100f;	// wifi button led
		cfg->usb_gpio = 0x1011;	//usb1 
		cfg->usb_gpio1 = 0x1012;	//usb2 
		break;
	case ROUTER_NETGEAR_R8500:
		cfg->power_gpio = 0x1002;	// power led 
		cfg->diag_gpio = 0x100f;	//      
		cfg->connected_gpio = 0x1009;	// wan led white 1Gb amber 100Mb
		cfg->usb_power = 0x0000;	// usb enable
		cfg->wlan0_gpio = 0x100b;	// radio 5G-1
		cfg->wlan1_gpio = 0x100d;	// radio 2G 
		cfg->wlan2_gpio = 0x100c;	// radio 5G-2
		cfg->ses_gpio = 0x100e;	// wps led
		cfg->wlan_gpio = 0x0014;	// wifi button led
		cfg->usb_gpio = 0x1011;	//usb1 
		cfg->usb_gpio1 = 0x1012;	//usb2 
		break;
	case ROUTER_NETGEAR_WNDR4500:
	case ROUTER_NETGEAR_WNDR4500V2:
		cfg->power_gpio = 0x1002;	//power led green
		cfg->diag_gpio = 0x1003;	// power led amber
		cfg->connected_gpio = 0x100f;	//wan led green
		cfg->wlan0_gpio = 0x1009;	//radio 0 led green
		cfg->wlan1_gpio = 0x100b;	// radio 1 led blue
		cfg->usb_gpio = 0x1008;	//usb led green
		cfg->usb_gpio1 = 0x100e;	//usb1 led green
		break;
	case ROUTER_ASUS_RTN66:
	case ROUTER_ASUS_AC66U:
		cfg->power_gpio = 0x100c;
		cfg->diag_gpio = 0x000c;
		cfg->usb_gpio = 0x100f;
		break;
	case ROUTER_NETGEAR_WNR2000V2:

		//cfg->power_gpio = ??;
		cfg->diag_gpio = 0x0002;
		cfg->ses_gpio = 0x0007;	//WPS led
		cfg->connected_gpio = 0x0006;
		break;
	case ROUTER_WRT320N:
		cfg->power_gpio = 0x0002;	//power/diag (disabled=blink)
		cfg->ses_gpio = 0x1003;	// ses blue
		cfg->connected_gpio = 0x1004;	//ses orange
		break;
	case ROUTER_ASUS_RTN12:
		cfg->power_gpio = 0x1002;
		cfg->diag_gpio = 0x0002;	// power blink
		break;
	case ROUTER_BOARD_NEPTUNE:
//              cfg->usb_gpio = 0x1008;
		// 0x100c //unknown gpio label, use as diag
#ifdef HAVE_RUT500
		cfg->diag_gpio = 0x100e;
#else
		cfg->diag_gpio = 0x100c;
#endif
		break;
	case ROUTER_ASUS_RTN10U:
		cfg->ses_gpio = 0x0007;
		cfg->usb_gpio = 0x0008;
		break;
	case ROUTER_ASUS_RTN12B:
		cfg->connected_gpio = 0x1005;
		break;
	case ROUTER_ASUS_RTN10PLUSD1:
		cfg->ses_gpio = 0x0007;
		cfg->power_gpio = 0x1006;
		cfg->diag_gpio = 0x0006;
		break;
	case ROUTER_ASUS_RTN10:
	case ROUTER_ASUS_RTN16:
	case ROUTER_NETCORE_NW618:
		cfg->power_gpio = 0x1001;
		cfg->diag_gpio = 0x0001;	// power blink
		break;
	case ROUTER_BELKIN_F7D3301:
	case ROUTER_BELKIN_F7D3302:
	case ROUTER_BELKIN_F7D4301:
	case ROUTER_BELKIN_F7D4302:
		cfg->power_gpio = 0x100a;	// green
		cfg->diag_gpio = 0x100b;	// red
		cfg->ses_gpio = 0x100d;	// wps orange
		break;
	case ROUTER_DYNEX_DX_NRUTER:
		cfg->power_gpio = 0x0001;
		cfg->diag_gpio = 0x1001;	// power blink
		cfg->connected_gpio = 0x1000;
		cfg->sec0_gpio = 0x1003;
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
#if (defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_MAGICBOX)  || (defined(HAVE_RB600) && !defined(HAVE_WDR4900)) || defined(HAVE_MERAKI) || defined(HAVE_LS2) || defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_LS5))  && (!defined(HAVE_DIR300) && !defined(HAVE_WRT54G2) && !defined(HAVE_RTG32) && !defined(HAVE_DIR400) && !defined(HAVE_BWRG1000))
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
		if ((use_gpio & 0xfff) != 0xfff)
			return 1;
		else
			return 0;
	}
	if ((use_gpio & 0xfff) != 0xfff) {
		int gpio_value = use_gpio & 0xfff;
		int enable = (use_gpio & 0x1000) == 0 ? 1 : 0;
		int disable = (use_gpio & 0x1000) == 0 ? 0 : 1;
		int setin = (use_gpio & 0x2000) == 0 ? 0 : 1;
		switch (act) {
		case LED_ON:
			set_gpio(gpio_value, enable);
			if (setin)
				get_gpio(gpio_value);
			break;
		case LED_OFF:
			set_gpio(gpio_value, disable);
			break;
		case LED_FLASH:	// will lit the led for 1 sec.
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

#if defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_XSCALE) || defined(HAVE_LAGUNA) || defined(HAVE_MAGICBOX) || defined(HAVE_RB600) || defined(HAVE_FONERA) || defined(HAVE_MERAKI) || defined(HAVE_LS2) || defined(HAVE_WHRAG108) || defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LS5) || defined(HAVE_FONERA) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_STORM) || defined(HAVE_ADM5120) || defined(HAVE_RT2880) || defined(HAVE_OPENRISC)
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
#if defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_XSCALE)  || defined(HAVE_LAGUNA) || defined(HAVE_MAGICBOX) || defined(HAVE_RB600) || defined(HAVE_FONERA) || defined(HAVE_MERAKI) || defined(HAVE_LS2) || defined(HAVE_WHRAG108) || defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LS5) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_STORM) || defined(HAVE_ADM5120) || defined(HAVE_RT2880) || defined(HAVE_OPENRISC)
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
#if defined(HAVE_IPQ806X) || defined(HAVE_MVEBU) || defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_XSCALE) || defined(HAVE_LAGUNA) || defined(HAVE_MAGICBOX) || defined(HAVE_RB600) || defined(HAVE_FONERA) || defined(HAVE_MERAKI)|| defined(HAVE_LS2) || defined(HAVE_WHRAG108) || defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LS5) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_STORM) || defined(HAVE_ADM5120) || defined(HAVE_RT2880) || defined(HAVE_OPENRISC) || defined(HAVE_ALPINE)
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
	case DIAG:		// GPIO 1
		if (hw_error) {
			write_gpio("/dev/gpio/out", (out & 0x7c) | 0x00);
			return 1;
		}

		if (act == STOP_LED) {	// stop blinking
			write_gpio("/dev/gpio/out", (out & 0x7c) | 0x83);
			// cprintf("tallest:=====( DIAG STOP_LED !!)=====\n");
		} else if (act == START_LED) {	// start blinking
			write_gpio("/dev/gpio/out", (out & 0x7c) | 0x81);
			// cprintf("tallest:=====( DIAG START_LED !!)=====\n");
		} else if (act == MALFUNCTION_LED) {	// start blinking
			write_gpio("/dev/gpio/out", (out & 0x7c) | 0x00);
			hw_error = 1;
			// cprintf("tallest:=====( DIAG MALFUNCTION_LED !!)=====\n");
		}
		break;

	}
	return 1;
#endif
}

static int diag_led_4712(int type, int act)
{

#if defined(HAVE_IPQ806X) || defined(HAVE_MVEBU) || defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_XSCALE) || defined(HAVE_LAGUNA) || defined(HAVE_MAGICBOX) || defined(HAVE_RB600) || defined(HAVE_FONERA)|| defined(HAVE_MERAKI) || defined(HAVE_LS2) || defined(HAVE_WHRAG108) || defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LS5) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_STORM) || defined(HAVE_ADM5120) || defined(HAVE_RT2880) || defined(HAVE_OPENRISC) | defined(HAVE_ALPINE)
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

	if (act == STOP_LED) {	// stop blinking
		// cprintf("%s: Stop GPIO %d\n", __FUNCTION__, type);
		write_gpio("/dev/gpio/out", out | out_mask);
	} else if (act == START_LED) {	// start blinking
		// cprintf("%s: Start GPIO %d\n", __FUNCTION__, type);
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
	else if ((brand == ROUTER_WRTSL54GS || brand == ROUTER_WRT310N || brand == ROUTER_WRT350N || brand == ROUTER_BUFFALO_WZRG144NH) && type == DIAG)
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

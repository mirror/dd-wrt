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

	cfg->power_gpio = 0x0ff;
	cfg->beeper_gpio = 0x0ff;
	cfg->diag_gpio = 0x0ff;
	cfg->diag_gpio_disabled = 0x0ff;
	cfg->dmz_gpio = 0x0ff;
	cfg->connected_gpio = 0x0ff;
	cfg->disconnected_gpio = 0x0ff;
	cfg->bridge_gpio = 0x0ff;
	cfg->vpn_gpio = 0x0ff;
	cfg->ses_gpio = 0x0ff;	// use for SES1 (Linksys), AOSS (Buffalo)
	cfg->ses2_gpio = 0x0ff;
	cfg->wlan_gpio = 0x0ff;	// wlan button led R7000
	cfg->wlan0_gpio = 0x0ff;	// use this only if wlan led is not controlled by hardware!
	cfg->wlan1_gpio = 0x0ff;
	cfg->wlan2_gpio = 0x0ff;
	cfg->usb_gpio = 0x0ff;
	cfg->usb_gpio1 = 0x0ff;
	cfg->sec0_gpio = 0x0ff;	// security leds, wrt600n
	cfg->sec1_gpio = 0x0ff;
	cfg->usb_power = 0x0ff;
	cfg->usb_power1 = 0x0ff;
	cfg->v1func = 0;
	cfg->connblue = nvram_matchi("connblue", 1) ? 1 : 0;

	switch (getRouterBrand())	// gpio definitions here: 0xYZ,
		// Y=0:normal, Y=1:inverted, Z:gpio
		// number (f=disabled)
	{
#ifndef HAVE_BUFFALO
	case ROUTER_BOARD_TECHNAXX3G:
		cfg->usb_gpio = 0x109;
		cfg->diag_gpio = 0x10c;
		cfg->connected_gpio = 0x10b;
		cfg->ses_gpio = 0x10c;
		break;
#ifdef HAVE_WPE72
	case ROUTER_BOARD_NS5M:
		cfg->diag_gpio = 0x10d;
		break;
#endif
	case ROUTER_BOARD_UNIFI:
		cfg->ses_gpio = 0x001;
		cfg->sec0_gpio = 0x001;
		break;
	case ROUTER_BOARD_UNIFI_V2:
		cfg->connected_gpio = 0x00d;
		break;
	case ROUTER_UBNT_UAPAC:
	case ROUTER_UBNT_UAPACPRO:
		cfg->ses_gpio = 0x007;
		cfg->sec0_gpio = 0x007;
		break;
	case ROUTER_BOARD_AIRROUTER:
		cfg->power_gpio = 0x10b;
		cfg->diag_gpio = 0x00b;
		cfg->connected_gpio = 0x100;
		break;
	case ROUTER_BOARD_DANUBE:
#ifdef HAVE_WMBR_G300NH
		cfg->diag_gpio = 0x105;
		cfg->ses_gpio = 0x10e;
		cfg->sec0_gpio = 0x10e;
		cfg->connected_gpio = 0x111;
		cfg->disconnected_gpio = 0x112;
		cfg->power_gpio = 0x101;
#endif
#ifdef HAVE_SX763
//              cfg->diag_gpio = 0x105;
//              cfg->ses_gpio = 0x10e;
//              cfg->sec0_gpio = 0x10e;
		cfg->connected_gpio = 0x1de;
//              cfg->disconnected_gpio = 0x112;
//              cfg->power_gpio = 0x101;
#endif
		break;
#ifdef HAVE_UNIWIP
	case ROUTER_BOARD_UNIWIP:
		break;
#endif
#ifdef HAVE_WDR4900
	case ROUTER_BOARD_WDR4900:
		cfg->diag_gpio = 0x000;
		cfg->usb_gpio = 0x001;
		cfg->usb_gpio1 = 0x002;
		cfg->usb_power = 0x103;
		break;
#endif
#ifdef HAVE_WRT1900AC
	case ROUTER_WRT_1200AC:
	case ROUTER_WRT_1900ACS:

	case ROUTER_WRT_1900ACV2:
		cfg->usb_power = 0x032;
	case ROUTER_WRT_1900AC:
		cfg->power_gpio = 0x000;
		cfg->diag_gpio = 0x100;
		cfg->connected_gpio = 0x006;
		cfg->disconnected_gpio = 0x007;
//              cfg->usb_gpio = 0x004;
//              cfg->usb_gpio1 = 0x005;
		cfg->ses_gpio = 0x009;
		break;
	case ROUTER_WRT_3200ACM:
	case ROUTER_WRT_32X:
//              cfg->usb_power = 0x02f;
		cfg->power_gpio = 0x000;
		cfg->diag_gpio = 0x100;
		cfg->connected_gpio = 0x006;
		cfg->disconnected_gpio = 0x007;
//              cfg->usb_gpio = 0x004;
//              cfg->usb_gpio1 = 0x005;
		cfg->ses_gpio = 0x009;
		break;
#endif
	case ROUTER_BOARD_PB42:
#ifdef HAVE_WA901
		cfg->diag_gpio = 0x102;
		cfg->ses_gpio = 0x004;
//              cfg->usb_gpio = 0x101;
#elif  HAVE_WR941
		cfg->diag_gpio = 0x102;
		cfg->ses_gpio = 0x005;
//              cfg->usb_gpio = 0x101;
#endif
#ifdef HAVE_MR3020
		cfg->connected_gpio = 0x11b;
		cfg->diag_gpio = 0x11a;
		cfg->usb_power = 0x008;
#elif HAVE_GL150
//              cfg->power_gpio = 0x11b;
//              cfg->diag_gpio = 0x01b;
//              cfg->usb_power = 0x008;
#elif HAVE_WR710
		cfg->power_gpio = 0x11b;
		cfg->diag_gpio = 0x01b;
#elif HAVE_WA701V2
		cfg->diag_gpio = 0x11b;
		cfg->ses_gpio = 0x001;
		cfg->sec0_gpio = 0x001;
#elif HAVE_WR703
		cfg->diag_gpio = 0x11b;
		cfg->ses_gpio = 0x001;
		cfg->sec0_gpio = 0x001;
		cfg->usb_power = 0x008;
#elif HAVE_WR842
		cfg->diag_gpio = 0x101;
		cfg->ses_gpio = 0x000;
		cfg->usb_power = 0x006;

#elif HAVE_WR741V4
		cfg->diag_gpio = 0x11b;
		cfg->ses_gpio = 0x001;
		cfg->sec0_gpio = 0x001;

#elif HAVE_MR3420
		cfg->diag_gpio = 0x101;
		cfg->connected_gpio = 0x108;
		cfg->usb_power = 0x006;
#elif HAVE_WR741
		cfg->diag_gpio = 0x101;
		cfg->ses_gpio = 0x000;
//              cfg->usb_gpio = 0x101;
#endif
#ifdef HAVE_WR1043
		cfg->diag_gpio = 0x102;
		cfg->ses_gpio = 0x005;
//              cfg->usb_gpio = 0x101;
#endif
#ifdef HAVE_WRT160NL
		cfg->power_gpio = 0x10e;
		cfg->connected_gpio = 0x109;
		cfg->ses_gpio = 0x108;
#endif
#ifdef HAVE_TG2521
		cfg->ses_gpio = 0x103;
		cfg->diag_gpio = 0x103;
		cfg->usb_power = 0x105;
#endif
#ifdef HAVE_TEW632BRP
		cfg->diag_gpio = 0x101;
		cfg->ses_gpio = 0x103;
#endif
#ifdef HAVE_WP543
		cfg->diag_gpio = 0x107;
		cfg->connected_gpio = 0x106;
#endif
#ifdef HAVE_WP546
		cfg->beeper_gpio = 0x001;
		cfg->diag_gpio = 0x107;
		cfg->connected_gpio = 0x106;
#endif
#ifdef HAVE_DIR825
		cfg->power_gpio = 0x102;
		cfg->diag_gpio = 0x101;
		cfg->connected_gpio = 0x10b;
		cfg->disconnected_gpio = 0x106;
		cfg->ses_gpio = 0x104;
		cfg->usb_gpio = 0x100;
//              cfg->wlan0_gpio = 0x0ff; //correct states missing
#endif
#ifdef HAVE_WNDR3700
		cfg->power_gpio = 0x102;
		cfg->diag_gpio = 0x101;
		cfg->connected_gpio = 0x106;
		cfg->ses_gpio = 0x104;
#endif
#ifdef HAVE_WZRG300NH
		cfg->diag_gpio = 0x101;
		cfg->connected_gpio = 0x112;
		cfg->ses_gpio = 0x111;
		cfg->sec0_gpio = 0x111;
#endif
#ifdef HAVE_DIR632
		cfg->power_gpio = 0x001;
		cfg->diag_gpio = 0x100;
		cfg->connected_gpio = 0x111;
		cfg->usb_gpio = 0x10b;
#endif
#ifdef HAVE_WZRG450
		cfg->diag_gpio = 0x10e;
		cfg->ses_gpio = 0x10d;
		cfg->sec0_gpio = 0x10d;
		cfg->usb_power = 0x010;
		cfg->connected_gpio = 0x12e;	// card 1, gpio 14
#endif
#ifdef HAVE_WZRG300NH2
		cfg->diag_gpio = 0x110;
		cfg->ses_gpio = 0x126;	// card 1, gpio 6
		cfg->sec0_gpio = 0x126;
		cfg->usb_power = 0x00d;
		cfg->connected_gpio = 0x127;	// card 1, gpio 7
#endif
#ifdef HAVE_WZRHPAG300NH
		cfg->diag_gpio = 0x101;
		cfg->connected_gpio = 0x133;	// card 2 gpio 3
		cfg->sec0_gpio = 0x125;
		cfg->sec1_gpio = 0x131;
		cfg->ses_gpio = 0x125;	// card 1 gpio 5
		cfg->ses2_gpio = 0x131;	// card 2 gpio 5
		cfg->usb_power = 0x002;
#endif
#ifdef HAVE_DIR615C1
		cfg->power_gpio = 0x104;
		cfg->wlan0_gpio = 0x10f;
#endif
#ifdef HAVE_DIR615E
		cfg->power_gpio = 0x006;
		cfg->diag_gpio = 0x001;
		cfg->connected_gpio = 0x111;
		cfg->disconnected_gpio = 0x007;
		cfg->ses_gpio = 0x100;
#endif
#ifdef HAVE_DAP2230
		cfg->diag_gpio = 0x00b;
		cfg->power_gpio = 0x10b;
#elif HAVE_LIMA
//              cfg->disconnected_gpio = 0x00f;
//              cfg->power_gpio = 0x105;
//              cfg->diag_gpio = 0x005;
#elif HAVE_RAMBUTAN
//              cfg->disconnected_gpio = 0x00f;
//              cfg->power_gpio = 0x105;
//              cfg->diag_gpio = 0x005;
#elif HAVE_WR940V6
		cfg->diag_gpio = 0x00f;
#elif HAVE_WR940V4
		cfg->disconnected_gpio = 0x00f;
		cfg->power_gpio = 0x105;
		cfg->diag_gpio = 0x005;

#elif HAVE_WR941V6
		cfg->disconnected_gpio = 0x00f;
		cfg->power_gpio = 0x112;
		cfg->diag_gpio = 0x012;

#elif HAVE_WR841V12
		cfg->power_gpio = 0x101;
		cfg->diag_gpio = 0x001;
		cfg->ses_gpio = 0x103;
		cfg->sec0_gpio = 0x103;
		cfg->connected_gpio = 0x102;
#elif HAVE_WR841V11
		cfg->power_gpio = 0x101;
		cfg->diag_gpio = 0x001;
		cfg->ses_gpio = 0x103;
		cfg->sec0_gpio = 0x103;
		cfg->connected_gpio = 0x102;
#elif HAVE_ARCHERC25
		cfg->power_gpio = 0x111;
		cfg->diag_gpio = 0x011;
		cfg->ses_gpio = 0x102;
		cfg->sec0_gpio = 0x102;
		cfg->connected_gpio = 0x07d;
		cfg->disconnected_gpio = 0x07c;
#elif HAVE_WR841V9
		cfg->diag_gpio = 0x103;
#elif HAVE_WR842V2
		cfg->connected_gpio = 0x10e;
		cfg->usb_power = 0x204;
		cfg->usb_gpio = 0x10f;
#elif HAVE_WR810N
		cfg->diag_gpio = 0x10d;
		cfg->usb_power = 0x00b;
#elif HAVE_WR841V8
		cfg->diag_gpio = 0x10f;
		cfg->connected_gpio = 0x10e;
#elif HAVE_DIR615I
		cfg->power_gpio = 0x00e;
		cfg->diag_gpio = 0x10f;
		cfg->connected_gpio = 0x10c;
		cfg->disconnected_gpio = 0x016;
#endif
#ifdef HAVE_WRT400
		cfg->power_gpio = 0x001;
		cfg->diag_gpio = 0x105;
		cfg->ses_gpio = 0x104;
		cfg->connected_gpio = 0x007;
#endif
#ifdef HAVE_ALFAAP94
		cfg->power_gpio = 0x005;
#endif
		break;
	case ROUTER_ALLNET01:
		cfg->connected_gpio = 0x100;
		break;
	case ROUTER_BOARD_WP54G:
		cfg->diag_gpio = 0x102;
		cfg->connected_gpio = 0x107;
		break;
	case ROUTER_BOARD_NP28G:
		cfg->diag_gpio = 0x102;
		cfg->connected_gpio = 0x106;
		break;
	case ROUTER_BOARD_GATEWORX_GW2369:
		cfg->connected_gpio = 0x102;
		break;
	case ROUTER_BOARD_GW2388:
	case ROUTER_BOARD_GW6400:
	case ROUTER_BOARD_GW2380:
#ifdef HAVE_NEWPORT

#elif defined(HAVE_VENTANA)
		cfg->power_gpio = 0x166;
		cfg->diag_gpio = 0x06F;
		cfg->connected_gpio = 0x066;
		cfg->disconnected_gpio = 0x067;
#else
		cfg->connected_gpio = 0x110;	// 16 is mapped to front led
#endif
		break;
	case ROUTER_BOARD_GATEWORX:
#ifdef HAVE_WG302V1
		cfg->diag_gpio = 0x104;
		cfg->wlan0_gpio = 0x105;
#elif HAVE_WG302
		cfg->diag_gpio = 0x102;
		cfg->wlan0_gpio = 0x104;
#else
		if (nvram_match("DD_BOARD", "Gateworks Cambria GW2350"))
			cfg->connected_gpio = 0x105;
		else if (nvram_match("DD_BOARD", "Gateworks Cambria GW2358-4"))
			cfg->connected_gpio = 0x118;
		else
			cfg->connected_gpio = 0x003;
#endif
		break;
	case ROUTER_BOARD_GATEWORX_SWAP:
		cfg->connected_gpio = 0x004;
		break;
	case ROUTER_BOARD_STORM:
		cfg->connected_gpio = 0x005;
		cfg->diag_gpio = 0x003;
		break;
	case ROUTER_LINKSYS_WRH54G:
		cfg->diag_gpio = 0x101;	// power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_WRT54G:
	case ROUTER_WRT54G_V8:
		cfg->power_gpio = 0x001;
		cfg->dmz_gpio = 0x107;
		cfg->connected_gpio = 0x103;	// ses orange
		cfg->ses_gpio = 0x102;	// ses white
		cfg->ses2_gpio = 0x103;	// ses orange
		break;
	case ROUTER_WRT54G_V81:
		cfg->power_gpio = 0x101;
		cfg->dmz_gpio = 0x102;
		cfg->connected_gpio = 0x104;	// ses orange
		cfg->ses_gpio = 0x103;	// ses white
		cfg->ses2_gpio = 0x104;	// ses orange
		break;
	case ROUTER_WRT54G1X:
		cfg->connected_gpio = 0x103;
		cfg->v1func = 1;
		break;
	case ROUTER_WRT350N:
		cfg->connected_gpio = 0x103;
		cfg->power_gpio = 0x001;
		cfg->ses2_gpio = 0x103;	// ses orange
		cfg->sec0_gpio = 0x109;
		cfg->usb_gpio = 0x10b;
		break;
	case ROUTER_WRT600N:
		cfg->power_gpio = 0x102;
		cfg->diag_gpio = 0x002;
		cfg->usb_gpio = 0x103;
		cfg->sec0_gpio = 0x109;
		cfg->sec1_gpio = 0x10b;
		break;
	case ROUTER_LINKSYS_WRT55AG:
		cfg->connected_gpio = 0x103;
		break;
	case ROUTER_DLINK_DIR330:
		cfg->diag_gpio = 0x106;
		cfg->connected_gpio = 0x100;
		cfg->usb_gpio = 0x104;
		break;
	case ROUTER_ASUS_RTN10PLUS:
//              cfg->diag_gpio = 0x10d;
//              cfg->connected_gpio = 0x108;
//              cfg->power_gpio = 0x109;
		break;
	case ROUTER_BOARD_DIR600B:
		cfg->diag_gpio = 0x10d;
		cfg->connected_gpio = 0x108;
		cfg->power_gpio = 0x109;
		break;
	case ROUTER_BOARD_DIR615D:
#ifdef HAVE_DIR615H
		cfg->diag_gpio = 0x007;
		cfg->connected_gpio = 0x10d;
		cfg->disconnected_gpio = 0x10c;
		cfg->ses_gpio = 0x10e;
		cfg->power_gpio = 0x009;
#else
		cfg->diag_gpio = 0x108;
		cfg->connected_gpio = 0x10c;
		cfg->disconnected_gpio = 0x10e;
		cfg->ses_gpio = 0x10b;
		cfg->power_gpio = 0x109;
#endif
		break;
		/*
		   DIR 882 
		   power LED red diag = 8 inv, green 16 inv

		 */
	case ROUTER_BOARD_W502U:
		cfg->connected_gpio = 0x10d;
		break;
	case ROUTER_BOARD_OPENRISC:
#ifndef HAVE_ERC
// ERC: diag button is used different / wlan button is handled by a script
		cfg->diag_gpio = 0x003;
		cfg->ses_gpio = 0x005;
#endif
		break;
	case ROUTER_BOARD_WR5422:
		cfg->ses_gpio = 0x10d;
		break;
	case ROUTER_BOARD_F5D8235:
		cfg->usb_gpio = 0x117;
		cfg->diag_gpio = 0x109;
		cfg->disconnected_gpio = 0x106;
		cfg->connected_gpio = 0x105;
		cfg->ses_gpio = 0x10c;
		break;
#else
	case ROUTER_BOARD_DANUBE:
#ifdef HAVE_WMBR_G300NH
		cfg->diag_gpio = 0x105;
		cfg->ses_gpio = 0x10e;
		cfg->sec0_gpio = 0x10e;
		cfg->connected_gpio = 0x111;
		cfg->disconnected_gpio = 0x112;
		cfg->power_gpio = 0x101;
#endif
		break;
	case ROUTER_BOARD_PB42:
#ifdef HAVE_WZRG300NH
		cfg->diag_gpio = 0x101;
		cfg->connected_gpio = 0x112;
		cfg->ses_gpio = 0x111;
		cfg->sec0_gpio = 0x111;
#endif
#ifdef HAVE_WZRHPAG300NH
		cfg->diag_gpio = 0x101;
		cfg->connected_gpio = 0x133;
		cfg->ses_gpio = 0x125;
		cfg->ses2_gpio = 0x131;
		cfg->sec0_gpio = 0x125;
		cfg->sec1_gpio = 0x131;
		cfg->usb_power = 0x002;
#endif
#ifdef HAVE_WZRG450
		cfg->diag_gpio = 0x10e;
		cfg->ses_gpio = 0x10d;
		cfg->sec0_gpio = 0x10d;
		cfg->usb_power = 0x010;
		cfg->connected_gpio = 0x12e;	// card 1, gpio 14
#endif
#ifdef HAVE_WZRG300NH2
		cfg->diag_gpio = 0x110;
		cfg->ses_gpio = 0x126;
		cfg->sec0_gpio = 0x126;
		cfg->usb_power = 0x00d;
		cfg->connected_gpio = 0x127;
#endif
		break;
#endif
	case ROUTER_BOARD_HAMEA15:
		cfg->diag_gpio = 0x111;
		cfg->connected_gpio = 0x114;
//              cfg->ses_gpio = 0x10e;
		break;
	case ROUTER_BOARD_WCRGN:
		cfg->diag_gpio = 0x107;
		cfg->connected_gpio = 0x10b;
//              cfg->ses_gpio = 0x10e;
		break;
	case ROUTER_DIR882:
		cfg->connected_gpio = 0x103;
		cfg->disconnected_gpio = 0x104;
		cfg->diag_gpio = 0x108;
		cfg->power_gpio = 0x110;
		cfg->usb_gpio = 0x10c;
		cfg->usb_gpio1 = 0x10e;
		break;
	case ROUTER_DIR860LB1:
		cfg->power_gpio = 0x10f;
		cfg->diag_gpio = 0x10d;
		cfg->diag_gpio_disabled = 0x10f;
		cfg->disconnected_gpio = 0x10e;
		cfg->connected_gpio = 0x110;
		break;
	case ROUTER_DIR810L:
		cfg->power_gpio = 0x009;
		cfg->diag_gpio = 0x00d;
		cfg->diag_gpio_disabled = 0x009;
		cfg->connected_gpio = 0x128;
		cfg->disconnected_gpio = 0x00c;
		break;
	case ROUTER_WHR300HP2:
		cfg->power_gpio = 0x109;
		cfg->diag_gpio = 0x107;
		cfg->diag_gpio_disabled = 0x109;
		cfg->wlan0_gpio = 0x108;
		cfg->sec0_gpio = 0x10a;
		cfg->ses_gpio = 0x10a;
		cfg->connected_gpio = 0x139;
		cfg->disconnected_gpio = 0x13b;
		break;
	case ROUTER_BOARD_WHRG300N:
		cfg->diag_gpio = 0x107;
		cfg->connected_gpio = 0x109;
		cfg->ses_gpio = 0x10e;
		break;
#ifdef HAVE_WNR2200
	case ROUTER_BOARD_WHRHPGN:
		cfg->power_gpio = 0x122;
		cfg->diag_gpio = 0x121;
		cfg->connected_gpio = 0x107;
		cfg->usb_power = 0x024;	// enable usb port 
		cfg->ses_gpio = 0x105;	//correct state missing
		cfg->usb_gpio = 0x108;
//              cfg->sec0_gpio = 0x104;
		break;
#elif HAVE_PERU
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x00c;
		cfg->beeper_gpio = 0x004;
		break;
#elif HAVE_WNR2000
	case ROUTER_BOARD_WHRHPGN:
		cfg->power_gpio = 0x123;
		cfg->diag_gpio = 0x122;
		cfg->connected_gpio = 0x100;
//              cfg->ses_gpio = 0x104;
//              cfg->sec0_gpio = 0x104;
		break;
#elif HAVE_WLAEAG300N
	case ROUTER_BOARD_WHRHPGN:
		cfg->power_gpio = 0x110;
		cfg->diag_gpio = 0x111;
		cfg->connected_gpio = 0x106;
		cfg->ses_gpio = 0x10e;
		cfg->sec0_gpio = 0x10e;
		break;
#elif HAVE_CARAMBOLA
#ifdef HAVE_ERC
	case ROUTER_BOARD_WHRHPGN:
		cfg->vpn_gpio = 0x11B;
		cfg->wlan0_gpio = 0x000;
		break;
#elif HAVE_FMS2111
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x013;
		cfg->beeper_gpio = 0x00c;
		break;
#else
	case ROUTER_BOARD_WHRHPGN:
//              cfg->usb_power = 0x01a;
//              cfg->usb_gpio = 0x001;
//              cfg->ses_gpio = 0x11b;
		break;
#endif
#elif HAVE_HORNET
	case ROUTER_BOARD_WHRHPGN:
		cfg->usb_power = 0x01a;
		cfg->usb_gpio = 0x001;
		cfg->ses_gpio = 0x11b;
		break;
#elif HAVE_RB2011
	case ROUTER_BOARD_WHRHPGN:
//              cfg->diag_gpio = 0x10f;
//              cfg->connected_gpio = 0x112;
//              cfg->disconnected_gpio = 0x113;
//              cfg->power_gpio = 0x10e;
//              cfg->usb_power = 0x01a;
//              cfg->usb_gpio = 0x10b;
//              cfg->ses_gpio = 0x11b;
		break;
#elif HAVE_WDR3500
	case ROUTER_BOARD_WHRHPGN:
		cfg->usb_gpio = 0x10b;
		cfg->usb_power = 0x00f;
		cfg->diag_gpio = 0x10e;
		cfg->connected_gpio = 0x10f;
		break;
#elif HAVE_WDR4300
	case ROUTER_BOARD_WHRHPGN:
		cfg->usb_gpio = 0x10b;
		cfg->usb_gpio1 = 0x10c;
		cfg->usb_power = 0x015;
		cfg->usb_power1 = 0x016;
		cfg->diag_gpio = 0x10e;
		cfg->connected_gpio = 0x10f;
		break;
#elif HAVE_WNDR3700V4
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x102;
		cfg->power_gpio = 0x100;
		cfg->connected_gpio = 0x101;
		cfg->disconnected_gpio = 0x103;
		cfg->usb_power = 0x020;
		cfg->usb_gpio = 0x10d;
		cfg->ses_gpio = 0x110;
		break;
#elif HAVE_DAP3662
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x10e;	// red
		cfg->diag_gpio_disabled = 0x117;	//
		cfg->power_gpio = 0x117;	// green
		break;
#elif HAVE_DIR862
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x10e;	// orange
		cfg->diag_gpio_disabled = 0x113;	// 
		cfg->power_gpio = 0x113;	// green
		cfg->connected_gpio = 0x116;	// green
		cfg->disconnected_gpio = 0x117;	// orange
		break;
#elif HAVE_XD9531
	case ROUTER_BOARD_WHRHPGN:
		cfg->connected_gpio = 0x104;
		cfg->diag_gpio = 0x10D;
		break;
#elif HAVE_CPE880
	case ROUTER_BOARD_WHRHPGN:
		cfg->connected_gpio = 0x112;
		break;
#elif HAVE_MMS344
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x10e;
		break;
#elif HAVE_WR1043V4
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x006;
		cfg->ses_gpio = 0x101;
		cfg->sec0_gpio = 0x101;
		cfg->usb_gpio = 0x107;
		cfg->usb_power = 0x008;

		break;
#elif HAVE_ARCHERC7V5
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x006;
		cfg->connected_gpio = 0x115;
		cfg->disconnected_gpio = 0x114;
		cfg->ses_gpio = 0x101;
		cfg->sec0_gpio = 0x101;
		cfg->usb_power = 0x013;
		cfg->usb_gpio = 0x107;

		break;
#elif HAVE_ARCHERC7V4
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x006;
		cfg->connected_gpio = 0x11a;
		cfg->disconnected_gpio = 0x119;
		cfg->ses_gpio = 0x11f;
		cfg->sec0_gpio = 0x11f;

//              cfg->usb_power = 0x016;
		cfg->usb_gpio = 0x107;

//              cfg->usb_power1 = 0x015;
		cfg->usb_gpio1 = 0x108;

		break;
#elif HAVE_ARCHERC7
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x010e;
		cfg->ses_gpio = 0x10f;
		cfg->sec0_gpio = 0x10f;

		cfg->usb_power = 0x016;
		cfg->usb_gpio = 0x112;

		cfg->usb_power1 = 0x015;
		cfg->usb_gpio1 = 0x113;

		break;
#elif HAVE_WR1043V2
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x113;
//              cfg->connected_gpio = 0x112;
//              cfg->disconnected_gpio = 0x113;
//              cfg->power_gpio = 0x10e;
		cfg->usb_power = 0x015;
		cfg->usb_gpio = 0x10f;
		cfg->ses_gpio = 0x112;
		cfg->sec0_gpio = 0x112;
		break;
#elif HAVE_WZR450HP2
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x114;
//              cfg->connected_gpio = 0x112;
//              cfg->disconnected_gpio = 0x113;
//              cfg->power_gpio = 0x10e;
//              cfg->usb_power = 0x01a;
//              cfg->usb_gpio = 0x10b;

		cfg->connected_gpio = 0x10d;
		cfg->power_gpio = 0x113;
		cfg->ses_gpio = 0x103;
		cfg->sec0_gpio = 0x103;
		break;
#elif HAVE_DHP1565
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x10e;
		cfg->diag_gpio_disabled = 0x116;
		cfg->connected_gpio = 0x112;
		cfg->disconnected_gpio = 0x113;
		cfg->power_gpio = 0x116;
		cfg->usb_gpio = 0x10b;
		cfg->ses_gpio = 0x10f;
		break;
#elif HAVE_E325N
	case ROUTER_BOARD_WHRHPGN:
		cfg->connected_gpio = 0x003;
		cfg->disconnected_gpio = 0x002;
		break;
#elif defined(HAVE_SR3200) || defined(HAVE_CPE890)
	case ROUTER_BOARD_WHRHPGN:
		cfg->power_gpio = 0x101;
		cfg->diag_gpio = 0x001;
		break;
#elif HAVE_XD3200
	case ROUTER_BOARD_WHRHPGN:
		break;
#elif HAVE_E380AC
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x003;
		break;
#elif HAVE_WR615N
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x101;
		cfg->connected_gpio = 0x102;
		cfg->disconnected_gpio = 0x103;
		cfg->ses_gpio = 0x10c;
		cfg->sec0_gpio = 0x10c;
		break;
#elif HAVE_E355AC
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x002;
		break;
#elif HAVE_WR650AC
	case ROUTER_BOARD_WHRHPGN:
		cfg->ses_gpio = 0x114;
		cfg->sec0_gpio = 0x114;
		cfg->connected_gpio = 0x104;
		cfg->diag_gpio = 0x004;
		break;
#elif HAVE_DIR869
	case ROUTER_BOARD_WHRHPGN:
		cfg->disconnected_gpio = 0x10f;
		cfg->connected_gpio = 0x110;
		cfg->diag_gpio = 0x00f;
		break;
#elif HAVE_DIR859
	case ROUTER_BOARD_WHRHPGN:
		cfg->power_gpio = 0x10f;
		cfg->connected_gpio = 0x110;
		cfg->diag_gpio = 0x00f;
		break;
#elif HAVE_JWAP606
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x10b;
		cfg->connected_gpio = 0x10d;
		cfg->disconnected_gpio = 0x10d;
		cfg->power_gpio = 0x10b;
//              cfg->usb_power = 0x01a;
//              cfg->usb_gpio = 0x10b;
//              cfg->ses_gpio = 0x11b;
		break;
#elif HAVE_DIR825C1
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x10f;
		cfg->connected_gpio = 0x112;
		cfg->disconnected_gpio = 0x113;
		cfg->power_gpio = 0x10e;
//              cfg->usb_power = 0x01a;
		cfg->usb_gpio = 0x10b;
//              cfg->ses_gpio = 0x11b;
		break;
#elif HAVE_WDR2543
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x100;
		cfg->usb_gpio = 0x108;
		break;
#elif HAVE_WASP
	case ROUTER_BOARD_WHRHPGN:
//              cfg->usb_power = 0x01a;
//              cfg->usb_gpio = 0x001;
//              cfg->ses_gpio = 0x11b;
		break;
#else
	case ROUTER_BOARD_WHRHPGN:
		cfg->diag_gpio = 0x101;
		cfg->connected_gpio = 0x106;
		cfg->ses_gpio = 0x100;
		cfg->sec0_gpio = 0x100;
		break;
#endif
	case ROUTER_BUFFALO_WBR54G:
		cfg->diag_gpio = 0x107;
		break;
	case ROUTER_BUFFALO_WBR2G54S:
		cfg->diag_gpio = 0x001;
		cfg->ses_gpio = 0x006;
		break;
	case ROUTER_BUFFALO_WLA2G54C:
		cfg->diag_gpio = 0x104;
		cfg->ses_gpio = 0x103;
		break;
	case ROUTER_BUFFALO_WLAH_G54:
		cfg->diag_gpio = 0x107;
		cfg->ses_gpio = 0x106;
		break;
	case ROUTER_BUFFALO_WAPM_HP_AM54G54:
		cfg->diag_gpio = 0x107;
		cfg->ses_gpio = 0x101;
		break;
	case ROUTER_BOARD_WHRAG108:
		cfg->diag_gpio = 0x107;
		cfg->bridge_gpio = 0x104;
		cfg->ses_gpio = 0x100;
		break;
	case ROUTER_BUFFALO_WHRG54S:
	case ROUTER_BUFFALO_WLI_TX4_G54HP:
		cfg->diag_gpio = 0x107;
		if (nvram_match("DD_BOARD", "Buffalo WHR-G125")) {
			cfg->connected_gpio = 0x101;
			cfg->sec0_gpio = 0x106;
		} else {
			cfg->bridge_gpio = 0x101;
			cfg->ses_gpio = 0x106;
		}
		break;
	case ROUTER_UBNT_UNIFIAC:
		cfg->power_gpio = 0x00e;
		cfg->diag_gpio = 0x00f;
		break;
	case ROUTER_D1800H:
		cfg->usb_gpio = 0x101;
		cfg->usb_power = 0x007;
		cfg->power_gpio = 0x002;
		cfg->diag_gpio = 0x00d;
		cfg->diag_gpio_disabled = 0x002;
		cfg->connected_gpio = 0x10f;
		cfg->disconnected_gpio = 0x10e;
		break;
	case ROUTER_BUFFALO_WZRRSG54:
		cfg->diag_gpio = 0x107;
		cfg->vpn_gpio = 0x101;
		cfg->ses_gpio = 0x106;
		break;
	case ROUTER_BUFFALO_WZRG300N:
		cfg->diag_gpio = 0x107;
		cfg->bridge_gpio = 0x101;
		break;
	case ROUTER_BUFFALO_WZRG144NH:
		cfg->diag_gpio = 0x103;
		cfg->bridge_gpio = 0x101;
		cfg->ses_gpio = 0x102;
		break;
	case ROUTER_BUFFALO_WZR900DHP:
	case ROUTER_BUFFALO_WZR600DHP2:
//              cfg->usb_power = 0x009;      // USB 2.0 ehci port
		cfg->usb_power1 = 0x10a;	// USB 3.0 xhci port
//              cfg->wlan0_gpio = 0x028; // wireless orange
//              cfg->wlan1_gpio = 0x029; // wireless blue
		cfg->connected_gpio = 0x02a;	// connected blue
		cfg->sec0_gpio = 0x02b;
		cfg->sec1_gpio = 0x02c;
		// 0x2b strange led orange
		// 0x2c strange led blue
		cfg->power_gpio = 0x02e;
		cfg->diag_gpio = 0x02d;
		cfg->diag_gpio_disabled = 0x02e;
		cfg->usb_gpio = 0x02f;
		break;

	case ROUTER_BUFFALO_WXR1900DHP:
		cfg->usb_power = 0x00d;	// USB 2.0 ehci port
		cfg->usb_power1 = 0x00e;	// USB 3.0 xhci port
//              cfg->wlan0_gpio = 0x028; // wireless orange
//              cfg->wlan1_gpio = 0x029; // wireless blue
		cfg->connected_gpio = 0x009;	// connected blue
		cfg->disconnected_gpio = 0x00a;	// connected blue
		cfg->sec0_gpio = 0x00b;
		cfg->sec1_gpio = 0x00b;
		// 0x2b strange led orange
		// 0x2c strange led blue
		cfg->power_gpio = 0x006;
		cfg->diag_gpio = 0x005;
		cfg->diag_gpio_disabled = 0x006;
		break;

	case ROUTER_BUFFALO_WZR1750:
		cfg->usb_power = 0x009;	// USB 2.0 ehci port
		cfg->usb_power1 = 0x10a;	// USB 3.0 xhci port
//              cfg->wlan0_gpio = 0x028; // wireless orange
//              cfg->wlan1_gpio = 0x029; // wireless blue
		cfg->connected_gpio = 0x02a;	// connected blue
		cfg->sec0_gpio = 0x02b;
		cfg->sec1_gpio = 0x02c;
		// 0x2b strange led orange
		// 0x2c strange led blue
		cfg->power_gpio = 0x02d;
		cfg->diag_gpio = 0x02e;
		cfg->diag_gpio_disabled = 0x02d;
		cfg->usb_gpio = 0x02f;
		break;
#ifndef HAVE_BUFFALO
#ifdef HAVE_DIR300
	case ROUTER_BOARD_FONERA:
		cfg->diag_gpio = 0x003;
		cfg->bridge_gpio = 0x004;
		cfg->ses_gpio = 0x001;
		break;
#endif
#ifdef HAVE_WRT54G2
	case ROUTER_BOARD_FONERA:
		cfg->bridge_gpio = 0x004;
		cfg->ses_gpio = 0x104;
		cfg->diag_gpio = 0x103;
		break;
#endif
#ifdef HAVE_RTG32
	case ROUTER_BOARD_FONERA:
		break;
#endif
#ifdef HAVE_BWRG1000
	case ROUTER_BOARD_LS2:
		cfg->diag_gpio = 0x007;
		break;
#endif
#ifdef HAVE_DIR400
	case ROUTER_BOARD_FONERA2200:
		cfg->diag_gpio = 0x003;
		cfg->bridge_gpio = 0x004;
		cfg->ses_gpio = 0x001;
		break;
#endif
#ifdef HAVE_WRK54G
	case ROUTER_BOARD_FONERA:
		cfg->diag_gpio = 0x107;
		cfg->dmz_gpio = 0x005;
		break;
#endif
	case ROUTER_BOARD_TW6600:
		cfg->diag_gpio = 0x107;
		cfg->bridge_gpio = 0x104;
		cfg->ses_gpio = 0x100;
		break;
	case ROUTER_MOTOROLA:
		cfg->power_gpio = 0x001;
		cfg->diag_gpio = 0x101;	// power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_RT210W:
		cfg->power_gpio = 0x105;
		cfg->diag_gpio = 0x005;	// power led blink / off to indicate factory
		// defaults
		cfg->connected_gpio = 0x100;
		cfg->wlan0_gpio = 0x103;
		break;
	case ROUTER_RT480W:
	case ROUTER_BELKIN_F5D7230_V2000:
	case ROUTER_BELKIN_F5D7231:
		cfg->power_gpio = 0x105;
		cfg->diag_gpio = 0x005;	// power led blink / off to indicate factory
		// defaults
		cfg->connected_gpio = 0x100;
		break;
	case ROUTER_MICROSOFT_MN700:
		cfg->power_gpio = 0x006;
		cfg->diag_gpio = 0x106;	// power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_ASUS_WL500GD:
	case ROUTER_ASUS_WL520GUGC:
		cfg->diag_gpio = 0x000;	// power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_ASUS_WL500G_PRE:
	case ROUTER_ASUS_WL700GE:
		cfg->power_gpio = 0x101;
		cfg->diag_gpio = 0x001;	// power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_ASUS_WL550GE:
		cfg->power_gpio = 0x102;
		cfg->diag_gpio = 0x002;	// power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_WRT54G3G:
	case ROUTER_WRTSL54GS:
		cfg->power_gpio = 0x001;
		cfg->dmz_gpio = 0x100;
		cfg->connected_gpio = 0x107;	// ses orange
		cfg->ses_gpio = 0x105;	// ses white
		cfg->ses2_gpio = 0x107;	// ses orange 
		break;
	case ROUTER_MOTOROLA_WE800G:
	case ROUTER_MOTOROLA_V1:
		cfg->diag_gpio = 0x103;
		cfg->wlan0_gpio = 0x101;
		cfg->bridge_gpio = 0x105;
		break;
	case ROUTER_DELL_TRUEMOBILE_2300:
	case ROUTER_DELL_TRUEMOBILE_2300_V2:
		cfg->power_gpio = 0x107;
		cfg->diag_gpio = 0x007;	// power led blink / off to indicate factory
		// defaults
		cfg->wlan0_gpio = 0x106;
		break;
	case ROUTER_NETGEAR_WNR834B:
		cfg->power_gpio = 0x104;
		cfg->diag_gpio = 0x105;
		cfg->wlan0_gpio = 0x106;
		break;
	case ROUTER_SITECOM_WL105B:
		cfg->power_gpio = 0x003;
		cfg->diag_gpio = 0x103;	// power led blink / off to indicate factory
		// defaults
		cfg->wlan0_gpio = 0x104;
		break;
	case ROUTER_WRT300N:
		cfg->power_gpio = 0x001;
		cfg->diag_gpio = 0x101;	// power led blink / off to indicate fac.def.
		break;
	case ROUTER_WRT150N:
		cfg->power_gpio = 0x001;
		cfg->diag_gpio = 0x101;	// power led blink / off to indicate fac.def.
		cfg->sec0_gpio = 0x105;
		break;
	case ROUTER_WRT300NV11:
		cfg->ses_gpio = 0x105;
		cfg->sec0_gpio = 0x103;
		// cfg->diag_gpio = 0x11; //power led blink / off to indicate fac.def.
		break;
	case ROUTER_WRT310N:
		cfg->connected_gpio = 0x103;	//ses orange
		cfg->power_gpio = 0x001;
		cfg->diag_gpio = 0x101;	// power led blink / off to indicate fac.def.
		cfg->ses_gpio = 0x109;	// ses blue
		break;
	case ROUTER_WRT310NV2:
		cfg->connected_gpio = 0x102;	// ses orange
		cfg->power_gpio = 0x001;
		cfg->diag_gpio = 0x101;	// power led blink / off to indicate fac.def.
		cfg->ses_gpio = 0x104;	// ses blue
		break;
	case ROUTER_WRT160N:
		cfg->power_gpio = 0x001;
		cfg->diag_gpio = 0x101;	// power led blink / off to indicate fac.def. 
		cfg->connected_gpio = 0x103;	// ses orange
		cfg->ses_gpio = 0x105;	// ses blue
		break;
	case ROUTER_WRT160NV3:
		cfg->power_gpio = 0x001;
		cfg->diag_gpio = 0x101;	// power led blink / off to indicate fac.def. 
		cfg->connected_gpio = 0x102;	// ses orange
		cfg->ses_gpio = 0x104;	// ses blue
		break;
	case ROUTER_LINKSYS_E800:
	case ROUTER_LINKSYS_E900:
	case ROUTER_LINKSYS_E1500:
	case ROUTER_LINKSYS_E1550:
		cfg->power_gpio = 0x106;
		cfg->diag_gpio = 0x006;	// power led blink / off to indicate fac.def.
		cfg->ses_gpio = 0x108;	// ses blue
		break;
	case ROUTER_LINKSYS_E1000V2:
		cfg->power_gpio = 0x106;
		cfg->diag_gpio = 0x006;	// power led blink / off to indicate fac.def. 
		cfg->connected_gpio = 0x007;	// ses orange
		cfg->ses_gpio = 0x008;	// ses blue
		break;
	case ROUTER_LINKSYS_E2500:
		cfg->power_gpio = 0x106;
		cfg->diag_gpio = 0x006;	// power led blink / off to indicate fac.def.
		break;
	case ROUTER_LINKSYS_E3200:
		cfg->power_gpio = 0x103;
		cfg->diag_gpio = 0x003;	// power led blink / off to indicate fac.def. 
		break;
	case ROUTER_LINKSYS_E4200:
		cfg->power_gpio = 0x105;	// white LED1
		cfg->diag_gpio = 0x103;	// power led blink / off to indicate fac.def. 
//              cfg->connected_gpio = 0x103; // white LED2
		break;
	case ROUTER_LINKSYS_EA6500:
		cfg->diag_gpio = 0x101;	// white led blink / off to indicate fac.def. 
		break;
	case ROUTER_LINKSYS_EA6500V2:
	case ROUTER_LINKSYS_EA6700:
	case ROUTER_LINKSYS_EA6400:
	case ROUTER_LINKSYS_EA6350:
	case ROUTER_LINKSYS_EA6900:
		cfg->usb_power = 0x009;	//usb power on/off
		cfg->usb_power1 = 0x00a;	//usb power on/off
		cfg->diag_gpio = 0x106;	// white led blink / off to indicate fac.def. 
		cfg->connected_gpio = 0x008;
		break;
	case ROUTER_LINKSYS_EA8500:
		cfg->power_gpio = 0x100;	// power led 
		cfg->diag_gpio = 0x000;	// power led orange     
		cfg->wlan0_gpio = 0x001;	// radio 0  
		cfg->ses_gpio = 0x102;	// wps led
		break;
	case ROUTER_ASUS_WL500G:
		cfg->power_gpio = 0x100;
		cfg->diag_gpio = 0x000;	// power led blink /off to indicate factory
		// defaults
		break;
	case ROUTER_ASUS_WL500W:
		cfg->power_gpio = 0x105;
		cfg->diag_gpio = 0x005;	// power led blink /off to indicate factory
		// defaults
		break;
	case ROUTER_LINKSYS_WTR54GS:
		cfg->diag_gpio = 0x001;
		break;
	case ROUTER_WAP54G_V1:
		cfg->diag_gpio = 0x103;
		cfg->wlan0_gpio = 0x104;	// LINK led
		break;
	case ROUTER_WAP54G_V3:
		cfg->ses_gpio = 0x10c;
		cfg->connected_gpio = 0x006;
		break;
	case ROUTER_NETGEAR_WNR834BV2:
		cfg->power_gpio = 0x002;
		cfg->diag_gpio = 0x003;	// power led amber 
		cfg->connected_gpio = 0x007;	// WAN led green 
		break;
	case ROUTER_NETGEAR_WNDR3300:
		cfg->power_gpio = 0x005;
		cfg->diag_gpio = 0x105;	// power led blink /off to indicate factory defaults
		cfg->connected_gpio = 0x007;	// WAN led green 
		break;
	case ROUTER_ASKEY_RT220XD:
		cfg->wlan0_gpio = 0x100;
		cfg->dmz_gpio = 0x101;	// not soldered 
		break;
	case ROUTER_WRT610N:
		cfg->power_gpio = 0x001;
		cfg->diag_gpio = 0x101;	// power led blink /off to indicate factory defaults
		cfg->connected_gpio = 0x103;	// ses amber
		cfg->ses_gpio = 0x109;	// ses blue
		cfg->usb_gpio = 0x100;
		break;
	case ROUTER_WRT610NV2:
		cfg->power_gpio = 0x005;
		cfg->diag_gpio = 0x105;	// power led blink
		cfg->connected_gpio = 0x100;	// ses amber
		cfg->ses_gpio = 0x103;	// ses blue
		cfg->usb_gpio = 0x007;
		break;
	case ROUTER_USR_5461:
		cfg->usb_gpio = 0x001;
		break;
	case ROUTER_USR_5465:
		//cfg->usb_gpio = 0x002; //or 0x001 ??
		break;
	case ROUTER_NETGEAR_WGR614L:
	case ROUTER_NETGEAR_WGR614V9:
		// cfg->power_gpio = 0x107;       // don't use - resets router
		cfg->diag_gpio = 0x006;
		cfg->connected_gpio = 0x104;
		break;
	case ROUTER_NETGEAR_WG602_V4:
		cfg->power_gpio = 0x101;	// trick: make lan led green for 100Mbps
		break;
	case ROUTER_BELKIN_F5D7231_V2000:
		cfg->connected_gpio = 0x104;
		cfg->diag_gpio = 0x001;	// power led blink /off to indicate factory defaults
		break;
	case ROUTER_NETGEAR_WNR3500L:
	case ROUTER_NETGEAR_WNR3500LV2:
		cfg->power_gpio = 0x003;	// power led green
		cfg->diag_gpio = 0x007;	// power led amber
		cfg->ses_gpio = 0x001;	// WPS led green
		cfg->connected_gpio = 0x002;	// wan led green
		cfg->wlan1_gpio = 0x000;	// radio 1 blue led
		cfg->usb_gpio = 0x014;	// usb power
		break;
	case ROUTER_NETGEAR_WNDR3400:
		cfg->power_gpio = 0x003;	//power led green
		cfg->diag_gpio = 0x007;	// power led amber
		cfg->connected_gpio = 0x001;	//wan led green
		cfg->usb_gpio = 0x102;	//usb led green
		cfg->wlan1_gpio = 0x000;	// radio 1 led blue
		break;
	case ROUTER_NETGEAR_WNDR4000:
		cfg->power_gpio = 0x000;	//power led green
		cfg->diag_gpio = 0x001;	// power led amber
		cfg->connected_gpio = 0x002;	//wan led green
		cfg->wlan0_gpio = 0x003;	//radio 0 led green
		cfg->wlan1_gpio = 0x004;	// radio 1 led blue
		cfg->usb_gpio = 0x005;	//usb led green
		cfg->ses_gpio = 0x106;	// WPS led green - inverse
		cfg->ses2_gpio = 0x107;	// WLAN led green - inverse
		break;
	case ROUTER_DLINK_DIR860:
		cfg->usb_power = 0x00a;
		cfg->connected_gpio = 0x104;
		cfg->disconnected_gpio = 0x103;
		cfg->power_gpio = 0x101;
		cfg->diag_gpio = 0x100;
		cfg->diag_gpio_disabled = 0x101;
		break;
	case ROUTER_DLINK_DIR868:
	case ROUTER_DLINK_DIR868C:
		cfg->usb_power = 0x00a;
		cfg->connected_gpio = 0x103;
		cfg->disconnected_gpio = 0x101;
		cfg->power_gpio = 0x102;
		cfg->diag_gpio = 0x100;
		break;

	case ROUTER_DLINK_DIR880:
		cfg->connected_gpio = 0x103;
		cfg->disconnected_gpio = 0x101;
		cfg->power_gpio = 0x102;
		cfg->diag_gpio = 0x100;
		cfg->diag_gpio_disabled = 0x102;
		cfg->usb_gpio = 0x108;
		cfg->usb_gpio1 = 0x10f;
//              cfg->wlan0_gpio = 0x10d;
//              cfg->wlan1_gpio = 0x10e;
		cfg->usb_power = 0x009;
		cfg->usb_power1 = 0x00a;
		break;
	case ROUTER_DLINK_DIR885:
		cfg->usb_power = 0x012;
		cfg->usb_gpio = 0x108;
		cfg->power_gpio = 0x100;
		cfg->diag_gpio = 0x102;
		cfg->diag_gpio_disabled = 0x100;
		cfg->disconnected_gpio = 0x103;
		cfg->connected_gpio = 0x101;
		cfg->wlan0_gpio = 0x10d;
		cfg->wlan1_gpio = 0x10e;
		break;
	case ROUTER_DLINK_DIR895:
		cfg->usb_power = 0x015;
		cfg->usb_power1 = 0x012;
		cfg->usb_gpio = 0x108;
		cfg->usb_gpio1 = 0x10f;
		cfg->power_gpio = 0x100;
		cfg->diag_gpio = 0x102;
		cfg->diag_gpio_disabled = 0x100;
		cfg->disconnected_gpio = 0x103;
		cfg->connected_gpio = 0x101;
		cfg->wlan0_gpio = 0x10d;
		cfg->wlan1_gpio = 0x10e;
		break;
	case ROUTER_DLINK_DIR890:
		cfg->usb_power = 0x015;
		cfg->usb_power1 = 0x012;
		cfg->usb_gpio = 0x108;
		cfg->usb_gpio1 = 0x10f;
		cfg->connected_gpio = 0x101;
		cfg->disconnected_gpio = 0x103;
		cfg->power_gpio = 0x102;
		cfg->diag_gpio = 0x002;
		break;
	case ROUTER_TRENDNET_TEW828:
		cfg->usb_gpio = 0x104;
		cfg->power_gpio = 0x106;
		cfg->diag_gpio = 0x006;
		break;
	case ROUTER_TRENDNET_TEW812:
		// gpio !1 = 2.4 ghz led
		// gpio !2 = 5 ghz led
		// gpio !3 = power somthing
		// gpio !8 = usb led
		// 
		cfg->usb_gpio = 0x108;
		cfg->diag_gpio = 0x103;
		cfg->wlan0_gpio = 0x101;
		cfg->wlan1_gpio = 0x102;
		break;
	case ROUTER_ASUS_RTN18U:
		cfg->power_gpio = 0x100;
//              cfg->usb_power = 0x00d;      //usb power on/off
		if (nvram_match("bl_version", "3.0.0.7")) {
			cfg->usb_gpio = 0x10e;
			cfg->connected_gpio = 0x103;
			cfg->disconnected_gpio = 0x106;
		} else if (nvram_match("bl_version", "1.0.0.0")) {
			cfg->usb_gpio = 0x103;
			cfg->connected_gpio = 0x106;
			cfg->disconnected_gpio = 0x109;
		} else {
			cfg->usb_gpio = 0x103;
			cfg->usb_gpio1 = 0x10e;
			cfg->connected_gpio = 0x106;
			cfg->disconnected_gpio = 0x109;
		}
		break;
	case ROUTER_TPLINK_ARCHERC9:
		cfg->ses_gpio = 0x002;
		cfg->usb_gpio = 0x006;
		cfg->usb_gpio1 = 0x007;
		cfg->disconnected_gpio = 0x00f;
		cfg->connected_gpio = 0x00e;
		cfg->power_gpio = 0x112;
		cfg->diag_gpio = 0x012;
		cfg->usb_power = 0x00c;	// usb 3
		cfg->usb_power1 = 0x00d;	// usb 2
		break;
	case ROUTER_TPLINK_ARCHERC3150:
		cfg->ses_gpio = 0x002;
//              cfg->usb_gpio = 0x006;
//              cfg->usb_gpio1 = 0x007;
//              cfg->disconnected_gpio = 0x00f;
//              cfg->connected_gpio = 0x00e;
//              cfg->power_gpio = 0x112;
//              cfg->diag_gpio = 0x012;
		cfg->usb_power = 0x00c;	// usb 3
		cfg->usb_power1 = 0x00d;	// usb 2
		break;
	case ROUTER_ASUS_AC67U:
	case ROUTER_ASUS_AC56U:
		cfg->wlan1_gpio = 0x106;
		cfg->power_gpio = 0x103;
		cfg->usb_power = 0x009;	//usb power on/off
		cfg->usb_power1 = 0x00a;	//usb power on/off
		cfg->usb_gpio = 0x10e;
		cfg->usb_gpio1 = 0x100;
		cfg->diag_gpio = 0x003;
		cfg->connected_gpio = 0x101;
		cfg->disconnected_gpio = 0x102;
		break;
	case ROUTER_ASUS_AC3200:
		cfg->usb_power = 0x009;
		cfg->power_gpio = 0x103;
		cfg->connected_gpio = 0x105;
		cfg->diag_gpio = 0x003;
		// wps gpio = 14
		break;
	case ROUTER_ASUS_AC1200:
		cfg->usb_power = 0x10a;
		cfg->diag_gpio = 0x00a;
		cfg->diag_gpio_disabled = 0x10a;
		cfg->usb_gpio = 0x10f;
		break;
	case ROUTER_ASUS_AC88U:
	case ROUTER_ASUS_AC3100:
	case ROUTER_ASUS_AC5300:
		cfg->usb_power = 0x009;
		cfg->usb_gpio = 0x110;
		cfg->usb_gpio1 = 0x111;
		cfg->power_gpio = 0x103;
		cfg->diag_gpio = 0x003;
		cfg->connected_gpio = 0x005;
		cfg->disconnected_gpio = 0x115;
		cfg->ses_gpio = 0x113;
		// komisches symbol gpio 21
		// quantenna reset 8 inv (off / on to reset)    
		break;
	case ROUTER_ASUS_AC87U:
		cfg->usb_power = 0x009;
		cfg->power_gpio = 0x103;
		cfg->connected_gpio = 0x105;
		cfg->ses_gpio = 0x101;
		// quantenna reset 8 inv (off / on to reset)    
		break;
	case ROUTER_NETGEAR_EX6200:
		//cfg->power_gpio = 0x109;   // connected red
		cfg->diag_gpio = 0x101;	// Netgear logo 
		cfg->connected_gpio = 0x108;	// connected green
		cfg->wlan1_gpio = 0x10b;	// radio led red 2.4G
		cfg->wlan0_gpio = 0x10d;	// radio led red 5G
		cfg->usb_gpio = 0x105;	// usb led 
		//cfg->usb_power = 0x000;    // usb enable
		break;
	case ROUTER_NETGEAR_AC1450:
		cfg->power_gpio = 0x102;	// power led green
		//cfg->diag_gpio = 0x103;    // power led orange
		cfg->diag_gpio = 0x101;	// Netgear logo 
		cfg->connected_gpio = 0x10a;	// wan led green - hw controlled
		cfg->wlan0_gpio = 0x10b;	// radio led blue
		cfg->usb_gpio = 0x108;	// usb led 
		//cfg->usb_power = 0x000;    // usb enable
		break;
	case ROUTER_NETGEAR_R6250:
		cfg->power_gpio = 0x102;	// power led green
		//cfg->diag_gpio = 0x103;    // power led orange
		cfg->diag_gpio = 0x001;	// Netgear logo
		//emblem0_gpio = 0x001; // NETGEAR Emblem       
		cfg->connected_gpio = 0x10f;	// wan led green
		cfg->wlan0_gpio = 0x10b;	// radio led blue
		cfg->usb_gpio = 0x108;	// usb led green
		//cfg->usb_power = 0x000;    // usb enable
		break;
	case ROUTER_NETGEAR_R6300:
		cfg->usb_gpio = 0x108;	//usb led
		cfg->usb_power = 0x000;	//usb power on/off
		cfg->connected_gpio = 0x10f;	//green led
		cfg->power_gpio = 0x102;	//power orange led
		cfg->diag_gpio = 0x103;	//power led orange
		//cfg->diag_gpio_disabled=0x009;//netgear logo led r
		//emblem0_gpio = 0x101;   // NETGEAR Emblem l     
		//emblem1_gpio = 0x109;   // NETGEAR Emblem r
		cfg->wlan0_gpio = 0x10b;	// radio led blue
		break;
	case ROUTER_NETGEAR_R6300V2:
		cfg->power_gpio = 0x102;	// power led green
		//cfg->diag_gpio = 0x103;    // power led orange
		cfg->diag_gpio = 0x101;	// Netgear logo 
		cfg->connected_gpio = 0x10a;	// wan led green - hw controlled
		cfg->wlan0_gpio = 0x10b;	// radio led blue
		cfg->usb_gpio = 0x108;	// usb led 
		//cfg->usb_power = 0x000;    // usb enable
		break;
	case ROUTER_NETGEAR_R6400:
	case ROUTER_NETGEAR_R6400V2:
	case ROUTER_NETGEAR_R6700V3:
		cfg->power_gpio = 0x101;	// 
		cfg->connected_gpio = 0x107;	//
		cfg->usb_power = 0x000;	//
		cfg->diag_gpio = 0x102;	// 
		cfg->wlan0_gpio = 0x109;	// radio 0 
		cfg->wlan1_gpio = 0x108;	// radio 1 
		cfg->ses_gpio = 0x10a;	// wps led
		cfg->wlan_gpio = 0x10b;	// wifi button led
		cfg->usb_gpio = 0x10c;	// usb1 
		cfg->usb_gpio1 = 0x10d;	// usb2
		break;
	case ROUTER_NETGEAR_R7000:
		cfg->power_gpio = 0x102;	// power led 
		cfg->diag_gpio = 0x103;	// power led orange     
		cfg->connected_gpio = 0x109;	// wan led
		cfg->usb_power = 0x000;	// usb enable
		cfg->wlan0_gpio = 0x10d;	// radio 0 
		cfg->wlan1_gpio = 0x10c;	// radio 1 
		cfg->ses_gpio = 0x10e;	// wps led
		//cfg->wlan_gpio = 0x10f;    // wifi button led
		cfg->usb_gpio = 0x111;	//usb1 
		cfg->usb_gpio1 = 0x112;	//usb2 
		break;
	case ROUTER_NETGEAR_R7000P:
		cfg->power_gpio = 0x102;	// power led *
		cfg->diag_gpio = 0x103;	// power led orange *    
		cfg->connected_gpio = 0x108;	// wan led
		//cfg->usb_power = 0x000;    // usb enable
		cfg->wlan0_gpio = 0x109;	// radio 0 *
		cfg->wlan1_gpio = 0x10a;	// radio 1 *
		cfg->ses_gpio = 0x10b;	// wps led * //13 is wifi
		//cfg->wlan_gpio = 0x10f;    // wifi button led
		cfg->usb_gpio = 0x10e;	//usb1 *
		cfg->usb_gpio1 = 0x10f;	//usb2 *
		break;
	case ROUTER_NETGEAR_R7500V2:
	case ROUTER_NETGEAR_R7500:
		cfg->power_gpio = 0x000;	// power led 
		cfg->diag_gpio = 0x00a;	// power led orange     
		cfg->diag_gpio_disabled = 0x000;	// power led orange     
		cfg->connected_gpio = 0x007;	// wan led
		cfg->usb_power = 0x010;	// usb enable
		cfg->usb_power1 = 0x00f;	// usb enable
		cfg->wlan0_gpio = 0x001;	// radio 0 
		cfg->wlan1_gpio = 0x102;	// radio 1 
		cfg->ses_gpio = 0x109;	// wps led
		cfg->wlan_gpio = 0x108;	// wifi button led
		cfg->usb_gpio = 0x004;	//usb1 
		cfg->usb_gpio1 = 0x005;	//usb2 
		break;
	case ROUTER_NETGEAR_R7800:
		cfg->power_gpio = 0x000;	// power led 
		cfg->diag_gpio = 0x00a;	// power led orange     
		cfg->diag_gpio_disabled = 0x000;	// power led orange     
		cfg->connected_gpio = 0x007;	// wan led
		cfg->usb_power = 0x010;	// usb enable
		cfg->usb_power1 = 0x00f;
		cfg->wlan0_gpio = 0x009;	// radio 5G 
		cfg->wlan1_gpio = 0x008;	// radio 2G
		//cfg->ses_gpio = 0x109;     // wps button led used for 2G
		//cfg->wlan_gpio = 0x008;    // wifi button led used for 5G
		cfg->usb_gpio = 0x004;	//usb1 
		cfg->usb_gpio1 = 0x005;	//usb2
		break;
	case ROUTER_ASROCK_G10:
		cfg->diag_gpio = 0x009;	// power led orange     
		cfg->connected_gpio = 0x008;	// wan led
		cfg->disconnected_gpio = 0x007;	// wan led
		break;
	case ROUTER_NETGEAR_R9000:

		cfg->power_gpio = 0x016;	// power led 
		cfg->diag_gpio = 0x116;	// power led orange     
		cfg->diag_gpio_disabled = 0x016;	// power led orange     
		cfg->connected_gpio = 0x017;	// wan led
//      cfg->usb_power = 0x010;      // usb enable
//      cfg->usb_power1 = 0x00f;
		cfg->ses_gpio = 0x127;	// wps button led used for 2G
		cfg->usb_gpio = 0x024;	//usb1 
		cfg->usb_gpio1 = 0x025;	//usb2
		break;
	case ROUTER_TRENDNET_TEW827:
		cfg->power_gpio = 0x135;	// power led 
		cfg->usb_gpio = 0x107;	// usb led
		break;
	case ROUTER_NETGEAR_R8000:
		cfg->power_gpio = 0x102;	// power led 
		cfg->diag_gpio = 0x103;	// power led orange     
		cfg->connected_gpio = 0x109;	// wan led green
		cfg->usb_power = 0x000;	// usb enable
		cfg->wlan0_gpio = 0x10d;	// radio 2G 
		cfg->wlan1_gpio = 0x10c;	// radio 5G-1 
		cfg->wlan2_gpio = 0x110;	// radio 5G-2
		cfg->ses_gpio = 0x10e;	// wps led
		cfg->wlan_gpio = 0x10f;	// wifi button led
		cfg->usb_gpio = 0x111;	//usb1 
		cfg->usb_gpio1 = 0x112;	//usb2 
		break;
	case ROUTER_NETGEAR_R8500:
		cfg->power_gpio = 0x102;	// power led 
		cfg->diag_gpio = 0x10f;	//      
		cfg->connected_gpio = 0x109;	// wan led white 1Gb amber 100Mb
		cfg->usb_power = 0x000;	// usb enable
		cfg->wlan0_gpio = 0x10b;	// radio 5G-1
		cfg->wlan1_gpio = 0x10d;	// radio 2G 
		cfg->wlan2_gpio = 0x10c;	// radio 5G-2
		cfg->ses_gpio = 0x10e;	// wps led
		cfg->wlan_gpio = 0x014;	// wifi button led
		cfg->usb_gpio = 0x111;	//usb1 
		cfg->usb_gpio1 = 0x112;	//usb2 
		break;
	case ROUTER_NETGEAR_WNDR4500:
	case ROUTER_NETGEAR_WNDR4500V2:
		cfg->power_gpio = 0x102;	//power led green
		cfg->diag_gpio = 0x103;	// power led amber
		cfg->connected_gpio = 0x10f;	//wan led green
		cfg->wlan0_gpio = 0x109;	//radio 0 led green
		cfg->wlan1_gpio = 0x10b;	// radio 1 led blue
		cfg->usb_gpio = 0x108;	//usb led green
		cfg->usb_gpio1 = 0x10e;	//usb1 led green
		break;
	case ROUTER_ASUS_RTN66:
	case ROUTER_ASUS_AC66U:
		cfg->power_gpio = 0x10c;
		cfg->diag_gpio = 0x00c;
		cfg->usb_gpio = 0x10f;
		break;
	case ROUTER_NETGEAR_WNR2000V2:

		//cfg->power_gpio = ??;
		cfg->diag_gpio = 0x002;
		cfg->ses_gpio = 0x007;	//WPS led
		cfg->connected_gpio = 0x006;
		break;
	case ROUTER_WRT320N:
		cfg->power_gpio = 0x002;	//power/diag (disabled=blink)
		cfg->ses_gpio = 0x103;	// ses blue
		cfg->connected_gpio = 0x104;	//ses orange
		break;
	case ROUTER_ASUS_RTN12:
		cfg->power_gpio = 0x102;
		cfg->diag_gpio = 0x002;	// power blink
		break;
	case ROUTER_BOARD_NEPTUNE:
//              cfg->usb_gpio = 0x108;
		// 0x10c //unknown gpio label, use as diag
#ifdef HAVE_RUT500
		cfg->diag_gpio = 0x10e;
#else
		cfg->diag_gpio = 0x10c;
#endif
		break;
	case ROUTER_ASUS_RTN10U:
		cfg->ses_gpio = 0x007;
		cfg->usb_gpio = 0x008;
		break;
	case ROUTER_ASUS_RTN12B:
		cfg->connected_gpio = 0x105;
		break;
	case ROUTER_ASUS_RTN10PLUSD1:
		cfg->ses_gpio = 0x007;
		cfg->power_gpio = 0x106;
		cfg->diag_gpio = 0x006;
		break;
	case ROUTER_ASUS_RTN10:
	case ROUTER_ASUS_RTN16:
	case ROUTER_NETCORE_NW618:
		cfg->power_gpio = 0x101;
		cfg->diag_gpio = 0x001;	// power blink
		break;
	case ROUTER_BELKIN_F7D3301:
	case ROUTER_BELKIN_F7D3302:
	case ROUTER_BELKIN_F7D4301:
	case ROUTER_BELKIN_F7D4302:
		cfg->power_gpio = 0x10a;	// green
		cfg->diag_gpio = 0x10b;	// red
		cfg->ses_gpio = 0x10d;	// wps orange
		break;
	case ROUTER_DYNEX_DX_NRUTER:
		cfg->power_gpio = 0x001;
		cfg->diag_gpio = 0x101;	// power blink
		cfg->connected_gpio = 0x100;
		cfg->sec0_gpio = 0x103;
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
	int use_gpio = 0x0ff;
	static struct ledconfig *cfg = NULL;
	if (!cfg) {
		cfg = &led_cfg;
		getledconfig(cfg);
	}

	switch (type) {
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
	case LED_SEC0:
		use_gpio = cfg->sec0_gpio;
		break;
	case LED_SEC1:
		use_gpio = cfg->sec1_gpio;
		break;
	}

	if ((use_gpio & 0x0ff) != 0x0ff) {
		int gpio_value = use_gpio & 0x0ff;
		int enable = (use_gpio & 0x100) == 0 ? 1 : 0;
		int disable = (use_gpio & 0x100) == 0 ? 0 : 1;
		int setin = (use_gpio & 0x200) == 0 ? 0 : 1;
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

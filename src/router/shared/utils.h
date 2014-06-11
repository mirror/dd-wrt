#ifndef UTILS_H
#define UTILS_H

#ifdef CDEBUG
#include <shutils.h>
#include <malloc.h>
#include <cy_conf.h>
#endif
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <typedefs.h>

/*
 * 2 byte router ID number; Eko 4.jul.06
 * 
 * X X X X X X X X R R R P N N N N = 0xXXXX --------------- ----- - -------
 * router num | | gpio num (1111 = f = disable resetbutton) | | | |--- resetbutton polarity (0: normal, 1 inversed) | |-------- reserved for
 * future use (maybe USB supp) 
 */

// Linksys WRT54G, WRT54GS and WRT54GL all models (v2 - v6) except WRT54G
// v1.0, v1.1 (fccid: Linksys Q87-WRT54G..., Q87_WT54G...)
#define ROUTER_WRT54G 0x0116

// Linksys WRT54Gv8, WRT54GSv7 (BCM5354 cpu, fccid: Linksys Q87-WRT54GV8,
// Q87-WRT54GSV7)
// Linksys WRT54Gv8.2, WRT54GSv7.2 (BCM5354 cpu, fccid: Linksys
// Q87-WRT54GV82, Q87-WRT54GSV72)
// Linksys WRT54Gv7.2 (EMEA only - same hardware as v8)
// Linksys WRT54G2v1 (BCM5354 cpu, fccid: Linksys Q87-WRT54G2V1)
// Linksys WRT54G2v1.3 (BCM5354 cpu, fccid: Linksys Q87-WRT54G2V13)
// Linksys WRT54GS2v1 (BCM5354 cpu, fccid: Linksys Q87-WRT54G2V1)
#define ROUTER_WRT54G_V8 0x0216

// Linksys WRT54G v1.0 and v1.1, Alnet ALL0277 (BCM4702 cpu, fccid: v1:
// Linksys PKW-WM54G; v1.1: Linksys Q87-WRT54G11)
#define ROUTER_WRT54G1X 0x0316

// Linksys WRTSL54GS (BCM4704 cpu, BCM5325E switch, fccid: Linksys
// Q87-WTSLGS, same without USB: Q87-WRTH54GS) 
#define ROUTER_WRTSL54GS 0x0416

// Linksys WRT55AG v1 (BCM4702 cpu, dual minipci (Broadcom + Atheros), fccid: 
// Linksys PKW-WRT55AG)
#define ROUTER_LINKSYS_WRT55AG 0x0516

// Asus WL-500G-Deluxe (BCM5365 cpu), (fccid: Asus MSQWL500GD)
#define ROUTER_ASUS_WL500GD 0x0606

// Buffalo WBR-G54, WLA-G54 (BCM4702 cpu, WLA-G54 has no switch)
#define ROUTER_BUFFALO_WBR54G 0x0704

// Buffalo WBR2-G54 (BCM4712 + ADM6996 switch, fccid: Buffalo
// FDI-04600142-0),
// Buffalo WBR2-G54S, WLA2-G54L (= WLA2-G54 in Japan)
#define ROUTER_BUFFALO_WBR2G54S 0x0807

// Buffalo WLA2-G54C == WLI3-TX1-G54 (BCM4712 cpu, no switch, fccid: Buffalo
// FDI-09101669-0)
#define ROUTER_BUFFALO_WLA2G54C 0x0910

// Buffalo WHR-G54S (BCM5352E cpu, fccid: Buffalo FDI-04600264-0) and
// WHR-HP-G54 (BCM5352E cpu, fccid: Buffalo FDI-09101577-0)
// Buffalo WHR-G125, WHR-HP-G125 (BCM5354 cpu, fccid: Buffalo FDI-09101584-0)
// Buffalo WHR-HP-G125 new (BCM5354 cpu, fccid: FDI-09101588-0)
#define ROUTER_BUFFALO_WHRG54S 0x0a04

// Buffalo WZR-RS-G54 (BCM4704 cpu), WZR-G54, WZR-HP-G54 (4704 cpu, fccid:
// Buffalo FDI-09101457-0), 
// Buffalo WHR3-AG54, WVR-G54-NF, WHR2-A54G54
#define ROUTER_BUFFALO_WZRRSG54 0x0b04

// Motorola WR850G v1 (BCM4702 cpu, fccid: Motorola ACQWR850G) 
#define ROUTER_MOTOROLA_V1 0x0c10

// Motorola WR850G v2/v3, WR850GP (BCM4712 cpu, ADM6996 switch, fccid:
// Motorola ACQWR850GV2)
#define ROUTER_MOTOROLA 0x0d15

// RT210W and RT211W generic and branded (BCM4702 cpu, fccid: Askey
// H8N-RT210W, H8N-RT211W)
// H8N-RT210W: Siemens se505 v1, Belkin F5D7230-4 v1000, v1xxx < 1444
// H8N-RT211W: Belkin F5D7130
#define ROUTER_RT210W 0x0e0f

// RT480W generic and branded (BCM4712 cpu, ADM6996 switch, fccid: Askey
// H8NRT480W),
// Siemens se505 v2, Belkin F5D7230-4 v1444 (2MB flash, fccid: Belkin
// K7S-F5D72304)
#define ROUTER_RT480W 0x0f0f

// Microtik RouterBOARD 500
#define ROUTER_BOARD_500 0x100f

// NMN A/B/G Router Protoype (266 Mhz Xscale, dual minipci)
#define ROUTER_BOARD_XSCALE 0x1101

// Generic BRCM 4702 boards
#define ROUTER_BRCM4702_GENERIC 0x120f

// Buffalo WLI_TX4_G54HP bridge
#define ROUTER_BUFFALO_WLI_TX4_G54HP 0x1304

// Microsoft MN-700 (BCM4702 cpu), (fccid: Microsoft C3KMN700)
#define ROUTER_MICROSOFT_MN700 0x1417

// Buffalo WLA-G54C == WLI-TX1-G54 (BCM4702 cpu - no switch, fccid:
// QDS-BRCM1005)
#define ROUTER_BUFFALO_WLAG54C 0x1510

// Asus WL-500g Premium (BCM4704 cpu, BCM5325E switch, (fccid: Asus
// MSQWL500GP)
#define ROUTER_ASUS_WL500G_PRE 0x1600

// Buffalo WZR-G300N MIMO Router (radio fccid: Buffalo FDI-09101466-0)
#define ROUTER_BUFFALO_WZRG300N 0x1704

// Linksys WRT300N v1 (BCM4704 cpu, BCM5325F switch, fccid: Linksys
// Q87-WRT300N or Q87-WRT300NV1)
#define ROUTER_WRT300N 0x1816

// Buffalo WHR-AM54G54
#define ROUTER_BUFFALO_WHRAM54G54 0x190f

// Magicbox PowerPC board
#define ROUTER_BOARD_MAGICBOX 0x1a01

// Magicbox PowerPC board 2.0
// #define ROUTER_BOARD_MAGICBOX20 0x1b01

// Buffalo WLI2-TX1-G54 Access point (BCM4702 cpu, no switch)
#define ROUTER_BUFFALO_WLI2_TX1_G54 0x1c10

// NMN A/B/G Router Protoype (266 Mhz Xscale, four minipci)
#define ROUTER_BOARD_GATEWORX 0x1d04

// Motorola WE800G v1 (BCM4702 cpu, no switch, minipci radio, fccid: Motorola 
// ACQWE800G, F2NWE800G)
#define ROUTER_MOTOROLA_WE800G 0x1e10

// Generic x86 PC / WRAP / Soekris
#define ROUTER_BOARD_X86 0x1f0f

// ?
#define ROUTER_SUPERGERRY 0x200f

// Linksys WRT350N (BCM4705 cpu, Gbit switch, PCMCIA radio card, fccid:
// Linksys Q87-WRT350N)
#define ROUTER_WRT350N 0x2116

// Linksys WAP54G v1 and v1.1 (BCM4702 cpu, no switch, fccid: Linksys
// PKW-WAP54G)
// 2 different boards (same fccid): WX5510_Vxx and WX5541_Vxx
#define ROUTER_WAP54G_V1 0x2210

// Linksys WAP54G v2 (BCM4712 cpu, no switch, fccid: Linksys Q87-WAP54GV2)
// Linksys WRE54G v1 (BCM4712 cpu, no ethernet port, fccid: Linksys Q87-WRE54G)
#define ROUTER_WAP54G_V2 0x2310

// ViewSonic WAPBR-100 (BCM4712 cpu, no switch, fccid: Viewsonic GSS-VS10407)
#define ROUTER_VIEWSONIC_WAPBR_100 0x2417

// Dell TrueMobile 2300 v1 (BCM4702 cpu, BCM5325 switch, mini pci radio,
// fccid: Gemtek MXF-R920401G)
#define ROUTER_DELL_TRUEMOBILE_2300 0x2510

// Dell TrueMobile 2300 v2 (BCM4712 cpu, ADM6996 switch, fccid: Gemtek
// MXF-R921212G)
// Sparklan WX-6615GT (4712 cpu, ADM6996 switch, fccid: Gemtek MXF-R930116G)
#define ROUTER_DELL_TRUEMOBILE_2300_V2 0x2610

// Fonera (Atheros SoC, no switch, fccid: Accton HED-FON2100)
#define ROUTER_BOARD_FONERA 0x2701

// Buffalo WHR-HP-AG108 (Atheros A+G, Kendin KS8895XA switch, fccid: Buffalo
// FDI-09101540-0)
#define ROUTER_BOARD_WHRAG108 0x2801

// swapped phy definition 
#define ROUTER_BOARD_GATEWORX_SWAP 0x290f

// Netgear WNR834Bv1 (BCM4704 cpu, BCM5325 switch, cardbus radio, fccid:
// Netgear PY306100032)
#define ROUTER_NETGEAR_WNR834B 0x2a17

// swapped phy definition and Kendin switch
#ifdef HAVE_MI424WR
#define ROUTER_BOARD_GATEWORX_GW2345 0x2b0a
#else
#define ROUTER_BOARD_GATEWORX_GW2345 0x2b0f
#endif
// Linksys WRT54G3G (BCM4712 cpu, 5325E switch, PCMCIA slot, fccid: Linksys
// Q87-WRT54G3G)
#define ROUTER_WRT54G3G 0x2c16

// Sitecom WL-105(b) (BCM4702 cpu, no switch, fccid: Gemtek MXF-A910910AB -
// no A-band)
#define ROUTER_SITECOM_WL105B 0x2d10

// Linksys WRT150N (BCM4704 cpu, BCM5325F switch, fccid: Linksys Q87-WRT150N)
#define ROUTER_WRT150N 0x2e16

// ?
#define ROUTER_BOARD_LS2 0x2f01

// Buffalo WAPM-HP-AM54G54 (only available on japanese market, dual radio 2.4 
// and 5 GHz Broadcom)
#define ROUTER_BUFFALO_WAPM_HP_AM54G54 0x3005

// Buffalo WLAH-G54 (only available on japanese market, single radio 2.4 ghz)
#define ROUTER_BUFFALO_WLAH_G54 0x3104

// Buffalo WZR-G144HH (only available on japanese market, 802.11n, Gigabit
// switch. Similar to WRT350N)
#define ROUTER_BUFFALO_WZRG144NH 0x3217

// U.S.Robotics USR5430 bridge (BCM4712 cpu, no switch, fccid: Gemtek
// MXF-EB921201G)
#define ROUTER_USR_5430 0x3310

// U.S.Robotics USR5432 bridge (BCM5350 cpu, 1 port, fccid: Universal Scientific Industrial Co. IXM-APGBR02)
// U.S.Robotics USR5441 range-ext. (BCM5350 cpu, 1 port, fccid: Universal Scientific Industrial Co. IXM-APGBR02)
// U.S.Robotics USR5451 AP (BCM5350 cpu, 1 port, fccid: Universal Scientific Industrial Co. IXM-APGBR02)
// U.S.Robotics USR5461 router (BCM5350 cpu, fccid: Universal Scientific Industrial Co. IXM-RTGBR02)
#define ROUTER_USR_5461 0x340f

// Meraki Mini (Atheros SoC, no switch, fccid: Meraki UDX-MERAKI-MINI)
#define ROUTER_BOARD_MERAKI 0x350f

// Asus WL300g (BCM4702 cpu, no switch, fccid: Asus MSQWL300G)
// Asus WL500g (BCM4702 cpu, fccid: Asus MSQWL500G)
#define ROUTER_ASUS_WL500G 0x3606

// ?
// #ifdef HAVE_ALPHA
// #define ROUTER_BOARD_CA8 0x3706
// #else
#define ROUTER_BOARD_CA8 0x3706
// #endif

// Fonera (Atheros SoC,...)
#define ROUTER_BOARD_FONERA2200 0x3801

// ?
#define ROUTER_BOARD_TW6600 0x3901

// Atheros PB42 prototype (AR5416 MIMO Wifi, 400 Mhz Mips CPU, 32 MB RAM)
#define ROUTER_BOARD_PB42 0x3a01

// Asus WL-500W (MIMO, 4704 cpu, fccid: Asus MSQWL500W)
#define ROUTER_ASUS_WL500W 0x3b06

// Asus WL550gE (BCM5352E cpu, fccid: Asus MSQWL550GE)
// Asus WL320gP/gE (BCM5352E cpu, 1 LAN only, fccid: Asus MSQWL320GP / MSQWL320GE)
#define ROUTER_ASUS_WL550GE 0x3c01

// Sitecom WL-111 (BCM4702 cpu, fccid: MXF-R920220G)
#define ROUTER_SITECOM_WL111 0x3d10

// D-Link DIR-320 (BCM5354 cpu, fccid: D Link KA2DIR320A1)
#define ROUTER_DLINK_DIR320 0x3e17

// Linksys WTR54GS travel router (BCM5350 cpu, 2 ports, fccid: v1, v2:
// Linksys Q87-WTR54GS; v2.1 Q87-WTR54GSV21)
#define ROUTER_LINKSYS_WTR54GS 0x3f13

// Belkin F5D7230 v2000 (BCM4712 cpu, 5325E switch, serial flash, fccid:
// Belkin K7SF5D7234A)
#define ROUTER_BELKIN_F5D7230_V2000 0x400f

// Belkin F5D7231-4 v1212UK (BCM5352E cpu, fccid: Askey H8NRT2406W)
#define ROUTER_BELKIN_F5D7231 0x410f

// Linksys WAP54G v3, WAP54G v3.1 (BCM5352E cpu, fccid: Linksys Q87-WAP54GV3, 
// Q87-WAP54GV31)
#define ROUTER_WAP54G_V3 0x4210

// Asus WL-520G (WL-500G-C in China) (BCM5350 cpu, fccid: Asus MSQWL520G)
#define ROUTER_ASUS_WL520G 0x430f

// Asus WL-520GU / WL-520GC (BCM5354 cpu, fccid: Asus MSQWL520GUGC)
#define ROUTER_ASUS_WL520GUGC 0x4412

// Netgear WG602 v3 (BCM5350 cpu, fccid: Netgear PY3WG602V3)
#define ROUTER_NETGEAR_WG602_V3 0x4511

// Netgear WG602 v4 (BCM5354 cpu, fccid: Netgear PY3WG602V4)
#define ROUTER_NETGEAR_WG602_V4 0x4617

// Linksys WRT600N (BCM4705 cpu, Gigabit switch, dual radio 2.4 and 5 GHz
// Broadcom, fccid: Linksys Q87-WRT600NV1, Q87-WRT600NV11)
#define ROUTER_WRT600N 0x4717

// Linksys WRH54G (BCM5354 cpu, fccid: Linksys Q87-WRH54G)
#define ROUTER_LINKSYS_WRH54G 0x4816

// Linksys WRT150N v1.1 (BCM4704 cpu, BCM5325F switch, fccid: Linksys
// Q87-WRT150NV11)
#define ROUTER_WRT150NV11 0x4916

// Linksys WRT150N v1.2 (fccid: Linksys ?)
#define ROUTER_WRT150NV12 0x4a16

// Linksys WRT160N v1 (BCM4703 cpu, ?? switch, fccid: Linksys Q87-WRT160N)
#define ROUTER_WRT160N 0x4b16

// Linksys WRT300N v1.1 (BCM4705 cpu, 5325 switch, fccid: Linksys
// Q87-WRT300NV11)
#define ROUTER_WRT300NV11 0x4c16

// Linksys WRT310N v1 (BCM4705 cpu, fccid: Linksys Q87-WRT310N)
#define ROUTER_WRT310N 0x4d16

// Linksys WRT310N v2 (BCM4716 cpu, fccid: Linksys Q87-WRT310NV2)
// Linksys M20 (BCM4716 cpu, fccid: Linksys Q87-M20)
#define ROUTER_WRT310NV2 0x4e16

// D-Link DIR330 (BCM5836 cpu, fccid: D Link KA2DIR330A1)
#define ROUTER_DLINK_DIR330 0x4f13

// Netgear WNDR3300 (BCM4704 cpu, BCM5325F switch, dual radio 2.4-N and 5 GHz 
// Broadcom, fccid: Netgear PY307300072)
#define ROUTER_NETGEAR_WNDR3300 0x5016

// Asus WL-330GE (BCM5354, no switch, fccid: MSQWL330GE)
#define ROUTER_ASUS_330GE 0x5112

// Linksys WRT54Gv8.1 (BCM5354 cpu, fccid: Linksys Q87-WRT54GV81)
#define ROUTER_WRT54G_V81 0x5217

// Netgear WGR614L (BCM5354 cpu, 4M serial flash - 16M ram, fccid: Netgear PY306400057)
// Netgear WGR614 v8 - same -
// Netgear WGR614 WW - same -
#define ROUTER_NETGEAR_WGR614L 0x5317

// Netgear WGR614L (BCM5354 cpu, 2M serial flash - 8M ram, fccid: Netgear PY306400057)
#define ROUTER_NETGEAR_WGR614V9 0x5417

// Netgear WNR834Bv2 (BCM4704 cpu, BCM5325 switch, fccid: Netgear
// PY307100061)
#define ROUTER_NETGEAR_WNR834BV2 0x5516

// Asus WL-500g Premium v2 (BCM5354 cpu, fccid: Asus MSQWL500GPV2)
#define ROUTER_ASUS_WL500G_PRE_V2 0x5612

// Wistron CA8-PRO (RDAA-81)
#define ROUTER_BOARD_CA8PRO 0x5706

// Askey board RT2205D-D56 / RT2206D-D56 (BCM4704 cpu, mini-pci)
// Belkin MIMO F5D8230_v2 or F5D8230_v1001ea (fccid: Airgo networks Inc.
// SA3-AGN0901AP0100)
#define ROUTER_ASKEY_RT220XD 0x5817

// U.S.Robotics USR5455 (BCM5354 cpu, no switch, fccid: Universal Scientific
// Industrial Co. IXM-APGBR03)
// U.S.Robotics USR5465 (BCM5354 cpu, fccid: Universal Scientific Industrial
// Co. IXM-RTGBR03)
#define ROUTER_USR_5465 0x590f

#define ROUTER_ALLNET01 0x5a13

// Linksys WRT610N (BCM4785 cpu, Gigabit switch, dual radio 2.4 and 5 GHz
// Broadcom, fccid: Linksys Q87-WRT610N)
#define ROUTER_WRT610N 0x5b16

// Linksys WRT610Nv2 (BCM4718 cpu, Gigabit switch, dual radio 2.4 and 5 GHz
// Broadcom, fccid: Linksys Q87-WRT610NV2)
// Linksys E3000 (BCM4718 cpu, Gigabit switch, dual radio 2.4 and 5 GHz
// Broadcom, fccid: Linksys Q87-E3000)
#define ROUTER_WRT610NV2 0x5c16

// Belkin F5D7230 v3000 (BCM5350 cpu, flash ?? , fccid: Belkin PD5F5D72304)
#define ROUTER_BELKIN_F5D7230_V3000 0x6011

#ifdef HAVE_WMBR_G300NH
#define ROUTER_BOARD_DANUBE 0x6101
#elif HAVE_VF803
#define ROUTER_BOARD_DANUBE 0x6101 //dummy. gpio 28
#elif HAVE_SX763
#define ROUTER_BOARD_DANUBE 0x6101 //dummy. gpio 28
#else
#define ROUTER_BOARD_DANUBE 0x610f
#endif
#define ROUTER_BOARD_STORM 0x6201	// value 1 is a fake to enable reset button code. real gpio is 60

#define ROUTER_BOARD_ADM5120 0x630f

#define ROUTER_BUFFALO_WCAG 0x6404

#define ROUTER_BOARD_WHRG300N 0x651a

#define ROUTER_BOARD_RT2880 0x660f

// Wistron RCAA01 (RCAA-01)
#define ROUTER_BOARD_RCAA01 0x6706

// Wistron RDAT81 (RDAT81)
#define ROUTER_BOARD_RDAT81 0x6806

// Senao ECB9750
#define ROUTER_BOARD_ECB9750 0x691b

// Compex WP54G (and compatible)
#define ROUTER_BOARD_WP54G 0x7004

// Compex NP28G (and compatible)
#define ROUTER_BOARD_NP28G 0x710f

// Belkin F5D7231-4 v2000 (BCM5352E cpu, fccid: Belkin K7SF5D7231B)
#define ROUTER_BELKIN_F5D7231_V2000 0x7213

#define ROUTER_BOARD_ESR6650 0x731a

// Netgear WNR3500L (BCM4718A cpu, 8MB serial flash, USB, Gigabit switch, fccid: Netgear PY308400093)
// Netgear WNR3500v2 - same - half flash
#define ROUTER_NETGEAR_WNR3500L 0x7414	//(18)

// Linksys WRT320N (BCM4717A cpu, 8MB serial flash, Gigabit switch, fccid: Linksys Q87-WRT320N)
// Linksys E2000 (BCM4717A cpu, 8MB serial flash, Gigabit switch, fccid: Linksys Q87-E2000)
#define ROUTER_WRT320N 0x7518

#define ROUTER_BOARD_ESR9752 0x7610

#define ROUTER_BOARD_DIR600B 0x771a

#define ROUTER_BOARD_ACXNR22 0x781a

#define ROUTER_BOARD_AR670W 0x7919

#define ROUTER_BOARD_EAP9550 0x8010

#define ROUTER_BOARD_GATEWORX_GW2369 0x8103

// Linksys WRT160Nv3 (BCM4716A cpu, 4MB serial flash, 5325E switch, fccid: Linksys Q87-WRT160NV3)
// CSE31 ports [W-1-2-3-4], CSE41 and CSE51 ports [W-4-3-2-1]
// Cisco Valet M10 (BCM4716A cpu, 4MB serial flash, 5325E switch, fccid: Linksys Q87-M10)
// Linksys E1000 v1 (BCM4716A cpu, 4MB serial flash, 5325E switch, fccid: Linksys Q87-E1000)
#define ROUTER_WRT160NV3 0x8216

#define ROUTER_BOARD_BS2M 0x831c	//bullet 2m
#define ROUTER_BOARD_BS5M 0x841c	//bullet 5m
#define ROUTER_BOARD_R2M 0x851c	//rocket 2m
#define ROUTER_BOARD_R5M 0x861c	//rocket 5m
#define ROUTER_BOARD_NS2M 0x871c	//nanostation 2m
#define ROUTER_BOARD_NS5M 0x881c	//nanostation 5m

// Asus RT-N10 (BCM5356 cpu, 4MB serial flash / 16MB ram, fccid: Asus MSQ-RTN10)
#define ROUTER_ASUS_RTN10 0x8913

// Asus RT-N12 (BCM4716B0 cpu, 4MB serial flash / 32MB ram, 5325E switch, fccid: Asus MSQ-RTN12)
#define ROUTER_ASUS_RTN12 0x8a11

// Asus RT-N16 (BCM4718A cpu, 32MB flash / 128MB ram, BCM53115 giga switch, fccid: Asus MSQRTN16)
#define ROUTER_ASUS_RTN16 0x8b16

#define ROUTER_BOARD_BR6574N 0x8c1a

#ifdef HAVE_HORNET
#define ROUTER_BOARD_WHRHPGN 0x8d1c	//Buffalo WHR HP GN AR7240 / 4 MB Flash / 32 MB RAM
#elif HAVE_CARAMBOLA
#define ROUTER_BOARD_WHRHPGN 0x8d1b	//Buffalo WHR HP GN AR7240 / 4 MB Flash / 32 MB RAM
#elif HAVE_DIR825C1
#define ROUTER_BOARD_WHRHPGN 0x8d111	//Buffalo WHR HP GN AR7240 / 4 MB Flash / 32 MB RAM
#elif HAVE_WASP
#define ROUTER_BOARD_WHRHPGN 0x8d110	//Buffalo WHR HP GN AR7240 / 4 MB Flash / 32 MB RAM
#elif HAVE_WNR2200
#define ROUTER_BOARD_WHRHPGN 0x8d126	//Buffalo WHR HP GN AR7240 / 4 MB Flash / 32 MB RAM
#elif HAVE_WNR2000
#define ROUTER_BOARD_WHRHPGN 0x8d128	//Buffalo WHR HP GN AR7240 / 4 MB Flash / 32 MB RAM
#else
#define ROUTER_BOARD_WHRHPGN 0x8d1b	//Buffalo WHR HP GN AR7240 / 4 MB Flash / 32 MB RAM
#endif
// Dynex DX-NRUTER (BCM4703 cpu, 4MB flash / 32MB ram, BCM5325E switch, fccid: Belkin K7SDXNRUTER)
#define ROUTER_DYNEX_DX_NRUTER 0x8e12

#define ROUTER_BOARD_OPENRISC 0x8f00

#define ROUTER_BOARD_ASUS_RTN13U 0x901a

// Linksys WRT160NL (Atheros 9130, 8MB flash / 32MB ram, fccid: Linksys Q87-WRT160NL)
// Linksys E2100L (Atheros 9130, 8MB flash / 32MB ram, fccid: Linksys Q87-E2100L)
#define ROUTER_BOARD_WRT160NL 0x9101	// fake id, reset button is gpio 21

// NetCore NW618 / Rosewill RNX-GX4 (BCM5354 cpu, 4MB serial flash / 16MB ram, fccid: Rosewill W6RRNX-GX4)
#define ROUTER_NETCORE_NW618 0x9216

#define ROUTER_BOARD_W502U 0x931a

#define ROUTER_BOARD_DIR615D 0x941a

#define ROUTER_BOARD_AR690W 0x9519

#define ROUTER_BOARD_RB600 0x960f

// Netgear WNR2000 v2 (BCM4716B0 cpu, 4MB serial flash / 32MB ram, fccid: Netgear PY309100105)
#define ROUTER_NETGEAR_WNR2000V2 0x9711

#ifdef HAVE_VENTANA
#define ROUTER_BOARD_GW2388 0x9800
#else
#define ROUTER_BOARD_GW2388 0x980f
#endif
// Belkin Share Max F5D8235v3 (BCM53115 giga switch, BCM4718 cpu, 8MB flash / 32MB ram, fccid: Belkin K7SF5D8235V3)
#define ROUTER_BELKIN_F5D8235V3 0x9916

#ifdef HAVE_VENTANA
#define ROUTER_BOARD_GW2380 0x9a00
#else
#define ROUTER_BOARD_GW2380 0x9a0f
#endif

// Belkin Share Max F7D3301v1 (BCM53115 giga switch, BCM4718 cpu, 8MB flash / 64MB ram, fccid: Belkin K7SF7D3301V1)
// Belkin Share Max N300 F7D7301v1  - same
#define ROUTER_BELKIN_F7D3301 0x9b16

// Belkin Share F7D3302v1 (BCM4718 cpu, BCM5325 switch, 8MB flash / 64MB ram, fccid: Belkin K7SF7D3302V1)
// Belkin Share N300 F7D7302v1  - same
#define ROUTER_BELKIN_F7D3302 0x9c16

// Belkin Play Max F7D4301v1 (dual radio, BCM53115 giga switch, BCM4718 cpu, 8MB flash / 64MB ram, fccid: Belkin K7SF7D4301V1)
// Belkin Play N600 HD F7D8301v1 - same
#define ROUTER_BELKIN_F7D4301 0x9d16

// Belkin Play F7D4302v1 (dual radio, BCM5325 switch, BCM4718 cpu, 8MB flash / 64MB ram, fccid: Belkin K7SF7D4302V1)
// Belkin Play N600 F7D8302v1 - same 
#define ROUTER_BELKIN_F7D4302 0x9e16

// Asus RT-N10+
#define ROUTER_ASUS_RTN10PLUS 0x9f1a

// Linksys E1000 v2 (BCM5357 cpu, 4MB serial flash, 32 MB ram, fccid: Linksys Q87-E1000V2)
// Linksys E1000 v2.1 (BCM5357 cpu, 4MB serial flash, 32 MB ram, fccid: Linksys Q87-E1000V21)
#define ROUTER_LINKSYS_E1000V2 0xa01a

// Repotec RP-WR5422
#define ROUTER_BOARD_WR5422 0xa11a

// Asus WL-700ge (BCM4780 cpu, 2MB flash, 64MB ram, BCM5325E switch)
#define ROUTER_ASUS_WL700GE 0xa217

#define ROUTER_BOARD_F5D8235 0xa31a

// Asus RT-15N
#define ROUTER_BOARD_RT15N 0xa410c

#define ROUTER_BOARD_TECHNAXX 0xa50f

#define ROUTER_BOARD_NEPTUNE 0xa61a

#define ROUTER_ASUS_RTN12B 0xa7116

// Linksys E4200 (dual radio, BCM53115 giga switch, BCM4718 cpu, 16MB flash / 64MB ram, fccid: Linksys Q87-E4200) >_60K nvram_<
#define ROUTER_LINKSYS_E4200 0xa816

#define ROUTER_BOARD_RT3352 0xa91a // concept

// Asus RT-N10U (BCM5358 cpu, 8MB serial flash / 32MB ram, USB 2.0)
#define ROUTER_ASUS_RTN10U 0xaa115

// Linksys E3200 (dual radio, BCM53125 giga switch, BCM5357 cpu, 16MB flash / 64MB ram, fccid: Linksys Q87-E3200) >_60K nvram_<
#define ROUTER_LINKSYS_E3200 0xab15

// Netgear WNDR3700v3 (dual radio, BCM53115 giga switch, BCM4718 cpu, 8MB flash / 64MB ram, fccid: Netgear PY311200166) >_64K nvram_<
// Netgear WNDR4000 (dual radio, BCM53115 giga switch, BCM4718 cpu, 8MB flash / 64MB ram, fccid: Netgear PY310400144) >_64K nvram_<
#define ROUTER_NETGEAR_WNDR4000 0xac13

#define ROUTER_NETCORE_NW715P 0xad0f

#define ROUTER_BOARD_UNIFI 0xae1c	//bullet 2m

// Asus RT-N66U (BCM4706 @ 600MHz cpu, 32MB flash / 256MB ram, fccid: Asus MSQ-RTN66U)
#define ROUTER_ASUS_RTN66 0xaf19

// Asus RT-N53		    
#define ROUTER_ASUS_RTN53 0xb013

// Netgear WNDR3400 (dual radio, BCM5325E switch, BCM4718 cpu, 8MB flash / 64MB ram, fccid: Netgear PY309300116) >_64K nvram_<
#define ROUTER_NETGEAR_WNDR3400 0xb114

// Netgear WNDRR6300(dual radio, BCM53115 giga switch, BCM4706 @ 600MHz cpu, 2MB serial + 128MB NAND flash / 128MB ram, fccid: Netgear PY312100188) >_64K nvram_<
// Netgear WNDR4500 (dual radio, BCM53115 giga switch, BCM4706 @ 600MHz cpu, 2MB serial + 128MB NAND flash / 128MB ram, fccid: Netgear PY311200162) >_64K nvram_<
#define ROUTER_NETGEAR_R6300 0xb216


#define ROUTER_BOARD_WCRGN 0xb31a

// Linksys E2500 (dual radio, BCM5358U cpu, 16MB flash / 64MB ram, fccid: Linksys Q87-E2500) >_60K nvram_<
#define ROUTER_LINKSYS_E2500 0xb41a

#define ROUTER_BOARD_TECHNAXX3G 0xb51a

// Linksys E900 (BCM53572 cpu, 8MB serial flash / 32 MB ram, fccid: Linksys Q87-E900) >_64K nvram_<
// Linksys E1200v2 (BCM53572 cpu, 8MB serial flash / 32 MB ram, fccid: Linksys Q87-E1200V2) >_64K nvram_<
#define ROUTER_LINKSYS_E900 0xb61a

// Linksys E1500 (BCM5357 cpu, 8MB serial flash / 32 MB ram, fccid: Linksys Q87-E1500) >_64K nvram_<
// Linksys E1200v1 (BCM5357 cpu, 4MB serial flash / 32 MB ram, fccid: Linksys Q87-E1200) >_64K nvram_<
#define ROUTER_LINKSYS_E1500 0xb71a

// Linksys E1550 (BCM5358U cpu, 16MB serial flash / 64MB ram, USB, fccid: Linksys Q87-E1550) >_60K nvram_<
#define ROUTER_LINKSYS_E1550 0xb81a

#define ROUTER_D1800H 0xb919

#define ROUTER_HUAWEI_B970B 0xba0f

#define ROUTER_BOARD_WNR3500LV2 0xbb0f

#define ROUTER_BOARD_HAMEA15 0xbc10

#define ROUTER_ASUS_AC66U 0xbd19

#define ROUTER_ASUS_AC56U 0xbe1b // BCM4708 SMP 800 Mhz 128 MB Nand Flash, 128 MB Ram

#define ROUTER_BOARD_NORTHSTAR 0xbf0f

#define ROUTER_ASUS_AC67U 0xc01b // BCM4708 SMP 800 Mhz 128 MB Nand Flash, 128 MB Ram

#define ROUTER_ASUS_RTN10PLUSD1 0xc1115

#define ROUTER_BOARD_WDR4900 0xc203

#define ROUTER_BUFFALO_WZR1750 0xc31b // BCM4708 SMP 800 Mhz 128 MB Nand Flash, 512 MB Ram

#define ROUTER_BUFFALO_WZR900DHP 0xc41b // BCM4707 Single Core 800 Mhz 128 MB Nand Flash, 256 MB Ram

#define ROUTER_BUFFALO_WZR600DHP2 0xc51b // BCM4707 Single Core 800 Mhz 128 MB Nand Flash, 256 MB Ram

#define ROUTER_LINKSYS_E800 0xc61a

#define ROUTER_LINKSYS_EA2700 0xc715

#define ROUTER_NETGEAR_WNDR4500 0xc816

#define ROUTER_NETGEAR_WNDR4500V2 0xc916

#define ROUTER_DLINK_DIR868 0xca1b

#define ROUTER_LINKSYS_EA6500 0xcb13

#define ROUTER_WHR300HP2 0xcc11

#define ROUTER_NETGEAR_R6250 0xcd16

#define ROUTER_NETGEAR_R6300V2 0xce16

#define ROUTER_NETGEAR_R7000 0xcf16

#define ROUTER_BOARD_UNIWIP 0xd001

#define ROUTER_LINKSYS_EA6900 0xd11b

#define ROUTER_LINKSYS_EA6500V2 0xd21b

#define ROUTER_TRENDNET_TEW812 0xd31b

#define ROUTER_TRENDNET_TEW811 0xd41b

#define ROUTER_LINKSYS_EA6700 0xd51b

#define ROUTER_BOARD_TI 0xd60f

#define ROUTER_NETGEAR_AC1450 0xd716

#define ROUTER_ASUS_RTN18U 0xd817 // BCM4708 SMP 800 Mhz 128 MB Nand Flash, 128 MB Ram

#define ROUTER_NETGEAR_EX6200 0xd916  // BCM4708 SMP 800 Mhz 8MB SPI Flash, 128 MB Ram

#define NVROUTER "DD_BOARD"

static inline int startswith(char *source, char *cmp)
{
	return !strncmp(source, cmp, strlen(cmp));
}

typedef struct {
	char *tz_name;
	const char *tz_string;
} TIMEZONE_TO_TZSTRING;

 
static TIMEZONE_TO_TZSTRING allTimezones[] = {
	{"Africa/Abidjan" , "GMT0" },
	{"Africa/Accra" , "GMT0" },
	{"Africa/Addis Ababa" , "EAT-3" },
	{"Africa/Algiers" , "CET-1" },
	{"Africa/Asmara" , "EAT-3" },
	{"Africa/Bamako" , "GMT0" },
	{"Africa/Bangui" , "WAT-1" },
	{"Africa/Banjul" , "GMT0" },
	{"Africa/Bissau" , "GMT0" },
	{"Africa/Blantyre" , "CAT-2" },
	{"Africa/Brazzaville" , "WAT-1" },
	{"Africa/Bujumbura" , "CAT-2" },
	{"Africa/Casablanca" , "WET0" },
	{"Africa/Ceuta" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Africa/Conakry" , "GMT0" },
	{"Africa/Dakar" , "GMT0" },
	{"Africa/Dar es Salaam" , "EAT-3" },
	{"Africa/Djibouti" , "EAT-3" },
	{"Africa/Douala" , "WAT-1" },
	{"Africa/El Aaiun" , "WET0" },
	{"Africa/Freetown" , "GMT0" },
	{"Africa/Gaborone" , "CAT-2" },
	{"Africa/Harare" , "CAT-2" },
	{"Africa/Johannesburg" , "SAST-2" },
	{"Africa/Kampala" , "EAT-3" },
	{"Africa/Khartoum" , "EAT-3" },
	{"Africa/Kigali" , "CAT-2" },
	{"Africa/Kinshasa" , "WAT-1" },
	{"Africa/Lagos" , "WAT-1" },
	{"Africa/Libreville" , "WAT-1" },
	{"Africa/Lome" , "GMT0" },
	{"Africa/Luanda" , "WAT-1" },
	{"Africa/Lubumbashi" , "CAT-2" },
	{"Africa/Lusaka" , "CAT-2" },
	{"Africa/Malabo" , "WAT-1" },
	{"Africa/Maputo" , "CAT-2" },
	{"Africa/Maseru" , "SAST-2" },
	{"Africa/Mbabane" , "SAST-2" },
	{"Africa/Mogadishu" , "EAT-3" },
	{"Africa/Monrovia" , "GMT0" },
	{"Africa/Nairobi" , "EAT-3" },
	{"Africa/Ndjamena" , "WAT-1" },
	{"Africa/Niamey" , "WAT-1" },
	{"Africa/Nouakchott" , "GMT0" },
	{"Africa/Ouagadougou" , "GMT0" },
	{"Africa/Porto-Novo" , "WAT-1" },
	{"Africa/Sao Tome" , "GMT0" },
	{"Africa/Tripoli" , "EET-2" },
	{"Africa/Tunis" , "CET-1" },
	{"Africa/Windhoek" , "WAT-1WAST,M9.1.0,M4.1.0" },
	{"America/Adak" , "HAST10HADT,M3.2.0,M11.1.0" },
	{"America/Anchorage" , "AKST9AKDT,M3.2.0,M11.1.0" },
	{"America/Anguilla" , "AST4" },
	{"America/Antigua" , "AST4" },
	{"America/Araguaina" , "BRT3" },
	{"America/Argentina/Buenos Aires" , "ART3" },
	{"America/Argentina/Catamarca" , "ART3" },
	{"America/Argentina/Cordoba" , "ART3" },
	{"America/Argentina/Jujuy" , "ART3" },
	{"America/Argentina/La Rioja" , "ART3" },
	{"America/Argentina/Mendoza" , "ART3" },
	{"America/Argentina/Rio Gallegos" , "ART3" },
	{"America/Argentina/Salta" , "ART3" },
	{"America/Argentina/San Juan" , "ART3" },
	{"America/Argentina/Tucuman" , "ART3" },
	{"America/Argentina/Ushuaia" , "ART3" },
	{"America/Aruba" , "AST4" },
	{"America/Asuncion" , "PYT4PYST,M10.1.0/0,M4.2.0/0" },
	{"America/Atikokan" , "EST5" },
	{"America/Bahia" , "BRT3" },
	{"America/Barbados" , "AST4" },
	{"America/Belem" , "BRT3" },
	{"America/Belize" , "CST6" },
	{"America/Blanc-Sablon" , "AST4" },
	{"America/Boa Vista" , "AMT4" },
	{"America/Bogota" , "COT5" },
	{"America/Boise" , "MST7MDT,M3.2.0,M11.1.0" },
	{"America/Cambridge Bay" , "MST7MDT,M3.2.0,M11.1.0" },
	{"America/Campo Grande" , "AMT4AMST,M10.3.0/0,M2.3.0/0" },
	{"America/Cancun" , "CST6CDT,M4.1.0,M10.5.0" },
	{"America/Caracas" , "VET4:30" },
	{"America/Cayenne" , "GFT3" },
	{"America/Cayman" , "EST5" },
	{"America/Chicago" , "CST6CDT,M3.2.0,M11.1.0" },
	{"America/Chihuahua" , "MST7MDT,M4.1.0,M10.5.0" },
	{"America/Costa Rica" , "CST6" },
	{"America/Cuiaba" , "AMT4AMST,M10.3.0/0,M2.3.0/0" },
	{"America/Curacao" , "AST4" },
	{"America/Danmarkshavn" , "GMT0" },
	{"America/Dawson" , "PST8PDT,M3.2.0,M11.1.0" },
	{"America/Dawson Creek" , "MST7" },
	{"America/Denver" , "MST7MDT,M3.2.0,M11.1.0" },
	{"America/Detroit" , "EST5EDT,M3.2.0,M11.1.0" },
	{"America/Dominica" , "AST4" },
	{"America/Edmonton" , "MST7MDT,M3.2.0,M11.1.0" },
	{"America/Eirunepe" , "AMT4" },
	{"America/El Salvador" , "CST6" },
	{"America/Fortaleza" , "BRT3" },
	{"America/Glace Bay" , "AST4ADT,M3.2.0,M11.1.0" },
	{"America/Goose Bay" , "AST4ADT,M3.2.0/0:01,M11.1.0/0:01" },
	{"America/Grand Turk" , "EST5EDT,M3.2.0,M11.1.0" },
	{"America/Grenada" , "AST4" },
	{"America/Guadeloupe" , "AST4" },
	{"America/Guatemala" , "CST6" },
	{"America/Guayaquil" , "ECT5" },
	{"America/Guyana" , "GYT4" },
	{"America/Halifax" , "AST4ADT,M3.2.0,M11.1.0" },
	{"America/Havana" , "CST5CDT,M3.2.0/0,M10.5.0/1" },
	{"America/Hermosillo" , "MST7" },
	{"America/Indiana/Indianapolis" , "EST5EDT,M3.2.0,M11.1.0" },
	{"America/Indiana/Knox" , "CST6CDT,M3.2.0,M11.1.0" },
	{"America/Indiana/Marengo" , "EST5EDT,M3.2.0,M11.1.0" },
	{"America/Indiana/Petersburg" , "EST5EDT,M3.2.0,M11.1.0" },
	{"America/Indiana/Tell City" , "CST6CDT,M3.2.0,M11.1.0" },
	{"America/Indiana/Vevay" , "EST5EDT,M3.2.0,M11.1.0" },
	{"America/Indiana/Vincennes" , "EST5EDT,M3.2.0,M11.1.0" },
	{"America/Indiana/Winamac" , "EST5EDT,M3.2.0,M11.1.0" },
	{"America/Inuvik" , "MST7MDT,M3.2.0,M11.1.0" },
	{"America/Iqaluit" , "EST5EDT,M3.2.0,M11.1.0" },
	{"America/Jamaica" , "EST5" },
	{"America/Juneau" , "AKST9AKDT,M3.2.0,M11.1.0" },
	{"America/Kentucky/Louisville" , "EST5EDT,M3.2.0,M11.1.0" },
	{"America/Kentucky/Monticello" , "EST5EDT,M3.2.0,M11.1.0" },
	{"America/La Paz" , "BOT4" },
	{"America/Lima" , "PET5" },
	{"America/Los Angeles" , "PST8PDT,M3.2.0,M11.1.0" },
	{"America/Maceio" , "BRT3" },
	{"America/Managua" , "CST6" },
	{"America/Manaus" , "AMT4" },
	{"America/Marigot" , "AST4" },
	{"America/Martinique" , "AST4" },
	{"America/Matamoros" , "CST6CDT,M3.2.0,M11.1.0" },
	{"America/Mazatlan" , "MST7MDT,M4.1.0,M10.5.0" },
	{"America/Menominee" , "CST6CDT,M3.2.0,M11.1.0" },
	{"America/Merida" , "CST6CDT,M4.1.0,M10.5.0" },
	{"America/Mexico City" , "CST6CDT,M4.1.0,M10.5.0" },
	{"America/Miquelon" , "PMST3PMDT,M3.2.0,M11.1.0" },
	{"America/Moncton" , "AST4ADT,M3.2.0,M11.1.0" },
	{"America/Monterrey" , "CST6CDT,M4.1.0,M10.5.0" },
	{"America/Montevideo" , "UYT3UYST,M10.1.0,M3.2.0" },
	{"America/Montreal" , "EST5EDT,M3.2.0,M11.1.0" },
	{"America/Montserrat" , "AST4" },
	{"America/Nassau" , "EST5EDT,M3.2.0,M11.1.0" },
	{"America/New York" , "EST5EDT,M3.2.0,M11.1.0" },
	{"America/Nipigon" , "EST5EDT,M3.2.0,M11.1.0" },
	{"America/Nome" , "AKST9AKDT,M3.2.0,M11.1.0" },
	{"America/Noronha" , "FNT2" },
	{"America/North Dakota/Center" , "CST6CDT,M3.2.0,M11.1.0" },
	{"America/North Dakota/New Salem" , "CST6CDT,M3.2.0,M11.1.0" },
	{"America/Ojinaga" , "MST7MDT,M3.2.0,M11.1.0" },
	{"America/Panama" , "EST5" },
	{"America/Pangnirtung" , "EST5EDT,M3.2.0,M11.1.0" },
	{"America/Paramaribo" , "SRT3" },
	{"America/Phoenix" , "MST7" },
	{"America/Port of Spain" , "AST4" },
	{"America/Port-au-Prince" , "EST5" },
	{"America/Porto Velho" , "AMT4" },
	{"America/Puerto Rico" , "AST4" },
	{"America/Rainy River" , "CST6CDT,M3.2.0,M11.1.0" },
	{"America/Rankin Inlet" , "CST6CDT,M3.2.0,M11.1.0" },
	{"America/Recife" , "BRT3" },
	{"America/Regina" , "CST6" },
	{"America/Rio Branco" , "AMT4" },
	{"America/Santa Isabel" , "PST8PDT,M4.1.0,M10.5.0" },
	{"America/Santarem" , "BRT3" },
	{"America/Santo Domingo" , "AST4" },
	{"America/Sao Paulo" , "BRT3BRST,M10.3.0/0,M2.3.0/0" },
	{"America/Scoresbysund" , "EGT1EGST,M3.5.0/0,M10.5.0/1" },
	{"America/Shiprock" , "MST7MDT,M3.2.0,M11.1.0" },
	{"America/St Barthelemy" , "AST4" },
	{"America/St Johns" , "NST3:30NDT,M3.2.0/0:01,M11.1.0/0:01" },
	{"America/St Kitts" , "AST4" },
	{"America/St Lucia" , "AST4" },
	{"America/St Thomas" , "AST4" },
	{"America/St Vincent" , "AST4" },
	{"America/Swift Current" , "CST6" },
	{"America/Tegucigalpa" , "CST6" },
	{"America/Thule" , "AST4ADT,M3.2.0,M11.1.0" },
	{"America/Thunder Bay" , "EST5EDT,M3.2.0,M11.1.0" },
	{"America/Tijuana" , "PST8PDT,M3.2.0,M11.1.0" },
	{"America/Toronto" , "EST5EDT,M3.2.0,M11.1.0" },
	{"America/Tortola" , "AST4" },
	{"America/Vancouver" , "PST8PDT,M3.2.0,M11.1.0" },
	{"America/Whitehorse" , "PST8PDT,M3.2.0,M11.1.0" },
	{"America/Winnipeg" , "CST6CDT,M3.2.0,M11.1.0" },
	{"America/Yakutat" , "AKST9AKDT,M3.2.0,M11.1.0" },
	{"America/Yellowknife" , "MST7MDT,M3.2.0,M11.1.0" },
	{"Antarctica/Casey" , "WST-8" },
	{"Antarctica/Davis" , "DAVT-7" },
	{"Antarctica/DumontDUrville" , "DDUT-10" },
	{"Antarctica/Macquarie" , "MIST-11" },
	{"Antarctica/Mawson" , "MAWT-5" },
	{"Antarctica/McMurdo" , "NZST-12NZDT,M9.5.0,M4.1.0/3" },
	{"Antarctica/Rothera" , "ROTT3" },
	{"Antarctica/South Pole" , "NZST-12NZDT,M9.5.0,M4.1.0/3" },
	{"Antarctica/Syowa" , "SYOT-3" },
	{"Antarctica/Vostok" , "VOST-6" },
	{"Arctic/Longyearbyen" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Asia/Aden" , "AST-3" },
	{"Asia/Almaty" , "ALMT-6" },
	{"Asia/Anadyr" , "ANAT-11ANAST,M3.5.0,M10.5.0/3" },
	{"Asia/Aqtau" , "AQTT-5" },
	{"Asia/Aqtobe" , "AQTT-5" },
	{"Asia/Ashgabat" , "TMT-5" },
	{"Asia/Baghdad" , "AST-3" },
	{"Asia/Bahrain" , "AST-3" },
	{"Asia/Baku" , "AZT-4AZST,M3.5.0/4,M10.5.0/5" },
	{"Asia/Bangkok" , "ICT-7" },
	{"Asia/Beirut" , "EET-2EEST,M3.5.0/0,M10.5.0/0" },
	{"Asia/Bishkek" , "KGT-6" },
	{"Asia/Brunei" , "BNT-8" },
	{"Asia/Choibalsan" , "CHOT-8" },
	{"Asia/Chongqing" , "CST-8" },
	{"Asia/Colombo" , "IST-5:30" },
	{"Asia/Damascus" , "EET-2EEST,M4.1.5/0,M10.5.5/0" },
	{"Asia/Dhaka" , "BDT-6" },
	{"Asia/Dili" , "TLT-9" },
	{"Asia/Dubai" , "GST-4" },
	{"Asia/Dushanbe" , "TJT-5" },
	{"Asia/Gaza" , "EET-2EEST,M3.5.6/0:01,M9.1.5" },
	{"Asia/Harbin" , "CST-8" },
	{"Asia/Ho Chi Minh" , "ICT-7" },
	{"Asia/Hong Kong" , "HKT-8" },
	{"Asia/Hovd" , "HOVT-7" },
	{"Asia/Irkutsk" , "IRKT-8IRKST,M3.5.0,M10.5.0/3" },
	{"Asia/Jakarta" , "WIT-7" },
	{"Asia/Jayapura" , "EIT-9" },
	{"Asia/Kabul" , "AFT-4:30" },
	{"Asia/Kamchatka" , "PETT-11PETST,M3.5.0,M10.5.0/3" },
	{"Asia/Karachi" , "PKT-5" },
	{"Asia/Kashgar" , "CST-8" },
	{"Asia/Kathmandu" , "NPT-5:45" },
	{"Asia/Kolkata" , "IST-5:30" },
	{"Asia/Krasnoyarsk" , "KRAT-7KRAST,M3.5.0,M10.5.0/3" },
	{"Asia/Kuala Lumpur" , "MYT-8" },
	{"Asia/Kuching" , "MYT-8" },
	{"Asia/Kuwait" , "AST-3" },
	{"Asia/Macau" , "CST-8" },
	{"Asia/Magadan" , "MAGT-11MAGST,M3.5.0,M10.5.0/3" },
	{"Asia/Makassar" , "CIT-8" },
	{"Asia/Manila" , "PHT-8" },
	{"Asia/Muscat" , "GST-4" },
	{"Asia/Nicosia" , "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{"Asia/Novokuznetsk" , "NOVT-6NOVST,M3.5.0,M10.5.0/3" },
	{"Asia/Novosibirsk" , "NOVT-6NOVST,M3.5.0,M10.5.0/3" },
	{"Asia/Omsk" , "OMST-7" },
	{"Asia/Oral" , "ORAT-5" },
	{"Asia/Phnom Penh" , "ICT-7" },
	{"Asia/Pontianak" , "WIT-7" },
	{"Asia/Pyongyang" , "KST-9" },
	{"Asia/Qatar" , "AST-3" },
	{"Asia/Qyzylorda" , "QYZT-6" },
	{"Asia/Rangoon" , "MMT-6:30" },
	{"Asia/Riyadh" , "AST-3" },
	{"Asia/Sakhalin" , "SAKT-10SAKST,M3.5.0,M10.5.0/3" },
	{"Asia/Samarkand" , "UZT-5" },
	{"Asia/Seoul" , "KST-9" },
	{"Asia/Shanghai" , "CST-8" },
	{"Asia/Singapore" , "SGT-8" },
	{"Asia/Taipei" , "CST-8" },
	{"Asia/Tashkent" , "UZT-5" },
	{"Asia/Tbilisi" , "GET-4" },
	{"Asia/Thimphu" , "BTT-6" },
	{"Asia/Tokyo" , "JST-9" },
	{"Asia/Ulaanbaatar" , "ULAT-8" },
	{"Asia/Urumqi" , "CST-8" },
	{"Asia/Vientiane" , "ICT-7" },
	{"Asia/Vladivostok" , "VLAT-10VLAST,M3.5.0,M10.5.0/3" },
	{"Asia/Yakutsk" , "YAKT-9YAKST,M3.5.0,M10.5.0/3" },
	{"Asia/Yekaterinburg" , "YEKT-5YEKST,M3.5.0,M10.5.0/3" },
	{"Asia/Yerevan" , "AMT-4AMST,M3.5.0,M10.5.0/3" },
	{"Atlantic/Azores" , "AZOT1AZOST,M3.5.0/0,M10.5.0/1" },
	{"Atlantic/Bermuda" , "AST4ADT,M3.2.0,M11.1.0" },
	{"Atlantic/Canary" , "WET0WEST,M3.5.0/1,M10.5.0" },
	{"Atlantic/Cape Verde" , "CVT1" },
	{"Atlantic/Faroe" , "WET0WEST,M3.5.0/1,M10.5.0" },
	{"Atlantic/Madeira" , "WET0WEST,M3.5.0/1,M10.5.0" },
	{"Atlantic/Reykjavik" , "GMT0" },
	{"Atlantic/South Georgia" , "GST2" },
	{"Atlantic/St Helena" , "GMT0" },
	{"Atlantic/Stanley" , "FKT4FKST,M9.1.0,M4.3.0" },
	{"Australia/Adelaide" , "CST-9:30CST,M10.1.0,M4.1.0/3" },
	{"Australia/Brisbane" , "EST-10" },
	{"Australia/Broken Hill" , "CST-9:30CST,M10.1.0,M4.1.0/3" },
	{"Australia/Currie" , "EST-10EST,M10.1.0,M4.1.0/3" },
	{"Australia/Darwin" , "CST-9:30" },
	{"Australia/Eucla" , "CWST-8:45" },
	{"Australia/Hobart" , "EST-10EST,M10.1.0,M4.1.0/3" },
	{"Australia/Lindeman" , "EST-10" },
	{"Australia/Lord Howe" , "LHST-10:30LHST-11,M10.1.0,M4.1.0" },
	{"Australia/Melbourne" , "EST-10EST,M10.1.0,M4.1.0/3" },
	{"Australia/Perth" , "WST-8" },
	{"Australia/Sydney" , "EST-10EST,M10.1.0,M4.1.0/3" },
	{"Europe/Amsterdam" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Andorra" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Athens" , "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{"Europe/Belgrade" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Berlin" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Bratislava" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Brussels" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Bucharest" , "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{"Europe/Budapest" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Chisinau" , "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{"Europe/Copenhagen" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Dublin" , "GMT0IST,M3.5.0/1,M10.5.0" },
	{"Europe/Gibraltar" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Guernsey" , "GMT0BST,M3.5.0/1,M10.5.0" },
	{"Europe/Helsinki" , "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{"Europe/Isle of Man" , "GMT0BST,M3.5.0/1,M10.5.0" },
	{"Europe/Istanbul" , "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{"Europe/Jersey" , "GMT0BST,M3.5.0/1,M10.5.0" },
	{"Europe/Kaliningrad" , "EET-2EEST,M3.5.0,M10.5.0/3" },
	{"Europe/Kiev" , "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{"Europe/Lisbon" , "WET0WEST,M3.5.0/1,M10.5.0" },
	{"Europe/Ljubljana" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/London" , "GMT0BST,M3.5.0/1,M10.5.0" },
	{"Europe/Luxembourg" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Madrid" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Malta" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Mariehamn" , "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{"Europe/Minsk" , "EET-2EEST,M3.5.0,M10.5.0/3" },
	{"Europe/Monaco" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Moscow" , "MSK-4" },
	{"Europe/Oslo" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Paris" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Podgorica" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Prague" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Riga" , "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{"Europe/Rome" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Samara" , "SAMT-3SAMST,M3.5.0,M10.5.0/3" },
	{"Europe/San Marino" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Sarajevo" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Simferopol" , "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{"Europe/Skopje" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Sofia" , "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{"Europe/Stockholm" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Tallinn" , "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{"Europe/Tirane" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Uzhgorod" , "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{"Europe/Vaduz" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Vatican" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Vienna" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Vilnius" , "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{"Europe/Volgograd" , "VOLT-3VOLST,M3.5.0,M10.5.0/3" },
	{"Europe/Warsaw" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Zagreb" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Europe/Zaporozhye" , "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{"Europe/Zurich" , "CET-1CEST,M3.5.0,M10.5.0/3" },
	{"Indian/Antananarivo" , "EAT-3" },
	{"Indian/Chagos" , "IOT-6" },
	{"Indian/Christmas" , "CXT-7" },
	{"Indian/Cocos" , "CCT-6:30" },
	{"Indian/Comoro" , "EAT-3" },
	{"Indian/Kerguelen" , "TFT-5" },
	{"Indian/Mahe" , "SCT-4" },
	{"Indian/Maldives" , "MVT-5" },
	{"Indian/Mauritius" , "MUT-4" },
	{"Indian/Mayotte" , "EAT-3" },
	{"Indian/Reunion" , "RET-4" },
	{"Pacific/Apia" , "WST11" },
	{"Pacific/Auckland" , "NZST-12NZDT,M9.5.0,M4.1.0/3" },
	{"Pacific/Chatham" , "CHAST-12:45CHADT,M9.5.0/2:45,M4.1.0/3:45" },
	{"Pacific/Efate" , "VUT-11" },
	{"Pacific/Enderbury" , "PHOT-13" },
	{"Pacific/Fakaofo" , "TKT10" },
	{"Pacific/Fiji" , "FJT-12" },
	{"Pacific/Funafuti" , "TVT-12" },
	{"Pacific/Galapagos" , "GALT6" },
	{"Pacific/Gambier" , "GAMT9" },
	{"Pacific/Guadalcanal" , "SBT-11" },
	{"Pacific/Guam" , "ChST-10" },
	{"Pacific/Honolulu" , "HST10" },
	{"Pacific/Johnston" , "HST10" },
	{"Pacific/Kiritimati" , "LINT-14" },
	{"Pacific/Kosrae" , "KOST-11" },
	{"Pacific/Kwajalein" , "MHT-12" },
	{"Pacific/Majuro" , "MHT-12" },
	{"Pacific/Marquesas" , "MART9:30" },
	{"Pacific/Midway" , "SST11" },
	{"Pacific/Nauru" , "NRT-12" },
	{"Pacific/Niue" , "NUT11" },
	{"Pacific/Norfolk" , "NFT-11:30" },
	{"Pacific/Noumea" , "NCT-11" },
	{"Pacific/Pago Pago" , "SST11" },
	{"Pacific/Palau" , "PWT-9" },
	{"Pacific/Pitcairn" , "PST8" },
	{"Pacific/Ponape" , "PONT-11" },
	{"Pacific/Port Moresby" , "PGT-10" },
	{"Pacific/Rarotonga" , "CKT10" },
	{"Pacific/Saipan" , "ChST-10" },
	{"Pacific/Tahiti" , "TAHT10" },
	{"Pacific/Tarawa" , "GILT-12" },
	{"Pacific/Tongatapu" , "TOT-13" },
	{"Pacific/Truk" , "TRUT-10" },
	{"Pacific/Wake" , "WAKT-12" },
	{"Pacific/Wallis" , "WFT-12" }
};

extern char *getBridge(char *ifname);
extern char *getRealBridge(char *ifname);
extern char *getWDSSTA(void);
extern char *getSTA(void);
extern char *getWET(void);
extern int wanChanged(void);
extern void notifywanChange(void);
extern int contains(const char *string, char value);
extern int getcpurev(void);
extern int nvram_used(int *space);
extern int cpu_plltype(void);
extern int check_vlan_support(void);

extern int remove_from_list(const char *name, char *list, int listsize);
extern int add_to_list(const char *name, char *list, int listsize);
extern char *find_in_list(const char *haystack, const char *needle);
extern int startswith(char *source, char *cmp);
extern int count_occurences(char *source, int cmp);
extern int pos_nthoccurence(char *source, int cmp, int which);
extern char *substring(int start, int stop, const char *src, char *dst);
extern void strtrim_right(char *p, int c);
extern unsigned int daysformonth(unsigned int month, unsigned int year);
extern int weekday(int month, int day, int year);
extern int getRouterBrand(void);
extern char *getRouter(void);
extern int diag_led(int type, int act);
extern int C_led(int i);
extern int get_single_ip(char *ipaddr, int which);
extern char *get_mac_from_ip(char *ip);
extern struct dns_lists *get_dns_list(void);
extern int dns_to_resolv(void);
extern char *get_wan_face(void);

extern int check_wan_link(int num);
extern char *get_wan_ipaddr(void);
extern char *get_complete_ip(char *from, char *to);
extern char *get_complete_lan_ip(char *ip);
extern int get_int_len(int num);
extern int file_to_buf(char *path, char *buf, int len);
extern int buf_to_file(char *path, char *buf);
extern pid_t *find_pid_by_name(char *pidName);
extern int find_pid_by_ps(char *pidName);
extern int *find_all_pid_by_ps(char *pidName);
extern char *find_name_by_proc(int pid);
extern int get_ppp_pid(char *file);
extern int convert_ver(char *ver);
extern int check_flash(void);
extern int check_action(void);
extern int check_now_boot(void);
extern int check_hw_type(void);
extern int is_exist(char *filename);
extern void set_ip_forward(char c);
struct mtu_lists *get_mtu(char *proto);
extern void set_host_domain_name(void);

extern void encode(char *buf, int len);
extern void decode(char *buf, int len);
extern char *zencrypt(char *passwd);

extern void getLANMac(char *newmac);
extern void getWirelessMac(char *newmac,int instance);
extern void getWANMac(char *newmac);
extern char *cpustring(void);
extern int isap8x(void);
extern int led_control(int type, int act);
enum { LED_POWER, LED_DIAG, LED_DIAG_DISABLED, LED_DMZ, LED_CONNECTED, LED_DISCONNECTED, LED_BRIDGE, LED_VPN,
	LED_SES, LED_SES2, LED_WLAN, LED_WLAN0, LED_WLAN1, LED_USB, LED_USB1, LED_SEC0, LED_SEC1, USB_POWER, USB_POWER1, BEEPER
};
enum { LED_ON, LED_OFF, LED_FLASH };

#ifdef CDEBUG
extern int mcoreleft(void);
extern int coreleft(void);
static void cdebug(char *function)
{
	// FILE *in = fopen ("/tmp/console.log", "a");
	fprintf(stderr, "free ram in %s = %d (malloc %d)\n", function,
		coreleft(), mcoreleft());
	// fclose (in);
}

#else
#define cdebug(a)
#endif
extern int first_time(void);

extern int set_register_value(unsigned short port_addr,
			      unsigned short option_content);
extern unsigned int get_register_value(unsigned short id, unsigned short num);
// extern int sys_netdev_ioctl(int family, int socket, char *if_name, int
// cmd, struct ifreq *ifr);

int ct_openlog(const char *ident, int option, int facility, char *log_name);
void ct_syslog(int level, int enable, const char *fmt, ...);
void ct_logger(int level, const char *fmt, ...);
struct wl_assoc_mac *get_wl_assoc_mac(int instance, int *c);

extern struct detect_wans *detect_protocol(char *wan_face, char *lan_face,
					   char *type);

enum { WL = 0,
	DIAG = 1,
	// SES_LED1 = 2,
	// SES_LED2 = 3,
	SES_BUTTON = 4,
	DMZ = 7
};

enum { START_LED, STOP_LED, MALFUNCTION_LED };

typedef enum { ACT_IDLE,
	ACT_TFTP_UPGRADE,
	ACT_WEB_UPGRADE,
#ifdef HAVE_HTTPS
	ACT_WEBS_UPGRADE,
#endif
	ACT_SW_RESTORE,
	ACT_HW_RESTORE,
	ACT_ERASE_NVRAM,
	ACT_NVRAM_COMMIT,
	ACT_UNKNOWN
} ACTION;

enum { UNKNOWN_BOOT = -1, PMON_BOOT, CFE_BOOT };

enum { BCM4702_CHIP, BCM4712_CHIP, BCM5325E_CHIP, BCM5350_CHIP, BCM5365_CHIP,
	BCM4704_BCM5325F_CHIP,
	BCM5352E_CHIP, BCM4712_BCM5325E_CHIP, BCM4704_BCM5325F_EWC_CHIP,
	BCM4705_BCM5397_EWC_CHIP, BCM5354G_CHIP, BCM4705L_BCM5325E_EWC_CHIP,
	BCM4705G_BCM5395S_EWC_CHIP,
	NO_DEFINE_CHIP
};

enum { FIRST, SECOND };

enum { SYSLOG_LOG = 1, SYSLOG_DEBUG, CONSOLE_ONLY, LOG_CONSOLE, DEBUG_CONSOLE };

#define ACTION(cmd)	buf_to_file(ACTION_FILE, cmd)

struct dns_lists {
	int num_servers;
	char dns_server[4][16];
};

#define NOT_USING	0
#define USING		1

struct wl_assoc_mac {
	char mac[18];
};

struct mtu_lists {
	char *proto;		/* protocol */
	char *min;		/* min mtu */
	char *max;		/* max mtu */
};

struct detect_wans {
	int proto;
	int count;
	char *name;
	char desc[1024];
};

#define	PROTO_DHCP	0
#define	PROTO_STATIC	1
#define	PROTO_PPPOE	2
#define	PROTO_PPTP	3
#define	PROTO_L2TP	4
#define	PROTO_HB	5
#define	PROTO_ERROR	-1

#define PPP_PSEUDO_IP	"10.64.64.64"
#define PPP_PSEUDO_NM	"255.255.255.255"
#define PPP_PSEUDO_GW	"10.112.112.112"

#define PING_TMP	"/tmp/ping.log"
// #define TRACEROUTE_TMP "/tmp/traceroute.log"

#define RESOLV_FILE	"/tmp/resolv.conf"
#define RESOLV_FORW	"/tmp/resolv.dnsmasq"
#define HOSTS_FILE	"/tmp/hosts"

#define LOG_FILE	"/var/log/mess"

#define ACTION_FILE	"/tmp/action"

#define split(word, wordlist, next, delim) \
	for (next = wordlist, \
	     strncpy(word, next, sizeof(word)), \
	     word[(next=strstr(next, delim)) ? strstr(word, delim) - word : sizeof(word) - 1] = '\0', \
	     next = next ? next + sizeof(delim) - 1 : NULL ; \
	     strlen(word); \
	     next = next ? : "", \
	     strncpy(word, next, sizeof(word)), \
	     word[(next=strstr(next, delim)) ? strstr(word, delim) - word : sizeof(word) - 1] = '\0', \
	     next = next ? next + sizeof(delim) - 1 : NULL)

#define STRUCT_LEN(name)    sizeof(name)/sizeof(name[0])

#define printHEX(str,len) { \
	int i; \
	for (i=0 ; i<len ; i++) { \
		printf("%02X ", (unsigned char)*(str+i)); \
		if(((i+1)%16) == 0) printf("- "); \
		if(((i+1)%32) == 0) printf("\n"); \
	} \
	printf("\n\n"); \
}

#define printASC(str,len) { \
	int i; \
	for (i=0 ; i<len ; i++) { \
		printf("%c", (unsigned char)*(str+i)); \
		if(((i+1)%16) == 0) printf("- "); \
		if(((i+1)%32) == 0) printf("\n"); \
	} \
	printf("\n\n"); \
}

void set_gpio(int gpio, int value);

int get_gpio(int gpio);

#ifdef HAVE_OLED
void lcdmessage(char *message);
void initlcd(void);
void lcdmessaged(char *dual, char *message);
#else
#define initlcd()
#define lcdmessage(a)
#define lcdmessaged(a,b)
#endif

extern char *getBridgeMTU(char *);
extern char *getMTU(char *);

/* NF packet marks */
char *get_NFServiceMark(char *service, uint32 mark);

#ifdef HAVE_SVQOS
#define qos_nfmark(x) get_NFServiceMark("QOS", (uint32)(x))

#if !(defined(ARCH_broadcom) && !defined(HAVE_BCMMODERN))
extern char *get_tcfmark(uint32 mark);
#endif

extern char *get_wshaper_dev(void);
extern char *get_mtu_val(void);
extern void add_client_mac_srvfilter(char *name, char *type, char *data, char *level, int base, char *client);
extern void add_client_ip_srvfilter(char *name, char *type, char *data, char *level, int base, char *client);
#ifdef HAVE_AQOS
extern void add_usermac(char *mac, int idx, char *upstream, char *downstream, char *lanstream);
extern void add_userip(char *ip, int idx, char *upstream, char *downstream, char *lanstream);
extern void add_client_classes(unsigned int base, unsigned int uprate, unsigned int downrate, unsigned int lanrate, unsigned int level);
#else
extern void add_client_classes(unsigned int base, unsigned int level);
#endif
#endif

void getHostName(char *buf, char *ip);
int ishexit(char c);
int haswifi(void);
int sv_valid_hwaddr(char *value);
int sv_valid_ipaddr(char *value);
int sv_valid_range(char *value, int low, int high);
int sv_valid_statics(char *value);
void get_network(char *ipaddr, char *netmask);
int get_net(char *netmask);
void get_broadcast(char *ipaddr, char *netmask);
int route_manip(int cmd, char *name, int metric, char *dst, char *gateway,
		char *genmask);
int route_add(char *name, int metric, char *dst, char *gateway, char *genmask);
int route_del(char *name, int metric, char *dst, char *gateway, char *genmask);
extern char *psname(int pid, char *buffer, int maxlen);
extern int pidof(const char *name);
extern int killall(const char *name, int sig);
extern int getifcount(const char *ifprefix);
extern int getIfList(char *buffer, const char *ifprefix);
extern void getIfLists(char *eths, int size);
extern int ifexists(const char *ifname);
extern void getinterfacelist(const char *ifprefix, char *buffer);
extern int count_processes(char *pidName);
#ifdef HAVE_ATH9K
extern int is_ath9k(const char *prefix);
extern int getath9kdevicecount(void);
#endif

extern char *get3GDeviceVendor(void);


int isGrep(char *string, char *cmp);
int softkill(char *name);

int getmask(char *netmask);
int doMultiCast(void);
int getMTD(char *name);
void getIPFromName(char *name, char *ip);

#define DEFAULT_USER "bJ/GddyoJuiU2"
#define DEFAULT_PASS "bJz7PcC1rCRJQ"

#define MAX_WDS_DEVS 10

#define OLD_NAME_IP	"/tmp/.old_name_ip"

#ifndef MAX_LEASES
#define MAX_LEASES 254
#endif

struct wl_client_mac {
	char hostname[32];
	char ipaddr[20];
	char hwaddr[20];
	int status;		// 0:offline 1:online
	int check;
};

extern int dd_timer_delete(timer_t timer);
extern int dd_timer_create(clockid_t clock_id, struct sigevent *evp, timer_t * pTimer);
extern int dd_timer_connect(timer_t timerid, void (*routine) (timer_t, int), int arg);
extern int dd_timer_settime(timer_t timerid, int flags, const struct itimerspec *value, struct itimerspec *ovalue);

int endswith(char *str, char *cmp);

int isListed(char *listname, char *value);
void addList(char *listname, char *value);
int searchfor(FILE * fp, char *str, int scansize);
int insmod(char *module);
void rmmod(char *module);

int nvram_backup(char *filename);

int nvram_restore(char *filename);

void nvram_clear(void);
int nvram_critical(char *name);

int do80211priv(const char *ifname, int op, void *data, size_t len);
int getsocket(void);
void closesocket(void);
int isEMP(char *ifname);
int isXR36(char *ifname);
int isFXXN_PRO(char *ifname);
char *get3GControlDevice(void);
int mac80211_get_maxmcs(char *interface);
int get_ath9k_phy_idx(int idx);
int get_ath9k_phy_ifname(const char *ifname);

void getPortMapping(int *vlanmap);

u_int64_t freediskSpace(char *path);

char *getIsoName(const char *country);
int has_gateway(void);		// return 1 if nat/gateway mode is enabled for wan
#if defined(HAVE_RT2880) || defined(HAVE_RT61)
char *getRADev(char *prefix);
#endif

#ifndef HAVE_SYSLOG
#define dd_syslog(a, args...) do { } while(0);
#else
#define dd_syslog(a, args...) syslog( a,## args);
#endif
#endif

void *getUEnv(char *name);

/* gartarp */

#ifndef unlikely
#define unlikely(x)     __builtin_expect((x), 0)
#endif

#define IP_ALEN		4

/*
struct arph {
	uint16_t hw_type;

	#define ARPOP_BROADCAST         1
	#define ARPHDR_ETHER            1
	uint16_t proto_type;

	char ha_len;
	char pa_len;
	uint16_t opcode;
	char source_add[ETH_ALEN];
	char source_ip[IP_ALEN];
	char dest_add[ETH_ALEN];
	char dest_ip[IP_ALEN];


} __attribute__((packed));
*/

#define ARP_HLEN	sizeof(struct arph) + ETH_HLEN
#define BCAST		"\xff\xff\xff\xff\xff\xff"

int gratarp_main(char *iface);

int writeproc(char *path,char *value);

int writevaproc(char *value, char *fmt,...);

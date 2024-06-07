#ifndef UTILS_H
#define UTILS_H

#ifdef CDEBUG
#include <shutils.h>
#include <malloc.h>
#include <cy_conf.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <typedefs.h>

#ifndef sys_reboot
#define sys_reboot() eval("event", "3", "1", "15")
#endif
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
#define ROUTER_BOARD_STORM 0x6201 // value 1 is a fake to enable reset button code. real gpio is 60

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
#define ROUTER_NETGEAR_WNR3500L 0x7414 //(18)
// Netgear WNR3500L V2 (BCM47186B0 cpu, 128MB ram,  128MB nand flash, int. antennae can be replaced with ext)
#define ROUTER_NETGEAR_WNR3500LV2 0x7416

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

#define ROUTER_BOARD_BS2M 0x831c //bullet 2m
#define ROUTER_BOARD_BS5M 0x841c //bullet 5m
#define ROUTER_BOARD_R2M 0x851c //rocket 2m
#define ROUTER_BOARD_R5M 0x861c //rocket 5m
#define ROUTER_BOARD_NS2M 0x871c //nanostation 2m
#define ROUTER_BOARD_NS5M 0x881c //nanostation 5m

// Asus RT-N10 (BCM5356 cpu, 4MB serial flash / 16MB ram, fccid: Asus MSQ-RTN10)
#define ROUTER_ASUS_RTN10 0x8913

// Asus RT-N12 (BCM4716B0 cpu, 4MB serial flash / 32MB ram, 5325E switch, fccid: Asus MSQ-RTN12)
#define ROUTER_ASUS_RTN12 0x8a11

// Asus RT-N16 (BCM4718A cpu, 32MB flash / 128MB ram, BCM53115 giga switch, fccid: Asus MSQRTN16)
#define ROUTER_ASUS_RTN16 0x8b16

#define ROUTER_BOARD_BR6574N 0x8c1a

#ifdef HAVE_HORNET
#define ROUTER_BOARD_WHRHPGN 0x8d1c //Buffalo WHR HP GN AR7240 / 4 MB Flash / 32 MB RAM
#elif HAVE_CARAMBOLA
#define ROUTER_BOARD_WHRHPGN 0x8d1b //Buffalo WHR HP GN AR7240 / 4 MB Flash / 32 MB RAM
#elif HAVE_DIR825C1
#define ROUTER_BOARD_WHRHPGN 0x8d111 //Buffalo WHR HP GN AR7240 / 4 MB Flash / 32 MB RAM
#elif HAVE_WASP
#define ROUTER_BOARD_WHRHPGN 0x8d110 //Buffalo WHR HP GN AR7240 / 4 MB Flash / 32 MB RAM
#elif HAVE_WNR2200
#define ROUTER_BOARD_WHRHPGN 0x8d126 //Buffalo WHR HP GN AR7240 / 4 MB Flash / 32 MB RAM
#elif HAVE_WNR2000
#define ROUTER_BOARD_WHRHPGN 0x8d128 //Buffalo WHR HP GN AR7240 / 4 MB Flash / 32 MB RAM
#else
#define ROUTER_BOARD_WHRHPGN 0x8d1b //Buffalo WHR HP GN AR7240 / 4 MB Flash / 32 MB RAM
#endif
// Dynex DX-NRUTER (BCM4703 cpu, 4MB flash / 32MB ram, BCM5325E switch, fccid: Belkin K7SDXNRUTER)
#define ROUTER_DYNEX_DX_NRUTER 0x8e12

#define ROUTER_BOARD_OPENRISC 0x8f00

#define ROUTER_BOARD_ASUS_RTN13U 0x901a

// Linksys WRT160NL (Atheros 9130, 8MB flash / 32MB ram, fccid: Linksys Q87-WRT160NL)
// Linksys E2100L (Atheros 9130, 8MB flash / 32MB ram, fccid: Linksys Q87-E2100L)
#define ROUTER_BOARD_WRT160NL 0x9101 // fake id, reset button is gpio 21

// NetCore NW618 / Rosewill RNX-GX4 (BCM5354 cpu, 4MB serial flash / 16MB ram, fccid: Rosewill W6RRNX-GX4)
#define ROUTER_NETCORE_NW618 0x9216

#define ROUTER_BOARD_W502U 0x931a

#define ROUTER_BOARD_DIR615D 0x941a

#define ROUTER_BOARD_AR690W 0x9519

#define ROUTER_BOARD_RB600 0x960f

// Netgear WNR2000 v2 (BCM4716B0 cpu, 4MB serial flash / 32MB ram, fccid: Netgear PY309100105)
#define ROUTER_NETGEAR_WNR2000V2 0x9711

#define ROUTER_BOARD_GW2388 0x9800
// Belkin Share Max F5D8235v3 (BCM53115 giga switch, BCM4718 cpu, 8MB flash / 32MB ram, fccid: Belkin K7SF5D8235V3)
#define ROUTER_BELKIN_F5D8235V3 0x9916

#define ROUTER_BOARD_GW2380 0x9a00

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

#define ROUTER_BOARD_UNIFI 0xae1c //bullet 2m

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

#define ROUTER_BOARD_WDR4900 0xc215

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

#define ROUTER_NETGEAR_R7000P 0xdf16

#define ROUTER_BOARD_UNIWIP 0xd001

#define ROUTER_LINKSYS_EA6900 0xd11b

#define ROUTER_LINKSYS_EA6500V2 0xd21b

#define ROUTER_TRENDNET_TEW812 0xd31b

#define ROUTER_TRENDNET_TEW811 0xd41b

#define ROUTER_LINKSYS_EA6700 0xd51b

#define ROUTER_BOARD_TI 0xd60f

#define ROUTER_NETGEAR_AC1450 0xd716

#define ROUTER_ASUS_RTN18U 0xd817 // BCM4708 SMP 800 Mhz 128 MB Nand Flash, 128 MB Ram

#define ROUTER_NETGEAR_EX6200 0xd916 // BCM4708 SMP 800 Mhz 8MB SPI Flash, 128 MB Ram

#define ROUTER_BUFFALO_WXR1900DHP 0xda10f // BCM4709 SMP 1000 Mhz 128 MB Nand Flash, 512 MB Ram

#define ROUTER_ASUS_AC87U 0xdb1b // BCM4708 SMP 800 Mhz 128 MB Nand Flash, 128 MB Ram

#define ROUTER_UBNT_EROUTERLITE 0xdc1b // Edgerouter Lite  (e100) Cavium Octeon SMP
#define ROUTER_UBNT_EROUTERPRO 0xdd10 // Edgerouter Lite  (e200) Cavium Octeon SMP

#define ROUTER_NETGEAR_R8000 0xde16

#define ROUTER_BOARD_E1700 0xdf11

#define ROUTER_DLINK_DIR865 0xe00f

#define ROUTER_BOARD_AIRROUTER 0xe11c

#define ROUTER_UBNT_UNIFIAC 0xe218

#define ROUTER_DIR810L 0xe311

#define ROUTER_WRT_1900AC 0xe411

#define ROUTER_DIR860LB1 0xe417

#define ROUTER_TPLINK_ARCHERC9 0xe513

#define ROUTER_DLINK_DIR890 0xe6111

#define ROUTER_DLINK_DIR880 0xe71b

#define ROUTER_DLINK_DIR860 0xe81b

#define ROUTER_TRENDNET_TEW828 0xe91b

#define ROUTER_DLINK_DIR868C 0xea1b

#define ROUTER_ASUS_AC3200 0xeb1b

#define ROUTER_LINKSYS_EA6400 0xec1b

#define ROUTER_WRT_1200AC 0xed11

#define ROUTER_WRT_1900ACV2 0xee11

#define ROUTER_DLINK_DIR885 0xef111

#define ROUTER_DLINK_DIR895 0xf0111

#define ROUTER_ASUS_AC88U 0xf11b

#define ROUTER_ASUS_AC5300 0xf21b

#define ROUTER_WRT_1900ACS 0xf311

#define ROUTER_LINKSYS_EA6350 0xf41b

#define ROUTER_NETGEAR_R7500 0xf50e

#define ROUTER_LINKSYS_EA8500 0xf60e

#define ROUTER_TRENDNET_TEW827 0xf701

#define ROUTER_NETGEAR_R7500V2 0xf70e

#define ROUTER_NETGEAR_R8500 0xf81a

#define ROUTER_UBNT_UAPAC 0xf912

#define ROUTER_ASUS_AC1200 0xfa15

#define ROUTER_NETGEAR_R7800 0xfb0e

#define ROUTER_NETGEAR_R6400 0xfc15

#define ROUTER_ASUS_AC3100 0xfd1b

#define ROUTER_ASROCK_G10 0xfe01

#define ROUTER_TPLINK_ARCHERC3150 0xff111

#define ROUTER_NETGEAR_R9000 0x100111

#define ROUTER_WRT_3200ACM 0x101101

#define ROUTER_DIR882 0x10210f // 15 reset button, 7 outer button 18. mid button

#define ROUTER_BOARD_NS5MXW 0x10310c //nanostation 5mxw

#define ROUTER_LINKSYS_EA9500 0x10410f

#define ROUTER_NETGEAR_R6400V2 0x105105

#define ROUTER_BOARD_GW6400 0x106000

#define ROUTER_WRT_32X 0x102101

#define ROUTER_TPLINK_ARCHERC3200 0x103111

#define ROUTER_NETGEAR_R6700V3 0x106105

#define ROUTER_UBNT_UAPACPRO 0x107102

#define ROUTER_BOARD_UNIFI_V2 0x10810c

#define ROUTER_BOARD_XD9531 0x10810d

#define ROUTER_UBNT_NANOAC 0x109102

#define ROUTER_HABANERO 0x109108

#define ROUTER_UBNT_POWERBEAMAC_GEN2 0x10a10c

#define ROUTER_R6800 0x10b10c

#define ROUTER_R6850 0x10c10e

#define ROUTER_R6220 0x10d10e

#define ROUTER_TPLINK_ARCHERC8 0x10e107

#define ROUTER_TPLINK_ARCHERC8_V4 0x10e103

#define ROUTER_NETGEAR_R6200 0x10f103

#define ROUTER_ASUS_AC58U 0x110104

#define ROUTER_LINKSYS_EA8300 0x11113b

#define ROUTER_IPQ6018 0x1120ff

#define ROUTER_LINKSYS_MR7350 0x113139

#define NVROUTER "DD_BOARD"
#define NVROUTER_ALT "alternate_name"

static inline int startswith(char *source, char *cmp)
{
	return !strncmp(source, cmp, strlen(cmp));
}

typedef struct {
	const char *tz_name;
	const char *tz_string;
} TIMEZONE_TO_TZSTRING;

extern void update_timezone(void);
extern TIMEZONE_TO_TZSTRING allTimezones[];

extern void setWifiPass();
extern char *getBridge(char *ifname, char *word);
extern char *getRealBridge(char *ifname, char *word);
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
extern char *substring(int start, int stop, const char *src, char *dst, size_t len);
extern void strtrim_right(char *p, int c);
extern unsigned int daysformonth(unsigned int month, unsigned int year);
const char *getifaddr(char *buf, char *ifname, int family, int linklocal);
const char *getifaddr_any(char *buf, char *ifname, int family);
extern int weekday(int month, int day, int year);
extern int getRouterBrand(void);
extern char *getRouter(void);
extern int diag_led(int type, int act);
extern int C_led(int i);
extern int get_single_ip(char *ipaddr, int which);
extern char *get_mac_from_ip(char *mac, char *ip);
extern int dns_to_resolv(void);
extern char *safe_get_wan_face(char *buffer);

extern int getBridgeSTP(char *br, char *word);
extern char *getBridgeSTPType(char *br, char *word);

extern int getBridgeForwardDelay(char *br);
extern int getBridgeMaxAge(char *br);
extern unsigned char *get_ether_hwaddr(const char *name, unsigned char *hwaddr);
extern int set_ether_hwaddr(const char *name, unsigned char *hwaddr);

extern char *get_hwaddr(const char *name, char *hwaddr);
extern int set_hwaddr(const char *name, char *hwaddr);

extern int check_wan_link(int num);
extern int wanactive(char *wanaddr);
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
#define MD5_OUT_BUFSIZE 36
extern unsigned char *zencrypt(unsigned char *passwd, unsigned char *passout);

extern void getLANMac(char *newmac);
extern void getWirelessMac(char *newmac, int instance);
extern void getWANMac(char *newmac);
extern char *cpustring(void);
extern int find_pattern(const char *data, size_t dlen, const char *pattern, size_t plen, char term, unsigned int *numoff,
			unsigned int *numlen);
extern int find_match_pattern(char *name, size_t mlen, const char *data, const char *pattern, char *def);

struct ledconfig {
	unsigned short power_gpio;
	unsigned short beeper_gpio;
	unsigned short diag_gpio;
	unsigned short diag_gpio_disabled;
	unsigned short dmz_gpio;
	unsigned short connected_gpio;
	unsigned short disconnected_gpio;
	unsigned short bridge_gpio;
	unsigned short vpn_gpio;
	unsigned short ses_gpio; // use for SES1 (Linksys), AOSS (Buffalo)
	unsigned short ses2_gpio;
	unsigned short wlan_gpio; // wlan button led R7000
	unsigned short wlan0_gpio; // use this only if wlan led is not controlled by hardware!
	unsigned short wlan1_gpio;
	unsigned short wlan2_gpio;
	unsigned short usb_gpio;
	unsigned short usb_gpio1;
	unsigned short sec_gpio; // generic
	unsigned short sec0_gpio; // security leds, wrt600n
	unsigned short sec1_gpio;
	unsigned short usb_power;
	unsigned short usb_power1;
	unsigned short v1func;
	unsigned short connblue;
	unsigned short poe_gpio;
};

extern int led_control(int type, int act);
enum {
	LED_POWER,
	LED_DIAG,
	LED_DIAG_DISABLED,
	LED_DMZ,
	LED_CONNECTED,
	LED_DISCONNECTED,
	LED_BRIDGE,
	LED_VPN,
	LED_SES,
	LED_SES2,
	LED_WLAN,
	LED_WLAN0,
	LED_WLAN1,
	LED_WLAN2,
	LED_USB,
	LED_USB1,
	LED_SEC,
	LED_SEC0,
	LED_SEC1,
	USB_POWER,
	USB_POWER1,
	BEEPER,
	POE_GPIO,
	GPIO_CHECK
};
enum { LED_ON, LED_OFF, LED_FLASH };

#ifdef CDEBUG
extern int mcoreleft(void);
extern int coreleft(void);
static void cdebug(char *function)
{
	// FILE *in = fopen ("/tmp/console.log", "a");
	fprintf(stderr, "free ram in %s = %d (malloc %d)\n", function, coreleft(), mcoreleft());
	// fclose (in);
}

#else
#define cdebug(a)
#endif
extern int first_time(void);

extern int set_register_value(unsigned short port_addr, unsigned short option_content);
extern unsigned int get_register_value(unsigned short id, unsigned short num);
// extern int sys_netdev_ioctl(int family, int socket, char *if_name, int
// cmd, struct ifreq *ifr);

int ct_openlog(const char *ident, int option, int facility, char *log_name);
void ct_syslog(int level, int enable, const char *fmt, ...);
void ct_logger(int level, const char *fmt, ...);
struct wl_assoc_mac *get_wl_assoc_mac(char *prefix, int *c);

extern struct detect_wans *detect_protocol(char *wan_face, char *lan_face, char *type);

enum {
	WL = 0,
	DIAG = 1,
	// SES_LED1 = 2,
	// SES_LED2 = 3,
	SES_BUTTON = 4,
	DMZ = 7
};

enum { START_LED, STOP_LED, MALFUNCTION_LED };

typedef enum {
	ACT_IDLE,
	//      ACT_TFTP_UPGRADE,
	ACT_WEB_UPGRADE,
	ACT_WEBS_UPGRADE,
	ACT_SW_RESTORE,
	ACT_HW_RESTORE,
	ACT_ERASE_NVRAM,
	ACT_NVRAM_COMMIT,
	ACT_UNKNOWN
} WEBACTION;

enum { UNKNOWN_BOOT = -1, PMON_BOOT, CFE_BOOT };

enum {
	BCM4702_CHIP,
	BCM4712_CHIP,
	BCM5325E_CHIP,
	BCM5350_CHIP,
	BCM5365_CHIP,
	BCM4704_BCM5325F_CHIP,
	BCM5352E_CHIP,
	BCM4712_BCM5325E_CHIP,
	BCM4704_BCM5325F_EWC_CHIP,
	BCM4705_BCM5397_EWC_CHIP,
	BCM5354G_CHIP,
	BCM4705L_BCM5325E_EWC_CHIP,
	BCM4705G_BCM5395S_EWC_CHIP,
	NO_DEFINE_CHIP
};

enum { FIRST, SECOND };

#define ACTION(cmd) buf_to_file(ACTION_FILE, cmd)
struct dns_entry {
	int type; //0 = isp, 1 = user defined
	int ipv6; // 0 = ipv4, 1 = ipv6
	char *ip;
};

struct dns_lists {
	int num_servers;
	struct dns_entry *dns_server;
};

extern struct dns_entry *get_dns_entry(struct dns_lists *dns_list, int idx);
extern struct dns_lists *get_dns_list(int ipv6);
extern void free_dns_list(struct dns_lists *list);

#define NOT_USING 0
#define USING 1

struct wl_assoc_mac {
	char mac[18];
};

struct mtu_lists {
	char *proto; /* protocol */
	char *min; /* min mtu */
	char *max; /* max mtu */
};

struct detect_wans {
	int proto;
	int count;
	char *name;
	char desc[1024];
};

#define GIF_LINKLOCAL 0x0001 /* return link-local addr */
#define GIF_PREFIXLEN 0x0002 /* return addr & prefix */

#define PROTO_DHCP 0
#define PROTO_STATIC 1
#define PROTO_PPPOE 2
#define PROTO_PPTP 3
#define PROTO_L2TP 4
#define PROTO_HB 5
#define PROTO_ERROR -1

#define PPP_PSEUDO_IP "10.64.64.64"
#define PPP_PSEUDO_NM "255.255.255.255"
#define PPP_PSEUDO_GW "10.112.112.112"

#define PING_TMP "/tmp/ping.log"
// #define TRACEROUTE_TMP "/tmp/traceroute.log"

#define RESOLV_FILE "/tmp/resolv.conf"
#define RESOLV_FORW "/tmp/resolv.dnsmasq"
#define HOSTS_FILE "/tmp/hosts"

#define LOG_FILE "/var/log/mess"

#define ACTION_FILE "/tmp/action"

#define split(word, wordlist, next, delim)                                                             \
	for (next = wordlist, strncpy(word, next, sizeof(word)),                                       \
	    word[(next = strstr(next, delim)) ? strstr(word, delim) - word : sizeof(word) - 1] = '\0', \
	    next = next ? next + sizeof(delim) - 1 : NULL;                                             \
	     word[0]; next = next ?: "", strncpy(word, next, sizeof(word)),                            \
	    word[(next = strstr(next, delim)) ? strstr(word, delim) - word : sizeof(word) - 1] = '\0', \
	    next = next ? next + sizeof(delim) - 1 : NULL)

#define STRUCT_LEN(name) sizeof(name) / sizeof(name[0])

#define printHEX(str, len)                                          \
	{                                                           \
		int i;                                              \
		for (i = 0; i < len; i++) {                         \
			printf("%02X ", (unsigned char)*(str + i)); \
			if (((i + 1) % 16) == 0)                    \
				printf("- ");                       \
			if (((i + 1) % 32) == 0)                    \
				printf("\n");                       \
		}                                                   \
		printf("\n\n");                                     \
	}

#define printASC(str, len)                                       \
	{                                                        \
		int i;                                           \
		for (i = 0; i < len; i++) {                      \
			printf("%c", (unsigned char)*(str + i)); \
			if (((i + 1) % 16) == 0)                 \
				printf("- ");                    \
			if (((i + 1) % 32) == 0)                 \
				printf("\n");                    \
		}                                                \
		printf("\n\n");                                  \
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
#define lcdmessaged(a, b)
#endif

extern char *getBridgeMTU(const char *, char *word);
extern char *getMTU(char *);
extern int getBridgeSTP(char *br, char *word);
extern char *get_NFServiceMark(char *buffer, size_t len, char *service, uint32 mark);

char *qos_nfmark(char *buffer, size_t len, uint32 x);

#if !defined(ARCH_broadcom) || defined(HAVE_BCMMODERN)
extern char *get_tcfmark(char *tcfmark, uint32 mark, int seg);
#endif

extern int get_mtu_val(void);
extern void add_client_dev_srvfilter(char *name, char *type, char *data, int level, int base, char *chain);
extern void add_client_mac_srvfilter(char *name, char *type, char *data, int level, int base, char *client);
extern void add_client_ip_srvfilter(char *name, char *type, char *data, int level, int base, char *client);
void deinit_qos(const char *wandev, const char *imq_wan, const char *imq_lan);
void init_qos(const char *type, int up, int down, const char *wandev, int mtu, const char *imq_wan, const char *aqd,
	      const char *imq_lan);
void init_ackprio(const char *wandev);
extern void add_usermac(char *mac, int idx, int upstream, int downstream, int lanstream);
extern void add_userip(char *ip, int idx, int upstream, int downstream, int lanstream);
extern void add_client_classes(unsigned int base, unsigned int uprate, unsigned int downrate, unsigned int lanrate,
			       unsigned int level);

void getHostName(char *buf, char *ip);
int ishexit(char c);
int haswifi(void);
int sv_valid_hwaddr(char *value);
int sv_valid_ipaddr(char *value);
int sv_valid_range(char *value, int low, int high);
int sv_valid_statics(char *value);
void get_network(char *ipaddr, char *netmask);
int isbridge(char *name);
int isbridged(char *name);
int has_multicast_to_unicast(char *name);
int isvlan(char *name);
void get_broadcast(char *ipaddr, size_t len, char *netmask);
int route_manip(int cmd, char *name, int metric, char *dst, char *gateway, char *genmask);
int route_add(char *name, int metric, char *dst, char *gateway, char *genmask);
int route_del(char *name, int metric, char *dst, char *gateway, char *genmask);
extern char *psname(int pid, char *buffer, int maxlen);
extern int pidof(const char *name);
extern int killall(const char *name, int sig);
extern int getifcount(const char *ifprefix);
extern int getIfByIdx(char *ifname, int index);
extern int getIfList(char *buffer, const char *ifprefix);
extern int getIfListNoPorts(char *buffer, const char *ifprefix);
extern int getIfListB(char *buffer, const char *ifprefix, int bridgesonly, int nosort, int noport);
extern void getIfLists(char *eths, int size);
extern int ifexists(const char *ifname);
extern void getinterfacelist(const char *ifprefix, char *buffer);
extern int count_processes(char *pidName);
char *hash_file_string(char *filename, char *hashbuf, size_t len);
char *hash_file(char *filename, char *hashbuf);
char *hash_string(char *str, char *hashbuf, size_t len);

#ifdef HAVE_ATH5K
extern int is_ath5k(const char *prefix);
extern int is_ath5k_ahb(const char *prefix);
extern int is_ath5k_pci(const char *prefix);
#else
static inline int is_ath5k(const char *prefix)
{
	return 0;
}

static inline int is_ath5k_pci(const char *prefix)
{
	return 0;
}

static inline int is_ath5k_ahb(const char *prefix)
{
	return 0;
}
#endif
#ifdef HAVE_WIL6210
int is_wil6210(const char *prefix);
#else
static inline int is_wil6210(const char *prefix)
{
	return 0;
}
#endif

#ifdef HAVE_ATH9K
extern int is_mac80211(const char *prefix);
extern int is_ap8x(char *prefix);
extern int has_channelsurvey(const char *prefix);
extern int has_nolivesurvey(const char *prefix);
extern int has_qboost(const char *prefix);
extern int has_no_apmode(const char *prefix);
extern int has_wdsap(const char *prefix);
extern int has_qboost_tdma(const char *prefix);
extern int has_beacon_limit(const char *prefix);
extern int has_spectralscanning(const char *prefix);
extern int has_half(const char *prefix);
extern int has_quarter(const char *prefix);
extern int has_qam256(const char *prefix);
extern int has_wave2(const char *prefix);
extern int has_ar900b(const char *prefix);
extern int has_ax(const char *prefix);
extern int has_dualband(const char *prefix);
extern int has_subquarter(const char *prefix);
extern int has_fwswitch(const char *prefix);
extern int getath9kdevicecount(void);
#else

static inline int is_mac80211(const char *prefix)
{
	return 0;
}

static inline int is_ap8x(char *prefix)
{
	return 0;
}

static inline int has_spectralscanning(char *prefix)
{
	return 0;
}

static inline int has_qboost(char *prefix)
{
	return 0;
}

static inline int has_qam256(char *prefix)
{
	return 0;
}

static inline int has_wave2(char *prefix)
{
	return 0;
}

static inline int has_ar900b(char *prefix)
{
	return 0;
}

static inline int has_ax(char *prefix)
{
	return 0;
}

static inline int has_dualband(char *prefix)
{
	return 0;
}

static inline int has_no_apmode(char *prefix)
{
	return 0;
}

#ifdef HAVE_MADWIFI
static inline int has_wdsap(char *prefix)
{
	return 1;
}
#else
static inline int has_wdsap(char *prefix)
{
	return 0;
}
#endif

static inline int has_nolivesurvey(char *prefix)
{
	return 0;
}

static inline int has_qboost_tdma(char *prefix)
{
	return 0;
}

static inline int has_beacon_limit(char *prefix)
{
	return 0;
}

static inline int has_channelsurvey(char *prefix)
{
	return 0;
}

static inline int has_half(char *prefix)
{
	return 0;
}

static inline int has_quarter(char *prefix)
{
	return 0;
}

static inline int has_subquarter(char *prefix)
{
	return 0;
}

static inline int has_fwswitch(char *prefix)
{
	return 0;
}

#endif
#ifdef HAVE_ATH10K
extern int is_ath10k(const char *prefix);
#else
static inline int is_ath10k(const char *prefix)
{
	return 0;
}
#endif
#ifdef HAVE_ATH11K
extern int is_ath11k(const char *prefix);
#else
static inline int is_ath11k(const char *prefix)
{
	return 0;
}
#endif
#ifdef HAVE_BRCMFMAC
extern int is_brcmfmac(const char *prefix);
#else
static inline int is_brcmfmac(const char *prefix)
{
	return 0;
}
#endif
#ifdef HAVE_ATH9K
extern int is_ath9k(const char *prefix);
#else
static inline int is_ath9k(const char *prefix)
{
	return 0;
}
#endif
#ifdef HAVE_MVEBU
extern int is_mvebu(const char *prefix);
#else
static inline int is_mvebu(const char *prefix)
{
	return 0;
}
#endif
#if defined(HAVE_MT76) || defined(HAVE_ATH11K)
extern int is_mt7615(const char *prefix);
extern int is_mt7915(const char *prefix);
extern int is_mt7921(const char *prefix);
extern int is_mt7603(const char *prefix);
extern int is_mt76x0(const char *prefix);
extern int is_mt76x2(const char *prefix);
extern int is_mt76(const char *prefix);
extern int is_rt2880_wmac(const char *prefix);
extern int is_rt2880_pci(const char *prefix);
#else
static inline int is_rt2880_wmac(const char *prefix)
{
	return 0;
}

static inline int is_rt2880_pci(const char *prefix)
{
	return 0;
}

static inline int is_mt7615(const char *prefix)
{
	return 0;
}
static inline int is_mt7915(const char *prefix)
{
	return 0;
}
static inline int is_mt7921(const char *prefix)
{
	return 0;
}

static inline int is_mt7603(const char *prefix)
{
	return 0;
}

static inline int is_mt76x0(const char *prefix)
{
	return 0;
}

static inline int is_mt76x2(const char *prefix)
{
	return 0;
}

static inline int is_mt76(const char *prefix)
{
	return 0;
}
#endif
#ifdef HAVE_ATH9K
int getmaxvaps(const char *prefix);
#else
static inline int getmaxvaps(const char *prefix)
{
	return 16;
}
#endif

int registered_has_subquarter(void);

extern char *get3GDeviceVendor(void);

int isGrep(char *string, char *cmp);
int softkill(char *name);

int getmask(char *netmask);
int doMultiCast(void);
int getMTD(char *name);
void getIPFromName(char *name, char *ip, size_t len);
int jffs_mounted(void);

#define DEFAULT_USER "$1$gHo0JRUz$DhmErZyHtCmTxxKPPb13o."
#define DEFAULT_PASS "$1$gHo0JRUz$cbb1yYxlmSCh0mTlTUsV81"

#define MAX_WDS_DEVS 10

#define OLD_NAME_IP "/tmp/.old_name_ip"

#ifndef MAX_LEASES
#define MAX_LEASES 254
#endif

struct wl_client_mac {
	char hostname[32];
	char ipaddr[20];
	char hwaddr[20];
	int status; // 0:offline 1:online
	int check;
};
#include <time.h>

extern int dd_timer_delete(timer_t timer);
extern int dd_timer_create(clockid_t clock_id, struct sigevent *evp, timer_t *pTimer);
extern int dd_timer_connect(timer_t timerid, void (*routine)(timer_t, int), int arg);
extern int dd_timer_settime(timer_t timerid, int flags, const struct itimerspec *value, struct itimerspec *ovalue);

int endswith(char *str, char *cmp);
char *getdisc(void);
int isListed(char *listname, char *value);
void addList(char *listname, char *value);
int searchfor(FILE *fp, char *str, int scansize);
int insmod(char *module);
int modprobe(char *module);
void rmmod(char *module);

int writeint(char *path, int a);
int writestr(char *path, char *a);

int nvram_backup(char *filename);

int nvram_restore(char *filename, int force);

const char *getdefaultconfig(char *service, char *path, size_t len, char *configname);

void nvram_clear(void);
int nvram_critical(char *name);
void getSystemMac(char *mac);

int do80211priv(const char *ifname, int op, void *data, size_t len);
int getsocket(void);
void dd_closesocket(void);
int isEMP(char *ifname);
int isXR36(char *ifname);
#ifdef HAVE_MADWIFI
int isFXXN_PRO(char *ifname);
#else
static inline int isFXXN_PRO(char *ifname)
{
	return 0;
}
#endif
void get3GControlDevice(void);
int mac80211_get_maxmcs(char *interface);
int mac80211_get_maxvhtmcs(char *interface);
int get_ath9k_phy_idx(int idx);
int get_ath9k_phy_ifname(const char *ifname);
char *getUUID(char *buf);
char *getWifiDeviceName(const char *prefix, int *flags);

void getPortMapping(int *vlanmap);

u_int64_t freediskSpace(char *path);

const char *getIsoName(char *country);
int has_gateway(void); // return 1 if nat/gateway mode is enabled for wan
#if defined(HAVE_RT2880) || defined(HAVE_RT61)
char *getRADev(char *prefix);
#endif

#ifndef HAVE_SYSLOG
#define dd_syslog(a, args...) \
	do {                  \
	} while (0)
#define dd_loginfo(a, fmt, args...) \
	do {                        \
	} while (0)
#define dd_logdebug(a, fmt, args...) \
	do {                         \
	} while (0)
#define dd_logerror(a, fmt, args...) \
	do {                         \
	} while (0)
#define dd_logstart(a, ret) \
	do {                \
	} while (0)
#else
#define dd_syslog(a, args...) syslog(a, ##args);
void dd_loginfo(const char *servicename, const char *fmt, ...);
void dd_logdebug(const char *servicename, const char *fmt, ...);
void dd_logerror(const char *servicename, const char *fmt, ...);
void dd_logstart(const char *servicename, int ret);
#endif
#endif

void *getUEnv(char *name);

/* gartarp */

#ifndef unlikely
#define unlikely(x) __builtin_expect((x), 0)
#endif

#define IP_ALEN 4

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

#define ARP_HLEN sizeof(struct arph) + ETH_HLEN
#define BCAST "\xff\xff\xff\xff\xff\xff"

int writeproc(char *path, char *value);
int writeprocsysnet(char *path, char *value);
int writeprocsys(char *path, char *value);

void set_smp_affinity(int irq, int cpu);

int writevaproc(char *value, char *fmt, ...);

void MAC_ADD(char *mac);
void MAC_SUB(char *mac);
void mac_add(char *mac);
void mac_sub(char *mac);

extern char *get_ipfrominterface(char *ifname, char *ip);
extern struct in_addr inet_netaddr_of(struct in_addr addr, struct in_addr msk);
extern struct in_addr inet_bcastaddr_of(struct in_addr net, struct in_addr msk);
extern void inet_addr_to_cidr(struct in_addr addr, struct in_addr msk, char *cidr_buf);
extern int inet_cidr_to_addr(char *cidr_str, struct in_addr *addr, struct in_addr *msk);
char *getUnmountedDrives(void);
char *getMountedDrives(void);
char *getAllDrives(void);
char *strstrtok(char *str, char del);
struct in_addr *osl_ifaddr(const char *ifname, struct in_addr *inaddr);

#define ETHER_ADDR_STR_LEN 18 /* 18-bytes of Ethernet address buffer length */
#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6 /* 18-bytes of Ethernet address buffer length */

#ifndef ABS
#define ABS(a) (((a) < 0) ? -(a) : (a))
#endif /* ABS */

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif /* MIN */

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif /* MAX */

#ifndef OFFSETOF
#define OFFSETOF(type, member) ((uint)(uintptr) & ((type *)0)->member)
#endif

#ifndef ROUNDUP
#define ROUNDUP(x, y) ((((x) + ((y)-1)) / (y)) * (y))
#endif

#define NR_RULES 20

#ifdef HAVE_SWCONFIG

int getPortStatus(int port);
int has_igmpsnooping(void);

#endif

#define SYSCTL_BLACKLIST                                                                                                      \
	"base_reachable_time", "retrans_time", "nf_conntrack_max", "nf_conntrack_helper", "bridge-nf-call-arptables",         \
		"bridge-nf-call-ip6tables", "bridge-nf-call-iptables", "drop_caches", "ledpin", "softled", "default_qdisc",   \
		"tcp_bic", "tcp_westwood", "tcp_vegas_cong_avoid", "osf", "tcp_tw_recycle", "scan_unevictable_pages", "ctf0", \
		"teql0"

void sysctl_apply(void *priv, void (*callback)(char *path, char *nvname, char *name, char *sysval, void *priv));

#endif

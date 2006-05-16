#ifndef __IPT_IPP2P_H
#define __IPT_IPP2P_H
#define IPP2P_VERSION "0.8.0"

struct ipt_p2p_info {
    int cmd;
    int debug;
};

#endif //__IPT_IPP2P_H

#define SHORT_HAND_IPP2P	1 /* --ipp2p switch*/
//#define SHORT_HAND_DATA		4 /* --ipp2p-data switch*/
#define SHORT_HAND_NONE		5 /* no short hand*/

#define IPP2P_EDK		2
#define IPP2P_DATA_KAZAA	8
#define IPP2P_DATA_EDK		16
#define IPP2P_DATA_DC		32
#define IPP2P_DC		64
#define IPP2P_DATA_GNU		128
#define IPP2P_GNU		256
#define IPP2P_KAZAA		512
#define IPP2P_BIT		1024
#define IPP2P_APPLE		2048
#define IPP2P_SOUL		4096
#define IPP2P_WINMX		8192
#define IPP2P_ARES		16384


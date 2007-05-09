#ifndef _IPT_TCPLAG_H
#define _IPT_TCPLAG_H

struct ipt_tcplag
{
	unsigned char level;
	unsigned char prefix[ 15 ];
};

#endif
